/*
 * Includes
 * Necessary libraries and header files for the web server functionality.
 */
#include "web_server.h"
#include "communication.h"
#include "storage.h"
#include "utils.h"
#include "config.h"

/*
 * Function: getOTAHTML
 * Returns the HTML string for the firmware update (OTA) page.
 */
String getOTAHTML() {
  return R"raw(
    <!DOCTYPE html><html><body style='font-family:sans-serif;background:#222;color:#fff;text-align:center;padding:50px;'>
    <h1>Firmware Update</h1><form method='POST' action='/update' enctype='multipart/form-data'>
    <input type='file' name='update' style='margin-bottom:20px;'><br>
    <input type='submit' value='Upload & Update' style='padding:10px 20px;cursor:pointer;'>
    </form></body></html>
  )raw";
}

/*
 * Function: setupOTA
 * Configures the web server routes for handling Over-The-Air (OTA) firmware updates.
 */
void setupOTA() {
  server.on("/update", HTTP_GET, []() {
    server.send(200, "text/html", getOTAHTML());
  });

  server.on("/update", HTTP_POST, []() {
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK - RESTARTING");
    delay(500); ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      if (BOARD_ID == 1) { 
        ledcWrite(0, 0); ledcWrite(1, 0); ledcWrite(2, 0); ledcWrite(3, 0);
      }
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) Update.printError(Serial);
    } else if (upload.status == UPLOAD_FILE_END) {
      if (!Update.end(true)) Update.printError(Serial);
    }
  });
}

/*
 * Function: handleData
 * Processes telemetry data and returns it in JSON format for the web interface.
 */
void handleData() {
  float fSpd = cfg_imperial ? 0.621371 : 1.0; float fDst = cfg_imperial ? 0.621371 : 1.0;
  
  float mtB = maxTempBat; float mtE = maxTempEsc;
  float tB = myTelem.tBat; float tE = myTelem.tEsc;
  
  if(isnan(tB)) tB = 0.0; if(isnan(tE)) tE = 0.0;
  if(isnan(mtB)) mtB = 0.0; if(isnan(mtE)) mtE = 0.0;

  if(cfg_fahrenheit) {
    if(tB > -90) tB = (tB * 1.8) + 32; 
    if(tE > -90) tE = (tE * 1.8) + 32;
    if(mtB > -90) mtB = (mtB * 1.8) + 32;
    if(mtE > -90) mtE = (mtE * 1.8) + 32;
  }

  float curPwr = myTelem.voltage * myTelem.current;
  if(isPairingMode) curPwr = -999; 

  float estRange = (myTelem.batPct / 100.0) * profiles[activeProfile].estRangeKm * fDst;
  float estRuntime = (remBatPct / 100.0) * cfg_remRuntimeHrs;
  
  int thrPct = (myCmd.throttle > 0) ? map(myCmd.throttle, 0, 4095, 0, 100) : 0;
  int brkPct = myCmd.brakeBtn ? 100 : map(myCmd.brakeAnalog, 0, 4095, 0, 100);

  if(isnan(myTelem.voltage)) myTelem.voltage = 0.0;
  if(isnan(myTelem.current)) myTelem.current = 0.0;
  if(isnan(curPwr)) curPwr = 0.0;
  if(isnan(remVoltageSmooth)) remVoltageSmooth = 0.0;

  String json = "{";
  json += "\"prof\":" + String(activeProfile) + ",";
  json += "\"spd\":" + safeFloat(myTelem.speed * fSpd, 1) + ",";
  json += "\"bat\":" + String(myTelem.batPct) + ",";
  json += "\"vol\":" + safeFloat(myTelem.voltage, 1) + ",";
  json += "\"amp\":" + safeFloat(myTelem.current, 1) + ",";
  json += "\"pwr\":" + safeFloat(curPwr, 0) + ",";
  json += "\"wh\":" + safeFloat(myTelem.WhConsumed, 1) + ","; 
  json += "\"mah\":" + safeFloat(myTelem.mAhConsumed, 0) + ","; 
  json += "\"whChg\":" + safeFloat(myTelem.WhCharging, 1) + ","; 
  json += "\"mahChg\":" + safeFloat(myTelem.mAhCharging, 0) + ","; 
  
  json += "\"map\":" + safeFloat(maxAmpsSession, 1) + ",";
  json += "\"mpw\":" + safeFloat(maxPowerSession, 0) + ",";

  json += "\"trip\":" + safeFloat(myTelem.distTrip * fDst, 2) + ",";
  json += "\"tot\":" + safeFloat(myTelem.distTotal * fDst, 1) + ",";
  json += "\"ses\":" + safeFloat(myTelem.distSession * fDst, 3) + ",";
  json += "\"estR\":" + safeFloat(estRange, 1) + ",";
  
  json += "\"lat_val\":" + String(lastLatency) + ",";
  json += "\"fs_ms\":" + String(myCmd.cfgFailsafeMs) + ",";
  json += "\"lat_ok\":" + String(lastPacketStatus ? 1 : 0) + ",";

  json += "\"tb\":" + safeFloat(tB, 0) + ","; json += "\"te\":" + safeFloat(tE, 0) + ","; 
  json += "\"mtb\":" + safeFloat(mtB, 0) + ","; json += "\"mte\":" + safeFloat(mtE, 0) + ","; 
  
  json += "\"rbPct\":" + String(remBatPct) + ",";
  json += "\"rbVol\":" + safeFloat(remVoltageSmooth, 2) + ",";
  json += "\"rbRun\":" + safeFloat(estRuntime, 1) + ",";
  
  json += "\"thr\":" + String(thrPct) + ","; 
  json += "\"brk\":" + String(brkPct) + ","; 
  json += "\"pair\":" + String(isPairingMode ? 1 : 0) + ",";
  
  json += "\"u_s\":\"" + String(cfg_imperial ? "MPH" : "KM/H") + "\","; 
  json += "\"u_d\":\"" + String(cfg_imperial ? "mi" : "km") + "\","; 
  json += "\"u_t\":\"" + String(cfg_fahrenheit ? "&deg;F" : "&deg;C") + "\","; 
  json += "\"gm\":" + String(cfg_gaugeMax) + ",";
  json += "\"boardChg\":" + String(myTelem.isCharging ? 1 : 0) + ",";
  json += "\"remChg\":" + String(cfg_isCharging ? 1 : 0);
  json += "}";
  server.send(200, "application/json", json);
}

/*
 * Function: getCss
 * Generates the CSS styling for the web interface based on user preferences.
 */
String getCss() {
  String cBg   = cfg_darkMode ? "#121212" : "#f0f2f5";
  String cCard = cfg_darkMode ? "#1e1e1e" : "#ffffff";
  String cTxt  = cfg_darkMode ? "#e0e0e0" : "#333333";
  String cSub  = cfg_darkMode ? "#aaaaaa" : "#666666";
  String cBord = cfg_darkMode ? "#333333" : "#cccccc";
  String cInp  = cfg_darkMode ? "#2c2c2c" : "#e4e6eb";
  String cAcc  = cfg_darkMode ? "#00e5ff" : "#0091ad"; 

  return "<style>"
    "body{font-family:'Segoe UI',sans-serif;background:" + cBg + ";color:" + cTxt + ";text-align:center;margin:0;padding:10px;padding-bottom:80px;}"
    ".card{background:" + cCard + ";padding:15px;border-radius:12px;margin-bottom:15px;border:1px solid " + cBord + ";box-shadow:0 2px 5px rgba(0,0,0,0.05);}"
    "h2{color:" + cAcc + ";margin:0 0 15px 0;font-size:1.1rem;text-transform:uppercase;}"
    "select,input[type=number],input[type=text]{padding:8px;border-radius:5px;border:1px solid " + cBord + ";background:" + cInp + ";color:" + cTxt + ";width:80px;text-align:center;}"
    "input[type=range]{width:90%;accent-color:" + cAcc + ";display:block;margin:10px auto;}"
    "input[type=checkbox]{transform:scale(1.5);margin:0;cursor:pointer;accent-color:" + cAcc + ";}"
    ".set-row{display:flex;justify-content:space-between;align-items:center;margin-bottom:12px;border-bottom:1px solid " + cBord + ";padding-bottom:8px;}"
    ".set-lbl{text-align:left;font-size:0.9rem;flex-grow:1;}"
    ".btn{background:" + cAcc + ";color:#000;border:none;width:100%;font-size:16px;font-weight:bold;padding:12px;border-radius:8px;cursor:pointer;display:block;text-decoration:none;box-sizing:border-box;margin-top:10px;}"
    ".btn-danger{background:#d32f2f;color:white;border:none;width:auto;display:inline-block;padding:5px 15px;font-size:12px;border-radius:5px;}"
    ".fab{position:fixed;bottom:20px;right:20px;width:50px;height:50px;border-radius:50%;background:#333;color:#fff;display:flex;align-items:center;justify-content:center;box-shadow:0 4px 10px rgba(0,0,0,0.3);border:none;z-index:100;text-decoration:none;}"
    ".fab svg{fill:#fff;width:24px;height:24px;}"
    ".fab-save{position:fixed;bottom:20px;right:20px;width:60px;height:60px;border-radius:50%;background:#00e676;color:#000;display:flex;align-items:center;justify-content:center;box-shadow:0 4px 10px rgba(0,230,118,0.4);border:none;z-index:100;font-size:24px;cursor:pointer;}"
    ".btn-group{display:flex;gap:5px;margin-top:0px;margin-bottom:15px;}"
    ".btn-prof{flex:1;padding:15px 5px;border:none;border-radius:8px;cursor:pointer;font-weight:bold;color:#fff;font-size:14px;opacity:0.5;transition:0.2s;}"
    ".btn-prof.active{opacity:1.0;transform:scale(1.05);box-shadow:0 0 10px rgba(0,0,0,0.2);border:2px solid " + cTxt + ";}"
    ".p-eco{background:#2e7d32;}.p-norm{background:#1565c0;}.p-max{background:#c62828;}"
    "canvas{background:transparent;width:100%;max-width:350px;}"
    ".dash-val{font-size:1.5rem;font-weight:bold;color:" + cTxt + ";}"
    ".speed-big{font-size:4.5rem;font-weight:800;color:" + cTxt + ";line-height:1;margin-top:-20px;display:block;text-shadow:0 0 10px " + cAcc + "4d;}"
    ".dash-lbl{font-size:0.7rem;color:" + cSub + ";text-transform:uppercase;letter-spacing:1px;}"
    ".flex-row{display:flex;justify-content:space-around;align-items:center;flex-wrap:wrap;}"
    ".info-box{font-size:0.9rem;color:" + cSub + ";background:" + cInp + ";padding:10px;border-radius:8px;margin:5px;text-align:center;box-shadow:0 2px 5px rgba(0,0,0,0.05);border:1px solid " + cBord + ";flex:1;}"
    ".bat-shell{border:2px solid " + cSub + ";width:30px;height:14px;display:inline-block;position:relative;border-radius:3px;margin-left:8px;vertical-align:middle;}"
    ".bat-nub{position:absolute;right:-5px;top:3px;width:3px;height:8px;background:" + cSub + ";border-radius:0 2px 2px 0;}"
    ".bat-fill{height:100%;width:50%;background:#00e676;transition:width 0.5s,background-color 0.5s;border-radius:1px;}"
    ".thr-shell {background:" + cInp + "; height:12px; border-radius:6px; margin:5px 25px; overflow:hidden; border:1px solid " + cBord + ";}"
    ".thr-fill {height:100%; width:0%; background:" + cAcc + "; transition:width 0.1s ease-out;}"
    ".brk-fill {height:100%; width:0%; background:#ff3d00; transition:width 0.1s ease-out;}"
    ".unit-small{font-size:0.8rem;color:" + cSub + ";font-weight:normal;margin-left:2px;}"
    ".max-lbl{font-size:0.75rem;color:#ff9800;display:block;margin-top:2px;}"
    ".rem-box{border:1px solid " + cBord + ";border-radius:8px;padding:10px;margin-top:15px;font-size:0.9rem;}"
    "table{width:100%;border-collapse:collapse;margin-top:10px;font-size:0.8rem;}"
    "th,td{border:1px solid " + cBord + ";padding:5px;text-align:center;}"
    "th{background:" + cInp + ";}"
    ".brk-grp{padding:5px; margin:10px 0;}" 
    "</style>";
}

/*
 * Function: handleRoot
 * Serves the main dashboard HTML page to the user.
 */
void handleRoot() {
  String unitS = cfg_imperial ? "MPH" : "KM/H";
  String unitD = cfg_imperial ? "mi" : "km";
  String unitT = cfg_fahrenheit ? "&deg;F" : "&deg;C";
  String cAcc = cfg_darkMode ? "#00e5ff" : "#0091ad"; 

  String h = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  h += "<title>Longboard System</title>" + getCss() + "</head><body>";
  
  h += "<div class='card' style='padding:10px; margin-bottom:10px'>";
  h += "<div class='btn-group'>";
  h += "<button type='button' onclick='setProfile(0)' id='prof0' class='btn-prof p-eco'>ECO</button>";
  h += "<button type='button' onclick='setProfile(1)' id='prof1' class='btn-prof p-norm'>NORMAL</button>";
  h += "<button type='button' onclick='setProfile(2)' id='prof2' class='btn-prof p-max'>MAX</button>";
  h += "</div></div>";

  h += "<div class='card' style='border-top: 3px solid " + cAcc + ";'>";
  h += "<canvas id='gauge' width='350' height='150'></canvas>";
  h += "<span id='d_spd' class='speed-big'>0.0</span><span class='dash-lbl' id='u_spd'>" + unitS + "</span>";
  
  h += "<div class='thr-shell'><div id='thr_fill' class='thr-fill'></div></div>";
  h += "<div class='thr-shell'><div id='brk_fill' class='brk-fill'></div></div>"; 
  
  h += "<hr style='border: 0; border-top: 1px solid #444; margin:15px 0;'>"; 
  
  h += "<div class='flex-row' style='margin-top:15px'>";
  h += "  <div class='info-box'>"; 
  h += "    <div style='font-size:0.9rem; font-weight:bold; color:" + cAcc + "; margin-bottom:-2px'><span id='d_estR'>--</span> " + unitD + "</div>";
  h += "    <div class='dash-val' style='display:flex; align-items:center; justify-content:center; margin-top:2px'>";
  h += "      <span id='d_bat'>0</span><span class='dash-val' style='font-size:1rem'>%</span>";
  h += "      <div class='bat-shell'><div id='bat_fill' class='bat-fill'></div><div class='bat-nub'></div></div><span id='board_chg_icon' style='display:none; color:#ffeb3b'>&#9889;</span>";
  h += "    </div>";
  h += "    <div class='dash-lbl'><span id='d_vol'>0.0</span> V</div>";
  h += "  </div>";
  
  h += "  <div class='info-box'>"; 
  h += "    <div><div class='dash-val' style='color:#ffeb3b'><span id='d_pwr'>0</span> W</div><div class='dash-lbl'><span id='d_amp'>0.0</span> A</div></div>";
  h += "    <div style='font-size:0.75rem; color:#ff9800; display:block; margin-top:5px;'>MAX: <span id='d_mpw' style='font-weight:bold'>--</span> W / <span id='d_map' style='font-weight:bold'>--</span> A</div>";
  h += "  </div>";
  h += "</div>";

  h += "<div class='flex-row' style='margin-top:15px'>";
  h += "  <div class='info-box'>BATTERY<br><span id='t_bat' style='font-weight:bold; font-size:1.2rem'>--</span> <span class='unit-t'>" + unitT + "</span><br><span class='max-lbl'>MAX: <span id='mt_bat'>--</span> <span class='unit-t'>" + unitT + "</span></span></div>";
  h += "  <div class='info-box'>CONTROLLER<br><span id='t_esc' style='font-weight:bold; font-size:1.2rem'>--</span> <span class='unit-t'>" + unitT + "</span><br><span class='max-lbl'>MAX: <span id='mt_esc'>--</span> <span class='unit-t'>" + unitT + "</span></span></div>";
  h += "</div>"; 
  h += "<div class='flex-row' style='margin-top:15px'>";
  h += "  <div class='info-box'>"; 
  h += "    <span style='text-transform:uppercase;'>CONSUMED</span>"; 
  h += "    <div style='display:flex; justify-content:center; align-items:baseline;'>";
  h += "      <span id='d_wh' class='dash-val' style='font-size:1.2rem'>0.0</span><span class='unit-small'> Wh</span>";
  h += "      <span style='margin: 0 5px;'>|</span>";
  h += "      <span id='d_mah' class='dash-val' style='font-size:1.2rem'>0</span><span class='unit-small'> mAh</span>";
  h += "    </div>";
  h += "    <form action='/save' method='GET' onsubmit=\"return confirm('Reset Consumed Wh/mAh?');\" style='margin-top:5px;'>";
  h += "      <input type='hidden' name='src' value='dash'>"; 
  h += "      <button type='submit' name='rstConsumed' value='1' class='btn btn-danger' style='padding:5px 10px; font-size:0.7rem; width:auto;'>RESET</button>";
  h += "    </form>";
  h += "  </div>"; 
  
  h += "  <div class='info-box'>"; 
  h += "    <span style='text-transform:uppercase;'>CHARGING</span>"; 
  h += "    <div style='display:flex; justify-content:center; align-items:baseline;'>";
  h += "      <span id='d_whChg' class='dash-val' style='font-size:1.2rem'>0.0</span><span class='unit-small'> Wh</span>";
  h += "      <span style='margin: 0 5px;'>|</span>";
  h += "      <span id='d_mahChg' class='dash-val' style='font-size:1.2rem'>0</span><span class='unit-small'> mAh</span>";
  h += "    </div>";
  h += "    <form action='/save' method='GET' onsubmit=\"return confirm('Reset Charging Wh/mAh?');\" style='margin-top:5px;'>";
  h += "      <input type='hidden' name='src' value='dash'>"; 
  h += "      <button type='submit' name='rstCharging' value='1' class='btn btn-danger' style='padding:5px 10px; font-size:0.7rem; width:auto;'>RESET</button>";
  h += "    </form>";
  h += "  </div>"; 
  h += "</div>"; 
  h += "<hr style='border: 0; border-top: 1px solid #444; margin:15px 0;'>"; 
  
  h += "<div class='flex-row' style='margin-top:15px'>"; 
  h += "  <div class='info-box'>"; 
  h += "    <span id='d_ses' class='dash-val' style='font-size:1.1rem'>0.00</span><span class='unit-small unit-d'>" + unitD + "</span><br><span class='dash-lbl'>SESSION</span>";
  h += "  </div>";
  h += "  <div class='info-box'>"; 
  h += "    <span id='d_trip' class='dash-val' style='font-size:1.2rem'>0.00</span><span class='unit-small unit-d'>" + unitD + "</span><br><span class='dash-lbl'>TRIP</span>";
  h += "    <form action='/save' method='GET' onsubmit=\"return confirm('Reset TRIP counter?');\" style='margin-top:5px;'>"; 
  h += "      <input type='hidden' name='src' value='dash'>"; 
  h += "      <button type='submit' name='rstTrip' value='1' class='btn btn-danger' style='padding:5px 10px; font-size:0.7rem; margin:0;'>RESET</button>";
  h += "    </form>";
  h += "  </div>";
  h += "  <div class='info-box'>"; 
  h += "    <span id='d_tot' class='dash-val' style='font-size:1.1rem'>0.0</span><span class='unit-small unit-d'>" + unitD + "</span><br><span class='dash-lbl'>TOTAL</span>";
  h += "  </div>";
  h += "</div>"; 


  h += "<div class='rem-box'><div style='margin-bottom:5px; opacity:0.7; font-weight:bold'>REMOTE</div>"; 
  h += "<div class='flex-row' style='font-size:0.9rem; align-items:center'>";
  h += "<div><div class='bat-shell'><div id='rb_fill' class='bat-fill'></div><div class='bat-nub'></div></div><span id='rem_chg_icon' style='display:none; color:#ffeb3b'>&#9889;</span> <span id='rb_pct' style='font-weight:bold'>--</span>%</div>";
  h += "<div>Ping: <span id='d_lat' style='font-weight:bold'>--</span></div>";
  h += "<div>Left: <span id='rb_run'>--</span>h</div>";
  h += "</div></div>";

  h += "</div>"; 

  h += "<a href='/settings' class='fab'><svg viewBox='0 0 24 24'><path d='M19.14 12.94c.04-.3.06-.61.06-.94 0-.32-.02-.64-.07-.94l2.03-1.58a.49.49 0 0 0 .12-.61l-1.92-3.32a.488.488 0 0 0-.59-.22l-2.39.96c-.5-.38-1.03-.7-1.62-.94l-.36-2.54a.484.484 0 0 0-.48-.41h-3.84c-.24 0-.43.17-.47.41l-.36 2.54c-.59.24-1.13.57-1.62.94l-2.39-.96c-.22-.08-.47 0-.59.22L2.74 8.87c-.12.21-.08.47.12.61l2.03 1.58c-.05.3-.09.63-.09.94s.02.64.07.94l-2.03 1.58a.49.49 0 0 0-.12.61l1.92 3.32c.12.22.37.29.59.22l2.39-.96c.5.38 1.03.7 1.62.94l.36 2.54c.05.24.24.41.48.41h3.84c.24 0 .44-.17.47-.41l.36-2.54c.59-.24 1.13-.58 1.62-.94l2.39.96c.22.08.47 0 .59-.22l1.92-3.32c.12-.22.07-.47-.12-.61l-2.01-1.58zM12 15.6c-1.98 0-3.6-1.62-3.6-3.6s1.62-3.6 3.6-3.6 3.6 1.62 3.6 3.6-1.62 3.6-3.6 3.6z'/></svg></a>";

  h += R"raw(
  <script>
    var cAcc = '" + cAcc + "';
    function setProfile(profId) { fetch('/save?src=dash&prof=' + profId).then(update); }
    function drawGauge(speed, maxSpeed) {
      var c = document.getElementById('gauge'); var ctx = c.getContext('2d');
      var w = c.width; var h = c.height; 
      ctx.clearRect(0, 0, w, h);
      ctx.beginPath(); ctx.arc(w/2, h-15, 110, Math.PI, 0);
      ctx.lineWidth=15; ctx.strokeStyle='#444'; ctx.stroke();
      var ratio = Math.min(speed, maxSpeed) / maxSpeed;
      var angle = Math.PI + (Math.PI * ratio);
      ctx.beginPath(); ctx.arc(w/2, h-15, 110, Math.PI, angle);
      ctx.lineWidth=15; ctx.strokeStyle = speed > (maxSpeed*0.75) ? '#ff3d00' : cAcc; ctx.stroke();
    }
    
    function update() {
      fetch('/data').then(r => r.json()).then(d => {
        document.getElementById('d_spd').innerText = d.spd;
        document.getElementById('u_spd').innerText = d.u_s;
        
        var uD = document.querySelectorAll('.unit-d'); uD.forEach(el => el.innerText = d.u_d);
        var uT = document.querySelectorAll('.unit-t'); uT.forEach(el => el.innerHTML = d.u_t);

        document.getElementById('d_bat').innerText = d.bat;
        document.getElementById('d_vol').innerText = d.vol;
        document.getElementById('d_pwr').innerText = d.pwr;
        document.getElementById('d_amp').innerText = d.amp;
        document.getElementById('d_estR').innerText = d.estR;
        
        document.getElementById('d_map').innerText = d.map;
        document.getElementById('d_mpw').innerText = d.mpw;
        
        document.getElementById('d_trip').innerText = d.trip;
        document.getElementById('d_tot').innerText = d.tot;
        document.getElementById('d_ses').innerText = d.ses;
        
        document.getElementById('d_wh').innerText = d.wh;
        document.getElementById('d_mah').innerText = d.mah;
        document.getElementById('d_whChg').innerText = Math.abs(d.whChg); 
        document.getElementById('d_mahChg').innerText = Math.abs(d.mahChg); 

        document.getElementById('t_bat').innerText = d.tb;
        document.getElementById('mt_bat').innerText = d.mtb;
        document.getElementById('t_esc').innerText = d.te;
        document.getElementById('mt_esc').innerText = d.mte;
        
        document.getElementById('rb_pct').innerText = d.rbPct;
        document.getElementById('rb_run').innerText = d.rbRun;
        const latElement = document.getElementById('d_lat');
        if (d.lat_ok === 0) { 
          latElement.innerText = "NO SIGNAL";
          latElement.style.color = "#d32f2f"; 
        } else if (d.lat_val > d.fs_ms) { 
          latElement.innerHTML = "HIGH PING<br>(" + d.lat_val + " ms)"; 
          latElement.style.color = "#ff9800"; 
        } else {
          latElement.innerText = d.lat_val + " ms"; 
          latElement.style.color = "#00e676"; 
        }
        
        var b = parseInt(d.bat);
        var col = b > 50 ? '#00e676' : (b > 20 ? '#ffea00' : '#ff3d00');
        var fill = document.getElementById('bat_fill');
        if(fill) { fill.style.width = b + '%'; fill.style.backgroundColor = col; }
        document.getElementById('board_chg_icon').style.display = d.boardChg ? 'inline' : 'none';
        
        var rb = parseInt(d.rbPct);
        var colRb = rb > 50 ? '#00e676' : (rb > 20 ? '#ffea00' : '#ff3d00');
        var fillRb = document.getElementById('rb_fill');
        if(fillRb) { fillRb.style.width = rb + '%'; fillRb.style.backgroundColor = colRb; }
        document.getElementById('rem_chg_icon').style.display = d.remChg ? 'inline' : 'none';
        
        var tp = parseInt(d.thr);
        document.getElementById('thr_fill').style.width = tp + '%';
        
        var bp = parseInt(d.brk);
        document.getElementById('brk_fill').style.width = bp + '%';
        
        if (d.pair == 1) {
            document.title = "PAIRING...";
        }

        for (let i = 0; i < 3; i++) {
          const btn = document.getElementById('prof' + i);
          if (i === d.prof) { btn.classList.add('active'); }
          else { btn.classList.remove('active'); }
        }

        drawGauge(parseFloat(d.spd), parseFloat(d.gm));
      });
    }
    window.onload = function() { update(); setInterval(update, 500); }; 
  </script></body></html>
  )raw";

  server.send(200, "text/html", h);
}

/*
 * Function: getBrakeRow
 * Helper function for generating HTML table rows for braking settings.
 */
String getBrakeRow(String prefix, uint8_t mask) {
    String s = "";
    for(int i=0; i<4; i++) {
        String chk = ((mask >> i) & 1) ? "checked" : "";
        s += "<td><input type='checkbox' name='" + prefix + String(i) + "' " + chk + "></td>";
    }
    return s;
}

/*
 * Function: handleSettings
 * Serves the settings HTML page, allowing users to configure profiles and hardware.
 */
void handleSettings() {
  loadRemoteSettings(); 
  DriveProfile *p = &profiles[activeProfile];
  
  float tB = myTelem.tBat; 
  if(cfg_fahrenheit && tB > -90) tB = (tB * 1.8) + 32;
  float tE = myTelem.tEsc;
  if(cfg_fahrenheit && tE > -90) tE = (tE * 1.8) + 32;
  String unitT = cfg_fahrenheit ? "F" : "C";

  String h = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  h += "<title>Settings</title>" + getCss() + "</head><body>";
  
  h += "<div class='card'><h2>Pairing</h2>";
  if(isPairingMode) h += "<div style='color:#00e676;font-weight:bold;animation:blink 1s infinite'>SEARCHING FOR BOARD...</div>";
  else h += "<form action='/save' method='GET'><button type='submit' name='pairBtn' value='1' class='btn'>PAIR NEW BOARD</button></form>";
  h += "</div>";

  h += "<div class='card'><h2>Select Profile</h2>";
  h += "<form action='/save' method='GET'>";
  h += "<input type='hidden' name='src' value='settings'>";
  h += "<div class='btn-group'>";
  h += "<button type='submit' name='prof' value='0' class='btn-prof p-eco " + String(activeProfile==0?"active":"") + "'>ECO</button>";
  h += "<button type='submit' name='prof' value='1' class='btn-prof p-norm " + String(activeProfile==1?"active":"") + "'>NORMAL</button>";
  h += "<button type='submit' name='prof' value='2' class='btn-prof p-max " + String(activeProfile==2?"active":"") + "'>MAX</button>";
  h += "</div></form></div>";

  h += "<form action='/save' method='GET'><input type='hidden' name='src' value='settings'>";

  h += "<div class='set-row'><div class='set-lbl'>Max Power (%)</div><input type='number' name='power' value='" + String(p->maxPower) + "'></div>";
  String speedUnitText = (cfg_imperial ? "MPH" : "KM/H");
  h += "<div class='set-row'><div class='set-lbl'>Speed Limit (" + speedUnitText + ")</div><input type='number' step='0.1' name='speedLim' value='" + String(p->speedLimit) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>Enable Speed Limit</div><input type='checkbox' name='enSpeedLim' " + String(p->enableSpeedLimit?"checked":"") + "></div>";
  
  h += "Throttle Curve: <span id='valC'>" + String(p->curve) + "</span><br>";
  h += "<canvas id='cv_curve' width='300' height='100'></canvas>";
  h += "<input type='range' name='curve' id='rngC' min='1.0' max='5.0' step='0.1' value='" + String(p->curve) + "' oninput='drawC()'><br>";
  
  h += "<div class='brk-grp'><b>Braking Configuration</b><br>";
  h += "Brake Curve: <span id='valBC'>" + String(cfg_brakeCurve) + "</span><br>";
  h += "<canvas id='bc_curve' width='300' height='100'></canvas>";
  h += "<input type='range' name='bcurve' id='rngBC' min='1.0' max='5.0' step='0.1' value='" + String(cfg_brakeCurve) + "' oninput='drawBC()'><br>";
  
  h += "<table><tr><th>Level</th><th>M1</th><th>M2</th><th>M3</th><th>M4</th></tr>";
  h += "<tr><td>Button</td>" + getBrakeRow("bbm", cfg_buttonBrakeMask) + "</tr>";
  h += "<tr><td>0-20%</td>" + getBrakeRow("bam0", cfg_analogBrakeMask[0]) + "</tr>";
  h += "<tr><td>20-40%</td>" + getBrakeRow("bam1", cfg_analogBrakeMask[1]) + "</tr>";
  h += "<tr><td>40-60%</td>" + getBrakeRow("bam2", cfg_analogBrakeMask[2]) + "</tr>";
  h += "<tr><td>60-80%</td>" + getBrakeRow("bam3", cfg_analogBrakeMask[3]) + "</tr>";
  h += "<tr><td>80-100%</td>" + getBrakeRow("bam4", cfg_analogBrakeMask[4]) + "</tr>";
  h += "</table></div>";

  h += "<b>Motor Power (%)</b><br>FL:<input type='number' name='m1' value='" + String(p->m1) + "'> FR:<input type='number' name='m2' value='" + String(p->m2) + "'><br>";
  h += "RL:<input type='number' name='m3' value='" + String(p->m3) + "'> RR:<input type='number' name='m4' value='" + String(p->m4) + "'></div>";

  h += "<div class='card' style='border-top: 3px solid #ffeb3b'><h2>Hardware & Calibration</h2>";
  h += "<div class='set-row'><div class='set-lbl'><b>Wheel Diameters (cm):</b></div></div>";
  h += "<div style='display:flex; justify-content:space-between; margin-bottom:5px'><span>FL: <input type='number' step='0.1' name='wFL' value='" + String(cfg_whFL) + "'></span><span>FR: <input type='number' step='0.1' name='wFR' value='" + String(cfg_whFR) + "'></span></div>";
  h += "<div style='display:flex; justify-content:space-between; margin-bottom:10px'><span>RL: <input type='number' step='0.1' name='wRL' value='" + String(cfg_whFL) + "'></span><span>RR: <input type='number' step='0.1' name='wRR' value='" + String(cfg_whRR) + "'></span></div>";
  h += "<div class='set-row'><div class='set-lbl'>Pulses per Revolution</div><input type='number' name='ppr' value='" + String(cfg_pulsesPerRev) + "'></div>"; 
  h += "<div class='set-row'><div class='set-lbl'>Visualisation max speed</div><input type='number' name='gMax' value='" + String(cfg_gaugeMax) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>Speed Units</div><select name='imp'><option value='0' " + String(!cfg_imperial?"selected":"") + ">Metric</option><option value='1' " + String(cfg_imperial?"selected":"") + ">Imperial</option></select></div>";
  h += "<div class='set-row'><div class='set-lbl'>Temp Units</div><select name='fahr'><option value='0' " + String(!cfg_fahrenheit?"selected":"") + ">Celsius</option><option value='1' " + String(cfg_fahrenheit?"selected":"") + ">Fahrenheit</option></select></div>";
  h += "<br><b>PID Control (Speed):</b><br>";
  h += "<div class='set-row'><div class='set-lbl'>Enable PID</div><input type='checkbox' name='pid' " + String(cfg_pidEn?"checked":"") + "></div>";
  h += "<div class='set-row'><div class='set-lbl'>Kp</div><input type='number' step='0.1' name='kp' value='" + String(cfg_Kp) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>Ki</div><input type='number' step='0.01' name='ki' value='" + String(cfg_Ki) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>Kd</div><input type='number' step='0.1' name='kd' value='" + String(cfg_Kd) + "'></div>";
  h += "<br><b>Longboard Battery:</b><br>";
  h += "<div class='set-row'><div class='set-lbl'>Cycles</div><span style='font-weight:bold'>" + String(myTelem.batCycles) + "</span></div>";
  h += "<div class='set-row'><div class='set-lbl'>Scale (Calibration) [Read: " + String(myTelem.voltage,1) + "V]</div><input type='number' step='0.1' name='vScl' value='" + String(cfg_voltsScale) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>Series cells</div><input type='number' name='sCells' value='" + String(cfg_seriesCells) + "'></div>"; 
  h += "<div class='set-row'><div class='set-lbl'>Auto Trip Reset</div><input type='checkbox' name='aRes' " + String(cfg_autoReset?"checked":"") + "></div>";
  
  h += "<br><b>Remote Battery:</b><br>";
  h += "<div class='set-row'><div class='set-lbl'>Cycles</div><span style='font-weight:bold'>" + String(remBatCycles) + "</span></div>";
  h += "<div class='set-row'><div class='set-lbl'>Scale (Calibration)</div><input type='number' step='0.01' name='rbScl' value='" + String(cfg_remBatScale) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>Current Read</div><span style='font-weight:bold'>" + String(remVoltageSmooth, 2) + " V</span></div>";
  h += "<div class='set-row'><div class='set-lbl'>Runtime @ 100% (h)</div><input type='number' step='0.1' name='rbRun' value='" + String(cfg_remRuntimeHrs) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>Series cells</div><input type='number' name='rbCells' value='" + String(cfg_remSeriesCells) + "'></div>"; 

  h += "<br><b>Battery Curves (Cell V for 0,10..100%):</b><br>";
  char buf[100];
  arrayToString(cfg_curveBat, buf);
  h += "<div style='margin-bottom:5px'>Longboard:<br><input type='text' name='cB' value='" + String(buf) + "' style='width:95%'></div>";
  arrayToString(cfg_curveRem, buf);
  h += "<div>Remote:<br><input type='text' name='cR' value='" + String(buf) + "' style='width:95%'></div>";

  h += "<br><b>Sensors:</b><br>";
  h += "<div class='set-row'><div class='set-lbl'>INA219 Scale</div><input type='number' step='0.01' name='inaScl' value='" + String(cfg_amps_scale_ina219) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>INA219 Offset (A)</div><input type='number' step='0.01' name='inaOff' value='" + String(cfg_amps_offset_ina219) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>Battery NTC Beta [" + String(tB, 0) + "&deg;" + unitT + "]</div><input type='number' name='bB' value='" + String(cfg_betaBat,0) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>ESC NTC Beta [" + String(tE, 0) + "&deg;" + unitT + "]</div><input type='number' name='bE' value='" + String(cfg_betaEsc,0) + "'></div>";
  h += "</div>";

  h += "<div class='card' style='border-top: 3px solid #ff3d00'><h2>Safety & System</h2>";
  h += "<div class='set-row'><div class='set-lbl'>Throttle Min Deadzone</div><input type='number' name='dead' value='" + String(cfg_deadzone) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>Throttle Max Threshold</div><input type='number' name='dMax' value='" + String(cfg_deadzoneMax) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>Brake Min Deadzone</div><input type='number' name='bDead' value='" + String(cfg_brakeDeadzone) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>Brake Max Threshold</div><input type='number' name='bDMax' value='" + String(cfg_brakeDeadzoneMax) + "'></div>";
  
  h += "<div class='set-row'><div class='set-lbl'>Connection Lost Timeout (ms)</div><input type='number' name='fsMs' value='" + String(cfg_fsMs) + "'></div>";
  
  h += "<div class='set-row'><div class='set-lbl'>Connection Lost Brake:</div></div>";
  h += "<table><tr><th>M1</th><th>M2</th><th>M3</th><th>M4</th></tr>";
  h += "<tr>" + getBrakeRow("fsMask", cfg_fsBrakeMask) + "</tr></table>";
  
  h += "<div class='set-row'><div class='set-lbl'>Blink on Lost</div><input type='checkbox' name='fsLgt' " + String(cfg_fsLight?"checked":"") + "></div>";
  h += "<div class='set-row'><div class='set-lbl'>Button Debounce (ms)</div><input type='number' name='dbnc' value='" + String(cfg_debounceMs) + "'></div>";
  h += "<div class='set-row'><div class='set-lbl'>Auto Off (min)</div><div style='display:flex;align-items:center'><input type='number' name='aoMin' value='" + String(cfg_autoOffMins) + "'><input type='checkbox' name='aoEn' " + String(cfg_autoOffEn?"checked":"") + "></div></div>";
  h += "<div class='set-row'><div class='set-lbl'>Disable PWM @ 0% Bat</div><input type='checkbox' name='disPwm' " + String(cfg_disablePwmOnLowBat?"checked":"") + "></div>"; 
  h += "<button type='submit' name='rstStat' value='1' class='btn btn-danger'>RESET SESSION STATS</button>"; 
  h += "</div>";

  h += "<div class='card' style='border-top: 3px solid #9c27b0'><h2>Outputs</h2>";
  h += "<div class='set-row'><div class='set-lbl'>Invert Brake 1</div><input type='checkbox' name='iB1' " + String(cfg_inv_brk1?"checked":"") + "></div>";
  h += "<div class='set-row'><div class='set-lbl'>Invert Brake 2</div><input type='checkbox' name='iB2' " + String(cfg_inv_brk2?"checked":"") + "></div>";
  h += "<div class='set-row'><div class='set-lbl'>Invert Brake 3</div><input type='checkbox' name='iB3' " + String(cfg_inv_brk3?"checked":"") + "></div>";
  h += "<div class='set-row'><div class='set-lbl'>Invert Brake 4</div><input type='checkbox' name='iB4' " + String(cfg_inv_brk4?"checked":"") + "></div>";
  h += "<div class='set-row'><div class='set-lbl'>Invert Direction</div><input type='checkbox' name='iDir' " + String(cfg_inv_dir?"checked":"") + "></div>";
  h += "<div class='set-row'><div class='set-lbl'>Invert Light</div><input type='checkbox' name='iLgt' " + String(cfg_inv_light?"checked":"") + "></div>";
  h += "<div class='set-row'><div class='set-lbl'>Invert PWM (100-0%)</div><input type='checkbox' name='iPwm' " + String(cfg_inv_pwm?"checked":"") + "></div>";
  h += "</div>";

  h += "<div class='card' style='padding:10px'>";
  h += "<div class='set-row' style='margin-bottom:0'><div class='set-lbl' style='font-weight:bold'>Dark Mode</div><input type='checkbox' name='dark' " + String(cfg_darkMode?"checked":"") + ">";
  h += "</div>";

  h += "<div class='card' style='text-align:left; font-size:0.8rem; color:#888'>";
  h += "<b>HARDWARE INFO:</b><br>";
  h += "Remote controller MAC: " + macToString(myMacAddr) + "<br>";
  h += "Longboard ESC MAC: " + macToString(targetMac);
  h += "</div>";

  h += "<br><div style='text-align:center; color:#555'>Firmware: " + String(FW_VERSION) + "</div>";
  h += "<button type='button' onclick=\"location.href='/update'\" class='btn' style='margin-top:5px'>UPDATE FIRMWARE</button>";

  h += "<br><br><button type='submit' class='fab-save'>&#128190;</button>";
  h += "<a href='/' class='btn btn-outline'>BACK TO DASHBOARD</a>";
  h += "</form>";

  h += R"raw(
  <script>
    function drawCurve(id, rangeId, col) {
      var ctx = document.getElementById(id).getContext('2d');
      var exp = document.getElementById(rangeId).value;
      if(id === 'cv_curve') document.getElementById('valC').innerText = exp;
      if(id === 'bc_curve') document.getElementById('valBC').innerText = exp;
      
      ctx.clearRect(0,0,300,100);
      ctx.strokeStyle='#444'; ctx.beginPath(); ctx.moveTo(0,100); ctx.lineTo(300,0); ctx.stroke();
      ctx.strokeStyle=col; ctx.lineWidth=3; ctx.beginPath(); ctx.moveTo(0,100);
      for(var x=0; x<=300; x+=5){ var nY = Math.pow(x/300.0, exp); ctx.lineTo(x, 100-(nY*100)); }
      ctx.stroke();
    }
    function drawC() { drawCurve('cv_curve', 'rngC', '#00e5ff'); }
    function drawBC() { drawCurve('bc_curve', 'rngBC', '#ff3d00'); }
    window.onload = function() { drawC(); drawBC(); };
  </script></body></html>
  )raw";
  
  server.send(200, "text/html", h);
}

/*
 * Function: handleSave
 * Processes and saves the configuration settings submitted from the web interface.
 */
void handleSave() {
  String src = server.arg("src"); 
  
  if (server.hasArg("pairBtn")) { 
      startPairing(); 
      server.sendHeader("Location", "/settings"); 
      server.send(303); 
      return; 
  }

  if (server.hasArg("rstTrip")) reqResetTrip = true; 
  if (server.hasArg("rstConsumed")) myCmd.cmdResetConsumed = true; 
  if (server.hasArg("rstCharging")) myCmd.cmdResetCharging = true;
  if (server.hasArg("rstStat")) { 
      maxAmpsSession = 0; maxPowerSession = 0;
      maxTempBat = -100; maxTempEsc = -100;
  }

  if (server.hasArg("prof")) activeProfile = server.arg("prof").toInt();

  if (server.hasArg("power")) profiles[activeProfile].maxPower = server.arg("power").toInt();
  if (server.hasArg("speedLim")) profiles[activeProfile].speedLimit = server.arg("speedLim").toFloat();
  profiles[activeProfile].enableSpeedLimit = server.hasArg("enSpeedLim");
  if (server.hasArg("curve")) profiles[activeProfile].curve = server.arg("curve").toFloat();
  if (server.hasArg("bcurve")) { 
    cfg_brakeCurve = server.arg("bcurve").toFloat();
  }
  
  if (server.hasArg("m1")) profiles[activeProfile].m1 = server.arg("m1").toInt();
  if (server.hasArg("m2")) profiles[activeProfile].m2 = server.arg("m2").toInt();
  if (server.hasArg("m3")) profiles[activeProfile].m3 = server.arg("m3").toInt();
  if (server.hasArg("m4")) profiles[activeProfile].m4 = server.arg("m4").toInt();
  if (server.hasArg("estR")) profiles[activeProfile].estRangeKm = server.arg("estR").toFloat();
  
  bool brakingMasksSubmitted = false;
  for(int m=0; m<4; m++) {
      if(server.hasArg("bbm" + String(m)) || server.hasArg("bam0" + String(m))) { 
          brakingMasksSubmitted = true;
          break;
      }
  }

  if (brakingMasksSubmitted) {
    cfg_buttonBrakeMask = 0;
    for(int j=0; j<5; j++) cfg_analogBrakeMask[j] = 0;

    for(int m=0; m<4; m++) {
        if(server.hasArg("bbm" + String(m))) cfg_buttonBrakeMask |= (1 << m);
        for(int level=0; level<5; level++) {
            if(server.hasArg("bam" + String(level) + String(m))) {
                cfg_analogBrakeMask[level] |= (1 << m);
            }
        }
    }
  }
  
  cfg_fsBrakeMask = 0;
  for(int m=0; m<4; m++) {
      if(server.hasArg("fsMask" + String(m))) cfg_fsBrakeMask |= (1 << m);
  }

  if (server.hasArg("wFL")) cfg_whFL = server.arg("wFL").toFloat();
  if (server.hasArg("wFR")) cfg_whFR = server.arg("wFR").toFloat();
  if (server.hasArg("wRL")) cfg_whRL = server.arg("wRL").toFloat();
  if (server.hasArg("wRR")) cfg_whRR = server.arg("wRR").toFloat();

  if (server.hasArg("ppr")) cfg_pulsesPerRev = server.arg("ppr").toInt(); 

  if (server.hasArg("inaScl")) cfg_amps_scale_ina219 = server.arg("inaScl").toFloat();
  if (server.hasArg("inaOff")) cfg_amps_offset_ina219 = server.arg("inaOff").toFloat();
  if (server.hasArg("vScl")) cfg_voltsScale = server.arg("vScl").toFloat();
  if (server.hasArg("sCells")) cfg_seriesCells = server.arg("sCells").toInt(); 

  if (server.hasArg("rbScl")) cfg_remBatScale = server.arg("rbScl").toFloat();
  if (server.hasArg("rbRun")) cfg_remRuntimeHrs = server.arg("rbRun").toFloat();
  if (server.hasArg("rbCells")) cfg_remSeriesCells = server.arg("rbCells").toInt(); 
  if (server.hasArg("rbCyc")) remBatCycles = server.arg("rbCyc").toInt(); 

  if (server.hasArg("pid") || src == "settings") cfg_pidEn = server.hasArg("pid");
  if (server.hasArg("kp")) cfg_Kp = server.arg("kp").toFloat();
  if (server.hasArg("ki")) cfg_Ki = server.arg("ki").toFloat();
  if (server.hasArg("kd")) cfg_Kd = server.arg("kd").toFloat();

  if (server.hasArg("aRes") || src == "settings") cfg_autoReset = server.hasArg("aRes");
  if (server.hasArg("dbnc")) cfg_debounceMs = server.arg("dbnc").toInt();
  if (server.hasArg("gMax")) cfg_gaugeMax = server.arg("gMax").toInt();
  
  if (server.hasArg("imp") || src == "settings") cfg_imperial = server.arg("imp").toInt();
  if (server.hasArg("fahr") || src == "settings") cfg_fahrenheit = server.arg("fahr").toInt();
  
  if (!server.hasArg("prof")) { 
      if (src == "settings") {
          cfg_darkMode = server.hasArg("dark"); 
      }
      cfg_inv_brk1 = server.hasArg("iB1");
      cfg_inv_brk2 = server.hasArg("iB2");
      cfg_inv_brk3 = server.hasArg("iB3");
      cfg_inv_brk4 = server.hasArg("iB4");
      cfg_inv_dir = server.hasArg("iDir");
      cfg_inv_light = server.hasArg("iLgt");
      cfg_inv_pwm = server.hasArg("iPwm");
  }

  if(server.hasArg("cB")) stringToArray(server.arg("cB"), cfg_curveBat);
  if(server.hasArg("cR")) stringToArray(server.arg("cR"), cfg_curveRem);

  if (server.hasArg("bB")) cfg_betaBat = server.arg("bB").toFloat();
  if (server.hasArg("bE")) cfg_betaEsc = server.arg("bE").toFloat();

  if (server.hasArg("fsMs")) cfg_fsMs = server.arg("fsMs").toInt(); 
  cfg_fsLight = server.hasArg("fsLgt");

  if (server.hasArg("dead")) cfg_deadzone = server.arg("dead").toInt();
  if (server.hasArg("dMax")) cfg_deadzoneMax = server.arg("dMax").toInt();
  if (server.hasArg("bDead")) cfg_brakeDeadzone = server.arg("bDead").toInt();
  if (server.hasArg("bDMax")) cfg_brakeDeadzoneMax = server.arg("bDMax").toInt();

  if (server.hasArg("aoMin")) cfg_autoOffMins = server.arg("aoMin").toInt();
  if (server.hasArg("aoEn") || src == "settings") cfg_autoOffEn = server.hasArg("aoEn");
  if (server.hasArg("disPwm") || src == "settings") cfg_disablePwmOnLowBat = server.hasArg("disPwm"); 

  saveRemoteSettings();

  if (src == "settings") server.sendHeader("Location", "/settings");
  else server.sendHeader("Location", "/");
  server.send(303);
}

/*
 * Function: setupWebServer
 * Initializes the web server and its associated routes based on the board's ID.
 */
void setupWebServer() {
    if (BOARD_ID == 1) {
        WiFi.softAP("LongboardController", "12345678", WIFI_CHANNEL);
        server.on("/", []() {
            String h = "<!DOCTYPE html><html><body style='font-family:sans-serif;background:#222;color:#fff;text-align:center;padding:50px;'>";
            h += "<h1>Longboard Controller</h1><h3>" + String(FW_VERSION) + "</h3>";
            h += "<button onclick=\"location.href='/update'\" style='padding:10px 20px;cursor:pointer;'>UPDATE FIRMWARE</button></body></html>";
            server.send(200, "text/html", h);
        });
        setupOTA();
        server.begin();
    }
    if (BOARD_ID == 2) {
        WiFi.softAP("LongboardRemote", "12345678", WIFI_CHANNEL);
        setupOTA();
        server.on("/", handleRoot);
        server.on("/settings", handleSettings);
        server.on("/save", handleSave);
        server.on("/data", handleData);
        server.begin();
    }
}

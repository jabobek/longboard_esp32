/*
 * Includes
 * Necessary libraries and header files for the remote board.
 */
#include "board_remote.h"
#include "config.h"
#include "utils.h"
#include "storage.h"
#include "web_server.h"
#include "communication.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

/*
 * Prototypes
 * Function declarations for the remote board.
 */
#if BOARD_ID == 2
void drawOLED();
#endif

/*
 * Function: drawOLED
 * Handles the graphical user interface on the OLED display, showing battery levels, speed, and other telemetry.
 */
#if BOARD_ID == 2
void drawOLED() {
  u8g2_uint_t display_w = u8g2.getDisplayWidth(); 
  u8g2_uint_t display_h = u8g2.getDisplayHeight(); 

  u8g2.clearBuffer();
  u8g2.setDrawColor(1);
  static bool blink_state = true;
  blink_state = !blink_state;

  u8g2.setFont(u8g2_font_helvR08_tr); 
  
  u8g2_uint_t bat_draw_w = 18; 
  u8g2_uint_t bat_draw_h = 8;
  u8g2_uint_t bat_text_y = 20; 

  u8g2_uint_t cont_bat_x = 2; 
  u8g2_uint_t cont_bat_y = 2; 
  
  u8g2.drawFrame(cont_bat_x, cont_bat_y, bat_draw_w, bat_draw_h);
  u8g2.drawBox(cont_bat_x + bat_draw_w, cont_bat_y + 2, 1, 4);
  u8g2.drawBox(cont_bat_x + 2, cont_bat_y + 2, (bat_draw_w - 4) * (myTelem.batPct / 100.0), 4);
  if (myTelem.isCharging && blink_state) {
    u8g2.setFont(u8g2_font_open_iconic_weather_4x_t);
    u8g2.drawGlyph(cont_bat_x + 5, cont_bat_y + 6, 0x26C6);
  }
  
  u8g2.setFont(u8g2_font_helvR08_tr);
  char board_bat_buf[8];
  sprintf(board_bat_buf, "%d%%", myTelem.batPct);
  u8g2.drawStr(cont_bat_x, bat_text_y, board_bat_buf); 

  int signal_x = (display_w / 2) - 6; 
  int signal_y_base = 10; 
  bool no_signal_condition = false; 

  if (millis() - lastTelemetryRecvTime > myCmd.cfgFailsafeMs * 2) { 
    no_signal_condition = true;
  } else {
    int signal_level = 0;
    if (lastLatency < 3) signal_level = 4;
    else if (lastLatency < 6) signal_level = 3;
    else if (lastLatency < 10) signal_level = 2;
    else if (lastLatency < myCmd.cfgFailsafeMs / 2) signal_level = 1; 

    for (int i = 0; i < 4; i++) {
      u8g2_uint_t bar_h = 2 + (i * 2);
      u8g2_uint_t bar_y = signal_y_base - bar_h;
      if (i < signal_level) u8g2.drawBox(signal_x + (i * 3), bar_y, 2, bar_h);
      else u8g2.drawFrame(signal_x + (i * 3), bar_y, 2, bar_h);
    }

    if (lastLatency > myCmd.cfgFailsafeMs) {
      u8g2.setFont(u8g2_font_open_iconic_thing_4x_t); 
      u8g2.drawGlyph(signal_x + 2, signal_y_base + 10, 0xE006); 
    }
  }

  u8g2_uint_t rem_bat_x_pos = display_w - bat_draw_w - 2; 
  u8g2_uint_t rem_bat_y_pos = 2; 
  
  u8g2.drawFrame(rem_bat_x_pos, rem_bat_y_pos, bat_draw_w, bat_draw_h);
  u8g2.drawBox(rem_bat_x_pos + bat_draw_w, rem_bat_y_pos + 2, 1, 4);
  u8g2.drawBox(rem_bat_x_pos + 2, rem_bat_y_pos + 2, (bat_draw_w - 4) * (remBatPct / 100.0), 4);
  
  if (cfg_isCharging && blink_state) {
    u8g2.setFont(u8g2_font_open_iconic_weather_4x_t);
    u8g2.drawGlyph(rem_bat_x_pos + 5, rem_bat_y_pos + 6, 0x26C6);
  }

  float remaining_hrs = cfg_remRuntimeHrs * (remBatPct / 100.0);
  char rem_time_buf[10];
  if (remaining_hrs < 10) { 
    dtostrf(remaining_hrs, 0, 1, rem_time_buf); 
  } else { 
    dtostrf(remaining_hrs, 0, 0, rem_time_buf); 
  }
  strcat(rem_time_buf, "h");
  u8g2.drawStr(rem_bat_x_pos, bat_text_y, rem_time_buf); 

  u8g2.drawHLine(0, 24, display_w); 

  if (no_signal_condition) {
    u8g2.setFont(u8g2_font_helvB24_tn); 
    const char* no_signal_char = "X";
    u8g2_uint_t x_pos = (display_w - u8g2.getStrWidth(no_signal_char)) / 2;
    u8g2.drawStr(x_pos, 56, no_signal_char); 
  } else {
    u8g2.setFont(u8g2_font_helvB24_tn);
    char speed_buf[10];
    float display_speed = myTelem.speed * (cfg_imperial ? 0.621371 : 1.0);
    dtostrf(display_speed, 0, 0, speed_buf);
    u8g2_uint_t text_width_speed = u8g2.getStrWidth(speed_buf);
    u8g2.drawStr((display_w - text_width_speed) / 2, 56, speed_buf); 
  }

  u8g2.drawHLine(0, 60, display_w); 

  u8g2.setFont(u8g2_font_helvR10_tr); 
  char trip_buf[10];
  float trip_dist = myTelem.distTrip * (cfg_imperial ? 0.621371 : 1.0);
  dtostrf(trip_dist, 0, 1, trip_buf);
  const char* unit_dist = cfg_imperial ? "mi" : "km";
  strcat(trip_buf, " "); 
  strcat(trip_buf, unit_dist);
  u8g2_uint_t text_width_trip = u8g2.getStrWidth(trip_buf);
  u8g2.drawStr((display_w - text_width_trip) / 2, 74, trip_buf); 

  u8g2.drawHLine(0, 78, display_w); 

  u8g2.setFont(u8g2_font_helvR08_tr);
  const char* profiles_str[] = {"E", "N", "M"};
  u8g2_uint_t profile_spacing = 2; 

  u8g2_uint_t font_height = u8g2.getFontAscent() - u8g2.getFontDescent();
  u8g2_uint_t box_size = font_height + 4; 

  u8g2_uint_t total_profiles_width = (box_size + profile_spacing) * 3 - profile_spacing;
  u8g2_uint_t start_x_profiles = (display_w - total_profiles_width) / 2;
  u8g2_uint_t profile_y = 84; 

  for (int i = 0; i < 3; i++) {
    u8g2_uint_t current_char_width = u8g2.getStrWidth(profiles_str[i]);
    u8g2_uint_t x_offset_char = (box_size - current_char_width) / 2;
    u8g2_uint_t y_offset_text = profile_y + (box_size - font_height) / 2 + u8g2.getFontAscent();


    if (i == activeProfile) {
      u8g2.drawBox(start_x_profiles, profile_y, box_size, box_size); 
      u8g2.setDrawColor(0); 
      u8g2.drawStr(start_x_profiles + x_offset_char, y_offset_text, profiles_str[i]);
      u8g2.setDrawColor(1); 
    } else {
      u8g2.drawFrame(start_x_profiles, profile_y, box_size, box_size); 
      u8g2.drawStr(start_x_profiles + x_offset_char, y_offset_text, profiles_str[i]);
    }
    start_x_profiles += box_size + profile_spacing; 
  }

  u8g2.drawHLine(0, 100, display_w); 

  u8g2.setFont(u8g2_font_helvR08_tr);
  
  char bat_temp_buf[10];
  float display_bat_temp = myTelem.tBat;
  if (cfg_fahrenheit) display_bat_temp = (display_bat_temp * 9.0/5.0) + 32.0;
  dtostrf(display_bat_temp, 0, 0, bat_temp_buf);
  strcat(bat_temp_buf, "°");
  strcat(bat_temp_buf, cfg_fahrenheit ? "F" : "C");
  u8g2.drawStr(2, 114, bat_temp_buf); 

  char esc_temp_buf[10];
  float display_esc_temp = myTelem.tEsc;
  if (cfg_fahrenheit) display_esc_temp = (display_esc_temp * 9.0/5.0) + 32.0;
  dtostrf(display_esc_temp, 0, 0, esc_temp_buf);
  strcat(esc_temp_buf, "°");
  strcat(esc_temp_buf, cfg_fahrenheit ? "F" : "C");
  u8g2.drawStr(2, 126, esc_temp_buf); 

  char range_buf[16]; 
  float remaining_range = profiles[activeProfile].estRangeKm * (myTelem.batPct / 100.0) * (cfg_imperial ? 0.621371 : 1.0);
  dtostrf(remaining_range, 0, 0, range_buf);
  strcat(range_buf, " "); 
  strcat(range_buf, cfg_imperial ? "mi" : "km");
  u8g2_uint_t range_width = u8g2.getStrWidth(range_buf);
  u8g2.drawStr(display_w - range_width - 2, 114, range_buf); 

  char pwr_buf[10];
  float pwr_val = (myTelem.voltage * myTelem.current) / 1000.0;
  dtostrf(pwr_val, 0, 1, pwr_buf);
  strcat(pwr_buf, "kW");
  u8g2_uint_t pwr_width = u8g2.getStrWidth(pwr_buf);
  u8g2.drawStr(display_w - pwr_width - 2, 126, pwr_buf); 
  u8g2.sendBuffer();
}

/*
 * Function: setupRemote
 * Initializes settings, wakeup sources, display, and pin modes for the remote.
 */
void setupRemote() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
    loadRemoteSettings(); 
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_26, 0);
         
    #if BOARD_ID == 2
  Wire.begin(OLED_SDA, OLED_SCL);
  u8g2.begin();
  u8g2.clearBuffer();
  
  u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.drawGlyph( (u8g2.getDisplayWidth() - 12) / 2, (u8g2.getDisplayHeight() / 2) - 10, 0x231B); 
  
  u8g2.setFont(u8g2_font_ncenB08_tr);
  String version_text = "v" + String(FW_VERSION);
  u8g2_uint_t version_width = u8g2.getStrWidth(version_text.c_str());
  u8g2.drawStr((u8g2.getDisplayWidth() - version_width) / 2, u8g2.getDisplayHeight() - 2, version_text.c_str()); 
  
  u8g2.sendBuffer();
  delay(2000);
#endif
    
        pinMode(PIN_JOY_X, INPUT);  
    pinMode(PIN_REM_BRK_ANA, INPUT); 
    
    pinMode(PIN_BTN_BRK, INPUT_PULLUP); 
    pinMode(PIN_BTN_DIR, INPUT_PULLUP); pinMode(PIN_BTN_LGT, INPUT_PULLUP); 
    pinMode(PIN_CHG, INPUT); 
    
    WiFi.mode(WIFI_AP_STA);
    lastActivity = millis(); 
}
#endif

/*
 * Function: loopRemote
 * Main execution loop for the remote, processing inputs, battery status, and updating telemetry to the controller.
 */
void loopRemote() {
    server.handleClient();
    unsigned long now = millis();

    float rbRaw = analogRead(PIN_REM_BAT) / 4095.0; 
    float rbVol = rbRaw * cfg_remBatScale * 3.3; 
    remVoltageSmooth = (remVoltageSmooth * 0.9) + (rbVol * 0.1);
    
    int rbPct = getBatPct(remVoltageSmooth / cfg_remSeriesCells, cfg_curveRem, 4.2); 
    remBatPct = rbPct;
    
    if (rbPct >= 100) remBatFullFlag = true;
    if (remBatFullFlag && rbPct < 90) { remBatCycles++; remBatFullFlag = false; saveRemoteSettings(); }

    if(myTelem.tBat > -90 && myTelem.tBat > maxTempBat) maxTempBat = myTelem.tBat;
    if(myTelem.tEsc > -90 && myTelem.tEsc > maxTempEsc) maxTempEsc = myTelem.tEsc;
    if(myTelem.current > maxAmpsSession) maxAmpsSession = myTelem.current;
    float curP = myTelem.voltage * myTelem.current;
    if(curP > maxPowerSession) maxPowerSession = curP;

    if (now - lastTick >= 10) { 
      int rawJoy = analogRead(PIN_JOY_X);
      int rawBrakeAna = analogRead(PIN_REM_BRK_ANA); 
      
      bool bBrk = readButtonDebounce(PIN_BTN_BRK, lastStateBrk, lastTimeBrk);
      bool bDir = readButtonDebounce(PIN_BTN_DIR, lastStateDir, lastTimeDir);
      
      bool bLgtRaw = (digitalRead(PIN_BTN_LGT) == LOW);
      if (bLgtRaw && !lgtBtnProcessed) {
         if (lgtPressStart == 0) lgtPressStart = now;
         if (now - lgtPressStart > 1000) { 
            if (rawJoy < cfg_deadzone) { 
               activeProfile++; if (activeProfile > 2) activeProfile = 0;
               delay(500); 
            }
            lgtBtnProcessed = true; lgtPressStart = 0; 
         }
      } else if (!bLgtRaw) {
         if (lgtPressStart > 0 && !lgtBtnProcessed) lightState = !lightState; 
         lgtPressStart = 0; lgtBtnProcessed = false; 
      }

      cfg_isCharging = digitalRead(PIN_CHG);

      if ((rawJoy > 250) || (rawBrakeAna > 100) || bBrk || bDir || bLgtRaw || cfg_isCharging) lastActivity = now; 
      if (cfg_autoOffEn && (now - lastActivity > (cfg_autoOffMins*60000UL))) {
          u8g2.clearDisplay(); 
          u8g2.sendBuffer();   
          delay(500); 
          esp_deep_sleep_start();
      }

      int mappedThrottle = 0;
      if (rawJoy > cfg_deadzone) {
          mappedThrottle = map(rawJoy, cfg_deadzone, cfg_deadzoneMax, 0, 4095);
          mappedThrottle = constrain(mappedThrottle, 0, 4095);
      }

      int mappedBrake = 0;
      if (rawBrakeAna > cfg_brakeDeadzone) {
          mappedBrake = map(rawBrakeAna, cfg_brakeDeadzone, cfg_brakeDeadzoneMax, 0, 4095);
          mappedBrake = constrain(mappedBrake, 0, 4095);
      }

      myCmd.throttle = mappedThrottle; 
      myCmd.brakeBtn = bBrk; 
      myCmd.brakeAnalog = mappedBrake; 
      myCmd.dir = bDir; myCmd.light = lightState;
      
      myCmd.cfgMasterPower = profiles[activeProfile].maxPower;
      myCmd.cfgSpeedLimit  = profiles[activeProfile].speedLimit;
      myCmd.cfgEnableSpeedLimit = profiles[activeProfile].enableSpeedLimit; 
      myCmd.cfgCurve       = profiles[activeProfile].curve;
      myCmd.powerM1 = profiles[activeProfile].m1; myCmd.powerM2 = profiles[activeProfile].m2;
      myCmd.powerM3 = profiles[activeProfile].m3; myCmd.powerM4 = profiles[activeProfile].m4;
      
      myCmd.cfgBrakeCurve = cfg_brakeCurve;
      myCmd.cfgButtonBrakeMask = cfg_buttonBrakeMask;
      for(int i=0; i<5; i++) myCmd.cfgAnalogBrakeMask[i] = cfg_analogBrakeMask[i];
      
      myCmd.cfgFailsafeMs = cfg_fsMs; 
      myCmd.cfgFailsafeBrakeMask = cfg_fsBrakeMask;
      myCmd.cfgFailsafeLight = cfg_fsLight;

      myCmd.whFL = cfg_whFL; myCmd.whRL = cfg_whRL; 
      myCmd.whRL = cfg_whRL; myCmd.whRR = cfg_whRR;

      myCmd.cfgVoltsScale = cfg_voltsScale;
      myCmd.cfgIna219Scale = cfg_amps_scale_ina219;
      myCmd.cfgIna219Offset = cfg_amps_offset_ina219;
      myCmd.cfgSeriesCells = cfg_seriesCells; 
      myCmd.cfgPulsesPerRev = cfg_pulsesPerRev; 
      
      myCmd.pidEn = cfg_pidEn; myCmd.Kp = cfg_Kp; myCmd.Ki = cfg_Ki; myCmd.Kd = cfg_Kd;

      myCmd.cfgAutoResetTrip = cfg_autoReset; myCmd.cmdResetTrip = reqResetTrip; 
      
      myCmd.cfgImperial = cfg_imperial; myCmd.cfgFahrenheit = cfg_fahrenheit;
      myCmd.cfgBetaBat = cfg_betaBat; myCmd.cfgBetaEsc = cfg_betaEsc;
      myCmd.isCharging = cfg_isCharging; 
      
      myCmd.cfg_inv_brk1 = cfg_inv_brk1; myCmd.cfg_inv_brk2 = cfg_inv_brk2;
      myCmd.cfg_inv_brk3 = cfg_inv_brk3; myCmd.cfg_inv_brk4 = cfg_inv_brk4;
      myCmd.cfg_inv_dir = cfg_inv_dir;
      myCmd.cfg_inv_light = cfg_inv_light;
      myCmd.cfg_inv_pwm = cfg_inv_pwm;
      
      memcpy(myCmd.batCurve, cfg_curveBat, sizeof(cfg_curveBat));

      if (reqResetTrip) reqResetTrip = false; 

      sendStartTime = micros(); 
      if(!isPairingMode) {
          esp_now_send(targetMac, (uint8_t *) &myCmd, sizeof(myCmd));
      }
      lastTick = now;
    }

    if (now - lastDraw > 1000) {
#if BOARD_ID == 2
      u8g2.firstPage();
      drawOLED(); 
#endif
      lastDraw = now;
    }
}

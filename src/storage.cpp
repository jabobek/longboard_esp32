/* Include storage, configuration, and utility headers */
#include "storage.h"
#include "config.h"
#include "utils.h"

/* Load remote settings from non-volatile storage */
void loadRemoteSettings() {
  prefs.begin("rc-rem", true); 
  
  if (BOARD_ID == 2) { 
      if(prefs.getBytes("target", targetMac, 6) != 6) { }
  }

  cfg_deadzone = prefs.getInt("dead", 250); cfg_deadzoneMax = prefs.getInt("dMax", 3900);
  cfg_brakeDeadzone = prefs.getInt("bDead", 250); cfg_brakeDeadzoneMax = prefs.getInt("bDMax", 3900);

  cfg_fsMs = prefs.getInt("fsMs", 1000);     
  cfg_fsBrakeMask = prefs.getUChar("fsMask", 15); 
  cfg_fsLight = prefs.getBool("fsLgt", true);
  
  cfg_autoOffEn = prefs.getBool("aoEn", true);
  cfg_autoOffMins = prefs.getInt("aoMin", 5); activeProfile = prefs.getInt("actProf", 1);
  cfg_darkMode = prefs.getBool("dark", true); 
  cfg_disablePwmOnLowBat = prefs.getBool("disPwm", true); 
  
  cfg_whFL = prefs.getFloat("wFL", 25.4); cfg_whFR = prefs.getFloat("wFR", 25.4);
  cfg_whRL = prefs.getFloat("wRL", 25.4); cfg_whRR = prefs.getFloat("wRR", 25.4);

  cfg_voltsScale = prefs.getFloat("vScl", 66.6); 
  cfg_amps_scale_ina219 = prefs.getFloat("inaScl", 1.0);
  cfg_amps_offset_ina219 = prefs.getFloat("inaOff", 0.0);
  
  cfg_remBatScale = prefs.getFloat("rbScl", 2.0); cfg_remRuntimeHrs = prefs.getFloat("rbRun", 5.0);
  remBatCycles = prefs.getInt("rbCyc", 0);
  if (remBatCycles > 5000) remBatCycles = 0; 

  cfg_pidEn = prefs.getBool("pid", false);
  cfg_Kp = prefs.getFloat("kp", 2.5); cfg_Ki = prefs.getFloat("ki", 0.1); cfg_Kd = prefs.getFloat("kd", 0.5);

  cfg_autoReset = prefs.getBool("aRes", false);
  cfg_debounceMs = prefs.getInt("dbnc", 50);
  cfg_gaugeMax = prefs.getInt("gMax", 40);
  cfg_seriesCells = prefs.getInt("sCells", 14);
  cfg_remSeriesCells = prefs.getInt("rbCells", 1);

  cfg_imperial = prefs.getBool("imp", false); cfg_fahrenheit = prefs.getBool("fahr", false);
  cfg_betaBat  = prefs.getFloat("bB", 3950.0); cfg_betaEsc  = prefs.getFloat("bE", 3950.0);

  cfg_pulsesPerRev = prefs.getInt("ppr", 15);

  cfg_inv_brk1 = prefs.getBool("iB1", false); cfg_inv_brk2 = prefs.getBool("iB2", false);
  cfg_inv_brk3 = prefs.getBool("iB3", false); cfg_inv_brk4 = prefs.getBool("iB4", false);
  cfg_inv_dir = prefs.getBool("iDir", false);
  cfg_inv_light = prefs.getBool("iLgt", false);
  cfg_inv_pwm = prefs.getBool("iPwm", false);

  String sC = prefs.getString("cB", "");
  if(sC.length() < 5) memcpy(cfg_curveBat, defaultCurve, sizeof(defaultCurve));
  else stringToArray(sC, cfg_curveBat);

  String sR = prefs.getString("cR", "");
  if(sR.length() < 5) memcpy(cfg_curveRem, defaultCurve, sizeof(defaultCurve));
  else stringToArray(sR, cfg_curveRem);

    for (int i=0; i<3; i++) {
      String p = "p" + String(i);
      profiles[i].maxPower = prefs.getInt((p+"_s").c_str(), 40);
      profiles[i].speedLimit = prefs.getFloat((p+"_sl").c_str(), 30.0);
      profiles[i].enableSpeedLimit = prefs.getBool((p+"_esl").c_str(), true);
      profiles[i].curve    = prefs.getFloat((p+"_c").c_str(), 3.0);
      profiles[i].m1 = prefs.getInt((p+"_1").c_str(), 80);
      profiles[i].m2 = prefs.getInt((p+"_2").c_str(), 80);
      profiles[i].m3 = prefs.getInt((p+"_3").c_str(), 80);
          profiles[i].m4 = prefs.getInt((p+"_4").c_str(), 80);
          profiles[i].estRangeKm = prefs.getFloat((p+"_r").c_str(), 20.0);
        }
        cfg_brakeCurve = prefs.getFloat("bCrv", cfg_brakeCurve);
        if(cfg_brakeCurve < 0.1) cfg_brakeCurve = 1.0;
        
        cfg_buttonBrakeMask = prefs.getUChar("bBbm", cfg_buttonBrakeMask);
        uint8_t defaultAnalogBrakeMasks[] = {0, 1, 3, 7, 15};
        for(int j=0; j<5; j++) cfg_analogBrakeMask[j] = prefs.getUChar(("bAm"+String(j)).c_str(), defaultAnalogBrakeMasks[j]);
      
        prefs.end();
}

/* Save remote settings to non-volatile storage */
void saveRemoteSettings() {
  prefs.begin("rc-rem", false); 
  
  if (BOARD_ID == 2) prefs.putBytes("target", targetMac, 6);

  prefs.putInt("dead", cfg_deadzone); prefs.putInt("dMax", cfg_deadzoneMax);
  prefs.putInt("bDead", cfg_brakeDeadzone); prefs.putInt("bDMax", cfg_brakeDeadzoneMax);

  prefs.putInt("fsMs", cfg_fsMs);     
  prefs.putUChar("fsMask", cfg_fsBrakeMask); 
  prefs.putBool("fsLgt", cfg_fsLight);
  
  prefs.putBool("aoEn", cfg_autoOffEn);
  prefs.putInt("aoMin", cfg_autoOffMins); prefs.putInt("actProf", activeProfile);
  prefs.putBool("dark", cfg_darkMode); 
  prefs.putBool("disPwm", cfg_disablePwmOnLowBat);
  
  prefs.putFloat("wFL", cfg_whFL); prefs.putFloat("wFR", cfg_whFR);
  prefs.putFloat("wRL", cfg_whRL); prefs.putFloat("wRR", cfg_whRR);

  prefs.putFloat("vScl", cfg_voltsScale);
  prefs.putFloat("inaScl", cfg_amps_scale_ina219);
  prefs.putFloat("inaOff", cfg_amps_offset_ina219);
  
  prefs.putFloat("rbScl", cfg_remBatScale); prefs.putFloat("rbRun", cfg_remRuntimeHrs);
  prefs.putInt("rbCyc", remBatCycles);

  prefs.putBool("pid", cfg_pidEn); prefs.putFloat("kp", cfg_Kp);
  prefs.putFloat("ki", cfg_Ki); prefs.putFloat("kd", cfg_Kd);

  prefs.putBool("aRes", cfg_autoReset);
  prefs.putInt("dbnc", cfg_debounceMs);
  prefs.putInt("gMax", cfg_gaugeMax);
  prefs.putInt("sCells", cfg_seriesCells);
  prefs.putInt("rbCells", cfg_remSeriesCells);
  
  prefs.putBool("imp", cfg_imperial); prefs.putBool("fahr", cfg_fahrenheit);
  prefs.putFloat("bB", cfg_betaBat); prefs.putFloat("bE", cfg_betaEsc);

  prefs.putInt("ppr", cfg_pulsesPerRev);
  
  prefs.putBool("iB1", cfg_inv_brk1); prefs.putBool("iB2", cfg_inv_brk2);
  prefs.putBool("iB3", cfg_inv_brk3); prefs.putBool("iB4", cfg_inv_brk4);
  prefs.putBool("iDir", cfg_inv_dir);
  prefs.putBool("iLgt", cfg_inv_light);
  prefs.putBool("iPwm", cfg_inv_pwm);

  char buf[100];
  arrayToString(cfg_curveBat, buf); prefs.putString("cB", String(buf));
  arrayToString(cfg_curveRem, buf); prefs.putString("cR", String(buf));

  for (int i=0; i<3; i++) {
    String p = "p" + String(i);
    prefs.putInt((p+"_s").c_str(), profiles[i].maxPower);
    prefs.putFloat((p+"_sl").c_str(), profiles[i].speedLimit);
    prefs.putBool((p+"_esl").c_str(), profiles[i].enableSpeedLimit);
    prefs.putFloat((p+"_c").c_str(), profiles[i].curve);
    prefs.putInt((p+"_1").c_str(), profiles[i].m1);
    prefs.putInt((p+"_2").c_str(), profiles[i].m2);
    prefs.putInt((p+"_3").c_str(), profiles[i].m3);
    prefs.putInt((p+"_4").c_str(), profiles[i].m4);
    prefs.putFloat((p+"_r").c_str(), profiles[i].estRangeKm);
  }
  prefs.putFloat("bCrv", cfg_brakeCurve);
  prefs.putUChar("bBbm", cfg_buttonBrakeMask);
  for(int j=0; j<5; j++) prefs.putUChar(("bAm"+String(j)).c_str(), cfg_analogBrakeMask[j]);

  prefs.end();
}

/* Load board cumulative statistics from non-volatile storage */
void loadBoardStats() {
  prefs.begin("rc-bot", true);
  valDistTotal = prefs.getFloat("dTot", 0.0F);
  if (isnan(valDistTotal)) valDistTotal = 0.0F;
  valDistTrip  = prefs.getFloat("dTrip", 0.0F);
  if (isnan(valDistTrip)) valDistTrip = 0.0F;
  valWhConsumed = prefs.getFloat("WhC", 0.0F);
  if (isnan(valWhConsumed)) valWhConsumed = 0.0F;
  valmAhConsumed = prefs.getFloat("mAhC", 0.0F);
  if (isnan(valmAhConsumed)) valmAhConsumed = 0.0F;
  valWhCharging = prefs.getFloat("WhChg", 0.0F);
  if (isnan(valWhCharging)) valWhCharging = 0.0F;
  valmAhCharging = prefs.getFloat("mAhChg", 0.0F);
  if (isnan(valmAhCharging)) valmAhCharging = 0.0F;
  batCycles    = prefs.getInt("cyc", 0);
  prefs.end();
}

/* Save board cumulative statistics to non-volatile storage */
void saveBoardStats() {
  prefs.begin("rc-bot", false);
  prefs.putFloat("dTot", valDistTotal); prefs.putFloat("dTrip", valDistTrip);
  prefs.putFloat("WhC", valWhConsumed);
  prefs.putFloat("mAhC", valmAhConsumed);
  prefs.putFloat("WhChg", valWhCharging);
  prefs.putFloat("mAhChg", valmAhCharging);
  prefs.putInt("cyc", batCycles);
  prefs.end();
}

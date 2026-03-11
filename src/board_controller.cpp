/*
 * Includes
 * Essential libraries and header files for the board controller.
 */
#include "board_controller.h"
#include "config.h"
#include "utils.h"
#include "storage.h"
#include "web_server.h"
#include "communication.h"
#include <Adafruit_INA219.h>

/*
 * Globals
 * Instance of the INA219 current sensor.
 */
Adafruit_INA219 ina219(INA219_I2C_ADDRESS);

/*
 * Function: setIna219calibration
 * Configures the INA219 sensor with custom calibration for a 100A/75mV shunt.
 */
void setIna219calibration() {
  uint16_t config = 0x299F; 
  uint16_t calValue = 17906;

  Wire.beginTransmission(INA219_I2C_ADDRESS);
  Wire.write(0x00); 
  Wire.write((config >> 8) & 0xFF);
  Wire.write(config & 0xFF);
  Wire.endTransmission();

  Wire.beginTransmission(INA219_I2C_ADDRESS);
  Wire.write(0x05); 
  Wire.write((calValue >> 8) & 0xFF);
  Wire.write(calValue & 0xFF);
  Wire.endTransmission();
  
  Serial.println("Custom INA219 calibration for 100A/75mV shunt applied.");
}

/*
 * Function: setupController
 * Initializes hardware peripherals, interrupts, and storage for the controller board.
 */
void setupController() {
    ledcAttachPin(PIN_M1, 0); ledcSetup(0, 10000, 8);
    ledcAttachPin(PIN_M2, 1); ledcSetup(1, 10000, 8);
    ledcAttachPin(PIN_M3, 2); ledcSetup(3, 10000, 8);
    ledcAttachPin(PIN_M4, 3); ledcSetup(3, 10000, 8);
    
    pinMode(PIN_ESC_BRK_1, OUTPUT);
    pinMode(PIN_ESC_BRK_2, OUTPUT);
    pinMode(PIN_ESC_BRK_3, OUTPUT);
    pinMode(PIN_ESC_BRK_4, OUTPUT);
    
    pinMode(PIN_ESC_DIR, OUTPUT); 
    pinMode(PIN_ESC_INV_DIR, OUTPUT); 
    pinMode(PIN_ESC_LIGHT, OUTPUT);
    
    pinMode(PIN_HALL_FL, INPUT_PULLUP); attachInterrupt(PIN_HALL_FL, isrFL, RISING);
    pinMode(PIN_HALL_FR, INPUT_PULLUP); attachInterrupt(PIN_HALL_FR, isrFR, RISING);
    pinMode(PIN_HALL_RL, INPUT_PULLUP); attachInterrupt(PIN_HALL_RL, isrRL, RISING);
    pinMode(PIN_HALL_RR, INPUT_PULLUP); attachInterrupt(PIN_HALL_RR, isrRR, RISING);
    pinMode(PIN_CHG, INPUT); 
    
    loadBoardStats(); 
    lastSpeedMeasureTime = millis(); 
    lastWhCalcTime = millis(); 
    
    Serial.begin(115200); 
    Wire.begin(OLED_SDA, OLED_SCL); 
    if (!ina219.begin()) {
        Serial.println("Failed to find INA219 chip");
    } else {
        setIna219calibration();
        Serial.println("INA219 initialized!");
    }

    WiFi.mode(WIFI_AP_STA); 
}

/*
 * Function: loopController
 * Main execution loop for the controller board handling telemetry, safety, and motor control.
 */
void loopController() {
    server.handleClient(); 
    unsigned long now = millis();

    noInterrupts(); 
    unsigned long pFL=pcFL; unsigned long pFR=pcFR; 
    unsigned long pRL=pcRL; unsigned long pRR=pcRR;
    pcFL=0; pcFR=0; pcRL=0; pcRR=0; 
    interrupts();

    float cFL = (3.14159 * (myCmd.whFL > 0 ? myCmd.whFL : 25.4)) / 100.0; 
    float cFR = (3.14159 * (myCmd.whFR > 0 ? myCmd.whFR : 25.4)) / 100.0;
    float cRL = (3.14159 * (myCmd.whRL > 0 ? myCmd.whRL : 25.4)) / 100.0;
    float cRR = (3.14159 * (myCmd.whRR > 0 ? myCmd.whRR : 25.4)) / 100.0;
    float avgCircumference_loop = (cFL + cFR + cRL + cRR) / 4.0; 

    double dFL_10ms = (myCmd.cfgPulsesPerRev > 0) ? (pFL/(float)myCmd.cfgPulsesPerRev)*cFL : 0;
    double dFR_10ms = (myCmd.cfgPulsesPerRev > 0) ? (pFR/(float)myCmd.cfgPulsesPerRev)*cFR : 0;
    double dRL_10ms = (myCmd.cfgPulsesPerRev > 0) ? (pRL/(float)myCmd.cfgPulsesPerRev)*cRL : 0;
    double dRR_10ms = (myCmd.cfgPulsesPerRev > 0) ? (pRR/(float)myCmd.cfgPulsesPerRev)*cRR : 0;
    double distAvg_10ms = (dFL_10ms+dFR_10ms+dRL_10ms+dRR_10ms)/4.0;

    valDistTotal += distAvg_10ms; valDistTrip += distAvg_10ms; valDistSession += distAvg_10ms;

    pulsesAccumulated[0] += pFL;
    pulsesAccumulated[1] += pFR;
    pulsesAccumulated[2] += pRL;
    pulsesAccumulated[3] += pRR;

    if (now - lastSpeedMeasureTime >= 500) {
        float accumulatedPulsesAvg = (pulsesAccumulated[0] + pulsesAccumulated[1] + pulsesAccumulated[2] + pulsesAccumulated[3]) / 4.0;
        
        if (myCmd.cfgPulsesPerRev > 0 && avgCircumference_loop > 0) { 
            float distance_500ms = (accumulatedPulsesAvg / (float)myCmd.cfgPulsesPerRev) * avgCircumference_loop; 
            currentKmh = (distance_500ms / 0.5) * 3.6; 
        } else {
            currentKmh = 0.0;
        }
        
        pulsesAccumulated[0] = 0; pulsesAccumulated[1] = 0; pulsesAccumulated[2] = 0; pulsesAccumulated[3] = 0;
        lastSpeedMeasureTime = now;
    }

    if (now - lastSaveTime > 10000) { saveBoardStats(); lastSaveTime = now; }

    float rawV = analogRead(PIN_ESC_BAT) / 4095.0;
    float voltage = rawV * (myCmd.cfgVoltsScale > 0 ? myCmd.cfgVoltsScale : 66.6);
    float amps = (ina219.getCurrent_mA() / 1000.0 + myCmd.cfgIna219Offset) * myCmd.cfgIna219Scale; 
    float tB = readNTC(PIN_ESC_NTC_B, myCmd.cfgBetaBat); float tE = readNTC(PIN_ESC_NTC_E, myCmd.cfgBetaEsc);
    
    float power_w = voltage * amps;
    unsigned long current_millis = millis();
    float time_elapsed_h = (float)(current_millis - lastWhCalcTime) / 3600000.0; 
    
    if (power_w >= 0) { 
        valWhConsumed += power_w * time_elapsed_h;
        valmAhConsumed += (amps * 1000) * time_elapsed_h;
    } else { 
        valWhCharging += power_w * time_elapsed_h; 
        valmAhCharging += (amps * 1000) * time_elapsed_h; 
    }
    lastWhCalcTime = current_millis;

    if (myCmd.cmdResetConsumed) {
        valWhConsumed = 0.0;
        valmAhConsumed = 0.0;
    }
    if (myCmd.cmdResetCharging) {
        valWhCharging = 0.0;
        valmAhCharging = 0.0;
    }

    float perCellVoltage = voltage / myCmd.cfgSeriesCells;
    int pct = getBatPct(perCellVoltage, myCmd.batCurve, 4.2); 

    if (pct >= 100) batFullFlag = true; if (batFullFlag && pct < 90) { batCycles++; batFullFlag = false; saveBoardStats(); }
    if (myCmd.cmdResetTrip) { valDistTrip = 0.0; saveBoardStats(); } if (myCmd.cfgAutoResetTrip && pct >= 100 && !batFullResetDone) { valDistTrip = 0.0; batFullResetDone = true; saveBoardStats(); } if (pct < 95) batFullResetDone = false; 
    
    bool currentIsTimedOut = (now - lastRecvTime > escFsMs);

    if (currentIsTimedOut) {
        if (failsafeTriggerTime == 0) { 
            failsafeTriggerTime = now;
        }
        if (now - failsafeTriggerTime > 50) { 
            escIsConnected = false; 
        } else {
            escIsConnected = true; 
        }
    } else { 
        escIsConnected = true;
        failsafeTriggerTime = 0;
    }

    uint8_t brakeMask = 0; 
    bool anyBrakeActive = false; 

    if (escIsConnected) {
        digitalWrite(PIN_ESC_DIR, myCmd.cfg_inv_dir ? !myCmd.dir : myCmd.dir); 
        digitalWrite(PIN_ESC_INV_DIR, myCmd.cfg_inv_dir ? myCmd.dir : !myCmd.dir);
        digitalWrite(PIN_ESC_LIGHT, myCmd.cfg_inv_light ? !myCmd.light : myCmd.light); 

        if (myCmd.brakeBtn) {
            brakeMask |= myCmd.cfgButtonBrakeMask;
        }

        float rawBrk = (float)myCmd.brakeAnalog / 4095.0;
        if (rawBrk > 0.05) { 
            float curvedBrk = pow(rawBrk, myCmd.cfgBrakeCurve);
            int interval = (int)(curvedBrk * 5.0);
            if (interval > 4) interval = 4;
            brakeMask |= myCmd.cfgAnalogBrakeMask[interval];
        }
    } else {
        brakeMask = escFsMask; 
        if (escFsLight) digitalWrite(PIN_ESC_LIGHT, (now / 500) % 2); else digitalWrite(PIN_ESC_LIGHT, LOW);
    }

    anyBrakeActive = (brakeMask > 0);
    digitalWrite(PIN_ESC_BRK_1, ((brakeMask & 1) ? HIGH : LOW) ^ myCmd.cfg_inv_brk1);
    digitalWrite(PIN_ESC_BRK_2, ((brakeMask & 2) ? HIGH : LOW) ^ myCmd.cfg_inv_brk2);
    digitalWrite(PIN_ESC_BRK_3, ((brakeMask & 4) ? HIGH : LOW) ^ myCmd.cfg_inv_brk3);
    digitalWrite(PIN_ESC_BRK_4, ((brakeMask & 8) ? HIGH : LOW) ^ myCmd.cfg_inv_brk4);

    if (myCmd.cmdDisablePwmOnLowBat && myTelem.batPct <= 0) { 
       ledcWrite(0, 0); ledcWrite(1, 0); ledcWrite(2, 0); ledcWrite(3, 0);
       for(int i=0; i<4; i++) { pidErrSum[i] = 0; pidLastErr[i] = 0; }
    } else if (anyBrakeActive || currentIsTimedOut) {
       ledcWrite(0, 0); ledcWrite(1, 0); ledcWrite(2, 0); ledcWrite(3, 0);
       for(int i=0; i<4; i++) { pidErrSum[i] = 0; pidLastErr[i] = 0; }
    } else {
       if (!myCmd.pidEn) {
          int basePwm = 0;
          if (myCmd.throttle > cfg_deadzone) { 
             float inNorm = (float)(myCmd.throttle - cfg_deadzone) / (4095.0 - cfg_deadzone); 
             if(inNorm > 1.0) inNorm = 1.0; if(inNorm < 0.0) inNorm = 0.0;
             
             if (myCmd.cfg_inv_pwm) inNorm = 1.0 - inNorm; 

             float curved = pow(inNorm, myCmd.cfgCurve);
             basePwm = (int)(curved * 255.0 * ((float)myCmd.cfgMasterPower/100.0));
             
             if (myCmd.cfgEnableSpeedLimit && myCmd.cfgSpeedLimit > 0) {
                 float speedRatio = 1.0;
                 if (currentKmh > myCmd.cfgSpeedLimit) {
                     speedRatio = myCmd.cfgSpeedLimit / currentKmh;
                     if (speedRatio < 0.0) speedRatio = 0.0; 
                 }
                 basePwm = (int)(basePwm * speedRatio);
             }

             if (basePwm > 255) basePwm = 255;
          } else { 
             if (myCmd.cfg_inv_pwm) {
                basePwm = (int)(255.0 * ((float)myCmd.cfgMasterPower/100.0));
             } else {
                basePwm = 0;
             }
          }
          
          ledcWrite(0, basePwm * (myCmd.powerM1 / 100.0)); ledcWrite(1, basePwm * (myCmd.powerM2 / 100.0)); 
          ledcWrite(2, basePwm * (myCmd.powerM3 / 100.0)); ledcWrite(3, basePwm * (myCmd.powerM4 / 100.0));
       } else {
       }
    }

    if (now - lastTick >= 100) {
      myTelem.speed = currentKmh;
      myTelem.voltage = voltage; myTelem.current = amps; myTelem.batPct = pct; myTelem.batCycles = batCycles;
      myTelem.isCharging = digitalRead(PIN_CHG); 
      myTelem.distTotal = valDistTotal / 1000.0; myTelem.distTrip  = valDistTrip / 1000.0;
      myTelem.distSession = valDistSession / 1000.0; myTelem.tBat = tB; myTelem.tEsc = tE;
      myTelem.WhConsumed = valWhConsumed; 
      myTelem.mAhConsumed = valmAhConsumed; 
      myTelem.WhCharging = valWhCharging; 
      myTelem.mAhCharging = valmAhCharging; 
      
      esp_now_send(connectedRemoteMac, (uint8_t *) &myTelem, sizeof(myTelem));
      lastTick = now;
    }
}

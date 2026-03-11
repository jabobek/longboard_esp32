/*
 * Header Guards
 */
#pragma once

/*
 * Includes
 */
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <WebServer.h>
#include <Preferences.h>
#include <math.h>
#include <Update.h>
#include <Wire.h>
#include "globals.h"

#if BOARD_ID == 1
#include <Adafruit_INA219.h>
#endif

#include <U8g2lib.h>

/*
 * Assertions
 */
#ifndef BOARD_ID
  #error "BOARD_ID is not defined! Check platformio.ini"
#endif

/*
 * Pin Definitions
 */
#define PIN_ESC_BRK_1 27
#define PIN_ESC_BRK_2 14
#define PIN_ESC_BRK_3 12
#define PIN_ESC_BRK_4 13
#define PIN_ESC_DIR   23
#define PIN_ESC_INV_DIR 5
#define PIN_ESC_LIGHT 18
#define PIN_ESC_BAT   35
#define PIN_ESC_NTC_B 34
#define PIN_ESC_NTC_E 39
#define PIN_M1 32
#define PIN_M2 33
#define PIN_M3 25
#define PIN_M4 26
#define PIN_HALL_FL 15
#define PIN_HALL_FR 4
#define PIN_HALL_RL 16
#define PIN_HALL_RR 17

#define INA219_I2C_ADDRESS 0x40

#define PIN_JOY_X       34
#define PIN_REM_BRK_ANA 32
#define PIN_BTN_BRK     26
#define PIN_BTN_DIR     27
#define PIN_BTN_LGT     25
#define PIN_REM_BAT     35

#define OLED_SDA 21
#define OLED_SCL 22

#if BOARD_ID == 1
#define PIN_CHG 19
#elif BOARD_ID == 2
#define PIN_CHG 33
#endif

/*
 * External Object and Global Variable Declarations
 */
extern uint8_t targetMac[];
extern uint8_t broadcastMac[];
extern uint8_t myMacAddr[6];
extern uint8_t connectedRemoteMac[6];

extern WebServer server;
extern Preferences prefs;

#if BOARD_ID == 2
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
#endif

extern int   activeProfile;
extern int   cfg_deadzone; extern int   cfg_deadzoneMax;
extern int   cfg_brakeDeadzone; extern int cfg_brakeDeadzoneMax;
extern int   cfg_fsMs;
extern uint8_t cfg_fsBrakeMask;
extern bool  cfg_fsLight;
extern bool  cfg_autoOffEn; extern int   cfg_autoOffMins;
extern bool  cfg_darkMode;

extern float cfg_whFL; extern float cfg_whFR;
extern float cfg_whRL; extern float cfg_whRR;

extern float cfg_voltsScale;
extern float cfg_amps_scale_ina219;
extern float cfg_amps_offset_ina219;
extern bool  cfg_autoReset; extern bool reqResetTrip;
extern int   cfg_debounceMs; extern int   cfg_gaugeMax;
extern int   cfg_seriesCells;

extern float cfg_remBatScale; extern float cfg_remRuntimeHrs;
extern int   cfg_remSeriesCells;
extern int   remBatCycles; extern bool remBatFullFlag;

extern float cfg_curveBat[11]; extern float cfg_curveRem[11];

extern bool  cfg_pidEn;
extern float cfg_Kp; extern float cfg_Ki; extern float cfg_Kd;

extern bool  cfg_imperial; extern bool cfg_fahrenheit;
extern float cfg_betaBat; extern float cfg_betaEsc;

extern float cfg_brakeCurve;
extern uint8_t cfg_analogBrakeMask[5];
extern uint8_t cfg_buttonBrakeMask;

extern int cfg_pulsesPerRev;

extern bool cfg_inv_brk1; extern bool cfg_inv_brk2; extern bool cfg_inv_brk3; extern bool cfg_inv_brk4;
extern bool cfg_inv_dir;
extern bool cfg_inv_light;
extern bool cfg_inv_pwm;

extern bool cfg_isCharging;

extern unsigned long lastTick; extern unsigned long lastDraw;
extern unsigned long sendStartTime; extern volatile long lastLatency;
extern bool lastPacketStatus; extern unsigned long lastActivity;
extern bool lightState;

extern int lastStateBrk; extern unsigned long lastTimeBrk;
extern int lastStateDir; extern unsigned long lastTimeDir;
extern int lastStateLgt; extern unsigned long lastTimeLgt;
extern bool lgtBtnProcessed; extern unsigned long lgtPressStart;

extern volatile unsigned long pcFL; extern volatile unsigned long pcFR;
extern volatile unsigned long pcRL; extern volatile unsigned long pcRR;
extern unsigned long lastRecvTime;
extern unsigned long lastSaveTime;
extern float currentKmh;
extern float valDistTotal; extern float valDistTrip; extern float valDistSession;
extern int batCycles; extern bool batFullFlag; extern bool batFullResetDone;
extern int escFsMs; extern bool escFsLight; extern bool escIsConnected;
extern uint8_t escFsMask;
extern float pidErrSum[4]; extern float pidLastErr[4];

extern float maxTempBat; extern float maxTempEsc;
extern float maxAmpsSession; extern float maxPowerSession;

extern unsigned long pulsesAccumulated[4];
extern unsigned long lastSpeedMeasureTime;
extern unsigned long failsafeTriggerTime;

extern float remVoltageSmooth; extern int remBatPct;

extern bool isPairingMode;
extern bool pairingLocked;

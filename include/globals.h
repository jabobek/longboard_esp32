/*
 * Header Guards
 */
#ifndef GLOBALS_H
#define GLOBALS_H

/*
 * Includes
 */
#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include <U8g2lib.h>
#include <esp_now.h>
#include <WiFi.h>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <Update.h>

/*
 * Definitions
 */
#define FW_VERSION "2.1"
#define WIFI_CHANNEL 1
#define PAIRING_MAGIC 0x42

/*
 * Data Structures
 */
struct DriveProfile {
  int maxPower;
  float speedLimit;
  bool enableSpeedLimit;
  float curve;
  int m1, m2, m3, m4;
  float estRangeKm;
};

typedef struct packet_cmd {
  int throttle;
  int brakeAnalog;
  bool brakeBtn;
  bool dir;
  bool light;
  int cfgMasterPower;
  float cfgSpeedLimit;
  bool cfgEnableSpeedLimit;
  float cfgCurve;
  int powerM1; int powerM2; int powerM3; int powerM4;
  bool pidEn; float Kp; float Ki; float Kd;
  int cfgFailsafeMs;
  bool cfgFailsafeLight;
  uint8_t cfgFailsafeBrakeMask;
  float cfgBrakeCurve;
  uint8_t cfgAnalogBrakeMask[5];
  uint8_t cfgButtonBrakeMask;
  float whFL; float whFR; float whRL; float whRR;
  float cfgVoltsScale;
  float cfgIna219Scale;
  float cfgIna219Offset;
  int cfgSeriesCells;
  int cfgPulsesPerRev;
  bool  cfgAutoResetTrip; bool cmdResetTrip;
  float cfgBetaBat; float cfgBetaEsc;
  bool  cfgImperial; bool cfgFahrenheit;
  bool isCharging;

  bool cfg_inv_brk1; bool cfg_inv_brk2; bool cfg_inv_brk3; bool cfg_inv_brk4;
  bool cfg_inv_dir;
  bool cfg_inv_light;
  bool cfg_inv_pwm;
  bool cmdResetConsumed;
  bool cmdResetCharging;
  bool cmdDisablePwmOnLowBat;
  float batCurve[11];
} packet_cmd;

typedef struct packet_telemetry {
  float voltage; float current; int batPct; int batCycles;
  float speed;
  float distTotal; float distTrip; float distSession;
  float tBat; float tEsc;
  bool isCharging;
  float WhConsumed;
  float mAhConsumed;
  float WhCharging;
  float mAhCharging;
} packet_telemetry;

typedef struct packet_pairing {
  uint8_t magic;
  uint8_t type;
  uint8_t macAddr[6];
} packet_pairing;

#define LOG_DURATION_MINS 15
#define LOG_FREQUENCY_HZ 1
#define LOG_BUFFER_SIZE (LOG_DURATION_MINS * 60 * LOG_FREQUENCY_HZ)

struct LogEntry {
  unsigned long timestamp;
  float speed;
  float power;
  float tempEsc;
  float tempBat;
  float voltage;
};

/*
 * External Object and Global Variable Declarations
 */
extern float defaultCurve[11];
extern DriveProfile profiles[3];
extern packet_cmd myCmd;
extern packet_telemetry myTelem;

extern uint8_t targetMac[];
extern uint8_t broadcastMac[];
extern uint8_t myMacAddr[6];
extern uint8_t connectedRemoteMac[6];

#if BOARD_ID == 2
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
#endif

extern WebServer server;
extern Preferences prefs;

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
extern bool  cfg_autoReset; extern bool reqResetTrip;
extern bool  cfg_disablePwmOnLowBat;
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
extern float valWhConsumed;
extern float valmAhConsumed;
extern float valWhCharging;
extern float valmAhCharging;
extern unsigned long lastWhCalcTime;
extern int batCycles; extern bool batFullFlag; extern bool batFullResetDone;
extern bool escIsConnected;
extern float pidErrSum[4]; extern float pidLastErr[4];

extern float maxTempBat; extern float maxTempEsc;
extern float maxAmpsSession; extern float maxPowerSession;

extern unsigned long pulsesAccumulated[4];
extern unsigned long lastSpeedMeasureTime;
extern unsigned long failsafeTriggerTime;
extern unsigned long lastTelemetryRecvTime;

extern float remVoltageSmooth; extern int remBatPct;

extern bool isPairingMode;
extern bool pairingLocked;

#endif

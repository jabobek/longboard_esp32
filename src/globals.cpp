/* Include project configuration */
#include "config.h"

/* Battery discharge curve and drive profiles */
float defaultCurve[11] = {3.00, 3.25, 3.40, 3.55, 3.65, 3.72, 3.80, 3.88, 3.98, 4.08, 4.20};

DriveProfile profiles[3] = {
  {40, 30.0, true, 3.0, 80, 80, 80, 80, 20.0},
  {80, 30.0, true, 1.5, 100, 100, 100, 100, 15.0},
  {100, 30.0, true, 1.0, 100, 100, 100, 100, 10.0}
};

/* Communication packets */
packet_cmd myCmd;
packet_telemetry myTelem;

/* MAC addresses and communication globals */
uint8_t targetMac[]    = {0xD0, 0xEF, 0x76, 0x5D, 0xC3, 0x6C};
uint8_t broadcastMac[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t myMacAddr[6];
uint8_t connectedRemoteMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/* Hardware objects and server instances */
#if BOARD_ID == 2
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R1, /* reset=*/ U8X8_PIN_NONE);
#endif

WebServer server(80);
Preferences prefs;

/* Configuration and state variables */
int   activeProfile = 1;
int   cfg_deadzone = 250; int   cfg_deadzoneMax = 3900;
int   cfg_brakeDeadzone = 250; int cfg_brakeDeadzoneMax = 3900;
int   cfg_fsMs = 1000;
uint8_t cfg_fsBrakeMask = 15;
bool  cfg_fsLight = true;
bool  cfg_autoOffEn = true; int   cfg_autoOffMins = 5;
bool  cfg_darkMode = true;
bool  cfg_disablePwmOnLowBat = true;

float cfg_whFL = 25.4; float cfg_whFR = 25.4;
float cfg_whRL = 25.4; float cfg_whRR = 25.4;

float cfg_voltsScale = 66.6;
float cfg_amps_scale_ina219 = 1.0;
float cfg_amps_offset_ina219 = 0.0;
bool  cfg_autoReset = false; bool reqResetTrip = false;
int   cfg_debounceMs = 50; int   cfg_gaugeMax = 40;
int   cfg_seriesCells = 14;

float cfg_remBatScale = 2.0; float cfg_remRuntimeHrs = 5.0;
int   cfg_remSeriesCells = 1;
int   remBatCycles = 0; bool remBatFullFlag = false;

float cfg_curveBat[11]; float cfg_curveRem[11];

bool  cfg_pidEn = false;
float cfg_Kp = 2.5; float cfg_Ki = 0.1; float cfg_Kd = 0.5;

bool  cfg_imperial = false; bool cfg_fahrenheit = false;
float cfg_betaBat = 3950.0; float cfg_betaEsc = 3950.0;

float cfg_brakeCurve = 1.0;
uint8_t cfg_analogBrakeMask[5] = {0, 1, 3, 7, 15};
uint8_t cfg_buttonBrakeMask = 15;

int cfg_pulsesPerRev = 15;

bool cfg_inv_brk1 = false; bool cfg_inv_brk2 = false; bool cfg_inv_brk3 = false; bool cfg_inv_brk4 = false;
bool cfg_inv_dir = false;
bool cfg_inv_light = false;
bool cfg_inv_pwm = false;

bool cfg_isCharging = false;

/* Timing and activity monitoring */
unsigned long lastTick = 0; unsigned long lastDraw = 0;
unsigned long sendStartTime = 0; volatile long lastLatency = 0;
bool lastPacketStatus = false; unsigned long lastActivity = 0;
bool lightState = false;

/* Button and input state tracking */
int lastStateBrk = HIGH; unsigned long lastTimeBrk = 0;
int lastStateDir = HIGH; unsigned long lastTimeDir = 0;
int lastStateLgt = HIGH; unsigned long lastTimeLgt = 0;
bool lgtBtnProcessed = false; unsigned long lgtPressStart = 0;

/* Sensor data and calculated values */
volatile unsigned long pcFL = 0; volatile unsigned long pcFR = 0;
volatile unsigned long pcRL = 0; volatile unsigned long pcRR = 0;
unsigned long lastRecvTime = 0;
unsigned long lastSaveTime = 0;
float currentKmh = 0.0;
float valDistTotal = 0.0; float valDistTrip = 0.0; float valDistSession = 0.0;
float valWhConsumed = 0.0;
float valmAhConsumed = 0.0;
float valWhCharging = 0.0;
float valmAhCharging = 0.0;
unsigned long lastWhCalcTime = 0;
int batCycles = 0; bool batFullFlag = false; bool batFullResetDone = false;
int escFsMs = 1000; bool escFsLight = true; bool escIsConnected = true;
uint8_t escFsMask = 15;
float pidErrSum[4] = {0}; float pidLastErr[4] = {0};

/* Maximum values and accumulated data */
float maxTempBat = -100.0; float maxTempEsc = -100.0;
float maxAmpsSession = 0.0; float maxPowerSession = 0.0;

unsigned long pulsesAccumulated[4] = {0,0,0,0};
unsigned long lastSpeedMeasureTime = 0;
unsigned long failsafeTriggerTime = 0;
unsigned long lastTelemetryRecvTime = 0;

/* Remote status and pairing */
float remVoltageSmooth = 0.0; int remBatPct = 0;
bool isPairingMode = false;
bool pairingLocked = false;

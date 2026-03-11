/* Include utility and configuration headers */
#include "utils.h"
#include "config.h"

/* MAC address to string conversion */
String macToString(const uint8_t* mac) {
  char buf[18];
  sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

/* NTC thermistor reading and temperature calculation */
float readNTC(int pin, float beta) {
  int adc = analogRead(pin);
  if (adc < 50 || adc > 4050) return 0.0;
  float R = 10000.0 / (4095.0 / (float)adc - 1.0);
  float T = 1.0 / (1.0 / 298.15 + log(R / 10000.0) / beta);
  return T - 273.15;
}

/* Safe float to string conversion handling NaN and Inf */
String safeFloat(float val, int dec) {
    if (isnan(val) || isinf(val)) return "0.0";
    return String(val, dec);
}

/* Debounced button state reading */
bool readButtonDebounce(int pin, int &lastState, unsigned long &lastTime) {
  int reading = digitalRead(pin);
  if (reading != lastState) lastTime = millis();
  lastState = reading;
  if ((millis() - lastTime) > cfg_debounceMs) return (reading == LOW);
  return false;
}

/* Battery percentage calculation from voltage and discharge curve */
int getBatPct(float totalVolt, float* curve, float maxVolt) {
  int s = round(maxVolt / 4.2);
  if (s < 1) s = 1;
  float vCell = totalVolt / (float)s;

  if (vCell <= curve[0]) return 0;
  if (vCell >= curve[10]) return 100;

  for (int i = 0; i < 10; i++) {
    if (vCell >= curve[i] && vCell <= curve[i+1]) {
      float range = curve[i+1] - curve[i];
      float diff = vCell - curve[i];
      return (i * 10) + (int)((diff / range) * 10.0);
    }
  }
  return 0;
}

/* Float array to comma-separated string conversion */
void arrayToString(float* arr, char* buf) {
  buf[0] = '\0';
  for(int i=0; i<11; i++) {
    char temp[10]; sprintf(temp, "%.2f", arr[i]);
    strcat(buf, temp);
    if(i<10) strcat(buf, ",");
  }
}

/* Comma-separated string to float array conversion */
void stringToArray(String s, float* arr) {
  int idx = 0;
  int lastP = 0;
  for(int i=0; i<11; i++) {
    int p = s.indexOf(',', lastP);
    if(p == -1) p = s.length();
    arr[idx++] = s.substring(lastP, p).toFloat();
    lastP = p + 1;
  }
}

/* Interrupt Service Routines for wheel speed pulses */
void IRAM_ATTR isrFL() { pcFL++; }
void IRAM_ATTR isrFR() { pcFR++; }
void IRAM_ATTR isrRL() { pcRL++; }
void IRAM_ATTR isrRR() { pcRR++; }

/* Include logging header */
#include "log.h"

/* Global log buffer and indexing variables */
LogEntry logBuffer[LOG_BUFFER_SIZE];
int logIndex = 0;
bool bufferFull = false;
unsigned long lastLogTime = 0;

/* Initialize the logging system */
void setupLogging() {
  clearLogs();
}

/* Add a new entry to the circular log buffer */
void addLogEntry(float speed, float power, float tempEsc, float tempBat, float voltage) {
  unsigned long now = millis();
  
  if (now - lastLogTime < (1000 / LOG_FREQUENCY_HZ)) {
    return;
  }
  lastLogTime = now;

  logBuffer[logIndex].timestamp = now;
  logBuffer[logIndex].speed = speed;
  logBuffer[logIndex].power = power;
  logBuffer[logIndex].tempEsc = tempEsc;
  logBuffer[logIndex].tempBat = tempBat;
  logBuffer[logIndex].voltage = voltage;

  logIndex++;

  if (logIndex >= LOG_BUFFER_SIZE) {
    logIndex = 0;
    bufferFull = true;
  }
}

/* Clear all entries from the log buffer */
void clearLogs() {
  logIndex = 0;
  bufferFull = false;
  memset(logBuffer, 0, sizeof(logBuffer));
}

/* Retrieve log data formatted as a JSON string */
String getLogsAsJson() {
  String json = "[";
  
  int count = bufferFull ? LOG_BUFFER_SIZE : logIndex;
  
  for (int i = 0; i < count; i++) {
    int currentIndex = 0;
    if (bufferFull) {
        currentIndex = (logIndex + i) % LOG_BUFFER_SIZE;
    } else {
        currentIndex = i;
    }

    json += "{";
    json += "\"t\":" + String(logBuffer[currentIndex].timestamp / 1000.0, 1) + ",";
    json += "\"s\":" + String(logBuffer[currentIndex].speed, 1) + ",";
    json += "\"p\":" + String(logBuffer[currentIndex].power, 0) + ",";
    json += "\"te\":" + String(logBuffer[currentIndex].tempEsc, 1) + ",";
    json += "\"tb\":" + String(logBuffer[currentIndex].tempBat, 1) + ",";
    json += "\"v\":" + String(logBuffer[currentIndex].voltage, 1);
    json += "}";

    if (i < count - 1) {
      json += ",";
    }
  }

  json += "]";
  return json;
}

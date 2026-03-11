/*
 * Header Guards
 */
#pragma once

/*
 * Includes
 */
#include "config.h"

/*
 * Function Declarations
 */
void setupLogging();
void addLogEntry(float speed, float power, float tempEsc, float tempBat, float voltage);
void clearLogs();
String getLogsAsJson();

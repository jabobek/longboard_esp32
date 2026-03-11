#pragma once

/* Include necessary headers */
#include "config.h"

/* Web server function declarations */
void setupOTA();
void handleData();
void handleRoot();
void handleSettings();
void handleSave();
void handleChartPage();
void handleLogData();
void handleResetLogs();
void setupWebServer();

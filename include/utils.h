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
String macToString(const uint8_t* mac);
float readNTC(int pin, float beta);
String safeFloat(float val, int dec);
bool readButtonDebounce(int pin, int &lastState, unsigned long &lastTime);
int getBatPct(float totalVolt, float* curve, float maxVolt);
void arrayToString(float* arr, char* buf);
void stringToArray(String s, float* arr);
void IRAM_ATTR isrFL();
void IRAM_ATTR isrFR();
void IRAM_ATTR isrRL();
void IRAM_ATTR isrRR();

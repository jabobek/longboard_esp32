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
void updatePeer(const uint8_t* macAddr);
void startPairing();
void OnDataRecv(const uint8_t * mac, const uint8_t *in, int len);
void OnDataSent(const uint8_t *mac, esp_now_send_status_t status);

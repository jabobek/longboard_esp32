/* Include communication, storage, and project headers */
#include "communication.h"
#include "storage.h"
#include "config.h"

/* Add or update an ESP-NOW peer in the peer list */
void updatePeer(const uint8_t* macAddr) {
  if (esp_now_is_peer_exist(macAddr)) { return; }
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, macAddr, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

/* Initialize and broadcast a pairing request */
void startPairing() {
    isPairingMode = true;
    packet_pairing req;
    req.magic = PAIRING_MAGIC;
    req.type = 1;
    WiFi.macAddress(req.macAddr);
    esp_now_send(broadcastMac, (uint8_t *) &req, sizeof(req));
}

/* ESP-NOW callback for received data handling pairing and telemetry */
void OnDataRecv(const uint8_t * mac, const uint8_t *in, int len) {
  if (len == sizeof(packet_pairing)) {
      packet_pairing rxPair;
      memcpy(&rxPair, in, sizeof(rxPair));
      
      if (rxPair.magic == PAIRING_MAGIC) {
          if (BOARD_ID == 1 && rxPair.type == 1) {
              if (currentKmh < 1.0 && !pairingLocked) { 
                  updatePeer(rxPair.macAddr);
                  packet_pairing resp;
                  resp.magic = PAIRING_MAGIC;
                  resp.type = 2;
                  memcpy(resp.macAddr, myMacAddr, 6);
                  esp_now_send(rxPair.macAddr, (uint8_t *) &resp, sizeof(resp));
              }
          }
          else if (BOARD_ID == 2 && rxPair.type == 2 && isPairingMode) {
              memcpy(targetMac, rxPair.macAddr, 6);
              saveRemoteSettings();
              updatePeer(targetMac);
              isPairingMode = false;
          }
      }
      return;
  }

  if (BOARD_ID == 1 && len == sizeof(myCmd)) { 
    if(!pairingLocked) {
        pairingLocked = true;
        memcpy(connectedRemoteMac, mac, 6);
        updatePeer(connectedRemoteMac);
    } else {
        if (memcmp(connectedRemoteMac, mac, 6) != 0) return;
    }

    memcpy(&myCmd, in, sizeof(myCmd));
    lastRecvTime = millis();
    
    escFsMs = myCmd.cfgFailsafeMs;
    escFsMask = myCmd.cfgFailsafeBrakeMask;
    escFsLight = myCmd.cfgFailsafeLight;
  } 
  else if (BOARD_ID == 2 && len == sizeof(myTelem)) { 
    memcpy(&myTelem, in, sizeof(myTelem));
    lastTelemetryRecvTime = millis();
  }
}

/* ESP-NOW callback for data transmission status and latency tracking */
void OnDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  if (micros() > sendStartTime) lastLatency = (long)((micros() - sendStartTime) / 1000);
  lastPacketStatus = (status == ESP_NOW_SEND_SUCCESS);
}

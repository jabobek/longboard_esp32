/* Include project-wide configuration and module headers */
#include "config.h"
#include "communication.h"
#include "board_controller.h"
#include "board_remote.h"
#include "web_server.h"

/* System initialization and hardware setup */
void setup() {
  Serial.begin(115200);
  memset(&myCmd, 0, sizeof(myCmd));
  memset(&myTelem, 0, sizeof(myTelem));
  WiFi.macAddress(myMacAddr);

  if (BOARD_ID == 1) {
    setupController();
  } else if (BOARD_ID == 2) {
    setupRemote();
  }
  
  setupWebServer();

  if (esp_now_init() != ESP_OK) Serial.println("ESP-NOW Error");
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_register_send_cb(OnDataSent);
  
  updatePeer(BOARD_ID == 1 ? broadcastMac : targetMac); 
}

/* Main execution loop */
void loop() {
  if (BOARD_ID == 1) {
    loopController();
  } else if (BOARD_ID == 2) {
    loopRemote();
  }
}

#include <espnow.h>
#include <ESP8266WiFi.h>

struct __attribute__((packed)) dataPacket {
  uint32_t buttonID;
  float battery;
} packet;


uint32_t btnID;
float batteryPer;

bool received = false;

void OnDataRecv(uint8_t* mac, uint8_t* incomingData, uint8_t len) {
  // callback function that will be executed when data is received
  dataPacket packet;
  memcpy(&packet, incomingData, sizeof(packet));
  btnID = packet.buttonID;
  batteryPer = packet.battery;

  received = true;
  Serial.println(btnID);
  Serial.println(batteryPer);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  //0 is used instead of ESP_OK as apparently it isn't defined in the header
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop(){
  if (received == true){
    Serial.println(btnID);
    Serial.println(batteryPer);
  }
  received = false;
}
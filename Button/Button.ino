#include <espnow.h>
#include <ESP8266WiFi.h>

int btnId = 3;

ADC_MODE(ADC_VCC);

// Set your slave device's MAC Address
uint8_t slaveMacAddress[] = { 0x80, 0x7D, 0x3A, 0x1D, 0x7D, 0x59 };

// Define the structure for the data to be transmitted
struct transmitData {
  uint32_t buttonID;
  float battery;  // Using uint16_t instead of float for battery voltage
} data;

// Callback function to handle the result of data transmission
void OnDataSent(uint8_t* mac_addr, uint8_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  if (status == 0) {
    Serial.println("Delivery Success");
  } else {
    Serial.println("Delivery Failed");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);

  // Initializing ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
  }

  // Setting this device as the controller
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Registering the callback function for data transmission
  esp_now_register_send_cb(OnDataSent);

  // Adding the slave device as a peer
  esp_now_add_peer(slaveMacAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  data.buttonID = btnId;
  data.battery = ((float)ESP.getVcc() / 1024.0);
}

void loop() {
  // Send the data
  if (millis() < 200){
    esp_now_send(slaveMacAddress, (uint8_t*)&data, sizeof(data));
    delay(50);
  } else {
    delay(2000);
    Serial.println("Putting to Sleep");
    ESP.deepSleep(0);
    Serial.println("I don't wanna sleep");
  }
}
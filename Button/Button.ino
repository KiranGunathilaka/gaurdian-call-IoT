#include <esp_now.h>
#include <WiFi.h>
#include "Preferences.h"

#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "Firebase_API_key"
#define DATABASE_URL "FireBase_database_link"

String path = "100001/Buttons/01";  //Device ID of 100001  and   button id of 01

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

const int btnPin = 4;
bool btnState;

Preferences preferences;

String ssid;
String password;
String haveReceived;

const char* ssidMem = "ssid";
const char* passwordMem = "password";
const char* credsReceived = "0";
//this mac 48:E7:29:A3:7B:18

struct __attribute__((packed)) dataPacket {
  String id;
  String pwd;
} packet;

bool isReceivedOkay = false;

void OnDataRecv(const uint8_t* mac, const uint8_t* incomingData, int len) {
  // callback function that will be executed when data is received
  dataPacket packet;
  memcpy(&packet, incomingData, sizeof(packet));
  ssid = packet.id;
  password = packet.pwd;
  isReceivedOkay = true;
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  preferences.begin("wifiCreds", false);

  ssid = preferences.getString(ssidMem, "");  //search for stored ssid and pwd in the preferences and return "" found none
  password = preferences.getString(passwordMem, "");
  haveReceived = preferences.getString(credsReceived, "0");


  // Set device as a Wi-Fi Station
  if (haveReceived == "0") {
    WiFi.mode(WIFI_STA);
    // Init ESP-NOW
    ssid = "";
    password ="";

    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }
    Serial.println("Waiting to connect to Master");
    esp_now_register_recv_cb(OnDataRecv);

    // byte count = 0;
    // while (!isReceivedOkay && count<30){
    //   delay(1000);
    //   count++;
    // }

    while (ssid =="" && password =="") {
    }
    Serial.println(ssid);

    preferences.putString(credsReceived, "1");
    preferences.putString(ssidMem, ssid);
    preferences.putString(passwordMem, password);
    ESP.restart();
  }

  WiFi.begin(ssid, password);

  byte attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    attempts++;
    Serial.print(".");
    if (attempts > 20) {
      Serial.print("Can't connect to ");
      Serial.print(ssid);
      preferences.putString(credsReceived, "0");
      ESP.restart();
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected.");
  }
  Serial.println(WiFi.status());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up.
  Serial.print("Sign up new user... ");
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  // Assign the callback function for the long running token generation task.
  config.token_status_callback = tokenStatusCallback;  //--> see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(btnPin, INPUT_PULLDOWN);
}


void loop() {

  btnState = digitalRead(btnPin);

  if (WiFi.status() != 3) {
    //preferences.putString(credsReceived, "0");
    delay(1000);
    Serial.println(WiFi.status());
  }

  if (Firebase.ready() && signupOK && btnState) {
    preferences.putString(credsReceived, "1");
    // Write an Int number on the database path test/random_Float_Val.
    if (Firebase.RTDB.setInt(&fbdo, path, 1)) {
      Serial.println("PASSED");
      delay(1000);
      Firebase.RTDB.setInt(&fbdo, path, 0);
      btnState = false;
    } else {
      Serial.println("Failed: " + fbdo.errorReason());
    }
  }
}
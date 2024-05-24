#include "WiFi.h"
#include "time.h"
#include "Preferences.h"
#include <esp_now.h>

#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#include "SPI.h"
#include <TFT_eSPI.h> // Hardware-specific library

TFT_eSPI tft = TFT_eSPI();       // Invoke custom library

#define API_KEY "Firebase_API_key"
#define DATABASE_URL "FireBase_database_link"

// here pins are assigned for SPI2 interface of ESP32 S3 Devkit 

String DeviceID = "gc100001";

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
const long sendDataIntervalMillis = 5000;
bool signupOK = false;

float store_random_Float_Val;
int store_random_Int_Val;

const char* ssidMem    = "ssid";
const char* passwordMem   = "password";  // this "ssid" , "password" are not just the values, they defined how these variables should have  stored in the preferences library
//Neme9s8i7s

Preferences preferences;

const char* ntpServer = "in.pool.ntp.org";
const long  gmtOffset_sec = 19800;//GMT+5:30
const int   daylightOffset_sec = 0;

struct __attribute__((packed)) dataPacket { 
  int btnID;
  bool btnState;
};

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  
  dataPacket packet;
  memcpy(&packet, incomingData, sizeof(packet));
  int btnID = (int)packet.btnID;
  bool btnState = (bool)packet.btnState;
  Serial.println(btnID);
  Serial.println(btnState, true);
}

void setup(){
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);

  targetTime = millis() + 1000;


  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  


  preferences.begin("Settings" , false);//opens a preferences library with the namespace "Settings"
  tft.begin();


  String ssid = preferences.getString(ssidMem, "");  //search for stored ssid and pwd in the preferences and return "" found none
  String password = preferences.getString(passwordMem  , "");

  bool isConnected;
  if (ssid == ""){
    isConnected = initialConnection();
    while (!isConnected){
      isConnected = initialConnection();
    };
  }
  else{
    isConnected = connectWifi(ssid , password);
    while (!isConnected){
      isConnected = initialConnection();
    };
  }

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);


  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up.
  Serial.print("Sign up new user... ");
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  // Assign the callback function for the long running token generation task.
  config.token_status_callback = tokenStatusCallback; //--> see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
}



void loop(){

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > sendDataIntervalMillis || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    
    // Write an Int number on the database path test/random_Float_Val.
    if (Firebase.RTDB.setFloat(&fbdo, "Device1/random_Float_Val", store_random_Float_Val)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
    
    // Write an Float number on the database path test/random_Int_Val.
    if (Firebase.RTDB.setInt(&fbdo, "Device1/random_Int_Val", store_random_Int_Val)) {
      Serial.println("PASSED");
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    printLocalTime();
  }
}



void printLocalTime(){
  struct tm timeinfo;

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  tft.println(&timeinfo, "%A, %B %d ");
  tft.println(&timeinfo, "%H:%M");
}



bool initialConnection(){
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  
  //remove when touch available
  tft.println("Enter the index of the wifi network you want to connect");
  tft.println("Nearby Networks");

  byte numNetworks = WiFi.scanNetworks(); //scan for wifi networks and return number of networks found

  for (int i = 0; i < numNetworks; ++i) {
    tft.print(i+1);
    tft.print(")  ");
    tft.println(WiFi.SSID(i)); //Iterate over WiFi object (global arduino core object) i times 
  }

  while(!Serial.available()){
    //this loop will halt until serail input is given
  }

  String input = Serial.readStringUntil('\n'); // Read input until newline character
  byte intInp = input.toInt(); 

  if(intInp < numNetworks+1){
    String inpSSID = WiFi.SSID(intInp-1);

    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 0);
    tft.print("Enter password of ");
    tft.println(inpSSID);

    while(!Serial.available()){
    //this loop will halt until password is given via serial
    }
    String inpPassword = Serial.readStringUntil('\n');

    bool isCon = connectWifi(inpSSID , inpPassword);
    return isCon;
  }
}



bool connectWifi(String id,String  pwd){
  // Connect to Wi-Fi
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.print("Connecting to ");
  tft.println(id);
  WiFi.begin(id.c_str(), pwd.c_str());

  byte attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    attempts++;
    tft.print(".");
    if (attempts > 20){
      tft.print("Can't connect to ");
      tft.print(id);
      tft.println(". Check your credentials");
      delay(2000);
      WiFi.disconnect(true);
      return false;
    }
  }

  if (WiFi.status() == WL_CONNECTED){
    tft.println("WiFi connected.");

    preferences.putString(ssidMem, id.c_str() ); //stores the successfully conneted wifi credentials on the preferences namespace named  settings
    preferences.putString(passwordMem, pwd.c_str() );
    return true;
  }
}


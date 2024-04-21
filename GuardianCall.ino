#include "WiFi.h"
#include "time.h"
#include "Preferences.h"

#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "Firebase_API_key"
#define DATABASE_URL "FireBase_database_link"

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

void setup(){
  Serial.begin(115200);
  preferences.begin("Settings" , false);//opens a preferences library with the namespace "Settings"

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
  delay(1000);
  printLocalTime();

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > sendDataIntervalMillis || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();

    int randNumber = random(15, 40);
    float f = (float)randNumber / 1.01;
    int i = (int(f*100));
    store_random_Float_Val = float(i) / 100;
    store_random_Int_Val = random(10, 99);

    Serial.print("Random Float_Val : ");
    Serial.println(store_random_Float_Val);
    Serial.print("Random Int_Val   : ");
    Serial.println(store_random_Int_Val);
    
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
  }
}



void printLocalTime(){
  struct tm timeinfo;

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d  %H:%M");
}



bool initialConnection(){
  Serial.println("Nearby Networks");
  byte numNetworks = WiFi.scanNetworks(); //scan for wifi networks and return number of networks found

  for (int i = 0; i < numNetworks; ++i) {
    Serial.print(i+1);
    Serial.print(")  ");
    Serial.println(WiFi.SSID(i)); //Iterate over WiFi object (global arduino core object) i times 
  }

  Serial.println("Enter the index of the wifi network you want to connect");

  while(!Serial.available()){
    //this loop will halt until serail input is given
  }

  String input = Serial.readStringUntil('\n'); // Read input until newline character
  byte intInp = input.toInt(); 

  if(intInp < numNetworks+1){
    String inpSSID = WiFi.SSID(intInp-1);

    Serial.print("Enter password of ");
    Serial.println(inpSSID);

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
  Serial.print("Connecting to ");
  Serial.println(id);
  WiFi.begin(id.c_str(), pwd.c_str());

  byte attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    attempts++;
    Serial.print(".");
    if (attempts > 20){
      Serial.print("Can't connect to ");
      Serial.print(id);
      Serial.println(". Check your credentials");
      WiFi.disconnect(true);
      return false;
    }
  }

  if (WiFi.status() == WL_CONNECTED){
    Serial.println("WiFi connected.");

    preferences.putString(ssidMem, id.c_str() ); //stores the successfully conneted wifi credentials on the preferences namespace named  settings
    preferences.putString(passwordMem, pwd.c_str() );
    return true;
  }
}
#include "WiFi.h"
#include "time.h"
#include "Preferences.h"

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

}

void loop(){
  delay(1000);
  printLocalTime();
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
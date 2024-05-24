#include <WiFi.h>
#include "time.h"
#include <Preferences.h>
#include <TFT_eSPI.h>
#include <FS.h>

#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define TX_PIN 41
#define RX_PIN 40

#define API_KEY "Firebase_API_key"
#define DATABASE_URL "FireBase_database_link"

String deviceID = "100001";
String path;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite pwdField = TFT_eSprite(&tft);

unsigned long milliStart = 0;

String symbol[8][12] = {
  { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=" },
  { "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]" },
  { "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "\\" },
  { "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", "`", " " },

  { "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+" },
  { "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}" },
  { "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "|" },
  { "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", "~", " " }
};


uint16_t calData[5] = { 396, 3350, 270, 3369, 7 };

const int buzzerPin = 5;
int pressTone = 2000;

unsigned long startTime;
unsigned long elapsedTime;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;


const char* ssidMem = "ssid";
const char* passwordMem = "password";  // this "ssid" , "password" are not just the values, they defined how these variables should have  stored in the preferences library
//Neme9s8i7s
const char* isConnected = "isConnected";

Preferences preferences;

const char* ntpServer = "in.pool.ntp.org";
const long gmtOffset_sec = 19800;  //GMT+5:30
const int daylightOffset_sec = 0;

int prevIntTime;
String msg;
bool buzzerOn;
byte buzzerCount;

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  //Initializing serial communication betweeen master ESP01s
  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

  pinMode(buzzerPin, OUTPUT);

  //initializing tft object and initial display
  tft.init();
  tft.setRotation(1);
  tft.setTouch(calData);
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);

  //opens a preferences library with the namespace "WiFiVars" search for stored ssid and pwd in the preferences and return "" found none
  preferences.begin("WiFiVars");
  String ssid = preferences.getString(ssidMem, "");
  String password = preferences.getString(passwordMem, "");
  String isCon = preferences.getString(isConnected, "0");  //this memorize whether the device was connected to a wifi before the last turn off

  welcomeScreen();

  if (isCon == "0") {
    initialConnection();
  } else {
    if (!connectWifi(ssid, password)) {
      ESP.restart();
    };
  }
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

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

  //interupt is caused by esp01s
  //attachInterrupt(digitalPinToInterrupt(triggerPin), serialInteruptHandler, RISING);
}


int hh, mm, day, month, monthDay, intTime;

void loop() {
  serialInteruptHandler();

  struct tm time = getLocalTime();

  hh = (int)time.tm_hour;
  mm = (int)time.tm_min;
  day = (int)time.tm_wday;
  month = (int)time.tm_mon;
  monthDay = (int)time.tm_mday;


  int intTime = hh * 100 + mm;

  if (WiFi.status() != 3) {
    Serial.println("WiFi Connection lost");

    byte input = wifiConnectionLost();  // Read touch input... 1 - new wifi, 2 wait
    if (input == 1) {
      String isCon = "0";
      preferences.putString(isConnected, isCon.c_str());
      ESP.restart();
    } else {
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
      tft.drawString("Restart if you want to", 160, 120, 1);
      tft.drawString("change networks", 160, 140, 1);
      delay(3000);
      while (WiFi.status() != 3) {
        clockNoAlarm(hh, mm, day, month, monthDay);
        tft.setTextColor(TFT_WHITE);
        tft.drawString("No Connection", 160, 200, 1);
        delay(20000);
      }
    }
  }

  if (prevIntTime != intTime) {
    prevIntTime = intTime;
    Serial.println(&time, "%A, %B %d %H:%M");

    path = deviceID + "/Alarms/" + day + "/" + intTime;
    if (Firebase.ready() && signupOK) {
      if (Firebase.RTDB.getString(&fbdo, path)) {
        msg = fbdo.stringData();
        Serial.println(msg);
        buzzerOn = true;
        buzzerCount = 0;
        alarmDisplay(hh, mm, msg);
      } else {
        if ((String)fbdo.errorReason() == "path not exist") {
          Serial.print("No alarms at : ");
          Serial.println(&time, "%H:%M");
          buzzerOn = false;
          clockNoAlarm(hh, mm, day, month, monthDay);
        }
      }
    }
  }

  if (prevIntTime == intTime && buzzerOn && buzzerCount < 20) {
    tone(buzzerPin, 1000);
    delay(400);
    tone(buzzerPin, 3000);
    delay(400);
    noTone(buzzerPin);
    delay(400);
    buzzerCount++;
  }
}

uint32_t btnID;
float battery;

//Callback used when it is need to send button press event to the firebase
void serialInteruptHandler() {
  //put a tone here
  if (Serial2.available()) {
    btnID = Serial2.readStringUntil('\n').toInt();
    Serial.println(btnID);
    battery = Serial2.readStringUntil('\n').toFloat();
    Serial.println(battery);

    String btnPathStatus = deviceID + "/Buttons/" + btnID + "/Status/";
    String btnPathBat = deviceID + "/Buttons/" + btnID + "/Battery/";

    if (Firebase.ready() && signupOK && btnID != 0) {
      // Write an btn status and battery percentage on the database path specified for the button ID.
      if (Firebase.RTDB.setInt(&fbdo, btnPathStatus, 1)) {
        Firebase.RTDB.setFloat(&fbdo, btnPathBat, battery);
        Serial.println("PASSED");
        tone(buzzerPin, 3000);
        delay(2000);
        noTone(buzzerPin);
        Firebase.RTDB.setInt(&fbdo, btnPathStatus, 0);
      } else {
        Serial.println("Failed: " + fbdo.errorReason());
      }
    } else if (btnID != 0) {
      Serial.print("Firebase Sign up error... SignUp status : ");
      Serial.println(signupOK);
    }
  }
}



//put a button namer prompts and function here


struct tm getLocalTime() {
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return {};
  }
  return timeinfo;
}


void initialConnection() {

  //remove when touch available
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(BL_DATUM);
  tft.drawString("Select a Network", 15, 20, 1);

  byte numNetworks = WiFi.scanNetworks();  //scan for wifi networks and return number of networks found

  //display this after sorting according to RSSI
  String temp;
  for (int i = 0; i < numNetworks; ++i) {
    temp = String(i + 1) + ") " + String(WiFi.SSID(i));
    tft.drawString(temp, 5, 40 + i * 20, 1);
    tft.drawRoundRect(2, 22 + i * 20, 318, 18, 5, TFT_CYAN);
    //Iterate over WiFi object (global arduino core object) i times
  }

  uint16_t x = 0, y = 0;  // To store the touch coordinates

  int pressedIndex = 0;
  while (pressedIndex == 0) {
    bool pressed = tft.getTouch(&x, &y);
    if (pressed) {
      for (int i = 1; i <= 11; i++) {
        if (y > i * 20 && y < (i + 1) * 20) {
          pressedIndex = i;
          tone(buzzerPin, pressTone);
          delay(100);
          noTone(buzzerPin);
          break;
        }
      }
    }
  }

  String inpSSID = WiFi.SSID(pressedIndex - 1);


  String inpPassword = getKeyboardOut(inpSSID);
  Serial.print("Final Password is : ");
  Serial.println(inpPassword);

  bool connectWifiResponse = connectWifi(inpSSID, inpPassword);

  if (connectWifiResponse) {
    preferences.putString(isConnected, "1");
  }
  ESP.restart();
}


bool connectWifi(String id, String pwd) {
  // Connect to Wi-Fi
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  String temp = "Connecting to " + id;
  tft.setTextDatum(MC_DATUM);
  tft.drawString(temp, 160, 120, 1);
  WiFi.begin(id.c_str(), pwd.c_str(), 1);

  byte attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    attempts++;
    Serial.print(".");
    //Serial.println(WiFi.status());
    if (attempts > 20) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_RED);
      temp = "Can't connect to " + id;
      tft.drawString(temp, 160, 120, 1);
      tft.drawString("Check your credentials", 160, 145, 1);
      preferences.putString(isConnected, "0");
      delay(2000);
      WiFi.disconnect(true);
      return false;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN);
    tft.drawString("WiFi Connected...", 160, 120, 2);
    preferences.putString(ssidMem, id.c_str());  //stores the successfully conneted wifi credentials on the preferences namespace named WiFiVars
    preferences.putString(passwordMem, pwd.c_str());
    return true;
  }
}

void welcomeScreen() {
  //Welcome Screen
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString("Guardian", 160, 60, 4);
  tft.drawString("Call", 160, 180, 4);
  delay(500);
  tft.drawString("<           >", 160, 120, 4);
  delay(500);
  tft.drawString(".", 120, 120, 4);
  delay(500);
  tft.drawString(".", 140, 120, 4);
  delay(500);
  tft.drawString(".", 160, 120, 4);
  delay(500);
  tft.drawString(".", 180, 120, 4);
  delay(500);
  tft.drawString(".", 200, 120, 4);
  delay(1000);
}


String clockNoAlarm(int hh, int mm, int day, int month, int monthDay) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.setTextDatum(MC_DATUM);
  String nowTime = analogTime(hh, mm);

  tft.drawString(nowTime, 160, 120, 7);

  String dayName;
  String monthName;
  String dayString;

  switch (day) {
    case 0:
      dayName = "Sunday";
      break;
    case 1:
      dayName = "Monday";
      break;
    case 2:
      dayName = "Tuesday";
      break;
    case 3:
      dayName = "Wednesday";
      break;
    case 4:
      dayName = "Thursday";
      break;
    case 5:
      dayName = "Friday";
      break;
    case 6:
      dayName = "Saturday";
      break;
  }

  switch (month) {
    case 0:
      monthName = "January";
      break;
    case 1:
      monthName = "February";
      break;
    case 2:
      monthName = "March";
      break;
    case 3:
      monthName = "April";
      break;
    case 4:
      monthName = "May";
      break;
    case 5:
      monthName = "June";
      break;
    case 6:
      monthName = "July";
      break;
    case 7:
      monthName = "August";
      break;
    case 8:
      monthName = "September";
      break;
    case 9:
      monthName = "October";
      break;
    case 10:
      monthName = "November";
      break;
    case 11:
      monthName = "December";
      break;
  }

  dayString = dayName + " , " + monthName + " " + String(monthDay);
  tft.drawString(dayString, 160, 40, 2);

  return nowTime;
}

void alarmDisplay(int hh, int mm, String msg) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_RED);
  tft.setTextDatum(MC_DATUM);

  String nowTime = analogTime(hh, mm);
  nowTime = "Alarm at " + nowTime;
  tft.drawString(nowTime, 160, 20, 2);

  int x = 7, y = 55;
  int length;
  String word = "";
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(TFT_WHITE);
  for (int i = 0; i < msg.length(); i++) {
    char currentChar = msg.charAt(i);
    word += currentChar;
    if (currentChar == ' ') {
      length = (int)word.length();
      if (x + 14 * (length - 1) > 320) {
        x = 7;
        y += 26;
      }
      tft.drawString(word, x, y, 2);
      x += 14 * (length);
      word = "";
    }
  }
  length = (int)word.length();
  if (x + 14 * (length - 1) > 320) {
    x = 7;
    y += 26;
  }
  tft.drawString(word, x, y, 2);
  x += 14 * (length);
  word = "";
}

String analogTime(int hh, int mm) {
  if (hh > 12) {
    hh = hh - 12;
  }

  String strhh;
  if (hh < 10) {
    strhh = "0" + String(hh);
  } else {
    strhh = String(hh);
  }
  String strmm;
  if (mm < 10) {
    strmm = "0" + String(mm);
  } else {
    strmm = String(mm);
  }
  String nowTime = strhh + ":" + strmm;
  return nowTime;
}

//ButtonConnectDisplay and ButtonConnectResponse removed... See Old code

int wifiConnectionLost() {
  // add a 20s sprite countdown

  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("WiFi disconnected...", 160, 80, 2);
  tft.fillRoundRect(55, 110, 200, 40, 10, 0x4B60);  //green
  tft.fillRoundRect(55, 170, 200, 40, 10, TFT_BLUE);
  tft.drawString("Connect New WiFi", 160, 130, 1);
  tft.drawString("Wait", 160, 190, 1);

  uint16_t x = 0, y = 0;  // To store the touch coordinates

  bool changePressed = false;
  bool waitPressed = false;
  startTime = millis();
  while (!(changePressed || waitPressed) && (millis() - startTime) < 20000) {
    bool pressed = tft.getTouch(&x, &y);
    if (pressed) {
      if (x > 55 && x < 255 && y > 110 && y < 150) {
        changePressed = true;
        tone(buzzerPin, pressTone);
        delay(100);
        noTone(buzzerPin);
        Serial.println("Connecting New");
        return 1;
      }
      if (x > 55 && x < 255 && y > 170 && y < 210) {
        waitPressed = true;
        tone(buzzerPin, pressTone);
        delay(100);
        noTone(buzzerPin);
        Serial.println("Waiting");
        return 2;
      }
    }
  }
  return 2;
}


String getKeyboardOut(String ssid) {
  uint16_t x = 0, y = 0;
  String pwd = "";
  String pressedKey;

  byte xIndex, yIndex;
  bool enterPressed = false;
  bool capsOn = false;

  pwdField.createSprite(315, 40);

  drawButtons(capsOn, ssid);

  milliStart = millis();

  while (!enterPressed) {
    bool pressed = tft.getTouch(&x, &y);
    if (pressed && (millis() - milliStart) > 400) {
      if (y > 85) {
        tone(buzzerPin, pressTone);
        delay(100);
        noTone(buzzerPin);
      }

      if (x < 55 && y > 201) {
        if (capsOn) {
          capsOn = false;
        } else {
          capsOn = true;
        }
        drawButtons(capsOn, ssid);
      }

      if (x > 55 && x < 193 && y > 201) {
        pressedKey = " ";
      }

      if (x > 193 && x < 249 && y > 201) {
        if (pwd != "") {
          pwd.remove(pwd.length() - 1);
          pwdField.fillSprite(TFT_WHITE);
        }
      }

      if ((x > 249 && y > 201) || (x > 287 && y > 172)) {
        if (enterPressed) {
          enterPressed = false;
        } else {
          enterPressed = true;
        }
      }

      if (y > 85 && y < 201) {
        if (capsOn) {
          xIndex = (int)(x - 1) / 26;
          yIndex = (int)(((y - 85)) / 29 + 4);
        } else {
          xIndex = (int)(x - 1) / 26;
          yIndex = (int)(((y - 85)) / 29);
        }
        pressedKey = String(symbol[yIndex][xIndex]);
      }

      milliStart = millis();

      pwd += pressedKey;

      Serial.println(pressedKey);
      Serial.println(pwd);

      pwdField.drawString(pwd, 10, 35, 4);
      pwdField.pushSprite(0, 40);

      pressedKey = "";
    }
  }

  return pwd;
}

void drawButtons(bool isCapsOn, String inpSSID) {
  tft.fillScreen(TFT_BLACK);
  String temp = "Enter password " + inpSSID;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(BL_DATUM);
  tft.drawString(temp, 5, 20, 1);

  pwdField.fillSprite(TFT_WHITE);
  pwdField.setRotation(1);
  pwdField.setTextDatum(BL_DATUM);
  pwdField.setTextColor(TFT_BLACK, TFT_WHITE);
  pwdField.pushSprite(0, 40);

  tft.setTextSize(2);
  tft.setTextColor(TFT_RED);
  if (!isCapsOn) {
    for (int j = 0; j < 4; j++) {
      for (int i = 0; i < 12; i++) {
        tft.fillRoundRect((i * 26) + 1, j * 29 + 85, 25, 28, 3, TFT_YELLOW);
        tft.setCursor(i * 26 + 6, j * 29 + 90);
        tft.print(symbol[j][i]);
      }
    }
  } else {
    for (int j = 4; j < 8; j++) {
      for (int i = 0; i < 12; i++) {
        tft.fillRoundRect((i * 26) + 1, (j - 4) * 29 + 85, 25, 28, 3, TFT_YELLOW);
        tft.setCursor(i * 26 + 6, (j - 4) * 29 + 90);
        tft.print(symbol[j][i]);
      }
    }
  }
  tft.fillRoundRect(1, 201, 55, 28, 3, TFT_CYAN);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(5, 211);
  tft.print("Caps");

  tft.fillRoundRect(57, 201, 135, 28, 3, TFT_CYAN);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(100, 211);
  tft.print("Space");

  tft.fillRoundRect(193, 201, 55, 28, 3, TFT_CYAN);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(198, 211);
  tft.print("Back");

  tft.fillRoundRect((11 * 26) + 1, (3) * 29 + 85, 25, 33, 3, TFT_CYAN);
  tft.fillRoundRect(249, 201, 63, 28, 3, TFT_CYAN);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(253, 211);
  tft.print("Enter");
}
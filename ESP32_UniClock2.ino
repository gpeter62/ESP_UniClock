/* 
 *    Universal Clock  (Nixie, VFD, LED, Numitron) for ESP8266 or ESP32
 *    with optional Dallas Thermometer and DS3231 RTC, Neopxels stripe, GPS and more...
 *    v2.2c  17/11/2020
 *    Copyright (C) 2020  Peter Gautier 
 *    
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License <http://www.gnu.org/licenses/> for more details.
 *    
 *    Always check the usable pins of 8266: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
 *               and usable pins for ESP32: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
 *    
 *    Web page files, wich are found in "dat" subfolder must upload to SPIFFS !
 *    ESP8266:  https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide/
 *    ESP32: https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
 */
//---------------------------- PROGRAM PARAMETERS -------------------------------------------------
#define DEBUG               //Enable Serial Monitor, 115200baud (only, if TX pin is not used anywhere!!!)
//#define USE_DALLAS_TEMP   //TEMP_SENSOR_PIN is used to connect the sensor, temperature measure
//#define USE_DHT_TEMP      //TEMP_SENSOR_PIN is used to connect the sensor,  temperature and humidity measure
//#define USE_RTC           //I2C pins are used!   SCL = D1 (GPIO5), SDA = D2 (GPIO4)
//#define USE_GPS           //use for standalone clock, without wifi internet access
#define USE_NEOPIXEL_MAKUNA   //WS2812B led stripe, for tubes lightning. Don't forget to define tubePixels[] !

#define MAXBRIGHTNESS 10  // (if MM5450, use 15 instead of 10)

//Use only 1 from the following options!
#define MULTIPLEX74141   //4..8 Nixie tubes generic driver for ESP8266 or ESP32
//#define MAX6921          //4..8 VFD tubes (IV18) driver for ESP8266 or ESP32

//ESP8266 only drivers
//#define NO_MULTIPLEX74141 //4..6 Nixie tubes, serial latch driver, 74141 for each tube 
//#define MM5450            //6..8 LEDS
//#define MAX7219CNG        //4..8 LED 
//#define Numitron_4511N      //Numitron 4x tube clock
//#define SN75512           //4..8 VFD tubes   
//#define samsung           //samsung serial display
//#define PCF_MULTIPLEX74141  //PCF pin expander for tube selection

#define COLON_PIN   -1        //Blinking Colon pin.  If not used, SET TO -1                 (redtube clock:2, IV16:16 Pinter:TX, oldClock: 2 or SDA)
#define TEMP_SENSOR_PIN -1     //DHT or Dallas temp sensor pin.  If not used, SET TO -1      (RX or any other free pin) (IV18clockGP: GPIO4)
#define LED_SWITCH_PIN -1     //external led lightning ON/OFF.  If not used, SET TO -1      (Pinter: 16)
#define DECIMALPOINT_PIN -1   //Nixie decimal point between digits. If not used, SET TO -1  (Pinter:16)
#define ALARMSPEAKER_PIN -1   //Alarm buzzer pin                                            (oldCLock: SCL, pcftube6Clock: 3, numitron: 1)
#define ALARMBUTTON_PIN -1    //Alarm switch off button pin 
//8266 Neopixel LEDstripe pin is always the RX pin!!!

//Display temperature and date in every minute between START..END seconds
#define ENABLE_CLOCK_DISPLAY true   //false, if no clock display is needed (for example: thermometer + hygrometer only)
#define TEMP_START  35
#define TEMP_END    40
#define HUMID_START 40
#define HUMID_END   45
#define DATE_START  45
#define DATE_END    50
#define ANIMSPEED   50  //Animation speed in millisec 
#define TEMP_CHARCODE 15    
#define GRAD_CHARCODE 16
#define PERCENT_CHARCODE 8

byte c_MinBrightness = 8; 
byte c_MaxBrightness = 255;
//--------------------------------------------------------------------------------------------------

#include <DNSServer.h>

#if defined(ESP8266)  
  char webName[] = "UniClock 2.2c";
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #include <ESP8266WiFi.h>
  #include <DNSServer.h>
  //#include <ESP8266mDNS.h>
  #include "ESPAsyncTCP.h"
  #include "FS.h"

#elif defined(ESP32)
  char webName[] = "ESP32UniClock 2.2c";
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""
  //#include <WiFi.h>
  #include <esp_wifi.h>
  #include <DNSServer.h>
  //#include <ESPmDNS.h>
  #include "AsyncTCP.h"
  #include "SPIFFS.h"

  hw_timer_t * ESP32timer = NULL;
  portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
#else
  #error "Board is not supported!"  
#endif

DNSServer dnsServer;
#include "ESPAsyncWebServer.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <TimeLib.h>
#include <Timezone.h>
#include <Wire.h>
#include <ESPAsyncWiFiManager.h>
#include <EEPROM.h>
#include "ArduinoJson.h"


#ifdef USE_NEOPIXEL_MAKUNA
  #include <NeoPixelBrightnessBus.h>  
#endif

extern void ICACHE_RAM_ATTR writeDisplay();
extern void writeDisplaySingle();
extern void setup_pins();
extern void clearTubes();
extern const int maxDigits;

const char* ssid = AP_NAME;  // Enter SSID here
const char* password = AP_PASSWORD;  //Enter Password here

AsyncWebServer server(80);
DNSServer dns;

#define BUFSIZE 10
byte digit[BUFSIZE];
byte newDigit[BUFSIZE];
byte oldDigit[BUFSIZE];
boolean digitDP[BUFSIZE];   //actual value to put to display
boolean digitsOnly = true;  //only 0..9 numbers are possible to display?
byte animMask[BUFSIZE];     //0 = no animation mask is used

boolean EEPROMsaving = false; //saving in progress - stop display refresh
#define MAGIC_VALUE 200
 
// 8266 internal pin registers
// https://github.com/esp8266/esp8266-wiki/wiki/gpio-registers
// example: https://github.com/mgo-tec/OLED_1351/blob/master/src/OLED_SSD1351.cpp


#ifdef DEBUG    //Macros are usually in all capital letters.
  #define DPRINT(...)       Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
  #define DPRINTLN(...)     Serial.println(__VA_ARGS__)   //DPRINT is a macro, debug print
  #define DPRINTBEGIN(...)  Serial.begin(__VA_ARGS__)     //DPRINTLN is a macro, debug print with new line
#else
  #define DPRINT(...)     //now defines a blank line
  #define DPRINTLN(...)   //now defines a blank line
  #define DPRINTBEGIN(...)   //now defines a blank line
#endif

#define PIN_OUT_SET PERIPHS_GPIO_BASEADDR + 4
#define PIN_OUT_CLEAR PERIPHS_GPIO_BASEADDR + 8

bool colonBlinkState = false;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 7200000); // Update time every two hours
IPAddress ip; 

// Set timezone rules.  Offsets set to zero, since they will be loaded from EEPROM
TimeChangeRule myDST = {"DST", Last, Sun, Mar, 2, 0};    
TimeChangeRule mySTD = {"STD", First, Sun, Nov, 2, 0};     
Timezone myTZ(myDST, mySTD);

boolean clockWifiMode = true;

byte useTemp = 0;   //number of temp sensors: 0,1,2
float temperature[2] = {0,0};
byte useHumid = 0;
float humid = 0;

//----------------- EEPROM addresses -------------------------------------------------------------------
const int EEPROM_addr = 0;

struct {
  int utc_offset =1;
  bool enableDST = true;           // Flag to enable DST (summer time...)
  bool set12_24 = true;            // Flag indicating 12 vs 24 hour time (false = 12, true = 24);
  bool showZero = true;            // Flag to indicate whether to show zero in the hour ten's place
  bool enableBlink = true;         // Flag to indicate whether center colon should blink
  int  interval = 15;              // prm.interval in minutes, with 0 = off
  bool enableAutoShutoff = true;   // Flag to enable/disable nighttime shut off
  byte dayHour = 8;                // start of daytime
  byte dayMin = 0;
  byte nightHour = 22;             // start of night time
  byte nightMin = 0;
  byte dayBright = MAXBRIGHTNESS;  // display daytime brightness
  byte nightBright = 5;            // display night brightness
  byte animMode = 2;               //0=no anim,  if 1 or 2 is used, animation, when a digit changes
  boolean alarmEnable = false;     //Yes or No
  byte alarmHour = 7;              //Alarm time
  byte alarmMin = 0;
  byte alarmPeriod = 15;           //Alarm length, sec 
  byte rgbEffect = 1;              //0=OFF, 1=FixColor
  byte rgbBrightness = 100;        // 0..255
  unsigned int  rgbFixColor = 150;          //0..255, 256 = white
  byte rgbSpeed = 50;              //0..255msec / step
  boolean rgbDir = false;          //false = right, true = left
  byte magic = MAGIC_VALUE;        //magic value, to check EEPROM version when starting the clock
} prm;


//-------------------------------------------------------------------------------------------------------
time_t prevTime = 0;
time_t protectTimer = 0;
bool displayON = true;
bool manualOverride = false;
bool initProtectionTimer = false;  // Set true at the top of the hour to synchronize protection timer with clock
bool decimalpointON = false;
bool alarmON = false;             //Alarm in progress
unsigned long alarmStarted = 0;   //Start timestamp millis()


void startTimer() {  
  #if defined(ESP8266)  
    timer1_attachInterrupt(writeDisplay);
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
    timer1_write(15000); 
    DPRINTLN("Starting 8266 timer...");
    
  #elif defined(ESP32)
    //  https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/
    ESP32timer = timerBegin(0, 80, true);  //set prescaler to 80 -> 1MHz signal, true = edge generated signal
    timerAttachInterrupt(ESP32timer, &writeDisplay, true);   
    timerAlarmWrite(ESP32timer, 10000, false);   //100millisec, no repeat
    timerAlarmEnable(ESP32timer);
  #else
    #error "Unknown controller board!"    
  #endif  
}

void stopTimer() {
  #if defined(ESP8266)  
    timer1_detachInterrupt();
  #elif defined(ESP32)
   // timerDetachInterrupt(ESP32timer);
  #endif
}

void configModeCallback (AsyncWiFiManager *myWiFiManager) {
  DPRINTLN("Switch to AP config mode...");
  DPRINTLN("To configure Wifi,  ");
  DPRINT("connect to Wifi network "); DPRINTLN(ssid);
  DPRINTLN("and open 192.168.4.1 in web browser");
  }

void clearDigits() {
  memset(oldDigit,10,sizeof(oldDigit));
  memset(digit,10,sizeof(digit));
  memset(newDigit,10,sizeof(newDigit));
  memset(digitDP,0,sizeof(digitDP));
}

void Fdelay(unsigned long d) {
  unsigned long dStart = millis();
  while((millis()-dStart)<d) {
    dnsServer.processNextRequest();
    //MDNS.update();
    enableDisplay(true,2000);
    doAnimationMakuna();
    alarmSound();
    writeDisplaySingle();
    yield();
  }
}

void enableDisplay(boolean flag, unsigned long timeout) {
  static unsigned long lastDisable = millis();  
  static boolean state = true;

#if defined(ESP32) || defined(PCF_MULTIPLEX74141)  //safety mode for slow multiplex hardvare to avoid crashing the flash
  if (flag) {
    if (!state && ((timeout ==0) || (millis()-lastDisable) > timeout) ) {
      state = true;
      EEPROMsaving = false;
      startTimer();
      DPRINTLN("Enable tubes");
    }
  }
  else {
    DPRINTLN("Disable tubes");
    if (state) {
      stopTimer();
      clearTubes();
    }
    state = false;
    EEPROMsaving = true;
    lastDisable = millis();
  }  
#else
  EEPROMsaving = !flag;
#endif  
}

void startWifiMode() {
    int count = 0;
    
    enableDisplay(false,0);
    DPRINTLN("Starting Clock in WiFi Mode!");
    WiFi.mode(WIFI_STA);
    #if defined(ESP32)
      esp_wifi_set_ps (WIFI_PS_NONE);
    #endif
    AsyncWiFiManager MyWifiManager(&server,&dns);
    MyWifiManager.setAPCallback(configModeCallback);
    for (int i=0;i<5;i++) {
      if(!MyWifiManager.autoConnect(AP_NAME,AP_PASSWORD)) {
        DPRINT("Retry to Connect:"); DPRINTLN(i);
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
        Fdelay(2000);
        WiFi.mode(WIFI_STA);
        if (i==4) {
          MyWifiManager.autoConnect(AP_NAME,AP_PASSWORD); // Default password is PASSWORD, change as needed
        }
      }
      else 
        break;
    }  //end for
    ip = WiFi.localIP();
    WiFi.setAutoReconnect(true);
    enableDisplay(true,0);
    DPRINTLN("Connecting to Time Server...");
    timeClient.begin();
    timeClient.forceUpdate();
    while (true) { 
      Fdelay(100);
      if (timeClient.update()) break;
      count ++; if (count>999) count = 1;
      writeIpTag(count);
      Fdelay(500); 
    }
}

void startStandaloneMode() {
    DPRINTLN("Starting Clock in Standalone Mode!");
    DPRINT("Clock's wifi SSID:"); DPRINTLN(ssid);
    DPRINTLN("IP: 192.168.4.1");
    IPAddress local_ip(192,168,4,1);
    IPAddress gateway(192,168,4,1);
    IPAddress subnet(255,255,255,0);
    WiFi.mode(WIFI_AP);
    
    boolean nwState;
    if (password != NULL) {
      nwState =  WiFi.softAP(ssid, password); 
    } 
    else {
      nwState =  WiFi.softAP(ssid);
    }
    delay(2000);   //info: https://github.com/espressif/arduino-esp32/issues/2025
    WiFi.softAPConfig(local_ip, gateway, subnet);
    DPRINT("AP status:"); DPRINTLN(nwState ? "Ready" : "Failed!");  //channel
    DPRINT("Mac address:"); DPRINTLN(WiFi.softAPmacAddress());
    ip = WiFi.softAPIP();
    DPRINT("SOFT_AP IP:"); DPRINTLN(WiFi.softAPIP());
 
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    dnsServer.start(53, "*", WiFi.softAPIP());
    //if (!MDNS.begin("uniclock", WiFi.softAPIP())) {
      //DPRINTLN(F("Error setting up MDNS responder!"));
    //}
    //MDNS.addService("http", "tcp", 80);
    enableDisplay(true,0);
    Fdelay(1000);
}


void startServer() {
  DPRINTLN("Starting Async Webserver...");
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    enableDisplay(false,0);
    DPRINTLN("Webserver: /");
    request->send(SPIFFS, "/index.html", "text/html");
  });

   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    enableDisplay(false,0);
    DPRINTLN("Webserver: /");
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  server.on("/jquery_351.js", HTTP_GET, [](AsyncWebServerRequest *request){
    enableDisplay(false,0);
    DPRINTLN("Webserver: /jquery_351.js");
    request->send(SPIFFS, "/jquery_351.js", "text/js");
  });

  server.on("/page.js", HTTP_GET, [](AsyncWebServerRequest *request){
    enableDisplay(false,0);
    DPRINTLN("Webserver: /page.js");
    request->send(SPIFFS, "/page.js", "text/js");
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    enableDisplay(false,0);
    DPRINTLN("Webserver: /favicon.ico");
    request->send(SPIFFS, "/favicon.ico", "image/png");
  });

  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){
    enableDisplay(false,0);
    DPRINTLN("Webserver: /generate_204");
    AsyncWebServerResponse *response = request->beginResponse(302); //
    response->addHeader("Location", String("http://") + WiFi.softAPIP().toString().c_str());
    request->send(response);


 /*   
    //request->send(SPIFFS, "/index.html", "text/html");
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("<!DOCTYPE html><html><head><title>UniClock</title></head><body>");
    response->print("<p><H1>");  response->print(webName);
    response->print("</p><p>Please, login to network and </p>");
    response->printf("<p>try opening <a href='http://%s'>%s</a> </p>", WiFi.softAPIP().toString().c_str(),WiFi.softAPIP().toString().c_str());
    //response->printf("192.168.4.1");
    response->print("<H1></body></html>");
    request->send(response);
    */
  });

  
  server.on("/site.css", HTTP_GET, [](AsyncWebServerRequest *request){
    enableDisplay(false,0);
    DPRINTLN("Webserver: /site.css");
    request->send(SPIFFS, "/site.css", "text/css");
  });
    
  server.on("/saveSetting", HTTP_POST, handleConfigChanged);
  server.on("/getConfiguration", HTTP_GET, handleSendConfig);
  
  server.onNotFound([](AsyncWebServerRequest *request) {
    
  int params = request->params();
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isFile()){ //p->isPost() is also true
      Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if(p->isPost()){
      Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
  }

  
    String message = "File Not Found\n\n";
    message += "URL: ";
    message += request->url();
    message += "\nMethod: ";
    message += ( request->method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += request->args();
    message += "\n";

    for ( uint8_t i = 0; i < request->args(); i++ ) {
      message += " " + request->argName ( i ) + ": " + request->arg ( i ) + "\n";
    }

    DPRINTLN(message);
    request->send( 204, "text/html", "URL Not Found" );
  });
  
  server.begin();

}  //end of procedure


void handleConfigChanged(AsyncWebServerRequest *request){
  if (request->hasParam("key", true) && request->hasParam("value", true)) {
    
    int args = request->args();
    
    //for(int i=0;i<args;i++){
    //  Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
    //}

    String key = request->getParam("key", true)->value();
    String value = request->getParam("value", true)->value();
    DPRINT(key); DPRINT(" = "); DPRINTLN(value); 
    
    boolean paramFound = true;
    
    if (key == "utc_offset")    {prm.utc_offset = value.toInt();}
    else if (key == "set12_24") {prm.set12_24 = (value == "true");  DPRINT("set12_24:"); DPRINTLN(prm.set12_24);}
    else if (key == "showZero") {prm.showZero = (value == "true"); DPRINT("showZero:"); DPRINTLN(prm.showZero);}        
    else if (key == "enableBlink"){prm.enableBlink = (value == "true"); } 
    else if (key == "enableDST")  {prm.enableDST = (value == "true"); }    
    else if (key == "interval")   {prm.interval = value.toInt(); }         
    else if (key == "enableAutoShutoff") {prm.enableAutoShutoff = (value == "true");  }

    else if (key == "dayTimeHours")   {prm.dayHour = value.toInt();}
    else if (key == "dayTimeMinutes") {prm.dayMin = value.toInt();}
    else if (key == "nightTimeHours") {prm.nightHour = value.toInt();}
    else if (key == "nightTimeMinutes") {prm.nightMin = value.toInt();}
    else if (key == "dayBright")    {prm.dayBright = value.toInt();}
    else if (key == "nightBright")  {prm.nightBright = value.toInt();}
    else if (key == "animMode")     {prm.animMode = value.toInt();}  
    else if (key == "manualOverride") {
      boolean v = value == "false";
      if (v != displayON) {manualOverride = true; displayON = v;}
    }
    else if (key == "alarmEnable")      {prm.alarmEnable = (value == "true");  }
    else if (key == "alarmTimeHours")   {prm.alarmHour = value.toInt();}
    else if (key == "alarmTimeMinutes") {prm.alarmMin = value.toInt();}
    else if (key == "alarmPeriod")      {prm.alarmPeriod = value.toInt();}
  
    //RGB LED values    
    else if(key == "rgbEffect")     {prm.rgbEffect = value.toInt();}     
    else if(key == "rgbBrightness") {prm.rgbBrightness = value.toInt();} 
    else if(key == "rgbFixColor")   {prm.rgbFixColor = value.toInt();}  
    else if(key == "rgbSpeed")      {prm.rgbSpeed = value.toInt();}   
    else if(key == "rgbDir")        {prm.rgbDir = (value == "true");}
    else if(key == "rgbMinBrightness") {c_MinBrightness = value.toInt(); }
    
    else  {paramFound = false;}

    if(paramFound){
      saveEEPROM();
      request->send(200, "text/plain", "Ok");
    }
    else{
      request->send(404, "text/plain", "404: Parameter not found");
    }
  }
  else{
    request->send(400, "text/plain", "400: Invalid Request. Parameters: key and value");
  }
}

void handleSendConfig(AsyncWebServerRequest *request){
  
  StaticJsonDocument<512> doc;
  char buf[20];  //conversion buffer
  DPRINTLN("Sending configuration to web client...");
  
  //Global data
  doc["version"] = webName;
  doc["maxDigits"] = maxDigits;   //number of digits (tubes)
  doc["maxBrightness"] = MAXBRIGHTNESS; //Maximum tube brightness usually 10, sometimes 12

  //Actual time and environment data
  sprintf(buf,"%4d.%02d.%02d",year(),month(),day());
  doc["currentDate"] = buf;
  sprintf(buf,"%02d:%02d",hour(),minute());
  doc["currentTime"] = buf;
  doc["temperature"] = temperature[0];  
  doc["humidity"] = 0;   
  
  //Clock calculation and display parameters
  doc["utc_offset"] = prm.utc_offset;
  doc["enableDST"] = prm.enableDST;         // Flag to enable DST (summer time...)
  doc["set12_24"] = prm.set12_24;           // Flag indicating 12 vs 24 hour time (false = 12, true = 24);
  doc["showZero"] = prm.showZero;           // Flag to indicate whether to show zero in the hour ten's place
  doc["enableBlink"] = prm.enableBlink;     // Flag to indicate whether center colon should blink
  doc["interval"] = prm.interval;           // doc["interval in minutes, with 0 = off
  
  //Day/Night dimmer parameters    
  doc["enableAutoShutoff"] = prm.enableAutoShutoff;  // Flag to enable/disable nighttime shut off
  sprintf(buf,"%02d:%02d",prm.dayHour,prm.dayMin);
  doc["dayTime"] = buf;
  sprintf(buf,"%02d:%02d",prm.nightHour,prm.nightMin);
  doc["nightTime"] = buf;
  doc["dayBright"] = prm.dayBright;
  doc["nightBright"] = prm.nightBright;
  doc["animMode"] = prm.animMode;  //Tube animation

  doc["manualOverride"] = !displayON; 
  
  //Alarm values
  doc["alarmEnable"] = prm.alarmEnable;   //1 = ON, 0 = OFF
  sprintf(buf,"%02d:%02d",prm.alarmHour,prm.alarmMin);
  doc["alarmTime"] = buf;
  doc["alarmPeriod"] = prm.alarmPeriod;
  
  //RGB LED values    
  doc["rgbEffect"] = prm.rgbEffect;       // if -1, no RGB exist!
  doc["rgbBrightness"] = prm.rgbBrightness; // c_MinBrightness..255
  doc["rgbFixColor"] = prm.rgbFixColor;   // 0..256
  doc["rgbSpeed"] = prm.rgbSpeed;       // 1..255
  doc["rgbDir"] = prm.rgbDir;          // 0 = right direction, 1 = left direction
  doc["rgbMinBrightness"] = c_MinBrightness;  //minimum brightness for range check!!
  
  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}

void setup() {
  //WiFi.mode(WIFI_OFF);
  Fdelay(200);
  DPRINTBEGIN(115200); DPRINTLN(" ");
  clearDigits(); 
  if (ALARMSPEAKER_PIN>=0) {pinMode(ALARMSPEAKER_PIN, OUTPUT); digitalWrite(ALARMSPEAKER_PIN,LOW);}
  if (ALARMBUTTON_PIN>=0)   pinMode(ALARMBUTTON_PIN, INPUT_PULLUP);  
  if (COLON_PIN>=0)         pinMode(COLON_PIN, OUTPUT);
  if (LED_SWITCH_PIN>=0)    pinMode(LED_SWITCH_PIN, OUTPUT);
  if (DECIMALPOINT_PIN>=0)  pinMode(DECIMALPOINT_PIN, OUTPUT);

  decimalpointON = false;
  
  if (TEMP_SENSOR_PIN>0) {
    setupTemp();
    setupDHTemp();
  }  
  setupRTC();
  setupGPS();

  DPRINT("Number of digits:"); DPRINTLN(maxDigits);
  loadEEPROM();
  if (prm.magic !=MAGIC_VALUE) factoryReset();
  
  setup_pins();
  setupNeopixelMakuna();  
  
  byte saveMode = prm.animMode;
  prm.animMode = 2;
  testTubes(300);
  
  clearDigits();
  enableDisplay(false,0);
  Fdelay(200);
  if(!SPIFFS.begin()){
     DPRINTLN("An Error has occurred while mounting SPIFFS");
     }
  else {
    #if defined(ESP32)
    DPRINT("SPIFFS Total bytes:    "); DPRINTLN(SPIFFS.totalBytes());
    DPRINT("SPIFFS Used bytes:     "); DPRINTLN(SPIFFS.usedBytes());
    #endif
    DPRINTLN(" ");
  }

  checkWifiMode();
  if (clockWifiMode) 
    startWifiMode();
  else 
    startStandaloneMode();
    
  delay(500);
  startServer();  
  clearDigits();
  delay(200);
  enableDisplay(true,0);
  showMyIp();  
  prm.animMode = saveMode;
  calcTime();  
  timeProgram();
}

void calcTime() {
  mySTD.offset = prm.utc_offset * 60;
  myDST.offset = mySTD.offset;
  if (prm.enableDST) { myDST.offset += 60; }
  myTZ = Timezone(myDST, mySTD);
  
  //if (clockWifiMode) DPRINTLN("clockWifiMode=true");
  
  if (clockWifiMode) {
    if (WiFi.status() == WL_CONNECTED) {
      if (timeClient.update()) {
        setTime(myTZ.toLocal(timeClient.getEpochTime()));
        if (year()>2019) 
          updateRTC();    //update RTC, if needed
        else {
          getRTC();  
          getGPS();
        }
      } //endif update?
    }  //endif Connected?
    else {
        getRTC();  
        getGPS();
    }
  }  //endif (clockWifiMode)
  else {  //standalone mode!
    getRTC();
    getGPS();
  }
}


void timeProgram() {
static unsigned long lastRun = 0;

  if ((millis()-lastRun)<300) return;

  lastRun = millis(); 
  calcTime();
  if (useTemp>0) {
    requestTemp(false);
    getTemp(); 
  }
  getDHTemp();
  if (prm.interval > 0) {  // protection is enabled
      // At the first top of the hour, initialize protection logic timer
      if (!initProtectionTimer && (minute() == 0)) {  
        protectTimer = 0;   // Ensures protection logic will be immediately triggered                                     
        initProtectionTimer = true;
      }
      if ((now() - protectTimer) >= 60 * prm.interval) {
        protectTimer = now();
        // The current time can drift slightly relative to the protectTimer when NIST time is updated
        // Need to make a small adjustment to the timer to ensure it is triggered at the minute change
        protectTimer -= ((second()+30)%60 - 30);  
        if (displayON) cathodeProtect();
      }      
  }
  
  if (now() != prevTime) { // Update time every second
    prevTime = now();
    evalShutoffTime();     // Check whether display should be turned off (Auto shutoff mode)
    if (!displayON || !prm.enableBlink) colonBlinkState = false;  
    else colonBlinkState = (bool)(second() % 2);
    
    if (maxDigits>=8)        displayTime8();         
    else if (maxDigits==6)   displayTime6(); 
    else displayTime4();
    
 //   if (COLON_PIN>=0) digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
    if ((LED_SWITCH_PIN>=0) && displayON) //Switch on underlightning LED only daytime
      digitalWrite(LED_SWITCH_PIN,HIGH);   
    else 
      digitalWrite(LED_SWITCH_PIN,LOW);
    printDigits(0);
    writeDisplaySingle();
  }
}

void loadEEPROM() {
  enableDisplay(false,0);
  DPRINT("Loading setting from EEPROM. EEPROM ver:");
  EEPROM.begin(sizeof(prm));
  EEPROM.get(EEPROM_addr,prm);
  EEPROM.end();
  DPRINTLN(prm.magic);
  enableDisplay(true,0);
}

void saveEEPROM() {
  enableDisplay(false,0);
  EEPROM.begin(sizeof(prm));
  EEPROM.put(EEPROM_addr,prm);     //(mod(mySTD.offset/60,24))); 
  EEPROM.commit();
  EEPROM.end();
  DPRINTLN("Settings saved to EEPROM!");
  enableDisplay(true,0);
}

void factoryReset() {
  prm.utc_offset = 1;
  prm.enableDST = false;          // Flag to enable DST (summer time...)
  prm.set12_24 = true;           // Flag indicating 12 vs 24 hour time (false = 12, true = 24);
  prm.showZero = true;           // Flag to indicate whether to show zero in the hour ten's place
  prm.enableBlink = true;        // Flag to indicate whether center colon should blink
  prm.interval = 15;             // prm.interval in minutes, with 0 = off
  prm.enableAutoShutoff = true;  // Flag to enable/disable nighttime shut off
  prm.dayHour = 7;
  prm.dayMin = 0;
  prm.nightHour = 22;
  prm.nightMin = 0;
  prm.dayBright = MAXBRIGHTNESS;
  prm.nightBright = 3;
  prm.animMode = 6; 
  prm.alarmEnable = false;
  prm.alarmHour = 7;
  prm.alarmMin = 0; 
  prm.alarmPeriod = 15;
  prm.rgbEffect = 1;
  prm.rgbBrightness = 100;
  prm.rgbFixColor = 150;
  prm.rgbSpeed = 50; 
  prm.rgbDir = 0;     
  prm.magic = MAGIC_VALUE;              //magic value to check the EEPROM version
  saveEEPROM();
  calcTime();
  DPRINTLN("Factory Reset!!!");
}

void cathodeProtect() {
  int hour12_24 = prm.set12_24 ? (byte)hour() : (byte)hourFormat12();
  byte hourShow = (byte)hour12_24;
  byte minShow  = (byte)minute();
  byte secShow  = (byte)second();
  byte dh1 = (hourShow /10), dh2 = (hourShow %10); 
  byte dm1 = (minShow /10),  dm2 = (minShow %10);
  byte ds1 = (secShow /10),  ds2 = (secShow %10);

  // All four digits will increment up at 10 Hz.
  // At T=2 sec, individual digits will stop at 
  // the correct time every second starting from 
  // the right and ending on the left
  for (int i = 0; i <= 70; i++){
    if (i >= 20) ds2 = (secShow % 10); 
    if (i >= 30) ds1 = (secShow /10); 
    if (i >= 40) dm2 = (minShow % 10);
    if (i >= 50) dm1 = (minShow / 10);
    if (i >= 60) dh2 = (hourShow % 10);
    if (i == 70) dh1 = (hourShow / 10);
    if (maxDigits>=8) {
      digit[7] = dh1;
      digit[6] = dh2;
      digit[5] = 11;
      digit[4] = dm1;
      digit[3] = dm2;
      digit[2] = 11;
      digit[1] = ds1;
      digit[0] = ds2;
    }
    else if (maxDigits==6) {
      digit[5] = dh1;
      digit[4] = dh2;
      digit[3] = dm1;
      digit[2] = dm2;
      digit[1] = ds1;
      digit[0] = ds2;
    }
    else {
      digit[3] = dh1;
      digit[2] = dh2;
      digit[1] = dm1;
      digit[0] = dm2;
    }
    incMod10(dh1); incMod10(dh2);
    incMod10(dm1); incMod10(dm2);
    incMod10(ds1); incMod10(ds2);
    Fdelay(100);
  }
  memcpy(oldDigit,digit,sizeof(oldDigit));
}

inline void incMod10(byte &x) { x = (x + 1 == 10 ? 0: x + 1); };


void displayTemp(byte ptr){
int digPtr = 0;
  for (int i=0;i<maxDigits;i++) {
    digitDP[i] = false; 
    newDigit[i] = 10; 
  }

  newDigit[digPtr++] = TEMP_CHARCODE;  //  "C"
  if (maxDigits>4) newDigit[digPtr++] = GRAD_CHARCODE;   //grad
  
  newDigit[digPtr++] = int(temperature[ptr]*10) % 10; 
  digitDP[digPtr] = true;
  newDigit[digPtr++] = int(temperature[ptr]) % 10;
  
  newDigit[digPtr] = int(temperature[ptr]) / 10; 
  if (newDigit[digPtr]==0) newDigit[digPtr] = 10;  //BLANK!!!
  if (prm.animMode == 0)  memcpy(oldDigit,newDigit,sizeof(oldDigit));  //don't do animation
  colonBlinkState = true;
  decimalpointON = true;
}  

void displayHumid(){
int digPtr = 0;
  for (int i=0;i<maxDigits;i++) {
    digitDP[i] = false; 
    newDigit[i] = 10; 
  }

  newDigit[digPtr++] = PERCENT_CHARCODE;  //  "%"
  newDigit[digPtr++] = int(humid*10) % 10; 
  digitDP[digPtr] = true;
  newDigit[digPtr++] = int(humid) % 10;
  
  newDigit[digPtr] = int(humid) / 10; 
  if (newDigit[digPtr]==0) newDigit[digPtr] = 10;  //BLANK!!!
  if (prm.animMode == 0)  memcpy(oldDigit,newDigit,sizeof(oldDigit));  //don't do animation
  colonBlinkState = true;
  decimalpointON = true;
} 

void displayTime4(){
  for (int i=0;i<maxDigits;i++) digitDP[i] = false;
  digitDP[4] = true;   digitDP[2] = true;    
  int hour12_24 = prm.set12_24 ? (byte)hour() : (byte)hourFormat12();
  if ((useTemp==2) && (second()>=TEMP_START+(TEMP_END-TEMP_START)/2) && (second()<TEMP_END)) displayTemp(1);
  else if ((useTemp>0) && (second()>=TEMP_START) && (second()<TEMP_END)) displayTemp(0);
  else if ((useHumid>0) && (second()>=HUMID_START) && (second()<HUMID_END)) displayHumid();
  else if ((ENABLE_CLOCK_DISPLAY) && (second()>=DATE_START)&& (second()<DATE_END)) {    
        newDigit[3] = month() / 10;
        newDigit[2] = month() % 10;   
        digitDP[2] = true; 
        newDigit[1] = day() / 10;
        newDigit[0] = day() % 10;
        colonBlinkState = true;
        }
      else if (ENABLE_CLOCK_DISPLAY){
        newDigit[3] = hour12_24 / 10;
        if ((!prm.showZero)  && (newDigit[3] == 0)) newDigit[3] = 10;
        newDigit[2] = hour12_24 % 10;
        newDigit[1] = minute() / 10;
        newDigit[0] = minute() % 10;
        if (prm.enableBlink && (second()%2 == 0)) digitDP[2] = false;  
       }
   changeDigit();    
}

void displayTime6(){
  for (int i=0;i<maxDigits;i++) digitDP[i] = false;
  digitDP[4] = true;   digitDP[2] = true;    
  int hour12_24 = prm.set12_24 ? (byte)hour() : (byte)hourFormat12();
  if ((useTemp==2) && (second()>=TEMP_START+(TEMP_END-TEMP_START)/2) && (second()<TEMP_END)) displayTemp(1);
  else if ((useTemp>0) && (second()>=TEMP_START) && (second()<TEMP_END)) displayTemp(0);
  else if ((useHumid>0) && (second()>=HUMID_START) && (second()<HUMID_END)) displayHumid();
  else if (ENABLE_CLOCK_DISPLAY && (second()>=DATE_START)&& (second()<DATE_END)) {    
        newDigit[5] = (year()%100) / 10;
        newDigit[4] = year() % 10;
        digitDP[4] = true;
        newDigit[3] = month() / 10;
        newDigit[2] = month() % 10;   
        digitDP[2] = true; 
        newDigit[1] = day() / 10;
        newDigit[0] = day() % 10;
        colonBlinkState = true;
        }
      else if (ENABLE_CLOCK_DISPLAY){
        newDigit[5] = hour12_24 / 10;
        if ((!prm.showZero)  && (newDigit[5] == 0)) newDigit[5] = 10;
        newDigit[4] = hour12_24 % 10;
        newDigit[3] = minute() / 10;
        newDigit[2] = minute() % 10;
        if (prm.enableBlink && (second()%2 == 0)) digitDP[2] = false;  
        newDigit[1] = second() / 10;
        newDigit[0] = second() % 10;
      }
  changeDigit();    
}

void displayTime8(){
  for (int i=0;i<maxDigits;i++) {digitDP[i] = false; newDigit[i] = 10;}
  int hour12_24 = prm.set12_24 ? (byte)hour() : (byte)hourFormat12();
  if ((useTemp==2) && (second()>=TEMP_START+(TEMP_END-TEMP_START)/2) && (second()<TEMP_END)) displayTemp(1);
  else if ((useTemp>0) && (second()>=TEMP_START) && (second()<TEMP_END)) displayTemp(0);
  else if ((useHumid>0) && (second()>=HUMID_START) && (second()<HUMID_END)) displayHumid();
  else {
    if (ENABLE_CLOCK_DISPLAY && (second()>=DATE_START) && (second()<DATE_END)) {    
      newDigit[7] = year() / 1000;
      newDigit[6] = (year()%1000) / 100;    
      newDigit[5] = (year()%100) / 10;
      newDigit[4] = year() % 10;
      digitDP[4] = true;
      newDigit[3] = month() / 10;
      newDigit[2] = month() % 10;   
      digitDP[2] = true; 
      newDigit[1] = day() / 10;
      newDigit[0] = day() % 10;
      colonBlinkState = true;
      if (prm.animMode == 1)  memcpy(oldDigit,newDigit,sizeof(oldDigit));  //don't do animation
    }
    else if (ENABLE_CLOCK_DISPLAY){
      newDigit[8] = 10;  //sign digit = BLANK
      newDigit[7] = hour12_24 / 10;
      if ((!prm.showZero)  && (newDigit[7] == 0)) newDigit[7] = 10;
      newDigit[6] = hour12_24 % 10;
      newDigit[5] = 11;  //- sign
      newDigit[4] = minute() / 10;
      newDigit[3] = minute() % 10;
      newDigit[2] = 11;  //- sign
      if (prm.enableBlink && (second()%2 == 0)) newDigit[2] = 10;  //BLANK
      newDigit[1] = second() / 10;
      newDigit[0] = second() % 10;
    }
  }
  changeDigit();
}


void changeDigit() {
  int j=0;
  byte tmp[30];
  byte space = 4;
  byte anim;
  
  anim = prm.animMode; 
  //anim = 0;
  if (anim == 6) anim = 1 + rand()%5;

  if (anim != 5) {
    for (int i=0;i<maxDigits;i++)   
      if ((newDigit[i]>9) || ((oldDigit[i]>9) && (newDigit[i]<=9) )) {
        digit[i] = newDigit[i];    //show special characters ASAP or if special char changes to numbers
        oldDigit[i] = newDigit[i];
      }
      if ((maxDigits>4) && (newDigit[0]!=0)) j=1;   //if 6 or 8 tube clock, dont play with seconds  
  }    

  if (memcmp(newDigit+j,oldDigit+j,maxDigits-j)!=0) {
    switch (anim) {
    case 1: 
      for (int tube=j;tube<maxDigits;tube++) {
        if ((newDigit[tube] != oldDigit[tube]) && (newDigit[tube]<=9)) { 
          for (int i=oldDigit[tube];i<=int(newDigit[tube]+10);i++) {
            digit[tube] = i % 10;
            writeDisplaySingle();
            Fdelay(ANIMSPEED);
          } //end for i
          writeDisplaySingle();
        } //endif
      }  //end for tube
      break;  
    case 2:
      for (int i=0;i<=9;i++) {
        for (int tube=j;tube<maxDigits;tube++) {
          if ((newDigit[tube] != oldDigit[tube]) && (newDigit[tube]<=9)) 
            digit[tube] = (oldDigit[tube]+i) % 10;
        } //end for tube
        writeDisplaySingle();
        Fdelay(ANIMSPEED);
      }  //end for i
      break;
    case 3:
      memcpy(tmp,oldDigit,maxDigits);    
      memcpy(tmp+maxDigits+space,newDigit,maxDigits);
      memset(tmp+maxDigits,11,space);  //----
      for (int i=0;i<maxDigits+space;i++) {
        for (int tube=0;tube<maxDigits;tube++) {
          digit[tube%maxDigits] = tmp[i+tube];
        } //end for tube
        writeDisplaySingle();
        Fdelay(ANIMSPEED);
      } //end for i
      break;
    case 4:
      memcpy(tmp,newDigit,maxDigits);    
      memcpy(tmp+maxDigits+space,oldDigit,maxDigits);
      memset(tmp+maxDigits,11,space);  //----
      for (int i=maxDigits-1+space;i>=0;i--) {
        for (int tube=0;tube<maxDigits;tube++) {
          digit[tube%maxDigits] = tmp[i+tube];
        } //end for tube
        writeDisplaySingle();
        Fdelay(ANIMSPEED);
      } //end for i
      break;
    case 5:
    memset(animMask,0,sizeof(animMask));
#if defined(MULTIPLEX74141) || defined(NO_MULTIPLEX74141)
    for (int i=1;i<10;i++) {
        for (int tube=j;tube<maxDigits;tube++) {
          if (oldDigit[tube] != newDigit[tube]) animMask[tube]=i;    //digit is changed
        }  //end for tube
        Fdelay(ANIMSPEED);
      }  //end for i
#else
      for (int i=1;i<=5;i++) {
        for (int tube=j;tube<maxDigits;tube++) {
          if (oldDigit[tube] != newDigit[tube]) animMask[tube]=i;   //digit is changed
        }  //end for tube
        writeDisplaySingle();
        Fdelay(ANIMSPEED);
      }  //end for i
      memcpy(digit,newDigit,sizeof(digit));
      for (int i=1;i<=5;i++) {
        for (int tube=j;tube<maxDigits;tube++) {
          if (oldDigit[tube] != newDigit[tube]) animMask[tube]=6-i;  //digit is changed
        }  //end for tube
         writeDisplaySingle();        
        Fdelay(ANIMSPEED);
      }  //end for i     
#endif   
      memset(animMask,0,sizeof(animMask));   
      break;
  }  
  } //endif memcmp  

  memcpy(digit,newDigit,sizeof(digit));
  memcpy(oldDigit,newDigit,sizeof(oldDigit));
}


void alarmSound() {
  static const unsigned int t[] = {0,3000,6000,6200,9000,9200,9400,12000,12200,12400,15000,15200,15400};  
  static int count = 0;
  static unsigned long nextEvent;
  const int cMax = sizeof(t) / sizeof(t[0]);  //number of time steps

  if (prm.alarmEnable) {
    if ( (!alarmON && prm.alarmHour == hour()) && (prm.alarmMin == minute()) && (second()<=3)) {    //switch ON alarm sound
      DPRINTLN("Alarm started!");
      alarmON = true;
      alarmStarted = millis();
      count = 0;
      nextEvent = alarmStarted-1;
    }
  }  
  else {
    alarmON = false;
  }
  
  if (!alarmON) {
    if (ALARMSPEAKER_PIN>=0) digitalWrite(ALARMSPEAKER_PIN,LOW);
    return;  //nothing to do
  }

  if ((millis()-alarmStarted) > 1000*(long)prm.alarmPeriod) { 
    alarmON = false;   //alarm period is over
    darkenNeopixels();
    DPRINTLN("Alarm ended.");
  }  
    
  if (ALARMBUTTON_PIN>=0) {   //is button installed?
    if (digitalRead(ALARMBUTTON_PIN)==LOW) {   //stop alarm
      DPRINTLN("Alarm stopped by button press.");
      alarmON = false;
    }
  }
  
  if (!prm.alarmEnable || !alarmON) {  //no alarm, switch off
      if (ALARMSPEAKER_PIN>=0) digitalWrite(ALARMSPEAKER_PIN,LOW);
      return;
  }

  //-------- Generate sound --------------------------------
   
  if (millis()>nextEvent) {   //go to the next event
    if (count % 2 == 0) {
      nextEvent += 150;
      if (ALARMSPEAKER_PIN>=0) digitalWrite(ALARMSPEAKER_PIN,HIGH);
      //DPRINT(" Sound ON");  DPRINT("  Next:"); DPRINTLN(nextEvent); 
    }
    else {
      if (ALARMSPEAKER_PIN>=0) digitalWrite(ALARMSPEAKER_PIN,LOW);
      nextEvent = (count/2 < cMax) ? alarmStarted +  t[count/2] : nextEvent + 200;
      //DPRINT("   OFF"); DPRINT("  Next:"); DPRINTLN(nextEvent); 
    }
    count++;  
  }
}


void evalShutoffTime() {  // Determine whether  tubes should be turned to NIGHT mode
  
  if (!prm.enableAutoShutoff) return;
  
  int mn = 60*hour() + minute();
  int mn_on = prm.dayHour*60 + prm.dayMin;  
  int mn_off = prm.nightHour*60 + prm.nightMin;
  
  static bool prevShutoffState = true;  
  if ( (((mn_off < mn_on) &&  (mn >= mn_off) && (mn < mn_on))) ||  // Nighttime
        ((mn_off > mn_on) && ((mn >= mn_off) || (mn < mn_on)))) { 
     if (!manualOverride) displayON = false;
     if (prevShutoffState == true) manualOverride = false; 
     prevShutoffState = false;
  }
  else {  // Tubes should be on
    if (!manualOverride) displayON = true;
    if (prevShutoffState == false) manualOverride = false; 
    prevShutoffState = true;
  }
  return;
  DPRINT("mn="); DPRINT(mn);
  DPRINT("  mn_on="); DPRINT(mn_on);
  DPRINT("  mn_off="); DPRINT(mn_off);
  DPRINT("  manOverride:"); DPRINT(manualOverride);
  DPRINT("  displayON:"); DPRINTLN(displayON);
  DPRINT("  enableBlink:"); DPRINTLN(prm.enableBlink);
  DPRINT("  blinkState:"); DPRINTLN(colonBlinkState);
}


void writeIpTag(byte iptag) {
  memset(newDigit,10,sizeof(newDigit));
  if (!digitsOnly && (maxDigits>=6)) {
    newDigit[4] = 1;
    newDigit[3] = 14;
  }
  if (iptag>=100) newDigit[2] = iptag / 100;
  newDigit[1] = (iptag % 100) / 10;
  newDigit[0] = iptag % 10;
  writeDisplaySingle();
  changeDigit();
}

void showMyIp() {   //at startup, show the web page internet address
  //clearDigits();
#define SPEED 1500  

  writeIpTag(ip[0]);
  printDigits(0);
  Fdelay(SPEED);
  writeIpTag(ip[1]);
  printDigits(0);
  Fdelay(SPEED);
  writeIpTag(ip[2]);
  printDigits(0);
  Fdelay(SPEED);
  writeIpTag(ip[3]);
  printDigits(0);
  Fdelay(SPEED);
}


void fifteenMinToHM(int &hours, int &minutes, int fifteenMin)
{
  hours = fifteenMin/4;
  minutes = (fifteenMin % 4) * 15; 
}

void resetWiFi(){
static unsigned long lastTest = millis();
static unsigned int counter =0;

  if ((millis()-lastTest)<60000) return;   //60000 //check in every 60sec
  lastTest = millis();
  counter++;   
  DPRINTLN("Wifi lost! Waiting for "); DPRINT(counter); DPRINTLN(" minutes...");
  if (counter<180) return;    //3 hours passed
  //WiFiManager MyWifiManager;
  //MyWifiManager.resetSettings();    //reset all AP and wifi passwords...
  DPRINTLN("Restart Clock...");
  Fdelay(1000);
  ESP.restart();
}

inline int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}


void testTubes(int dely) {
   Fdelay(dely);
   DPRINT("Testing tubes: ");
   for (int i=0; i<10;i++) { 
     DPRINT(i); DPRINT(" ");
     for (int j=0;j<maxDigits;j++) {
      newDigit[j] = i;
      digitDP[j] = i%2;
     }
     changeDigit();
     writeDisplaySingle();
     Fdelay(dely); 
   }
   DPRINTLN(" ");
   Fdelay(1000);
   memset(digitDP,0,sizeof(digitDP));
}

void printDigits(unsigned long timeout) {
static unsigned long lastRun = millis();

  if ((millis()-lastRun)<timeout) return;
  lastRun = millis();
  for (int i=maxDigits-1;i>=0;i--)  
    if (digit[i]<10) DPRINT(digit[i]);  
    else DPRINT("-");
  if (colonBlinkState) DPRINT(" B ");
  else DPRINT("   ");
  //DPRINT(ESP.getFreeHeap());   //show free memory for debugging memory leak
  DPRINTLN(" ");
}

void loop() {  
  dnsServer.processNextRequest();
  //MDNS.update();
  enableDisplay(true,2000);
  timeProgram();
  writeDisplaySingle();
  doAnimationMakuna();
  alarmSound();
  checkWifiMode();
  if (clockWifiMode) { //Wifi Clock Mode
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
      resetWiFi();
    }  
  }
  else {   //Manual Clock Mode
     editor();
  } //endelse
  yield();
}

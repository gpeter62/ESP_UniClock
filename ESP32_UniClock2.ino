/*
      Universal Clock  (Nixie, VFD, LED, Numitron) for ESP8266 or ESP32
      with optional Dallas Thermometer and DS3231 RTC, Neopxels stripe, GPS and more...
      18/01/2021
      Copyright (C) 2020  Peter Gautier

      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 3 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License <http://www.gnu.org/licenses/> for more details.

      Always check the usable pins of 8266: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
                 and usable pins for ESP32: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/

      //https://lastminuteengineers.com/esp32-arduino-ide-tutorial/

      Web page files, wich are found in "dat" subfolder must upload to SPIFFS !
      ESP8266:  https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide/
      ESP32: https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
*/

#include "clocks.h"  //DEFINE YOUR CLOCKS SETUP IN THIS FILE!!!

/*_______________________________ USABLE PARAMETERS ____________________________________________
  //#define DEBUG  //Enable Serial Monitor, 115200baud (only, if TX pin is not used anywhere!!!)
  //---------------------------- CLOCK EXTRA OPTION PARAMETERS ---------------------------------
  //#define USE_DALLAS_TEMP //TEMP_DALLAS_PIN is used to connect DS18B20 temperature sensors
  //#define USE_DHT_TEMP    //TEMP_DHT_PIN is sensor pin 
  //#define USE_BME280      //I2C Temperature + humidity + pressure, SDA+SCL I2C pins are used!   
  //#define USE_BMP280      //I2C Temperature + barometric pressure, SDA+SCL I2C pins are used!   
  //#define USE_AHTX0       //I2C Temperature + humidity, SDA+SCL I2C pins are used!   
  //#define USE_SHT21       //I2C Temperature + humidity, SDA+SCL I2C pins are used!   
  //#define USE_RTC         //DS3231 realtime clock, SDA+SCL I2C pins are used!   
  //#define USE_GPS         //use for standalone clock, without wifi internet access
  //#define USE_NEOPIXEL    //WS2812B led stripe, for tubes backlight. Don't forget to define tubePixels[] !
  //#define USE_MQTT        //Home Assistant integration: https://www.home-assistant.io/

  //----- DRIVER SELECTION ------ Use only 1 driver from the following options in the clocks.h file!
  //#define MULTIPLEX74141_ESP32  //4..8 Nixie tubes generic driver for ESP32
  //#define MAX6921_ESP32         //4..8 VFD tubes (IV18) driver for ESP8232
  //#define HV5122                //4..8 Nixie driver - development in progress
  //-------------- 8266 clock drivers --------------------------------------------------
  //#define MULTIPLEX74141        //4..8 Nixie tubes generic driver for ESP8266
  //#define MAX6921               //4..8 VFD tubes (IV18) driver for ESP8266
  //#define NO_MULTIPLEX74141     //4..6 Nixie tubes, serial latch driver, 74141 for each tube
  //#define MM5450                //6..8 LEDS
  //#define MAX7219CNG            //4..8 LED
  //#define Numitron_4511N        //Numitron 4x tube clock
  //#define SN75512               //4..8 VFD tubes
  //#define samsung               //samsung serial display
  //#define PCF_74141             //PCF pin expander for tube selection
  //#define PT6355                //VFD clock - development in progress

  //--------------------- PINOUT & PIN PARAMETERS ---------------------------------------
  //#define PIN_SDA xx             // you can set the used SDA and SCL pins
  //#define PIN_SCL xx             // if it is not default value
  //#define COLON_PIN   -1        //Blinking Colon pin.  If not used, SET TO -1
  //#define TEMP_DALLAS_PIN -1    //Dallas DS18B20 temp sensor pin.  If not used, SET TO -1
  //#define TEMP_DHT_PIN -1       //DHT temp sensor pin.  If not used, SET TO -1
  //#define DHTTYPE DHT22         //DHT sensor type, if used...
  //#define LED_SWITCH_PIN -1     //external led backlight ON/OFF.  If not used, SET TO -1
  //#define DECIMALPOINT_PIN -1   //Nixie decimal point between digits. If not used, SET TO -1
  //#define ALARMSPEAKER_PIN -1   //Alarm buzzer pin
  //#define ALARMBUTTON_PIN -1    //Alarm switch off button pin
  //#define ALARM_ON HIGH         //HIGH or LOW level is needed to switch ON the buzzer?
  //#define NEOPIXEL_PIN 3        //8266 Neopixel LEDstripe pin is always the RX pin!!!
  //#define RGB_MIN_BRIGHTNESS 8   //Neopixel leds minimum brightness
  //#define RGB_MAX_BRIGHTNESS 255 //Neopixel leds maximum brightness
  //#define RADAR_PIN -1          //Radar sensor pin
  //#define RADAR_TIMEOUT 300     //Automatic switch off tubes (without radar detecting somebody) after xxx sec
  //#define TUBE_POWER_PIN -1     //Filament or HV switch ON/OFF pin
  //#define TUBE_POWER_ON HIGH    //HIGH or LOW level is needed to switch ON the TUBE POWER?
  
  //#define LIGHT_SENSOR_PIN -1   //Environment light sensor, only ADC pins are usable! ESP32 for example: 34,35,36,39... 8266: only A0
  //#define REF_RESISTANCE    10000.0         // Resistor value is 10k, between LDR sensor and GND
  //#define MAXIMUM_LUX 100                   //Lux level for maximum tube brightness
  //#define LUX_CALC_SCALAR   12518931 * 1.2  //Calibrate here the LDR sensor
  //#define LUX_CALC_EXPONENT -1.405          //LDR sensor characteristic

  //#define MAXBRIGHTNESS 10      //Do not change this value!

  //Display temperature and date in every minute between START..END seconds
  //#define ENABLE_CLOCK_DISPLAY true  //false, if no clock display is needed (for example: thermometer + humidity only)
  //#define DATE_REPEAT_MIN   1     //show date only every xxx minute. If zero, datum is never displayed!
  //#define SHIFT_TUBES_LEFT_BY_1   //shift leftIP address by 1 tube the display, if a thermometer is used with spec tube
  //#define LEFTDECIMAL false      //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  //#define TEMP_START  35        //Temperature display start..end
  //#define TEMP_END    40
  //#define HUMID_START 40        //Humidity% display start..end
  //#define HUMID_END   45
  //#define DATE_START  45        //Date is displayed start..end
  //#define DATE_END    50
  //#define ANIMSPEED   50        //Animation speed in millisec
  //#define TEMP_CHARCODE 15      //Thermometer "C", set to -1 to disable
  //#define GRAD_CHARCODE 16      //Thermometer grad, set to -1 to disable
  //#define PERCENT_CHARCODE 17   //Humidity %
  //#define DOUBLE_BLINK          //both separator points are blinking (6 or 8 tubes VFD clock)

  //#define AP_NAME "UNICLOCK"  	//Access Point name
  //#define AP_PASSWORD ""	   		//AP password
  //#define WEBNAME "LED UniClock"  //Clock's name on the web page
*/
#define TIMESERVER_REFRESH 7200000     //7200000   Refresh time in millisec   86400000 = 24h
unsigned long timeserverErrors = 0;        //timeserver refresh errors

boolean autoBrightness = false; //Enable automatic brightness levels
int LuxValue = MAXIMUM_LUX;         //Enviroment LUX value, set by Light Sensor
int timerON,timerOFF;       //for debugging

byte c_MinBrightness = RGB_MIN_BRIGHTNESS;       //minimum LED brightness
byte c_MaxBrightness = RGB_MAX_BRIGHTNESS;     //maximum LED brightness
unsigned long intCounter = 0;   //for testing only, interrupt counter
//--------------------------------------------------------------------------------------------------

#if defined(ESP8266)
  #ifndef WEBNAME
    #define WEBNAME "ESP32_UniClock 2.5"
  #endif
  #ifndef AP_NAME
    #define AP_NAME "UNICLOCK"
  #endif
  #ifndef AP_PASSWORD
    #define AP_PASSWORD ""
  #endif
  #include <ESP8266WiFi.h>
  //#include <ESP8266mDNS.h>
  #include "ESPAsyncTCP.h"
  #include "FS.h"
  const char ESPpinout[] = {"OOOOOO      OOOOOI"};   //GPIO 0..5, 12..16, A0)  usable pins

#elif defined(ESP32)
  #ifndef WEBNAME
    #define WEBNAME "ESP32UniClock 2.5"
  #endif
  #ifndef AP_NAME
    #define AP_NAME "UNICLOCK32"
  #endif
  #ifndef AP_PASSWORD
    #define AP_PASSWORD ""
  #endif
  
  //#include <WiFi.h>
  #include <esp_wifi.h>
  //#include <ESPmDNS.h>
  #include "AsyncTCP.h"
  #include "SPIFFS.h"

  hw_timer_t * ESP32timer = NULL;
  portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
  #define PRESCALER 15   //multiplex timer prescaler, about 80Hz for 6 tubes
  
  const char ESPpinout[] = {"OOOOOO      OOOOOOOO OOO OOO    OOIII  I"};   //GPIO 0..5, 12..19, 21..23, 25..27, 32..33, Input:34..36, 39)  usable pins 
#else
  #error "Board is not supported!"
#endif

#include <DNSServer.h>
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

#ifdef USE_NEOPIXEL
#include <NeoPixelBrightnessBus.h>
#endif

extern void ICACHE_RAM_ATTR writeDisplay();
extern void writeDisplaySingle();
extern void setup_pins();
extern void clearTubes();
extern const int maxDigits;

const char* ssid = AP_NAME;  // Enter SSID here
const char* password = AP_PASSWORD;  //Enter Password here
char webName[] = WEBNAME;
AsyncWebServer server(80);
//DNSServer dns;
#define CACHE_MAX_AGE "max-age=31536000" //maximum is: 31536000

//#include <WiFiMulti.h>
//WiFiMulti wifiMulti;

#define BUFSIZE 12
byte digit[BUFSIZE];
byte newDigit[BUFSIZE];
byte oldDigit[BUFSIZE];
boolean digitDP[BUFSIZE];   //actual value to put to display
boolean digitsOnly = true;  //only 0..9 numbers are possible to display?
byte animMask[BUFSIZE];     //0 = no animation mask is used

volatile boolean dState = false;
volatile unsigned long lastDisable = 0;
volatile boolean EEPROMsaving = false; //saving in progress - stop display refresh

#define MAGIC_VALUE 200   //EEPROM version

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
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, TIMESERVER_REFRESH); // Refresh time in millisec
IPAddress ip;

// Set timezone rules.  Offsets set to zero, since they will be loaded from EEPROM
TimeChangeRule myDST = {"DST", Last, Sun, Mar, 2, 0};
TimeChangeRule mySTD = {"STD", First, Sun, Nov, 2, 0};
Timezone myTZ(myDST, mySTD);

boolean clockWifiMode = true;
boolean radarON = true;

byte useDallasTemp = 0;   //number of Dallas temperature sensors: 0,1,2
byte useTemp = 0;         //Total number of any temperature sensors: 0..6
float temperature[6] = {0,0,0,0,0,0};
byte useHumid = 0;        //Total number of humidity sensors
float humid[6] = {0,0,0,0,0,0};  
byte usePress = 0;        //Total number of pressure sensors
float pressur[6] = {0,0,0,0,0,0};  
//----------------- EEPROM addresses -------------------------------------------------------------------
const int EEPROM_addr = 0;

struct {
  int utc_offset = 1;
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
  unsigned int  rgbFixColor = 150;          //0..255
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

boolean showDate = false;
boolean showTemp0 = false;
boolean showTemp1 = false;
boolean showHumid0 = false;
boolean showHumid1 = false;
int lastCathodeProt = -1;

#define MAX_PIN sizeof(ESPpinout)-1
#define PIN_TXT_LEN 16
char pinTxt[MAX_PIN][PIN_TXT_LEN+1];

void regPin(byte p,const char * txt) {  //register used pins
  
  DPRINT("- "); DPRINT(txt); DPRINT(": GPIO"); DPRINT(p); 
  if (p>MAX_PIN-1) {
    DPRINT("  ERROR: PIN# DOESN'T EXIST.");
  }
  else if (ESPpinout[p]==' ') {
    DPRINT("  ERROR: RESERVED PIN.");
  }
  else if (ESPpinout[p]=='I') {
    DPRINT("  Warning: input only pin");
  }
  DPRINTLN(" ");
  if (strlen(pinTxt[p])>0) {
    DPRINT("*** ERROR *** "); DPRINT(txt); DPRINT(" on PIN#"); DPRINT(p);  
    DPRINT(" ALREADY DEFINED AS "); DPRINTLN(pinTxt[p]); 
    strncpy(pinTxt[p],"ERROR! MULTI DEF",PIN_TXT_LEN);
  }
  else 
    strncpy(pinTxt[p],txt,16);
}

void listPins() {
  DPRINTLN("_______________________");
  DPRINTLN("___ USED CLOCK PINS ___");
  for (int i=0;i<=MAX_PIN;i++) {
    if (strlen(pinTxt[i])>0) {
      DPRINT(i); DPRINT(": ");  DPRINTLN(pinTxt[i]);
    }
  }
  DPRINTLN("_______________________");
  //DPRINT("MAX_PIN"); DPRINTLN(MAX_PIN);
}

void startTimer() {   //ESP_INTR_FLAG_IRAM
#if defined(ESP8266)
  timer1_attachInterrupt(writeDisplay);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(15000);
  DPRINTLN("Starting 8266 timer...");

#elif defined(ESP32)
  //  https://techtutorialsx.com/2017/10/07/esp32-arduino-timer-interrupts/
  ESP32timer = timerBegin(0, PRESCALER, true); //set prescaler to 80 -> 1MHz signal, true = edge generated signal
  timerAttachInterrupt(ESP32timer, &writeDisplay, true);
  timerAlarmWrite(ESP32timer, 1000, true);   //100millisec, no repeat
  timerAlarmEnable(ESP32timer);
  //DPRINTLN("Starting ESP32 timer...");
#else
#error "Unknown controller board!"
#endif
}

void stopTimer() {
#if defined(ESP8266)
  timer1_detachInterrupt();
#endif
}

void configModeCallback (AsyncWiFiManager *myWiFiManager) {
  DPRINTLN("Switch to AP config mode...");
  DPRINTLN("To configure Wifi,  ");
  DPRINT("connect to Wifi network "); DPRINTLN(ssid);
  DPRINTLN("and open 192.168.4.1 in web browser");
}

void clearDigits() {
  memset(oldDigit, 10, sizeof(oldDigit));
  memset(digit, 10, sizeof(digit));
  memset(newDigit, 10, sizeof(newDigit));
  memset(digitDP, 0, sizeof(digitDP));
}

void Fdelay(unsigned long d) {
  unsigned long dStart = millis();

  doAnimationMakuna();
  while ((millis() - dStart) < d) {
    dnsServer.processNextRequest();
    //MDNS.update();
    enableDisplay(2000);
    getLightSensor();
    doAnimationMakuna();
    alarmSound();
    yield();
  }
}

float round1(float in) {
  return round(10.0 * in)/10.0; 
}

//--------------------------------------------------------------------------------------------------------------------

void enableDisplay(unsigned long timeout) {
  unsigned long seged2;
#if defined(ESP32) || defined(PCF_74141)  //safety mode for slow multiplex hardvare to avoid crash when write to flash memory
  if (dState) return;
  seged2 = lastDisable;
  if (millis() < (seged2 + timeout)) return;

  dState = true;
  EEPROMsaving = false;
  DPRINT("Enable tubes:"); DPRINTLN(timeout); //    DPRINT(millis()); DPRINT("/"); DPRINTLN(lastDisable);
  //startTimer();
#else
  EEPROMsaving = false;
#endif
}

void disableDisplay()  {

  EEPROMsaving = true;

#if defined(ESP32) || defined(PCF_74141)  //safety mode for slow multiplex hardvare to avoid crash when write to flash memory
  DPRINTLN("Disable tubes");
  if (dState) {
    clearTubes();
    //stopTimer();
    delay(20);
    writeDisplay();
  }
#endif

  dState = false;
  lastDisable = millis();
}

void startWifiMode() {
  int count = 0;

  disableDisplay();
  DPRINTLN("Starting Clock in WiFi Mode!");
  WiFi.mode(WIFI_STA);
#if defined(ESP32)
  WiFi.setHostname(webName);
#else
  WiFi.hostname(webName);
#endif
#if defined(ESP32)
  esp_wifi_set_ps (WIFI_PS_NONE);
#endif
  AsyncWiFiManager MyWifiManager(&server, &dnsServer);
  MyWifiManager.setAPCallback(configModeCallback);
  //MyWifiManager.setConfigPortalTimeout(180);
  for (int i = 0; i < 5; i++) {
    if (!MyWifiManager.autoConnect(AP_NAME)) {  //or autoConnect(AP_NAME,AP_PASSWORD))
      DPRINT("Retry to Connect:"); DPRINTLN(i);
      WiFi.disconnect();
      WiFi.mode(WIFI_OFF);
      delay(1000);
      WiFi.mode(WIFI_STA);
#if defined(ESP32)
      WiFi.setHostname(webName);
#else
      WiFi.hostname(webName);
#endif
      if (i == 4) {
        MyWifiManager.autoConnect(AP_NAME); // or autoConnect(AP_NAME,AP_PASSWORD))
      }
    }
    else
      break;
  }  //end for
  ip = WiFi.localIP();
  WiFi.setAutoReconnect(true);
  enableDisplay(100);
  showMyIp();
  DPRINTLN("Connecting to Time Server...");
  timeClient.begin();
  timeClient.forceUpdate();
  while (true) {
    delay(100);
    enableDisplay(1000);
    if (timeClient.update()) break;
    DPRINT(" - Connecting to timeserver: "); DPRINTLN(count);
    count ++; if (count > 30) restartClock(); //restart clock
    writeIpTag(count);
    Fdelay(500);
  }
}

void startStandaloneMode() {
  DPRINTLN("Starting Clock in Standalone Mode!");
  DPRINT("Clock's wifi SSID:"); DPRINTLN(ssid);
  DPRINTLN("IP: 192.168.4.1");
  IPAddress local_ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
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
  enableDisplay(0);
  Fdelay(1000);
  showMyIp();
}


void startServer() {
  DPRINTLN("Starting Async Webserver...");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    String filenam;
    boolean gzip;

    disableDisplay();
    DPRINTLN("Webserver: /  -> index.html");
    gzip = SPIFFS.exists("/index.html.gz");
    if (gzip)
      filenam = "/index.html.gz";
    else
      filenam = "/index.html";

    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, filenam, "text/html");   //gzip compressed file sending
    if (gzip)
      response->addHeader("Content-Encoding", "gzip");

    //response->addHeader("Cache-Control", CACHE_MAX_AGE);
    request->send(response);
  });

  server.on("/jquery_351.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    String filenam;
    boolean gzip;

    disableDisplay();
    DPRINTLN("Webserver: /jquery_351.js");
    gzip = SPIFFS.exists("/jquery_351.js.gz");
    if (gzip)
      filenam = "/jquery_351.js.gz";
    else
      filenam = "/jquery_351.js";

    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, filenam, "text/js");   //gzip compressed file sending
    if (gzip)
      response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", CACHE_MAX_AGE);
    request->send(response);
  });

  server.on("/page.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    String filenam;
    boolean gzip;

    disableDisplay();
    DPRINTLN("Webserver: /page.js");
    gzip = SPIFFS.exists("/page.js.gz");
    if (gzip)
      filenam = "/page.js.gz";
    else
      filenam = "/page.js";

    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, filenam, "text/js");   //gzip compressed file sending
    if (gzip)
      response->addHeader("Content-Encoding", "gzip");

    response->addHeader("Cache-Control", CACHE_MAX_AGE);
    request->send(response);
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest * request) {
    disableDisplay();
    DPRINTLN("Webserver: /favicon.ico");
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/favicon.png", "image/png");
    response->addHeader("Cache-Control", CACHE_MAX_AGE);
    request->send(response);
  });

  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest * request) {
    disableDisplay();
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


  server.on("/site.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    String filenam;
    boolean gzip;

    disableDisplay();
    DPRINTLN("Webserver: /site.css");
    gzip = SPIFFS.exists("/site.css.gz");

    if (gzip)
      filenam = "/site.css.gz";
    else
      filenam = "/site.css";

    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, filenam, "text/css");   //gzip compressed file sending
    if (gzip)
      response->addHeader("Content-Encoding", "gzip");
    response->addHeader("Cache-Control", CACHE_MAX_AGE);
    request->send(response);
  });

  server.on("/saveSetting", HTTP_POST, handleConfigChanged);
  server.on("/getConfiguration", HTTP_GET, handleSendConfig);
  server.on("/getCurrentInfos", HTTP_GET, handleSendCurrentInfos);
  server.onNotFound(handleNotFound);

  server.begin();

}  //end of procedure

void handleNotFound(AsyncWebServerRequest *request) {
  int params = request->params();
  for (int i = 0; i < params; i++) {
    AsyncWebParameter* p = request->getParam(i);
    if (p->isFile()) { //p->isPost() is also true
      Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if (p->isPost()) {
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
}

void handleConfigChanged(AsyncWebServerRequest *request) {
  if (request->hasParam("key", true) && request->hasParam("value", true)) {

    //int args = request->args();

    //for(int i=0;i<args;i++){
    //  Serial.printf("ARG[%s]: %s\n", request->argName(i).c_str(), request->arg(i).c_str());
    //}

    String key = request->getParam("key", true)->value();
    String value = request->getParam("value", true)->value();
    DPRINT(key); DPRINT(" = "); DPRINTLN(value);

    boolean paramFound = true;

    if (key == "utc_offset")    {
      prm.utc_offset = value.toInt();
    }
    else if (key == "set12_24") {
      prm.set12_24 = (value == "true");
      DPRINT("set12_24:");
      DPRINTLN(prm.set12_24);
    }
    else if (key == "showZero") {
      prm.showZero = (value == "true");
      DPRINT("showZero:");
      DPRINTLN(prm.showZero);
    }
    else if (key == "enableBlink") {
      prm.enableBlink = (value == "true");
    }
    else if (key == "enableDST")  {
      prm.enableDST = (value == "true");
    }
    else if (key == "interval")   {
      prm.interval = value.toInt();
    }
    else if (key == "enableAutoShutoff") {
      prm.enableAutoShutoff = (value == "true");
    }

    else if (key == "dayTimeHours")   {
      prm.dayHour = value.toInt();
    }
    else if (key == "dayTimeMinutes") {
      prm.dayMin = value.toInt();
    }
    else if (key == "nightTimeHours") {
      prm.nightHour = value.toInt();
    }
    else if (key == "nightTimeMinutes") {
      prm.nightMin = value.toInt();
    }
    else if (key == "dayBright")    {
      prm.dayBright = value.toInt();
    }
    else if (key == "nightBright")  {
      prm.nightBright = value.toInt();
    }
    else if (key == "animMode")     {
      prm.animMode = value.toInt();
    }
    else if (key == "manualOverride") {
      boolean v = value == "false";
      if (v != displayON) {
        manualOverride = true;
        displayON = v;
      }
    }
    else if (key == "alarmEnable")      {
      prm.alarmEnable = (value == "true");
    }
    else if (key == "alarmTimeHours")   {
      prm.alarmHour = value.toInt();
    }
    else if (key == "alarmTimeMinutes") {
      prm.alarmMin = value.toInt();
    }
    else if (key == "alarmPeriod")      {
      prm.alarmPeriod = value.toInt();
    }

    //RGB LED values
    else if (key == "rgbEffect")     {
      prm.rgbEffect = value.toInt();
    }
    else if (key == "rgbBrightness") {
      prm.rgbBrightness = value.toInt();
    }
    else if (key == "rgbFixColor")   {
      prm.rgbFixColor = value.toInt();
    }
    else if (key == "rgbSpeed")      {
      prm.rgbSpeed = value.toInt();
    }
    else if (key == "rgbDir")        {
      prm.rgbDir = (value == "true");
    }
    else if (key == "rgbMinBrightness") {
      c_MinBrightness = value.toInt();
    }

    else  {
      paramFound = false;
    }

    if (paramFound) {
      saveEEPROM();
      request->send(200, "text/plain", "Ok");
    }
    else {
      request->send(404, "text/plain", "404: Parameter not found");
    }
  }
  else {
    request->send(400, "text/plain", "400: Invalid Request. Parameters: key and value");
  }
}

void handleSendConfig(AsyncWebServerRequest *request) {
  StaticJsonDocument<1024> doc;
  char buf[20];  //conversion buffer

  DPRINTLN("Sending configuration to web client.");

  //Global data
  doc["version"] = webName;
  doc["maxDigits"] = maxDigits;   //number of digits (tubes)
  doc["maxBrightness"] = MAXBRIGHTNESS; //Maximum tube brightness usually 10, sometimes 12

  //Actual time and environment data
  sprintf(buf, "%4d.%02d.%02d", year(), month(), day());
  doc["currentDate"] = buf;
  sprintf(buf, "%02d:%02d", hour(), minute());
  doc["currentTime"] = buf;

  //useTemp = 2;  temperature[0] = 20; temperature[1] = 22;  //test only
  //useHumid = 2; humid[0] = 41; humid[1] = 42;
  
  if (useTemp > 0) {
    doc["temperature"] = temperature[0];  
  }
  else
    doc["temperature"] = 255;

if (useTemp > 1)
    doc["temperature2"] = temperature[1];
  else
    doc["temperature2"] = 255;

  if (useHumid>0) {
    doc["humidity"] = humid[0];
  }
  else
    doc["humidity"] = 255;
 
  if (useHumid>1)
    doc["humidity2"] = humid[1];
  else
    doc["humidity2"] = 255;

  //Clock calculation and display parameters
  doc["utc_offset"] = prm.utc_offset;
  doc["enableDST"] = prm.enableDST;         // Flag to enable DST (summer time...)
  doc["set12_24"] = prm.set12_24;           // Flag indicating 12 vs 24 hour time (false = 12, true = 24);
  doc["showZero"] = prm.showZero;           // Flag to indicate whether to show zero in the hour ten's place
  doc["enableBlink"] = prm.enableBlink;     // Flag to indicate whether center colon should blink
  doc["interval"] = prm.interval;           // doc["interval in minutes, with 0 = off

  //Day/Night dimmer parameters
  doc["enableAutoShutoff"] = prm.enableAutoShutoff;  // Flag to enable/disable nighttime shut off
  sprintf(buf, "%02d:%02d", prm.dayHour, prm.dayMin);
  doc["dayTime"] = buf;
  sprintf(buf, "%02d:%02d", prm.nightHour, prm.nightMin);
  doc["nightTime"] = buf;
  doc["dayBright"] = prm.dayBright;
  doc["nightBright"] = prm.nightBright;
  doc["animMode"] = prm.animMode;  //Tube animation

  doc["manualOverride"] = !displayON;

  //Alarm values
  doc["alarmEnable"] = prm.alarmEnable;   //1 = ON, 0 = OFF
  sprintf(buf, "%02d:%02d", prm.alarmHour, prm.alarmMin);
  doc["alarmTime"] = buf;
  doc["alarmPeriod"] = prm.alarmPeriod;

  //RGB LED values
#ifdef USE_NEOPIXEL
  doc["rgbEffect"] = prm.rgbEffect;       // if 255, no RGB exist!
#else
  doc["rgbEffect"] = 255;   //Not installed Neopixels!!!
#endif
  doc["rgbBrightness"] = prm.rgbBrightness; // c_MinBrightness..255
  doc["rgbFixColor"] = prm.rgbFixColor;   // 0..256
  doc["rgbSpeed"] = prm.rgbSpeed;       // 1..255
  doc["rgbDir"] = prm.rgbDir;          // 0 = right direction, 1 = left direction
  doc["rgbMinBrightness"] = c_MinBrightness;  //minimum brightness for range check!!

  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}

void handleSendCurrentInfos(AsyncWebServerRequest *request) {
  StaticJsonDocument<200> doc;
  //StaticJsonDocument<512> doc;
  char buf[20];  //conversion buffer

  DPRINTLN("WebRefresh");
  //Actual time and environment data
  sprintf(buf, "%4d.%02d.%02d", year(), month(), day());
  doc["currentDate"] = buf;
  sprintf(buf, "%02d:%02d", hour(), minute());
  doc["currentTime"] = buf;

  if (useTemp > 0) {
    doc["temperature1"] = temperature[0];
    doc["temperature"] = temperature[0];  //for compatibility with the old web page
  }
  else
    doc["temperature1"] = 255;

if (useTemp > 1)
    doc["temperature2"] = temperature[1];
  else
    doc["temperature2"] = 255;

  if (useHumid>0) {
    doc["humidity1"] = humid[0];
    doc["humidity"] = humid[0];  //for compatibility with the old web page
  }
  else
    doc["humidity1"] = 255;
 
  if (useHumid>1)
    doc["humidity2"] = humid[1];
  else
    doc["humidity2"] = 255;

  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}

void setup() {
  //WiFi.mode(WIFI_OFF);
  memset(pinTxt,0,sizeof(pinTxt));
  delay(200);
  DPRINTBEGIN(115200); DPRINTLN(" ");
  DPRINT("Starting "); DPRINTLN(webName);
  clearDigits();
  #if ALARMSPEAKER_PIN >= 0
    pinMode(ALARMSPEAKER_PIN, OUTPUT); regPin(ALARMSPEAKER_PIN,"ALARMSPEAKER_PIN");
    DPRINT("  - ON state:");
    if (ALARM_ON == HIGH) {DPRINTLN("HIGH"); }
    else {DPRINTLN("LOW"); }
  #endif
  #if ALARMBUTTON_PIN >= 0
    pinMode(ALARMBUTTON_PIN, INPUT_PULLUP); regPin(ALARMBUTTON_PIN,"ALARMBUTTON_PIN");
  #endif
  #if COLON_PIN >= 0
    pinMode(COLON_PIN, OUTPUT); regPin(COLON_PIN,"COLON_PIN");
  #endif
  #if LED_SWITCH_PIN >= 0
    pinMode(LED_SWITCH_PIN, OUTPUT); regPin(LED_SWITCH_PIN,"LED_SWITCH_PIN");
  #endif
  #if DECIMALPOINT_PIN >= 0
    pinMode(DECIMALPOINT_PIN, OUTPUT); regPin(DECIMALPOINT_PIN,"DECIMALPOINT_PIN");
  #endif
  #if RADAR_PIN >= 0
    pinMode(RADAR_PIN, INPUT);  regPin(RADAR_PIN,"RADAR_PIN");
    DPRINT("  - RADAR Timeout:");   DPRINTLN(RADAR_TIMEOUT);
  #endif
  #if TUBE_POWER_PIN >= 0
    pinMode(TUBE_POWER_PIN, OUTPUT); regPin(TUBE_POWER_PIN,"TUBE_POWER_PIN");
    digitalWrite(TUBE_POWER_PIN,TUBE_POWER_ON);
    DPRINT("  - ON state:"); 
    if (TUBE_POWER_ON == HIGH) {DPRINTLN("HIGH"); }
    else {DPRINTLN("LOW"); }
  #endif
  #if LIGHT_SENSOR_PIN >= 0
    pinMode(LIGHT_SENSOR_PIN, INPUT); regPin(LIGHT_SENSOR_PIN,"LIGHT_SENSOR_PIN");
    DPRINT("  - MAXIMUM_LUX for max.brightness:");  DPRINTLN(MAXIMUM_LUX);
    autoBrightness = true;
  #endif
  
  #if TEMP_DALLAS_PIN >= 0 
    setupDallasTemp();
  #endif
  #if TEMP_DHT_PIN >= 0
    setupDHTemp();
  #endif
  #ifdef USE_I2CSENSORS
    setupI2Csensors();
  #endif
  
  #ifdef USE_RTC
    setupRTC();
  #endif
  
  #ifdef USE_GPS
    setupGPS();
  #endif

  #ifdef USE_MQTT
    setupMqtt();
  #endif

  
  decimalpointON = false;    
  setup_pins();
  
  DPRINT("Number of digits:"); DPRINTLN(maxDigits);
  loadEEPROM();
  if (prm.magic != MAGIC_VALUE) factoryReset();

  setupNeopixel();
  listPins();
  
  getDHTemp();  //get the first DHT temp+humid measure
    
  byte saveMode = prm.animMode;
  prm.animMode = 2;
  testTubes(300);

  clearDigits();
  disableDisplay();
  Fdelay(200);
  if (!SPIFFS.begin()) {
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
  enableDisplay(0);
  //showMyIp();
  prm.animMode = saveMode;
  calcTime();
  timeProgram();
}

void calcTime() {
  mySTD.offset = prm.utc_offset * 60;
  myDST.offset = mySTD.offset;
  if (prm.enableDST) {
    myDST.offset += 60;
  }
  myTZ = Timezone(myDST, mySTD);

  //if (clockWifiMode) DPRINTLN("clockWifiMode=true");

  if (clockWifiMode) {
    if (WiFi.status() == WL_CONNECTED) {
      if (timeClient.update()) {
        timeserverErrors = 0;
        setTime(myTZ.toLocal(timeClient.getEpochTime()));
        if (year() > 2019)
          updateRTC();    //update RTC, if needed
        else {
          getRTC();
          getGPS();
        }
      } //endif update?
      else {
        timeserverErrors++;
        DPRINT("TS error. ");
        //DPRINT(180-timeserverErrors); DPRINTLN(" seconds clock will restart.");
        //  if (timeserverErrors>180) restartClock();  //restart clock
      }
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

  if ((millis() - lastRun) < 300) return;

  lastRun = millis();
  calcTime();
  if (useDallasTemp > 0) {
    requestDallasTemp(false);
    getTemp();
  }
  getDHTemp();
  getI2Csensors();
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
      protectTimer -= ((second() + 30) % 60 - 30);
      if (displayON && (millis() > 30000)) cathodeProtect();
    }
  }

  if (now() != prevTime) { // Update time every second
    prevTime = now();
    evalShutoffTime();     // Check whether display should be turned off (Auto shutoff mode)
    if (!displayON || !prm.enableBlink) colonBlinkState = false;
    else colonBlinkState = (bool)(second() % 2);
    
    showDate = ENABLE_CLOCK_DISPLAY && (second() >= DATE_START) && (second() < DATE_END);
    #ifdef DATE_REPEAT_MIN
      if (showDate) {
        int dateRepeatPush = 0;
        //if ((lastCathodeProt>=0) && (lastCathodeProt % DATE_REPEAT_MIN == 0)) {
        //  dateRepeatPush = 1;  //shift date display by 1 minute
        //  DPRINTLN("Shift Date display by 1 min");
        //}
        if ((DATE_REPEAT_MIN==0) || ((minute() % DATE_REPEAT_MIN) != dateRepeatPush)) {
          showDate = false;
        }
      }
    #endif
   
    showTemp0 = (useTemp > 0) && (second() >= TEMP_START) && (second() < TEMP_END);
    showTemp1 = (useTemp > 1) && (second() >= TEMP_START + (TEMP_END - TEMP_START) / 2) && (second() < TEMP_END);
    showHumid0 = (useHumid > 0) && (second() >= HUMID_START) && (second() < HUMID_END);
    showHumid1 = (useHumid >1) && (second() >= HUMID_START + (HUMID_END-HUMID_START)/2) && (second() < HUMID_END);
    
    if (maxDigits >= 8)      displayTime8();
    else if (maxDigits == 6) displayTime6();
    else                     displayTime4();

    //   if (COLON_PIN>=0) digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
    if ((LED_SWITCH_PIN >= 0) && displayON) //Switch on backlight LED only daytime
      digitalWrite(LED_SWITCH_PIN, HIGH);
    else
      digitalWrite(LED_SWITCH_PIN, LOW);
    printDigits(0);
    writeDisplaySingle();
  }
}

void loadEEPROM() {
  disableDisplay();
  DPRINT("Loading setting from EEPROM. EEPROM ver:");
  EEPROM.begin(sizeof(prm));
  EEPROM.get(EEPROM_addr, prm);
  EEPROM.end();
  DPRINTLN(prm.magic);
  enableDisplay(0);
}

void saveEEPROM() {
  disableDisplay();
  EEPROM.begin(sizeof(prm));
  EEPROM.put(EEPROM_addr, prm);    //(mod(mySTD.offset/60,24)));
  EEPROM.commit();
  EEPROM.end();
  DPRINTLN("Settings saved to EEPROM!");
  enableDisplay(0);
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
  byte dh1 = (hourShow / 10), dh2 = (hourShow % 10);
  byte dm1 = (minShow / 10),  dm2 = (minShow % 10);
  byte ds1 = (secShow / 10),  ds2 = (secShow % 10);

  DPRINTLN("Cathode Protect running!");
  // All four digits will increment up at 10 Hz.
  // At T=2 sec, individual digits will stop at
  // the correct time every second starting from
  // the right and ending on the left
  for (int i = 0; i <= 70; i++) {
    if (i >= 20) ds2 = (secShow % 10);
    if (i >= 30) ds1 = (secShow / 10);
    if (i >= 40) dm2 = (minShow % 10);
    if (i >= 50) dm1 = (minShow / 10);
    if (i >= 60) dh2 = (hourShow % 10);
    if (i == 70) dh1 = (hourShow / 10);
    if (maxDigits >= 8) {
      digit[7] = dh1;
      digit[6] = dh2;
      digit[5] = 11;
      digit[4] = dm1;
      digit[3] = dm2;
      digit[2] = 11;
      digit[1] = ds1;
      digit[0] = ds2;
    }
    else if (maxDigits == 6) {
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
  memcpy(oldDigit, digit, sizeof(oldDigit));
  lastCathodeProt = minute();
}

inline void incMod10(byte &x) {
  x = (x + 1 == 10 ? 0 : x + 1);
};


void displayTemp(byte ptr) {
  int digPtr = maxDigits-1;
  for (int i = 0; i < maxDigits; i++) {
    digitDP[i] = false;
    newDigit[i] = 10;
  }
  
  newDigit[digPtr] = int(temperature[ptr]) / 10;
  if (newDigit[digPtr] == 0) newDigit[digPtr] = 10; //BLANK!!!
  newDigit[--digPtr] = int(temperature[ptr]) % 10;
  digitDP[digPtr] = true;
  newDigit[--digPtr] = int(temperature[ptr] * 10) % 10;
  if ((maxDigits > 4) && (GRAD_CHARCODE >= 0)) {
    digPtr -=1;
    newDigit[digPtr] = GRAD_CHARCODE; //grad
  }
  if (TEMP_CHARCODE >= 0) newDigit[--digPtr] = TEMP_CHARCODE; //  "C"


  if (prm.animMode == 0)  memcpy(oldDigit, newDigit, sizeof(oldDigit)); //don't do animation
  colonBlinkState = true;
  decimalpointON = true;
}

void displayHumid(byte ptr) {
  int digPtr = maxDigits-1;
  for (int i = 0; i < maxDigits; i++) {
    digitDP[i] = false;
    newDigit[i] = 10;
  }
  
  newDigit[digPtr] = int(humid[ptr]) / 10;
  if (newDigit[digPtr] == 0) newDigit[digPtr] = 10; //BLANK if zero!!!
  newDigit[--digPtr] = int(humid[ptr]) % 10;
  digitDP[digPtr] = true;
  newDigit[--digPtr] = int(humid[ptr] * 10) % 10;
  if (maxDigits > 4) {
    if (digitsOnly) {
      newDigit[--digPtr] = 10;   //empty character
      newDigit[--digPtr] = PERCENT_CHARCODE;  //  "%"
    }
    else {
      newDigit[--digPtr] = 10;   //empty character
      newDigit[--digPtr] = 16;   //upper circle
      newDigit[--digPtr] = 18;  // lower circle
    }
  } //4 tubes only
  else {
    newDigit[--digPtr] = PERCENT_CHARCODE;  //  "%"
  }
     
  if (prm.animMode == 0)  memcpy(oldDigit, newDigit, sizeof(oldDigit)); //don't do animation
  colonBlinkState = true;
  decimalpointON = true;
}

void displayTime4() {
  for (int i = 0; i < maxDigits; i++) digitDP[i] = false;
  digitDP[4] = true;   digitDP[2] = true;
  int hour12_24 = prm.set12_24 ? (byte)hour() : (byte)hourFormat12();
  if      (showTemp1) displayTemp(1);
  else if (showTemp0) displayTemp(0);
  else if (showHumid1) displayHumid(1);
  else if (showHumid0) displayHumid(0);
  else if (showDate) {
    newDigit[3] = month() / 10;
    newDigit[2] = month() % 10;
    digitDP[2] = true;
    newDigit[1] = day() / 10;
    newDigit[0] = day() % 10;
    colonBlinkState = true;
  }
  else if (ENABLE_CLOCK_DISPLAY) {
    newDigit[3] = hour12_24 / 10;
    if ((!prm.showZero)  && (newDigit[3] == 0)) newDigit[3] = 10;
    newDigit[2] = hour12_24 % 10;
    newDigit[1] = minute() / 10;
    newDigit[0] = minute() % 10;
    if (prm.enableBlink && (second() % 2 == 0)) digitDP[2] = false;
  }
  changeDigit();
  writeDisplaySingle();
}

void displayTime6() {
  for (int i = 0; i < maxDigits; i++) digitDP[i] = false;
  digitDP[4] = true;   digitDP[2] = true;
  int hour12_24 = prm.set12_24 ? (byte)hour() : (byte)hourFormat12();
  if      (showTemp1) displayTemp(1);
  else if (showTemp0) displayTemp(0);
  else if (showHumid1) displayHumid(1);
  else if (showHumid0) displayHumid(0);
  else if (showDate) {
    newDigit[5] = (year() % 100) / 10;
    newDigit[4] = year() % 10;
    digitDP[4] = true;
    newDigit[3] = month() / 10;
    newDigit[2] = month() % 10;
    digitDP[2] = true;
    newDigit[1] = day() / 10;
    newDigit[0] = day() % 10;
    colonBlinkState = true;
  }
  else if (ENABLE_CLOCK_DISPLAY) {
    newDigit[5] = hour12_24 / 10;
    if ((!prm.showZero)  && (newDigit[5] == 0)) newDigit[5] = 10;
    newDigit[4] = hour12_24 % 10;
    newDigit[3] = minute() / 10;
    newDigit[2] = minute() % 10;
    if (prm.enableBlink && (second() % 2 == 0)){
      digitDP[2] = false;
      #ifdef DOUBLE_BLINK
        digitDP[4] = false;
      #endif 
    }
    newDigit[1] = second() / 10;
    newDigit[0] = second() % 10;
  }
  changeDigit();
  writeDisplaySingle();
}

void displayTime8() {
  for (int i = 0; i < maxDigits; i++) {
    digitDP[i] = false;
    newDigit[i] = 10;
  }
  int hour12_24 = prm.set12_24 ? (byte)hour() : (byte)hourFormat12();
  if      (showTemp1) displayTemp(1);
  else if (showTemp0) displayTemp(0);
  else if (showHumid1) displayHumid(1);
  else if (showHumid0) displayHumid(0);
  else {
    if (showDate) {
      newDigit[7] = year() / 1000;
      newDigit[6] = (year() % 1000) / 100;
      newDigit[5] = (year() % 100) / 10;
      newDigit[4] = year() % 10;
      digitDP[4] = true;
      newDigit[3] = month() / 10;
      newDigit[2] = month() % 10;
      digitDP[2] = true;
      newDigit[1] = day() / 10;
      newDigit[0] = day() % 10;
      colonBlinkState = true;
      if (prm.animMode == 1)  memcpy(oldDigit, newDigit, sizeof(oldDigit)); //don't do animation
    }
    else if (ENABLE_CLOCK_DISPLAY) {
      newDigit[8] = 10;  //sign digit = BLANK
      newDigit[7] = hour12_24 / 10;
      if ((!prm.showZero)  && (newDigit[7] == 0)) newDigit[7] = 10;
      newDigit[6] = hour12_24 % 10;
      newDigit[5] = 11;  //- sign
      newDigit[4] = minute() / 10;
      newDigit[3] = minute() % 10;
      newDigit[2] = 11;  //- sign
      if (prm.enableBlink && (second() % 2 == 0)) {
        digitDP[2] = 10; //BLANK
        #ifdef DOUBLE_BLINK
          digitDP[5] = 10; //BLANK
        #endif 
      }
      newDigit[1] = second() / 10;
      newDigit[0] = second() % 10;
    }
  }
  changeDigit();
  writeDisplaySingle();
}


void changeDigit() {
  int j = 0;
  byte tmp[50];
  byte space = 4;
  byte anim;

  anim = prm.animMode;
  //anim = 0;
  if (anim == 6) anim = 1 + rand() % 5;

  if (anim != 5) {
    for (int i = 0; i < maxDigits; i++)
      if ((newDigit[i] > 9) || ((oldDigit[i] > 9) && (newDigit[i] <= 9) )) {
        digit[i] = newDigit[i];    //show special characters ASAP or if special char changes to numbers
        oldDigit[i] = newDigit[i];
      }
    if ((maxDigits > 4) && (newDigit[0] != 0)) j = 1; //if 6 or 8 tube clock, dont play with seconds
  }

  if (memcmp(newDigit + j, oldDigit + j, maxDigits - j) != 0) {
    switch (anim) {
      case 1:
        for (int tube = j; tube < maxDigits; tube++) {
          if ((newDigit[tube] != oldDigit[tube]) && (newDigit[tube] <= 9)) {
            for (int i = oldDigit[tube]; i <= int(newDigit[tube] + 10); i++) {
              digit[tube] = i % 10;
              writeDisplaySingle();
              Fdelay(ANIMSPEED);
            } //end for i
            writeDisplaySingle();
          } //endif
        }  //end for tube
        break;
      case 2:
        for (int i = 0; i <= 9; i++) {
          for (int tube = j; tube < maxDigits; tube++) {
            if ((newDigit[tube] != oldDigit[tube]) && (newDigit[tube] <= 9))
              digit[tube] = (oldDigit[tube] + i) % 10;
          } //end for tube
          writeDisplaySingle();
          Fdelay(ANIMSPEED);
        }  //end for i
        break;
      case 3:
        memcpy(tmp, oldDigit, maxDigits);
        memcpy(tmp + maxDigits + space, newDigit, maxDigits);
        memset(tmp + maxDigits, 11, space); //----
        for (int i = 0; i < maxDigits + space; i++) {
          for (int tube = 0; tube < maxDigits; tube++) {
            digit[tube % maxDigits] = tmp[i + tube];
          } //end for tube
          writeDisplaySingle();
          Fdelay(ANIMSPEED);
        } //end for i
        break;
      case 4:
        memcpy(tmp, newDigit, maxDigits);
        memcpy(tmp + maxDigits + space, oldDigit, maxDigits);
        memset(tmp + maxDigits, 11, space); //----
        for (int i = maxDigits - 1 + space; i >= 0; i--) {
          for (int tube = 0; tube < maxDigits; tube++) {
            digit[tube % maxDigits] = tmp[i + tube];
          } //end for tube
          writeDisplaySingle();
          Fdelay(ANIMSPEED);
        } //end for i
        break;
      case 5:
        memset(animMask, 0, sizeof(animMask));
#if defined(MULTIPLEX74141) || defined(NO_MULTIPLEX74141)
        for (int i = 1; i < 10; i++) {
          for (int tube = j; tube < maxDigits; tube++) {
            if (oldDigit[tube] != newDigit[tube]) animMask[tube] = i;  //digit is changed
          }  //end for tube
          writeDisplaySingle();
          Fdelay(ANIMSPEED);
        }  //end for i
#else
        for (int i = 1; i <= 5; i++) {
          for (int tube = j; tube < maxDigits; tube++) {
            if (oldDigit[tube] != newDigit[tube]) animMask[tube] = i; //digit is changed
          }  //end for tube
          writeDisplaySingle();
          Fdelay(ANIMSPEED);
        }  //end for i
        memcpy(digit, newDigit, sizeof(digit));
        for (int i = 1; i <= 5; i++) {
          for (int tube = j; tube < maxDigits; tube++) {
            if (oldDigit[tube] != newDigit[tube]) animMask[tube] = 6 - i; //digit is changed
          }  //end for tube
          writeDisplaySingle();
          Fdelay(ANIMSPEED);
        }  //end for i
#endif
        memset(animMask, 0, sizeof(animMask));
        break;
    }
  } //endif memcmp

  memcpy(digit, newDigit, sizeof(digit));
  memcpy(oldDigit, newDigit, sizeof(oldDigit));
}


void alarmSound() {
  static const unsigned int t[] = {0, 3000, 6000, 6200, 9000, 9200, 9400, 12000, 12200, 12400, 15000, 15200, 15400};
  static int count = 0;
  static unsigned long nextEvent;
  const int cMax = sizeof(t) / sizeof(t[0]);  //number of time steps

  if (prm.alarmEnable) {
    if ( (!alarmON && prm.alarmHour == hour()) && (prm.alarmMin == minute()) && (second() <= 3)) {  //switch ON alarm sound
      DPRINTLN("Alarm started!");
      alarmON = true;
      alarmStarted = millis();
      count = 0;
      nextEvent = alarmStarted - 1;
    }
  }
  else {
    alarmON = false;
  }

  if (!alarmON) {
    if (ALARMSPEAKER_PIN >= 0) digitalWrite(ALARMSPEAKER_PIN, !ALARM_ON);
    return;  //nothing to do
  }

  if ((millis() - alarmStarted) > 1000 * (long)prm.alarmPeriod) {
    alarmON = false;   //alarm period is over
    darkenNeopixels();
    DPRINTLN("Alarm ended.");
  }

  if (ALARMBUTTON_PIN >= 0) { //is button installed?
    if (digitalRead(ALARMBUTTON_PIN) == LOW) { //stop alarm
      DPRINTLN("Alarm stopped by button press.");
      alarmON = false;
    }
  }

  if (!prm.alarmEnable || !alarmON) {  //no alarm, switch off
    if (ALARMSPEAKER_PIN >= 0) digitalWrite(ALARMSPEAKER_PIN, !ALARM_ON);
    return;
  }

  //-------- Generate sound --------------------------------

  if (millis() > nextEvent) { //go to the next event
    if (count % 2 == 0) {
      nextEvent += 150;
      if (ALARMSPEAKER_PIN >= 0) digitalWrite(ALARMSPEAKER_PIN, ALARM_ON);
      //DPRINT(" Sound ON");  DPRINT("  Next:"); DPRINTLN(nextEvent);
    }
    else {
      if (ALARMSPEAKER_PIN >= 0) digitalWrite(ALARMSPEAKER_PIN, !ALARM_ON);
      nextEvent = (count / 2 < cMax) ? alarmStarted +  t[count / 2] : nextEvent + 200;
      //DPRINT("   OFF"); DPRINT("  Next:"); DPRINTLN(nextEvent);
    }
    count++;
  }
}


void evalShutoffTime() {  // Determine whether  tubes should be turned to NIGHT mode

  if (!prm.enableAutoShutoff) return;

  int mn = 60 * hour() + minute();
  int mn_on = prm.dayHour * 60 + prm.dayMin;
  int mn_off = prm.nightHour * 60 + prm.nightMin;

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

#ifdef SHIFT_TUBES_LEFT_BY_1
  byte offset = 1;
#else
  byte offset = 0;
#endif

  memset(newDigit, 10, sizeof(newDigit));
  if (!digitsOnly && (maxDigits >= 6)) {
    newDigit[5] = 19;  //'I'
    newDigit[4] = 14;  //'P'
  }
  if (iptag >= 100) newDigit[2 + offset] = iptag / 100;
  newDigit[1 + offset] = (iptag % 100) / 10;
  newDigit[0 + offset] = iptag % 10;
  changeDigit();
  writeDisplaySingle();
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
  hours = fifteenMin / 4;
  minutes = (fifteenMin % 4) * 15;
}

void resetWiFi() {
  static unsigned long lastTest = millis();
  static unsigned int counter = 0;

  if ((millis() - lastTest) < 600000) return; //check in every 10 min  
  lastTest = millis();
  //disableDisplay();
  #if defined(ESP8266)  
    WiFi.reconnect();
  #endif  
  //enableDisplay(5000);
  DPRINTLN("Lost WiFi. Reconnect.");
  return;

  counter++;
  DPRINT("Wifi lost! Minutes to restart:"); DPRINTLN(180 - counter);
  if (counter < 180) return;  //3 hours passed
  //WiFiManager MyWifiManager;
  //MyWifiManager.resetSettings();    //reset all AP and wifi passwords...
  DPRINTLN("Restart Clock...");
  delay(1000);
  restartClock();
}

inline int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}


void testTubes(int dely) {
  Fdelay(dely);
  DPRINT("Testing tubes: ");
  for (int i = 0; i < 10; i++) {
    DPRINT(i); DPRINT(" ");
    for (int j = 0; j < maxDigits; j++) {
      newDigit[j] = i;
      digitDP[j] = i % 2;
    }
    changeDigit();
    writeDisplaySingle();
    Fdelay(dely);
  }
  DPRINTLN(" ");
  Fdelay(1000);
  memset(digitDP, 0, sizeof(digitDP));
}


void printSensors() {
  static unsigned long lastRun = millis();
  
  if ((millis() - lastRun) < 30000) return;
  lastRun = millis();  
  if (useTemp>0) {
    DPRINT("Temperature ("); DPRINT(useTemp); DPRINT("): ");
    for (int i=0;i<useTemp;i++) {
      DPRINT(temperature[i]); DPRINT(", ");
    }
    DPRINTLN(" ");
  }
  if (useHumid>0) {  
    DPRINT("Humidity    ("); DPRINT(useHumid); DPRINT("): ");
    for (int i=0;i<useHumid;i++) {
      DPRINT(humid[i]); DPRINT(", ");
    }
    DPRINTLN(" ");
  }
}

void printDigits(unsigned long timeout) {
  static unsigned long lastRun = millis();

#ifdef DEBUG
  if ((millis() - lastRun) < timeout) return;
  lastRun = millis();
  
  for (int i = maxDigits - 1; i >= 0; i--) {
    if (digit[i] < 10)      {DPRINT(digit[i]);}
    else if (digit[i]==10)  {DPRINT(" ");}
    else if (digit[i]==TEMP_CHARCODE)    {DPRINT("C");}
    else if (digit[i]==GRAD_CHARCODE)    {DPRINT("°");}
    else if (digit[i]==18)    {DPRINT(".");}
    else if (digit[i]==PERCENT_CHARCODE) {DPRINT("%"); }   
    else if (digit[i]==19) {DPRINT("I");}    
    else if (digit[i]==14) {DPRINT("P");}    
    else    DPRINT("-");
    
    if (digitDP[i]) {DPRINT(".");} else {DPRINT(" ");}
  }

  if (colonBlinkState) {DPRINT(" * ");}
  else  {DPRINT("   ");}
  /*
    if ((millis()/1000%10) == 1) {  //show free memory for debugging memory leak
    DPRINT("Heap:"); DPRINT(ESP.getFreeHeap()); DPRINT(" byte");
    #if defined(ESP8266)
      DPRINT(" Fragmentation:");  DPRINT(100 - ESP.getMaxFreeBlockSize() * 100.0 / ESP.getFreeHeap()); DPRINT("%");
    #endif
    }
  */
  //DPRINT("INT:"); DPRINT(intCounter);   //show multiplex interrupt counter
  //DPRINT(" ESaving:"); DPRINT(EEPROMsaving);
  #if LIGHT_SENSOR_PIN >=0
    DPRINT("  Lux:"); DPRINT(LuxValue);
    //DPRINT("  tON:"); DPRINT(timerON); DPRINT("  tOFF:"); DPRINT(timerOFF);   //Multiplex timing values for testing
  #endif  
  DPRINTLN(" ");
  printSensors();
  #endif  
}


void checkTubePowerOnOff() {

    static unsigned long lastRun = 0;
    static unsigned long lastON = 0;
    static boolean oldRadarON = true;
    
    if ((millis()-lastRun)<1000) return;
    lastRun = millis();

    #if RADAR_PIN >=0
      if (digitalRead(RADAR_PIN) == HIGH) lastON = millis(); 
      radarON = ((millis()-lastON)<long(1000*RADAR_TIMEOUT));
      if (radarON != oldRadarON) {
        oldRadarON = radarON;
        if (radarON) DPRINTLN("RADAR: Switching ON tubes.");
        else DPRINTLN("RADAR: Switching OFF tubes.");
      }
      //DPRINT("RadarON:"); DPRINT(radarON);  DPRINT(":"); DPRINTLN((millis()-lastON));
    #else
      radarON = true;  //without radar sensor, always ON 
    #endif
    #if TUBE_POWER_PIN >=0
      if (((displayON ?  prm.dayBright : prm.nightBright) == 0) || !radarON) {
        digitalWrite(TUBE_POWER_PIN,!TUBE_POWER_ON);  //Switch OFF
        //DPRINTLN("OFF");
      }
      else   {
        digitalWrite(TUBE_POWER_PIN,TUBE_POWER_ON);   //Switch ON
        //DPRINTLN("ON");
      }
    #endif   
}

#if LIGHT_SENSOR_PIN >=0
//Calculation parameters are defined in clocks.h
//https://www.pcboard.ca/ldr-light-dependent-resistor
int luxmeter() {      
  static float oldLux = MAXIMUM_LUX;
  float ADCdata;
  float ldrResistance;
  float ldrLux;
  
  ADCdata = analogRead(LIGHT_SENSOR_PIN); //DPRINT("ADC:"); DPRINTLN(ADCdata);
  ldrResistance = (MAX_ADC_READING - ADCdata) / ADCdata * REF_RESISTANCE;
  ldrLux = LUX_CALC_SCALAR * pow(ldrResistance, LUX_CALC_EXPONENT);
  if (ldrLux>MAXIMUM_LUX) ldrLux = MAXIMUM_LUX;   //Limited //Limit lux value to maximum
  oldLux = oldLux + (ldrLux-oldLux)/10;   //slow down Lux change
  return (int)oldLux;
}
#endif

void getLightSensor() {
  #if LIGHT_SENSOR_PIN >=0
    static unsigned long lastRun = 0;
    if ((millis()-lastRun)<500) return;
    lastRun = millis();
    LuxValue = luxmeter();
  #else
    LuxValue = MAXIMUM_LUX;
  #endif
}


void loop() {
  dnsServer.processNextRequest();
  //MDNS.update();
  enableDisplay(3000);
  timeProgram();
  writeDisplaySingle();
  doAnimationMakuna();
  alarmSound();
  checkTubePowerOnOff();
  getLightSensor();
  mqttSend();
  checkWifiMode();
  if (clockWifiMode) { //Wifi Clock Mode
    if (WiFi.status() != WL_CONNECTED) {
      resetWiFi();
    }
  }
  else {   //Manual Clock Mode
    editor();
  } //endelse
  yield();
}

void restartClock() {
#if defined(ESP8266)
  WiFi.setOutputPower(0);
#endif

  WiFi.disconnect();
  delay(1000);
  ESP.restart();
}

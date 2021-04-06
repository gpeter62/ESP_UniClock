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
  //#define USE_BH1750      //I2C Luxmeter sensor, SDA+SCL I2C pins are used!   
  //#define USE_RTC         //DS3231 realtime clock, SDA+SCL I2C pins are used!   
  //#define USE_GPS         //use for standalone clock, without wifi internet access
  //#define USE_NEOPIXEL    //WS2812B led stripe, for tubes backlight. Don't forget to define tubePixels[] !
  //#define USE_PWMLEDS     //WWM led driver on 3 pins for RG
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
  //#define RADAR_TIMEOUT 5     //Automatic switch off tubes (without radar detecting somebody) after xxx min
  //#define TUBE_POWER_PIN -1     //Filament or HV switch ON/OFF pin
  //#define TUBE_POWER_ON HIGH    //HIGH or LOW level is needed to switch ON the TUBE POWER?
  //#define LIGHT_SENSOR_PIN -1   //Environment light sensor, only ADC pins are usable! ESP32 for example: 34,35,36,39... 8266: only A0
  //#define REF_RESISTANCE    10000.0         // Resistor value is 10k, between LDR sensor and GND
  //#define MAXIMUM_LUX 100                   //Lux level for maximum tube brightness
  //#define LUX_CALC_SCALAR   12518931 * 1.2  //Calibrate here the LDR sensor
  //#define LUX_CALC_EXPONENT -1.405          //LDR sensor characteristic
  //#define PWM1_PIN -1  //red   channel if USE_PWMLEDS is defined
  //#define PWM2_PIN -1  //green channel if USE_PWMLEDS is defined
  //#define PWM3_PIN -1  //blue  channel if USE_PWMLEDS is defined
  //#define MAXBRIGHTNESS 10      //Do not change this value!

  //Display temperature and date in every minute between START..END seconds
  //#define ENABLE_CLOCK_DISPLAY true  //false, if no clock display is needed (for example: thermometer + humidity only)
  //#define SHIFT_TUBES_LEFT_BY_1   //shift leftIP address by 1 tube the display, if a thermometer is used with spec tube
  //#define LEFTDECIMAL false      //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  //#define DATE_REPEAT_MIN   1    //show date only every xxx minute. If zero, datum is never displayed!  
  //#define DOUBLE_BLINK          //both separator points are blinking (6 or 8 tubes VFD clock)
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
  //#define AP_NAME "UNICLOCK"  	//Access Point name
  //#define AP_PASSWORD ""	   		//AP password
  //#define WEBNAME "LED UniClock"  //Clock's name on the web page
*/
#define TIMESERVER_REFRESH 7200000     //2h, Refresh time in millisec   86400000 = 24h
unsigned long timeserverErrors = 0;        //timeserver refresh errors

int timerON=0;   //for debugging
int timerOFF=0;      
unsigned long intCounter = 0;   //for testing only, interrupt counter

byte c_MinBrightness = RGB_MIN_BRIGHTNESS;     //minimum LED brightness
byte c_MaxBrightness = RGB_MAX_BRIGHTNESS;     //maximum LED brightness

//--------------------------------------------------------------------------------------------------
#define FIRMWARE_SERVER "http://c.landventure.hu/store"
String usedPinsStr;
String driverSetupStr;

#if defined(ESP8266)
  #ifndef WEBNAME
    #define WEBNAME "ESP32_UniClock 3.0"
  #endif
  #ifndef AP_NAME
    #define AP_NAME "UNICLOCK"
  #endif
  #ifndef AP_PASSWORD
    #define AP_PASSWORD ""
  #endif
  #include <ESP8266WiFi.h>
  #include <ESP8266WiFiMulti.h>
  ESP8266WiFiMulti wifiMulti;
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
  #include <ESP8266httpUpdate.h>
  //#include <ESP8266mDNS.h>
  #include "ESPAsyncTCP.h"
  #include "FS.h"
  const char ESPpinout[] = {"OOOOOO      OOOOOI"};   //GPIO 0..5, 12..16, A0)  usable pins

#elif defined(ESP32)
  #ifndef WEBNAME
    #define WEBNAME "ESP32UniClock 3.0"
  #endif
  #ifndef AP_NAME
    #define AP_NAME "UNICLOCK32"
  #endif
  #ifndef AP_PASSWORD
    #define AP_PASSWORD ""
  #endif
  
  #include <esp_wifi.h>
  #include <WiFi.h>
  #include <WiFiMulti.h>
  WiFiMulti wifiMulti;
  #include <WiFiClient.h>
  #include <HTTPClient.h>
  #include <ESP32httpUpdate.h>
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
extern char tubeDriver[];

char webName[] = WEBNAME;
AsyncWebServer server(80);
//DNSServer dns;
#define CACHE_MAX_AGE "max-age=31536000" //maximum is: 31536000

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

#define MAGIC_VALUE 304   //EEPROM version

// 8266 internal pin registers
// https://github.com/esp8266/esp8266-wiki/wiki/gpio-registers
// example: https://github.com/mgo-tec/OLED_1351/blob/master/src/OLED_SSD1351.cpp


#ifdef DEBUG    //Macros are usually in all capital letters.
#define DPRINT(...)       Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
#define DPRINTLN(...)     Serial.println(__VA_ARGS__)   //DPRINT is a macro, debug print
#define DPRINTF(...)      Serial.printf(__VA_ARGS__)    //DPRINT is a macro, debug print
#define DPRINTBEGIN(...)  Serial.begin(__VA_ARGS__)     //DPRINTLN is a macro, debug print with new line
#else
#define DPRINT(...)     //now defines a blank line
#define DPRINTLN(...)   //now defines a blank line
#define DPRINTF(...)     //now defines a blank line
#define DPRINTBEGIN(...)   //now defines a blank line
#endif

#define PIN_OUT_SET PERIPHS_GPIO_BASEADDR + 4
#define PIN_OUT_CLEAR PERIPHS_GPIO_BASEADDR + 8

bool colonBlinkState = false;
boolean radarON = true;
boolean makeFirmwareUpdate = false;
unsigned long lastTimeUpdate = 0;  //last time refresh from GPS or internet timeserver
boolean RTCexist = false;
boolean GPSexist = false;
boolean BME280exist = false;
boolean BMP280exist = false;
boolean AHTX0exist = false;
boolean SHT21exist = false;
boolean BH1750exist = false;
boolean LDRexist = false;

byte useDallasTemp = 0;   //number of Dallas temperature sensors: 0,1,2
byte useTemp = 0;         //Total number of any temperature sensors: 0..6
byte usePress = 0;        //Total number of pressure sensors
byte useHumid = 0;        //Total number of humidity sensors
byte useLux = 0;
float temperature[6] = {0,0,0,0,0,0};
float humid[6] = {0,0,0,0,0,0};  
float pressur[6] = {0,0,0,0,0,0};  
int lx = 0;               //Enviroment LUX value, set by Light Sensor
boolean autoBrightness = false; //Enable automatic brightness levels
//----------------- EEPROM addresses -------------------------------------------------------------------
const int EEPROM_addr = 0;

struct {
//Main screen settings _____________________________________________________________________________________ 
  boolean alarmEnable = false;     //Yes or No
  byte alarmHour = 7;              //Alarm time
  byte alarmMin = 0;
  byte alarmPeriod = 15;           //Alarm length, sec
//RGB settings ____________________________________________________________________________________________
  byte rgbEffect = 1;              //0=OFF, 1=FixColor
  byte rgbBrightness = 100;        // 0..255
  unsigned int rgbFixColor = 150;  //0..255
  byte rgbSpeed = 50;              //0..255msec / step
  boolean rgbDir = false;          //false = right, true = left
//Wifi / ip settings _______________________________________________________________________________________ 
  boolean wifiMode = true; 
  char wifiSsid[20];
  char wifiPsw[20];
  char ApSsid[20];
  char ApPsw[20];
  char NtpServer[30];
  char mqttBrokerAddr[30]; 
  char mqttBrokerUser[20] = "mqtt";
  char mqttBrokerPsw[20] = "mqtt";
  int mqttBrokerRefresh = 10;  //sec
  boolean mqttEnable = false;
  char firmwareServer[80];
//Tube settings  ______________________________________________________________________________________
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
  byte dateMode = 2;               // 0:dd/mm/yyyy 1:mm/dd/yyyy 2:yyyy/mm/dd
  boolean tempCF = false;          //Temperature Celsius=false / Fahrenheit=true
  boolean enableTimeDisplay;       //ENABLE_CLOCK_DISPLAY
  byte dateStart;                  //Date is displayed start..end
  byte dateEnd;    
  byte tempStart;                  //Temperature display start..end
  byte tempEnd;  
  byte humidStart;                 //Humidity% display start..end
  byte humidEnd;
  byte dateRepeatMin;              //show date only every xxx minute. If zero, datum is never displayed!  
  boolean enableDoubleBlink;       //both separator points are blinking (6 or 8 tubes VFD clock)
  boolean enableAutoDim = false;   //Automatic dimming by luxmeter
  boolean enableRadar = false;     //Radar sensor
  int radarTimeout = 5;          //min
  float corrT0 = 0;
  float corrT1 = 0;
  float corrH0 = 0;
  float corrH1 = 0;
//____________________________________________________________________________________________________  
  int magic = MAGIC_VALUE;        //magic value, to check EEPROM version when starting the clock
} prm;

#define EEPROM_SIZE sizeof(prm)

//-------------------------------------------------------------------------------------------------------

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, prm.NtpServer, 0, TIMESERVER_REFRESH); // Refresh time in millisec
IPAddress ip;

// Set timezone rules.  Offsets set to zero, since they will be loaded from EEPROM
TimeChangeRule myDST = {"DST", Last, Sun, Mar, 2, 0};
TimeChangeRule mySTD = {"STD", First, Sun, Nov, 2, 0};
Timezone myTZ(myDST, mySTD);
time_t prevTime = 0;
time_t protectTimer = 0;
bool displayON = true;
bool manualOverride = false;
bool initProtectionTimer = false;  // Set true at the top of the hour to synchronize protection timer with clock
bool decimalpointON = false;
bool alarmON = false;             //Alarm in progress
unsigned long alarmStarted = 0;   //Start timestamp millis()

boolean showClock = false;
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
  #if defined(ESP32)
    usedPinsStr = String("ESP32 used pins:<br>");
  #else
    usedPinsStr = String("ESP8266 used pins:<br>");
  #endif  
  for (int i=0;i<=MAX_PIN;i++) {
    if (strlen(pinTxt[i])>0) {
      usedPinsStr += String(i) + ": " + String(pinTxt[i]) + "<br>";
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
  DPRINT("connect to Wifi network "); DPRINTLN(prm.wifiSsid);
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
    doAnimationPWM();
    alarmSound();
    #ifdef USE_GPS
      smartDelay(1);  //feed GPS
    #endif  
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
  //if (wifiMulti.run() != WL_CONNECTED) return;
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
    //esp_wifi_set_ps (WIFI_PS_NONE);  //power saving disable!
    esp_wifi_set_max_tx_power(78);     //Maximum output power
  #endif
  wifiMulti.addAP(prm.wifiSsid, prm.wifiPsw);
  //wifiMulti.addAP(DEFAULT_SSID, DEFAULT_WIFIPSW);
  wifiMulti.addAP("farm32", "birka12345");  //for testing
  DPRINTLN(" ");
  DPRINT("Connecting to WiFi ");
  int counter = 0;
  while (wifiMulti.run() != WL_CONNECTED) {
    DPRINT('.');
    counter++;
    delay(1000);
    if (counter>5) return;
  }
  DPRINTLN(" ");
/*
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
  */
  ip = WiFi.localIP();
  WiFi.setAutoReconnect(true);
  enableDisplay(100);
  showMyIp();
}

boolean updateTimefromTimeserver() {  //true, if successful
  boolean res = false;
  int count = 1;
  
  if (((millis()-lastTimeUpdate)<TIMESERVER_REFRESH) && (lastTimeUpdate!=0))
    return(res);

  if (WiFi.status() == WL_CONNECTED) {
    while (true) {
      DPRINT("Connecting to timeserver: "); DPRINTLN(count);
      enableDisplay(1000);
      res = timeClient.forceUpdate();
      if (res) {
        setTime(myTZ.toLocal(timeClient.getEpochTime()));
        lastTimeUpdate = millis();
        DPRINT("Timeserver date:"); DPRINT(year()); DPRINT("/"); DPRINT(month()); DPRINT("/"); DPRINT(day());      
        DPRINT(" time:");   DPRINT(hour()); DPRINT(":"); DPRINT(minute()); DPRINT(":"); DPRINTLN(second());  
        DPRINTLN("Clock refreshed from timeserver.");
      }
      count ++; 
      if (res || (count > 5)) break;
      writeIpTag(count);
      Fdelay(1000);
    } //end while
  }
  return (res);
}

void startStandaloneMode() {
  DPRINTLN("Starting Clock in Standalone Mode!");
  DPRINT("Clock's AP SSID:"); DPRINTLN(prm.ApSsid);
  DPRINTLN("IP: 192.168.4.1");
  IPAddress local_ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.mode(WIFI_AP);

  boolean nwState;
  if (strlen(prm.ApPsw)) {
    nwState =  WiFi.softAP(prm.ApSsid, prm.ApPsw);
  }
  else {
    nwState =  WiFi.softAP(prm.ApSsid);
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

void doFirmwareUpdate(){
    if (wifiMulti.run() != WL_CONNECTED) {
      DPRINTLN("Wifi disconnected. FirmwareUpdate failed.");
      return;
    }
    delay(2000);
    disableDisplay();
    yield();
    String fname = String(prm.firmwareServer)+"/"+String(FW)+".bin";
    DPRINT("Update firmware: "); DPRINTLN(fname);
    t_httpUpdate_return ret;
    boolean succ = false;
    ESPhttpUpdate.rebootOnUpdate(false);
    ret = ESPhttpUpdate.update(fname);

    switch(ret) {
            case HTTP_UPDATE_FAILED:
                DPRINTF("HTTP_UPDATE_FAILED Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                DPRINTLN("HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                DPRINTLN("HTTP_UPDATE_OK");
                delay(1000);
                succ = true;
                break;
    }
        
    fname = String(prm.firmwareServer)+"/"+String(FW)+".spiffs.bin";
    DPRINT("Update SPIFFS: "); DPRINTLN(fname);

    ret = ESPhttpUpdate.updateSpiffs(fname);
      if(ret == HTTP_UPDATE_OK) {

            switch(ret) {
                case HTTP_UPDATE_FAILED:
                    DPRINTF("HTTP_UPDATE_FAILED Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                    break;

                case HTTP_UPDATE_NO_UPDATES:
                    DPRINTLN("HTTP_UPDATE_NO_UPDATES");
                    break;

                case HTTP_UPDATE_OK:
                    DPRINTLN("HTTP_UPDATE_OK");
                    succ = true;
                    break;
            }
        }
    if (succ) doReset();
    DPRINTLN(" ");
}

void startServer() {
  DPRINTLN("Starting Async Webserver...");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    disableDisplay();
    DPRINTLN("Webserver: /  -> index.html");
    if(SPIFFS.exists("/index.html")) {
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html", "text/html"); 
      request->send(response);
    }
    else {
    request->send( 204, "text/html", "File Not Found" );
    DPRINTLN("/index.html not found");
    }
  });

  server.on("/jquery_351.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    disableDisplay();
    DPRINTLN("Webserver: /jquery_351.js");
    if (SPIFFS.exists("/jquery_351.js")) {
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/jquery_351.js", "text/js");   
      response->addHeader("Cache-Control", CACHE_MAX_AGE);
      request->send(response);
    }
    else {
      request->send( 204, "text/html", "File Not Found" );
      DPRINTLN("/jquery_351.js not found");
    }
  });

  server.on("/page.js", HTTP_GET, [](AsyncWebServerRequest * request) {

    disableDisplay();
    DPRINTLN("Webserver: /page.js");
    if (SPIFFS.exists("/page.js")) {
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/page.js", "text/js");   
      response->addHeader("Cache-Control", CACHE_MAX_AGE);
      request->send(response);
    }
    else {
      request->send( 204, "text/html", "File Not Found" );
      DPRINTLN("/page.js not found");
    }
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest * request) {
    disableDisplay();
    DPRINTLN("Webserver: /favicon.ico");
    if (SPIFFS.exists("/favicon.ico")){
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/favicon.ico", "image/png");
      response->addHeader("Cache-Control", CACHE_MAX_AGE);
      request->send(response);
    }
    else {
      request->send( 204, "text/html", "File Not Found" );
      DPRINTLN("/favicon.ico not found");
    }    
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

//____________________________________________________________________________
  server.on("/site.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    disableDisplay();
    DPRINTLN("Webserver: /site.css");
    if (SPIFFS.exists("/site.css")) {
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/site.css", "text/css");   
      response->addHeader("Cache-Control", CACHE_MAX_AGE);
      request->send(response);
    }
    else {
      request->send( 204, "text/html", "File Not Found" );
      DPRINTLN("/site.css not found");
    }     
  });
//____________________________________________________________________________
  server.on("/reset", HTTP_POST, [] (AsyncWebServerRequest *request) {
    DPRINTLN("/reset:");
    
    request->send(200, "text/plain", "Reset: Restarting the Box!");
    delay(200);
    doReset();
  });
//____________________________________________________________________________
  server.on("/factoryreset", HTTP_POST, [] (AsyncWebServerRequest *request) {
    DPRINTLN("/factoryreset:");

    request->send(200, "text/plain", "Factory Reset!");
    delay(200);
    factoryReset();
    doReset();
  });

//____________________________________________________________________________
  server.on("/firmwareupdate", HTTP_POST, [](AsyncWebServerRequest * request) {
    makeFirmwareUpdate = true;
  });

  server.on("/saveSetting", HTTP_POST, handleConfigChanged);
  server.on("/getConfiguration", HTTP_GET, handleSendConfig);
  server.on("/getCurrentInfos", HTTP_GET, handleSendCurrentInfos);
  server.on("/getClockDetails", HTTP_GET, handleSendClockDetails);
  server.onNotFound(handleNotFound);

  server.begin();

}  //end of procedure

void handleNotFound(AsyncWebServerRequest *request) {
  int params = request->params();
  for (int i = 0; i < params; i++) {
    #ifdef DEBUG
    AsyncWebParameter* p = request->getParam(i);
    if (p->isFile()) { //p->isPost() is also true
      Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
    } else if (p->isPost()) {
      Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    } else {
      Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    }
    #endif
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
    boolean oldDST = prm.enableDST;
    int old_utc_offset = prm.utc_offset;
    
    String key = request->getParam("key", true)->value();
    String value = request->getParam("value", true)->value();
    DPRINT(key); DPRINT(" = "); DPRINTLN(value);

    boolean paramFound = true;

    if (key == "utc_offset")    {
      prm.utc_offset = value.toInt();
      if (old_utc_offset != prm.utc_offset) {   //Change time zone
        setTime(now()+(prm.utc_offset-old_utc_offset)*3600);
      }
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
      if (oldDST && !prm.enableDST) {   //Switching off DST
        setTime(now()-3600);
      }
      if (!oldDST && prm.enableDST) {   //Switching on DST
        setTime(now()+3600);
      }
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
    else if (key == "wifiMode")      {
      prm.wifiMode = (value == "true");
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
    else if (key == "wifiSsid") {
      value.toCharArray(prm.wifiSsid,sizeof(prm.wifiSsid));
    }
    else if (key == "wifiPsw") {
      value.toCharArray(prm.wifiPsw,sizeof(prm.wifiPsw));
    }
    else if (key == "ApSsid") {
      for (int i=0;i<strlen(prm.ApSsid);i++) {  //repair bad chars in AP SSID
        if ((prm.ApSsid[i]<32) || (prm.ApSsid[i]>126)) prm.ApSsid[i]='_';
      }
      value.toCharArray(prm.ApSsid,sizeof(prm.ApSsid));
    }
    else if (key == "ApPsw") {
      value.toCharArray(prm.ApPsw,sizeof(prm.ApPsw));
    }
    else if (key == "NtpServer") {
      value.toCharArray(prm.NtpServer,sizeof(prm.NtpServer));
    }
    else if (key == "firmware") {
      value.toCharArray(prm.firmwareServer,sizeof(prm.firmwareServer));
      makeFirmwareUpdate = true; //for testing only
    }
      else if (key == "mqttBrokerAddr") {
        value.toCharArray(prm.mqttBrokerAddr,sizeof(prm.mqttBrokerAddr));
      }
      else if (key == "mqttBrokerUser") {
        value.toCharArray(prm.mqttBrokerUser,sizeof(prm.mqttBrokerUser));
      }
      else if (key == "mqttBrokerPsw") {
        value.toCharArray(prm.mqttBrokerPsw,sizeof(prm.mqttBrokerPsw));
      }         
      else if (key == "mqttBrokerRefresh") {
        prm.mqttBrokerRefresh = value.toInt();
      }     
      else if (key == "mqttEnable") {
        prm.mqttEnable = (value == "true");
      }  
    else if (key == "dateMode") {
      prm.dateMode = value.toInt();
    }
    else if (key == "dateStart") {
      prm.dateStart = value.toInt();
    }
    else if (key == "dateEnd") {
      prm.dateEnd = value.toInt();
    }
    else if (key == "tempStart") {
      prm.tempStart = value.toInt();
    }
    else if (key == "tempEnd") {
      prm.tempEnd = value.toInt();
    }                
    else if (key == "humidStart") {
      prm.humidStart = value.toInt();
    }
    else if (key == "humidEnd") {
      prm.humidEnd = value.toInt();
    }     
    else if (key == "dateRepeatMin") {
      prm.dateRepeatMin = value.toInt();
    } 
    else if (key == "enableDoubleBlink")  {
      prm.enableDoubleBlink = (value == "true");
    }
    else if (key == "enableTimeDisplay") {
      prm.enableTimeDisplay = (value == "true");
    }    
    else if (key == "enableAutoDim") {
      prm.enableAutoDim = (value == "true");
    }
    else if (key == "enableRadar")  {
      prm.enableRadar = (value == "true");
    }
    else if (key == "radarTimeout"){
      prm.radarTimeout = value.toInt();
      if (prm.radarTimeout>60) prm.radarTimeout = 60;
    }  
    else if (key == "tempCF") {
      prm.tempCF = (value == "true");
    }   
    else if (key == "corrT0") {
      prm.corrT0 = value.toInt();
    } 
    else if (key == "corrT1") {
      prm.corrT1 = value.toInt();
    } 
    else if (key == "corrH0") {
      prm.corrH0 = value.toInt();
    } 
    else if (key == "corrH1") {
      prm.corrH1 = value.toInt();
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
  StaticJsonDocument<2048> doc;
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
    doc["temperature"] = temperature[0] + prm.corrT0;  
  }
  else
    doc["temperature"] = 255;

if (useTemp > 1)
    doc["temperature2"] = temperature[1] + prm.corrT1;
  else
    doc["temperature2"] = 255;

  if (useHumid>0) {
    doc["humidity"] = humid[0] + prm.corrH0;
  }
  else
    doc["humidity"] = 255;
 
  if (useHumid>1)
    doc["humidity2"] = humid[1] + prm.corrH1;
  else
    doc["humidity2"] = 255;

  if (usePress>0) {
    doc["pressure"] = pressur[0];
  }
  else
    doc["pressure"] = 255;
    
  if (useLux>0) {
    doc["lux"] = lx;
  }
  else
    doc["lux"] = 255;

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
  #if defined(USE_NEOPIXEL) || defined(USE_PWMLEDS)
    doc["rgbEffect"] = prm.rgbEffect;   // if 255, no RGB exist!
  #else
    doc["rgbEffect"] = 255;   //Not installed Neopixels!!!
  #endif
  
  doc["rgbBrightness"] = prm.rgbBrightness; // c_MinBrightness..255
  doc["rgbFixColor"] = prm.rgbFixColor;   // 0..256
  doc["rgbSpeed"] = prm.rgbSpeed;       // 1..255
  doc["rgbDir"] = prm.rgbDir;          // 0 = right direction, 1 = left direction
  doc["rgbMinBrightness"] = c_MinBrightness;  //minimum brightness for range check!!
  doc["wifiMode"] = prm.wifiMode; 
  doc["wifiSsid"] = prm.wifiSsid;
  doc["wifiPsw"] = prm.wifiPsw;
  doc["ApSsid"] = prm.ApSsid;
  doc["ApPsw"] = prm.ApPsw;  
  doc["NtpServer"] = prm.NtpServer;
  doc["firmware"] = prm.firmwareServer;

  doc["mqttBrokerAddr"] = prm.mqttBrokerAddr;
  doc["mqttBrokerUser"] = prm.mqttBrokerUser;
  doc["mqttBrokerPsw"] = prm.mqttBrokerPsw;
  doc["mqttEnable"] = prm.mqttEnable;
  #if defined(USE_MQTT)
    doc["mqttBrokerRefresh"] = prm.mqttBrokerRefresh;  
  #else
    doc["mqttBrokerRefresh"] = 0;  
  #endif
  doc["dateMode"] = prm.dateMode; 
  doc["dateRepeatMin"] = prm.dateRepeatMin;   
  doc["tempCF"] = prm.tempCF;   
  doc["enableTimeDisplay"] = prm.enableTimeDisplay; 
  doc["enableDoubleBlink"] = prm.enableDoubleBlink;
  doc["dateStart"] = prm.dateStart; 
  doc["dateEnd"] = prm.dateEnd; 
  doc["tempStart"] = prm.tempStart; 
  doc["tempEnd"] = prm.tempEnd; 
  doc["humidStart"] = prm.humidStart; 
  doc["humidEnd"] = prm.humidEnd; 
  doc["enableAutoDim"] = prm.enableAutoDim; 
  #if RADAR_PIN>=0
    doc["enableRadar"] = prm.enableRadar;     
    doc["radarTimeout"] = prm.radarTimeout;   
  #else
    doc["radarTimeout"] = 0;   
  #endif
  doc["corrT0"] = prm.corrT0;
  doc["corrT1"] = prm.corrT1;
  doc["corrH0"] = prm.corrH0;
  doc["corrH1"] = prm.corrH1;
  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}

void handleSendClockDetails(AsyncWebServerRequest *request) {
  StaticJsonDocument<200> doc;
  //StaticJsonDocument<512> doc;
  String dat;

  DPRINTLN("SendClockDetails");
  dat = String("<br>FirmwareID:") + FW + "<br>";
  dat += String("  Tube driver:") + tubeDriver + "<br>";
  dat += String("  Mac:") + String(WiFi.macAddress()) + "<br>";
  dat += String("MAXBRIGHTNESS:") + String(MAXBRIGHTNESS) + "<br>";
  if (RTCexist) dat += String("RTC exist: YES <br>");
  if (useTemp>0) dat += String("Temperature sensors:") + useTemp + "<br>";
  if (useHumid>0) dat += String("Humidity sensors:") + useHumid + "<br>";
  if (usePress>0) dat += String("Pressure sensors:") + usePress + "<br>";
  dat += usedPinsStr + "<br>";
  dat += driverSetupStr + "<br>";
  DPRINTLN(dat);
  request->send(200, "text/html", dat);
}

void handleSendCurrentInfos(AsyncWebServerRequest *request) {
  StaticJsonDocument<400> doc;
  //StaticJsonDocument<512> doc;
  char buf[20];  //conversion buffer

  DPRINTLN("WebRefresh");
  //Actual time and environment data
  sprintf(buf, "%4d.%02d.%02d", year(), month(), day());
  doc["currentDate"] = buf;
  sprintf(buf, "%02d:%02d", hour(), minute());
  doc["currentTime"] = buf;

  if (useTemp > 0) {
    doc["temperature1"] = temperature[0] + prm.corrT0;
    doc["temperature"] = temperature[0] + prm.corrT0;  //for compatibility with the old web page
  }
  else
    doc["temperature1"] = 255;

if (useTemp > 1)
    doc["temperature2"] = temperature[1] + prm.corrT1;
  else
    doc["temperature2"] = 255;

  if (useHumid>0) {
    doc["humidity1"] = humid[0] + prm.corrH0;
    doc["humidity"] = humid[0] + prm.corrH0;  //for compatibility with the old web page
  }
  else
    doc["humidity1"] = 255;
 
  if (useHumid>1)
    doc["humidity2"] = humid[1] + prm.corrH1;
  else
    doc["humidity2"] = 255;
  
  if (usePress>0) {
    doc["pressure"] = pressur[0];
  }
  else
    doc["pressure"] = 255;

  if (useLux>0) {
    doc["lux"] = lx;
  }
  else
    doc["lx"] = 255;
    
  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}

void setup() {
  //WiFi.mode(WIFI_OFF);
  EEPROM.begin(EEPROM_SIZE);
  memset(pinTxt,0,sizeof(pinTxt));
  delay(1000);
  DPRINTBEGIN(115200); DPRINTLN(" ");
  DPRINTLN(F("================================================="));
  DPRINT("Starting "); DPRINTLN(webName);
  DPRINT("Firmware code:"); DPRINT(FW); DPRINT("   Tube driver:"); DPRINTLN(tubeDriver);
  DPRINT("MAXBRIGHTNESS:"); DPRINTLN(MAXBRIGHTNESS);
  DPRINTLN(F("================================================="));
  clearDigits();
  setup_pins();
  #if ALARMSPEAKER_PIN >= 0
    pinMode(ALARMSPEAKER_PIN, OUTPUT); regPin(ALARMSPEAKER_PIN,"ALARMSPEAKER_PIN");
    digitalWrite(ALARMSPEAKER_PIN,!ALARM_ON);
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
    DPRINT("  - RADAR Timeout:");   DPRINTLN(prm.radarTimeout);
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
    DPRINT("  - MAXIMUM_LUX for MAXBRIGHTNESS:");  DPRINTLN(MAXIMUM_LUX);
    useLux++;
    LDRexist = true;
  #endif

  #ifdef USE_PWMLEDS
    #if PWM1_PIN >= 0
      pinMode(PWM1_PIN, OUTPUT);  regPin(PWM1_PIN,"PWM1_PIN");
      ledcAttachPin(PWM1_PIN, 0);
      ledcSetup(0, 200, 8); // 12
    #endif
    #if PWM2_PIN >= 0
      pinMode(PWM2_PIN, OUTPUT);  regPin(PWM2_PIN,"PWM2_PIN");
      ledcAttachPin(PWM2_PIN, 1);
      ledcSetup(1, 200, 8); // 12
    #endif
    #if PWM3_PIN >= 0
      pinMode(PWM3_PIN, OUTPUT);  regPin(PWM3_PIN,"PWM3_PIN");
      ledcAttachPin(PWM3_PIN, 2);
      ledcSetup(2, 200, 8); // 12
    #endif
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

  decimalpointON = false;   
  DPRINT("Number of digits:"); DPRINTLN(maxDigits); 

  loadEEPROM();
  if (prm.magic != MAGIC_VALUE) factoryReset();

  setupNeopixel();
  listPins();
  writeAlarmPin(ALARM_ON); writeAlarmPin(!ALARM_ON);
  getDHTemp();  //get the first DHT temp+humid measure
    
  byte saveMode = prm.animMode;
  prm.animMode = 2;
  doAnimationPWM();
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
    DPRINTLN("SPIFFS started.");
  }

  if (prm.wifiMode) {
    startWifiMode();
    if (wifiMulti.run() != WL_CONNECTED) //failed to connect to wifi
      startStandaloneMode();
  }
  else 
    startStandaloneMode();
    
  delay(500);
  startServer();
  #ifdef USE_MQTT
    setupMqtt();
  #endif
  clearDigits();
  delay(200);
  enableDisplay(0);
  //showMyIp();
  prm.animMode = saveMode;
  timeClient.begin();
  calcTime();
  timeProgram();
}

void calcTime() {
  boolean refreshed = false;
  boolean refreshed2 = false;
  
 
  refreshed = updateTimefromTimeserver();  //update time from wifi
  if (refreshed) {
    mySTD.offset = prm.utc_offset * 60;
    myDST.offset = mySTD.offset;
    if (prm.enableDST) {
      myDST.offset += 60;
    }
    myTZ = Timezone(myDST, mySTD);  
    setTime(myTZ.toLocal(timeClient.getEpochTime()));
  }
      
  if (GPSexist) { //update time from GPS, if exist
    refreshed2 = getGPS();
  }
  
  if (RTCexist) {
    if (refreshed || refreshed2) 
      updateRTC();    //update RTC, if needed
    else 
      getRTC();
  }
}


void timeProgram() {
  static unsigned long lastRun = 0;
  
  if ((millis() - lastRun) < 200) return;
  lastRun = millis();

  calcTime();
  if (useDallasTemp > 0) {
    requestDallasTemp(false);  //start measure
    getTemp();                 //get result, if ready
  }
  getDHTemp();
  getI2Csensors();             //check all existing I2C sensors
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
    
    showClock = false;
    showDate = (prm.enableTimeDisplay && (prm.dateRepeatMin>0) && ((second() >= prm.dateStart) && (second() < prm.dateEnd)));
    if (showDate && (prm.dateRepeatMin>1)) {
      if ((minute() % prm.dateRepeatMin) != 0) {
        showDate = false;
      }
    }

    showTemp0 = (useTemp > 0) && (second() >= prm.tempStart) && (second() < prm.tempEnd);
    showTemp1 = (useTemp > 1) && (second() >= prm.tempStart + (prm.tempEnd - prm.tempStart) / 2) && (second() < prm.tempEnd);
    showHumid0 = (useHumid > 0) && (second() >= prm.humidStart) && (second() < prm.humidEnd);
    showHumid1 = (useHumid >1) && (second() >= prm.humidStart + (prm.humidEnd-prm.humidStart)/2) && (second() < prm.humidEnd);
    
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
  byte d;
  DPRINTLN("Loading setting from EEPROM.");
  DPRINT("Size:"); DPRINTLN(EEPROM_SIZE);
  EEPROM.get(EEPROM_addr, prm);
  DPRINT("EEPROM ver:"); DPRINTLN(prm.magic);
  enableDisplay(0);
}

void saveEEPROM() {
  disableDisplay();
  EEPROM.put(EEPROM_addr, prm);    
  EEPROM.commit();
  DPRINTLN("Settings saved to EEPROM!");
  enableDisplay(0);
}

void factoryReset() {
  DPRINTLN("Factory Reset!!!");
  prm.alarmEnable = false;
  prm.alarmHour = 7;
  prm.alarmMin = 0;
  prm.alarmPeriod = 15;
  prm.rgbEffect = 2;
  prm.rgbBrightness = 80;
  prm.rgbFixColor = 150;
  prm.rgbSpeed = 50;
  prm.rgbDir = 0;
  prm.wifiMode = true;
  strcpy(prm.wifiSsid,"");
  strcpy(prm.wifiPsw,"");
  for (int i=0;i<strlen(prm.ApSsid);i++) {  //repair bad chars in AP SSID
    if ((prm.ApSsid[i]<32) || (prm.ApSsid[i]>126)) prm.ApSsid[i]='_';
  }
  strncpy(prm.ApSsid,AP_NAME,sizeof(prm.ApSsid));
  strncpy(prm.ApPsw,AP_PASSWORD,sizeof(prm.ApPsw));
  strncpy(prm.NtpServer,"pool.ntp.org",sizeof(prm.NtpServer));
  strcpy(prm.mqttBrokerAddr,"10.10.0.202"); 
  strcpy(prm.mqttBrokerUser,"mqtt");
  strcpy(prm.mqttBrokerPsw,"mqtt");
  char tmp[100];
  strncpy(prm.firmwareServer,FIRMWARE_SERVER,sizeof(prm.firmwareServer));
  prm.mqttEnable = false;
  prm.mqttBrokerRefresh = 10; //sec
  prm.utc_offset = 1;
  prm.enableDST = false;          // Flag to enable DST (summer time...)
  prm.set12_24 = true;           // Flag indicating 12 vs 24 hour time (false = 12, true = 24);
  prm.showZero = true;           // Flag to indicate whether to show zero in the hour ten's place
  prm.enableBlink = true;        // Flag to indicate whether center colon should blink
  prm.interval = 15;             // prm.interval in minutes, with 0 = off
  prm.enableAutoShutoff = true;  // Flag to enable/disable nighttime shut off
  prm.dayHour = 7;
  prm.dayMin = 0;
  prm.nightHour = 20;
  prm.nightMin = 0;
  prm.dayBright = MAXBRIGHTNESS;
  prm.nightBright = 3;
  prm.animMode = 6;  
  prm.dateMode = 2;               // 0:dd/mm/yyyy 1:mm/dd/yyyy 2:yyyy/mm/dd
  prm.tempCF = false;               //Temperature Celsius / Fahrenheit
  prm.enableTimeDisplay = ENABLE_CLOCK_DISPLAY;
  prm.dateStart = DATE_START;                  //Date is displayed start..end
  prm.dateEnd = DATE_END;   
  prm.tempStart = TEMP_START;                  //Temperature display start..end
  prm.tempEnd = TEMP_END;  
  prm.humidStart = HUMID_START;                //Humidity% display start..end
  prm.humidEnd = HUMID_END;
  prm.dateRepeatMin = DATE_REPEAT_MIN;         //show date only every xxx minute. If zero, datum is never displayed!  
  #ifdef DOUBLE_BLINK
    prm.enableDoubleBlink = true;              //both separator points are blinking (6 or 8 tubes VFD clock)
  #else
    prm.enableDoubleBlink = false; 
  #endif  
  prm.enableAutoDim = false;          //Automatic dimming by luxmeter
  prm.enableRadar = false;            //Radar sensor
  prm.radarTimeout = 5;             //min
  prm.corrT0 = 0;
  prm.corrT1 = 0;
  prm.corrH0 = 0;
  prm.corrH1 = 0;  
  prm.magic = MAGIC_VALUE;              //magic value to check the EEPROM version
  saveEEPROM();
  calcTime();
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
    writeDisplaySingle();
    Fdelay(100);
  }
  memcpy(oldDigit, digit, sizeof(oldDigit));
  lastCathodeProt = minute();
}

inline void incMod10(byte &x) {
  x = (x + 1 == 10 ? 0 : x + 1);
};


void displayTemp(byte ptr) {
  float t = temperature[ptr];
  if (ptr==0) t += prm.corrT0;
  if (ptr==1) t += prm.corrT1;
  if (prm.tempCF) {
    t = round1((temperature[ptr] * 9/5)+32);
  }
  int digPtr = maxDigits-1;
  for (int i = 0; i < maxDigits; i++) {
    digitDP[i] = false;
    newDigit[i] = 10;
  }
  
  newDigit[digPtr] = t / 10;
  if (newDigit[digPtr] == 0) newDigit[digPtr] = 10; //BLANK!!!
  newDigit[--digPtr] = int(t) % 10;
  digitDP[digPtr] = true;
  newDigit[--digPtr] = int(t * 10) % 10;
  if ((maxDigits > 4) && (GRAD_CHARCODE >= 0)) {
    digPtr -=1;
    newDigit[digPtr] = GRAD_CHARCODE; //grad
  }
  if (TEMP_CHARCODE >= 0) {
    if (prm.tempCF) newDigit[--digPtr] = TEMP_CHARCODE+5; //  'F'
    else            newDigit[--digPtr] = TEMP_CHARCODE;   //  'C'
  }

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
  float h = humid[ptr];
  if (ptr==0) h += prm.corrH0;
  if (ptr==1) h += prm.corrH1;
  newDigit[digPtr] = int(h) / 10;
  if (newDigit[digPtr] == 0) newDigit[digPtr] = 10; //BLANK if zero!!!
  newDigit[--digPtr] = int(h) % 10;
  digitDP[digPtr] = true;
  newDigit[--digPtr] = int(h * 10) % 10;
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

void displayDate()  {
  byte m = prm.dateMode;
  if (m>2) m=2;
  byte p[3][8] = 
  {     //tube# 76543210    543210    3210
    {2,1,0},  //ddmmyyyy    ddmmyy    ddmm
    {2,0,1},  //mmddyyyy    mmddyy    mmdd
    {0,1,2}   //yyyymmdd    yymmdd    mmdd
  };  
  
  int t = 0;
  for (int i=0;i<3;i++) {
    if (p[m][i] == 2) {
      if (maxDigits>6) {
        newDigit[t++] = (year() % 1000) / 100;
        newDigit[t++] = year() / 1000;
      }
      if (maxDigits>4) {
        newDigit[t++] = year() % 10;
        newDigit[t++] = (year() % 100) / 10;
        digitDP[4] = true;
      }
    }
    if (p[m][i] == 1) {
      newDigit[t++] = month() % 10;
      newDigit[t++] = month() / 10;
    }
    if (p[m][i] == 0) {
      newDigit[t++] = day() % 10;
      newDigit[t++] = day() / 10;
    }
  } //end for
  digitDP[2] = true;
  colonBlinkState = true;
  if (prm.animMode == 1)  memcpy(oldDigit, newDigit, sizeof(oldDigit)); //don't do animation
}

void displayTime4() {
  for (int i = 0; i < maxDigits; i++) digitDP[i] = false;
  digitDP[4] = true;   digitDP[2] = true;
  int hour12_24 = prm.set12_24 ? (byte)hour() : (byte)hourFormat12();
  if      (showTemp1) displayTemp(1);
  else if (showTemp0) displayTemp(0);
  else if (showHumid1) displayHumid(1);
  else if (showHumid0) displayHumid(0);
  else if (showDate && prm.enableTimeDisplay) displayDate();
  else if (prm.enableTimeDisplay) {
    showClock = true;
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
  else if (showDate && prm.enableTimeDisplay)  displayDate();
  else if (prm.enableTimeDisplay) {
    showClock = true;
    newDigit[5] = hour12_24 / 10;
    if ((!prm.showZero)  && (newDigit[5] == 0)) newDigit[5] = 10;
    newDigit[4] = hour12_24 % 10;
    newDigit[3] = minute() / 10;
    newDigit[2] = minute() % 10;
    if (prm.enableBlink && (second() % 2 == 0)){
      digitDP[2] = false;
      if (prm.enableDoubleBlink) {
        digitDP[4] = false;
      }
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
  else if (showDate && prm.enableTimeDisplay) displayDate();
  else if (prm.enableTimeDisplay) {
      showClock = true;
      newDigit[8] = 10;  //sign digit = BLANK
      newDigit[7] = hour12_24 / 10;
      if ((!prm.showZero)  && (newDigit[7] == 0)) newDigit[7] = 10;
      newDigit[6] = hour12_24 % 10;
      newDigit[5] = 11;  //- sign
      newDigit[4] = minute() / 10;
      newDigit[3] = minute() % 10;
      newDigit[2] = 11;  //- sign
      if (prm.enableBlink && (second() % 2 == 0)) {
        newDigit[2] = 10; //BLANK
        if (prm.enableDoubleBlink) {
          newDigit[5] = 10; //BLANK
        } 
      }
      newDigit[1] = second() / 10;
      newDigit[0] = second() % 10;
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

void writeAlarmPin(boolean newState) {
  static boolean oldState = !ALARM_ON;  
  #if ALARMSPEAKER_PIN>=0
    if (oldState != newState) {
      oldState = newState;
      //if (newState == ALARM_ON) DPRINTLN("Alarm ON"); else DPRINTLN("Alarm OFF");
      digitalWrite(ALARMSPEAKER_PIN, newState);
    }
  #endif
}

void alarmSound(void) {
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
    writeAlarmPin(!ALARM_ON);
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
    writeAlarmPin(!ALARM_ON);
    return;
  }

  //-------- Generate sound --------------------------------

  if (millis() > nextEvent) { //go to the next event
    if (count % 2 == 0) {
      nextEvent += 500;
      writeAlarmPin(ALARM_ON);
      //DPRINT(" Sound ON");  DPRINT("  Next:"); DPRINTLN(nextEvent);
    }
    else {
      writeAlarmPin(!ALARM_ON);
      nextEvent = (count / 2 < cMax) ? alarmStarted +  t[count / 2] : nextEvent + 500;
      //DPRINT("   OFF"); DPRINT("  Next:"); DPRINTLN(nextEvent);
    }
    count++;
  }
}


void evalShutoffTime(void) {  // Determine whether  tubes should be turned to NIGHT mode

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

void showMyIp(void) {   //at startup, show the web page internet address
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

void resetWiFi(void) {
  static unsigned long lastTest = millis();
  static unsigned int counter = 0;

  if ((millis() - lastTest) < 600000) return; //check in every 10 min  
  lastTest = millis();
  DPRINTLN("Lost WiFi. Reconnect.");
  //disableDisplay();
  //#if defined(ESP8266)  
    WiFi.disconnect();
    WiFi.reconnect();
  //#endif  
  delay(3000);
  //if (WiFi.status() == WL_CONNECTED) lastTest = 
  //enableDisplay(5000);
  return;

  counter++;
  DPRINT("Wifi lost! Minutes to restart:"); DPRINTLN(180 - counter);
  if (counter < 180) return;  //3 hours passed
  //WiFiManager MyWifiManager;
  //MyWifiManager.resetSettings();    //reset all AP and wifi passwords...
  DPRINTLN("Restart Clock...");
  delay(1000);
  doReset();
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


void printSensors(void) {
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

  if ((millis() - lastRun) < timeout) return;
  lastRun = millis();
  
  #ifdef DEBUG
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
  if (useLux>0) {
    DPRINT("  Lux:"); DPRINT(lx);
  }
  //DPRINT("  tON:"); DPRINT(timerON); DPRINT("  tOFF:"); DPRINT(timerOFF);   //Multiplex timing values for testing
  //if (wifiMulti.run() != WL_CONNECTED) DPRINT("  no WIFI");
  if (WiFi.status() != WL_CONNECTED) DPRINT("  no WIFI");
  DPRINTLN(" ");
  printSensors();
  #endif  
}


void checkTubePowerOnOff(void) {
    static unsigned long lastRun = 0;
    static unsigned long lastON = 0;
    static boolean oldRadarON = true;
    
    if ((millis()-lastRun)<1000) return;
    lastRun = millis();

    #if RADAR_PIN >=0
      if (digitalRead(RADAR_PIN) == HIGH) lastON = millis(); 
      radarON = ((millis()-lastON)<60000l*prm.radarTimeout);
      if (radarON != oldRadarON) {
        oldRadarON = radarON;
        if (radarON) DPRINTLN("RADAR: Switching ON tubes.");
        else DPRINTLN("RADAR: Switching OFF tubes.");
      }
      //DPRINT("RadarON:"); DPRINT(radarON);  DPRINT(":"); DPRINTLN((millis()-lastON));
    #else
      radarON = true;  //without radar sensor, always ON 
    #endif
    
    if (!prm.enableRadar) radarON = true;  //If Radar is disabled, always ON !
    #if TUBE_POWER_PIN >=0
      if (((displayON ?  prm.dayBright : prm.nightBright) == 0) || !radarON) {
        digitalWrite(TUBE_POWER_PIN,!TUBE_POWER_ON);  //Switch OFF
        //DPRINTLN("POWER OFF");
      }
      else   {
        digitalWrite(TUBE_POWER_PIN,TUBE_POWER_ON);   //Switch ON
        //DPRINTLN("POWER ON");
      }
    #endif   
}


//Calculation parameters are defined in clocks.h
//https://www.pcboard.ca/ldr-light-dependent-resistor
 
int luxMeter(void) {    
  static float oldLux = MAXIMUM_LUX;
  float ADCdata;
  float ldrResistance;
  float ldrLux;
  
#if LIGHT_SENSOR_PIN >=0     
  ADCdata = analogRead(LIGHT_SENSOR_PIN); //DPRINT("ADC:"); DPRINTLN(ADCdata);
  ldrResistance = (MAX_ADC_READING - ADCdata) / ADCdata * REF_RESISTANCE;
  ldrLux = LUX_CALC_SCALAR * pow(ldrResistance, LUX_CALC_EXPONENT);
  if ((ldrLux>=MAXIMUM_LUX)||(ldrLux <0)) ldrLux = MAXIMUM_LUX;   //Limit lux value to maximum
  oldLux = oldLux + (ldrLux-oldLux)/10;   //slow down Lux change
  if (oldLux>=MAXIMUM_LUX-2) oldLux = MAXIMUM_LUX;
  return (int)oldLux;
#else
  return (0);
#endif
}

void getLightSensor(void) {
  static unsigned long lastRun = 0;
  static int oldLx = 0;
  int tmp;
  if ((millis()-lastRun)<500) return;
  lastRun = millis();

  if (BH1750exist) {
    tmp = getBH1750();
    if ((abs(tmp-oldLx)>1) || (tmp == MAXIMUM_LUX)) {
      lx=tmp; oldLx = lx; 
      autoBrightness = prm.enableAutoDim;
    }
  }
  else if (LDRexist) {
    tmp = luxMeter();
    if ((abs(tmp-oldLx)>1) || (tmp == MAXIMUM_LUX))  {
      lx=tmp;  oldLx = lx; 
    }
    autoBrightness = prm.enableAutoDim;
  }
  else {
    lx = MAXIMUM_LUX;
    autoBrightness = false;
  }
}

void checkWifiMode() {     
static boolean oldMode = prm.wifiMode;  

  if (oldMode != prm.wifiMode) {
    if (prm.wifiMode) {
        startWifiMode();
      }
      else  {
        startStandaloneMode();
      }
        oldMode = prm.wifiMode;  
    }  
}

void loop(void) {
  dnsServer.processNextRequest();
  //MDNS.update();
  enableDisplay(3000);
  timeProgram();
  writeDisplaySingle();
  doAnimationMakuna();
  doAnimationPWM();
  alarmSound();
  checkTubePowerOnOff();
  getLightSensor();
  if (prm.mqttEnable && prm.wifiMode) mqttSend();
  
  checkWifiMode();
  if (prm.wifiMode) { //Wifi Clock Mode
    if (WiFi.status() != WL_CONNECTED) {
      resetWiFi();
    }
  }
  else {   //Manual Clock Mode
    editor();
  } //endelse
  
  if (makeFirmwareUpdate) {
    makeFirmwareUpdate = false;
    doFirmwareUpdate();
  }
  yield();
}

void doReset(void) {
  DPRINTLN("Restart Clock...");
  #if defined(ESP8266)
    WiFi.setOutputPower(0);
  #endif

  WiFi.disconnect();
  delay(1000);
  ESP.restart();
}

/*
      Universal Clock  v3.xx (Nixie, VFD, LED, Numitron) for ESP8266 or ESP32
      with optional Dallas Thermometer and DS3231 RTC, Neopixel strip, GPS and more...
      03/04/2022
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

      Web page files, which are found in the "dat" subfolder, must be uploaded to SPIFFS !
      ESP8266:  https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide/
      ESP32: https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
*/
//====================================================================================================
// ESP32_UniClock2.ino - CORE file of the project. This is the ONE file shared by all ~65 physical
// clock builds (see clocks.h) - it contains everything that is NOT specific to how a digit gets
// written to the hardware: setup()/loop(), WiFi connection management + web admin server (REST-ish
// /configChanged API + JSON status endpoints), EEPROM-persisted settings (`prm` struct), time-source
// selection (NTP/GPS/RTC priority chain in calcTime()), the display/animation pipeline
// (displayTimeN()->changeDigit()->writeDisplaySingle()), alarm buzzer, day/night brightness,
// radar-based auto shutoff, light-sensor auto-dim, and cathode/element "protect" (anti burn-in) test
// patterns. The Arduino build concatenates every .ino file in this sketch folder into one compilation
// unit, and exactly one of the ~25 driver .ino files (e.g. MAX6921.ino, ZM1500.ino, wordclock.ino)
// actually compiles in per build, selected by whichever driver macro the active CLOCK_NN block in
// clocks.h defines - this core file calls that driver only through a small set of `extern` functions
// (setup_pins/writeDisplay/writeDisplaySingle/clearTubes) and shared global buffers (digit[]/newDigit[]
// /oldDigit[]/digitDP[]/animMask[]), so it never needs to know which physical driver is active.
// See docs/DEVELOPER_GUIDE.md for the full architecture writeup (this file is section 4) and
// docs/CODE_REVIEW.md for the list of known bugs/dead-code spots (also marked in-place below with
// "// POSSIBLE BUG:" / "// NOTE:").
//====================================================================================================
#include <stddef.h>
//#define byte uint8_t
#include "clocks.h"  //DEFINE YOUR CLOCKS SETUP IN THIS FILE!!! - clocks.h picks the single active CLOCK_NN block, which #defines the driver macro, pins and feature flags read throughout this file

// Detect which generation of the ESP8266 Arduino core is in use: older cores (2.6.x/2.7.x) name the
// RAM-resident-ISR attribute ICACHE_RAM_ATTR, while newer cores renamed it to IRAM_ATTR (shared with
// ESP32) - the #define below normalizes ICACHE_RAM_ATTR to mean IRAM_ATTR on newer cores so driver
// .ino files can use either name interchangeably regardless of installed core version.
#if defined(ESP8266)
  #if defined(ARDUINO_ESP8266_RELEASE_2_6_0) || \
    defined(ARDUINO_ESP8266_RELEASE_2_6_1) || \
    defined(ARDUINO_ESP8266_RELEASE_2_6_2) || \
    defined(ARDUINO_ESP8266_RELEASE_2_6_3) || \
    defined(ARDUINO_ESP8266_RELEASE_2_7_0) || \
    defined(ARDUINO_ESP8266_RELEASE_2_7_1) || \
    defined(ARDUINO_ESP8266_RELEASE_2_7_2) || \
    defined(ARDUINO_ESP8266_RELEASE_2_7_3) || \
    defined(ARDUINO_ESP8266_RELEASE_2_7_4)
    #define ESP8266_CORE_2xx
    #pragma message "8266_Core 2.xx"
  #else
    #pragma message "8266_Core_3.xx"
    #define ICACHE_RAM_ATTR IRAM_ATTR
  #endif
#endif
/*_______________________________ USABLE PARAMETERS ____________________________________________
  //#define DEBUG  //Enable Serial Monitor, 115200baud (only, if TX pin is not used anywhere!!!)
  //---------------------------- CLOCK EXTRA OPTION PARAMETERS ---------------------------------
  //#define USE_DALLAS_TEMP //TEMP_DALLAS_PIN is used to connect DS18B20 temperature sensors
  //#define USE_DHT_TEMP    //TEMP_DHT_PIN is the DHT sensor's data pin
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
  //#define USE_WIFIMANAGER //if no WiFi defined, use Wifimanager
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
  //#define SHIFT_TUBES_LEFT_BY_1   //shift the display left by 1 tube, if a thermometer is used with a special tube
  //#define LEFTDECIMAL false      //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  //#define DATE_REPEAT_MIN   1    //show date only every xxx minute. If zero, the date is never displayed!
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
  //#define AP_NAME "UNICLOCK"    //Access Point name
  //#define AP_PASSWORD ""        //AP password
  //#define WEBNAME "LED UniClock"  //Clock's name on the web page
  //#define DEFAULT_SSID ""       //factoryReset default value for WiFi
  //#define DEFAULT_PSW ""        //factoryReset default value  for WiFi password
  //#define FACTORYRESET_PIN -1   //if defined, do a factory reset if the button is pushed
*/
//#define USE_MDNS
#define TIMESERVER_REFRESH 7200000     //2h, Refresh time in millisec   86400000 = 24h
unsigned long timeserverErrors = 0;        //timeserver refresh errors

int timerON=0;   //for debugging
int timerOFF=0;
unsigned long intCounter = 0;   //for testing only, interrupt counter

byte c_MinBrightness = RGB_MIN_BRIGHTNESS;     //minimum LED brightness
byte c_MaxBrightness = RGB_MAX_BRIGHTNESS;     //maximum LED brightness
//--------------------------------------------------------------------------------------------------
#define FIRMWARE_SERVER "http://c.landventure.hu/store"   //default OTA update server (see doFirmwareUpdate()); overridable per-clock via prm.firmwareServer / the web admin "firmware" field
String usedPinsStr;      //human-readable list of registered GPIOs, built by listPins(), shown on the web admin "clock details" page
String driverSetupStr;   //driver-specific extra setup info string (set by some drivers), also shown on the web admin page

// Platform-specific setup: WiFi/HTTP/filesystem headers, board defaults (web page title / AP name),
// DRAM_ATTR (only meaningful on ESP32, defined empty here so ESP8266 code using it still compiles),
// and the ASCII-art `ESPpinout[]` string used by regPin() below to sanity-check GPIO usage per pin
// (each character is one GPIO: 'O'=usable output/IO pin, 'I'=input-only pin, ' '=reserved/unusable).
#if defined(ESP8266)
  #define DRAM_ATTR     //https://docs.espressif.com/projects/esp-idf/en/v4.3-beta2/esp32/api-reference/storage/spi_flash_concurrency.html
  #ifndef WEBNAME
    #define WEBNAME "ESP32_UniClock 3.1"
  #endif
  #ifndef AP_NAME
    #define AP_NAME "UNICLOCK"
  #endif
  #ifndef AP_PASSWORD
    #define AP_PASSWORD ""
  #endif
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
  #include "ESPAsyncTCP.h"
  #include "FS.h"
  #include <ESP8266httpUpdate.h>
  #ifdef USE_MDNS
    #include <ESP8266mDNS.h>
  #endif  

  
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
  #include <WiFi.h>  
  #include <esp_wifi.h>
  #include <WiFiClient.h>
  #include <HTTPClient.h>
  #include <ESP32httpUpdate.h>

  #include "AsyncTCP.h"
  #include "SPIFFS.h"
  #include "FS.h"  
  #include "soc/soc.h"
  #include "soc/rtc_cntl_reg.h"
  #ifdef USE_MDNS
    #include <ESPmDNS.h>
  #endif
  hw_timer_t * ESP32timer = NULL;        //hardware timer handle, started by startTimer() below; drives the ISR-based multiplex drivers' writeDisplay()
  portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;   //spinlock used by drivers to guard shared state accessed from both the ISR and the main loop
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
#include <EEPROM.h>
#include "ArduinoJson.h"

#if defined(USE_NEOPIXEL) || defined(WORDCLOCK)
  #include <NeoPixelBrightnessBus.h>
#endif

// Driver-interface contract: every driver .ino file must implement these regardless of which chip it
// targets (see docs/DEVELOPER_GUIDE.md section 3) - this is the entire surface the core code needs to
// drive an arbitrary physical display. writeDisplay() is the ISR-based multiplex refresh (a no-op for
// "smart" chip drivers that scan internally, e.g. PT6311/PT6355/VQC10); writeDisplaySingle() is a
// blocking, non-ISR, one-shot render of the current digit[]/digitDP[] buffers.
extern void ICACHE_RAM_ATTR writeDisplay();
extern void writeDisplaySingle();
extern void setup_pins();
extern void clearTubes();
extern int maxDigits;      //number of physical digit positions (tubes) on this clock build
extern char tubeDriver[];  //human-readable driver name string, shown on the web admin page

char webName[] = WEBNAME;
AsyncWebServer server(80);
#define CACHE_MAX_AGE "max-age=31536000" //maximum is: 31536000

#ifdef USE_WIFIMANAGER
  #include <ESPAsyncWiFiManager.h>
  AsyncWiFiManager myWiFiManager(&server,&dnsServer);
#endif

//______________ WiFi finding variables ____________________________________________
byte bestChn;         //the recent found best wifi AP's channel No.
uint8_t bestBssid[6]; //the recent found best wifi AP's BSSID (mac address)
uint8_t oldBssid[6];  //the current wifi AP's BSSID (mac address)
int bestRSSI = -200;  //the recent found best wifi AP's RSSI value
unsigned long lastWifiScan = 0;  //last wifi scan timestamp in CPU msec millis()
boolean wifiOK = false;
char myIp[20]="";

//Display buffers - shared with every driver .ino file, indexed 0..maxDigits-1 (0 = rightmost/seconds digit).
//Value encoding (see docs/DEVELOPER_GUIDE.md section 3): 0-9 = digit to show, 10 = BLANK, 11 = dash/minus,
//12 = standalone decimal-point glyph (a few drivers only), 13+ = driver-specific extra characters
//(letters, degree sign, %, circles, etc. - each driver's own charDefinition[]/charTable[] maps these).
#define BUFSIZE 12
byte DRAM_ATTR digit[BUFSIZE];        //what is CURRENTLY on the physical display (what writeDisplay()/writeDisplaySingle() actually render)
byte DRAM_ATTR newDigit[BUFSIZE];     //the target value for the next display update, filled in by displayTime4/6/8()/displayDate()/displayTemp()/displayHumid()
byte DRAM_ATTR oldDigit[BUFSIZE];     //the value that was displayed before the current changeDigit() animation started - used to detect per-tube changes and drive the various animMode animations
boolean DRAM_ATTR digitDP[BUFSIZE];   //actual value to put to display
boolean digitsOnly = true;  //only 0..9 numbers are possible to display?
byte DRAM_ATTR animMask[BUFSIZE];     //0 = no animation mask is used; used by the "wipe" animation (animMode==5, see changeDigit()) - a driver-specific progress value per tube, drivers read this to fade/wipe segments in or out

volatile boolean EEPROMsaving = false; //saving in progress - stop display refresh

// EEPROM/prm struct format version. Bump this whenever the `prm` struct below changes shape (fields
// added/removed/reordered/resized) - loadEEPROM() compares the stored value against this constant in
// setup(), and a mismatch triggers an automatic factoryReset() (old EEPROM bytes would otherwise be
// misinterpreted as the new struct layout, since EEPROM.get()/put() do a raw memcpy with no versioning
// of their own).
#define MAGIC_VALUE 304   //EEPROM version


//#define THERMOMETER_CLOCK     //it means, first digit is +- sign, last digit is C/F, 4 digit is for numbers
#ifdef THERMOMETER_CLOCK    //it means 6 tubes:  first digit is +- sign, last digit is C/F, and only 4 digit are for numbers
  boolean thermometerClock = true;
#else
  boolean thermometerClock = false;
#endif

// 8266 internal pin registers
// https://github.com/esp8266/esp8266-wiki/wiki/gpio-registers
// example: https://github.com/mgo-tec/OLED_1351/blob/master/src/OLED_SSD1351.cpp


// Debug print macros: compile to real Serial.print calls when DEBUG is defined in clocks.h for the
// active CLOCK_NN, otherwise compile to nothing (zero code/RAM cost) - used everywhere in this file
// and in every driver .ino instead of calling Serial.* directly, so a release build can drop all
// debug logging just by commenting out one #define.
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
boolean mqttRadarON = true;
unsigned long radarLastOn = 0;
boolean makeFirmwareUpdate = false;
boolean makeCathodeProtect = false;
int cathProtMin = 5;
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
int lx = 0;               //Environment LUX value, set by Light Sensor
boolean autoBrightness = false; //Enable automatic brightness levels
//----------------- EEPROM addresses -------------------------------------------------------------------
const int EEPROM_addr = 0;

// `prm` - the ONE struct holding every user-configurable, persistent setting (alarm, RGB effect, WiFi
// credentials, MQTT broker, brightness/day-night schedule, date/temp/humidity display windows,
// calibration offsets, etc). Saved/loaded as a single raw block via EEPROM.put()/EEPROM.get()
// (saveEEPROM()/loadEEPROM() below) - NOT field-by-field, so struct layout changes require bumping
// MAGIC_VALUE (see above) or old EEPROM contents get reinterpreted as garbage. The web admin's
// POST /configChanged (handleConfigChanged()) is the single entry point that mutates these fields at
// runtime; handleSendConfig() serializes (most of) this struct back out to the client as JSON.
// In-struct default values (e.g. "= 7") are only the compiled-in fallback used until the first
// factoryReset()/loadEEPROM() actually runs - factoryReset() re-assigns most fields explicitly.
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
//WiFi / IP settings _______________________________________________________________________________________
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
  char firmwareServer[78];
//Tube settings  ______________________________________________________________________________________
  int tempRepeatMin = 1;             //temperature and humidity display repeat (min)
  int utc_offset = 1;              //time zone offset
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
  byte dateRepeatMin;              //show date only every xxx minute. If zero, the date is never displayed!
  boolean enableDoubleBlink;       //both separator points are blinking (6 or 8 tubes VFD clock)
  boolean enableAutoDim = false;   //Automatic dimming by luxmeter
  boolean enableRadar = false;     //Radar sensor
  int radarTimeout = 5;          //min
  float corrT0 = 0;
  float corrT1 = 0;
  float corrH0 = 0;
  float corrH1 = 0;
//____________________________________________________________________________________________________
  int magic = MAGIC_VALUE;        //magic value, to check EEPROM version when starting the clock - see loadEEPROM()/factoryReset() and the MAGIC_VALUE comment above
} prm;

#define EEPROM_SIZE sizeof(prm)   //EEPROM.begin() is sized exactly to the struct - no extra reserved space for future growth, so any struct change is effectively a breaking EEPROM-layout change (mitigated only by the magic-number factory-reset above)

//-------------------------------------------------------------------------------------------------------

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, prm.NtpServer, 0, TIMESERVER_REFRESH); // Refresh time in millisec
IPAddress ip;

// Set timezone rules.  Offsets set to zero, since they will be loaded from EEPROM
// DST rule is HARD-CODED to the EU convention (last Sunday of March -> first Sunday of November) -
// prm.utc_offset/prm.enableDST only scale the offset (see updateTimefromTimeserver()), the actual
// switchover DATES are not configurable from the web UI. myTZ is rebuilt with the current offsets
// every time updateTimefromTimeserver() runs (WiFi/NTP path only - the GPS-only fallback in gps.ino
// uses a simpler, always-on +1h DST rule, see CODE_REVIEW.md).
TimeChangeRule myDST = {"DST", Last, Sun, Mar, 2, 0};
TimeChangeRule mySTD = {"STD", First, Sun, Nov, 2, 0};
Timezone myTZ(myDST, mySTD);
time_t prevTime = 0;      //last second() value that was actually rendered to the display - timeProgram() only redraws when now()!=prevTime, i.e. at most once per real second
time_t protectTimer = 0;  //timestamp of the last cathode-protect animation, used by timeProgram()'s prm.interval-minute scheduler
bool displayON = true;         //current day/night brightness state (true=day brightness) - set by evalShutoffTime(), or forced by manualOverride via the web UI
bool manualOverride = false;    //user has manually forced displayON via the web "manualOverride" key, overriding the automatic day/night schedule until evalShutoffTime() sees the schedule naturally cross into the opposite state again
bool initProtectionTimer = false;  // Set true at the top of the hour to synchronize protection timer with clock
bool decimalpointON = false;
bool alarmON = false;             //Alarm in progress
unsigned long alarmStarted = 0;   //Start timestamp millis()

// Which "screen" timeProgram() decided to render this second - set once per second, then read by
// displayTime4/6/8() to pick between clock/date/temperature/humidity output. Mutually exclusive
// (the display*() dispatch chain checks them in priority order: temp1 > temp0 > humid1 > humid0 > date > clock).
boolean showClock = false;
boolean showDate = false;
boolean showTemp0 = false;
boolean showTemp1 = false;
boolean showHumid0 = false;
boolean showHumid1 = false;
int lastCathodeProt = -1;          //minute() value when the cathode-protect animation last ran, informational only (not read elsewhere in this file)
boolean cathodeProtRunning = false;  //true while any cathode-protect animation is active - other code (e.g. wordclock.ino) can check this to skip normal rendering
boolean ipShowRunning = false;       //true while showMyIp() is running at boot (IP address shown digit-by-digit on the tubes)
boolean wifiConnectRunning = false;  //true while setup() is attempting the initial WiFi connection
boolean editorRunning = false;       //true while the RTC manual date/time editor (rtc.ino) is active - read by wordclock.ino to show an "editing" indicator

#define MAX_PIN sizeof(ESPpinout)-1
#define PIN_TXT_LEN 30
char pinTxt[MAX_PIN][PIN_TXT_LEN+1];

// Every driver/peripheral setup call is expected to call this once per GPIO it uses (label is a
// human-readable name, e.g. "PIN_SDA"). Purpose is twofold: (1) sanity-check the pin against the
// board's ESPpinout[] map (flags reserved/input-only/nonexistent pins) and (2) detect two features
// accidentally configured onto the same physical pin (a common clocks.h copy-paste mistake) - both
// classes of problem are only reported via DPRINT, not by refusing to continue, so a build with DEBUG
// off will silently run with a bad pin assignment.
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
    strncpy(pinTxt[p],"Error:MULTI DEF",PIN_TXT_LEN);
  }
  else 
    strncpy(pinTxt[p],txt,sizeof(pinTxt[p]));
}

// Builds usedPinsStr (the HTML fragment shown on the web admin "clock details" page, see
// handleSendClockDetails()) from everything regPin() recorded, and also dumps the same list to the
// debug console.
void listPins() {
  DPRINTLN("_______________________");
  DPRINTLN("___ USED CLOCK PINS ___");
  #if defined(ESP32)
    usedPinsStr = String("ESP32 used pins:<br>");
  #else
    usedPinsStr = String("ESP8266 used pins:<br>");
  #endif  
  for (byte i=0;i<=MAX_PIN;i++) {
    if (strlen(pinTxt[i])>0) {
      usedPinsStr += String(i) + ": " + String(pinTxt[i]) + "<br>";
      DPRINT(i); DPRINT(": ");  DPRINTLN(pinTxt[i]);
    }
  }
  DPRINTLN("_______________________");
  //DPRINT("MAX_PIN"); DPRINTLN(MAX_PIN);
}

// Starts the hardware-timer interrupt that periodically calls the active driver's writeDisplay() (the
// ISR-based multiplex refresh). Called from most drivers' setup_pins() - drivers that scan their
// display internally (PT6311/PT6355/VQC10/MAX7219/etc, see docs/DEVELOPER_GUIDE.md section 3) never
// call this, since their writeDisplay() is an empty no-op and refresh instead happens cooperatively
// in writeDisplay2()/writeDisplaySingle().
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

#ifdef USE_WIFIMANAGER
void configModeCallback (AsyncWiFiManager *myWiFiManager) {
  DPRINTLN("Switch to AP config mode...");
  DPRINTLN("To configure Wifi,  ");
  DPRINT("connect to Wifi network "); DPRINTLN(prm.wifiSsid);
  DPRINTLN("and open 192.168.4.1 in web browser");
}
#endif

// Resets all three digit buffers to BLANK (10) and clears every decimal-point flag - a full display
// clear, used at startup and around mode transitions (WiFi connect, IP display, etc).
void clearDigits() {
  memset(oldDigit, 10, sizeof(oldDigit));
  memset(digit, 10, sizeof(digit));
  memset(newDigit, 10, sizeof(newDigit));
  memset(digitDP, 0, sizeof(digitDP));
}

// A cooperative-multitasking substitute for delay(d): the firmware has no delay()-based blocking
// waits in loop() (see docs/DEVELOPER_GUIDE.md section 4.2), so any function that needs to "wait"
// (animations, WiFi connect retries, cathode-protect, IP display, etc.) calls Fdelay() instead - it
// busy-waits for `d` milliseconds but keeps servicing the DNS captive-portal server (AP mode only),
// RGB/PWM background animations, the alarm buzzer, the light sensor and (if USE_GPS) the GPS serial
// feed, so none of those subsystems stalls during a long animation/connect sequence.
void Fdelay(unsigned long d) {
  unsigned long dStart = millis();

  doAnimationMakuna();
  while ((millis() - dStart) < d) {
    if (WiFi.getMode() == 2) dnsServer.processNextRequest();
    writeDisplay2();
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

// Scans for WiFi networks and, among all APs broadcasting the configured SSID (prm.wifiSsid),
// remembers the one with the best RSSI (bestChn/bestBssid/bestRSSI) - useful when multiple APs/mesh
// nodes share the same SSID, so the clock can explicitly target the strongest one via WiFi.begin(...,
// bestChn, bestBssid, true) in startNewWifiMode() below.
// NOTE: dead code in the current build - its only two call sites (in setup() and resetWiFi() further
// down) are both commented out (`//findBestWifi();`), so this function, its result variables
// (bestChn/bestBssid/bestRSSI/lastWifiScan) and its consumer startNewWifiMode()/checkWifiMode() are
// effectively unused; the live WiFi-connect path is startWifiMode() (plain WiFi.begin(ssid,psw), no
// BSSID targeting).
boolean findBestWifi() {
  int n;
  int newRssi;
  boolean found = false;
  
  if (WiFi.status() == WL_CONNECTED) {  //save wifi settings to prm
    WiFi.disconnect();
  }
  WiFi.scanNetworks(false);
  n = WiFi.scanComplete();
  DPRINT(F("Scanning WiFi, found networks:")); DPRINTLN(n); 
  if (n>=0){
    bestRSSI = -200;
    memset(bestBssid,0,sizeof(bestBssid));
    for (int i = 0; i < n; ++i){
      if (WiFi.SSID(i) == String(prm.wifiSsid)) {
        newRssi = WiFi.RSSI(i);
        DPRINT(F("scan: RSSI")); DPRINT(newRssi);
        if (bestRSSI<newRssi) {  //better rsssi found...
          bestChn = WiFi.channel(i);
          bestRSSI = newRssi;
          memcpy(bestBssid,WiFi.BSSID(i),sizeof(bestBssid));
          DPRINT(F("  FOUND: Channel:")); DPRINT(bestChn);
          DPRINT(F("  BSSID:")); DPRINTLN(WiFi.BSSIDstr(i));
          found = true;
          lastWifiScan = millis();
        }
        else {
          DPRINTLN(" ");
        }
      }
    }
    WiFi.scanDelete();
    }  //end else
    else {
     // WiFi.scanNetworks(false);
    }
    return(found);
}

// Fallback when a direct WiFi.begin(prm.wifiSsid,...) connect fails (or no SSID is stored at all):
// opens the ESPAsyncWiFiManager captive-configuration portal (its own temporary AP + web form) so the
// user can pick/enter WiFi credentials from a phone/laptop. Retries autoConnect() up to 5 times before
// forcing the portal open unconditionally. On success, persists the newly-chosen SSID/password into
// prm and saves EEPROM. Compiled to an empty no-op when USE_WIFIMANAGER is not defined for this
// CLOCK_NN (the #ifdef below spans the whole function body).
void wifiManager() {
  #ifdef USE_WIFIMANAGER
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

  if (WiFi.status() == WL_CONNECTED) {  //save wifi settings to prm
    String s = WiFi.SSID();
    String p = WiFi.psk();
    memcpy(oldBssid,WiFi.BSSID(),sizeof(oldBssid));
    s.toCharArray(prm.wifiSsid,sizeof(prm.wifiSsid));
    p.toCharArray(prm.wifiPsw,sizeof(prm.wifiPsw));
    saveEEPROM();
    ip = WiFi.localIP();
    WiFi.setAutoReconnect(true); WiFi.persistent(true);
  }
  #endif
}

// Alternate STA-mode connect path that targets the specific BSSID/channel found by findBestWifi()
// (WiFi.begin(ssid,psw,channel,bssid,true) instead of a plain SSID connect). See the findBestWifi()
// NOTE above - this function is currently unreachable from setup()/loop() (only called from the
// disabled checkWifiMode()), so it and its bestRSSI/lastWifiScan freshness gating are dead in practice.
void startNewWifiMode() { // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
 if (((millis()-lastWifiScan)<90000) && (bestRSSI > -95) && (WiFi.status() != WL_CONNECTED)) {
  DPRINTLN(F("_____________ Connecting WiFi ___________________"));
  if (strlen(prm.wifiSsid)==0) return;
  WiFi.disconnect();
  delay(200);
  WiFi.mode(WIFI_OFF);
  delay(200);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  #if defined(ESP32)
    esp_wifi_set_ps(WIFI_PS_NONE);
    WiFi.setHostname(webName);
    WiFi.setSleep(false);
  #else  
    WiFi.hostname(webName);
  #endif
  
  char tmp[20];
  sprintf(tmp,"%02X:%02X:%02X:%02X:%02X:%02X",bestBssid[0],bestBssid[1],bestBssid[2],bestBssid[3],bestBssid[4],bestBssid[5]);
  DPRINT(F("Connecting to best WiFi AP:")); DPRINT(prm.wifiSsid);  DPRINT(F("  PSW:")); DPRINT(prm.wifiPsw);
  DPRINT(F(" BSSID:")); DPRINT(tmp); DPRINT(" Chn:"); DPRINTLN(bestChn); 
  WiFi.begin(prm.wifiSsid, prm.wifiPsw, bestChn, bestBssid, true);

  Fdelay(500);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    DPRINT('.');
    Fdelay(3000);
    if (counter++>3) {
      DPRINTLN("Connecting to WiFi failed.");
      return;
    }  
  }    
  ip = WiFi.localIP();
  sprintf(myIp,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
  DPRINT("\nConnected to ");  DPRINT(WiFi.SSID());   
  DPRINT(". LocalIP:"); DPRINT(myIp);     
  DPRINT(" BSSID:"); DPRINT(WiFi.BSSIDstr());    
  DPRINT(" RSSI:"); DPRINT(WiFi.RSSI());   
  DPRINT(" CHN:"); DPRINTLN(WiFi.channel());   
  DPRINTLN("\n\nNetwork Configuration:");
  DPRINTLN("----------------------");
  DPRINT("         SSID: "); DPRINTLN(WiFi.SSID());
  DPRINT("  Wifi Status: "); DPRINTLN(WiFi.status());
  DPRINT("Wifi Strength: "); DPRINT(WiFi.RSSI()); DPRINTLN(" dBm");
  DPRINT("          MAC: "); DPRINTLN(WiFi.macAddress());
  DPRINT("           IP: "); DPRINTLN(myIp);
  DPRINT("       Subnet: "); DPRINTLN(WiFi.subnetMask());
  DPRINT("      Gateway: "); DPRINTLN(WiFi.gatewayIP());
  DPRINT("        DNS 1: "); DPRINTLN(WiFi.dnsIP(0));
  DPRINT("        DNS 2: "); DPRINTLN(WiFi.dnsIP(1));
  DPRINT("        DNS 3: "); DPRINTLN(WiFi.dnsIP(2));
  DPRINTLN("_____________________________________________________________________________"); 
  memcpy(oldBssid,WiFi.BSSID(),sizeof(oldBssid));
}
  DPRINTLN("\r\n");
}


// The LIVE Infrastructure(STA)-mode connect path, called from setup() when prm.wifiMode==true. Plain
// WiFi.begin(ssid,psw) (no BSSID targeting, unlike startNewWifiMode() above), retries once a second
// for up to ~10s, then falls back to wifiManager() (config portal) if still not connected - and
// wifiManager() itself falls straight through here again if no SSID was stored at all.
void startWifiMode() {
  DPRINTLN("Starting Clock in WiFi Mode!");
  if (strlen(prm.wifiSsid)==0) {
    DPRINTLN("WiFi SSID not defined!");
    wifiManager();
    return;
  }
  WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_STA);
  delay(100);
  //Fdelay(1000);
  #if defined(ESP32)
    WiFi.setHostname(webName);
    WiFi.setSleep(false);
  #else
    WiFi.hostname(webName);
  #endif
  #if defined(ESP32)
    esp_wifi_set_ps (WIFI_PS_NONE);  //power saving disable!
  #endif
  //if (WiFi.status() == WL_CONNECTED) return;

  DPRINT("\nConnecting to WiFi SSID:("); DPRINT(prm.wifiSsid); DPRINT(")  PSW:("); DPRINT(prm.wifiPsw); DPRINTLN(")");
  int counter = 0;
  if (strlen(prm.wifiPsw)!=0)
    WiFi.begin(prm.wifiSsid, prm.wifiPsw);
  else  
    WiFi.begin(prm.wifiSsid);
  while (WiFi.status() != WL_CONNECTED) {
    DPRINT('.');
    //playTubes();
    //Fdelay(3000);
    delay(1000);
    if (counter++>10) {
      wifiManager();
      return;
    }
  }
  DPRINTLN(" ");
  ip = WiFi.localIP();
  WiFi.setAutoReconnect(true);
}


// Primary time source (see calcTime()/docs/DEVELOPER_GUIDE.md 4.4): refreshes the system clock from
// NTP over WiFi, at most once every TIMESERVER_REFRESH (2h) - or every 5 minutes if the previous
// attempt failed (lastTimeFailure backoff), retrying the NTP request itself up to 5 times per call.
// On success, rebuilds myTZ from the current prm.utc_offset/prm.enableDST (so a live timezone/DST
// change from the web UI takes effect on the very next scheduled refresh) and calls setTime() with
// the timezone-adjusted local time. Returns true only if this call actually refreshed the clock -
// callers (calcTime()) use that to decide whether the RTC also needs updating this cycle.
boolean updateTimefromTimeserver() {  //true, if successful
  static unsigned long lastTimeFailure = 0;
  boolean res = false;
  int count = 1;

  if (((millis()-lastTimeUpdate)<TIMESERVER_REFRESH) && (lastTimeUpdate!=0))
    return(res);

  if ((lastTimeFailure>0) && ((millis()-lastTimeFailure)<300000)) return(res);   //wait for 5min to retry, if no success

  if (WiFi.status() == WL_CONNECTED) {
    while (true) {
      DPRINT("Connecting to timeserver: "); DPRINTLN(count);
      res = timeClient.forceUpdate();
      if (res) {
        mySTD.offset = prm.utc_offset * 60;
        myDST.offset = mySTD.offset;
        if (prm.enableDST) {
          myDST.offset += 60;
        }
        myTZ = Timezone(myDST, mySTD);  
        setTime(myTZ.toLocal(timeClient.getEpochTime()));
        lastTimeUpdate = millis();
        DPRINT("Timeserver date:"); DPRINT(year()); DPRINT("/"); DPRINT(month()); DPRINT("/"); DPRINT(day());      
        DPRINT(" time:");   DPRINT(hour()); DPRINT(":"); DPRINT(minute()); DPRINT(":"); DPRINTLN(second());  
        DPRINTLN("Clock refreshed from timeserver.");
        lastTimeFailure = 0;
      }
      count ++; 
      if (res) break;  //success!!!
      Fdelay(1000);
      if (count > 5) {  //failure, but stop trying
        lastTimeFailure = millis();
        break;
      }
    } //end while
  }
  return (res);
}

#ifndef MDNSNAME
    #define MDNSNAME "uniclock"
#endif

// Optional mDNS responder (only compiled/started if USE_MDNS is defined - commented out by default
// near the top of this file) so the clock can be reached as e.g. "uniclock.local" instead of typing
// its IP address. Guards against re-starting an already-running responder via the static flag.
void startMDNS() {
  static boolean mdnsStarted = false;

  if (mdnsStarted) return;   //running and not changed
  #ifdef USE_MDNS
    if (!MDNS.begin(MDNSNAME)) {   //mdns_free() switch off
      DPRINTLN(F("Error setting up MDNS responder!"));
      mdnsStarted = false;
    }
    else {
      DPRINT(F("Started mDns service:")); DPRINTLN(MDNSNAME);
      MDNS.addService("http", "tcp", 80);  
      mdnsStarted = true;
    }
  #endif  
}

// The Standalone(AP)-mode path (prm.wifiMode==false, or a fallback from a fully-failed STA connect):
// the clock hosts its own WiFi hotspot at the fixed address 192.168.4.1 (prm.ApSsid/prm.ApPsw) and
// runs a DNS server that answers every query with its own IP (dnsServer.start(53,"*",...)) so most
// phones/laptops auto-launch a captive-portal browser pointed at the admin page. In this mode there
// is no internet, so calcTime() never gets a WiFi/NTP refresh - GPS (if fitted) or the RTC are the
// only time sources.
void startStandaloneMode() {
  DPRINTLN("Starting Clock in Standalone Mode!");
  DPRINT("Clock's AP SSID:"); DPRINT(prm.ApSsid);
  DPRINT("   PSW:"); DPRINTLN(prm.ApPsw);
  DPRINTLN("IP: 192.168.4.1");
  IPAddress local_ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  boolean nwState;
  if (strlen(prm.ApPsw)) {
    nwState =  WiFi.softAP(prm.ApSsid, prm.ApPsw);
  }
  else {
    nwState =  WiFi.softAP(prm.ApSsid);
  }
  Fdelay(2000);   //info: https://github.com/espressif/arduino-esp32/issues/2025
  
  DPRINT("AP status:"); DPRINTLN(nwState ? "Ready" : "Failed!");  //channel
  DPRINT("Mac address:"); DPRINTLN(WiFi.softAPmacAddress());
  ip = WiFi.softAPIP();
  DPRINT("SOFT_AP IP:"); DPRINTLN(WiFi.softAPIP());

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());

}

// OTA firmware+filesystem update over plain HTTP. Triggered by the web admin's POST /firmwareupdate
// (sets makeFirmwareUpdate, actually run from loop() so it doesn't block the request handler) or
// automatically whenever the "firmware" server URL is changed via /configChanged ("for testing only",
// per the comment there). Downloads and flashes two files in sequence from prm.firmwareServer:
// "<FW>.bin" (the sketch binary) then "<FW>.spiffs.bin" (the SPIFFS web-page image) - FW is the
// firmware ID string (see clocks.h). Stops the web server first (frees RAM/sockets for the update),
// and only calls doReset() to actually reboot into the new firmware if BOTH updates reported success;
// otherwise the (old) server is simply restarted so the clock keeps running on the current firmware.
void doFirmwareUpdate(){
    if (WiFi.status() != WL_CONNECTED) {
      DPRINTLN("Wifi disconnected. FirmwareUpdate failed.");
      return;
    }

    DPRINTLN("Webserver stopped...");
    server.reset();  //Stop server
    delay(2000);
    yield();
    String fname = String(prm.firmwareServer)+"/"+String(FW)+".bin";
    DPRINT("Update firmware: "); DPRINTLN(fname);
    t_httpUpdate_return ret,ret2;
    boolean succ = false;
    WiFiClient client;
    ESPhttpUpdate.rebootOnUpdate(false);
    #if defined(ESP32) || defined(ESP8266_CORE_2xx)
      ret = ESPhttpUpdate.update(fname);    //ESP32 and old 8266 Core
    #else
      ret = ESPhttpUpdate.update(client,fname);   //new 8266 core
    #endif  
    switch(ret) {
            case HTTP_UPDATE_FAILED:
                DPRINTF("HTTP_UPDATE_FAILED Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                DPRINTLN(" ");
                break;

            case HTTP_UPDATE_NO_UPDATES:
                DPRINTLN("HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                DPRINTLN("HTTP_UPDATE_OK");
                succ = true;
                break;
    }

    fname = String(prm.firmwareServer)+"/"+String(FW)+".spiffs.bin";
    DPRINT("Update SPIFFS: "); DPRINTLN(fname);
    #if defined(ESP32)|| defined(ESP8266_CORE_2xx)
      ret2 = ESPhttpUpdate.updateSpiffs(fname);    //ESP32 or old 8266 Core
    #else  
      ret2 = ESPhttpUpdate.updateFS(client,fname);   //new 8266 core
    #endif  
            switch(ret2) {
                case HTTP_UPDATE_FAILED:
                    DPRINTF("HTTP_UPDATE_FAILED Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                    DPRINTLN(" ");
                    break;

                case HTTP_UPDATE_NO_UPDATES:
                    DPRINTLN("HTTP_UPDATE_NO_UPDATES");
                    break;

                case HTTP_UPDATE_OK:
                    DPRINTLN("HTTP_UPDATE_OK");
                    succ = true;
                    break;
            }
 
    if (succ) {
      DPRINTLN(" ");
      delay(1000);
      doReset();
    }
    DPRINTLN(" ");
    startServer();  //restart webserver
}

// Simple "run all digits through 0-9" anti-cathode-poisoning/burn-in pattern, triggered by the web
// admin's POST /cathodeProtect button (via makeCathodeProtect, executed from loop()) for
// cathProtMin minutes at full brightness (temporarily forces MAXBRIGHTNESS and disables auto-dim,
// restored at the end). Distinct from the automatic, scheduled newCathodeProtect() below (which runs
// every prm.interval minutes via timeProgram() and has per-tube stop timing/direction) - this one is
// the manual, user-triggered, uniform version.
void doCathodeProtect() {
  unsigned long started = millis();
  byte num =0;
  int db = prm.dayBright;  //save brightness values
  int nb = prm.nightBright;
  boolean ab = autoBrightness;
  byte onOff = 0;
  
  prm.dayBright=MAXBRIGHTNESS;  
  prm.nightBright = MAXBRIGHTNESS;
  autoBrightness = false;
  cathodeProtRunning = true;
  
  DPRINT("Cathode Protect is running for "); DPRINT(cathProtMin); DPRINTLN(" minutes.");
  memset(digitDP, 0, sizeof(digitDP));
  memset(animMask,0,sizeof(animMask));
  while (true) {
   for (int i=0;i<maxDigits;i++)  {  
    if (i%2) digit[i] = num;
    else     digit[i] = 9-num;
    newDigit[i] = digit[i];
    digitDP[i] = (onOff%2 == i%2);
   } 
   writeDisplaySingle();
   Fdelay(100);
   num++; if (num>9) num = 0;
   onOff++;
   if ((millis()-started)>uint32_t(cathProtMin)*60000l) break;
  } //end while
  
  prm.dayBright = db;  //restore brightness values
  prm.nightBright = nb;
  autoBrightness = ab; 
  cathodeProtRunning = false;
}

// Sets up the ESPAsyncWebServer instance (port 80) that backs the web admin UI: static file routes
// serve the SPA's HTML/CSS/JS straight out of SPIFFS (uploaded separately via the IDE's filesystem
// uploader from the repo's data/ folder - see file header), plus a handful of action routes
// (/reset, /factoryreset, /firmwareupdate, /cathodeProtect) and the JSON/form REST-ish API
// (handleConfigChanged/handleSendConfig/handleSendCurrentInfos/handleSendClockDetails, defined below).
void startServer() {
  DPRINTLN("Starting Async Webserver...");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
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

  server.on("/jquery_360.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    DPRINTLN("Webserver: /jquery_360.js");
    if (SPIFFS.exists("/jquery_360.js")) {
      AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/jquery_360.js", "text/js");   
      response->addHeader("Cache-Control", CACHE_MAX_AGE);
      request->send(response);
    }
    else {
      request->send( 204, "text/html", "File Not Found" );
      DPRINTLN("/jquery_360.js not found");
    }
  });

  server.on("/page.js", HTTP_GET, [](AsyncWebServerRequest * request) {

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
//____________________________________________________________________________
  server.on("/cathodeProtect", HTTP_POST, [](AsyncWebServerRequest * request) {
    makeCathodeProtect = true;
  });


  server.on("/saveSetting", HTTP_POST, handleConfigChanged);
  server.on("/getConfiguration", HTTP_GET, handleSendConfig);
  server.on("/getCurrentInfos", HTTP_GET, handleSendCurrentInfos);
  server.on("/getClockDetails", HTTP_GET, handleSendClockDetails);
  server.onNotFound(handleNotFound);

  server.begin();

}  //end of procedure

// Catch-all 404 handler (server.onNotFound) - also doubles as a debug request-dumper (prints every
// request param, GET/POST, to the serial console when DEBUG is on) which is handy for reverse-
// engineering what the SPA's JS is actually sending during development.
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

// POST /saveSetting - the SINGLE entry point for every settings change made from the web admin UI.
// Request carries two form fields, `key` (a string identifying which setting) and `value` (its new
// value as a string) - handled as one long if/else-if chain matching `key`, converting `value` to the
// right type and writing it into the matching prm.* field (or, for a few keys, a plain global like
// c_MinBrightness/cathProtMin/autoBrightness). Any recognized key ends with saveEEPROM() so the
// change survives a reboot; an unrecognized key returns HTTP 404 without touching EEPROM. Most
// branches are simple 1:1 field assignments and aren't commented individually below - only branches
// with side effects beyond "store this value" are called out.
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
      // Changing the UTC offset immediately shifts the LIVE running clock (not just the stored
      // preference) by the delta in hours, and pushes that change into the RTC right away via
      // updateRTC() - so the displayed time updates instantly rather than waiting for the next NTP
      // sync (which would otherwise take up to TIMESERVER_REFRESH to reflect the new zone).
      prm.utc_offset = value.toInt();
      if (old_utc_offset != prm.utc_offset) {   //Change time zone
        setTime(now()+(prm.utc_offset-old_utc_offset)*3600);
        updateRTC();
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
      // Same immediate-effect pattern as utc_offset above: toggling DST support live-shifts the
      // running clock by +-1h right away and re-syncs the RTC, instead of only taking effect on the
      // next scheduled NTP refresh (myTZ itself is rebuilt from prm.enableDST inside
      // updateTimefromTimeserver() on that next refresh).
      prm.enableDST = (value == "true");
      if (oldDST && !prm.enableDST) {   //Switching off DST
        setTime(now()-3600);
        updateRTC();
      }
      if (!oldDST && prm.enableDST) {   //Switching on DST
        setTime(now()+3600);
        updateRTC();
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
      // Not a prm.* field - "manualOverride" is the web UI's day/night ON/OFF toggle: it flips the
      // live displayON state directly and sets the manualOverride flag so evalShutoffTime() (called
      // every second from timeProgram()) stops auto-driving displayON from the day/night schedule
      // until the schedule naturally catches up to the state the user forced (see evalShutoffTime()).
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
      // Sanitizes the OLD prm.ApSsid in place (replacing any non-printable/non-ASCII byte with '_')
      // BEFORE overwriting it with the new value from the web form - i.e. this cleans up a
      // possibly-corrupted stored SSID from a prior session, not the incoming `value` itself (same
      // repair loop also runs unconditionally in factoryReset(), see below).
      for (int i=0;i<(int)strlen(prm.ApSsid);i++) {  //repair bad chars in AP SSID
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
      // Side effect: merely changing the firmware-server URL setting also immediately kicks off an
      // OTA update attempt (makeFirmwareUpdate, picked up by loop()->doFirmwareUpdate()) - per the
      // "for testing only" comment this looks like a leftover dev convenience rather than intended
      // production behavior (a user just correcting a typo in the URL would trigger a real update).
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
      if (prm.dateRepeatMin>10) prm.dateRepeatMin = 10;
    } 
    else if (key == "tempRepeatMin") {
      prm.tempRepeatMin = value.toInt();
      if (prm.tempRepeatMin>10) prm.tempRepeatMin = 10;
    }     
    else if (key == "enableDoubleBlink")  {
      prm.enableDoubleBlink = (value == "true");
    }
    else if (key == "enableTimeDisplay") {
      prm.enableTimeDisplay = (value == "true");
    }    
    else if (key == "enableAutoDim") {
      prm.enableAutoDim = (value == "true");
      autoBrightness = prm.enableAutoDim;   //mirrors the flag into the live `autoBrightness` global read by getLightSensor()/the display brightness logic, so the change takes effect immediately rather than only after the next getLightSensor() poll
    }
    else if (key == "enableRadar")  {
      prm.enableRadar = (value == "true");
      radarLastOn = millis();  //resets the radar "last motion seen" timestamp so toggling the feature on doesn't immediately look timed-out and shut the tubes off
      radarON = true;
    }
    else if (key == "radarTimeout"){
      prm.radarTimeout = value.toInt();
      if (prm.radarTimeout>60) prm.radarTimeout = 60;
      if (prm.radarTimeout<1) prm.radarTimeout = 1;
      radarLastOn = millis();   //same reasoning as enableRadar above - avoid an immediate false shutoff after changing the timeout value
      radarON = true;
    }

    else if (key == "tempCF") {
      prm.tempCF = (value == "true");
    }   
    else if (key == "corrT0") {
      prm.corrT0 = value.toFloat();
    } 
    else if (key == "corrT1") {
      prm.corrT1 = value.toFloat();
    } 
    else if (key == "corrH0") {
      prm.corrH0 = value.toFloat();
    } 
    else if (key == "corrH1") {
      prm.corrH1 = value.toFloat();
    }        
    else if (key == "cathProtMin") {
      cathProtMin = value.toInt();
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

// GET /getConfiguration - dumps (almost) the entire `prm` struct plus a handful of live/derived values
// (current date/time, sensor readings, MAC, driver name...) as one JSON object. This is what the web
// SPA fetches once on page load to populate every settings form field; a "255" sentinel value marks a
// sensor reading/feature as "not installed" (the JS treats 255 specially rather than 0, since 0 is a
// plausible real reading). Compare with handleSendCurrentInfos() below, which is the lightweight,
// frequently-polled subset used to keep the live clock/sensor display refreshed without re-sending
// the whole settings form each time.
void handleSendConfig(AsyncWebServerRequest *request) {
  StaticJsonDocument<2048> doc;
  char buf[20];  //conversion buffer

  DPRINTLN("Sending configuration to web client.");

  //Global data
  doc["version"] = webName;
  doc["FW"] = FW;
  doc["tubeDriver"] = tubeDriver;
  doc["maxDigits"] = maxDigits;   //number of digits (tubes)
  doc["maxBrightness"] = MAXBRIGHTNESS; //Maximum tube brightness usually 10, sometimes 12

  //Actual time and environment data
  sprintf(buf, "%4d.%02d.%02d", year(), month(), day());
  doc["currentDate"] = buf;
  sprintf(buf, "%02d:%02d", hour(), minute());
  doc["currentTime"] = buf;
  //DPRINT("useTemp:"); DPRINT(useTemp); DPRINT("useHumid:"); DPRINT(useHumid);
  if (useTemp > 0) {
    doc["temperature"] = round1(temperature[0] + prm.corrT0);  
  }
  else
    doc["temperature"] = 255;

if (useTemp > 1)
    doc["temperature2"] = round1(temperature[1] + prm.corrT1);
  else
    doc["temperature2"] = 255;

  if (useHumid>0) {
    doc["humidity"] = round1(humid[0] + prm.corrH0);
  }
  else
    doc["humidity"] = 255;
 
  if (useHumid>1)
    doc["humidity2"] = round1(humid[1] + prm.corrH1);
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
  doc["interval"] = prm.interval;           // interval in minutes, with 0 = off

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
  doc["tempRepeatMin"] = prm.tempRepeatMin;  
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
  doc["enableRadar"] = prm.enableRadar;    
  #if (RADAR_PIN >= 0) || defined(USE_MQTT)
    if (prm.radarTimeout<1) prm.radarTimeout = 1;
    doc["radarTimeout"] = prm.radarTimeout;   
  #else
    doc["radarTimeout"] = 0;
  #endif  
  doc["corrT0"] = prm.corrT0;
  doc["corrT1"] = prm.corrT1;
  doc["corrH0"] = prm.corrH0;
  doc["corrH1"] = prm.corrH1;
  doc["cathProtMin"] = 3;   //default value for cathProtMin slider
  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}

// GET /getClockDetails - returns a pre-formatted HTML fragment (not JSON, unlike the other two
// handlers) with static/diagnostic info: firmware ID, driver name, MAC address, sensor counts, and
// the full used-GPIO listing built by listPins()/regPin() (usedPinsStr) plus any driver-supplied
// extra setup info (driverSetupStr). Shown on an "about/diagnostics" panel of the web admin UI.
void handleSendClockDetails(AsyncWebServerRequest *request) {
  StaticJsonDocument<200> doc;
  //StaticJsonDocument<512> doc;
  String dat;

  DPRINTLN("SendClockDetails");
  dat = String("<br><br><strong>FirmwareID:") + FW + "<br>";
  dat += String("  Tube driver:") + tubeDriver + "<br>";
  dat += String("  Mac:") + String(WiFi.macAddress()) + "</strong><br>";
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

// GET /getCurrentInfos - the lightweight, frequently AJAX-polled JSON endpoint (date/time,
// temperature/humidity/pressure/lux, WiFi RSSI) used to keep the web admin page's live readouts
// current without re-fetching the whole settings form (see handleSendConfig() above). Keeps both new
// ("temperature1"/"humidity1") and legacy ("temperature"/"humidity") key names for backwards
// compatibility with older versions of the web front-end.
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
  //DPRINT("useTemp:"); DPRINT(useTemp); DPRINT("useHumid:"); DPRINT(useHumid);
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
  doc["rssi"] = WiFi.RSSI();  
  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}

// Startup sequence (see docs/DEVELOPER_GUIDE.md section 4.1 for the full ordered summary):
// brownout detector (optional) -> EEPROM init -> setup_pins() (driver-specific GPIO setup) ->
// optional peripherals (alarm/colon/radar/tube-power/light-sensor/PWM LEDs) -> temperature/I2C/RTC/GPS
// setup -> loadEEPROM() (factoryReset() if the magic number doesn't match) -> NeoPixel setup -> tube
// test animation -> SPIFFS mount (web page files) -> WiFi connect (STA or AP mode) -> web server start
// -> MQTT setup -> show IP address on the tubes -> initial time sync. Order matters here: pins must be
// configured before any peripheral touches them, and EEPROM must be loaded before WiFi connects
// (credentials live in prm).
void setup() {
  #if defined(DISABLE_BROWNOUT) && defined(ESP32)
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  #endif
  delay(1000);
  WiFi.mode(WIFI_OFF);
  EEPROM.begin(EEPROM_SIZE);
  memset(pinTxt,0,sizeof(pinTxt));
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
  
  #if defined(PIN_EXTINP1)
    if (PIN_EXTINP1>=0)
      pinMode(PIN_EXTINP1, INPUT); regPin(PIN_EXTINP1,"NOT_USED_INPUT");
  #endif
  #if defined(PIN_EXTINP2)
    if (PIN_EXTINP2>=0)
      pinMode(PIN_EXTINP2, INPUT); regPin(PIN_EXTINP2,"NOT_USED_INPUT");
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

  #ifdef FACTORYRESET_PIN
  if (FACTORYRESET_PIN>=0) {
      DPRINTLN("FactoryReset button is pushed!");
      pinMode(FACTORYRESET_PIN,INPUT_PULLUP);  regPin(FACTORYRESET_PIN,"FACTORYRESET_PIN");
      if (digitalRead(FACTORYRESET_PIN)==LOW) factoryReset();
  }
  #endif
  loadEEPROM();
  if (prm.magic != MAGIC_VALUE) factoryReset();   //EEPROM struct version mismatch (see MAGIC_VALUE comment) - wipe to sane defaults

  setupNeopixel();
  listPins();
  writeAlarmPin(ALARM_ON); writeAlarmPin(!ALARM_ON);   //quick blip of the alarm pin at boot (on then immediately off) - a self-test/audible "I'm alive" click, not a real alarm
  getDHTemp();  //get the first DHT temp+humid measure

  // Temporarily force animMode to 2 (all changed tubes spin together) for the power-on tube test
  // below, regardless of the user's saved animation preference, then restore it afterwards - ensures
  // testTubes() always animates visibly even if the stored prm.animMode is 0 (no animation).
  byte saveMode = prm.animMode;
  prm.animMode = 2;
  doAnimationPWM();
  testTubes(300);

  clearDigits();
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
    if(!SPIFFS.exists("/index.html")) {DPRINTLN("/index.html not found!");}
    if(!SPIFFS.exists("/site.css")) {DPRINTLN("/site.css not found!");}
    if(!SPIFFS.exists("/page.js")) {DPRINTLN("/page.js not found!");}
    if(!SPIFFS.exists("/jquery_360.js")) {DPRINTLN("/jquery_360.js not found!");}
  }
  memset(digit,10,sizeof(digit));   //clear display
  memset(newDigit,10,sizeof(newDigit));
  memset(oldDigit,10,sizeof(oldDigit));
  
  wifiConnectRunning = true;
  writeDisplay2();
  if (prm.wifiMode) {
    //findBestWifi();
    startWifiMode();   //note: startWifiMode() already falls back to wifiManager() internally on failure/empty SSID - the retry loop below is an extra layer only exercised when USE_WIFIMANAGER is enabled

   #ifdef USE_WIFIMANAGER
   int counter = 0;
    while (WiFi.status() != WL_CONNECTED) {   //try to use wifiManeger, if enabled
      DPRINT('.');
      Fdelay(3000);
      if (counter++>2) {
        wifiManager();
        break;
      }
    }
    DPRINTLN(" ");
   #endif

    if (WiFi.status() != WL_CONNECTED) { //failed to connect to wifi - give up on infrastructure mode entirely and fall back to hosting our own AP so the clock is at least reachable
      startStandaloneMode();
    }
    else {
      ip = WiFi.localIP();
      WiFi.setAutoReconnect(true);
    }
  }
  else {
    startStandaloneMode();
  }
  wifiConnectRunning = false;

  startMDNS();
  startServer();
  #ifdef USE_MQTT
    setupMqtt();
  #endif
  
  showMyIp();
  Fdelay(500);
  clearDigits();
  Fdelay(200);
  prm.animMode = saveMode;
  timeClient.begin();
  calcTime();
  timeProgram();

  //newCathodeProtect(5000,0);
}

// Time-source priority chain (docs/DEVELOPER_GUIDE.md section 4.4): NTP-over-WiFi is always tried
// first when connected (updateTimefromTimeserver() self-throttles to once per TIMESERVER_REFRESH, so
// this is cheap to call every cycle); GPS is tried next if fitted; the DS3231 RTC (if fitted) is the
// LAST-RESORT fallback: if either of the above actually refreshed the system clock this cycle, the RTC
// is instead WRITTEN (updateRTC(), to keep it in sync as a fallback for the next power-loss/no-network
// boot) - otherwise the RTC is READ (getRTC()) to keep the clock ticking with no network/GPS at all.
// A build with none of WiFi/GPS/RTC available simply free-runs on TimeLib's own millis()-derived clock
// (set once at some point in the past) with no further correction.
void calcTime() {
  boolean refreshed = false;
  boolean refreshed2 = false;

  if (WiFi.status() == WL_CONNECTED) {        //check wifi connection
    refreshed = updateTimefromTimeserver();  //update time from wifi
  }
  if (GPSexist) { //update time from GPS, if exist
    refreshed2 = getGPS();
  }

  if (RTCexist) {  //update time from RTC, if exist
    if (refreshed || refreshed2)
      updateRTC();    //update RTC, if needed
    else
      getRTC();
  }
}


// Main per-tick clock/sensor/display driver, called unconditionally every loop() iteration but
// self-throttled to run at most every 200ms (calcTime()/sensor polling cadence) - the actual visible
// digit refresh only happens on real second boundaries (`if (now() != prevTime)` below), so most 200ms
// ticks only refresh sensor readings and return without touching the display buffers.
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
  
  #ifdef SIMULATE_TEMPERATURE_SENSOR   //for test only
    useTemp = 2;  temperature[0] = 20.4; temperature[1] = 22.8;  //test only
    useHumid = 2; humid[0] = 41.2; humid[1] = 42.7;
  #endif
  
  if (now() != prevTime) { // Update time every second
    prevTime = now();
    evalShutoffTime();     // Check whether display should be turned off (Auto shutoff mode)
    if (!displayON || !prm.enableBlink) colonBlinkState = false;
    else colonBlinkState = (bool)(second() % 2);

    // Decide which "screen" to show this second, in priority order: temperature > humidity > date >
    // plain clock (see the showClock/showDate/showTemp*/showHumid* globals declared near the top of
    // this file). Each window is an independent [xStart,xEnd) range of seconds-within-minute, so e.g.
    // temperature and date windows can be configured to overlap or not from the web UI; whichever
    // display*() function actually runs is decided later in displayTime4/6/8() by checking these
    // flags in the same priority order.
    showClock = false;
    showDate = (prm.enableTimeDisplay && (prm.dateRepeatMin>0) && ((second() >= prm.dateStart) && (second() < prm.dateEnd)));
    if (showDate && (prm.dateRepeatMin>1)) {
      if ((minute() % prm.dateRepeatMin) != 0) {
        showDate = false;
      }
    }
//useTemp = 1; temperature[0] = -12.34; useHumid = 1; humid[0] = 26.7; //for test only
    showTemp0 = (useTemp > 0) && (second() >= prm.tempStart) && (second() < prm.tempEnd);
    showTemp1 = (useTemp > 1) && (second() >= prm.tempStart + (prm.tempEnd - prm.tempStart) / 2) && (second() < prm.tempEnd);
    showHumid0 = (useHumid > 0) && (second() >= prm.humidStart) && (second() < prm.humidEnd);
    showHumid1 = (useHumid >1) && (second() >= prm.humidStart + (prm.humidEnd-prm.humidStart)/2) && (second() < prm.humidEnd);
    if ((prm.tempRepeatMin==0) || ((minute() % prm.tempRepeatMin) != 0)) {
        showTemp0 = false;
        showTemp1 = false;
        showHumid0 = false;
        showHumid1 = false;
    }
    if (maxDigits >= 8)      displayTime8();
    else if (maxDigits == 6) displayTime6();
    else                     displayTime4();
    //   if (COLON_PIN>=0) digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
    #if LED_SWITCH_PIN >= 0
      if (displayON) //Switch on backlight LED only daytime
        digitalWrite(LED_SWITCH_PIN, HIGH);
      else
        digitalWrite(LED_SWITCH_PIN, LOW);
    #endif
    changeDigit();
    printDigits(0);
    
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
        // random(3)-1 picks the animation direction for this run: -1, 0 (random-order) or +1 - see
        // newCathodeProtect()'s dir parameter and its POSSIBLE BUG note further down.
        if (displayON && (millis() > 50000)) newCathodeProtect(maxDigits*1500,random(3)-1);   //don't play in the first 50 sec
      }
    }
  }
}

// Raw single-block read of the whole `prm` struct from EEPROM address 0 - does NOT itself check
// prm.magic; the caller (setup()) is responsible for calling factoryReset() if the loaded magic value
// doesn't match MAGIC_VALUE.
void loadEEPROM() {
  DPRINT("Loading setting from EEPROM.  Size:");  DPRINT(EEPROM_SIZE);

  EEPROM.get(EEPROM_addr, prm);
  DPRINT("  version:"); DPRINTLN(prm.magic);
}

// Raw single-block write of the whole `prm` struct back to EEPROM (address 0) - called after every
// successful key/value change in handleConfigChanged(), so every settings tweak from the web UI does
// a full-struct EEPROM commit (flash wear consideration: frequent single-field changes each cost a
// full-struct flash write, not just the changed bytes).
void saveEEPROM() {
  EEPROM.put(EEPROM_addr, prm);
  EEPROM.commit();
  DPRINTLN("Settings saved to EEPROM!");
}

// Resets every prm.* field to its hard-coded default (falling back to clocks.h-provided compile-time
// defaults like DEFAULT_SSID/AP_NAME/DATE_START/etc where available), then saves and re-syncs the
// clock. Triggered by: EEPROM magic-number mismatch at boot (see setup()), the web admin's
// POST /factoryreset, or (if wired) a held FACTORYRESET_PIN button at boot.
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
  #ifdef DEFAULT_SSID
    strncpy(prm.wifiSsid,DEFAULT_SSID,sizeof(prm.wifiSsid));
  #else
    strcpy(prm.wifiSsid,"");
  #endif
  #ifdef DEFAULT_PSW
    strncpy(prm.wifiPsw,DEFAULT_PSW,sizeof(prm.wifiPsw));
  #else
    strcpy(prm.wifiPsw,"");
  #endif  
  for (int i=0;i<(int)strlen(prm.ApSsid);i++) {  //repair bad chars in AP SSID
    if ((prm.ApSsid[i]<32) || (prm.ApSsid[i]>126)) prm.ApSsid[i]='_';
  }
  strncpy(prm.ApSsid,AP_NAME,sizeof(prm.ApSsid));
  strncpy(prm.ApPsw,AP_PASSWORD,sizeof(prm.ApPsw));
  strncpy(prm.NtpServer,"pool.ntp.org",sizeof(prm.NtpServer));
  strcpy(prm.mqttBrokerAddr,"10.10.0.202"); 
  strcpy(prm.mqttBrokerUser,"mqtt");
  strcpy(prm.mqttBrokerPsw,"mqtt");
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
  prm.dateRepeatMin = DATE_REPEAT_MIN;         //show date only every xxx minute. If zero, the date is never displayed!
  prm.tempRepeatMin = 1;
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


// Scheduled/automatic anti-cathode-poisoning animation: every tube counts rapidly through 0-9, then
// tubes stop one-by-one (in the order controlled by `dir`) once they reach their real target digit
// value (`tmp[]`, the display content saved at entry). dir<0 stops digits left-to-right (index 0
// first), dir>0 stops them right-to-left (highest index first), dir==0 is meant to stop them in random
// order - see the POSSIBLE BUG note below. Called automatically every prm.interval minutes from
// timeProgram(), and also reachable via the commented-out `newCathodeProtect(5000,0);` line at the end
// of setup(). Distinct from the simpler, manually-triggered doCathodeProtect() above.
void newCathodeProtect(unsigned long t,int dir) {    //t = time in msec, dir = direction -1,0,1    (0=random)
  byte tmp[10];
  boolean tmpDP[10];
  unsigned long started = millis();
  boolean finish, stopThis;
  byte fin[10];
  byte nextStoppedDigit;
  int sum = 0;
  
  cathodeProtRunning = true;
  DPRINT("Cathode Protect running! dir:"); DPRINTLN(dir); 
  memcpy(tmp,digit,sizeof(tmp));
  memcpy(tmpDP,digitDP,sizeof(tmpDP));
  memset(fin,0,sizeof(fin));
  switch (dir) {
    case -1:
      nextStoppedDigit = 0;
      break;
    case 0:
      nextStoppedDigit = random(maxDigits);
      break;
    case 1:
      nextStoppedDigit = maxDigits-1;
      break;
  }
  // POSSIBLE BUG: this line unconditionally overwrites whatever the switch above just computed,
  // including the `case 0` (dir==0, "random") result - so the FIRST digit to stop is never actually
  // random, it's always index 0 (dir<=0) or maxDigits-1 (dir>0), identical to the dir<0/dir>0 cases.
  // The random-order behavior only actually shows up LATER, in the `else if (sum<=maxDigits-2)` branch
  // below that picks each SUBSEQUENT stop digit randomly - so dir==0 does look random from the second
  // stopped digit onward, just not on the very first one, which is likely not what was intended.
  nextStoppedDigit = dir>0 ? 0 : maxDigits-1;
  while(true) {
    for (int i=0;i<maxDigits;i++) {
      finish = ((millis()-started) >= t);
      stopThis = (i == nextStoppedDigit); 
        
      if (finish && stopThis) {  //this digit stops
        fin[i] = 1;
        digitDP[i] = tmpDP[i];   //restore original DP value
        digit[i] = tmp[i];
        t +=1000;
        if (dir>0) {  //find next stop digit
          nextStoppedDigit++;
          if (nextStoppedDigit>=maxDigits) nextStoppedDigit =0;
        }
        else if (dir<0) {
          nextStoppedDigit--;
          if (nextStoppedDigit==255) nextStoppedDigit = maxDigits-1;
        }
        else if (sum<=maxDigits-2) {
          while(true) {
          nextStoppedDigit = random(maxDigits);
          if (fin[nextStoppedDigit] ==0) break;  //found a running digit
          }
        }
      }
      else if (fin[i]==0) {  //get next number
        if (dir>=0) 
          digit[i] = (digit[i]+1) % 10;
        else {
          digit[i]--; if (digit[i] == 255) digit[i] = 9;
        } 
        digitDP[i] = ((digit[i]%2) == 0);
      }
  } //end for
   //DPRINT("lastStoppedDigit:"); DPRINT(lastStoppedDigit);
   //DPRINT("  nextStoppedDigit:"); DPRINTLN(nextStoppedDigit);
  
    writeDisplaySingle();
    Fdelay(100);
    sum = 0;
    for (int i=0;i<maxDigits;i++)  sum += fin[i];
    //DPRINT("Sum:"); DPRINTLN(sum);
    if (sum >= maxDigits)  //all digits ready
      break;  
    if ((millis()-started) >= (t+5000)) {
      DPRINTLN("Safety exit");
      break;
    }
  } //end while
   
  memcpy(oldDigit, digit, sizeof(oldDigit));
  lastCathodeProt = minute();
  cathodeProtRunning = false;
}

// NOTE: earlier/simpler cathode-protect implementation, superseded by newCathodeProtect() above
// (kept for reference/comparison - "original version"). Not called from anywhere else in this file;
// appears to be dead code left in place.
void cathodeProtect() {   //original version
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
  for (int d=0;d<maxDigits;d++) {
    digitDP[d] = ((digit[d]%2) == 0);
  }
    writeDisplaySingle();
    Fdelay(100);
  }
  memcpy(oldDigit, digit, sizeof(oldDigit));
  lastCathodeProt = minute();
}

inline void incMod10(byte &x) {
  x = (x + 1 == 10 ? 0 : x + 1);
};


// displayTemp()/displayHumid() render a sensor reading into newDigit[]/digitDP[] (right-aligned,
// working backwards from digPtr), applying the per-sensor calibration offset (prm.corrT0/corrT1 or
// corrH0/corrH1 - ptr selects which sensor slot, 0 or 1). Two build-time variants exist per function:
// a compact DISPLAYTEMP_ONLY_2DIGITS/DISPLAYHUMID_ONLY_2DIGITS version for clocks with too few digit
// positions to show units/sign, and the full version below (unit letter + degree/percent glyph +
// optional leading sign). `ptr==0`/`ptr==1` only ever apply corrT0/corrH0 vs corrT1/corrH1 - a 3rd or
// 4th simultaneous sensor slot (see I2Csensors.ino review notes) gets no calibration correction here.
#ifdef DISPLAYTEMP_ONLY_2DIGITS
void displayTemp(byte ptr) {   //special version, 2 digits only
  float t = temperature[ptr];
  if (ptr==0) t += prm.corrT0;
  if (ptr==1) t += prm.corrT1;
  if (prm.tempCF) {
    t = round1((temperature[ptr] * 9/5)+32);
  }
  int digPtr = 3;   //tube number: first digit of minutes (hours:5,4  minutes:3,2   seconds: 1,0 )
  #ifdef TEMP_START_POSITION
    digPtr = TEMP_START_POSITION;  
  #endif  
  for (int i = 0; i < maxDigits; i++) {   //clear display
    digitDP[i] = false;
    newDigit[i] = 10;
  }

  t = abs(t);
  newDigit[digPtr] = t / 10;
  if (newDigit[digPtr] == 0) newDigit[digPtr] = 10; //BLANK, if zero
  newDigit[--digPtr] = int(t) % 10;

  if (prm.animMode == 0)  memcpy(oldDigit, newDigit, sizeof(oldDigit)); //don't do animation
  colonBlinkState = false;
  decimalpointON = false;
}

#else
void displayTemp(byte ptr) {
  float t = temperature[ptr];
  if (ptr==0) t += prm.corrT0;
  if (ptr==1) t += prm.corrT1;
  if (prm.tempCF) {
    t = round1((temperature[ptr] * 9/5)+32);
  }
  int digPtr = maxDigits-1;
  #ifdef TEMP_START_POSITION
    digPtr = TEMP_START_POSITION;  
  #endif
  for (int i = 0; i < maxDigits; i++) {
    digitDP[i] = false;
    newDigit[i] = 10;
  }
  #if defined(PLUS_CHARCODE) || defined(MINUS_CHARCODE)   //leading sign 
    if (t>=0) newDigit[digPtr] = PLUS_CHARCODE;
    if (t<0) newDigit[digPtr] = MINUS_CHARCODE;
    digPtr--;
  #endif
  t = abs(t);
  newDigit[digPtr] = t / 10;
  if (newDigit[digPtr] == 0) newDigit[digPtr] = 10; //BLANK!!!
  newDigit[--digPtr] = int(t) % 10;
  digitDP[digPtr] = true;
  newDigit[--digPtr] = int(t * 10) % 10;
  if ((maxDigits > 5) && (GRAD_CHARCODE >= 0)) {
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
#endif

#ifdef DISPLAYHUMID_ONLY_2DIGITS
void displayHumid(byte ptr) {   //special version, 2 digits only
  float h = humid[ptr];
  if (ptr==0) h = round(h + prm.corrH0);
  if (ptr==1) h = round(h + prm.corrH1);

  int digPtr = 3;   //tube number: first digit of minutes (hours:5,4  minutes:3,2   seconds: 1,0 )
  #ifdef HUMID_START_POSITION
    digPtr = HUMID_START_POSITION;  
  #endif  
  for (int i = 0; i < maxDigits; i++) {   //clear display
    digitDP[i] = false;
    newDigit[i] = 10;
  }

  newDigit[digPtr] = h / 10;
  if (newDigit[digPtr] == 0) newDigit[digPtr] = 10; //BLANK, if zero
  newDigit[--digPtr] = int(h) % 10;

  if (prm.animMode == 0)  memcpy(oldDigit, newDigit, sizeof(oldDigit)); //don't do animation
  colonBlinkState = false;
  decimalpointON = false;
}

#else
void displayHumid(byte ptr) {
  int digPtr = maxDigits-1;
  #ifdef HUMID_START_POSITION
    digPtr = HUMID_START_POSITION;  
  #endif
  for (int i = 0; i < maxDigits; i++) {
    digitDP[i] = false;
    newDigit[i] = 10;
  }
  float h = humid[ptr];
  if (ptr==0) h += prm.corrH0;
  if (ptr==1) h += prm.corrH1;
  #if defined(PLUS_CHARCODE) || defined(MINUS_CHARCODE)   //leading sign 
    digPtr--;
  #endif
  newDigit[digPtr] = int(h) / 10;
  if (newDigit[digPtr] == 0) newDigit[digPtr] = 10; //BLANK if zero!!!
  newDigit[--digPtr] = int(h) % 10;
  digitDP[digPtr] = true;
  newDigit[--digPtr] = int(h * 10) % 10;
  if (maxDigits > 5) {
    if (digitsOnly) {
      newDigit[--digPtr] = 10;   //empty character
      newDigit[--digPtr] = PERCENT_CHARCODE;  //  "%"
    }
    else {
      newDigit[--digPtr] = 10;   //empty character
      newDigit[--digPtr] = UPPER_CIRCLE_CHARCODE;   //upper circle = 16
      newDigit[--digPtr] = LOWER_CIRCLE_CHARCODE;  // lower circle = 18
    }
  } 
  else {  //4 tubes only
	if (digitsOnly) {
		newDigit[--digPtr] = PERCENT_CHARCODE;  //  "%"
	}
	else {
		for (int i = 0; i < maxDigits; i++) {digitDP[i] = false;}   //clear any decimal point
		newDigit[digPtr] = UPPER_CIRCLE_CHARCODE;   //upper circle = 16
		newDigit[--digPtr] = LOWER_CIRCLE_CHARCODE;  // lower circle = 18		
	}	
  }
     
  if (prm.animMode == 0)  memcpy(oldDigit, newDigit, sizeof(oldDigit)); //don't do animation
  colonBlinkState = true;
  decimalpointON = true;
}
#endif

// Renders the current calendar date into newDigit[] according to prm.dateMode (0=dd/mm/yyyy,
// 1=mm/dd/yyyy, 2=yyyy/mm/dd), truncating the year portion on clocks with fewer than 8 digit
// positions (>6 digits shows a 2-digit year, <=6 shows none) - the `p[m][i]` lookup table encodes
// which date component (0=day,1=month,2=year) goes in which position for each mode.
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
  if (thermometerClock)  t = 1;
  for (int i=0;i<3;i++) {
    if (p[m][i] == 2) {
      if (!thermometerClock) {
        if (maxDigits>4) {
          newDigit[t++] = year() % 10;
          newDigit[t++] = (year() % 100) / 10;
        }
        if (maxDigits>6) {
          newDigit[t++] = (year() % 1000) / 100;
          newDigit[t++] = year() / 1000;
        }      
      }  //!thermometerClock
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
  if ((maxDigits>6) && (m<2)) {
    digitDP[4] = true;
    digitDP[6] = true;
  }
  else {
    if (thermometerClock) {
      digitDP[3] = true;
      newDigit[0] = 10;
      newDigit[5] = 10;
    }
    else {
      digitDP[2] = true;
      digitDP[4] = true;
    }
  }
  colonBlinkState = true;
  if (prm.animMode == 1)  memcpy(oldDigit, newDigit, sizeof(oldDigit)); //don't do animation
}

// displayTime4/6/8() are the top-level "what goes on the display this second" dispatchers, selected by
// timeProgram() based on maxDigits. Each one first blanks digitDP[], sets the colon-separator DP
// positions, then dispatches by priority to temperature/humidity/date, falling through to the normal
// H:M(:S) clock rendering only if none of those "special screens" is currently active AND
// prm.enableTimeDisplay is set (a thermometer-only build can disable the clock face entirely).
// showZero controls whether a leading-zero hour (e.g. "07") shows the 0 or blanks it (digit value 10).
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
}


// 6-digit variant of displayTime4() above - has an extra branch for `thermometerClock` builds (see
// THERMOMETER_CLOCK in clocks.h: first digit reserved for +/- sign, last for C/F, middle 4 for HH:MM)
// vs the normal 6-tube H:M:S layout.
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
    if (thermometerClock) {
      newDigit[4] = hour12_24 / 10;
      if ((!prm.showZero)  && (newDigit[4] == 0)) newDigit[4] = 10;
      newDigit[3] = hour12_24 % 10;
      newDigit[2] = minute() / 10;
      newDigit[1] = minute() % 10;
      newDigit[0] = 10;
      newDigit[5] = 10;
      if (prm.enableBlink && (second() % 2 == 0)) digitDP[3] = false;
    }
    else {  //normal clock
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
  }
}

// 8-digit variant of displayTime4() above - full H:M:S with dash separators (digit value 11, see the
// BUFSIZE encoding comment near the digit[] declaration) instead of decimal points between the groups.
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
}


// The driver-independent animation engine: compares newDigit[] (the target the current displayTimeN()/
// displayDate()/displayTemp()/displayHumid() call just computed) against oldDigit[] (what was on
// screen before) and, if anything changed, plays the animation selected by prm.animMode (0-7, see
// docs/DEVELOPER_GUIDE.md section 4.5):
//   0 = no animation, instant switch
//   1 = each changed tube "rolls" through the digits from its old value up to its new one, one tube at a time
//   2 = all changed tubes spin through 0-9 together, in lockstep
//   3/4 = whole digit-row slides in from the right/left (with a "----" gap between old and new content)
//   5 = "wipe" - drives animMask[] with a ramp value per changed tube; interpretation is driver-specific
//       (some multiplex-family drivers get a finer 1..19 ramp, others a coarser 1..5-then-back-down ramp)
//   6 = picks a random animation from 1-5 on every call
//   7 = picks a random animation from 1-4 on every call (excludes the wipe animation)
// Special characters (value >9, e.g. blank/dash/custom glyphs) are always applied immediately outside
// the animation (never rolled through like a numeric digit change) since there's no sensible
// "in-between" glyph to animate through. On clocks with >4 digits, the seconds tube (index 0) is
// excluded from most animations (`j=1`) so a normal ticking seconds digit doesn't animate every second.
void changeDigit() {
  int j = 0;
  byte tmp[50];
  byte space = 4;
  byte anim;

  #if defined(TUBE1CLOCK)   //no tube animation possible
    writeDisplaySingle();
    return;
  #endif
  anim = prm.animMode;
  #if defined(DISABLE_NIGHT_ANIMATION)
    if (!displayON) anim = 0;   //At NIGHT switch off animation
  #endif
  #if defined(WORDCLOCK)
    anim = 0;
  #endif

  if (anim == 6) anim = 1 + rand() % 5;
  else if (anim == 7) anim = 1 + rand() % 4;

  if (anim != 5) {
    for (int i = 0; i < maxDigits; i++)
      if ((newDigit[i] > 9) || ((oldDigit[i] > 9) && (newDigit[i] <= 9) )) {
        digit[i] = newDigit[i];    //show special characters ASAP or if special char changes to numbers
        oldDigit[i] = newDigit[i];
      }
    if ((maxDigits > 4) && (newDigit[0] != 0)) j = 1; //if 6 or 8 tube clock, don't play (animate) the seconds digit
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
        // "Wipe" animation - two different ramp shapes depending on which driver is active, since
        // different segment/multiplex hardware interprets the animMask[] progress value differently:
        // the SN74141-family/VQC10/HV5122 drivers get a fine 1..19 monotonic ramp (their own
        // writeDisplay*() decodes finer wipe steps), everything else gets a coarser 1..5-then-5..1
        // "ramp up then back down" pattern, with digit[] snapped to newDigit[] partway through.
        memset(animMask, 0, sizeof(animMask));
#if defined(MULTIPLEX74141) || defined(NO_MULTIPLEX74141) || defined(MULTIPLEX74141_ESP32) || defined(MULE_V2) || defined(PCF_74141) || defined(NO_MULTIPLEX_ESP32) || defined(VQC10) || defined(NEWHV5122)
        for (int i = 1; i < 20; i++) {
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
  writeDisplaySingle();
}

// Drives ALARMSPEAKER_PIN, but only actually calls digitalWrite() when the requested state differs
// from what's already set (oldState) - avoids redundant GPIO writes on every alarmSound() tick.
void writeAlarmPin(boolean newState) {
  #if ALARMSPEAKER_PIN>=0
    static boolean oldState = !ALARM_ON; 
    if (oldState != newState) {
      oldState = newState;
      //if (newState == ALARM_ON) DPRINTLN("Alarm ON"); else DPRINTLN("Alarm OFF");
      digitalWrite(ALARMSPEAKER_PIN, newState);
    }
  #endif
}

// Called every loop() iteration. Non-blocking alarm-tone state machine: when the wall-clock time
// matches prm.alarmHour:prm.alarmMin (checked once, in the first 3 seconds of that minute, to avoid
// re-triggering all through the minute), it starts a beep pattern defined by the `t[]` timestamp table
// below (an escalating series of short beeps, in milliseconds relative to alarmStarted) that repeats
// beyond the table's end as plain 500ms on/off beeping, for prm.alarmPeriod seconds total. Can be
// silenced early via ALARMBUTTON_PIN if wired.
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


// Called once per second from timeProgram(). Decides displayON (day/night brightness state) from the
// prm.dayHour:dayMin / prm.nightHour:nightMin schedule, handling the case where the night window wraps
// past midnight (mn_off > mn_on, e.g. night starts 22:00 and ends 07:00) vs. the (unusual) case where
// it doesn't. manualOverride (set by the web UI's "manualOverride" key in handleConfigChanged()) lets
// the user force displayON one way regardless of the schedule; that override is automatically cleared
// again the moment the schedule's own computed state (prevShutoffState) naturally transitions to match
// what the override had forced - i.e. a manual override lasts until the next scheduled day/night
// boundary, not forever.
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
  // POSSIBLE BUG / NOTE: unconditional `return` above makes the debug DPRINT block below permanently
  // unreachable dead code - harmless (it only ever fed DPRINT, a no-op without DEBUG), but the
  // early-return means these diagnostics can never be re-enabled just by turning DEBUG on; the `return`
  // itself would also need removing.
  DPRINT("mn="); DPRINT(mn);
  DPRINT("  mn_on="); DPRINT(mn_on);
  DPRINT("  mn_off="); DPRINT(mn_off);
  DPRINT("  manOverride:"); DPRINT(manualOverride);
  DPRINT("  displayON:"); DPRINTLN(displayON);
  DPRINT("  enableBlink:"); DPRINTLN(prm.enableBlink);
  DPRINT("  blinkState:"); DPRINTLN(colonBlinkState);
}


// Renders a single IP-address octet (0-255) as a 3-digit number, optionally preceded by "IP" glyphs
// on displays with an alphabetic character set and >=6 digit positions, and shows it once via
// changeDigit(). Called 4 times in a row by showMyIp() below, once per octet.
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
  //if (iptag >= 100) 
  newDigit[2 + offset] = iptag / 100;
  newDigit[1 + offset] = (iptag % 100) / 10;
  newDigit[0 + offset] = iptag % 10;
  memcpy(digit,newDigit,sizeof(digit));
  printDigits(0);
  changeDigit();
}

// Called once at the end of setup(): displays the clock's IP address on the tubes, one octet at a
// time (each shown for SPEED=1500ms via writeIpTag()), so the user knows what address to browse to
// for the web admin UI without needing a router's client list or a phone app.
void showMyIp(void) {   //at startup, show the web page internet address
  clearDigits();
  #define SPEED 1500
  for (int i=0;i<4;i++) {
    ipShowRunning = true;
    writeIpTag(ip[i]);
    Fdelay(SPEED);
  }
  ipShowRunning = false;
}


// NOTE: converts a "number of 15-minute steps since midnight" value into hours+minutes - not called
// anywhere in this file or elsewhere in the project; dead/unused helper.
void fifteenMinToHM(int &hours, int &minutes, int fifteenMin)
{
  hours = fifteenMin / 4;
  minutes = (fifteenMin % 4) * 15;
}

// Called from loop() whenever prm.wifiMode is true but WiFi.status()!=WL_CONNECTED. Self-throttled to
// try at most once every 10 minutes: first attempts a lightweight WiFi.disconnect()+WiFi.reconnect()
// (reuses previously stored credentials), and only falls back to the heavier startWifiMode() (which
// can itself open the wifiManager() config portal) if that quick reconnect didn't succeed.
void resetWiFi(void) {
  static unsigned long lastTest = millis();
  static unsigned int counter = 0;

  if ((millis() - lastTest) < 600000) return; //check in every 10 min
  lastTest = millis();
  DPRINTLN("Lost WiFi. Reconnect.");
  //#if defined(ESP8266)
    WiFi.disconnect();
    WiFi.reconnect();
  //#endif
  if (WiFi.status() == WL_CONNECTED) return;

  //findBestWifi();
  startWifiMode();
  return;
  // POSSIBLE BUG / NOTE: unconditional `return` above makes everything below permanently unreachable -
  // including a 3-hour "give up and force a full reboot" counter/message. As written, resetWiFi() will
  // retry forever (every 10 minutes) without ever calling doReset(), no matter how long WiFi stays
  // down. Likely intentional (a full reboot loop might have been judged riskier than an endless retry),
  // but flagged as dead code worth reviewing - see docs/CODE_REVIEW.md.
  counter++;
  DPRINT("Wifi lost! Minutes to restart:"); DPRINTLN(180 - counter);
  if (counter < 180) return;  //3 hours passed

  DPRINTLN("Restart Clock...");
  delay(1000);
  doReset();
}

// NOTE: proper (always-non-negative) modulo helper - not called anywhere in this file or elsewhere in
// the project; dead/unused helper.
inline int mod(int a, int b) {
  int r = a % b;
  return r < 0 ? r + b : r;
}


// Power-on self-test animation: sweeps every tube through 0-9 in unison (called once from setup()).
void testTubes(int dely) {
  Fdelay(dely);
  cathodeProtRunning = true;
  DPRINT("Testing tubes: ");
  for (int i = 0; i < 10; i++) {
    DPRINT(i); DPRINT(" ");
    for (int j = 0; j < maxDigits; j++) {
      newDigit[j] = i;
      // POSSIBLE BUG: likely meant to be `digit[j] = i;` (j is the tube-index loop variable here; i is
      // the 0..9 character being swept through). As written this indexes digit[] by the CHARACTER
      // value i (0..9) instead of the tube position j, so it writes into at most the first 10 entries
      // of digit[] regardless of which tube is being iterated - harmless in practice only because
      // changeDigit() (called right below, every outer-loop iteration) immediately overwrites the
      // whole digit[] buffer from newDigit[] anyway, so this line's result is never actually rendered.
      digit[i] = i;
      digitDP[j] = i % 2;
    }
    changeDigit();
    Fdelay(dely);
  }
  DPRINTLN(" ");
  Fdelay(1000);
  memset(digitDP, 0, sizeof(digitDP));
  cathodeProtRunning = false;
}

// NOTE: cycles a rotating "walking digit" pattern across the tubes - not called from setup()/loop() or
// anywhere else in this file; dead/unused (a leftover manual test helper, presumably invoked ad hoc
// during development).
void playTubes() {
  static byte tube = 0;
  memset(digitDP, 0, sizeof(digitDP));
  for (int j = 0; j < maxDigits; j++) {
    newDigit[j] = tube++ % maxDigits;
  }
  changeDigit();
}

// Debug helper: dumps all temperature/humidity sensor readings to the serial console, throttled to
// once every 30s. Called from printDigits() below (itself a no-op unless DEBUG is defined).
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

// Debug helper: prints one digit[]/newDigit[] buffer value (see the BUFSIZE encoding comment near the
// digit[] declaration near the top of this file) as its human-readable glyph to the serial console.
// NOTE: only decodes a handful of the possible 13+ "special character" codes by hardcoded value/macro
// (TEMP_CHARCODE, GRAD_CHARCODE, PERCENT_CHARCODE, plus literal 18/19/14) - any other special glyph
// code prints as a generic '-', so this debug view doesn't fully reflect every driver's char set.
void printChar(int i) {
  if (i < 10)      {DPRINT(i);}
    else if (i==10)  {DPRINT(" ");}
    else if (i==TEMP_CHARCODE)    {DPRINT("C");}
    else if (i==GRAD_CHARCODE)    {DPRINT("°");}
    else if (i==18)    {DPRINT(".");}
    else if (i==PERCENT_CHARCODE) {DPRINT("%"); }   
    else if (i==19) {DPRINT("I");}    
    else if (i==14) {DPRINT("P");}    
    else    DPRINT("-");
    
    if (digitDP[i]) {DPRINT(".");} else {DPRINT(" ");}
}

// Debug console status line: current digit[] content (rendered via printChar()), colon-blink state,
// lux reading, WiFi status, plus periodic sensor dump (printSensors()) - entire body compiles to
// nothing when DEBUG is not defined. `timeout` throttles how often it actually prints (0 = every call).
void printDigits(unsigned long timeout) {
  static unsigned long lastRun = millis();

  if ((millis() - lastRun) < timeout) return;
  lastRun = millis();
  
  #ifdef DEBUG
  DPRINT("   digit: [");  for (int i = maxDigits - 1; i >= 0; i--) {printChar(digit[i]);} DPRINT("]");
  DPRINT(colonBlinkState ? " * " : "   ");
  //DPRINT("\noldDigit: ");  for (int i = maxDigits - 1; i >= 0; i--) {printChar(oldDigit[i]);}
  //DPRINT("\nnewDigit: ");  for (int i = maxDigits - 1; i >= 0; i--) {printChar(newDigit[i]);}

  /*
    if ((millis()/1000%10) == 1) {  //show free memory for debugging memory leak
    DPRINT("Heap:"); DPRINT(ESP.getFreeHeap()); DPRINT(" byte");
    #if defined(ESP8266)
      DPRINT(" Fragmentation:");  DPRINT(100 - ESP.getMaxFreeBlockSize() * 100.0 / ESP.getFreeHeap()); DPRINT("%");
    #endif
    }
  */
  //DPRINT("INT:"); DPRINT(intCounter); intCounter = 0;  //show multiplex interrupt counter
  //DPRINT(" ESaving:"); DPRINT(EEPROMsaving);
  if (useLux>0) {
    DPRINT("  Lux:"); DPRINT(lx);
  }
  if (RADAR_PIN>=0) {
    //DPRINT("  RadarPin:"); DPRINT(digitalRead(RADAR_PIN)); DPRINT("  lastOn:"); DPRINT(radarLastOn);  DPRINT("  sec:"); DPRINTLN((millis()-radarLastOn)/1000);  
  }
  //DPRINT("  tON:"); DPRINT(timerON); DPRINT("  tOFF:"); DPRINT(timerOFF);   //Multiplex timing values for testing
  if (WiFi.status() != WL_CONNECTED) DPRINT("  no WIFI");
  DPRINTLN(" ");
  printSensors();
  #endif  
}


// Called every loop() iteration (self-throttled to 500ms). Drives TUBE_POWER_PIN (filament/HV supply
// on/off) based on radar-detected presence: motion resets a timeout (radarLastOn), and if no motion is
// seen for prm.radarTimeout minutes the tubes' power supply is switched off entirely (a harder cutoff
// than just dimming brightness). The radar signal can come from a LOCAL RADAR_PIN sensor (if wired -
// takes priority) or from mqttRadarON (a remote/MQTT-published radar state, see mqtt.ino's master/
// slave radar sharing). If prm.enableRadar is off, radarON is forced true (always powered) regardless
// of any radar input.
void checkTubePowerOnOff(void) {
    static unsigned long lastRun = 0;

    if ((millis()-lastRun)<500) return;
    lastRun = millis();

    boolean newP = mqttRadarON;
    if (RADAR_PIN >=0) {   //if local RADAR is defined, it has priority
      newP = digitalRead(RADAR_PIN);
    }
    if (!radarON && newP) {   //check, is switching ON?
      DPRINTLN("RADAR: Switching ON tubes.");
    }
    if (newP) {  //restart timeout
      radarLastOn = millis();
      radarON = true;
    }
    if (radarON && (millis()-radarLastOn)>60000ul*uint32_t(prm.radarTimeout)) {   //check if should switch off?
              radarON = false;
              DPRINTLN("RADAR: Switching OFF tubes.");
    }
    //DPRINT("RadarON:"); DPRINT(radarON);  DPRINT(" timeout:"); DPRINTLN((millis()-radarLastOn));
    
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
 
// Converts a raw ADC reading from an LDR (light-dependent resistor) voltage divider into an
// approximate lux value, using the LUX_CALC_SCALAR/LUX_CALC_EXPONENT calibration constants from
// clocks.h (LDR characteristic curve) and a simple exponential smoothing filter (oldLux) to avoid
// jumpy brightness changes from ADC noise. Returns 0 (a build with no LIGHT_SENSOR_PIN configured)
// when there's no LDR wired at all - see getLightSensor() below for the BH1750 (digital lux sensor)
// alternative, which takes priority over this analog LDR path when both are present.
int luxMeter(void) {
#if LIGHT_SENSOR_PIN >=0   
  static float oldLux = MAXIMUM_LUX;  
  float ADCdata = analogRead(LIGHT_SENSOR_PIN); //DPRINT("ADC:"); DPRINTLN(ADCdata);
  float ldrResistance = (MAX_ADC_READING - ADCdata) / ADCdata * REF_RESISTANCE;
  float ldrLux = LUX_CALC_SCALAR * pow(ldrResistance, LUX_CALC_EXPONENT);
  if ((ldrLux>=MAXIMUM_LUX)||(ldrLux <0)) ldrLux = MAXIMUM_LUX;   //Limit lux value to maximum
  oldLux = oldLux + (ldrLux-oldLux)/10;   //slow down Lux change
  return (int)oldLux;
#else
  return (0);
#endif
}

// Called from loop()/Fdelay() (self-throttled to 500ms) to refresh the global `lx` (ambient lux) and
// `autoBrightness` (whether auto-dim is actually active) values. Prefers a BH1750 digital sensor if
// present, falls back to an analog LDR (luxMeter()) if not, and if neither exists forces a fixed
// MAXIMUM_LUX reading with autoBrightness permanently off (i.e. "auto-dim" silently has no hardware to
// act on). autoBrightness itself is only ever actually true when prm.enableAutoDim is also set by the
// user - this function just gates it on hardware presence.
void getLightSensor(void) {
  static unsigned long lastRun = 0;
  static int oldLx = 0;
  int tmp;
  if ((millis()-lastRun)<500) return;
  lastRun = millis();

  if (BH1750exist) {
    tmp = getBH1750();
    if ((abs(tmp-oldLx)>1) || (tmp >= MAXIMUM_LUX)) {
      lx=tmp; oldLx = lx; 
    }
    if (lx>=MAXIMUM_LUX-2) lx = MAXIMUM_LUX;
    autoBrightness = prm.enableAutoDim;
  }
  else if (LDRexist) {
    tmp = luxMeter();
    if ((abs(tmp-oldLx)>1) || (tmp >= MAXIMUM_LUX))  {
      lx=tmp;  oldLx = lx; 
    }
    if (lx>=MAXIMUM_LUX-2) lx = MAXIMUM_LUX;
    autoBrightness = prm.enableAutoDim;
  }
  else {
    lx = MAXIMUM_LUX;
    autoBrightness = false;
  }
}

// NOTE: intended to let the web UI's "wifiMode" (STA vs AP) toggle switch the radio mode live, without
// a reboot - would re-run startNewWifiMode()/startStandaloneMode() whenever prm.wifiMode changes. Its
// only call site in loop() is commented out (`//checkWifiMode();`), so this is currently dead code;
// changing prm.wifiMode from the web UI today only takes effect after the next reboot/doReset().
void checkWifiMode() {
static boolean oldMode = prm.wifiMode;

  if (oldMode != prm.wifiMode) {
    if (prm.wifiMode) {
        startNewWifiMode();
      }
      else  {
        startStandaloneMode();
      }
       oldMode = prm.wifiMode;
    }
}

// Debug helper: logs to serial whenever any byte of `prm` differs from the last time this was called -
// a crude way to spot unexpected/unintended settings changes during development. Its only call site in
// loop() is commented out (`//checkPrm();`), so it is currently dead code / not part of the live build.
void checkPrm() {
  static byte oldPrm[sizeof(prm)];
  int tmp = memcmp(oldPrm,&prm,sizeof(prm));
  if (tmp!=0){
    DPRINT("Prm mismatch!"); DPRINTLN(tmp);
    memcpy(oldPrm,&prm,sizeof(prm));
  }
}


// Main loop. Deliberately delay()-free (see docs/DEVELOPER_GUIDE.md section 4.2 and the Fdelay()
// comment above) - every subsystem gets serviced once per iteration: cooperative display refresh
// (writeDisplay2(), only does anything for non-ISR/"smart chip" drivers), DNS captive-portal service
// (AP mode only), the clock/sensor/display tick (timeProgram(), itself throttled to ~200ms), RGB/PWM
// backlight animations, the alarm buzzer, radar-driven tube power, the light sensor, MQTT publish (STA
// mode + WiFi connected) or WiFi reconnect attempts (STA mode + WiFi down), the RTC manual-editor
// button scan, and finally any pending OTA-update/cathode-protect request queued by the web server
// (makeFirmwareUpdate/makeCathodeProtect flags, set from server route handlers which must return
// quickly and can't run these long operations inline).
void loop(void) {
  //checkPrm();
  writeDisplay2();
  if (WiFi.status() != WL_CONNECTED)
    dnsServer.processNextRequest();
  timeProgram();
  //writeDisplaySingle();
  writeDisplay2();
  doAnimationMakuna();
  doAnimationPWM();
  alarmSound();
  checkTubePowerOnOff();
  getLightSensor();
  //checkWifiMode();
  if (prm.wifiMode) { //WiFi Clock Mode
    if (WiFi.status() == WL_CONNECTED) {
      if (prm.mqttEnable) mqttSend();
    }
    else {
      resetWiFi();
    }
  }
  editor();
  if (makeFirmwareUpdate) {
    makeFirmwareUpdate = false;
    doFirmwareUpdate();
  }
  if (makeCathodeProtect) {
    makeCathodeProtect = false;
    doCathodeProtect();
  }  
  yield();
}

// Clean reboot: drops the WiFi radio (and, on ESP8266, its TX power) before calling ESP.restart(),
// which avoids leaving the radio associated to the AP mid-reset. Called after a successful OTA update,
// from the web admin's POST /reset and /factoryreset routes, and (dead-code path only, see the
// resetWiFi() NOTE above) from the unreachable end of resetWiFi().
void doReset(void) {
  DPRINTLN("Restart Clock...");
  #if defined(ESP8266)
    WiFi.setOutputPower(0);
  #endif

  WiFi.disconnect();
  delay(1000);
  ESP.restart();
}

// Default empty stand-in for writeDisplay2() - the cooperative (non-ISR) "give the display a chance to
// refresh" hook called from loop()/Fdelay(). Only VQC10/TUBE1CLOCK/WORDCLOCK actually define a real
// writeDisplay2() (in their own driver .ino files) because their hardware needs cooperative,
// non-interrupt-safe timing (e.g. delayMicroseconds() during row scanning) that a hardware-timer ISR
// can't safely provide; every other driver relies purely on the ISR-based writeDisplay(), so this
// no-op keeps the call sites unconditional without needing per-driver #ifdef guards everywhere.
#if defined(VQC10) || defined(TUBE1CLOCK) || defined(WORDCLOCK)
#else
  void writeDisplay2(void){}  //not interrupt driven display handler
#endif

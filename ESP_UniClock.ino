/* 
 *    Universal Clock  Nixie, VFD, LED, Numitron
 *    with optional Dallas Thermometer and DS3231 RTC
 *    v1.1  05/17/2020
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
 *    Always check the usable pins of 8266:
 *    https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
 */
//---------------------------- PROGRAM PARAMETERS -------------------------------------------------
#define DEBUG
#define USE_DALLAS_TEMP   //TEMP_SENSOR_PIN is used to connect the sensor
//#define USE_RTC           //I2C pins are used!   SCL = D1 (GPIO5), SDA = D2 (GPIO4)
#define MAXBRIGHTNESS 10  //10...15    (if too high value is used, the multiplex may be too slow...)

//Use only 1 from the following options!
//#define MULTIPLEX74141    //4..8 Nixie tubes
//#define NO_MULTIPLEX74141 //4..6 Nixie tubes
#define MAX6921           //4..8 VFD tubes   (IV18)
//#define MM5450            //6..8 LEDS
//#define MAX7219CNG        //4..8 LED 
//#define Numitron_4511N
//#define SN75512           //4..8 VFD tubes   

#define colonPin -1        //Blinking Colon pin.  If not used, SET TO -1
#define TEMP_SENSOR_PIN 4  //3 or 4??  Dallas temp sensor pin

//Display temperature and date in every minute between START..END seconds
#define TEMP_START  35
#define TEMP_END    40
#define DATE_START  45
#define DATE_END    50
#define ANIMSPEED   50  //Animation speed in millisec 

char webName[] = "UniClock 1.2";
//--------------------------------------------------------------------------------------------------

#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <Timezone.h>
#include <Wire.h>
#include <WiFiManager.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>

extern void ICACHE_RAM_ATTR writeDisplay();
extern void writeDisplaySingle();
extern void setup_pins();
extern int maxDigits;

#define AP_NAME "UNICLOCK"
#define AP_PASSWORD ""

WiFiServer server(80);
String header;

#define BUFSIZE 10
byte digit[BUFSIZE];
byte newDigit[BUFSIZE];
byte oldDigit[BUFSIZE];
boolean digitDP[BUFSIZE];   //actual value to put to display
boolean digitsOnly = true;  //only 0..9 numbers are possible to display?
byte animMask[BUFSIZE];     //0 = no animation mask is used

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
boolean wifiStarted = false;

boolean useDallasTemp = false;
float dallasTemp = 0;

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
  byte magic = 133;                //magic value, to check EEPROM at first start
} prm;


//-------------------------------------------------------------------------------------------------------
time_t prevTime = 0;
time_t protectTimer = 0;
bool displayON = true;
bool manualOverride = false;
bool initProtectionTimer = false;  // Set true at the top of the hour to synchronize protection timer with clock

void configModeCallback (WiFiManager *myWiFiManager) {
  DPRINTLN("Switch to AP config mode...");
  DPRINTLN("To configure Wifi,  ");
  DPRINTLN("connect to Wifi network ESPCLOCK and");
  DPRINTLN("open 192.168.4.1 in web browser");
  }


void setup() {
  DPRINTBEGIN(115200); DPRINTLN(" ");
  delay(100);
  memset(animMask,0,sizeof(animMask));
  if (colonPin>=0)  pinMode(colonPin, OUTPUT);

  setupTemp();
  setupRTC();

  setup_pins();
  DPRINT("Number of digits:"); DPRINTLN(maxDigits);
  delay(500);
  testTubes(200);
  checkWifiMode();
  
  if (clockWifiMode) {
    DPRINTLN("Starting Clock in WiFi Mode!");
    WiFiManager MyWifiManager;
    MyWifiManager.setAPCallback(configModeCallback);
    MyWifiManager.autoConnect(AP_NAME,AP_PASSWORD); // Default password is PASSWORD, change as needed
    ip = WiFi.localIP();

    timeClient.begin();
    while (!timeClient.update()) { delay(500); }
    wifiStarted = true;
    server.begin();
    delay(100);
    DPRINTLN("---------------------");
    showMyIp();
  }
  else {
        DPRINTLN("Starting Clock in Manual Mode!");
  }
  
  EEPROM.begin(sizeof(prm));
  delay(100);  
  loadEEPROM();
  if (prm.magic !=133) factoryReset();  
  calcTime();
}

void calcTime() {
  mySTD.offset = prm.utc_offset * 60;
  myDST.offset = mySTD.offset;
  if (prm.enableDST) { myDST.offset += 60; }
  myTZ = Timezone(myDST, mySTD);

  if (clockWifiMode) {
    if (WiFi.status() == WL_CONNECTED) {
      timeClient.update();
      setTime(myTZ.toLocal(timeClient.getEpochTime()));
      if (year()>2019) 
        updateRTC();    //update RTC, if needed
      else 
        getRTC();  
    }
  }  //endif (clockWifiMode)
  else {
    getRTC();
  }
}


void timeProgram() {
static unsigned long lastRun = millis();
  if ((millis()-lastRun)<300) return;
  calcTime();
  if (useDallasTemp) {
    requestTemp(false);
    getTemp();
  }

  lastRun = millis();  
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
    
    if (colonPin>=0) digitalWrite(colonPin,colonBlinkState);  // Blink colon pin
    printDigits(0);
    writeDisplaySingle();

  // Note that UTC offset and DST setting changes can change now() by one hour
  // (3600 seconds), so modulus operator is needed
  }
}

void loop() {  
  timeProgram();

  checkWifiMode();
  
  if (clockWifiMode) { //Wifi Clock Mode
    if (WiFi.status() == WL_CONNECTED) wifiCode();
    else resetWiFi();
  }
  else {   //Manual Clock Mode
    editor();
  }
}

void loadEEPROM() {
  EEPROM.get(EEPROM_addr,prm);
}

void saveEEPROM() {
      EEPROM.put(EEPROM_addr,prm);     //(mod(mySTD.offset/60,24))); 
      EEPROM.commit();
}

void factoryReset() {
  prm.utc_offset = 1;
  prm.enableDST = true;          // Flag to enable DST (summer time...)
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
  prm.animMode = 5;        
  prm.magic = 133;              //magic value to check to first start
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
    delay(100);
  }
  memcpy(oldDigit,digit,sizeof(oldDigit));
}

inline void incMod10(byte &x) { x = (x + 1 == 10 ? 0: x + 1); };


void displayTemp(){
  for (int i=0;i<maxDigits;i++) {
    digitDP[i] = false; 
    newDigit[i] = 10; }
  newDigit[3] = int(dallasTemp) / 10; 
  if (newDigit[4]==0) newDigit[4] = 10;  //BLANK!!!
  newDigit[2] = int(dallasTemp) % 10;
  digitDP[2] = true;
  newDigit[1] = int(dallasTemp*10) % 10; 
  newDigit[0] = 15;
  if (prm.animMode == 1)  memcpy(oldDigit,newDigit,sizeof(digit));  //don't do animation
}  

void displayTime4(){
  for (int i=0;i<maxDigits;i++) digitDP[i] = false;
  digitDP[4] = true;   digitDP[2] = true;    
  int hour12_24 = prm.set12_24 ? (byte)hour() : (byte)hourFormat12();
  if (useDallasTemp && (second()>=TEMP_START) && (second()<TEMP_END)) displayTemp();
  else if ((second()>=DATE_START)&& (second()<DATE_END)) {    
        newDigit[3] = month() / 10;
        newDigit[2] = month() % 10;   
        digitDP[2] = true; 
        newDigit[1] = day() / 10;
        newDigit[0] = day() % 10;
        }
      else {
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
  if (useDallasTemp && (second()>=TEMP_START) && (second()<TEMP_END)) displayTemp();
  else if ((second()>=DATE_START)&& (second()<DATE_END)) {    
        newDigit[5] = (year()%100) / 10;
        newDigit[4] = year() % 10;
        digitDP[4] = true;
        newDigit[3] = month() / 10;
        newDigit[2] = month() % 10;   
        digitDP[2] = true; 
        newDigit[1] = day() / 10;
        newDigit[0] = day() % 10;
        }
      else {
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

  if (useDallasTemp && (second()>=TEMP_START) && (second()<TEMP_END)) displayTemp();
  else {
    if ((second()>=DATE_START) && (second()<DATE_END)) {    
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
      if (prm.animMode == 1)  memcpy(oldDigit,newDigit,sizeof(oldDigit));  //don't do animation
    }
    else {
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
  
  anim = prm.animMode; if (anim == 6) anim = 1 + rand()%5;

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
            delay(ANIMSPEED);
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
        delay(ANIMSPEED);
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
        delay(ANIMSPEED);
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
        delay(ANIMSPEED);
      } //end for i
      break;
    case 5:
    memset(animMask,0,sizeof(animMask));
      for (int i=1;i<=5;i++) {
        for (int tube=j;tube<maxDigits;tube++) {
          if (oldDigit[tube] != newDigit[tube]) animMask[tube]=i; 
        }  //end for tube
        writeDisplaySingle();
        delay(ANIMSPEED);
      }  //end for i
      memcpy(digit,newDigit,sizeof(digit));
      for (int i=1;i<=5;i++) {
        for (int tube=j;tube<maxDigits;tube++) {
          if (oldDigit[tube] != newDigit[tube]) animMask[tube]=6-i;
        }  //end for tube
         writeDisplaySingle();        
        delay(ANIMSPEED);
      }  //end for i     
      memset(animMask,0,sizeof(animMask)); 
      break;
  }  
  } //endif memcmp  

  memcpy(digit,newDigit,sizeof(digit));
  memcpy(oldDigit,newDigit,sizeof(oldDigit));

}




void evalShutoffTime() {  // Determine whether  tubes should be turned to NIGHT mode
  
  if (!prm.enableAutoShutoff) return;
  
  int mn = 60*hour() + minute();
  int mn_on = prm.dayHour*60 + prm.dayMin;  
  int mn_off = prm.nightHour*60 + prm.nightMin;
  
  static bool prevShutoffState = true;  
  if ( ((mn_off < mn_on) &&  (mn >= mn_off) && (mn < mn_on)) ||  // Nighttime
        (mn_off > mn_on) && ((mn >= mn_off) || (mn < mn_on))) { 
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
  for (int i=0;i<maxDigits;i++) digit[i] = 10;
  if (!digitsOnly && (maxDigits>=6)) {
    digit[4] = 1;
    digit[3] = 14;
  }
  if (iptag>=100) digit[2] = iptag / 100;
  digit[1] = (iptag % 100) / 10;
  digit[0] = iptag % 10;
  writeDisplaySingle();
}

void showMyIp() {   //at startup, show the web page internet address
  writeIpTag(ip[0]);
  printDigits(0);
  delay(2000);
  writeIpTag(ip[1]);
  printDigits(0);
  delay(2000);
  writeIpTag(ip[2]);
  printDigits(0);
  delay(2000);
  writeIpTag(ip[3]);
  printDigits(0);
  delay(2000);
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
  delay(1000);
  ESP.restart();
}


inline int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}


void testTubes(int dely) {
   delay(dely);
   for (int i=0; i<10;i++) { 
     for (int j=0;j<maxDigits;j++) digit[j] = i;
     writeDisplaySingle();
     delay(dely); 
   }
   delay(1000);
}

void printDigits(unsigned long timeout) {
static unsigned long lastRun = millis();
  if ((millis()-lastRun)<timeout) return;
  lastRun = millis();
  for (int i=maxDigits-1;i>=0;i--)  
    if (digit[i]<10) DPRINT(digit[i]);  
    else DPRINT("-");
  if (colonBlinkState) DPRINTLN(" B");
  else DPRINTLN(" ");
}



void wifiCode() {
static char txt[50];
static WiFiClient client;
bool mod = false;  
int tmp = 0;


  client = server.available();
  //char clientline[BUFSIZ];
  //int index = 0;
  if (client) {
    boolean currentLineIsBlank = true;
    boolean currentLineIsGet = true;
    int tCount = 0;
    char tBuf[512];
    char *pch;
    DPRINTLN("WiFi Client request...");
    while (client.connected()) {
       //DPRINTLN(txt);
       while (client.available()) {
        char c = client.read();
        header += c;
        if (currentLineIsGet && tCount < sizeof(tBuf)) {
          tBuf[tCount] = c;
          tCount++;
          tBuf[tCount] = 0;
        }
        if (c == '\n' && currentLineIsBlank) {
          while (client.available()) client.read();
          pch = strtok(tBuf, "?");
          mod = false;
          while (pch != NULL) {
            if (strncmp(pch, "utc_offset=", 11) == 0) {
              tmp = atoi(pch + 11);
              if (tmp > 12) tmp = 12;
              if (tmp < -12) tmp = -12;
              if (tmp != prm.utc_offset) {  
                prm.utc_offset = tmp;
                mod = true;
                calcTime();
              }
            }
            else if (strncmp(pch, "anim=", 5) == 0) {
              tmp = atoi(pch + 5); 
              if (tmp < 0) tmp = 0;
              if (tmp > 6) tmp = 6;
              if (tmp != prm.animMode) {
                prm.animMode = tmp;
                mod = true;              
              }
            }
            else if (strncmp(pch, "interval=", 9) == 0) {
              tmp = atoi(pch + 9); 
              if (tmp < 0) tmp = 0;
              if (tmp > 240) tmp = 240;
              if (tmp != prm.interval) {
                prm.interval = tmp;
                mod = true;              
              }
            }
            else if (strncmp(pch, "dayBright=", 10) == 0) {
              tmp = atoi(pch + 10); 
              if (tmp < 1) tmp = 1;
              if (tmp > MAXBRIGHTNESS) tmp = MAXBRIGHTNESS;
              if (tmp != prm.dayBright) {
                prm.dayBright = tmp;
                mod = true;              
              }
            }    
            else if (strncmp(pch, "nightBright=", 12) == 0) {
              tmp = atoi(pch + 12); 
              if (tmp < 1) tmp = 0;
              if (tmp > MAXBRIGHTNESS) tmp = MAXBRIGHTNESS;
              if (tmp != prm.nightBright) {
                prm.nightBright = tmp;
                mod = true;              
              }  
            }
            else if (strncmp(pch, "dayHour=", 8) == 0) {
              tmp = atoi(pch + 8); 
              if (tmp <0) tmp = 0;
              if (tmp>23) tmp = 23;
              if (tmp != prm.dayHour) {
                prm.dayHour = tmp;
                mod = true;              
              }              
            }
            else if (strncmp(pch, "dayMin=", 7) == 0) {
              tmp = atoi(pch + 7); 
              if (tmp <0) tmp = 0;
              if (tmp>59) tmp = 59;
              if (tmp != prm.dayMin) {
                prm.dayMin = tmp;
                mod = true;              
              }              
            } 
            else if (strncmp(pch, "nightHour=", 10) == 0) {
              tmp = atoi(pch + 10); 
              if (tmp <0) tmp = 0;
              if (tmp>23) tmp = 23;
              if (tmp != prm.nightHour) {
                prm.nightHour = tmp;
                mod = true;              
              }              
            }
            else if (strncmp(pch, "nightMin=", 9) == 0) {
              tmp = atoi(pch + 9); 
              if (tmp <0) tmp = 0;
              if (tmp>59) tmp = 59;
              if (tmp != prm.nightMin) {
                prm.nightMin = tmp;
                mod = true;              
              }              
            }
           pch = strtok(NULL, "& ");
          }  //end while pch...
          
          if (header.indexOf("GET /DAY") >= 0)        {displayON = true; manualOverride = true;} 
          else if (header.indexOf("GET /NIGHT") >= 0) {displayON = false; manualOverride = true;} 
          else if (header.indexOf("GET /DNON") >= 0)  {prm.enableAutoShutoff = true; mod = true;} 
          else if (header.indexOf("GET /DNOFF") >= 0) {prm.enableAutoShutoff = false; mod = true;}           
          else if (header.indexOf("GET /BLINKON") >= 0) {prm.enableBlink = true; mod = true;} 
          else if (header.indexOf("GET /BLINKOFF") >= 0) {prm.enableBlink = false; mod = true;} 
          else if (header.indexOf("GET /DSTON") >= 0) {prm.enableDST = true; mod = true;}
          else if (header.indexOf("GET /DSTOFF") >= 0) {prm.enableDST = false; mod = true;}
          else if (header.indexOf("GET /SET12") >= 0) {prm.set12_24 = false; mod = true;}
          else if (header.indexOf("GET /SET24") >= 0) {prm.set12_24 = true; mod = true;}
          else if (header.indexOf("GET /ZERO") >= 0) {prm.showZero = true; mod = true;}
          else if (header.indexOf("GET /NOZERO") >= 0) {prm.showZero = false; mod = true;}
          else if (header.indexOf("GET /factReset") >= 0) { factoryReset();  }

          if (mod) {  //Save modified parameters
            DPRINTLN("Saved to EEPROM!");
            saveEEPROM();    
            }
           if (useDallasTemp)   
              sprintf(txt,"<H1>%02d/%02d/%4d %02d:%02d      T:%2.1fC</H1>",month(),day(),year(),hour(),minute(),dallasTemp);
          else 
              sprintf(txt,"<H1>%02d/%02d/%4d %02d:%02d </H1>",month(),day(),year(),hour(),minute());
          client.print("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><H1>"); client.print(webName);
          client.print("</H1><form method=GET>");
          client.println(txt);  
          client.print("<br>UTC offset (-12..12):<input type=text  maxlength=3 size=3 name=utc_offset value=");  client.print(prm.utc_offset);
          client.print("><br>ANIMATE (0..6): <input type=text maxlength=1 size=3 name=anim value=");    client.print(prm.animMode);
          client.print(">  SHOW (0..240min): <input type=text maxlength=3 size=3 name=interval value=");    client.print(prm.interval);
          client.print("><br>DAY &#160;&#160;&#160;&#160;Brightness (1.."); client.print(MAXBRIGHTNESS);
          client.print("): <input type=text maxlength=2 size=1 name=dayBright value=");    client.print(prm.dayBright);
          client.print("><br>NIGHT Brightness (0.."); client.print(MAXBRIGHTNESS);
          client.print("): <input type=text maxlength=2 size=1 name=nightBright value=");    client.print(prm.nightBright);
          if (prm.enableAutoShutoff) {  
            client.print("><br>DAY &#160;&#160;&#160;&#160;hour (0..23): <input type=text maxlength=2 size=2 name=dayHour value=");  client.print(prm.dayHour);
            client.print("> min (0..59): <input type=text maxlength=2 size=2 name=dayMin value=");             client.print(prm.dayMin);
            client.print("><br>NIGHT hour (0..23): <input type=text maxlength=2 size=2 name=nightHour value=");  client.print(prm.nightHour);
            client.print("> min (0..59): <input type=text maxlength=2 size=2 name=nightMin value=");           client.print(prm.nightMin);          
          }
          client.print("><br><input type=submit value=Submit></form>");
          
          client.println("<!DOCTYPE html><html>");
          client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<link rel=\"icon\" href=\"data:,\">");
          client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: left;}");
          client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 8px 20px;");
          client.println("text-decoration: none; font-size: 15px; margin: 2px; cursor: pointer;}");
          client.println(".button2 {background-color: #555555;}</style></head>");
//   ---------- body  -  Buttons ... --------------------          
          //client.println("<body><h1>Control buttons</h1>");
          if (!displayON)       client.println("<p><a href=\"/DAY\"><button class=\"button button2\">DAY!</button></a>");
          if (displayON)        client.println("<p><a href=\"/NIGHT\"><button class=\"button button2\">NIGHT!</button></a>");
          if (!prm.enableAutoShutoff) client.println("<a href=\"/DNON\"><button class=\"button button2\">AUTO DAY/NIGHT ON</button></a>");
          if (prm.enableAutoShutoff)  client.println("<a href=\"/DNOFF\"><button class=\"button button2\">AUTO DAY/NIGHT OFF</button></a>");          
          if (!prm.enableBlink) client.println("<p><a href=\"/BLINKON\"><button class=\"button button2\">BLINK ON</button></a>");
          if (prm.enableBlink)  client.println("<p><a href=\"/BLINKOFF\"><button class=\"button button2\">BLINK OFF</button></a>");          
          if (!prm.enableDST)   client.println("<a href=\"/DSTON\"><button class=\"button button2\">DST ON</button></a>");
          if (prm.enableDST)    client.println("<a href=\"/DSTOFF\"><button class=\"button button2\">DST OFF</button></a>");
          if (prm.set12_24)     client.println("<p><a href=\"/SET12\"><button class=\"button button2\">SET 12h</button></a>");
          if (!prm.set12_24)    client.println("<p><a href=\"/SET24\"><button class=\"button button2\">SET 24h</button></a>");
          if (!prm.showZero)    client.println("<a href=\"/ZERO\"><button class=\"button button2\">Show Zero</button></a></p>");
          if (prm.showZero)     client.println("<a href=\"/NOZERO\"><button class=\"button button2\">NO Zero</button></a></p>");
          client.println("<p><a href=\"/factReset\"><button class=\"button button2\">Factory Reset</button></a></p>");
          client.println("<br>(c) 2020 Peter Gautier<br>gautier.p62@gmail.com<br>");
          client.write("</body></html>\r\n\r\n");
          header = "";
          client.stop();
        }
        else if (c == '\n') {currentLineIsBlank = true;  currentLineIsGet = false;  }
        else if (c != '\r') { currentLineIsBlank = false;  }
      } //endwhile client available
    timeProgram();  
    } //endwhile client connected
  } //endif (client)
}  

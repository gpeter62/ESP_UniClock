/*
 ******************* CLOCK DEFINITIONS ***********************
 * Define here your clock's setup  
 * Append a new clock or modify this sample configurations!
 * ***********************************************************
*/

//#define CLOCK_1
//#define CLOCK_2
//#define CLOCK_3
//#define CLOCK_4
//#define CLOCK_5
//#define CLOCK_6
#define CLOCK_7   //6 tube, PCF digit selector
//#define CLOCK_8
//#define CLOCK_9
//#define CLOCK_10
//#define CLOCK_11
//#define CLOCK_12
//#define CLOCK_13

//#define CLOCK_20
//#define CLOCK_21
//#define CLOCK_22

//#define CLOCK_30
//#define CLOCK_31

//______________________ESP8266 CLOCKS by UNFI and GP ______________________________________________________
#ifdef CLOCK_1   //8266, UNFI PCB clock, 4x IN-16 tubes
  #define DEBUG 
  #define USE_NEOPIXEL_MAKUNA 
  byte tubePixels[] = {3,2,1,0};        //4 tubes, single leds, reverse direction
  //#define USE_DALLAS_TEMP
  #define TEMP_SENSOR_PIN -1    //DHT or Dallas temp sensor pin.  If not used, SET TO -1    
  #define MULTIPLEX74141
  const byte digitEnablePins[] = {14,12,13,15};   //IN16 4x tube clock
  const byte ABCDPins[4] =  {2,4,5,0};   
  const int DpPin = -1;         // decimalPoint inside Nixie tube, set -1, if not used!
  #define COLON_PIN   16        //Blinking Colon pin.  If not used, SET TO -1      
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IN-16 Nixie Clock"
#endif

#ifdef CLOCK_2   //8266, UNFI PCB clock, 4x Z-570M tubes
  #define DEBUG 
  //#define USE_NEOPIXEL_MAKUNA 
  //#define USE_DALLAS_TEMP
  #define TEMP_SENSOR_PIN -1    //DHT or Dallas temp sensor pin.  
  #define MULTIPLEX74141
  const byte digitEnablePins[] = {13,12,14,15};  
  const byte ABCDPins[4] = {16,5,4,0};
  const int DpPin = -1;         // decimalPoint inside Nixie tube, set -1, if not used!
  #define COLON_PIN   2        //Blinking Colon pin.  If not used, SET TO -1          
  #define ALARMSPEAKER_PIN -1   //Alarm buzzer pin                             
  #define ALARMBUTTON_PIN -1    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "Z-570M Nixie Clock"
#endif

#ifdef CLOCK_3   //8266, PCB less clock, IV-18 VFD tube   https://www.thingiverse.com/thing:3417955
  #define DEBUG 
  #define USE_DALLAS_TEMP
  #define TEMP_SENSOR_PIN 4    //DHT or Dallas temp sensor pin.  If not used, SET TO -1
  #define MAX6921
  byte segmentEnablePins[] =  {0,2,5,6,4,1,3,7};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {18,11,17,12,16,13,14,15}; //19};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  (You may define any number)
  //MAX6921 pins
    #define PIN_LE    12  // D6 Shift Register Latch Enable
    #define PIN_CLK   13  // D7 Shift Register Clock
    #define PIN_DATA  14  // D5 Shift Register Data
    #define PIN_BL    15  // D8 Shift Register Blank (1=display off     0=display on)
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IV-18 VFD Clock"
#endif


#ifdef CLOCK_4   //8266, custom clock, 4x IV-11 VFD tubes
  #define DEBUG 
  #define MAX6921
  byte segmentEnablePins[] =  {7,5,1,0,2,4,6,3};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {19,18,17,16};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  (You may define any number)
  //MAX6921 pins
    #define PIN_LE    12  // D6 Shift Register Latch Enable
    #define PIN_CLK   13  // D7 Shift Register Clock
    #define PIN_DATA  14  // D5 Shift Register Data
    #define PIN_BL    15  // D8 Shift Register Blank (1=display off     0=display on)
  #define ALARMSPEAKER_PIN -1   //Alarm buzzer pin                                        
  #define ALARMBUTTON_PIN -1    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IV-11 VFD Clock"
#endif

#ifdef CLOCK_5   //8266, UNFI PCB clock IVL-2-5/7 tube with RTC
  //#define DEBUG 
  #define USE_DALLAS_TEMP
  #define TEMP_SENSOR_PIN 3
  #define USE_RTC    
  #define MAX6921
  //Fill this table with the OUT positions of the MAX6921 chip!   
    byte segmentEnablePins[] =  {11,13,5,8,3,12,2,1};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
    byte digitEnablePins[] = {10,9,4,0};  //segment enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  
  //MAX6921 pins
    #define PIN_LE    13  // D6 Shift Register Latch Enable
    #define PIN_CLK   12  // D7 Shift Register Clock
    #define PIN_DATA  14  // D5 Shift Register Data
    #define PIN_BL    15  // D8 Shift Register Blank (1=display off     0=display on)
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IVL-2 VFD Clock"
#endif

#ifdef CLOCK_6   //8266, GP PCB clock, 4x IN-1 tubes and  4x74141  driver
  #define DEBUG
  //#define USE_DALLAS_TEMP
  //#define TEMP_SENSOR_PIN -1
  #define USE_NEOPIXEL_MAKUNA
  byte tubePixels[] = {0,0,1,1,2,2,3,3,3,3,2,2,1,1,0,0,0};  //4 tubes, double row, 17 leds (GP)  
  #define NO_MULTIPLEX74141
  #define TEMP_SENSOR_PIN -1
  #define COLON_PIN   2        //Blinking Colon pin.  If not used, SET TO -1                 ( old IN-1-Clock: white:2 brown: SDA)
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IN-1 Nixie Clock"
#endif


#ifdef CLOCK_7   //8266 UNFI PCB clock 6x Z574M/Z573M tubes, tube selection by PCF8574 chip
  #define DEBUG 
  //#define USE_DALLAS_TEMP
  //#define TEMP_SENSOR_PIN -1
  //#define USE_NEOPIXEL_MAKUNA
  //byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  //byte tubePixels[] = {5,4,3,2,1,0};    //6 tubes, single leds, reverse direction 
  #define LEFTDECIMAL false 
  #define PCF_74141
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "Z574M Nixie Clock"
#endif

#ifdef CLOCK_8   //8266 GP PCB v3 clock with 4x IV-16 Numitron tubes (plexi box)
  #define DEBUG 
  #define PCB_VERSION 3   //1,2 or 3
  #define USE_NEOPIXEL_MAKUNA
  byte tubePixels[] = {3,2,6,1,0};    //Numitron 4 tubes, 4 x single leds + 1. The extra led in the middle is not used, is always dark!
  #define Numitron_4511N
  #define ALARMSPEAKER_PIN 1    //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN -1    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "Numitron UniClock"
#endif

#ifdef CLOCK_9   //8266 GP PCB v1 clock with 4x IV-16 Numitron tubes (brown box)
  #define DEBUG 
  #define PCB_VERSION 1   //1,2 or 3
  #define Numitron_4511N
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "Numitron UniClock"
#endif

#ifdef CLOCK_10   //8266 LED Clock with MAX7219 chip
  #define DEBUG 
  #define MAX7219CNG
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "LED UniClock"
#endif

#ifdef CLOCK_11   //8266 LED Clock with MM5450 chip
  #define DEBUG 
  #define MM5450
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "LED UniClock"
#endif

//____________ P.S. clocks / thermometers ____________________________________________
#ifdef CLOCK_20   //8266 D1-mini, P.S. PCB 4xIN14 clock-thermometer 
  //#define DEBUG 
  #define USE_DALLAS_TEMP
  #define TEMP_SENSOR_PIN 3 //RX
  #define MULTIPLEX74141
  const byte digitEnablePins[] = {15,13,12,14};   
  const byte ABCDPins[4] =  {2,4,5,0};
  const int DpPin = -1; // decimalPoint inside Nixie tube, set -1, if not used!
  #define ENABLE_CLOCK_DISPLAY true  //don't display date/time!!!
  #define SHIFT_TUBES_LEFT_BY_1  //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 15
  #define GRAD_CHARCODE 16 
  #define PERCENT_CHARCODE 17
  #define COLON_PIN  1  //TX        //Blinking Colon pin.  If not used, SET TO -1               
  #define LED_SWITCH_PIN 16     //external led lightning ON/OFF.  If not used, SET TO -1 
  #define AP_NAME "Nixie ORA"
  #define AP_PASSWORD "q1w2e3r4"
  #define WEBNAME "Nixie IN-14 ora"  
#endif

#ifdef CLOCK_21   //8266 D1-mini, P.S. PCB 4xIN14 thermometer / hygrometer 
  //#define DEBUG 
  #define USE_DHT_TEMP
  #define DHTTYPE DHT22
  #define TEMP_SENSOR_PIN  3  //RX
  #define MULTIPLEX74141
  const byte digitEnablePins[] = {15,13,12,14};  
  const byte ABCDPins[4] =  {2,4,5,0};
  const int DpPin = -1; // decimalPoint inside Nixie tube, set -1, if not used!
  #define ENABLE_CLOCK_DISPLAY false  //don't display date/time!!!
  #define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 4
  #define GRAD_CHARCODE 16 
  #define PERCENT_CHARCODE 7
  #define LED_SWITCH_PIN   16   //external led lightning ON/OFF.  If not used, SET TO -1 
  #define DECIMALPOINT_PIN 1 //TX   //Nixie decimal point between digits. If not used, SET TO -1 
  #define AP_NAME "Nixie Homero 5.6"
  #define AP_PASSWORD "q1w2e3r4"
  #define WEBNAME "Nixie Homero 5.6"
#endif

#ifdef CLOCK_22   //8266 NOMEMCU, P.S. PCB 4xIN14 thermometer / hygrometer
  //#define DEBUG 
  #define USE_NEOPIXEL_MAKUNA 
  byte tubePixels[] = {0,1,2,3};        //4 tubes, single leds
  #define USE_DHT_TEMP
  #define DHTTYPE DHT22
  #define TEMP_SENSOR_PIN  1
  #define MULTIPLEX74141
  const byte digitEnablePins[] = {15,13,12,14};  
  const byte ABCDPins[4] =  {2,4,5,0};
  const int DpPin = -1;     // decimalPoint inside Nixie tube, set -1, if not used!
  #define ENABLE_CLOCK_DISPLAY false  //don't display date/time!!!
  #define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 4
  #define GRAD_CHARCODE 16 
  #define PERCENT_CHARCODE 7
  #define DECIMALPOINT_PIN  16  //Nixie decimal point between digits. If not used, SET TO -1 
  #define AP_NAME "Nixie Homero 6.0"
  #define AP_PASSWORD "q1w2e3r4"
  #define WEBNAME "Nixie Homero 6.0"
#endif

//______________________ESP-32 CLOCKS______________________________________________________
#ifdef CLOCK_30   //ESP32, UNFI PCB clock, 6 x IV-11 VFD tubes
  #define DEBUG 
  #define USE_NEOPIXEL_MAKUNA 
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_SENSOR_PIN 23    //DHT or Dallas temp sensor pin.  If not used, SET TO -1     
  #define MAX6921
  byte segmentEnablePins[] =  {19,17,15,12,13,16,18,14};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {9,8,7,2,1,0};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6)  (You may define any number)
  //MAX6921 pins
    #define PIN_LE    14  // Shift Register Latch Enable
    #define PIN_CLK   13  // Shift Register Clock
    #define PIN_DATA  27  // Shift Register Data
    #define PIN_BL    12  // Shift Register Blank (1=display off     0=display on)
  #define ALARMSPEAKER_PIN 33   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 32    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""  
  #define WEBNAME "ESP32UniClock 2.3"
#endif

#ifdef CLOCK_31   //ESP32, UNFI board, 6 x Z573M Nixie tubes
  #define DEBUG 
  #define USE_NEOPIXEL_MAKUNA 
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_SENSOR_PIN 23    //DHT or Dallas temp sensor pin.  If not used, SET TO -1    
  #define MULTIPLEX74141
  const byte digitEnablePins[] = {4,16,17,5,18,19};   //ESP32 6x tube Clock
  const byte ABCDPins[4] =  {12,27,14,13};   
  const int DpPin = 15; // decimalPoint inside Nixie tube, set -1, if not used!
  #define LEFTDECIMAL false   //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  #define ALARMSPEAKER_PIN 33   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 32    //Alarm switch off button pin 
  #define ALARM_ON HIGH
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""
  #define WEBNAME "ESP32UniClock 2.3"
#endif


//if not defined, here is defined as NOT USED
#ifndef COLON_PIN
  #define COLON_PIN   -1        //Blinking Colon pin.
#endif

#ifndef TEMP_SENSOR_PIN
  #define TEMP_SENSOR_PIN -1     //DHT or Dallas temp sensor pin.
#endif

#ifndef LED_SWITCH_PIN
  #define LED_SWITCH_PIN -1     //external led lightning ON/OFF.
#endif
  
#ifndef DECIMALPOINT_PIN
  #define DECIMALPOINT_PIN -1   //Nixie decimal point between digits.
#endif
  
#ifndef ALARMSPEAKER_PIN
  #define ALARMSPEAKER_PIN -1   //Alarm buzzer pin
#endif
  
#ifndef ALARMBUTTON_PIN
  #define ALARMBUTTON_PIN -1    //Alarm switch off button pin 
#endif
  
#ifndef ALARM_ON
  #define ALARM_ON HIGH  
#endif  

#ifndef MAXBRIGHTNESS 
  #define MAXBRIGHTNESS 10 // (if MM5450, use 15 instead of 10)
#endif 

//Display temperature and date in every minute between START..END seconds
#ifndef ENABLE_CLOCK_DISPLAY 
  #define ENABLE_CLOCK_DISPLAY true  //false, if no clock display is needed (for example: thermometer + hygrometer only)
#endif

#ifndef TEMP_START    
  #define TEMP_START  35 
#endif

#ifndef TEMP_END      
  #define TEMP_END    40 
#endif

#ifndef HUMID_START   
  #define HUMID_START 40 
#endif

#ifndef HUMID_END     
  #define HUMID_END   45 
#endif

#ifndef DATE_START    
  #define DATE_START  45 
#endif

#ifndef DATE_END      
  #define DATE_END    50 
#endif

#ifndef ANIMSPEED     
  #define ANIMSPEED   50   //Animation speed in millisec 
#endif

#ifndef TEMP_CHARCODE 
  #define TEMP_CHARCODE 15   
#endif

#ifndef GRAD_CHARCODE 
  #define GRAD_CHARCODE 16 
#endif

#ifndef PERCENT_CHARCODE 
  #define PERCENT_CHARCODE 17 
#endif

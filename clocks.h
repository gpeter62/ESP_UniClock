/*
 ******************* CLOCK DEFINITIONS ***********************
 * Define here your clock's setup  
 * Append a new clock or modify this sample configurations!
 * ***********************************************************
*/

//#define CLOCK_1   //8266, UNFI PCB clock, 4x IN-16 tubes
//#define CLOCK_2   //8266, UNFI PCB clock, 4x Z-570M tubes
//#define CLOCK_3   //8266, PCB less clock, IV-18 VFD tube   https://www.thingiverse.com/thing:3417955
//#define CLOCK_4   //8266, custom clock, 4x IV-11 VFD tubes
//#define CLOCK_5   //8266, UNFI PCB clock IVL-2-5/7 tube with RTC
//#define CLOCK_6   //8266, GP PCB clock, 4x IN-1 tubes and  4x74141  driver (NON-MULTIPLEX)
//#define CLOCK_7   //8266 UNFI PCB clock 6x Z574M/Z573M tubes, tube selection by PCF8574 chip
//#define CLOCK_8   //8266 GP PCB v3 clock with 4x IV-16 Numitron tubes (plexi box)
//#define CLOCK_9   //8266 GP PCB v1 clock with 4x IV-16 Numitron tubes (brown box)
//#define CLOCK_10  //8266 LED Clock with MAX7219 chip
//#define CLOCK_11  //8266 LED Clock with MM5450 chip
//#define CLOCK_12  //8266 VFD Clock with PT6355 chip
//#define CLOCK_13   //8266 GP PCB v1 clock with 4x IV-16 Numitron tubes + GPS timesync (white box)

//#define CLOCK_20  //8266 D1-mini, P.S. PCB 4xIN14 clock-thermometer 
//#define CLOCK_21  //8266 D1-mini, P.S. PCB 4xIN14 thermometer / humidity 
//#define CLOCK_22  //8266 NODEMCU, P.S. PCB 4xIN14 thermometer / humidity

//#define CLOCK_30  //ESP32 prototype, UNFI PCB clock, 6 x IV-11 VFD tubes
//#define CLOCK_31  //ESP32 prototype, UNFI PCB board, 6 x Z573M Nixie tubes
//#define CLOCK_32  //ESP32 prototype, UNFI 6 x IV-11 VFD tubes clock, DHT22 sensor

//#define CLOCK_40  //ESP32 D1 mini, UNFI 6 x IV-11 VFD tubes clock
//#define CLOCK_41  //ESP32 D1 mini, UNFI board, 6 x Z573M Nixie tubes
//#define CLOCK_42  //ESP32 D1 mini, UNFI 6 x IV-11 VFD tubes clock SHT21
//#define CLOCK_43  //ESP32 D1 mini, UNFI 6 x IV-11 VFD tubes clock SHT21
//#define CLOCK_44  //ESP32 D1 mini, Numitron clock
//#define CLOCK_45  //ESP32 D1 mini, UNFI 6 x IV-11 VFD tubes clock (Ovidiu)

#define CLOCK_50   //ESP32 D1 mini, UNFI 2xHV5122 PCB version, 6xZ573 clock   PROTOTYPE TESTING!!!
//#define CLOCK_51   //ESP32 D1 mini, P.S. 2xHV5122 PCB version, 6xIN18 clock   PROTOTYPE TESTING!!!
//#define CLOCK_52   //ESP8266 Wemos D1 mini,UNFI 2xHV5122 PCB version, 6xZ573 clock   PROTOTYPE TESTING!!!
//#define CLOCK_55   // D1 R32 ESP32 (Uno compatible), + NCS312 Nixie clock KIT!!!

//______________________ESP8266 CLOCKS by UNFI and GP ______________________________________________________
#ifdef CLOCK_1   //8266, UNFI PCB clock, 4x IN-16 tubes
  #define DEBUG 
  #define FW "fw1"  //firmware name
  #define MAXBRIGHTNESS 100  
  #define USE_NEOPIXEL 
  byte tubePixels[] = {3,2,1,0};        //4 tubes, single leds, reverse direction
  //#define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN -1    //Dallas temp sensor pin.  If not used, SET TO -1    
  #define MULTIPLEX74141
  const byte digitEnablePins[] = {14,12,13,15};   //IN16 4x tube clock
  const byte ABCDPins[4] =  {2,4,5,0};   
  #define DP_PIN -1             // decimalPoint inside Nixie tube, set -1, if not used!
  #define COLON_PIN   16        //Blinking Colon pin.  If not used, SET TO -1      
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IN-16 Nixie Clock"
#endif

#ifdef CLOCK_2   //8266, UNFI PCB clock, 4x Z-570M tubes
  #define DEBUG 
  #define FW "fw2"  //firmware name
  #define MAXBRIGHTNESS 100  
  //#define USE_NEOPIXEL 
  //#define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN -1    //Dallas temp sensor pin.  
  #define MULTIPLEX74141
  const byte digitEnablePins[] = {13,12,14,15};  
  const byte ABCDPins[4] = {16,5,4,0};
  #define DP_PIN -1             // decimalPoint inside Nixie tube, set -1, if not used!
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
  #define FW "fw3"  //firmware name
  #define MAXBRIGHTNESS 100  
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 4    //Dallas temp sensor pin.  If not used, SET TO -1
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
  #define FW "fw4"  //firmware name
  #define MAXBRIGHTNESS 100
  #define MAX6921
  byte segmentEnablePins[] =  {7,5,1,0,2,4,6,3};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {19,18,17,16};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  (You may define any number)
  //MAX6921 pins
    #define PIN_LE    12  // D6 Shift Register Latch Enable
    #define PIN_CLK   13  // D7 Shift Register Clock
    #define PIN_DATA  14  // D5 Shift Register Data
    #define PIN_BL    15  // D8 Shift Register Blank (1=display off     0=display on)
  #define USE_NEOPIXEL 
  byte tubePixels[] = {0,9,1,9,2,9,3};  //4 tubes, single leds, 3 leds not used
  #define RADAR_PIN 21
  #define RADAR_TIMEOUT 300  //second (5min)
  //#define TUBE_POWER_PIN 2
  //#define TUBE_POWER_ON  LOW
  //#define LIGHT_SENSOR_PIN A0  //Only ADC pins are usable! for example: 34,35,36,39... 
  #define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  #define LUX_CALC_SCALAR   12518931 * 1.2 
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IV-11 VFD Clock"
#endif

#ifdef CLOCK_5   //8266, UNFI PCB clock IVL-2-5/7 tube with RTC
  //#define DEBUG
  #define FW "fw5"  //firmware name 
  #define USE_DALLAS_TEMP
  #define MAXBRIGHTNESS 100
  #define TEMP_DALLAS_PIN 3
  #define USE_RTC    
  #define PIN_MODE_SWITCH  17   //Analog A0 port!!!    
  #define PIN_FLD_BUTTON   16     
  #define PIN_SET_BUTTON   0   
  #define PIN_SDA 5           //D1   SDA/SCL swapped
  #define PIN_SCL 4           //D2
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

#ifdef CLOCK_6   //8266, GP PCB clock, 4x IN-1 tubes and  4x74141  driver  (NON-MULTIPLEX)
  #define DEBUG
  #define FW "fw6"  //firmware name
  #define MAXBRIGHTNESS 100
  //#define USE_DALLAS_TEMP
  //#define TEMP_DALLAS_PIN -1
  #define USE_NEOPIXEL
  byte tubePixels[] = {0,0,1,1,2,2,3,3,3,3,2,2,1,1,0,0,0};  //4 tubes, double row, 17 leds (GP)  
  #define NO_MULTIPLEX74141
  #define TEMP_DALLAS_PIN -1
  #define COLON_PIN   2        //Blinking Colon pin.  If not used, SET TO -1                 ( old IN-1-Clock: white:2 brown: SDA)
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IN-1 Nixie Clock"
#endif


#ifdef CLOCK_7   //8266 UNFI PCB clock 6x Z574M/Z573M tubes, tube selection by PCF8574 chip
  #define DEBUG
  #define FW "fw7"  //firmware name 
  #define MAXBRIGHTNESS 100
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 0
  //#define USE_NEOPIXEL
  //byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  //byte tubePixels[] = {5,4,3,2,1,0};    //6 tubes, single leds, reverse direction 
  #define LEFTDECIMAL true   //set true (Z574M), false to Z573M
  #define TEMP_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define GRAD_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define LIGHT_SENSOR_PIN A0  //Only ADC pins are usable! for example: 34,35,36,39... 
  #define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  #define LUX_CALC_SCALAR   12518931 * 1.2 
  #define PCF_74141
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "Z574M Nixie Clock"
#endif

#ifdef CLOCK_8   //8266 GP PCB v3 clock with 4x IV-16 Numitron tubes (plexi box) //Flash size: 1MB (FS:160k, OTA:422k)
  #define DEBUG
  #define FW "fw8"  //firmware name 
  #define MAXBRIGHTNESS 10  
  #define PCB_VERSION 3   //1,2 or 3
  #define LTBIpin 5
  byte digitEnablePins[] = {13,12,14,16};    //define here the digit enable pins from 4 to 8
  byte ABCDPins[4] = {4,0,2,15};
  #define USE_NEOPIXEL
  byte tubePixels[] = {3,2,6,1,0};    //Numitron 4 tubes, 4 x single leds + 1. The extra led in the middle is not used, is always dark!
  #define Numitron_4511N
  #define ALARMSPEAKER_PIN 1    //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN -1    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "Numitron UniClock"
#endif

#ifdef CLOCK_9   //8266 GP PCB v1 clock with 4x IV-16 Numitron tubes (brown box) //Flash size: 1MB (FS:160k, OTA:422k)
  #define DEBUG
  #define FW "fw9"  //firmware name 
  #define MAXBRIGHTNESS 10
  #define PCB_VERSION 1   //1,2 or 3
  #define Numitron_4511N
  #define LTBIpin 5
  byte digitEnablePins[] = {13,12,14,16};    //define here the digit enable pins from 4 to 8
  byte ABCDPins[4] = {4,0,2,15};
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "Numitron UniClock"
#endif

#ifdef CLOCK_10   //8266 LED Clock with MAX7219 chip
  #define DEBUG
  #define FW "fw10"  //firmware name 
  #define MAX7219CNG
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "LED UniClock"
#endif

#ifdef CLOCK_11   //8266 LED Clock with MM5450 chip
  #define DEBUG
  #define FW "fw11"  //firmware name 
  #define MAXBRIGHTNESS 100  
  //#define USE_DALLAS_TEMP
  //#define TEMP_DALLAS_PIN 2
  //#define RADAR_PIN 4
  #define RADAR_TIMEOUT 300  //second (5min)
  #define LIGHT_SENSOR_PIN A0  //Only ADC pins are usable! for example: 34,35,36,39... 
  #define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  #define MM5450
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "LED UniClock"
#endif

#ifdef CLOCK_12   //8266 VFD Clock with PT6355 chip
  #define DEBUG
  #define FW "fw12"  //firmware name 
  #define PT6355
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "VFD UniClock"
#endif

#ifdef CLOCK_13   //8266 GP PCB v1 clock with 4x IV-16 Numitron tubes + GPS timesync (white box)
  #define DEBUG
  #define FW "fw13"  //firmware name 
  #define MAXBRIGHTNESS 10
  #define USE_GPS
  #define PCB_VERSION 3   //1,2 or 3
  #define Numitron_4511N
  #define LTBIpin 5
  byte digitEnablePins[] = {13,12,14,16};    //define here the digit enable pins from 4 to 8
  byte ABCDPins[4] = {4,0,2,15};
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "Numitron UniClock"
#endif

//____________ P.S. clocks / thermometers ____________________________________________
#ifdef CLOCK_20   //8266 D1-mini, P.S. PCB 4xIN14 clock-thermometer 
  //#define DEBUG 
  #define FW "fw20"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 3 //RX
  #define MULTIPLEX74141
  const byte digitEnablePins[] = {15,13,12,14};   
  const byte ABCDPins[4] =  {2,4,5,0};
  #define DP_PIN  -1 // decimalPoint inside Nixie tube, set -1, if not used!
  #define ENABLE_CLOCK_DISPLAY true  //don't display date/time!!!
  #define DATE_REPEAT_MIN 3       //show date only every xxx minute. If zero, datum is never displayed
  #define SHIFT_TUBES_LEFT_BY_1  //shift left IP address by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 15
  #define GRAD_CHARCODE 16 
  #define PERCENT_CHARCODE 17
  #define COLON_PIN  1  //TX     //Blinking Colon pin.  If not used, SET TO -1               
  #define LED_SWITCH_PIN 16     //external led backlight ON/OFF.  If not used, SET TO -1 
  #define AP_NAME "Nixie ORA"
  #define AP_PASSWORD ""
  #define WEBNAME "Nixie IN-14 Óra"  
#endif

#ifdef CLOCK_21   //8266 D1-mini, P.S. PCB 3xIN14 1xIN-19A thermometer / humidity 
  //#define DEBUG
  #define FW "fw21"  //firmware name 
  #define MAXBRIGHTNESS 100    
  #define USE_DHT_TEMP
  #define DHTTYPE DHT22
  #define TEMP_DHT_PIN  3  //RX
  #define MULTIPLEX74141
  const byte digitEnablePins[] = {15,13,12,14};  
  const byte ABCDPins[4] =  {2,4,5,0};
  #define DP_PIN -1             // decimalPoint inside Nixie tube, set -1, if not used!
  #define ENABLE_CLOCK_DISPLAY false  //don't display date/time!!!
  #define SHIFT_TUBES_LEFT_BY_1 //shift left IP address by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 4
  #define GRAD_CHARCODE 16 
  #define PERCENT_CHARCODE 7
  #define LED_SWITCH_PIN   16   //external led backlight ON/OFF.  If not used, SET TO -1 
  #define DECIMALPOINT_PIN 1 //TX   //Nixie decimal point between digits. If not used, SET TO -1 
  #define DATE_REPEAT_MIN 0       //show date only every xxx minute. If zero, datum is never displayed
  #define TEMP_START  01
  #define TEMP_END    45 
  #define HUMID_START 45 
  #define HUMID_END   59 
  #define AP_NAME "Nixie Homero"
  #define AP_PASSWORD ""
  #define WEBNAME "Nixie Hőmérő"
#endif

#ifdef CLOCK_22   //8266 NODEMCU, P.S. PCB 3xIN14 1xIN-19A thermometer / humidity
  //#define DEBUG
  #define FW "fw22"  //firmware name 
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  byte tubePixels[] = {0,1,2,3};        //4 tubes, single leds
  #define USE_DHT_TEMP
  #define DHTTYPE DHT22
  #define TEMP_DHT_PIN  1
  #define MULTIPLEX74141
  const byte digitEnablePins[] = {15,13,12,14};  
  const byte ABCDPins[4] =  {2,4,5,0};
  #define DP_PIN -1             // decimalPoint inside Nixie tube, set -1, if not used!
  #define ENABLE_CLOCK_DISPLAY false  //don't display date/time!!!
  #define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 5
  #define GRAD_CHARCODE 16 
  #define PERCENT_CHARCODE 6
  #define DECIMALPOINT_PIN  16  //Nixie decimal point between digits. If not used, SET TO -1 
  #define DATE_REPEAT_MIN 0       //show date only every xxx minute. If zero, datum is never displayed
  #define TEMP_START  01
  #define TEMP_END    45 
  #define HUMID_START 45 
  #define HUMID_END   59 
  #define AP_NAME "Nixie Homero"
  #define AP_PASSWORD ""
  #define WEBNAME "Nixie Hőmérő"
#endif


//______________________ESP-32 CLOCKS  (2x18pin ESP32 modul) ______________________________________________________
#ifdef CLOCK_30   //ESP32, UNFI PCB clock, 6 x IV-11 VFD tubes
  #define DEBUG
  #define FW "fw30"  //firmware name 
  #define MAXBRIGHTNESS 100
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 2
  #define USE_MQTT
  byte mac[6] = {12,14,0,0,2,3};  //Unique address
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define DATE_REPEAT_MIN 3     //show date only every 3 minutes. If zero, datum is never displayed
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 23  //15    //Dallas temp sensor pin.  If not used, SET TO -1     
  #define MAX6921_ESP32
  byte segmentEnablePins[] =  {19,17,15,12,13,16,18,14};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {9,8,7,2,1,0};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6)  (You may define any number)
  #define DOUBLE_BLINK  //both separator points are blinking 
  //MAX6921 pins
    #define PIN_LE    14  // Shift Register Latch Enable
    #define PIN_CLK   13  // Shift Register Clock
    #define PIN_DATA  27  // Shift Register Data
    #define PIN_BL    12  // Shift Register Blank (1=display off     0=display on)
  //#define PIN_SDA 21          // you can set the used SDA and SCL pins
  //#define PIN_SCL 22           // if it is not default value    
  #define ALARMSPEAKER_PIN 33   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN -1  //32    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define RADAR_PIN -1
  #define RADAR_TIMEOUT 300  //second (5min)
  //#define TUBE_POWER_PIN 25
  //#define TUBE_POWER_ON  LOW
  #define LIGHT_SENSOR_PIN 35  //Only ADC pins are usable! for example: 34,35,36,39... 
  #define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  #define LUX_CALC_SCALAR   12518931 * 1.2 
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""  
  #define WEBNAME "GP IV-11"
#endif

#ifdef CLOCK_31   //ESP32, UNFI board, 6 x Z573M Nixie tubes
  #define DEBUG
  #define FW "fw31"  //firmware name 
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 2
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 23    //Dallas temp sensor pin.  If not used, SET TO -1    
  #define MULTIPLEX74141_ESP32
  const byte digitEnablePins[] = {4,16,17,5,18,19};   //ESP32 6x tube Clock
  const byte ABCDPins[4] =  {12,27,14,13};   
  #define DP_PIN 15             // decimalPoint inside Nixie tube, set -1, if not used!
  #define LEFTDECIMAL false   //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  //#define TEMP_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define GRAD_CHARCODE -1   //disable char => shift display right with 1 digit
  #define ALARMSPEAKER_PIN 33   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 32    //Alarm switch off button pin 
  #define ALARM_ON HIGH
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""
  #define WEBNAME "ESP32UniClock 3.0"
#endif


#ifdef CLOCK_32   //ESP32, UNFI 6 x IV-11 VFD tubes clock, DHT22 sensor
  #define DEBUG
  #define FW "fw32"  //firmware name 
  #define MAXBRIGHTNESS 100  
  #define USE_MQTT
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 2
  #define LIGHT_SENSOR_PIN 23
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DHT_TEMP
  #define DHTTYPE DHT22
  #define TEMP_DHT_PIN 23    //DHT temp sensor pin.  If not used, SET TO -1     
  #define MAX6921_ESP32
  byte segmentEnablePins[] =  {19,17,15,12,13,16,18,14};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {9,8,7,2,1,0};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6)  (You may define any number)
  //MAX6921 pins
    #define PIN_LE    14  // Shift Register Latch Enable
    #define PIN_CLK   13  // Shift Register Clock
    #define PIN_DATA  27  // Shift Register Data
    #define PIN_BL    12  // Shift Register Blank (1=display off     0=display on)
  //#define ALARMSPEAKER_PIN 33   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 32    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""  
  #define WEBNAME "ESP32UniClock 3.0"
#endif

//______________________ESP-32 CLOCKS  (2x20pin ESP32 D1 mini modul) ______________________________________________________
#ifdef CLOCK_40   //ESP32 D1 mini, UNFI 6 x IV-11 VFD tubes clock
  #define DEBUG
  #define FW "fw40"  //firmware name 
  #define MAXBRIGHTNESS 100
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 22
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  //#define USE_DALLAS_TEMP
  //#define TEMP_DALLAS_PIN -1    //Dallas temp sensor pin.  If not used, SET TO -1    
  #define USE_DHT_TEMP
  #define DHTTYPE DHT22
  #define TEMP_DHT_PIN 25    //DHT temp sensor pin.  If not used, SET TO -1     
  #define MAX6921_ESP32
  byte segmentEnablePins[] =  {19,17,15,12,13,16,18,14};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {9,8,7,2,1,0};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6)  (You may define any number)
  #define DOUBLE_BLINK  //both separator points are blinking 
  //MAX6921 pins
    #define PIN_LE    4  // Shift Register Latch Enable
    #define PIN_CLK   17  // Shift Register Clock
    #define PIN_DATA  16  // Shift Register Data
    #define PIN_BL    32  // Shift Register Blank (1=display off     0=display on)
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 0    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""  
  #define WEBNAME "ESP32 IV-11 VFD-Clock"
#endif

#ifdef CLOCK_41   //ESP32 D1 mini, UNFI board, 6 x Z573M Nixie tubes
  #define DEBUG
  #define FW "fw41"  //firmware name 
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 22
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 25
  #define TEMP_DALLAS_PIN 25    //Dallas temp sensor pin.  If not used, SET TO -1    
  #define MULTIPLEX74141_ESP32
  const byte digitEnablePins[] = {26,18,33,19,23,5};   //ESP32 6x tube Clock
  const byte ABCDPins[4] =  {32,16,4,17};   
  #define DP_PIN 27             // decimalPoint inside Nixie tube, set -1, if not used!
  #define LEFTDECIMAL false   //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  //#define TEMP_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define GRAD_CHARCODE -1   //disable char => shift display right with 1 digit
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 0    //Alarm switch off button pin 
  #define ALARM_ON HIGH
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""
  #define WEBNAME "ESP32 Z573M Nixie-Clock"
#endif


#ifdef CLOCK_42   //ESP32 D1 mini, UNFI 6 x IV-11 VFD tubes clock, SHT21
  #define DEBUG
  #define FW "fw42"  //firmware name 
  #define MAXBRIGHTNESS 100
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 22
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 25    //Dallas temp sensor pin.  If not used, SET TO -1    
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 25    //DHT temp sensor pin.  If not used, SET TO -1     
  //#define USE_BME280            //I2C Temperature + humidity + pressure
  //#define USE_BMP280            //I2C Temperature + barometric  pressure
  //#define USE_AHTX0             //I2C Temperature + humidity
  //#define USE_SHT21             //I2C Temperature + humidity
  //#define USE_RTC
  #define USE_MQTT
  #define PIN_SDA 26           // you can set the used SDA and SCL pins
  #define PIN_SCL 27           // if it is not default value
  #define MAX6921_ESP32
  byte segmentEnablePins[] =  {19,17,15,12,13,16,18,14};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {9,8,7,2,1,0};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6)  (You may define any number)
  #define DOUBLE_BLINK  //both separator points are blinking 
  #define DATE_REPEAT_MIN 60       //show date only every xxx minute. If zero, datum is never displayed
  #define DATE_START  07
  #define DATE_END    12 
  #define TEMP_START  25
  #define TEMP_END    30 
  #define HUMID_START 30 
  #define HUMID_END   35 
  //MAX6921 pins
    #define PIN_LE    4  // Shift Register Latch Enable
    #define PIN_CLK   17  // Shift Register Clock
    #define PIN_DATA  16  // Shift Register Data
    #define PIN_BL    32  // Shift Register Blank (1=display off     0=display on)
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 0    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "VFD-Ora"
  #define AP_PASSWORD ""  
  #define WEBNAME "VFD-Óra"
#endif

#ifdef CLOCK_43   //ESP32 D1 mini, UNFI 6 x IV-11 VFD tubes clock, SHT21 & DHT22 sensor
  #define DEBUG
  #define FW "fw43"  //firmware name 
  #define MAXBRIGHTNESS 100
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 22
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  //#define USE_DALLAS_TEMP
  //#define TEMP_DALLAS_PIN -1    //Dallas temp sensor pin.  If not used, SET TO -1    
  //#define USE_DHT_TEMP
  #define DHTTYPE DHT22
  #define TEMP_DHT_PIN 25    //DHT temp sensor pin.  If not used, SET TO -1     
  #define USE_SHT21          //I2C Temperature + humidity
  #define PIN_SDA 26         // you can set the used SDA and SCL pins
  #define PIN_SCL 27         // if it is not default value
  #define MAX6921_ESP32
  byte segmentEnablePins[] =  {19,17,15,12,13,16,18,14};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {9,8,7,2,1,0};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6)  (You may define any number)
  #define DOUBLE_BLINK  //both separator points are blinking 
  #define DATE_REPEAT_MIN 1       //show date only every xxx minute. If zero, datum is never displayed
  #define DATE_START  00
  #define TEMP_END    35 
  #define HUMID_START 35 
  #define HUMID_END   40 
  //MAX6921 pins
    #define PIN_LE    4   // Shift Register Latch Enable
    #define PIN_CLK   17  // Shift Register Clock
    #define PIN_DATA  16  // Shift Register Data
    #define PIN_BL    32  // Shift Register Blank (1=display off     0=display on)
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 0    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "VFD-Óra"
  #define AP_PASSWORD ""  
  #define WEBNAME "VFD-Óra"
#endif

#ifdef CLOCK_44   //ESP32 D1 mini, UNFI board, 6 x IV-9 Numitron tubes
  #define DEBUG
  #define FW "fw44"  //firmware name 
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 22
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 27
  #define TEMP_DALLAS_PIN 27    //Dallas temp sensor pin.  If not used, SET TO -1    
  //#define LIGHT_SENSOR_PIN 35
  #define PIN_SDA 5             // you can set the used SDA and SCL pins
  #define PIN_SCL 0             // if it is not default value
  #define USE_BH1750            //I2C luxmeter sensor
  #define Numitron_4511N
  #define PCB_VERSION 32
  #define LTBIpin 23
  const byte digitEnablePins[] = {4,16,17,32,21,25};   //ESP32 6x tube Clock
  const byte ABCDPins[4] =  {19,18,26,33};   
  #define DP_PIN 23             // decimalPoint inside Nixie tube, set -1, if not used!
  #define LEFTDECIMAL false   //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  //#define TEMP_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define GRAD_CHARCODE -1   //disable char => shift display right with 1 digit
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 34    //Alarm switch off button pin 
  #define ALARM_ON HIGH
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""
  #define WEBNAME "ESP32 Numitron-Clock"
#endif

#ifdef CLOCK_45   //ESP32 D1 mini, UNFI 6 x IV-11 VFD tubes clock, (Ovidiu)
  #define DEBUG
  #define FW "fw45"  //firmware name
  #define MAXBRIGHTNESS 100
  #define USE_RTC 
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 22
  #define USE_MQTT
  byte mac[6] = {7,6,0,0,3,2};  //This is the unique ID - set it as you want or mac address of the clock (length: 6) VFD
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  //#define USE_DALLAS_TEMP
  //#define TEMP_DALLAS_PIN 25    //Dallas temp sensor pin.  If not used, SET TO -1    
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 25    //DHT temp sensor pin.  If not used, SET TO -1
  #define USE_BH1750     
  #define USE_BME280            //I2C Temperature + humidity + pressure
  #define USE_BMP280            //I2C Temperature + barometric  pressure
  #define USE_AHTX0             //I2C Temperature + humidity
  #define USE_SHT21             //I2C Temperature + humidity
  #define PIN_SDA 26           // you can set the used SDA and SCL pins
  #define PIN_SCL 27           // if it is not default value
  #define MAX6921_ESP32
  byte segmentEnablePins[] =  {19,17,15,12,13,16,18,14};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {9,8,7,2,1,0};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6)  (You may define any number)
  #define DOUBLE_BLINK  //both separator points are blinking 
  #define DATE_REPEAT_MIN 3      //show date only every xxx minute. If zero, datum is never displayed
  #define DATE_START  07
  #define DATE_END    12 
  #define TEMP_START  25
  #define TEMP_END    30 
  #define HUMID_START 30 
  #define HUMID_END   35 
  //MAX6921 pins
    #define PIN_LE    4  // Shift Register Latch Enable
    #define PIN_CLK   17  // Shift Register Clock
    #define PIN_DATA  16  // Shift Register Data
    #define PIN_BL    32  // Shift Register Blank (1=display off     0=display on)
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 0    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define TUBE_POWER_PIN 15
  #define RADAR_PIN 21
  #define RADAR_TIMEOUT 160  //second (5min)
  #define TUBE_POWER_ON  HIGH
  //#define LIGHT_SENSOR_PIN 35  //Only ADC pins are usable! for example: 34,35,36,39... 
  #define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  #define LUX_CALC_SCALAR   12518931 * 1.2 
  #define AP_NAME "Uniclock VFD"
  #define AP_PASSWORD ""  
  #define WEBNAME "Uniclock VFD"
#endif

#ifdef CLOCK_50   //ESP32 D1 mini,UNFI 2xHV5122 PCB version, 6xIN18 clock   Example clock!!!
  #define DEBUG
  #define FW "fw50"  //firmware name 
  #define MAXBRIGHTNESS 100
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 27
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  //#define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 26    //Dallas temp sensor pin.  If not used, SET TO -1   
  //#define LIGHT_SENSOR_PIN 23
  #define PIN_SDA 4             // you can set the used SDA and SCL pins
  #define PIN_SCL 32             // if it is not default value
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT11
  //#define TEMP_DHT_PIN  23
  //#define USE_BME280            //I2C Temperature + humidity + pressure
  //#define USE_BMP280            //I2C Temperature + barometric  pressure
  //#define USE_AHTX0             //I2C Temperature + humidity
  //#define USE_SHT21             //I2C Temperature + humidity
  #define USE_BH1750            //I2C luxmeter sensor
//_______________________________ HV5122 setup ____________________________________________________  
  #define HV5122
  #define PIN_DIN  22   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  17   // Clock
  #define PIN_OE   21   // OutputEnable
  const int maxDigits = 6;
  byte digitPins[maxDigits+1][10] = {            ////Data pin numbers 100+ means: chip1 pins are used. Chip0-s DOUT is connected to chip1's DIN
    {2,10,9,8,7,6,5,4,3,1},                      //sec  1 , chip0  (tube#0)
    {11,32,20,19,18,17,16,15,14,13},             //sec  10 , chip0 (tube#1)
    {22,31,29,30,27,28,25,26,23,24},             //min   1 , chip0 (tube#2)
    {101,131,110,109,108,107,106,105,104,103},   //min  10 , chip1 (tube#3)
    {111,132,120,119,118,117,116,115,114,113},   //hour  1 , chip1 (tube#4)
    {122,129,130,127,128,125,126,123,124,121},   //hour 10 , chip1 (tube#5)
    {0,12,21,102,112,0,    0,0,0,0}              //extra decimalPoint/blinking dots (tube0...tube6)
    };    
   //#define MAKE_BLINKING_DOTS //it means, the extra datapins are used as 4 blinking dot instead of decimal points!  #1..#4 positions are used
//___________________________________________________________________________________    
  #define ALARMSPEAKER_PIN 16   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 23    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define RADAR_PIN 4
  #define RADAR_TIMEOUT 300  //second (5min)
  //#define TUBE_POWER_PIN 4
  #define TUBE_POWER_ON  LOW
  #define ENABLE_CLOCK_DISPLAY true  
  #define DATE_REPEAT_MIN 3       //show date only every xxx minute. If zero, datum is never displayed
  //#define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 10
  #define GRAD_CHARCODE 10 
  #define PERCENT_CHARCODE 10
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD "" 
  #define WEBNAME "Nixie Clock HV5122"
#endif

#ifdef CLOCK_51   //ESP32 D1 mini, P.S. 2xHV5122 PCB version, 6xIN18 clock   PROTOTYPE TESTING!!!
  #define DEBUG
  #define FW "fw51"  //firmware name 
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 22
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  //#define USE_DALLAS_TEMP
  //#define TEMP_DALLAS_PIN 26    //Dallas temp sensor pin.  If not used, SET TO -1   
  //#define LIGHT_SENSOR_PIN 23
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT11
  //#define TEMP_DHT_PIN  23
  //#define USE_BME280            //I2C Temperature + humidity + pressure
  //#define USE_BMP280            //I2C Temperature + barometric  pressure
  //#define USE_AHTX0             //I2C Temperature + humidity
  #define USE_SHT21               //I2C Temperature + humidity
  #define PIN_SDA 4               // you can set the used SDA and SCL pins
  #define PIN_SCL 32              // if it is not default value
//_______________________________ HV5122 setup ____________________________________________________    
  #define HV5122
  #define PIN_DIN  16   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  17   // Clock
  #define PIN_OE   21   // OutputEnable
  const int maxDigits = 6;
  byte digitPins[maxDigits+1][10] = {
    {4,5,6,8,2,1,10,9,7,3},                               //hour 10 , CHIP0
    {16,17,18,20,14,13,22,21,19,15},                      //hour 1 , CHIP0
    {26,27,28,30,24,23,32,31,29,25},                      //min 10 , CHIP0
    {104,105,106,108,102,101,110,109,107,103},            //min 1 , CHIP1
    {116,117,118,120,114,113,122,121,119,115},            //secound 10 , CHIP1
    {126,127,128,130,124,123,132,131,129,125},            //secound 1 , CHIP1
    {0,12,11,112,111,0,   0,0,0,0}                        //extra GL dots  //GL1,GL2,GL3,GL4
    };    
    #define MAKE_BLINKING_DOTS //it means, the extra datapins are used as 4 blinking dot instead of decimal points!  #1..#4 positions are used, #0. and  #5..#9. positions are zero!
//_______________________________________________________________________________________    
  #define ALARMSPEAKER_PIN 23   //Alarm buzzer pin                                            
  //#define ALARMBUTTON_PIN 23    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define RADAR_PIN 4
  //#define RADAR_TIMEOUT 300  //second (5min)
  //#define TUBE_POWER_PIN 4
  //#define TUBE_POWER_ON  LOW
  #define ENABLE_CLOCK_DISPLAY true  
  #define DATE_REPEAT_MIN 60       //show date only every xxx minute. If zero, datum is never displayed
  #define DATE_START  07
  #define DATE_END    12 
  #define TEMP_START  25
  #define TEMP_END    30 
  #define HUMID_START 30 
  #define HUMID_END   35 
  #define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 10
  #define GRAD_CHARCODE 10 
  #define PERCENT_CHARCODE 10
  #define AP_NAME "IN-18 Ora"
  #define AP_PASSWORD "" 
  #define WEBNAME "Nixie IN-18 Óra"
#endif

#ifdef CLOCK_52   //ESP8266 Wemos D1 mini,UNFI 2xHV5122 PCB version, 6xIN18 clock  
  #define DEBUG
  #define FW "fw52"  //firmware name 
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 3
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 16    //Dallas temp sensor pin.  If not used, SET TO -1   
  //#define LIGHT_SENSOR_PIN 23
  //#define PIN_SDA 4             // you can set the used SDA and SCL pins
  //#define PIN_SCL 32             // if it is not default value
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT11
  //#define TEMP_DHT_PIN  23
  //#define USE_BME280            //I2C Temperature + humidity + pressure
  //#define USE_BMP280            //I2C Temperature + barometric  pressure
  //#define USE_AHTX0             //I2C Temperature + humidity
  //#define USE_SHT21             //I2C Temperature + humidity
//_______________________________ HV5122 setup ____________________________________________________    
  #define HV5122
  #define PIN_DIN  5   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  0   // Clock
  #define PIN_OE   4   // OutputEnable
  const int maxDigits = 6;
  byte digitPins[maxDigits+1][10] = {
    {2,10,9,8,7,6,5,4,3,1},            //sec  1 , chip0
    {11,32,20,19,18,17,16,15,14,13},  //sec  10 , chip0
    {22,31,29,30,27,28,25,26,23,24},  //min   1 , chip0
    {101,131,110,109,108,107,106,105,104,103},   //min  10 , chip1
    {111,132,120,119,118,117,116,115,114,113},   //hour  1 , chip1
    {122,129,130,127,128,125,126,123,124,121},   //hour 10 , chip1
    {0,12,21,102,112,0,    0,0,0,0}              //extra decimal point (tube0...tube6)
    };    
  //#define MAKE_BLINKING_DOTS //it means, the extra datapins are used as 4 blinking dot instead of decimal points!  #1..#4 positions are used, #0. and  #5..#9. positions are zero!    
//__________________________________________________________________________________________________      
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 13    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define RADAR_PIN 4
  #define RADAR_TIMEOUT 300  //second (5min)
  //#define TUBE_POWER_PIN 4
  #define TUBE_POWER_ON  LOW
  #define ENABLE_CLOCK_DISPLAY true  
  #define DATE_REPEAT_MIN 3       //show date only every xxx minute. If zero, datum is never displayed
  //#define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 10
  #define GRAD_CHARCODE 10 
  #define PERCENT_CHARCODE 10
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD "" 
  #define WEBNAME "UNFI Nixie Clock HV5122"
#endif

#ifdef CLOCK_55   // D1 R32 ESP32 (Uno compatible), + NCS312 Nixie clock KIT!!!
  #define DEBUG
  #define FW "fw55"  //firmware name
  #define DOUBLE_BLINK 
  //#define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 22
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 14    //Dallas temp sensor pin.  If not used, SET TO -1   
  //#define LIGHT_SENSOR_PIN 23
  #define PIN_SDA 26     //on ESP32 only below GPIO34 are digit pins! 36..39 are INPUT ONLY!            
  #define PIN_SCL 27        
  #define USE_RTC
  #define USE_MQTT
  //#define PIN_MODE_SWITCH 2   ////Uno: A0 mode onNCS312 board
  //#define PIN_FLD_BUTTON  4   ////Uno: A1 down
  //#define PIN_SET_BUTTON 35   ////Uno: A2 up
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT11
  //#define TEMP_DHT_PIN  23
  //#define USE_BME280            //I2C Temperature + humidity + pressure
  //#define USE_BMP280            //I2C Temperature + barometric  pressure
  //#define USE_AHTX0             //I2C Temperature + humidity
  //#define USE_SHT21             //I2C Temperature + humidity
  #define HV5122
  #define PIN_DIN  23   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  18   // Clock
  #define PIN_OE   5  // OutputEnable
  const int maxDigits = 6;
  byte digitPins[maxDigits+1][10] = {
    {121,122,123,124,125,126,127,128,129,130},  //sec   1 , chip1
    {111,112,113,114,115,116,117,118,119,120},  //sec  10 , chip1
    {101,102,103,104,105,106,107,108,109,110},  //min   1 , chip1
    {21,22,23,24,25,26,27,28,29,30},            //min  10 , chip0
    {11,12,13,14,15,16,17,18,19,20},            //hour  1 , chip0
    {1,2,3,4,5,6,7,8,9,10},                     //hour 10 , chip0
    {0,131,132,31,32,0,0,0,0,0}                 //extra GL dots
    };    
  #define MAKE_BLINKING_DOTS 
  #define USE_PWMLEDS
  #define PWM1_PIN 13  //Uno: D9 Red
  //#define PWM2_PIN 27  //Uno: D6 Green
  #define PWM3_PIN 25  //Uno: D3 Blue
  //#define ALARMSPEAKER_PIN 17   //Alarm buzzer pin                                            
  //#define ALARMBUTTON_PIN 2    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define RADAR_PIN 4
  //#define RADAR_TIMEOUT 300  //second (5min)
  //#define TUBE_POWER_PIN 16
  #define TUBE_POWER_ON  LOW
  #define ENABLE_CLOCK_DISPLAY true  
  #define TEMP_CHARCODE 10
  #define GRAD_CHARCODE 10 
  #define PERCENT_CHARCODE 10
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD "" 
  #define WEBNAME "NCS312 Nixie Clock"
#endif


//__________________________________________________________________________________________________________________________
//if not defined, here is defined as NOT USED
#ifndef COLON_PIN
  #define COLON_PIN   -1        //Blinking Colon pin.
#endif

#ifndef TEMP_DALLAS_PIN
  #define TEMP_DALLAS_PIN -1     //Dallas temp sensor pin.
#endif

#ifndef TEMP_DHT_PIN
  #define TEMP_DHT_PIN -1     //DHT temp sensor pin.
#endif

#if defined(USE_BME280) || defined(USE_BMP280) || defined(USE_AHTX0) || defined(USE_SHT21) || defined(USE_BH1750) || defined(USE_RTC)
  #define USE_I2CSENSORS
#endif

#ifndef LED_SWITCH_PIN
  #define LED_SWITCH_PIN -1     //external led backlight ON/OFF.
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

#ifdef USE_NEOPIXEL
  #ifndef NEOPIXEL_PIN
    #define NEOPIXEL_PIN 3
  #endif 
#endif
#ifndef RGB_MIN_BRIGHTNESS
  #define RGB_MIN_BRIGHTNESS 8   //Neopixel leds minimum brightness
#endif
#ifndef  RGB_MAX_BRIGHTNESS
  #define RGB_MAX_BRIGHTNESS 255 //Neopixel leds maximum brightness
#endif

#ifndef TUBE_POWER_PIN
  #define TUBE_POWER_PIN -1    //Tube filament or HV ON/OFF
#endif
#ifndef TUBE_POWER_PIN
  #define TUBE_POWER_ON LOW    //Tube Power On is HIGH or LOW?
#endif
  
#ifndef RADAR_PIN
  #define RADAR_PIN -1    //Radar sensor for automatic ON/OFF the TUBE_POWER
#endif

#ifndef LIGHT_SENSOR_PIN
  #define LIGHT_SENSOR_PIN -1    ///Environment light sensor, analog input pin
#endif

#ifndef RADAR_TIMEOUT
  #define RADAR_TIMEOUT 300     //Automatic switch off tubes (without radar detecting somebody) after xxx sec
#endif

#ifndef ALARM_ON
  #define ALARM_ON HIGH  
#endif  

#ifndef MAXBRIGHTNESS 
  #define MAXBRIGHTNESS 10   //do not change this value
#endif 

//Display temperature and date in every minute between START..END seconds
#ifndef ENABLE_CLOCK_DISPLAY 
  #define ENABLE_CLOCK_DISPLAY true  //false, if no clock display is needed (for example: thermometer + humidity only)
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

#ifndef LEFTDECIMAL
  #define LEFTDECIMAL false   //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
#endif

//______________________________________________________________________________________
//--------------- LDR LIGHT_SENSOR LUXMETER parameters ------------
#ifndef LIGHT_SENSOR_PIN
 #define LIGHT_SENSOR_PIN -1
#endif 
#if defined(ESP32) 
  #define MAX_ADC_READING 4095    //ESP32 ADC_REF_VOLTAGE = 3.3V !!
#else
  #define MAX_ADC_READING 1023    //8266  ADC_REF_VOLTAGE = 1.0V !!
#endif
#ifndef REF_RESISTANCE
  #define REF_RESISTANCE    10000.0   // Resistor value is 10k, between LDR sensor and GND
#endif
#ifndef LUX_CALC_SCALAR
  #define LUX_CALC_SCALAR   12518931 * 1.2   //Calibrate here 
#endif
#ifndef LUX_CALC_EXPONENT
  #define LUX_CALC_EXPONENT -1.405
#endif
#ifndef MAXIMUM_LUX
  #define MAXIMUM_LUX 100
#endif
#ifndef PWM1_PIN
  #define PWM1_PIN -1
#endif
#ifndef PWM2_PIN
  #define PWM2_PIN -1
#endif
#ifndef PWM3_PIN
  #define PWM3_PIN -1
#endif
#ifndef DATE_REPEAT_MIN
  #define DATE_REPEAT_MIN 1
#endif

//______________________________________________________________________________________

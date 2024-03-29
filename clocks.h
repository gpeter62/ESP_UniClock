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
//#define CLOCK_6   //8266,  GP PCB clock,   4x IN-1 tubes and  4x74141  driver (NON-MULTIPLEX)
//#define CLOCK_6A   //8266, UNFI PCB clock, 4x Z568 tubes and  4x74141  driver  (NON-MULTIPLEX)
//#define CLOCK_7   //8266 UNFI PCB clock 6x Z574M/Z573M tubes, tube selection by PCF8574 chip
//#define CLOCK_8   //8266 GP PCB v3 clock with 4x IV-16 Numitron tubes (plexi box)
//#define CLOCK_9   //8266 GP PCB v1 clock with 4x IV-16 Numitron tubes (brown box)
//#define CLOCK_10  //8266 LED Clock with MAX7219 chip
//#define CLOCK_11  //8266 LED Clock with MM5450 chip
//#define CLOCK_12  //8266 VFD Clock with PT6355 chip
//#define CLOCK_13   //8266 GP PCB v1 clock with 4x IV-16 Numitron tubes + GPS timesync (white box)
//#define CLOCK_14  //8266 6X LD8035 VFD-clock
//#define CLOCK_15  //8266 6X IV-6 VFD-clock RGB
//#define CLOCK_15a  //8266 6X IV-6 VFD-clock RGB
//#define CLOCK_16  //8266 4X IV-6, 1X LD8035
//#define CLOCK_17  //8266 + PT6311 4X IV-22, russian clock
//#define CLOCK_18  //8266 IV-18 VFD-clock RGB
//#define CLOCK_20  //8266 D1-mini, P.S. PCB 4xIN14 clock-thermometer 
//#define CLOCK_21  //8266 D1-mini, P.S. PCB 4xIN14 thermometer / humidity 
//#define CLOCK_22  //8266 NODEMCU, P.S. PCB 4xIN14 thermometer / humidity
//#define CLOCK_23    ////ESP32 D1 mini, P.S. PCB 3xIN14 1xIN-19A thermometer / humidity
//#define CLOCK_23A   //ESP32 WROOM, U.Z. PCB 4xZ566M 1xIN-19A thermometer / humidity
//#define CLOCK_23B   //ESP32 WROOM U.Z. PCB Z571M+4xZ566M+IN-19A clock/thermometer/humidity
//#define CLOCK_26  //ESP32 D1 mini, UNFI 6 x IV-11 VFD tubes clock, SHT21
//#define CLOCK_30  //ESP32 prototype, UNFI PCB clock, 6 x IV-11 VFD tubes
//#define CLOCK_31  //ESP32 prototype, UNFI PCB board, 6 x Z573M Nixie tubes
//#define CLOCK_32  //ESP32 prototype, UNFI 6 x IV-11 VFD tubes clock, DHT22 sensor
//#define CLOCK_33   //TOM025 ESP32, Pálfi S. board, 6 x Z573M Nixie tubes
//#define CLOCK_34   //Mule V2 ESP32, board, 6 x SA40 LED Display
#define CLOCK_35   //ESP32, 6x Z566M and 3x74595 6x74141 (NON-MULTIPLEX)
//#define CLOCK_36   //ESP32, 4x IN-1 tubes and  4x74141  driver  (NON-MULTIPLEX)
//#define CLOCK_40  //V1  ESP32, UNFI 6 x IV-11 VFD tubes clock
//#define CLOCK_41  //V2  ESP32, UNFI 6 x IV-11 VFD tubes clock (átkötés)
//#define CLOCK_42  //V3  ESP32, UNFI 6 x IV-11 VFD tubes clock
//#define CLOCK_43  //V1  ESP32, UNFI 6 x Z573M Nixie tubes
//#define CLOCK_44  //V2  ESP32, UNFI 6 x Z573M Nixie tubes (átkötés)
//#define CLOCK_45  //V3  ESP32, UNFI 6 x Z573M Nixie tubes
//#define CLOCK_46  //V1  ESP32, UNFI 6 x IV-9 Numitron clock
//#define CLOCK_47  //V3  ESP32, UNFI 10-LT-50G VFD tubes clock
//#define CLOCK_48  //WROOM ESP32, UNFI  SN75518 6xIV-17 clock
//#define CLOCK_49  //WROOM ESP32, UNFI  SN75518 6xIV-11 clock
//--------------------------------------------------------------------------------------------------------------
//#define CLOCK_50   //V1  ESP32, UNFI 2xHV5122 PCB version, 6xZ573 clock
//#define CLOCK_51   //V2  ESP32, UNFI 2xHV5122 PCB version, 6xZ573 clock
//#define CLOCK_52   //ESP32,    P.S. 2xHV5122 PCB version, 6xIN18 clock   PROTOTYPE TESTING!!!
//#define CLOCK_53   //ESP8266,  UNFI 2xHV5122 PCB version, 6xZ573 clock   PROTOTYPE TESTING!!!
//#define CLOCK_54   // D1 R32 ESP32 (Uno compatible), + NCS312 Nixie clock KIT!!!
//#define CLOCK_55   //ESP8266,  UNFI 2xHV5122 PCB version, 6xZ573 clock
//#define CLOCK_56   //WROOM ESP32, UNFI 2xHV5122 6xZ573 clock   /flat panel/
//#define CLOCK_57   //V2-WROOM ESP32, UNFI 2xHV5122 6xZM1000 clock  /flat panel
//#define CLOCK_57A  //V2-WROOM ESP32, UNFI 2xHV5222 6xZ57xM clock  /flat panel
//#define CLOCK_70   //1 tube esp8266 Nixie Clock by UNFI 
//#define CLOCK_71   //2x VQC10 clock by UNFI 
//#define CLOCK_72   //ESP32, UNFI board, ZM1500
//#define CLOCK_80   //word clock, english version
//#define CLOCK_81   //word clock, german version
//#define CLOCK_82   //word clock, hungarian version
//#define CLOCK_99   //Dummy clock, sensor box with MQTT connector

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
  byte digitEnablePins[] = {14,12,13,15};   //IN16 4x tube clock
  byte ABCDPins[4] =  {2,4,5,0};   
  #define DP_PIN -1             // decimalPoint inside Nixie tube, set -1, if not used!
  #define COLON_PIN   16        //Blinking Colon pin.  If not used, SET TO -1      
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IN-16 Nixie Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER   
#endif

#ifdef CLOCK_2   //8266, UNFI PCB clock, 4x Z-570M tubes
  #define DEBUG 
  #define FW "fw2"  //firmware name
  #define MAXBRIGHTNESS 100  
  //#define USE_NEOPIXEL 
  //#define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN -1    //Dallas temp sensor pin.  
  #define MULTIPLEX74141
  byte digitEnablePins[] = {13,12,14,15};  
  byte ABCDPins[4] = {16,5,4,0};
  #define DP_PIN -1             // decimalPoint inside Nixie tube, set -1, if not used!
  #define COLON_PIN   2        //Blinking Colon pin.  If not used, SET TO -1          
  #define ALARMSPEAKER_PIN -1   //Alarm buzzer pin                             
  #define ALARMBUTTON_PIN -1    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "Z-570M Nixie Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER 
  //#define DISABLE_NIGHT_ANIMATION
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
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER   
  //#define DISABLE_NIGHT_ANIMATION
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
  //#define RADAR_PIN 21
  #define RADAR_TIMEOUT 5  //(min)
  //#define TUBE_POWER_PIN 2
  //#define TUBE_POWER_ON  LOW
  #define LIGHT_SENSOR_PIN A0  //Only ADC pins are usable! for example: 34,35,36,39... 
  #define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  #define LUX_CALC_SCALAR   12518931 * 1.2 
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IV-11 VFD Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER 
  //#define DISABLE_NIGHT_ANIMATION  
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
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER 
  //#define DISABLE_NIGHT_ANIMATION  
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
  byte tubes[] = {3,2,1,0};         //4 tubes,   old clock...   in clocks.h    
  #define dataPin  14  //D5
  #define latchPin 12  //D6
  #define clkPin   13  //D7
  #define TEMP_DALLAS_PIN -1
  #define COLON_PIN   2        //Blinking Colon pin.  If not used, SET TO -1                 ( old IN-1-Clock: white:2 brown: SDA)
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IN-1 Nixie Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION   
#endif

#ifdef CLOCK_6A   //8266, UNFI PCB clock, 4x Z568 tubes and  4x74141  driver  (NON-MULTIPLEX)
  #define DEBUG
  #define FW "fw6a"  //firmware name
  #define MAXBRIGHTNESS 100
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 4
  #define GRAD_CHARCODE -1   //no display grad char
  #define TEMP_CHARCODE -1   //no display temp char
  //#define USE_NEOPIXEL
  //byte tubes[] = {2,3,0,1};         //4 tubes,   old clock...  
  byte tubes[] = {5,4,2,3,0,1};       //6 tubes,   Kovecses clock...  
  #define NO_MULTIPLEX74141  
  #define dataPin  14  //D5
  #define latchPin 12  //D6
  #define clkPin   13  //D7  
  #define COLON_PIN   15        //Blinking Colon pin.  If not used, SET TO -1                 ( old IN-1-Clock: white:2 brown: SDA)
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "Z568M Nixie Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER   
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
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION   
#endif

#ifdef CLOCK_8   //8266 GP PCB v3 clock with 4x IV-16 Numitron tubes (plexi box) //Flash size: 1MB (FS:160k, OTA:422k)
  //#define DEBUG
  #define FW "fw8"  //firmware name 
  #define MAXBRIGHTNESS 100  
  #define PCB_VERSION 3   //1,2 or 3
  #define LTBIpin 5
  byte digitEnablePins[] = {13,12,14,16};    //define here the digit enable pins from 4 to 8
  byte ABCDPins[4] = {4,0,2,15};
  #define USE_NEOPIXEL
  byte tubePixels[] = {3,2,6,1,0};    //Numitron 4 tubes, 4 x single leds + 1. The extra led in the middle is not used, is always dark!
  #define Numitron_4511N
  #define ALARMSPEAKER_PIN -1    //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN -1    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "Numitron Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION   
#endif

#ifdef CLOCK_9   //8266 GP PCB v1 clock with 4x IV-16 Numitron tubes (brown box) //Flash size: 1MB (FS:160k, OTA:422k)
  #define DEBUG
  #define FW "fw9"  //firmware name 
  #define MAXBRIGHTNESS 100
  #define PCB_VERSION 1   //1,2 or 3
  #define Numitron_4511N
  #define LTBIpin 5
  byte digitEnablePins[] = {13,12,14,16};    //define here the digit enable pins from 4 to 8
  byte ABCDPins[4] = {4,0,2,15};
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "Numitron Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION   
#endif

#ifdef CLOCK_10   //8266 LED Clock with MAX7219 chip
  #define DEBUG
  #define MAXBRIGHTNESS 10
  #define FW "fw10"  //firmware name 
  #define MAX7219CNG
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "LED UniClock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION   
#endif

#ifdef CLOCK_11   //8266 LED Clock with MM5450 chip
  #define DEBUG
  #define FW "fw11"  //firmware name 
  #define MAXBRIGHTNESS 100  
  //#define USE_DALLAS_TEMP
  //#define TEMP_DALLAS_PIN 2
  //#define RADAR_PIN 4
  #define RADAR_TIMEOUT 5  //min
  #define LIGHT_SENSOR_PIN A0  //Only ADC pins are usable! for example: 34,35,36,39... 
  #define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  #define MM5450
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "LED UniClock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION   
#endif

#ifdef CLOCK_12   //8266 VFD Clock with PT6355 chip
  #define DEBUG
  #define FW "fw12"  //firmware name 
  #define PT6355
  int maxDigits = 6;
  byte segmentEnablePins[] =  {16,7,0,1,2,3,4,5,6,8,9,11,10,12,13,14,15};    //segment enable bits 
  #define PIN_CS 22 //12 or 15     // D6 CS_
  #define PIN_CLK  16   // D7 Clock
  #define PIN_SIN  21   // D5 DataIN  
  #define PIN_SDA 19
  #define PIN_SCL 23
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 5
  #define ALARMSPEAKER_PIN 26   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 4  //32    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define RADAR_PIN -1
  #define RADAR_TIMEOUT 5  //5min
  #define AP_NAME "UNICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "VFD UniClock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION   
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
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION   
#endif

#ifdef CLOCK_14   //8266 6X LD8035 VFD-clock
  #define DEBUG 
  #define FW "fw14"  //firmware name
  #define MAXBRIGHTNESS 100  
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 4    //Dallas temp sensor pin.  If not used, SET TO -1
  #define MAX6921
  byte segmentEnablePins[] =  {7,9,2,4,8,5,6,3};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {14,15,19,18,17,16};   //digit enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  (You may define any number)
  //MAX6921 pins
    #define PIN_LE    0  // D3 Shift Register Latch Enable
    #define PIN_CLK   2  // D4 Shift Register Clock
    #define PIN_DATA  5  // D1 Shift Register Data
    #define PIN_BL    15  // D8 Shift Register Blank (1=display off     0=display on)
  //#define LIGHT_SENSOR_PIN A0  //Only ADC pins are usable!  
  //#define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  //#define LUX_CALC_SCALAR   12518931 * 1.2
  #define AP_NAME "UNFICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "LD 8035 VFD Clock"
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_15   //8266 6X IV-6 VFD-clock RGB
  #define DEBUG 
  #define FW "fw15"  //firmware name
  #define MAXBRIGHTNESS 100
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 3
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds  
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 4    //Dallas temp sensor pin.  If not used, SET TO -1
  #define MAX6921
  byte segmentEnablePins[] =  {7,9,2,4,8,5,6,3};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {14,15,19,18,17,16};   //digit enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  (You may define any number)
  //MAX6921 pins
  #define PIN_LE    0  // D3 Shift Register Latch Enable
  #define PIN_CLK   2  // D4 Shift Register Clock
  #define PIN_DATA  5  // D1 Shift Register Data
  #define PIN_BL    15  // D8 Shift Register Blank (1=display off     0=display on)
  #define ALARMSPEAKER_PIN 12   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN  13   //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define LIGHT_SENSOR_PIN A0  //Only ADC pins are usable!  
  //#define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  //#define LUX_CALC_SCALAR   12518931 * 1.2
  #define AP_NAME "UNFICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IV-6 VFD Clock"
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_15a   //8266 6X IV-6 VFD-clock RGB
  #define DEBUG 
  #define FW "fw15A"  //firmware name
  #define MAXBRIGHTNESS 100
  //#define USE_NEOPIXEL 
  //#define NEOPIXEL_PIN 3
  #define PIN_FLD_BUTTON 16
  #define PIN_SET_BUTTON 13
  #define USE_RTC
  #define PIN_SDA 2
  #define PIN_SCL 0
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds  
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 5    //Dallas temp sensor pin.  If not used, SET TO -1
  #define MAX6921
  byte segmentEnablePins[] =  {10,11,16,15,14,13,12,17};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {6,7,8,9,19,18};   //digit enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  (You may define any number)
  //MAX6921 pins
  #define PIN_LE    12  // D3 Shift Register Latch Enable
  #define PIN_CLK   14  // D4 Shift Register Clock
  #define PIN_DATA  15  // D1 Shift Register Data
  #define PIN_BL    4  // D8 Shift Register Blank (1=display off     0=display on)
  //#define LIGHT_SENSOR_PIN A0  //Only ADC pins are usable!  
  //#define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  //#define LUX_CALC_SCALAR   12518931 * 1.2
  #define AP_NAME "UNFICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IV-6 VFD Clock"
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_16   //8266 4X IV-6, 1X LD8035
  #define DEBUG 
  #define FW "fw16"  //firmware name
  #define MAXBRIGHTNESS 100  
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 4    //Dallas temp sensor pin.  If not used, SET TO -1
  #define MAX6921
  byte segmentEnablePins[] =  {7,9,2,4,8,5,6,3};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {19,18,17,16};         //digit enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  (You may define any number)
  //MAX6921 pins
    #define PIN_LE    0  // D3 Shift Register Latch Enable
    #define PIN_CLK   2  // D4 Shift Register Clock
    #define PIN_DATA  5  // D1 Shift Register Data
    #define PIN_BL    15  // D8 Shift Register Blank (1=display off     0=display on)
  //#define LIGHT_SENSOR_PIN A0  //Only ADC pins are usable!  
  //#define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  //#define LUX_CALC_SCALAR   12518931 * 1.2
  #define AP_NAME "UNFICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IV-6 VFD Clock"
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_17  //8266 + PT6311 4X IV-22, russian clock
  #define DEBUG 
  #define USE_RTC
  #define FW "fw17"  //firmware name
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 3
  byte tubePixels[] = {0,1,2,3};        //4 tubes, single leds
  #define MAXBRIGHTNESS 10  
  //#define USE_DALLAS_TEMP
  //#define TEMP_DALLAS_PIN 4    //Dallas temp sensor pin.  If not used, SET TO -1
  #define USE_RTC
  #define PT6311
  byte segmentEnablePins[] =  {4,3,1,7,6,5,2,0};    
  byte digitEnablePins[] = {0,1,2,3};         
  //PT6311 pins
  #define PIN_CLK  14   //  Clock
  #define PIN_DIN  13   //  DataIN
  #define PIN_LOAD 15   //  STB pin 
  //#define LIGHT_SENSOR_PIN A0  //Only ADC pins are usable!  
  //#define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  //#define LUX_CALC_SCALAR   12518931 * 1.2
  #define PIN_SDA  4              
  #define PIN_SCL  5             
  #define AP_NAME "UNFICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IV-22 VFD Clock"
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_18   //8266 IV-18 VFD-clock RGB
  #define DEBUG 
  #define FW "fw18"  //firmware name
  #define MAXBRIGHTNESS 100
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 3
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds  
  //#define USE_DALLAS_TEMP
  //#define TEMP_DALLAS_PIN -1   //Dallas temp sensor pin.  If not used, SET TO -1
  #define USE_DHT_TEMP 
  #define DHTTYPE DHT11
  #define TEMP_DHT_PIN  12 
  #define MAX6921
  byte segmentEnablePins[] =  {13,15,18,19,17,14,16,12};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {6,10,5,4,0,3,2,1};   //digit enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  (You may define any number)
  //MAX6921 pins
  #define PIN_LE    4  // D4 Shift Register Latch Enable
  #define PIN_CLK   5  // D1 Shift Register Clock
  #define PIN_DATA  2  // D4 Shift Register Data
  #define PIN_BL    14  // D5 Shift Register Blank (1=display off     0=display on)
  #define ALARMSPEAKER_PIN 16   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN  13   //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define LIGHT_SENSOR_PIN A0  //Only ADC pins are usable!  
  //#define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  //#define LUX_CALC_SCALAR   12518931 * 1.2
  #define AP_NAME "UNFICLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "IV-18 VFD Clock"
  //#define DISABLE_NIGHT_ANIMATION
#endif

//____________ P.S. clocks / thermometers ____________________________________________
#ifdef CLOCK_20   //8266 D1-mini, P.S. PCB 4xIN14 clock-thermometer 
  //#define DEBUG 
  #define FW "fw20"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 3 //RX
  #define MULTIPLEX74141
  byte digitEnablePins[] = {15,13,12,14};   
  byte ABCDPins[4] =  {2,4,5,0};
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
  #define DEFAULT_SSID ""
  #define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_21   //8266 D1-mini, P.S. PCB 3xIN14 1xIN-19A thermometer / humidity 
  //#define DEBUG
  #define FW "fw21"  //firmware name 
  #define MAXBRIGHTNESS 100    
  #define USE_DHT_TEMP
  #define DHTTYPE DHT22
  #define TEMP_DHT_PIN  3  //RX
  #define MULTIPLEX74141
  byte digitEnablePins[] = {15,13,12,14};  
  byte ABCDPins[4] =  {2,4,5,0};
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
  #define DEFAULT_SSID ""
  #define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION
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
  byte digitEnablePins[] = {15,13,12,14};  
  byte ABCDPins[4] =  {2,4,5,0};
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
  #define DEFAULT_SSID ""
  #define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_23   //ESP32 D1 mini, P.S. PCB 3xIN14 1xIN-19A thermometer / humidity
  #define DEBUG
  #define FW "fw23"  //firmware name
  #define MAXBRIGHTNESS 100   
  #define USE_NEOPIXEL
  #define NEOPIXEL_PIN 22
  byte tubePixels[] = {0,1,2,3};        //4 tubes, single leds
  #define USE_DHT_TEMP
  #define DHTTYPE DHT22
  #define TEMP_DHT_PIN  1
  #define MULTIPLEX74141_ESP32
  //byte digitEnablePins[] = {26,18,33,19};  //Old sw,old ESP32
  byte digitEnablePins[] = {19,33,18,26};
  byte ABCDPins[4] =  {32,16,4,17};
  #define DP_PIN -1             // decimalPoint inside Nixie tube, set -1, if not used!
  #define ENABLE_CLOCK_DISPLAY false  //don't display date/time!!!
  #define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define USE_SHT21                //I2C Temperature + humidity
  #define PIN_SDA  25              // you can set the used SDA and SCL pins
  #define PIN_SCL  21              // if it is not default value
  #define TEMP_CHARCODE 5
  #define GRAD_CHARCODE 10
  #define PERCENT_CHARCODE 6
  #define DECIMALPOINT_PIN  23  //Nixie decimal point between digits. If not used, SET TO -1
  #define DATE_REPEAT_MIN 0       //show date only every xxx minute. If zero, datum is never displayed
  #define TEMP_START  01
  #define TEMP_END    45
  #define HUMID_START 45
  #define HUMID_END   59
  #define AP_NAME "Nixie-Homero"
  #define AP_PASSWORD ""
  #define WEBNAME "Nixie Hőmérő"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  #define USE_WIFIMANAGER 
  #define USE_MQTT
  #define MQTT_PREFIX "Nixie_Homero"
  #define USE_MASTER_CLOCK  //enable it, if you want to get any data from MASTER CLOCK. This will be the sensor#0
  //#define USE_MASTER_TEMP   //enable it, if you want to get temperature from MASTER CLOCK
  //#define USE_MASTER_HUMID  //enable it, if you want to get humidity from MASTER CLOCK
  #define USE_MASTER_RADAR  //enable it, if you want to get radar from MASTER CLOCK
  //#define MASTER_TEMPERATURE_TOPIC "homeassistant/sensor/10521c5e14c4/temperature/state"
  //#define MASTER_HUMIDITY_TOPIC    "homeassistant/sensor/10521c5e14c4/humidity/state"
  #define MASTER_RADAR_TOPIC         "Nixie_IN-18_Clock/sensor/3c6105172188/radar/state"
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_23A   //ESP32 WROOM, U.Z. PCB 4xZ566M 1xIN-19A thermometer / humidity
  #define DEBUG
  #define FW "fw23A"  //firmware name
  #define MAXBRIGHTNESS 100
  byte tubePixels[] = {0,1,2,3};        //4 tubes, single leds
  #define USE_DHT_TEMP
  #define DHTTYPE DHT11
  #define TEMP_DHT_PIN  19
  #define MULTIPLEX74141_ESP32
  byte digitEnablePins[] = {18,13,12,27,5};  //C/%-tizedfok-fok1-fok10-előjel:18
  byte ABCDPins[4] =  {26,33,32,25};
  #define COLON_PIN 14             // decimalPoint inside Nixie tube, set -1, if not used!
  #define ENABLE_CLOCK_DISPLAY false  //don't display date/time!!!
  #define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define USE_SHT21                //I2C Temperature + humidity
  #define PIN_SDA  21              // you can set the used SDA and SCL pins
  #define PIN_SCL  22              // if it is not default value
  #define TEMP_CHARCODE 4
  #define GRAD_CHARCODE 10
  #define PERCENT_CHARCODE 7 
  #define PLUS_CHARCODE 0
  #define MINUS_CHARCODE 1 
  #define DATE_REPEAT_MIN 0       //show date only every xxx minute. If zero, datum is never displayed
  #define TEMP_START  01
  #define TEMP_END    45
  #define HUMID_START 45
  #define HUMID_END   59
  #define AP_NAME "Nixie-Homero"
  #define AP_PASSWORD ""
  #define WEBNAME "Nixie Hőmérő"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER 
  //#define USE_MQTT
  //#define MQTT_PREFIX "Nixie_Homero"
  //#define USE_MASTER_CLOCK  //enable it, if you want to get any data from MASTER CLOCK. This will be the sensor#0
  //#define USE_MASTER_TEMP   //enable it, if you want to get temperature from MASTER CLOCK
  //#define USE_MASTER_HUMID  //enable it, if you want to get humidity from MASTER CLOCK
  //#define USE_MASTER_RADAR  //enable it, if you want to get radar from MASTER CLOCK
  //#define MASTER_TEMPERATURE_TOPIC "homeassistant/sensor/10521c5e14c4/temperature/state"
  //#define MASTER_HUMIDITY_TOPIC    "homeassistant/sensor/10521c5e14c4/humidity/state"
  //#define MASTER_RADAR_TOPIC         "Nixie_IN-18_Clock/sensor/3c6105172188/radar/state"
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_23B   //ESP32 WROOM U.Z. PCB Z571M+4xZ566M+IN-19A clock/thermometer/humidity
  #define DEBUG
  #define FW "fw23B"  //firmware name
  #define THERMOMETER_CLOCK
  #define MAXBRIGHTNESS 100  
  #define USE_DHT_TEMP
  #define DHTTYPE DHT11
  #define TEMP_DHT_PIN 19
  #define MULTIPLEX74141_ESP32
  byte digitEnablePins[] = {13,17,12,27,5,18};  //előjel(18(-fok10-fok1-tizedfok-századfok-C/%(13)
  byte ABCDPins[4] =  {26,33,32,25};
  #define COLON_PIN 14             // decimalPoint inside Nixie tube, set -1, if not used!
  #define ENABLE_CLOCK_DISPLAY  false  //don't display date/time!!!
  #define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define USE_SHT21                //I2C Temperature + humidity
  #define PIN_SDA  21              // you can set the used SDA and SCL pins
  #define PIN_SCL  22              // if it is not default value
  #define TEMP_CHARCODE 4  
  #define PERCENT_CHARCODE 7 
  #define PLUS_CHARCODE 0
  #define MINUS_CHARCODE 1 
  #define DATE_REPEAT_MIN_1       //show date only every xxx minute. If zero, datum is never displayed
  #define TEMP_START  40
  #define TEMP_END    45
  #define HUMID_START 45
  #define HUMID_END   50
  #define AP_NAME "Nixie-Ora-Homero"
  #define AP_PASSWORD ""
  #define WEBNAME "Nixie Óra-Hőmérő-Páramérő" 
#endif

#ifdef CLOCK_26   //ESP32 D1 mini, UNFI 6 x IV-11 VFD tubes clock, SHT21
  #define DEBUG 
  #define FW "fw26"  //firmware name 
  #define MAXBRIGHTNESS 100 
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 22
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 25    //Dallas temp sensor pin.  If not used, SET TO -1    
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 25    //DHT temp sensor pin.  If not used, SET TO -1     
  #define USE_BME280            //I2C Temperature + humidity + pressure
  #define USE_BMP280            //I2C Temperature + barometric  pressure
  #define USE_AHTX0             //I2C Temperature + humidity
  #define USE_SHT21             //I2C Temperature + humidity
  #define PIN_SDA 26           // you can set the used SDA and SCL pins
  #define PIN_SCL 27           // if it is not default value
  //#define USE_MQTT
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
  #define DEFAULT_SSID "***NET***"
  #define DEFAULT_PSW "Szentatya5"
  //#define USE_WIFIMANAGER  
   #define DISABLE_NIGHT_ANIMATION
  //#define DISABLE_BROWNOUT
   #define USE_MQTT
  //#define MQTT_PREFIX "VFD-Clock"
  #define USE_MASTER_CLOCK  //enable it, if you want to get any data from MASTER CLOCK. This will be the sensor#0
  //#define USE_MASTER_TEMP   //enable it, if you want to get temperature from MASTER CLOCK
  //#define USE_MASTER_HUMID  //enable it, if you want to get humidity from MASTER CLOCK
  #define USE_MASTER_RADAR  //enable it, if you want to get radar from MASTER CLOCK
  //#define MASTER_TEMPERATURE_TOPIC "homeassistant/sensor/10521c5e14c4/temperature/state"
  //#define MASTER_HUMIDITY_TOPIC    "homeassistant/sensor/10521c5e14c4/humidity/state"
  #define MASTER_RADAR_TOPIC         "Nixie_IN-18_Clock/sensor/10521c5e14c4/radar/state"
  
#endif

//______________________ESP-32 CLOCKS  (2x18pin ESP32 modul) ______________________________________________________

#ifdef CLOCK_30   //ESP32, UNFI PCB clock, 6 x IV-11 VFD tubes
  #define DEBUG
  #define FW "fw30"  //firmware name 
  #define MAXBRIGHTNESS 100
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 2
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
  #define RADAR_TIMEOUT 5  //5min
  //#define TUBE_POWER_PIN 25
  //#define TUBE_POWER_ON  LOW
  #define LIGHT_SENSOR_PIN 35  //Only ADC pins are usable! for example: 34,35,36,39... 
  #define MAXIMUM_LUX 100    //Lux level for maximum tube brightness
  #define LUX_CALC_SCALAR   12518931 * 1.2 
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""  
  #define WEBNAME "GP IV-11"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
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
  byte digitEnablePins[] = {4,16,17,5,18,19};   //ESP32 6x tube Clock
  byte ABCDPins[4] =  {12,27,14,13};   
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
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif


#ifdef CLOCK_32   //ESP32, UNFI 6 x IV-11 VFD tubes clock, DHT22 sensor
  #define DEBUG
  #define FW "fw32"  //firmware name 
  #define MAXBRIGHTNESS 100  
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
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_33   //TOM025 ESP32, Palfi S. board, 6 x Z573M Nixie tubes
  #define DEBUG 
  #define FW "fw33"
  #define MAXBRIGHTNESS 100
  //#define USE_NEOPIXEL 
  //#define NEOPIXEL_PIN 2
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  //#define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 23    //Dallas temp sensor pin.  If not used, SET TO -1    
  #define MULTIPLEX74141_ESP32
  byte digitEnablePins[] = {22,23,21,19,12,14};   //ESP32 6x tube Clock
  byte ABCDPins[4] =  {18,5,16,17};   
  #define DP_PIN 15             // decimalPoint inside Nixie tube, set -1, if not used!
  #define LEFTDECIMAL false   //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  //#define TEMP_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define GRAD_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define ALARMSPEAKER_PIN 33   //Alarm buzzer pin                                            
  //#define ALARMBUTTON_PIN 32    //Alarm switch off button pin 
  //#define ALARM_ON HIGH
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""
  #define WEBNAME "ESP32UniClock 3.0"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_34   //Mule V2 ESP32, board, 6 x SA40 LED Display
  #define DEBUG 
  #define FW "fw34"
  #define MAXBRIGHTNESS 100
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 26
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  //#define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 25    //Dallas temp sensor pin.  If not used, SET TO -1    
  #define MULE_V2
  byte digitEnablePins[] = {5,16,23};   //CD4028 A,B,C
  byte ABCDPins[4] =  {33,32,14,13};    //CD4543 A,B,C,D
  //#define DP_PIN 15             // decimalPoint inside Nixie tube, set -1, if not used!
  //#define LEFTDECIMAL false   //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  //#define TEMP_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define GRAD_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define ALARMSPEAKER_PIN 22   //Alarm buzzer pin                                            
  //#define ALARMBUTTON_PIN 32    //Alarm switch off button pin 
  //#define ALARM_ON HIGH
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""
  #define WEBNAME "ESP32UniClock 3.0"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_35   //ESP32, 6X Z566M and 3x 74595 6x74141  driver  (NON-MULTIPLEX)
  #define DEBUG
  #define SIMULATE_TEMPERATURE_SENSOR
  #define FW "fw35"  //firmware name
  #define MAXBRIGHTNESS 100
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 32
  #define USE_NEOPIXEL
  #define NEOPIXEL_PIN 25
  //#define USE_DHT_TEMP
  #define TEMP_DHT_PIN 32
  byte tubes[] = {5,4,3,2,1,0};   //6 tubes, reverse order 
  byte tubePixels[] = {5,4,3,2,1,0}; 
  #define dataPin  14  //D5
  #define latchPin 27  //D6
  #define clkPin   26  //D7
  #define NO_MULTIPLEX_ESP32 
  #define COLON_PIN  13         //Blinking Colon pin.  If not used, SET TO -1
  #define ALARMSPEAKER_PIN 12   //Alarm buzzer pin                                            
  //#define ALARMBUTTON_PIN 36   //Alarm switch off button pin
  #define ALARM_ON HIGH
  #define LIGHT_SENSOR_PIN 35
  #define MAXIMUM_LUX 100
  #define LUX_CALC_SCALAR   12518931 * 1.2   //Calibrate here 
  //#define RADAR_PIN 15
  //#define RADAR_TIMEOUT  5 //min
  #define USE_RTC
  #define PIN_SDA 21
  #define PIN_SCL 22
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD ""
  #define WEBNAME "Z566M Nixie Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION   
  #define DISPLAYTEMP_ONLY_2DIGITS
  #define DISPLAYHUMID_ONLY_2DIGITS
  #define GRAD_CHARCODE 10   //no display grad char
  #define TEMP_CHARCODE 10   //no display temp char
  #define PERCENT_CHARCODE 10 //no % tube
  #define TEMP_START_POSITION 3
  #define HUMID_START_POSITION 3
#endif  

#ifdef CLOCK_36   //ESP32, 4x IN-1 tubes and  4x74141  driver  (NON-MULTIPLEX)
  #define DEBUG
  #define FW "fw35"  //firmware name
  #define MAXBRIGHTNESS 100
  //#define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN -1
  #define USE_NEOPIXEL
  #define dataPin  14  //D5
  #define latchPin 27  //D6
  #define clkPin   26  //D7  
  byte tubes[] = {3,2,1,0};         //4 tubes,   old clock...     
  byte tubePixels[] = {0,0,1,1,2,2,3,3,3,3,2,2,1,1,0,0,0};  //4 tubes, double row, 17 leds (GP)  
  #define NO_MULTIPLEX_ESP32
  #define COLON_PIN   2        //Blinking Colon pin.  If not used, SET TO -1                 ( old IN-1-Clock: white:2 brown: SDA)
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD ""
  #define WEBNAME "IN-1 Nixie Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION   
#endif

//______________________ESP-32 CLOCKS  (2x20pin ESP32 D1 mini modul) ______________________________________________________
#ifdef CLOCK_40   //V1  ESP32, UNFI 6 x IV-11 VFD tubes clock
  #define DEBUG 
  #define FW "fw40"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 22
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 25    //Dallas temp sensor pin.  If not used, SET TO -1    
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 25    //DHT temp sensor pin.  If not used, SET TO -1     
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
  #define TUBE_POWER_PIN 23
  #define TUBE_POWER_ON  HIGH 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD ""  
  #define WEBNAME "ESP32 IV-11 VFD-Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_41   //V2  ESP32, UNFI 6 x IV-11 VFD tubes clock (jumper wire)
  #define DEBUG 
  #define FW "fw41"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 32
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 4    //Dallas temp sensor pin.  If not used, SET TO -1    
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 4    //DHT temp sensor pin.  If not used, SET TO -1     
  #define MAX6921_ESP32
  byte segmentEnablePins[] =  {19,17,15,12,13,16,18,14};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {9,8,7,2,1,0};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6)  (You may define any number)
  #define DOUBLE_BLINK  //both separator points are blinking 
  //MAX6921 pins
    #define PIN_LE    22  // Shift Register Latch Enable
    #define PIN_CLK   17  // Shift Register Clock
    #define PIN_DATA  16  // Shift Register Data
    #define PIN_BL    21  // Shift Register Blank (1=display off     0=display on)
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 0    //Alarm switch off button pin
  #define TUBE_POWER_PIN 23
  #define TUBE_POWER_ON  HIGH 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD ""  
  #define WEBNAME "ESP32 IV-11 VFD-Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_42   //V3  ESP32, UNFI 6 x IV-11 VFD tubes clock
  #define DEBUG 
  #define FW "fw42"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 25
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 22    //Dallas temp sensor pin.  If not used, SET TO -1    
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 22    //DHT temp sensor pin.  If not used, SET TO -1     
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
  #define TUBE_POWER_PIN 27
  #define TUBE_POWER_ON  HIGH 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD ""  
  #define WEBNAME "ESP32 IV-11 VFD-Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif


#ifdef CLOCK_43   //V1  ESP32, UNFI board, 6 x Z573M Nixie tubes
  #define DEBUG 
  //#define SIMULATE_TEMPERATURE_SENSOR
  #define FW "fw43"  //firmware name
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
  byte DRAM_ATTR digitEnablePins[] = {26,18,33,19,23,5};   //ESP32 6x tube Clock
  byte DRAM_ATTR ABCDPins[4] =  {32,16,4,17};   
  #define DP_PIN 27             // decimalPoint inside Nixie tube, set -1, if not used!
  #define LEFTDECIMAL false   //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  //#define TEMP_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define GRAD_CHARCODE -1   //disable char => shift display right with 1 digit
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 0    //Alarm switch off button pin 
  #define ALARM_ON HIGH
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD ""
  #define WEBNAME "Z573M Nixie-Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_44   //V2  ESP32, UNFI board, 6 x Z573M Nixie tubes (átkötéses)
  #define DEBUG 
  #define FW "fw44"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 32
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 4
  #define TEMP_DALLAS_PIN 4    //Dallas temp sensor pin.  If not used, SET TO -1    
  #define MULTIPLEX74141_ESP32
  byte digitEnablePins[] = {26,18,33,19,5,25};   //ESP32 6x tube Clock
  byte ABCDPins[4] =  {21,16,22,17};   
  #define DP_PIN 27             // decimalPoint inside Nixie tube, set -1, if not used!
  #define LEFTDECIMAL false   //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  //#define TEMP_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define GRAD_CHARCODE -1   //disable char => shift display right with 1 digit
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 0    //Alarm switch off button pin 
  #define ALARM_ON HIGH
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD ""
  #define WEBNAME "ESP32 Z573M Nixie-Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_45   //V3  ESP32, UNFI board, 6 x Z573M Nixie tubes
  #define DEBUG 
  #define FW "fw45"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 25
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 22
  #define TEMP_DALLAS_PIN 22    //Dallas temp sensor pin.  If not used, SET TO -1    
  #define MULTIPLEX74141_ESP32
  byte digitEnablePins[] = {26,18,33,19,23,5};   //ESP32 6x tube Clock
  byte ABCDPins[4] =  {32,16,4,17};   
  #define DP_PIN 27             // decimalPoint inside Nixie tube, set -1, if not used!
  #define LEFTDECIMAL false   //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  //#define TEMP_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define GRAD_CHARCODE -1   //disable char => shift display right with 1 digit
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 0    //Alarm switch off button pin 
  #define ALARM_ON HIGH
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD ""
  #define WEBNAME "ESP32 Z573M Nixie-Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_46   //ESP32, UNFI board, 6 x IV-9 Numitron tubes
  #define DEBUG 
  #define ANIMSPEED 70
  #define FW "fw46"  //firmware name
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
  #define USE_BME280            //I2C Temperature + humidity + pressure
  #define USE_BMP280            //I2C Temperature + barometric  pressure
  #define USE_AHTX0             //I2C Temperature + humidity
  #define USE_SHT21             //I2C Temperature + humidity
  #define Numitron_4511N
  #define PCB_VERSION 32
  #define LTBIpin 23
  byte digitEnablePins[] = {4,16,17,32,21,25};   //ESP32 6x tube Clock
  byte ABCDPins[4] =  {19,18,26,33};   
  #define DP_PIN -1             // decimalPoint inside Nixie tube, set -1, if not used!
  #define LEFTDECIMAL false   //set true (Z574M), if decimal point is on the left side on the tube. Else set false (Z573M)!
  //#define TEMP_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define GRAD_CHARCODE -1   //disable char => shift display right with 1 digit
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  //#define ALARMBUTTON_PIN 34    //Alarm switch off button pin 
  #define ALARM_ON HIGH
  #define AP_NAME "NUMITRON6_CLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "ESP32 Numitron-Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_47   //V3  ESP32, UNFI 10-LT-50G VFD tubes clock
  #define DEBUG 
  #define FW "fw47"  //firmware name
  #define MAXBRIGHTNESS 100  
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 22    //Dallas temp sensor pin.  If not used, SET TO -1    
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 22    //DHT temp sensor pin.  If not used, SET TO -1     
  #define MAX6921_ESP32
  byte segmentEnablePins[] =  {12,13,16,18,17,14,15,19};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {10,9,8,7,2,1,0,11};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  (You may define any number)
  #define DOUBLE_BLINK  //both separator points are blinking 
  //MAX6921 pins
  #define PIN_LE    4  // Shift Register Latch Enable
  #define PIN_CLK   17  // Shift Register Clock
  #define PIN_DATA  16  // Shift Register Data
  #define PIN_BL    32  // Shift Register Blank (1=display off     0=display on)
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 0    //Alarm switch off button pin
  #define TUBE_POWER_PIN 27
  #define TUBE_POWER_ON  HIGH 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD ""  
  #define WEBNAME "ESP32 10-LT-50G VFD-Clock"
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_48   //WROOM ESP32, UNFI  SN75518 6xIV-17 clock
  #define DEBUG 
  #define FW "fw48"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 26
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 23    //Dallas temp sensor pin.  If not used, SET TO -1    
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 23    //DHT temp sensor pin.  If not used, SET TO -1
  //#define LIGHT_SENSOR_PIN 32
  #define LOWER_CIRCLE_CHARCODE 10
  #define UPPER_CIRCLE_CHARCODE 17
  #define PIN_SDA 21             // you can set the used SDA and SCL pins
  #define PIN_SCL 22             // if it is not default value   
  #define USE_BH1750            //I2C luxmeter sensor
  #define USE_BME280            //I2C Temperature + humidity + pressure
  #define USE_BMP280            //I2C Temperature + barometric  pressure
  #define USE_AHTX0             //I2C Temperature + humidity
  #define USE_SHT21             //I2C Temperature + humidity  
  #define SN75518_ESP32
  //Segments:  Dutsrpnmkhgfedcba   
  byte segmentEnablePins[] =  {23,30,18,19,20,26,27,28,29,32,15,17,21,22,24,25,31};   //segment enable OUTbits of SN75518 (2,3,4,5,6,7,8,9,10,13,14,15,16,17,18,19,20,21)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] =    {6,5,4,3,2,1};  //digit enable OUTbits of SN75518 (1,2,3,4,5,6)  (You may define any number)
  #define DOUBLE_BLINK  //both separator points are blinking 
  //SN75518 pins
    #define PIN_LE    13  // Shift Register Latch Enable
    #define PIN_CLK   12  // Shift Register Clock
    #define PIN_DATA  27  // Shift Register Data
    #define PIN_STB   14  // Shift Register Strobe
  #define ALARMSPEAKER_PIN 19   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 33    //Alarm switch off button pin
  //#define TUBE_POWER_PIN 
  #define TUBE_POWER_ON  HIGH 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD ""  
  #define WEBNAME "ESP32 IV-17 VFD-Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER  
  //#define DISABLE_BROWNOUT
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_49   //WROOM ESP32, UNFI  SN75518 6xIV-11 clock
  #define DEBUG 
  #define FW "fw49"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 26
  byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 23    //Dallas temp sensor pin.  If not used, SET TO -1    
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 23    //DHT temp sensor pin.  If not used, SET TO -1
  //#define LIGHT_SENSOR_PIN 32
  #define PIN_SDA 21             // you can set the used SDA and SCL pins
  #define PIN_SCL 22             // if it is not default value     
  #define SN75518_ESP32
  //Segments:  abcdefgD  
  #define SEGMENT8 
  byte segmentEnablePins[] =  {17,18,19,20,21,22,23,24};   //IV-11 segment enable OUTbits of SN75518   (You MUST define  8 Pins!!!)
  byte digitEnablePins[] =    {27,28,29,30,31,32};         //IV-11 digit enable OUTbits of SN75518 (1,2,3,4,5,6)  (You may define any number)
  //byte segmentEnablePins[] =  {17,18,19,20,21,22,23,24};   //IV-12 segment enable OUTbits of SN75518   (You MUST define  8 Pins!!!)
  //byte digitEnablePins[] =    {27,28,29,30,31,32};        //IV-12 digit enable OUTbits of SN75518 (1,2,3,4,5,6)  (You may define any number)
  #define DOUBLE_BLINK  //both separator points are blinking 
  //SN75518 pins
    #define PIN_LE    13  // Shift Register Latch Enable
    #define PIN_CLK   12  // Shift Register Clock
    #define PIN_DATA  27  // Shift Register Data
    #define PIN_STB   14  // Shift Register Strobe
  #define ALARMSPEAKER_PIN 19   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 33    //Alarm switch off button pin
  //#define TUBE_POWER_PIN 
  #define TUBE_POWER_ON  HIGH 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD ""  
  #define WEBNAME "ESP32 IV-11 VFD-Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER  
  //#define DISABLE_BROWNOUT
  //#define DISABLE_NIGHT_ANIMATION
#endif

//---------------------ESP32   2X HV5122--------------------------------------------------

#ifdef CLOCK_50   //V1  ESP32,UNFI 2xHV5122 PCB version, 6xZ573M clock   Example clock!!!
  #define DEBUG 
  #define FW "fw50"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 27
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  //#define USE_DALLAS_TEMP
  //#define TEMP_DALLAS_PIN 26    //Dallas temp sensor pin.  If not used, SET TO -1   
  //#define LIGHT_SENSOR_PIN 23
  #define PIN_SDA 4             // you can set the used SDA and SCL pins
  #define PIN_SCL 32             // if it is not default value
  #define USE_DHT_TEMP
  #define DHTTYPE DHT22
  #define TEMP_DHT_PIN  26
  //#define USE_RTC
  //#define USE_GPS
  #define USE_BME280            //I2C Temperature + humidity + pressure
  #define USE_BMP280            //I2C Temperature + barometric  pressure
  #define USE_AHTX0             //I2C Temperature + humidity
  #define USE_SHT21             //I2C Temperature + humidity
  #define USE_BH1750            //I2C luxmeter sensor
//_______________________________ HV5122 setup ____________________________________________________  
  #define NEWHV5122
  #define PIN_DIN  22   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  17   // Clock
  #define PIN_OE   21   // OutputEnable
  #define MAXDIGITS 6
  int maxDigits = MAXDIGITS;
  byte digitPins[MAXDIGITS+1][10] = {            ////Data pin numbers 100+ means: chip1 pins are used. Chip0-s DOUT is connected to chip1's DIN
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
  //#define ALARMBUTTON_PIN 23    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define RADAR_PIN 4
  #define RADAR_TIMEOUT  5 //min
  //#define TUBE_POWER_PIN 4
  #define TUBE_POWER_ON  HIGH
  #define ENABLE_CLOCK_DISPLAY true  
  #define DATE_REPEAT_MIN 3       //show date only every xxx minute. If zero, datum is never displayed
  //#define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 10
  #define GRAD_CHARCODE 10 
  #define PERCENT_CHARCODE 10
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD "" 
  #define WEBNAME "UNFI Nixie-Clock HV5122"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_51   //V2  ESP32, UNFI 2xHV5122 PCB version, 6xZ573M clock   Example clock!!!
  #define DEBUG 
  #define FW "fw51"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 27
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 19    //Dallas temp sensor pin.  If not used, SET TO -1   
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
  //#define USE_BH1750            //I2C luxmeter sensor
  //USE_MQTT
  //#define MQTT_PREFIX "UniClock32"
  //#define USE_MASTER_CLOCK  //enable it, if you want to get any data from MASTER CLOCK. This will be the sensor#0
  //#define USE_MASTER_TEMP   //enable it, if you want to get temperature from MASTER CLOCK
  //#define USE_MASTER_HUMID  //enable it, if you want to get humidity from MASTER CLOCK
  //#define USE_MASTER_RADAR  //enable it, if you want to get radar from MASTER CLOCK
  //#define MASTER_TEMPERATURE_TOPIC "homeassistant/sensor/10521c5e14c4/temperature/state"
  //#define MASTER_HUMIDITY_TOPIC    "homeassistant/sensor/10521c5e14c4/humidity/state"
  //#define MASTER_RADAR_TOPIC       "homeassistant/sensor/10521c5e14c4/radar/state"  
//_______________________________ HV5122 setup ____________________________________________________  
  #define NEWHV5122
  #define PIN_DIN  22   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  17   // Clock
  #define PIN_OE   21   // OutputEnable
  #define MAXDIGITS 6
  int maxDigits = MAXDIGITS;
  byte digitPins[MAXDIGITS+1][10] = {
    {2,10,9,8,7,6,5,4,3,1},                      //sec  1 , chip0  (tube#0)
    {11,32,20,19,18,17,16,15,14,13},             //sec  10 , chip0 (tube#1)
    {22,31,29,30,27,28,25,26,23,24},             //min   1 , chip0 (tube#2)
    {101,131,110,109,108,107,106,105,104,103},   //min  10 , chip1 (tube#3)
    {111,132,120,119,118,117,116,115,114,113},   //hour  1 , chip1 (tube#4)
    {122,129,130,127,128,125,126,123,124,121},   //hour 10 , chip1 (tube#5)
    {0,21,12,112,102,0,    0,0,0,0}              //extra decimalPoint/blinking dots (tube0...tube6)
    };    
   //#define MAKE_BLINKING_DOTS //it means, the extra datapins are used as 4 blinking dot instead of decimal points!  #1..#4 positions are used
//___________________________________________________________________________________    
  #define ALARMSPEAKER_PIN 23   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 16    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define RADAR_PIN 4
  #define RADAR_TIMEOUT 5  //min
  //#define TUBE_POWER_PIN 4
  #define TUBE_POWER_ON  HIGH
  #define ENABLE_CLOCK_DISPLAY true  
  #define DATE_REPEAT_MIN 1       //show date only every xxx minute. If zero, datum is never displayed
  //#define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 10
  #define GRAD_CHARCODE 10 
  #define PERCENT_CHARCODE 10
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD "" 
  #define WEBNAME "Nixie-Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_52   //ESP32, P.S. 2xHV5122 PCB version, 6xIN18 clock   PROTOTYPE TESTING!!!
  #define DEBUG 
  #define FW "fw52"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 22
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  //#define USE_DALLAS_TEMP
  //#define TEMP_DALLAS_PIN 26    //Dallas temp sensor pin.  If not used, SET TO -1   
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT11
  //#define TEMP_DHT_PIN  23
  //#define USE_BME280            //I2C Temperature + humidity + pressure
  //#define USE_BMP280            //I2C Temperature + barometric  pressure
  //#define USE_AHTX0             //I2C Temperature + humidity
  //#define USE_SHT21               //I2C Temperature + humidity
  //#define PIN_SDA  4              // you can set the used SDA and SCL pins
  //#define PIN_SCL 32              // if it is not default value
  #define PIN_EXTINP1  4           //extra input pin (not used)
  #define PIN_EXTINP2  32          //extra input pin (not used)
  #define USE_RTC    
  #define PIN_SDA 27               //D1   SDA/SCL swapped
  #define PIN_SCL 26               //D2
//_______________________________ HV5122 setup ____________________________________________________    
  #define NEWHV5122
  #define PIN_DIN  16   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  17   // Clock
  #define PIN_OE   21   // OutputEnable
  #define MAXDIGITS 6
  int maxDigits = MAXDIGITS;
  byte digitPins[MAXDIGITS+1][10] = {            ////Data pin numbers 100+ means: chip1 pins are used. Chip0-s DOUT is connected to chip1's DIN
    {126,127,128,130,124,123,132,131,129,125},            //hour 10 , CHIP0
    {116,117,118,120,114,113,122,121,119,115},            //hour 1 , CHIP0
    {104,105,106,108,102,101,110,109,107,103},            //min 10 , CHIP0
    {26,27,28,30,24,23,32,31,29,25},                      //min 1 , CHIP1
    {16,17,18,20,14,13,22,21,19,15},                      //secound 10 , CHIP1
    {4,5,6,8,2,1,10,9,7,3},                               //secound 1 , CHIP1
    {0,111,112,11,12,0,   0,0,0,0}                        //extra GL dots  //GL1,GL2,GL3,GL4
    };    
    #define MAKE_BLINKING_DOTS //it means, the extra datapins are used as 4 blinking dot instead of decimal points!  #1..#4 positions are used, #0. and  #5..#9. positions are zero!
//_______________________________________________________________________________________    
  #define ALARMSPEAKER_PIN 23   //Alarm buzzer pin                                            
  //#define ALARMBUTTON_PIN 23    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define LIGHT_SENSOR_PIN 34
  #define RADAR_PIN 19
  #define RADAR_TIMEOUT 5  //min
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
  #define AP_NAME "IN-18-Clock"
  #define AP_PASSWORD "" 
  #define WEBNAME "Nixie IN-18 Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_53   //ESP8266 Wemos D1 mini,UNFI 2xHV5122 PCB version, 6xIN18 clock  
  #define DEBUG 
  #define FW "fw53"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 3
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 12    //Dallas temp sensor pin.  If not used, SET TO -1   
  //#define LIGHT_SENSOR_PIN 23
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT11
  //#define TEMP_DHT_PIN  23

//_______________________________ HV5122 setup ____________________________________________________    
  #define NEWHV5122
  #define PIN_DIN  5   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  0   // Clock
  #define PIN_OE   4   // OutputEnable
  #define MAXDIGITS 6
  int maxDigits = MAXDIGITS;
  byte digitPins[MAXDIGITS+1][10] = {            ////Data pin numbers 100+ means: chip1 pins are used. Chip0-s DOUT is connected to chip1's DIN
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
  #define RADAR_TIMEOUT 5  //min
  //#define TUBE_POWER_PIN 4
  #define TUBE_POWER_ON  LOW
  #define ENABLE_CLOCK_DISPLAY true  
  #define DATE_REPEAT_MIN 1       //show date only every xxx minute. If zero, datum is never displayed
  //#define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 10
  #define GRAD_CHARCODE 10 
  #define PERCENT_CHARCODE 10
  #define AP_NAME "UNFICLOCK"
  #define AP_PASSWORD "" 
  #define WEBNAME "UNFI Nixie Clock HV5122"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_54   // D1 R32 ESP32 (Uno compatible), + NCS312 Nixie clock KIT!!!
  #define DEBUG
  #define FW "fw54"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define DOUBLE_BLINK 
  //#define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 22
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 14    //Dallas temp sensor pin.  If not used, SET TO -1   
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
  #define NEWHV5122
  #define PIN_DIN  23   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  18   // Clock
  #define PIN_OE   5  // OutputEnable
  #define MAXDIGITS 6
  int maxDigits = MAXDIGITS;
  byte digitPins[MAXDIGITS+1][10] = {            ////Data pin numbers 100+ means: chip1 pins are used. Chip0-s DOUT is connected to chip1's DIN
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
  #define PWM1_PIN 27  //Uno: D6
  #define PWM2_PIN 13  //Uno: D9
  #define PWM3_PIN 25  //Uno: D3
  //#define ALARMSPEAKER_PIN 26   //Alarm buzzer pin                                            
  //#define ALARMBUTTON_PIN 2    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define RADAR_PIN 4
  //#define RADAR_TIMEOUT 5  //min
  //#define TUBE_POWER_PIN 16
  #define TUBE_POWER_ON  LOW
  #define ENABLE_CLOCK_DISPLAY true  
  #define TEMP_CHARCODE 10
  #define GRAD_CHARCODE 10 
  #define PERCENT_CHARCODE 10
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD "" 
  #define WEBNAME "NCS312 Nixie Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_55   //ESP8266 D1 Mini,UNFI 2xHV5122 PCB version, 6xZ573M clock
  #define DEBUG 
  #define FW "fw55"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 3
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 12    //Dallas temp sensor pin.  If not used, SET TO -1   
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
  //#define USE_BH1750            //I2C luxmeter sensor
//_______________________________ HV5122 setup ____________________________________________________  
  #define NEWHV5122
  #define PIN_DIN  5   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  0   // Clock
  #define PIN_OE   4   // OutputEnable
  #define MAXDIGITS 6
  int maxDigits = MAXDIGITS;
  byte digitPins[MAXDIGITS+1][10] = {            ////Data pin numbers 100+ means: chip1 pins are used. Chip0-s DOUT is connected to chip1's DIN
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
  #define ALARMSPEAKER_PIN 2   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 13    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define RADAR_PIN 4
  #define RADAR_TIMEOUT 5  //5min
  //#define TUBE_POWER_PIN 4
  #define TUBE_POWER_ON  LOW
  #define ENABLE_CLOCK_DISPLAY true  
  //#define DATE_REPEAT_MIN 1       //show date only every xxx minute. If zero, datum is never displayed
  //#define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 10
  #define GRAD_CHARCODE 10 
  #define PERCENT_CHARCODE 10
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD "" 
  #define WEBNAME "UNFI Nixie Clock HV5122"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_56   //WROOM ESP32, UNFI 2xHV5122 PCB version, 6xZ573M clock   Example clock!!!
  #define DEBUG 
  #define FW "fw56"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 26
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 23   //Dallas temp sensor pin.  If not used, SET TO -1   
  //#define LIGHT_SENSOR_PIN 18
  #define PIN_SDA 21             // you can set the used SDA and SCL pins
  #define PIN_SCL 22             // if it is not default value
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT11
  //#define TEMP_DHT_PIN  23
  //#define USE_BME280            //I2C Temperature + humidity + pressure
  //#define USE_BMP280            //I2C Temperature + barometric  pressure
  //#define USE_AHTX0             //I2C Temperature + humidity
  //#define USE_SHT21             //I2C Temperature + humidity
  //#define USE_BH1750            //I2C luxmeter sensor
//_______________________________ HV5122 setup ____________________________________________________  
  #define NEWHV5122
  #define PIN_DIN  14   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  12   // Clock
  #define PIN_OE   27   // OutputEnable
  #define MAXDIGITS 6
  int maxDigits = MAXDIGITS;
  byte digitPins[MAXDIGITS+1][10] = {   
    {129,128,127,126,125,124,123,132,131,130},  //sec   1 , chip1 (tube#0)
    {118,117,116,115,114,113,112,122,121,119},  //sec  10 , chip1 (tube#1)
    {107,106,105,104,103,102,101,111,110,108},  //min   1 , chip1 (tube#2)    
    {28,27,26,25,24,23,22,32,31,29},            //min  10 , chip0  (tube#3)
    {17,16,15,14,13,12,11,21,20,18},            //hour 10 , chip0 (tube#4)
    {7,6,5,4,3,2,1,10,9,8},                     //hour   1 , chip0 (tube#5)    
    {0,0,109,30,19,0,    0,0,0,0}              //extra decimalPoint/blinking dots (tube0...tube6)
    };    
   //#define MAKE_BLINKING_DOTS //it means, the extra datapins are used as 4 blinking dot instead of decimal points!  #1..#4 positions are used
//___________________________________________________________________________________    
  #define ALARMSPEAKER_PIN 32   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 33    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define RADAR_PIN 4
  #define RADAR_TIMEOUT 5  //min
  //#define TUBE_POWER_PIN 4
  #define TUBE_POWER_ON  HIGH
  #define ENABLE_CLOCK_DISPLAY true  
  #define DATE_REPEAT_MIN 1       //show date only every xxx minute. If zero, datum is never displayed
  //#define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 10
  #define GRAD_CHARCODE 10 
  #define PERCENT_CHARCODE 10
  #define AP_NAME "UNICLOCK32"
  #define AP_PASSWORD "uniclock" 
  #define WEBNAME "UNFI Nixie-Clock HV5122"
  #define DEFAULT_SSID "Unferdorben_2"
  #define DEFAULT_PSW "4073404540734"
  #define USE_WIFIMANAGER  
  #define DISABLE_NIGHT_ANIMATION   //in night mode, no animation allowed
#endif

#ifdef CLOCK_57   //WROOM ESP32, UNFI 2xHV5122 PCB version, 6xZM1000 clock PLCC44 socket!
  #define DEBUG 
  #define FW "fw57"  //firmware name
  #define MAXBRIGHTNESS 100
  #define FACTORYRESET_PIN 25    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 26
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 23   //Dallas temp sensor pin.  If not used, SET TO -1   
  //#define LIGHT_SENSOR_PIN 18
  #define PIN_SDA 21             // you can set the used SDA and SCL pins
  #define PIN_SCL 22             // if it is not default value
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT11
  //#define TEMP_DHT_PIN  23
  //#define USE_BME280            //I2C Temperature + humidity + pressure
  //#define USE_BMP280            //I2C Temperature + barometric  pressure
  //#define USE_AHTX0             //I2C Temperature + humidity
  //#define USE_SHT21             //I2C Temperature + humidity
  //#define USE_BH1750            //I2C luxmeter sensor
  //#define USE_MQTT
//_______________________________ HV5122 setup ____________________________________________________  
  #define NEWHV5122
  #define PIN_DIN  14   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  13   // Clock
  #define PIN_OE   27   // OutputEnable
  #define MAXDIGITS 6
  int maxDigits = MAXDIGITS;
  byte digitPins[MAXDIGITS+1][10] = {   
    {130,128,126,124,123,125,127,129,131,132},  //sec   1 , chip1 (tube#0) 0-1-2-3-4-5-6-7-8-9
    {120,118,116,114,113,115,117,119,121,122},  //sec  10 , chip1 (tube#1)
    {109,107,105,103,102,104,106,108,110,111},  //min   1 , chip1 (tube#2)    
    {30,28,26,24,23,25,27,29,31,32},            //min  10 , chip0  (tube#3)
    {19,17,15,13,12,14,16,18,20,21},            //hour 10 , chip0 (tube#4)
    {8,6,4,2,1,3,5,7,9,10},                     //hour   1 , chip0 (tube#5)    
    {0,112,101,22,11,0}              //Z574M {0,101,112,11,22,0}blinking dots (tube0...tube6)Z573M {0,112,101,22,11,0}
    };    
   //#define MAKE_BLINKING_DOTS //it means, the extra datapins are used as 4 blinking dot instead of decimal points!  #1..#4 positions are used
//___________________________________________________________________________________    
  #define ALARMSPEAKER_PIN 19   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 33    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define RADAR_PIN 4
  #define RADAR_TIMEOUT 5  //min
  //#define TUBE_POWER_PIN 4
  #define TUBE_POWER_ON  HIGH  
  //#define ENABLE_CLOCK_DISPLAY true  
  //#define DATE_REPEAT_MIN 1       //show date only every xxx minute. If zero, datum is never displayed
  //#define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  //#define TEMP_CHARCODE 10
  //#define GRAD_CHARCODE 10 
  //#define PERCENT_CHARCODE 10
  #define AP_NAME "UNFICLOCK32"
  #define AP_PASSWORD "uniclock" 
  #define WEBNAME " Nixie-Clock"
  #define DEFAULT_SSID ""
  #define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER  
#endif

#ifdef CLOCK_57A   //WROOM ESP32, UNFI 2xHV5222 PCB version, 6xZ57xM clock   Example clock!!!
  #define DEBUG 
  #define FW "fw57A"  //firmware name
  #define MAXBRIGHTNESS 100    
  #define USE_NEOPIXEL 
  #define NEOPIXEL_PIN 26
  byte tubePixels[] = {0,1,2,3,4,5};        //6 tubes, single leds
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 23   //Dallas temp sensor pin.  If not used, SET TO -1   
  //#define LIGHT_SENSOR_PIN 18
  #define PIN_SDA 21             // you can set the used SDA and SCL pins
  #define PIN_SCL 22             // if it is not default value
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT11
  //#define TEMP_DHT_PIN  23
  //#define USE_BME280            //I2C Temperature + humidity + pressure
  //#define USE_BMP280            //I2C Temperature + barometric  pressure
  //#define USE_AHTX0             //I2C Temperature + humidity
  //#define USE_SHT21             //I2C Temperature + humidity
  //#define USE_BH1750            //I2C luxmeter sensor
  //#define USE_MQTT
//_______________________________ HV5122 setup ____________________________________________________  
  #define NEWHV5122
  #define PIN_DIN  14   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
  #define PIN_CLK  13   // Clock
  #define PIN_OE   27   // OutputEnable
  #define MAXDIGITS 6
  int maxDigits = MAXDIGITS;
  byte digitPins[MAXDIGITS+1][10] = {   
    {103,105,107,109,110,108,106,104,102,101},  //sec   1 , chip1 (tube#0) 0-1-2-3-4-5-6-7-8-9
    {113,115,117,119,120,118,116,114,112,111},  //sec  10 , chip1 (tube#1)
    {124,126,128,130,131,129,127,125,123,122},  //min   1 , chip1 (tube#2)    
    {3,5,7,9,10,8,6,4,2,1},                     //min  10 , chip0  (tube#3)
    {14,16,18,20,21,19,17,15,13,12},            //hour 10 , chip0 (tube#4)
    {25,27,29,31,32,30,28,26,24,23},            //hour   1 , chip0 (tube#5)    
    {0,121,132,11,22,0,0,0,0}                        //extra decimalPoint/blinking dots (tube0...tube6)
    };    
   //#define MAKE_BLINKING_DOTS //it means, the extra datapins are used as 4 blinking dot instead of decimal points!  #1..#4 positions are used
//___________________________________________________________________________________    
  #define ALARMSPEAKER_PIN 19   //Alarm buzzer pin                                            
  #define ALARMBUTTON_PIN 33    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  //#define RADAR_PIN 4
  #define RADAR_TIMEOUT 5  //min
  //#define TUBE_POWER_PIN 4
  #define TUBE_POWER_ON  HIGH
  //#define ENABLE_CLOCK_DISPLAY true  
  //#define DATE_REPEAT_MIN 1       //show date only every xxx minute. If zero, datum is never displayed
  //#define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  //#define TEMP_CHARCODE 10
  //#define GRAD_CHARCODE 10 
  //#define PERCENT_CHARCODE 10
  #define AP_NAME "UNFICLOCK"
  #define AP_PASSWORD "uniclock" 
  #define WEBNAME "ZM1336 Nixie-Clock"
  #define DEFAULT_SSID ""
  #define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER  
#endif

#ifdef CLOCK_70   //1 tube esp8266 Nixie Clock by UNFI 
  #define DEBUG 
  #define FW "fw70"  //firmware name
  #define MAXBRIGHTNESS 100  
  #define TUBE1CLOCK
  byte ABCDPins[4] =  {2,4,5,15};   
  #define AP_NAME "1TUBE_CLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "1TUBE Nixie Clock"
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_71   //2x VQC10 clock by UNFI 
  #define DEBUG 
  #define PIN_FLD_BUTTON   12     
  #define PIN_SET_BUTTON   13  
  //#define USE_GPS
  #define USE_RTC   //no RTC exist, but the push buttons are defined here, too
  #define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 15    //PIN16 not working!!!
  #define FW "fw71"  //firmware name
  #define MAXBRIGHTNESS 100  
  #define VQC10
  #define latchPin  5 //D2 RCK
  #define clockPin  4 //D1 SCK
  #define dataPin   2 //D4 DIN
  #define ledOffpin 0 //D5
  #define D5pin  14  
  #define AP_NAME "VQC10_LED_CLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "VQC10 LED Clock"
  //#define DEFAULT_SSID "xxx"   //factory reset default wifi ssid/psw
  //#define DEFAULT_PSW "yyy"
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION
#endif

#ifdef CLOCK_72   //ESP32, UNFI board, ZM1500
  #define DEBUG 
  #define ANIMSPEED 50
  #define FW "fw72"  //firmware name
  #define MAXBRIGHTNESS 255    
  #define SIMULATE_TEMPERATURE_SENSOR
  //#define USE_DALLAS_TEMP
  //#define USE_DHT_TEMP
  //#define DHTTYPE DHT22
  //#define TEMP_DHT_PIN 27
  #define TEMP_DALLAS_PIN 27    //Dallas temp sensor pin.  If not used, SET TO -1    
  #define ZM1500_ESP32
  byte digitEnablePins[] = {11,10,9,8,7,6,5,4,3,2,1,0};  //{11,10,9,8,7,6,5,4,3,2,1,0};   //12 tubes selection values
  byte segmentEnablePins[] = {7,6,5,4,3,2,1,0};
  #define dataPin  23  //D5
  #define latchPin 18  //D6
  #define clkPin   19  //D7
  //#define TEMP_CHARCODE -1   //disable char => shift display right with 1 digit
  //#define GRAD_CHARCODE -1   //disable char => shift display right with 1 digit
  #define AP_NAME "ZM1500_CLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "ZM1500-Clock"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION  
#endif

#ifdef CLOCK_80   //word clock, english version
  #define DEBUG 
  //#define SIMULATE_TEMPERATURE_SENSOR //for test only!!!
  #define NEOPIXEL_PIN 4
  #define PIN_FLD_BUTTON   27     
  #define PIN_SET_BUTTON   14
  #define LED1_PIN 25
  #define LED2_PIN 26 
  #define LIGHT_SENSOR_PIN 35
  //#define RADAR_PIN 16
  #define RADAR_TIMEOUT 5  //min
  //#define USE_GPS
  #define PIN_SDA 21          // you can set the used SDA and SCL pins
  #define PIN_SCL 22          // if it is not default value
  #define USE_RTC             //RTC + push buttons are defined here
  #define USE_BME280          //I2C Temperature + humidity + pressure
  #define USE_AHTX0             //I2C Temperature + humidity
  #define USE_SHT21             //I2C Temperature + humidity  
  #define FW "fw80"           //firmware name
  #define START_CORNER LEFTUP    //RIGHTUP, LEFTUP, RIGHTDOWN, LEFTDOWN  : starting corner of led stripe
  #define MAXBRIGHTNESS 255   //maximum led brightness, 100..255
  #define MAXIMUM_LUX 40
  #define WORDCLOCK
  #define AP_NAME "WORD_CLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "WordClock"
  #define MDNSNAME "wordclock"
  //#define DEFAULT_SSID "xxx"   //factory reset default wifi ssid/psw
  //#define DEFAULT_PSW "yyy"
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION
  //
  //----- WORD CLOCK TEXT DEFINITIONS -----------------
  #define WORD_LENGTH 15   //maximum length of any word
  struct wordDef{
    char txt[WORD_LENGTH+1];   //readable word
    int startX;    //led X positions
    int startY;    //led Y position
    int len;       //number of characters
  };  
  #define XMAX 11  //Clock dimensions
  #define YMAX 11
  #define WEEKDAY_POSX  1      //start of weekdays
  #define WEEKDAY_POSY  11      //start of weekdays
  int extraDots[4] = {XMAX*YMAX+3,XMAX*YMAX+2,XMAX*YMAX+1,XMAX*YMAX};   //extra dots led numbers
  #define WEEKDAY_POSX  1      //start of weekdays
  #define WEEKDAY_POSY  11      //start of weekdays
  #define SENSOR_POSX 8     //start position of extra dots
  #define SENSOR_POSY 11    
  
  const char matrix[YMAX][XMAX+1] = {
  {"itLisASwifi"},
  {"quarterXset"},
  {"twentyfiveX"},
  {"halfBtenFto"},
  {"pastERUNINE"},
  {"ONESIXTHREE"},
  {"FOURFIVETWO"},
  {"EIGHTELEVEN"},
  {"SEVENTWELVE"},
  {"TENSEoclock"},
  {"SMTWTFSCF%W"}
  };

  const char sequence[12][40] = {   //#1 = actual hour,  #2 = actual hour + 1
  "it is #1 oclock",
  "it is five past #1",
  "it is ten past #1",
  "it is quarter past #1",
  "it is twenty past #1",
  "it is twenty five past #1",
  "it is half past #1",
  "it is twenty five to #2",
  "it is twenty to #2",
  "it is quarter to #2",
  "it is ten to #2",
  "it is five to #2"
  };

  struct wordDef hours[] {
    {"O"},{"ONE"},{"TWO"},{"THREE"},{"FOUR"},{"FIVE"},{"SIX"},{"SEVEN"},{"EIGHT"},{"NINE"},{"TEN"},{"ELEVEN"},{"TWELVE"},
  };

  struct wordDef words[] {
    {"it"},{"is"},{"wifi"},{"ip"},{"twenty"},{"five"},{"ten"},{"quarter"},{"half"},{"to"},{"past"},{"oclock"},{"SMTWTFS"},{"CF%W"}
  };  
#endif

#ifdef CLOCK_81   //word clock, german version
  #define DEBUG 
  #define NEOPIXEL_PIN 4
  #define PIN_FLD_BUTTON   27     
  #define PIN_SET_BUTTON   14
  #define LED1_PIN 25
  #define LED2_PIN 26 
  //#define LIGHT_SENSOR_PIN 35
  //#define RADAR_PIN 16
  //#define RADAR_TIMEOUT 5  //min
  //#define USE_GPS
  #define PIN_SDA 21          // you can set the used SDA and SCL pins
  #define PIN_SCL 22          // if it is not default value
  #define USE_RTC             //RTC + push buttons are defined here
  #define USE_BME280          //I2C Temperature + humidity + pressure
  #define USE_AHTX0             //I2C Temperature + humidity
  #define USE_SHT21             //I2C Temperature + humidity  
  #define FW "fw81"  //firmware name
  #define START_CORNER RIGHTUP    //RIGHTUP, LEFTUP, RIGHTDOWN, LEFTDOWN  : starting corner of led stripe
  #define MAXBRIGHTNESS 255   //led maximum brightness 100...255
  #define MAXIMUM_LUX 40
  #define WORDCLOCK
  #define AP_NAME "WORD_CLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "wordclock"
  //#define DEFAULT_SSID "xxx"   //factory reset default wifi ssid/psw
  //#define DEFAULT_PSW "yyy"
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION
  //
  //----- WORD CLOCK TEXT DEFINITIONS -----------------
  #define WORD_LENGTH 15   //maximum length of any word
  struct wordDef{
    char txt[WORD_LENGTH+1];   //readable word
    int startX;    //led X positions
    int startY;    //led Y position
    int len;       //number of characters
  };  
  
  #define XMAX 11  //Clock dimensions
  #define YMAX 11
  #define WEEKDAY_POSX  1      //start of weekdays
  #define WEEKDAY_POSY  11      //start of weekdays
  #define EXTRADOTS_POSX 1     //start position of extra dots
  #define EXTRADOTS_POSY 12  
  #define WEEKDAY_POSX  1      //start of weekdays
  #define WEEKDAY_POSY  11      //start of weekdays
  #define SENSOR_POSX 8     //start position of extra dots
  #define SENSOR_POSY 11    
  
  const char matrix[YMAX][XMAX+1] = {
  {"esListAwifi"},
  {"zehnzwanzig"},
  {"dreiviertel"},
  {"funfvornach"},
  {"halbOELFUNF"},
  {"EINSXAMZWEI"},
  {"DREIAUJVIER"},
  {"SECHSNLACHT"},
  {"SIEBENZWOLF"},
  {"ZEHNEUNKuhr"},
  {"SMTWTFSCF%W"}
  };

  const char sequence[12][40] = {   //#1 = actual hour,  #2 = actual hour + 1
  "es ist #1 uhr",
  "es ist funf nach #1",
  "es ist zehn nach #1",
  "es ist viertel #2",
  "es ist zehn vor halb #2",
  "es ist funf vor halb #2",
  "es ist halb #2",
  "es ist funf nach halb #2",
  "es ist zehn nach halb #2",
  "es ist drei viertel #2",
  "es ist zehn vor #2",
  "es ist funf vor #2"
  };

  struct wordDef hours[] {
    {"O"},{"EIN"},{"ZWEI"},{"DREI"},{"FIER"},{"FUNF"},{"SECHS"},{"SIEBEN"},{"ACHT"},{"NEUN"},{"ZEHN"},{"ELF"},{"ZWOLF"},
  };

  struct wordDef words[] {
    {"es"},{"ist"},{"zwanzig"},{"funf"},{"zehn"},{"drei"},{"viertel"},{"halb"},{"vor"},{"nach"},{"uhr"},{"SMDMDFS"},{"CF%W"},{"wifi"}
  };  
#endif

#ifdef CLOCK_82   //word clock, hungarian version
  #define DEBUG 
  #define NEOPIXEL_PIN 4
  #define PIN_FLD_BUTTON   27     
  #define PIN_SET_BUTTON   14
  #define LED1_PIN 25
  #define LED2_PIN 26 
  #define LIGHT_SENSOR_PIN 35
  //#define RADAR_PIN 16
  //#define RADAR_TIMEOUT 5  //min
  //#define USE_GPS
  #define PIN_SDA 21          // you can set the used SDA and SCL pins
  #define PIN_SCL 22          // if it is not default value
  #define USE_RTC             //RTC + push buttons are defined here
  #define USE_BME280          //I2C Temperature + humidity + pressure
  #define USE_AHTX0             //I2C Temperature + humidity
  #define USE_SHT21             //I2C Temperature + humidity  
  #define FW "fw82"  //firmware name
  #define START_CORNER RIGHTUP    //RIGHTUP, LEFTUP, RIGHTDOWN, LEFTDOWN  : starting corner of led stripe  
  #define MAXBRIGHTNESS 255   //led max brightness 100...255
  #define MAXIMUM_LUX 40
  #define WORDCLOCK
  #define AP_NAME "WORD_CLOCK"
  #define AP_PASSWORD ""
  #define WEBNAME "wordclock"
  //#define DEFAULT_SSID "xxx"   //factory reset default wifi ssid/psw
  //#define DEFAULT_PSW "yyy"
  //#define USE_WIFIMANAGER
  //#define DISABLE_NIGHT_ANIMATION
  //
  //----- WORD CLOCK TEXT DEFINITIONS -----------------
  #define WORD_LENGTH 15   //maximum length of any word
  struct wordDef{
    char txt[WORD_LENGTH+1];   //readable word
    int startX;    //led X positions
    int startY;    //led Y position
    int len;       //number of characters
  };
    
  #define XMAX 11  //Clock dimensions
  #define YMAX 11
  #define WEEKDAY_POSX  1      //start of weekdays
  #define WEEKDAY_POSY  11      //start of weekdays
  #define EXTRADOTS_POSX 1     //start position of extra dots
  #define EXTRADOTS_POSY 12  
  #define WEEKDAY_POSX  1      //start of weekdays
  #define WEEKDAY_POSY  11      //start of weekdays
  #define SENSOR_POSX 8     //start position of extra dots
  #define SENSOR_POSY 11    
  
  const char matrix[YMAX][XMAX+1] = {
  {"otBtizJwifi"},
  {"perccelEWip"},
  {"multmulvaQZ"},
  {"haromUXfelJ"},
  {"negyedXHHAT"},
  {"NEGYAHAROMS"},
  {"NYOLCOTZHET"},
  {"TIZENEGYEBC"},
  {"TIZENKETTOA"},
  {"KILENCFORAP"},
  {"SMTWTFSCF%W"}
  };

  const char sequence[12][40] = {   //#1 = actual hour,  #2 = actual hour + 1
  "#1 ora",
  "#1 ora mult ot perccel",
  "#1 ora mult tiz perccel",
  "negyed #2",
  "tiz perc mulva fel #2",
  "ot perc mulva fel #2",
  "fel #2",
  "fel #2 mult ot perccel",
  "fel #2 mult tiz perccel",
  "harom negyed #2",
  "tiz perc mulva #2",
  "ot perc mulva #2"
  };

  struct wordDef hours[] {
    {"0"},{"EGY"},{"KET"},{"HAROM"},{"NEGY"},{"OT"},{"HAT"},{"HET"},{"NYOLC"},{"KILENC"},{"TIZ"},{"TIZENEGY"},{"TIZENKETTO"},
  };

  struct wordDef words[] {
    {"ora"},{"ot"},{"tiz"},{"harom"},{"negyed"},{"fel"},{"perc"},{"perccel"},{"mulva"},{"mult"},{"wifi"},{"ip"},{"VHKSCPS"}
  };  
#endif

#ifdef CLOCK_99   //Dummy clock, sensor box with MQTT connector
  #define DEBUG 
  #define FW "fw99"  //firmware name
  #define MAXBRIGHTNESS 100    
  //#define USE_DALLAS_TEMP
  #define TEMP_DALLAS_PIN 26    //Dallas temp sensor pin.  If not used, SET TO -1   
  //#define LIGHT_SENSOR_PIN 23
  #define PIN_SDA 4             // you can set the used SDA and SCL pins
  #define PIN_SCL 32             // if it is not default value
  #define USE_DHT_TEMP
  #define DHTTYPE DHT11
  #define TEMP_DHT_PIN  26
  //#define USE_RTC
  //#define USE_GPS
  #define USE_BME280            //I2C Temperature + humidity + pressure
  #define USE_BMP280            //I2C Temperature + barometric  pressure
  #define USE_AHTX0             //I2C Temperature + humidity
  #define USE_SHT21             //I2C Temperature + humidity
  #define USE_BH1750            //I2C luxmeter sensor
  #define USE_MQTT
  //#define USE_MASTER_CLOCK  //if used, this will be the sensor#0
  //#define USE_MASTER_TEMP
  //#define USE_MASTER_HUMID
  //#define MASTER_TEMPERATURE_TOPIC "homeassistant/sensor/10521c5e14c4/temperature/state"
  //#define MASTER_HUMIDITY_TOPIC "homeassistant/sensor/10521c5e14c4/humidity/state"
  //#define MASTER_RADAR_TOPIC "homeassistant/sensor/10521c5e14c4/radar/state"
//_______________________________ Driver setup ____________________________________________________  
  #define DUMMY
 //___________________________________________________________________________________    
  #define ALARMSPEAKER_PIN 16   //Alarm buzzer pin                                            
  //#define ALARMBUTTON_PIN 23    //Alarm switch off button pin 
  #define ALARM_ON HIGH         //How to switch ON alarm buzzer
  #define RADAR_PIN 22
  #define RADAR_TIMEOUT  5 //min
  #define ENABLE_CLOCK_DISPLAY true  
  #define DATE_REPEAT_MIN 3       //show date only every xxx minute. If zero, datum is never displayed
  //#define SHIFT_TUBES_LEFT_BY_1 //shift left by 1 tube the display, if a thermometer is used with spec tube
  #define TEMP_CHARCODE 10
  #define GRAD_CHARCODE 10 
  #define PERCENT_CHARCODE 10
  #define AP_NAME "SENSORBOX"
  #define AP_PASSWORD "" 
  #define WEBNAME "SENSORBOX"
  //#define DEFAULT_SSID ""
  //#define DEFAULT_PSW ""
  //#define USE_WIFIMANAGER  
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
  #if defined(PIN_SDA) && defined(PIN_SCL)
    #define USE_I2CSENSORS
  #else
    #warning "PIN_SDA or PIN_SCL is not defined!"  
  #endif
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
  #define RADAR_TIMEOUT 5     //Automatic switch off tubes (without radar detecting somebody) after xxx min
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

#ifndef UPPER_CIRCLE_CHARCODE 
  #define UPPER_CIRCLE_CHARCODE 16 
#endif

#ifndef LOWER_CIRCLE_CHARCODE 
  #define LOWER_CIRCLE_CHARCODE 18 
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
#ifndef FW
  #define FW "UNKNOWN"
#endif
#ifndef DP_PIN
  #define DP_PIN   -1        //Decimal Point pin.
#endif
//______________________________________________________________________________________

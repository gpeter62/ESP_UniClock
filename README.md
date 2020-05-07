# ESP_UniClock

Universal WifiClock with Nixie, VFD, Numitron tubes or LED display
Number of digits: 4, 6 or 8

Adjustable day and night brightness

ANIMATE: 4 animation modes, 5 means random animation

Hardver:

Wemos D1 Mini (or any other ESP8266 modul) 160MHz

Drivers: SN74141 (multiplex or latch), MAX6921, MAX7219, MM5450

Optional: DS18B20 temperature sensor

Optional: DS3231 RTC Clock with Wifi/Manual mode switch and pushbuttons to set the date/time

User defined pin numbers for digits and segments

Web page for setup. See: Screenshot.png
-----------------------------------------------------------------------------------------------------
Program parameters:

#define DEBUG
//#define USE_DALLAS_TEMP   //TEMP_SENSOR_PIN is used to connect the sensor
//#define USE_RTC           //I2C pins are used!   SCL = D1 (GPIO5), SDA = D2 (GPIO4)
#define MAXBRIGHTNESS 10  //10...15    (if too high value is used, the multiplex may be too slow...)

//Use only 1 from the following options!
//#define MULTIPLEX74141    //4..8 Nixie tubes
//#define NO_MULTIPLEX74141 //4..6 Nixie tubes
//#define MAX6921           //4..8 VFD tubes   (IV18)
//#define MM5450            //6..8 LEDS
//#define MAX7219CNG        //4..8 LED 
#define Numitron_4511N

#define colonPin 2        //Blinking Colon pin.  If not used, SET TO -1
#define TEMP_SENSOR_PIN 4  //Dallas temp sensor pin

//Display temperature and date in every minute between START..END seconds
#define TEMP_START  35
#define TEMP_END    40
#define DATE_START  30
#define DATE_END    38
#define ANIMSPEED   50  //Animation speed in millisec 

char webName[] = "Numitron Clock 1.1";
--------------------------------------------------------------------------------------------------------
Special parameters (for example) for MAX6921:
#define VFDrefresh 1200    //msec, Multiplex time period. Greater value => slower multiplex frequency

//Fill this table with the pin numbers of MAX6921 chip!   
byte segmentEnablePins[] =  {0,2,5,6,4,1,3,7};   //segment enable pins of MAX6921 (a,b,c,d,e,f,g,DP)
byte digitEnablePins[] = {18,11,17,12,16,13,14,15}; //19};  //segment enable pins of MAX6921 (1,2,3,4,5,6,7,8)  

//MAX6921 pins
#define PIN_LE    12  // D6 Shift Register Latch Enable
#define PIN_CLK   13  // D7 Shift Register Clock
#define PIN_DATA  14  // D5 Shift Register Data
#define PIN_BL    15  // D8 Shift Register Blank (1=display off     0=display on)
--------------------------------------------------------------------------------------------------------
More info:

Peter Gautier
 Email: gautier.p62@gmail.com

#ifdef USE_GPS

#include <TinyGPS++.h>                                  // Tiny GPS Plus Library

#if defined(ESP8266)  
  #include "SoftwareSerial.h"     // Software Serial Library so we can use other Pins for communication with the GPS module
  static int RXPin = 3;     //RX pin  -  remove wire, when uploading program to 8266          
  static int TXPin = -1;    //not used   
  SoftwareSerial ss(RXPin, TXPin);  
#else
  #include <HardwareSerial.h>   //on ESP32 no SoftwareSerial is needed
  static int RXPin = 25;     //RX pin 
  static int TXPin = -1;    //not used   
  HardwareSerial ss(1);  
#endif
           
// The serial connection to the GPS device
static uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600
TinyGPSPlus gps;                                  // Create an Instance of the TinyGPS++ object called gps

void setupGPS() { 
  delay(1500);     
  DPRINTLN("Starting GPS..."); 
  if (RXPin>=0) regPin(RXPin,"RXPin");   
  if (TXPin>=0) regPin(TXPin,"TXPin");
  #if defined(ESP8266)
    ss.begin(GPSBaud);  
    //ss.begin(GPSBaud, SWSERIAL_8N1, RXPin, TXPin, false); 
  #else        
    ss.begin(GPSBaud, SERIAL_8N1, RXPin);    
  #endif
  delay(200);
  GPSexist = true;
  smartDelay(500);
  getGPS(); 
}

boolean getGPS(){ 
static unsigned long lastRun = 0;
boolean res = false;

   if ((lastRun!=0) && (millis() - lastRun) < 10000) //refresh rate
      return(res);   
      
   lastRun = millis();
   smartDelay(500);
   if (gps.time.isValid() && gps.date.isValid() && gps.time.isUpdated())  {
    if (gps.date.year()>2020) {
      DPRINTLN("Time refreshed from GPS");
      setTime(gps.time.hour(),gps.time.minute(),gps.time.second(),gps.date.day(),gps.date.month(),gps.date.year());  //set the time (hr,min,sec,day,mnth,yr)
      setTime(now()+(prm.utc_offset + (prm.enableDST ? 1 : 0)) * 3600);
      lastTimeUpdate = millis();
    }
    #ifdef DEBUG
      printGPS();
    #endif
    
    res = true;
   } //endif valid date&time
   return (res);
}

void printGPS() {
  DPRINT("Latitude  : ");  DPRINT(gps.location.lat(), 5);  DPRINT("  Longitude : ");  DPRINTLN(gps.location.lng(), 4);
  DPRINT("Satellites: ");  DPRINT(gps.satellites.value());  DPRINT("  Elevation : ");  DPRINT(gps.altitude.feet()); DPRINTLN("ft"); 
  DPRINT("GPS Time UTC  : ");  DPRINT(gps.time.hour());   // GPS time UTC 
  DPRINT(":");  DPRINT(gps.time.minute());                // Minutes
  DPRINT(":");  DPRINTLN(gps.time.second());              // Seconds
}


void smartDelay(unsigned long ms)  {              // This custom version of delay() ensures that the gps object is being "fed".
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

//------------------------------------------------------------------------------------------

#else
void setupGPS() {GPSexist = false;}
boolean getGPS() {return(false);}
#endif

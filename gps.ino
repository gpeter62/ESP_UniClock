#ifdef USE_GPS

#include <TinyGPS++.h>                                  // Tiny GPS Plus Library

#if defined(ESP8266)  
  #include <SoftwareSerial.h>     // Software Serial Library so we can use other Pins for communication with the GPS module
  static const int RXPin = 3;     //RX pin  -  remove, when upload program to 8266          
  static const int TXPin = -1;    //not used            
  SoftwareSerial ss(RXPin, TXPin);                        // The serial connection to the GPS device
  
#elif defined(ESP32)
//The ESP32 has 3 different Serial Ports (UART). You can just use one of them:
//Serial0: RX0 on GPIO3, TX0 on GPIO1
//Serial1: RX1 on GPIO9, TX1 on GPIO10 (+CTS1 and RTS1)
//Serial2: RX2 on GPIO16, TX2 on GPIO17 (+CTS2 and RTS2)
//https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/HardwareSerial.cpp

  #include <HardwareSerial.h>     // Software Serial Library so we can use other Pins for communication with the GPS module
  static const int RXPin = 9;     //RX pin  -  remove, when upload program to 8266          
  static const int TXPin = 10;    //not used            
  HardwareSerial ss(1);                        // The serial connection to the GPS device Serial1
#endif

static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600

TinyGPSPlus gps;                                        // Create an Instance of the TinyGPS++ object called gps


void setupGPS() { 
  Fdelay(1500);     
  DPRINTLN("Starting GPS..."); 
  regPin(RXPin,"RXPin");   
  regPin(TXPin,"TXPin");
  clockWifiMode = false;
                         
  ss.begin(GPSBaud);                                    // Set Software Serial Comm Speed to 9600    
  delay(200);
  GPSexist = true;
  smartDelay(500);
  getGPS(); 
}

boolean getGPS(){ 
static unsigned long lastRun = 0;
boolean res = false;

   if ((lastRun==0) || (millis() - lastRun) < 10000) 
      return(res);   //refresh rate
      
   lastRun = millis();
   smartDelay(500);
   if (gps.time.isValid() && gps.date.isValid() && gps.time.isUpdated())  {
   lastTimeUpdate = millis();
   /*
   int hours = gps.time.hour() + prm.utc_offset; if (prm.enableDST) hours++;
    if (hours > 23) {
      hours = hours - 24; 
    }
    else {
      if (hours < 0) {
        hours = hours + 24;
      }
    }
    */
      
    setTime(gps.time.hour(),gps.time.minute(),gps.time.second(),gps.date.day(),gps.date.month(),gps.date.year());  //set the time (hr,min,sec,day,mnth,yr)
    setTime(now()+(prm.utc_offset + prm.enableDST ? 1 : 0) * 3600);
    #ifdef DEBUG
      printGPS();
    #endif
    
    res = true;
   } //endif valid date&time
   
   return (res);
}

void printGPS() {
  DPRINT("Latitude  : ");  DPRINTLN(gps.location.lat(), 5);
  DPRINT("Longitude : ");  DPRINTLN(gps.location.lng(), 4);
  DPRINT("Satellites: ");  DPRINTLN(gps.satellites.value());
  DPRINT("Elevation : ");  DPRINT(gps.altitude.feet()); DPRINTLN("ft"); 
  DPRINT("Time UTC  : ");  DPRINT(gps.time.hour());   // GPS time UTC 
  DPRINT(":");  DPRINT(gps.time.minute());            // Minutes
  DPRINT(":");  DPRINTLN(gps.time.second());            // Seconds
}


static void smartDelay(unsigned long ms)  {              // This custom version of delay() ensures that the gps object is being "fed".
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

//------------------------------------------------------------------------------------------

#else
void setupGPS() {GPSexist = false;}
boolean getGPS() {return(false);}
#endif

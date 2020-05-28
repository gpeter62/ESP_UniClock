#ifdef USE_GPS

#include <TinyGPS++.h>                                  // Tiny GPS Plus Library
#include <SoftwareSerial.h>                             // Software Serial Library so we can use other Pins for communication with the GPS module

static const int RXPin = 3;     //RX pin  -  remove, when upload program to 8266          
static const int TXPin = -1;    //not used            
static const uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600

TinyGPSPlus gps;                                        // Create an Instance of the TinyGPS++ object called gps
SoftwareSerial ss(RXPin, TXPin);                        // The serial connection to the GPS device

void setupGPS() { 
  delay(1500);     
  DPRINTLN("Starting GPS...");    
  DPRINT("- RX: GPIO"); DPRINTLN(RXPin);
  DPRINT("- TX: GPIO"); DPRINTLN(TXPin);
  clockWifiMode = false;
                         
  ss.begin(GPSBaud);                                    // Set Software Serial Comm Speed to 9600    
  delay(200);
  smartDelay(500);
  getGPS(); 
}

void getGPS(){ 
static unsigned long lastRun = 0;

   if ((millis() - lastRun) < 10000) return;   //refresh rate
   lastRun = millis();
   smartDelay(500);
   if (gps.time.isValid() && gps.date.isValid() && gps.time.isUpdated())  {
   int hours = gps.time.hour() + prm.utc_offset; if (prm.enableDST) hours++;
    if (hours > 23) {
      hours = hours - 24; 
    }
    else {
      if (hours < 0) {
        hours = hours + 24;
      }
    }
    printGPS();
    setTime(hours,gps.time.minute(),gps.time.second(),gps.date.day(),gps.date.month(),gps.date.year());  //set the time (hr,min,sec,day,mnth,yr)
   } //endif valid date&time
}

void printGPS() {
    Serial.print("Latitude  : ");  Serial.println(gps.location.lat(), 5);
  Serial.print("Longitude : ");  Serial.println(gps.location.lng(), 4);
  Serial.print("Satellites: ");  Serial.println(gps.satellites.value());
  Serial.print("Elevation : ");  Serial.print(gps.altitude.feet());  Serial.println("ft"); 
  Serial.print("Time UTC  : ");  Serial.print(gps.time.hour());                       // GPS time UTC 
  Serial.print(":");  Serial.print(gps.time.minute());                     // Minutes
  Serial.print(":");  Serial.println(gps.time.second());                   // Seconds
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
void setupGPS() {}
void getGPS() {}
#endif

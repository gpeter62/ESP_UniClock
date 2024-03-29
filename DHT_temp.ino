#ifdef USE_DHT_TEMP

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"
// TEMP_DHT_PIN is used to connect the sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11
#ifndef DHTTYPE
  #define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321  //default DHT type
#endif  
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like ESP8266 or ESP32 connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor


DHT dht(TEMP_DHT_PIN, DHTTYPE);
int DHTtempPtr = -1;
int DHThumidPtr = -1;

void setupDHTemp() {
  float tempTMP, humidTMP;
  regPin(TEMP_DHT_PIN,"TEMP_DHT_PIN");
  dht.begin();
  int i= 0;
  while (i<5) {
    humidTMP = dht.readHumidity();
    tempTMP = dht.readTemperature();
    DPRINT("DHT Test:"); DPRINT(tempTMP); DPRINT("/"); DPRINTLN(humidTMP);
    i++;
    if (!isnan(tempTMP)) 
      break;
    delay(1000);
  }  
  if (isnan(tempTMP)) {
    DPRINT("No DHTxx sensor found on GPIO"); DPRINTLN(TEMP_DHT_PIN);
    return;
  }
  DHTtempPtr = useTemp;  //remember my ID-s
  DHThumidPtr = useHumid;
  useTemp++;   //increase the number of sensors
  useHumid++;
}

void getDHTemp() {
static unsigned long lastRun = 0;
float tempTMP, humidTMP;

  if (EEPROMsaving) return;
  if (DHTtempPtr<0) return;  //no sensor
  //if (((millis()-lastRun)<2500) || (second()!=prm.tempStart)) return;
  if ((((millis()-lastRun)<2500) || (second()!=prm.tempStart)) && (lastRun !=0)) return;
  lastRun = millis();  

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  disableDisplay();
  humidTMP = round1(dht.readHumidity());
  if ((humidTMP>0) && (humidTMP<=100))  humid[DHThumidPtr] = humidTMP;
  tempTMP = round1(dht.readTemperature());      // Read temperature as Celsius (the default)
  if (tempTMP<99) temperature[DHTtempPtr] = tempTMP;
  enableDisplay(0);

  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float f = dht.readTemperature(true);   Compute heat index in Fahrenheit (the default)
  //float hif = dht.computeHeatIndex(f, h);
  
  //float hic = dht.computeHeatIndex(t, h, false);  // Compute heat index in Celsius (isFahreheit = false)
  
  if (isnan(humidTMP) || isnan(tempTMP)) {  // Check if any reads failed and exit early (to try again).
    DPRINTLN("DHT sensor reading error!");
    //temperature[DHTtempPtr] = 99.9f;
    //humid[DHThumidPtr] = 0;
  }
  else {
      DPRINT("DHTxx Temperature:"); DPRINT(temperature[DHTtempPtr]); DPRINT("C  Humidity:"); DPRINT(humid[DHThumidPtr]); DPRINTLN(" %");
  }
}

#else
void setupDHTemp() {}
void getDHTemp() {}
#endif

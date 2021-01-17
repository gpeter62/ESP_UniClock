#ifdef USE_DHT_TEMP
// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// - Adafruit Unified Sensor Lib: https://github.com/adafruit/Adafruit_Sensor

#include "DHT.h"
// TEMP_SENSOR_PIN is used to connect the sensor
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


DHT dht(TEMP_SENSOR_PIN, DHTTYPE);

void setupDHTemp() {
  DPRINT("DHTxx sensor setup: pin = "); DPRINTLN(TEMP_SENSOR_PIN);
  dht.begin();
  getDHTemp();
}

void getDHTemp() {
static unsigned long lastRun = 0;
float tempTMP, humidTMP;

  if (EEPROMsaving) return;
  if (((millis()-lastRun)<2500) || (second()!=TEMP_START)) return;
  lastRun = millis();  

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  disableDisplay();
  humidTMP = dht.readHumidity();
  if ((humidTMP>0) && (humidTMP<=100))  humid = humidTMP;
  tempTMP = dht.readTemperature();      // Read temperature as Celsius (the default)
  if (tempTMP<99) temperature[0] = tempTMP;
  enableDisplay(0);
  useTemp = 1;
  useHumid = 1;
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float f = dht.readTemperature(true);   Compute heat index in Fahrenheit (the default)
  //float hif = dht.computeHeatIndex(f, h);
  
  //float hic = dht.computeHeatIndex(t, h, false);  // Compute heat index in Celsius (isFahreheit = false)
  
  if (isnan(humid) || isnan(temperature[0])) {  // Check if any reads failed and exit early (to try again).
    DPRINTLN("No DHT sensor!");
    useTemp = 1;
    temperature[0] = 99.9f;
    humid = 0;
    useHumid = 0;
  }
  else {
      DPRINT("Temp:"); DPRINT(temperature[0]); DPRINT("C  Humidity:"); DPRINT(humid); DPRINTLN(" %");
  }
}

#else
void setupDHTemp() {}
void getDHTemp() {}
#endif

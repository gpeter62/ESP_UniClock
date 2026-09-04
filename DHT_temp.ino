// =================================================================================================
// DHT_temp.ino - DHT11 / DHT22 (AM2302/AM2321) / DHT21 combined temperature+humidity sensor driver
// -------------------------------------------------------------------------------------------------
// Single-wire (not 1-Wire protocol, just one signal wire) sensor read via the Adafruit DHT sensor
// library (libraries/DHT_sensor_library.zip), on pin TEMP_DHT_PIN with sensor type DHTTYPE.
// Enabled per-clock via USE_DHT_TEMP in clocks.h (pin: TEMP_DHT_PIN, type: DHTTYPE, default DHT22).
// Used (as of this writing) by CLOCK_18, CLOCK_21, CLOCK_22, CLOCK_23A, CLOCK_23B, CLOCK_32, CLOCK_50
// and CLOCK_99 - see clocks.h for the exact list. Unlike dallas_temp.ino, this driver only ever
// supports a single physical sensor (one temperature + one humidity reading).
//
// DHT sensors are slow and must not be polled too often (Adafruit recommends >=2s between reads,
// otherwise you get stale or garbage data back) - getDHTemp() below enforces its own minimum gap
// between reads independent of the DHT library (which does not enforce this itself).
// =================================================================================================
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
int DHTtempPtr = -1;    // index into the shared temperature[] array reserved for this sensor; -1 = "no sensor found / not set up"
int DHThumidPtr = -1;   // index into the shared humid[] array reserved for this sensor; -1 = "no sensor found / not set up"

// Probes for a DHT sensor at boot. Up to 5 attempts, 1 second apart (DHT sensors need time between
// reads to settle - see getDHTemp() below for the steady-state rate limit), stopping as soon as one
// attempt returns a valid (non-NaN) temperature. Only DHTtempPtr/DHThumidPtr are left at -1 (their
// initialized value) if no sensor ever responds, which is how getDHTemp() knows to do nothing later.
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
    if (!isnan(tempTMP))    // note: only the temperature reading is checked here, humidity's NaN state is not verified
      break;
    delay(1000);
  }
  if (isnan(tempTMP)) {
    DPRINT("No DHTxx sensor found on GPIO"); DPRINTLN(TEMP_DHT_PIN);
    return;    // DHTtempPtr/DHThumidPtr stay -1 and useTemp/useHumid are not incremented - sensor is effectively disabled
  }
  DHTtempPtr = useTemp;  //remember my ID-s
  DHThumidPtr = useHumid;
  useTemp++;   //increase the number of sensors
  useHumid++;
}

// Called every ~200ms from timeProgram(), AND once directly from setup() right after pin init (see
// ESP32_UniClock2.ino) to get an initial reading before the first display refresh.
// Unlike the DS18B20 driver, the DHT library's readTemperature()/readHumidity() calls themselves
// block for ~250ms (bit-banged single-wire protocol, no async/non-blocking mode available) - there is
// no separate request/poll split here, it's a straightforward "gate how often we call it" scheme instead.
void getDHTemp() {
static unsigned long lastRun = 0;
float tempTMP, humidTMP;

  if (EEPROMsaving) return;
  if (DHTtempPtr<0) return;  //no sensor
  // Rate limiting: DHT11/DHT22 sensors must not be read faster than roughly once every 2 seconds, or
  // they return stale/garbage data - readings are gated to once per minute, at the exact second
  // prm.tempStart (the same "start of temperature display window" second used to schedule the Dallas
  // driver's request), which is always >=2.5s apart in practice for any sane tempStart. The 2500ms
  // millis()-based check below is really just a safety net (e.g. against `second()` briefly matching
  // more than once, or the caller re-entering the same second) - the schedule itself already respects
  // the sensor's minimum read interval.
  //if (((millis()-lastRun)<2500) || (second()!=prm.tempStart)) return;
  // `&& (lastRun !=0)` makes the very first call (from setup(), before lastRun has ever been set)
  // bypass both the 2.5s and the second()==tempStart gating, so a reading is taken immediately at boot
  // instead of waiting for the clock to reach prm.tempStart.
  if ((((millis()-lastRun)<2500) || (second()!=prm.tempStart)) && (lastRun !=0)) return;
  lastRun = millis();

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  humidTMP = round1(dht.readHumidity());
  // Range check doubles as the NaN/error guard: a NaN humidity fails both comparisons (NaN vs any value
  // is always false in C/C++), so a failed read simply leaves humid[DHThumidPtr] holding its previous value.
  if ((humidTMP>0) && (humidTMP<=100))  humid[DHThumidPtr] = humidTMP;
  tempTMP = round1(dht.readTemperature());      // Read temperature as Celsius (the default)
  // Same implicit-NaN-guard trick as above: NaN<99 is false, so a failed read is silently skipped and
  // temperature[DHTtempPtr] keeps its last good value rather than being overwritten with garbage.
  if (tempTMP<99) temperature[DHTtempPtr] = tempTMP;

  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float f = dht.readTemperature(true);   Compute heat index in Fahrenheit (the default)
  //float hif = dht.computeHeatIndex(f, h);

  //float hic = dht.computeHeatIndex(t, h, false);  // Compute heat index in Celsius (isFahreheit = false)

  // NOTE: this isnan() check runs AFTER the assignments above have already decided (via the range
  // checks) whether to update temperature[]/humid[] - by this point the display has already been
  // re-enabled too. So this block is purely diagnostic logging on the raw (pre-round1) values, it does
  // not itself prevent a bad reading from being used (that already happened, or didn't, above).
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

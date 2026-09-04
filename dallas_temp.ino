// =================================================================================================
// dallas_temp.ino - DS18B20 (1-Wire) digital temperature sensor driver
// -------------------------------------------------------------------------------------------------
// Uses the OneWire + DallasTemperature libraries (libraries/OneWire.zip, libraries/DallasTemperature.zip)
// to talk to one or two DS18B20 sensors sharing a SINGLE 1-Wire data bus on pin TEMP_DALLAS_PIN
// (a 4.7k pull-up resistor between DATA and VCC is required on the bus, as usual for 1-Wire).
// Enabled per-clock via USE_DALLAS_TEMP in clocks.h; the pin itself is set via TEMP_DALLAS_PIN.
// Used (as of this writing) by CLOCK_3, CLOCK_5, CLOCK_6A, CLOCK_7, CLOCK_12, CLOCK_14, CLOCK_15,
// CLOCK_15a, CLOCK_16, CLOCK_20, CLOCK_26, CLOCK_30, CLOCK_31, CLOCK_35, CLOCK_40..CLOCK_49 (Nixie/VFD
// UNFI board revisions), CLOCK_51, CLOCK_53..CLOCK_57A and CLOCK_71 - see clocks.h for the exact list.
//
// Because a DS18B20 conversion takes up to ~750ms, readings are NOT taken with a blocking call.
// Instead a request/poll pattern is used: requestDallasTemp() kicks off a conversion, and getTemp()
// is called repeatedly (every ~200ms from timeProgram() in the core .ino) until DS_delay has elapsed,
// at which point the converted value is actually fetched from the sensor. See the two functions below.
// =================================================================================================
#ifdef USE_DALLAS_TEMP   //DS18B20 sensor

// Change HERE   0<>1, to swap sensors!!!
// NOTE: these two macros are not referenced anywhere else in this file (or the rest of the project) -
// looks like dead/leftover code; swapping sensor order today has to be done differently (see below).
#define SENSOR1 0
#define SENSOR2 1
#define TEMPERATURE_PRECISION 12   // DS18B20 resolution in bits (9-12); 12 bits = 0.0625C steps, ~750ms conversion time

#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(TEMP_DALLAS_PIN);        // Set up a OneWire instance to communicate with OneWire devices
DallasTemperature tempSensors(&oneWire); // Create an instance of the temperature sensor class
DeviceAddress thermometer1, thermometer2;  // 64-bit ROM addresses of sensor #1 / #2, read once in setupDallasTemp()
boolean requested = false;      // true while a conversion is in progress (request sent, result not yet fetched)
unsigned long lastRequest = millis();  // millis() timestamp of the last requestTemperatures() call
boolean DallasOK = true;   //is any measure from Dallas thermometer?
float lastTemperature = 0;      // NOTE: written nowhere, read nowhere else in the project - appears to be dead/unused
char lastTemperatureStr[5] = "----";   // NOTE: only ever written (see getTemp() below), never read back anywhere - looks unused/vestigial
unsigned long intervalTemp = 30000;      // Do a temperature measurement every 30sec
unsigned long DS_delay = 800;         // Reading the temperature from the DS18x20 can take up to 750ms

// Indexes into the shared temperature[]/humid[] arrays (declared in ESP32_UniClock2.ino) assigned to
// this driver's sensor(s) at setup time. Multiple sensor drivers (Dallas/DHT/I2C) share those arrays,
// each claiming the next free slot via the global useTemp counter - see setupDallasTemp() below.
byte d1Ptr = 0;
byte d2Ptr = 0;

void setupDallasTemp() {
  regPin(TEMP_DALLAS_PIN,"TEMP_DALLAS_PIN");
  pinMode(TEMP_DALLAS_PIN,OUTPUT);
  #if defined(ESP8266)
    oneWire.reset();   // ESP8266 needs an explicit bus reset before the first transaction; ESP32 does not
    delay(200);  //200ms
  #endif
  tempSensors.begin();                     // Start the temperature sensor
  tempSensors.setResolution(TEMPERATURE_PRECISION);
  tempSensors.setWaitForConversion(false); // Don't block the program while the temperature sensor is reading
  delay(100);  //100ms
  // Blocking probe done ONCE at boot: request a conversion and wait out the full 800ms here (setup()
  // isn't time-critical), just so we can count/enumerate the sensors below. During normal operation
  // (requestDallasTemp()/getTemp()) the same conversion is done non-blocking instead.
  tempSensors.requestTemperatures();
  delay(800);
  int counter = 0;
  while ((tempSensors.getDeviceCount() == 0) && (counter<5)) {
    if (counter>3) {
      DPRINT("No DS18x20 temperature sensor found on GPIO"); DPRINTLN(TEMP_DALLAS_PIN);
      resetSensors();
    }
    counter++;
    delay(100);
  }  //end while

  // useDallasTemp: 0 = no sensor, 1 = single sensor, 2 = two sensors on the same bus.
  // The two DS18B20s are NOT told apart by their ROM address/serial number - they are simply
  // enumerated by their scan order on the bus (DallasTemperature index 0 and index 1, see
  // getAddress()/getTempCByIndex() below), which is whatever order the library's internal 1-Wire
  // search finds them in. There is no user-facing way here to pin a physical sensor to a fixed
  // index; the SENSOR1/SENSOR2 macros above look like an abandoned attempt at that.
  useDallasTemp = tempSensors.getDeviceCount();
  if (useDallasTemp>0) {
    d1Ptr = useTemp;  //remember the pointers of sensors
    useTemp++;
    if (useDallasTemp>1) {
      d2Ptr = useTemp;
      useTemp++;
    }
    DPRINT("DS18B20 sensors found:"); DPRINTLN(useDallasTemp);
    if (!tempSensors.getAddress(thermometer1, 0)) DPRINTLN("Unable to find address for Device 0");
    if ((useDallasTemp>1) && !tempSensors.getAddress(thermometer2, 1)) DPRINTLN("Unable to find address for Device 1");
    // Kick off one more conversion right away (rather than waiting for the normal once-a-minute
    // trigger in requestDallasTemp()) so a real reading is available soon after boot.
    // POSSIBLE BUG: this only delays 200ms, but getTemp() below requires DS_delay (800ms) to have
    // elapsed since the request before it will actually fetch anything - with only 200ms elapsed,
    // getTemp() just returns immediately here without reading temperature[]. `requested` stays true,
    // so the real fetch only happens later, whenever timeProgram()'s periodic getTemp() call is
    // reached ~800ms after this request - i.e. this call does not actually pre-populate temperature[]
    // before setup() returns, despite appearances.
    requestDallasTemp(true);
    delay(200);
    getTemp();
  }
}

// Called every ~200ms from timeProgram() (with force=false). Does NOT block: requestTemperatures()
// with setWaitForConversion(false) (set in setupDallasTemp()) just tells the sensor(s) to start
// converting and returns immediately; the actual value is picked up later by getTemp() once enough
// time has passed. EEPROMsaving is checked because 1-Wire bus timing is sensitive to interrupts/delays,
// and EEPROMsaving also doubles as a "display refresh suspended" flag elsewhere in the project.
void requestDallasTemp(boolean force) {
    if (EEPROMsaving) return;

    // Old, no-longer-used scheme: re-request every intervalTemp (30s) regardless of clock time.
    //if (force || (!requested && ((millis()-lastRequest) > intervalTemp))) {       //request a new reading
    // Current scheme: instead of a fixed interval, a new conversion is requested once per minute,
    // exactly 1 second before prm.tempStart (the second-of-minute when the display starts showing the
    // temperature) - so by the time the display wants to show it, the ~750-800ms conversion has had
    // time to finish and getTemp() already has a fresh value ready.
    if (force || (!requested && (second()==prm.tempStart-1))) {       //request a new reading
      tempSensors.requestTemperatures(); // Request the temperature from the sensor (it takes some time to read it)
      requested = true;
      lastRequest = millis();
      DPRINTLN("Temperature requested");
      }
}

// Called every ~200ms from timeProgram() (and once directly from setupDallasTemp()). This is the
// other half of the async request/read pattern: it does nothing until DS_delay has elapsed since the
// matching requestDallasTemp() call, at which point the conversion is guaranteed finished and the
// (already-converted) value can be read from the sensor over 1-Wire without blocking/waiting.
void getTemp() {              //get the earlier requested temperature data
static int errors = 0;
float tmp_temp;

    if (!requested) return;   // no conversion pending - nothing to fetch yet
    if ((millis()- lastRequest) < DS_delay) return;  // 1000 ms after requesting the temperature
    // ^ comment says 1000ms but DS_delay is actually 800ms (see top of file) - conversion itself needs
    // up to ~750ms at 12-bit resolution, so 800ms leaves a small safety margin.
    if (EEPROMsaving) return;   // bus busy (flash write in progress) - try again on the next ~200ms tick

    requested = false;   // mark conversion consumed; a new request is needed before we read again
    if (tempSensors.getDeviceCount() == 0) {
          // Sensor(s) no longer answering on the bus (unplugged, wiring fault, bus glitch, etc.)
          // POSSIBLE BUG: temperature[d2Ptr] is zeroed unconditionally here, even when useDallasTemp<=1
          // (i.e. only one sensor was ever configured and d2Ptr was never assigned an index, so it is
          // still 0 - its C++ default). If some other sensor driver's reading happens to live at slot 0
          // and this Dallas sensor's own slot (d1Ptr) is not 0, this line would stomp on that other
          // sensor's temperature[] entry instead of (or in addition to) this one. Contrast with the
          // success path below, which correctly guards the second-sensor update with `if (useDallasTemp>1)`.
          temperature[d1Ptr] = 0; temperature[d2Ptr] = 0;
          DallasOK = false;
          resetSensors();
          DPRINTLN("Missing TEMP sensor!");
          // NOTE: decrementing lastRequest here has no actual effect on when the retry happens, because
          // the trigger condition in requestDallasTemp() is `second()==prm.tempStart-1`, a fixed point in
          // the minute, not a millis()-based interval check anymore (that scheme is the commented-out
          // line above). So despite the "retry ASAP!" comment, the next request still only fires at the
          // next occurrence of second()==prm.tempStart-1 (i.e. up to ~60s later), same as `errors` below.
          lastRequest -= intervalTemp;  //retry ASAP!
          }
    else {
          delay(10);
          tmp_temp = tempSensors.getTempCByIndex(0); // Get the temperature from the sensor
          // DallasTemperature reports -127C (DEVICE_DISCONNECTED_C) when a read fails - checked here
          // via "> -127" rather than comparing to the library's named constant.
          if (tmp_temp > -127) {
            tmp_temp = round(tmp_temp * 10.0) / 10.0; // round temperature to 1 digits
            DallasOK = true;
            // POSSIBLE BUG: writes to temperature[0] literally instead of temperature[d1Ptr] (used
            // everywhere else, e.g. the DPRINT right below and in the "sensor missing" branch above).
            // This happens to be equivalent today because setupDallasTemp() is always called first in
            // setup() (see ESP32_UniClock2.ino), so d1Ptr is always 0 - but it's fragile: if any other
            // sensor driver (DHT/I2C) were ever initialized before this one, or a config combined Dallas
            // with another sensor type such that useTemp>0 at the point setupDallasTemp() runs, this
            // sensor's reading would land in the wrong slot in temperature[], while d1Ptr would still
            // point at its correctly-reserved (but now unwritten) slot.
            temperature[0] = tmp_temp;
            DPRINT("Dallas#1 temp:"); DPRINTLN(temperature[d1Ptr]);
            dtostrf(temperature[0], 4, 1, lastTemperatureStr);   // formats temperature[0] into lastTemperatureStr, which (see top of file) is never read anywhere
            errors = 0;   // any successful read clears the consecutive-error counter
            if (useDallasTemp>1) {
              temperature[d2Ptr] = round(tempSensors.getTempCByIndex(1) * 10.0) / 10.0; // round temperature to 1 digits
              DPRINT("Dallas#2 temp:"); DPRINTLN(temperature[d2Ptr]);
            }
            }
          else {
            // Sensor answered on the bus but returned a bad/disconnected reading (-127C) this cycle.
            DPRINT("Temp sensor error!!!");
            DPRINTLN(tmp_temp);
            errors++;
            lastRequest -= intervalTemp;  //retry ASAP!  (see NOTE above - has no effect on the actual retry timing)
            resetSensors();
            }
    }
  // After 20 consecutive failed reads (~20 minutes, since retries are gated to once/minute), give up
  // on the sensor entirely so the UI/display stops trying to show a Dallas temperature.
  if (errors>20)
      useDallasTemp = 0; //forget BAD sensor!!!
}

// Attempts to recover a 1-Wire bus that has stopped responding (called from getTemp() whenever a
// sensor is missing or returns a bad reading). Blocking (delay(500) x2 = ~1s total), but it's only
// invoked on error paths, not on every normal read cycle, so it doesn't affect steady-state timing.
void resetSensors() {
  if (EEPROMsaving) return;
  DPRINTLN("Reset sensors...");
  //#if defined(ESP8266)
    oneWire.reset();
  //#endif
  delay(500);  //200ms

  tempSensors.begin(); //try to restart sensor
  tempSensors.setWaitForConversion(false);
  tempSensors.getDeviceCount();
  delay(500);  //200ms
}
//-----------------------------------------------------------------------------------------------------
#else
// USE_DALLAS_TEMP not defined for this CLOCK_NN config: compile out to no-op stubs so the rest of the
// codebase (setup()/timeProgram() in ESP32_UniClock2.ino) can call these unconditionally.
void setupDallasTemp() {}
void requestDallasTemp(boolean force) {}
void getTemp() {}
void resetSensors() {}
#endif

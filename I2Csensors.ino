// ============================================================================
// I2Csensors.ino
// ----------------------------------------------------------------------------
// Drivers for the I2C environmental sensors supported by this codebase.
// All sensors below share ONE physical I2C bus (Wire, PIN_SDA/PIN_SCL, set
// per-CLOCK_NN in clocks.h) and can be enabled in ANY combination for a given
// clock build - several CLOCK_NN configs enable 3-5 of them at once:
//
//   USE_BME280  - Bosch BME280, temperature + humidity + pressure (Adafruit_BME280_Library)
//                 I2C address 0x76 (hardcoded here; chip also supports 0x77)
//   USE_BMP280  - Bosch BMP280, temperature + pressure only        (Adafruit_BMP280_Library)
//                 I2C address 0x77 (hardcoded here; chip also supports 0x76)
//                 -> BME280 and BMP280 are deliberately pinned to different
//                    default addresses so both can coexist on the same bus.
//   USE_AHTX0   - AHT10/AHT20, temperature + humidity              (Adafruit_AHTX0)
//                 I2C address 0x38 (fixed, not configurable on the chip)
//   USE_SHT21   - Sensirion SHT21, temperature + humidity          (custom SHT21 lib)
//                 I2C address 0x40 (7-bit); see getSHT21()/detection code below
//   USE_BH1750  - Ambient light / lux meter                        (BH1750 lib)
//                 I2C address 0x23 (hardcoded; chip also supports 0x5C)
//
// Which CLOCK_NN configs use which sensor (see clocks.h):
//   USE_SHT21  alone            : CLOCK_23, CLOCK_23A, CLOCK_23B
//   BME280+BMP280+AHTX0+SHT21   : CLOCK_26
//   All five (incl. BH1750)     : CLOCK_46, CLOCK_48, CLOCK_50 (and, commented
//                                 out / inactive by default, CLOCK_51, 52, 54,
//                                 55, 56, 57, 57A), CLOCK_99
//   BME280+AHTX0+SHT21          : CLOCK_80, CLOCK_81, CLOCK_82
//
// USE_I2CSENSORS (the #ifdef guarding this whole file) is NOT set directly by
// any CLOCK_NN block - it is auto-derived in clocks.h:
//   #if defined(USE_BME280) || defined(USE_BMP280) || defined(USE_AHTX0) ||
//       defined(USE_SHT21) || defined(USE_BH1750) || defined(USE_RTC)
//     #define USE_I2CSENSORS
// i.e. it turns on automatically as soon as any one sensor (or the RTC) is
// requested for the selected clock.
//
// MULTI-SENSOR INDEXING: temperature[6]/humid[6]/pressur[6] (declared in the
// core .ino) are shared result arrays. Each sensor type that measures a given
// quantity claims the NEXT FREE slot in that quantity's array at boot, using
// three independent counters (useTemp/useHumid/usePress) - see the detailed
// comment in setupI2Csensors() below for exactly how collisions are avoided
// when several sensors are active simultaneously (e.g. CLOCK_26).
//
// Calibration offsets (prm.corrT0/corrT1/corrH0/corrH1) are NOT applied in
// this file - they are added later, at display time, in displayTemp()/
// displayHumid() in the core .ino. This file only stores raw (rounded)
// readings, so there is no risk of double-applying the correction here.
// ============================================================================
#ifdef USE_I2CSENSORS

// NOTE: these are commented-out and inert by design - the real per-build
// enabling of each sensor happens in clocks.h inside the relevant CLOCK_NN
// block. Since this .ino is compiled into every build regardless of which
// clock is selected, uncommenting one of these lines here would force that
// sensor ON for EVERY clock (as long as USE_I2CSENSORS ends up defined by
// some other means), not just the one you're working on. Leave commented;
// they exist only as a quick reference of the five supported macros.
//#define USE_BME280
//#define USE_BMP280
//#define USE_AHTX0
//#define USE_SHT21
//#define USE_BH1750

#include <Wire.h>
#include <SPI.h>

#ifdef USE_BME280
  #include <Adafruit_BME280.h>
  Adafruit_BME280 bme; // use I2C interface   address = 0x76 or 0x77
  Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
  Adafruit_Sensor *bme_pressure = bme.getPressureSensor();
  Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();
  // *Ptr variables below hold this sensor's assigned slot (index) in the
  // shared temperature[]/humid[]/pressur[] arrays. They are filled in once,
  // in setupI2Csensors(), and stay 0 (default global init) if the sensor
  // isn't found - harmless, since the getBME280() reader is only ever
  // called when BME280exist is true.
  byte BME280tempPtr;
  byte BME280humidPtr;
  byte BME280pressPtr;
#endif

#ifdef USE_BMP280
  #include <Adafruit_BMP280.h>
  // Hardcoded to 0x77 (BME280 above is hardcoded to 0x76) so that both chips
  // can be wired to the same bus at once without an address clash - several
  // CLOCK_NN configs (e.g. CLOCK_26, CLOCK_46/48/50, CLOCK_99) enable both
  // USE_BME280 and USE_BMP280 simultaneously.
  Adafruit_BMP280 bmp; // use I2C interface   address = 0x76 or 0x77
  Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
  Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();
  byte BMP280tempPtr;
  byte BMP280humidPtr;   // NOTE: BMP280 has no humidity sensor - this pointer is declared for symmetry but never used/assigned (BMP280 never touches humid[]).
  byte BMP280pressPtr;
#endif

#ifdef USE_AHTX0
  #include <Adafruit_AHTX0.h>     //I2C address: 0x38
  Adafruit_AHTX0 aht;
  Adafruit_Sensor *aht_humidity, *aht_temp;
  byte AHTX0tempPtr;
  byte AHTX0humidPtr;
#endif

#ifdef USE_SHT21
  // POSSIBLE BUG (see detection code in setupI2Csensors() below): this header
  // comment says the 8-bit write/read addresses are 0x80/0x81, which
  // correspond to 7-bit address 0x40 (the SHT21's actual, standard I2C
  // address). The Arduino Wire library used here takes a 7-bit address
  // (0-127), so 0x80 is out of range for Wire.beginTransmission() - see the
  // detection code further down which probes both 0x40 (correct) and 0x80
  // (likely meaningless/dead check for this Wire API).
  #include <SHT21.h>     //I2C address: 0x80 and 0x81
  SHT21 sht;
  float SHT21temp,SHT21humid =0;
  byte SHT21tempPtr;
  byte SHT21humidPtr;
#endif

#ifdef USE_BH1750
  #include <BH1750.h>
  BH1750 lightMeter(0x23);  // fixed address 0x23 (ADDR pin low/floating); chip's alternate address is 0x5C
#endif

void setupI2Csensors() {
  DPRINTLN("Starting I2Csensors sensors...");

  // pinMode(OUTPUT) here is just for regPin() bookkeeping (pin-conflict
  // registration/debug); Wire.begin() below reconfigures both pins for I2C
  // (open-drain + pull-ups) regardless of this pinMode call.
  pinMode(PIN_SDA,OUTPUT);  regPin(PIN_SDA,"PIN_SDA");
  pinMode(PIN_SCL,OUTPUT);  regPin(PIN_SCL,"PIN_SCL");
  Wire.begin(PIN_SDA,PIN_SCL);
  delay(100);
  I2Cscanner();  // debug-only bus scan; logs every ACKing address, does not affect detection below

#ifdef USE_RTC
  DPRINTLN("Starting RTC Clock...");    
  while(millis()<2000) {yield();}  //waiting for 2 sec from startup
  //I2C_ClearBus();
  delay(100);
  #if defined(PIN_SDA) && defined(PIN_SCL)
    Wire.beginTransmission(0x68);
    byte error = Wire.endTransmission();
    if (error == 0) {   //DS3231_get_addr(0x68)
      RTCexist = true;
      DPRINTLN("RTC found on 0x68.");
    }
    else { 
      DPRINTLN("!!!No RTC found on 0x68!!!");
    }
  #else
    RTCexist = false;
    DPRINTLN("PIN_SDA or PIN_SCL not defined!");
  #endif  
#endif


  // ---------------------------------------------------------------------
  // MULTI-SENSOR INDEXING SCHEME (applies to every #ifdef block below):
  // temperature[]/humid[]/pressur[] are shared 6-slot arrays that can hold
  // readings from several sensors at once. There are THREE INDEPENDENT
  // counters - useTemp, useHumid, usePress - one per array/quantity, each
  // incremented only by sensors that actually measure that quantity.
  // Each sensor, when found, "reserves" the array slot equal to the CURRENT
  // value of the relevant counter (its own <Sensor>xxxPtr), then increments
  // that counter so the NEXT detected sensor of that quantity gets the next
  // free slot. Because each array has its own counter, two sensors can
  // never collide/overwrite each other's slot in the SAME array, even when
  // several are active together (e.g. CLOCK_26: BME280+BMP280+AHTX0+SHT21):
  //   temperature[]: BME280->0, BMP280->1, AHTX0->2, SHT21->3  (useTemp=4)
  //   humid[]      : BME280->0,             AHTX0->1, SHT21->2 (useHumid=3, BMP280 has none)
  //   pressur[]    : BME280->0, BMP280->1                      (usePress=2, AHTX0/SHT21 have none)
  // Detection order below (BME280, BMP280, AHTX0, SHT21, BH1750) therefore
  // determines slot order. Downstream, only slots 0 and 1 of temperature[]/
  // humid[] receive a manual calibration offset (prm.corrT0/corrT1/corrH0/
  // corrH1, applied in displayTemp()/displayHumid() in the core .ino) - a
  // 3rd+ simultaneous temperature/humidity sensor (e.g. SHT21 at slot 2/3
  // above) is displayed uncorrected. That is a limitation of the core
  // display code, not of the indexing here, but worth knowing when wiring
  // up configs with 3+ temp/humid sensors.
  // ---------------------------------------------------------------------
#ifdef USE_BME280
  if (!bme.begin(0x76)) {
    DPRINTLN("Could not find a valid BME280 sensor on address 0x76, check wiring!");
  }
  else {
    BME280exist = true;
    DPRINTLN("BME280 Found!");
    bme_temp->printSensorDetails();
    bme_pressure->printSensorDetails();
    bme_humidity->printSensorDetails();
    BME280tempPtr = useTemp;  //remember my ID-s
    BME280humidPtr = useHumid;
    BME280pressPtr = usePress;
    useTemp++;   //increase the number of sensors
    useHumid++;
    usePress++;
  }
 #endif

#ifdef USE_BMP280  
  if (!bmp.begin(0x77)) {
    DPRINTLN("Could not find a valid BMP280 sensor on address 0x77, check wiring!");
  }
  else {
    BMP280exist = true;
    /* BMP280 default settings from datasheet. */
    DPRINTLN("BMP280 Found!");
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
    bmp_temp->printSensorDetails();
    BMP280tempPtr = useTemp;  //remember my ID-s
    BMP280pressPtr = usePress;  //remember my ID-s
    useTemp++;   //increase the number of sensors
    usePress++;   //increase the number of sensors
  }
#endif

#ifdef USE_AHTX0
  if (!aht.begin()) {
    DPRINTLN("Could not find a valid AHT10/AHT20 sensor, check wiring!");
  }
  else {
    AHTX0exist = true;
    DPRINTLN("AHT10/AHT20 Found!");
    aht_temp = aht.getTemperatureSensor();
    aht_temp->printSensorDetails();
    aht_humidity = aht.getHumiditySensor();
    aht_humidity->printSensorDetails();
    AHTX0tempPtr = useTemp;  //remember my ID-s
    AHTX0humidPtr = useHumid;
    useTemp++;   //increase the number of sensors
    useHumid++;    
  }
#endif  
  DPRINTLN(" ");

#ifdef USE_SHT21
  int error1,error2;
  Wire.beginTransmission(0x40);   // correct 7-bit SHT21 address
  error1 = Wire.endTransmission();
  // POSSIBLE BUG: 0x80 is not a valid 7-bit I2C address for Wire.beginTransmission()
  // (Wire expects 0-127; 0x80/128 is the 8-bit write address, i.e. 0x40<<1).
  // This second probe is likely a no-op/dead check left over from confusing
  // 7-bit vs 8-bit addressing (see comment above the #include <SHT21.h> line).
  // Harmless as written because of the "&&" below (SHT21exist still gets set
  // correctly whenever the real 0x40 probe succeeds), but error2 does not
  // reliably indicate anything about a real device.
  Wire.beginTransmission(0x80);
  error2 = Wire.endTransmission();
  if ((error1 !=0) && (error2!=0)) {
    DPRINTLN("Could not find a valid SHT21 sensor on 0x40 / 0x80, check wiring!");
  }
  else {
    SHT21exist = true;
    DPRINTLN("SHT21 Found!");
    SHT21tempPtr = useTemp;  //remember my ID-s
    SHT21humidPtr = useHumid;
    useTemp++;   //increase the number of sensors
    useHumid++;    
  }
#endif  

#ifdef USE_BH1750
  // BH1750 doesn't share temperature[]/humid[]/pressur[] - it has its own
  // dedicated useLux counter/flow (see getBH1750() at the bottom of this
  // file, called from getLightSensor() in the core .ino), so no slot/Ptr
  // bookkeeping is needed here.
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    DPRINTLN("BH1750 luxmeter sensor found");
    BH1750exist = true;
    useLux++;
  }
  else {
    DPRINTLN("Error initialising BH1750 on address 0x23");
  }
#endif

  DPRINTLN(" ");
} //end of SetupI2Csensors()


// getBME280()/getBMP280()/getAHTX0()/getSHT21() below each write ONLY into
// their own reserved slot (the *Ptr index assigned in setupI2Csensors(), see
// the indexing scheme comment there) and store the RAW rounded reading - no
// calibration offset (prm.corrT0/corrT1/corrH0/corrH1) is applied here; that
// happens later, at display time, in displayTemp()/displayHumid() (core
// .ino), so there's no double-correction between this file and that one.
void getBME280() {
#ifdef USE_BME280
  temperature[BME280tempPtr] = round1(bme.readTemperature());
  humid[BME280humidPtr] = round1(bme.readHumidity());
  pressur[BME280pressPtr] = round1(bme.readPressure()/100);
  DPRINT("BME280: ");
  DPRINT("Temperature = ");
  DPRINT(temperature[BME280tempPtr]);
  DPRINT(" *C");

  DPRINT("  Humidity = ");
  DPRINT(humid[BME280humidPtr]);
  DPRINT(" %");

  DPRINT("  Pressure = ");
  DPRINT(pressur[BME280pressPtr]);
  DPRINTLN(" hPa");
#endif
}


void getBMP280() {
#ifdef USE_BMP280  
  temperature[BMP280tempPtr] = round1(bmp.readTemperature());
  pressur[BMP280pressPtr] = round1(bmp.readPressure()/100);
  DPRINT("BMP280: ");
  DPRINT(F("Temperature = "));
  DPRINT(temperature[BMP280tempPtr]);
  DPRINT(" *C");
  
  DPRINT(F("  Pressure = "));
  DPRINT(pressur[BMP280pressPtr]);
  DPRINTLN(" hPa");
#endif
}

void getAHTX0() {
#ifdef USE_AHTX0
  //  /* Get a new normalized sensor event */
  sensors_event_t humidity;
  sensors_event_t temp;
  aht_humidity->getEvent(&humidity);
  aht_temp->getEvent(&temp);
  temperature[AHTX0tempPtr] = round1(temp.temperature);
  DPRINT("AHT sensor:  ");
  DPRINT("  Temperature ");
  DPRINT(temperature[AHTX0tempPtr]);
  DPRINT(" deg C");
  
  DPRINT("  Humidity: ");  /* Display the results (humidity is measured in % relative humidity (% rH) */
  humid[AHTX0humidPtr] = round1(humidity.relative_humidity);
  DPRINT(humid[AHTX0humidPtr]);
  DPRINTLN(" % rH");
#endif
}

void getSHT21() {
#ifdef USE_SHT21
  temperature[SHT21tempPtr] = round1(sht.getTemperature());
  DPRINT("SHT21 sensor:  ");
  DPRINT("  Temperature ");
  DPRINT(temperature[SHT21tempPtr]);
  DPRINT(" deg C");
  
  DPRINT("  Humidity: ");  /* Display the results (humidity is measured in % relative humidity (% rH) */
  humid[SHT21humidPtr] = round1(sht.getHumidity());
  DPRINT(humid[SHT21humidPtr]);
  DPRINTLN(" % rH");
#endif
}



void getI2Csensors() {
  static unsigned long lastRun = 0;

  // Called every ~200ms from timeProgram() (core .ino), but the actual I2C
  // reads only happen once every 30s per sensor - this throttle avoids
  // hammering the bus/sensors and is independent of the caller's cadence.
  // lastRun==0 lets the very first call (at millis()==0..~30000) through
  // immediately so readings are available right after boot.
  if (((millis()-lastRun)<30000) && (lastRun !=0)) return;  // || (second()!=TEMP_START)
  lastRun = millis();
  if (BME280exist) getBME280();
  if (BMP280exist) getBMP280();
  if (AHTX0exist) getAHTX0();
  if (SHT21exist) getSHT21();
  if (BH1750exist) getBH1750();
}


void I2Cscanner() {
  byte error, address;
  int nDevices=0;
  DPRINTLN(F("________________________________"));
  DPRINTLN("Scanning I2C devices...");

  for(address = 1; address < 127; address++ )   {
    // The i2c_scanner uses the return value of
    // Wire.endTransmission() to see if
    // a device acknowledged the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)  {
      DPRINT("I2C device found at address 0x");
      if (address<16) 
        DPRINT("0");
      DPRINT(address,HEX);
      DPRINTLN("  !");

      nDevices++;
    }
    else if (error==4) {
      DPRINT("Unknown error at address 0x");
      if (address<16) 
        DPRINT("0");
      DPRINTLN(address,HEX);
    }    
  }  //end for
  
  if (nDevices == 0)
    DPRINTLN("No I2C devices found\n");
  DPRINTLN(F("_________________________________"));
}


#else
void setupI2Csensors() {}
void getI2Csensors() {}
#endif

#ifdef USE_BH1750
// Called directly from getLightSensor() (core .ino) whenever BH1750exist is
// true - unlike the other get*() functions this is NOT gated through
// getI2Csensors()'s 30s throttle, so it can be polled as often as needed;
// the BH1750 library itself paces new measurements (measurementReady()).
int getBH1750() {
  static float oldLux = MAXIMUM_LUX;

  if (lightMeter.measurementReady()) {
    float lux = lightMeter.readLightLevel();
    //DPRINT("BH1750 Light: "); DPRINT(lux); DPRINTLN(" lx");

    if (lux>MAXIMUM_LUX) lux = MAXIMUM_LUX;   //Limited //Limit lux value to maximum
    if (lux<0) lux = oldLux;   // negative reading = bus glitch/error from the library; ignore and keep last good value
    oldLux = oldLux + (lux-oldLux)/10;   //slow down Lux change (simple exponential smoothing, ~10-sample time constant)
  }
  return (int)oldLux;
}
#else
// Stub used when USE_BH1750 is not defined for the selected clock, so
// getLightSensor() (core .ino) can call getBH1750() unconditionally.
int getBH1750() {
  return(0);
}
#endif

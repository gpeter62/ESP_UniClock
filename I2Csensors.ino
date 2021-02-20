//BME280, and AHT20+BMP280 sensor drivers
#ifdef USE_I2CSENSORS

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
  byte BME280tempPtr;
  byte BME280humidPtr;
  byte BME280pressPtr;
#endif

#ifdef USE_BMP280
  #include <Adafruit_BMP280.h>
  Adafruit_BMP280 bmp; // use I2C interface   address = 0x76 or 0x77
  Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
  Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();
  byte BMP280tempPtr;
  byte BMP280humidPtr;
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
  #include <SHT21.h>     //I2C address: 0x80 and 0x81
  SHT21 sht; 
  float SHT21temp,SHT21humid =0;
  byte SHT21tempPtr;
  byte SHT21humidPtr;
#endif

#ifdef USE_BH1750
  #include <BH1750.h>
  BH1750 lightMeter(0x23);
#endif

void setupI2Csensors() {
  DPRINTLN("Starting I2Csensors sensors...");    
  
  pinMode(PIN_SDA,OUTPUT);  regPin(PIN_SDA,"PIN_SDA");
  pinMode(PIN_SCL,OUTPUT);  regPin(PIN_SCL,"PIN_SCL");
  Wire.begin(PIN_SDA,PIN_SCL); 
  delay(100);
  I2Cscanner();
  
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
  Wire.beginTransmission(0x40);
  error1 = Wire.endTransmission();
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
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    DPRINTLN("BH1750 luxmeter sensor found");
    BH1750exist = true;
    autoBrightness = true;
  }
  else {
    DPRINTLN("Error initialising BH1750 on address 0x23");
  }
#endif

  DPRINTLN(" ");
} //end of SetupI2Csensors()


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

  if ((((millis()-lastRun)<2500) || (second()!=TEMP_START)) && (lastRun !=0)) return;
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
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
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
int getBH1750() {
  static float oldLux = MAXIMUM_LUX;
  
  if (lightMeter.measurementReady()) {
    float lux = lightMeter.readLightLevel();
    DPRINT("BH1750 Light: "); DPRINT(lux); DPRINTLN(" lx");
    
    if (lux>MAXIMUM_LUX) lux = MAXIMUM_LUX;   //Limited //Limit lux value to maximum
    oldLux = oldLux + (lux-oldLux)/10;   //slow down Lux change
  }
  return (int)oldLux;
}
#else
int getBH1750() {
  return(0);
}  
#endif

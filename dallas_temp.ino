#ifdef USE_DALLAS_TEMP

// Change HERE   0<>1, to swap sensors!!!
#define SENSOR1 0
#define SENSOR2 1
#define TEMPERATURE_PRECISION 12

#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(TEMP_SENSOR_PIN);        // Set up a OneWire instance to communicate with OneWire devices
DallasTemperature tempSensors(&oneWire); // Create an instance of the temperature sensor class
DeviceAddress thermometer1, thermometer2;
boolean requested = false;
unsigned long lastRequest = millis();
boolean DallasOK = true;   //is any measure from Dallas thermometer?
float lastTemperature = 0;
char lastTemperatureStr[5] = "----";
const long intervalTemp = 30000;      // Do a temperature measurement every 30sec
const long DS_delay = 1000;         // Reading the temperature from the DS18x20 can take up to 750ms

void setupTemp() {
  DPRINT("Starting Dallas thermometer on GPIO");  DPRINTLN(TEMP_SENSOR_PIN);
  //pinMode(TEMP_SENSOR_PIN,OUTPUT);
  //#if defined(ESP8266)
    oneWire.reset();
    delay(200);  //200ms
  //#endif  
  tempSensors.begin();                     // Start the temperature sensor  
  tempSensors.setResolution(TEMPERATURE_PRECISION);
  tempSensors.setWaitForConversion(false); // Don't block the program while the temperature sensor is reading
  delay(100);  //100ms
  
  int counter = 0;
  while ((tempSensors.getDeviceCount() == 0) && (counter<10)) {
    DPRINT("No DS18x20 temperature sensor found on GPIO"); DPRINTLN(TEMP_SENSOR_PIN); 
    resetSensors();
    counter++;
  }  //end while
  
  useTemp = tempSensors.getDeviceCount();
  if (useTemp>0) {
  DPRINT("DS18B20 sensors found:"); DPRINTLN(useTemp);
  if (!tempSensors.getAddress(thermometer1, 0)) DPRINTLN("Unable to find address for Device 0");
  if ((useTemp>1) && !tempSensors.getAddress(thermometer2, 1)) DPRINTLN("Unable to find address for Device 1");
  requestTemp(true);
  delay(200);
  getTemp();
  }
}

void requestTemp(boolean force) {
    if (force || (!requested && ((millis()-lastRequest) > intervalTemp))) {       //request a new reading
      tempSensors.requestTemperatures(); // Request the temperature from the sensor (it takes some time to read it)
      requested = true;
      lastRequest = millis();
      DPRINTLN("Temperature requested");
      }
}

void getTemp() {              //get the earlier requested temperature data
static int errors = 0;  
float tmp_temp;

    if (!requested) return;
    if ((millis()- lastRequest) < DS_delay) return;  // 1000 ms after requesting the temperature
    //DPRINTLN("getTemp");
    requested = false;
    if (tempSensors.getDeviceCount() == 0) {
          temperature[0] = 0; temperature[1] = 0;
          DallasOK = false;
          resetSensors();
          DPRINTLN("Missing TEMP sensor!");
          lastRequest -= intervalTemp;  //retry ASAP!
          }
    else {    
          delay(10);
          tmp_temp = tempSensors.getTempCByIndex(0); // Get the temperature from the sensor
          if (tmp_temp > -127) {
            tmp_temp = round(tmp_temp * 10.0) / 10.0; // round temperature to 1 digits
            DallasOK = true;
            temperature[0] = tmp_temp;     
            DPRINT("Dallas#1 temp:"); DPRINTLN(temperature[0]);
            dtostrf(temperature[0], 4, 1, lastTemperatureStr);
            errors = 0;
            if (useTemp>1) {
              temperature[1] = round(tempSensors.getTempCByIndex(1) * 10.0) / 10.0; // round temperature to 1 digits
              DPRINT("Dallas#2 temp:"); DPRINTLN(temperature[1]);
            }
            }
          else {
            DPRINT("Temp sensor error!!!");  
            DPRINTLN(tmp_temp);
            errors++;
            lastRequest -= intervalTemp;  //retry ASAP!
            resetSensors();
            }
    }
  if (errors>20) 
      useTemp = 0; //forget BAD sensor!!! 
}

void resetSensors() {

  DPRINTLN("Reset sensors...");
  //#if defined(ESP8266)
    oneWire.reset();
  //#endif
  delay(200);  //200ms

  tempSensors.begin(); //try to restart sensor
  tempSensors.setWaitForConversion(false);
  tempSensors.getDeviceCount();
  delay(200);  //200ms
}
//-----------------------------------------------------------------------------------------------------
#else
void setupTemp() {  useTemp = 0;}
void requestTemp(boolean force) {}
void getTemp() {}
void resetSensors() {}
#endif

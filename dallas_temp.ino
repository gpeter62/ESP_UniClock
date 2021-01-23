#ifdef USE_DALLAS_TEMP   //DS18B20 sensor

// Change HERE   0<>1, to swap sensors!!!
#define SENSOR1 0
#define SENSOR2 1
#define TEMPERATURE_PRECISION 12

#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(TEMP_DALLAS_PIN);        // Set up a OneWire instance to communicate with OneWire devices
DallasTemperature tempSensors(&oneWire); // Create an instance of the temperature sensor class
DeviceAddress thermometer1, thermometer2;
boolean requested = false;
unsigned long lastRequest = millis();
boolean DallasOK = true;   //is any measure from Dallas thermometer?
float lastTemperature = 0;
char lastTemperatureStr[5] = "----";
const long intervalTemp = 30000;      // Do a temperature measurement every 30sec
const long DS_delay = 800;         // Reading the temperature from the DS18x20 can take up to 750ms

byte d1Ptr = 0;
byte d2Ptr = 0;

void setupDallasTemp() {
  regPin(TEMP_DALLAS_PIN,"TEMP_DALLAS_PIN"); 
  //pinMode(TEMP_DALLAS_PIN,OUTPUT);
  //#if defined(ESP8266)
    oneWire.reset();
    delay(200);  //200ms
  //#endif  
  tempSensors.begin();                     // Start the temperature sensor  
  tempSensors.setResolution(TEMPERATURE_PRECISION);
  tempSensors.setWaitForConversion(false); // Don't block the program while the temperature sensor is reading
  delay(100);  //100ms
  tempSensors.requestTemperatures();
  delay(800);
  int counter = 0;
  while ((tempSensors.getDeviceCount() == 0) && (counter<10)) {
    if (counter>2) {
      DPRINT("No DS18x20 temperature sensor found on GPIO"); DPRINTLN(TEMP_DALLAS_PIN); 
     resetSensors();
    }
    counter++;
    delay(100);
  }  //end while
  
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
    requestDallasTemp(true);
    delay(200);
    getTemp();
  }
}

void requestDallasTemp(boolean force) {
    if (EEPROMsaving) return;
    
    //if (force || (!requested && ((millis()-lastRequest) > intervalTemp))) {       //request a new reading
    if (force || (!requested && (second()==TEMP_START-1))) {       //request a new reading
      //disableDisplay();
      tempSensors.requestTemperatures(); // Request the temperature from the sensor (it takes some time to read it)
      //enableDisplay(0);
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
    if (EEPROMsaving) return;

    requested = false;
    if (tempSensors.getDeviceCount() == 0) {
          temperature[d1Ptr] = 0; temperature[d2Ptr] = 0;
          DallasOK = false;
          resetSensors();
          DPRINTLN("Missing TEMP sensor!");
          lastRequest -= intervalTemp;  //retry ASAP!
          }
    else {    
          delay(10);
          //disableDisplay();
          tmp_temp = tempSensors.getTempCByIndex(0); // Get the temperature from the sensor
          //enableDisplay(0);
          if (tmp_temp > -127) {
            tmp_temp = round(tmp_temp * 10.0) / 10.0; // round temperature to 1 digits
            DallasOK = true;
            temperature[0] = tmp_temp;     
            DPRINT("Dallas#1 temp:"); DPRINTLN(temperature[d1Ptr]);
            dtostrf(temperature[0], 4, 1, lastTemperatureStr);
            errors = 0;
            if (useDallasTemp>1) {
              //disableDisplay();
              temperature[d2Ptr] = round(tempSensors.getTempCByIndex(1) * 10.0) / 10.0; // round temperature to 1 digits
              //enableDisplay(0);
              DPRINT("Dallas#2 temp:"); DPRINTLN(temperature[d2Ptr]);
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
      useDallasTemp = 0; //forget BAD sensor!!! 
}

void resetSensors() {
   if (EEPROMsaving) return;
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
void setupDallasTemp() {}
void requestDallasTemp(boolean force) {}
void getTemp() {}
void resetSensors() {}
#endif

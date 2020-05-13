#ifdef USE_DALLAS_TEMP

#include <DallasTemperature.h>
OneWire oneWire(TEMP_SENSOR_PIN);        // Set up a OneWire instance to communicate with OneWire devices
DallasTemperature tempSensors(&oneWire); // Create an instance of the temperature sensor class
boolean requested = false;
unsigned long lastRequest = millis();
boolean DallasOK = true;   //is any measure from Dallas thermometer?
int resolution = 11;
float lastTemperature = 0;
char lastTemperatureStr[5] = "----";
const long intervalTemp = 30000;      // Do a temperature measurement every 30sec
const long DS_delay = 1000;         // Reading the temperature from the DS18x20 can take up to 750ms

void setupTemp() {
  DPRINT("Starting Dallas thermometer on GPIO");  DPRINTLN(TEMP_SENSOR_PIN);
  pinMode(TEMP_SENSOR_PIN,OUTPUT);
  oneWire.reset();
  delay(200 );  //200ms

  tempSensors.setResolution(resolution);
  tempSensors.setWaitForConversion(false); // Don't block the program while the temperature sensor is reading
  tempSensors.begin();                     // Start the temperature sensor  
  delay(100);  //200ms
  
  int counter = 0;
  while ((tempSensors.getDeviceCount() == 0) && (counter<10)) {
    DPRINT("No DS18x20 temperature sensor found on GPIO"); DPRINTLN(TEMP_SENSOR_PIN); 
    resetSensors();
    counter++;
  }  //end while
  
  useDallasTemp = (tempSensors.getDeviceCount() > 0) ;
  if (useDallasTemp) {  //exist valid temp sensor
  DPRINT("DS18B20 sensors found:"); DPRINTLN(tempSensors.getDeviceCount());
  requestTemp(true);
  delay(1000);
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
    
    requested = false;
    if (tempSensors.getDeviceCount() == 0) {
          dallasTemp = 0; 
          DallasOK = false;
          resetSensors();
          DPRINTLN("Missing TEMP sensor!");
          lastRequest -= intervalTemp;  //retry ASAP!
          }
    else {    
          delay(10);
          tmp_temp = tempSensors.getTempCByIndex(0); // Get the temperature from the sensor
          if (tmp_temp != -127.00) tmp_temp = tempSensors.getTempCByIndex(0); // Get the temperature from the sensor
          tmp_temp = round(tmp_temp * 10.0) / 10.0; // round temperature to 1 digits
          if (tmp_temp != -127.00) {
            DallasOK = true;
            dallasTemp = tmp_temp+0.5f;
            DPRINT("Dallas temp:"); DPRINTLN(dallasTemp);
            dtostrf(dallasTemp, 4, 1, lastTemperatureStr);
            errors = 0;
            }
          else {
            DPRINT("Temp sensor error!!!");  
            DPRINTLN(tmp_temp);
            errors++;
            lastRequest -= intervalTemp;  //retry ASAP!
            //resetSensors();
            }
    }
  if (errors>20) 
      useDallasTemp = false; //forget BAD sensor!!! 
}

void resetSensors() {
  DPRINTLN("Reset sensors...");
  oneWire.reset();
  delay(200);  //200ms

  tempSensors.begin(); //try to restart sensor
  tempSensors.setWaitForConversion(false);
  tempSensors.getDeviceCount();
  delay(200);  //200ms

}
//-----------------------------------------------------------------------------------------------------
#else
void setupTemp() {  useDallasTemp = false;}
void requestTemp(boolean force) {}
void getTemp() {}
void resetSensors() {}
#endif

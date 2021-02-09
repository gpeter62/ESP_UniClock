#ifdef USE_MQTT

#include <ArduinoHA.h>   //https://pubsubclient.knolleary.net/     https://github.com/knolleary/pubsubclient
//#include <WiFi.h>
#define BROKER_ADDR         IPAddress(10,10,0,202)
#define BROKER_USERNAME     "mqtt" // replace with your credentials
#define BROKER_PASSWORD     "mqtt"

byte mac[6] = {12,14,0,0,2,3};  //This is the unique ID - set it as you want or mac address of the clock (length: 6)
WiFiClient client;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);

HASensor<float> mqttTemp("temperature", 0, mqtt);
HASensor<float> mqttHumid("humidity", 0, mqtt);
HASensor<float> mqttPress("pressure", 0, mqtt);


void setupMqtt() {
    // set device's details (optional)
    device.setName("UniClock32");
    device.setSoftwareVersion("2.5");
    mqttTemp.setUnitOfMeasurement("°C");
    mqttHumid.setUnitOfMeasurement("%");
    mqttPress.setUnitOfMeasurement("hPa");
    DPRINTLN("Attempting MQTT connection...");
    mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);
}

void mqttSend() {
  static unsigned long lastRun = millis();
  
  mqtt.loop();
    
  if ((millis() - lastRun) < 30000) return;
  lastRun = millis();  
  
  DPRINT("MQTT send:");
  mqttTemp.setValue(temperature[0]);  DPRINT(temperature[0]); DPRINT(",");
  mqttHumid.setValue(humid[0]);  DPRINT(humid[0]); DPRINT(",");
  mqttPress.setValue(pressur[0]);  DPRINT(pressur[0]); 
  DPRINTLN(" ");
}
  
#else
  void mqttSend() {}
#endif

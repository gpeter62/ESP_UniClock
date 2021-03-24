#ifdef USE_MQTT

#include <ArduinoHA.h>   //https://pubsubclient.knolleary.net/     https://github.com/knolleary/pubsubclient

#define BROKER_ADDR      IPAddress(10,10,0,202) //moved to prm.mqttBrokerAddr
#define BROKER_USERNAME  "mqtt"        //moved to prm.mqttBrokerUser
#define BROKER_PASSWORD  "mqtt"        //moved to mqttBrokerPsw

byte mac[6];// = {12,14,0,0,2,3};  //This is the unique ID - set it as you want or mac address of the clock (length: 6)

WiFiClient client;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);

HASensor<float> mqttTemp("temperature", 0, mqtt);
HASensor<float> mqttHumid("humidity", 0, mqtt);
HASensor<float> mqttPress("pressure", 0, mqtt);
HASensor<float> mqttLux("lux", 0, mqtt);

void setupMqtt() {
    char s[30];
    // set device's details (optional)
    WiFi.macAddress(mac);   //get the ESP board's MAC address
    device.setName("UniClock32");
    device.setSoftwareVersion("3.0");
    if (useTemp>0) mqttTemp.setUnitOfMeasurement("°C");
    if (useHumid>0) mqttHumid.setUnitOfMeasurement("%");
    if (usePress>0) mqttPress.setUnitOfMeasurement("hPa");
    if (useLux>0) mqttLux.setUnitOfMeasurement("lux");

    IPAddress tmp;
    tmp.fromString(String(prm.mqttBrokerAddr));
    DPRINT("Attempting MQTT connection. IP:"); DPRINT(tmp);
    sprintf(s,"  Clock's MAC:%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    DPRINTLN(s);
    mqtt.begin(tmp, prm.mqttBrokerUser, prm.mqttBrokerPsw);
    //mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);
}

void mqttSend() {
  static unsigned long lastRun = millis();
  
  mqtt.loop();
    
  if ((millis() - lastRun) < prm.mqttBrokerRefresh*1000) return;
  lastRun = millis();  
  
  DPRINT("MQTT send:");
  if (useTemp>0)  {mqttTemp.setValue(temperature[0]);  DPRINT(temperature[0]); DPRINT(",");}
  if (useHumid>0) {mqttHumid.setValue(humid[0]);  DPRINT(humid[0]); DPRINT(",");}
  if (usePress>0) {mqttPress.setValue(pressur[0]);  DPRINT(pressur[0]);  DPRINT(",");}
  if (useLux>0)   {mqttLux.setValue(lx);   DPRINT(lx);  DPRINTLN(" ");}
}
  
#else
  void mqttSend() {}
#endif

#ifdef USE_MQTT

#include <ArduinoHA.h>   //https://github.com/dawidchyrzynski/arduino-home-assistant
//https://pubsubclient.knolleary.net/     https://github.com/knolleary/pubsubclient

//#define BROKER_ADDR      IPAddress(192,168,1,241) //moved to prm.mqttBrokerAddr
//#define BROKER_USERNAME  ""        //moved to prm.mqttBrokerUser
//#define BROKER_PASSWORD  ""        //moved to mqttBrokerPsw

//byte mac[6] = {0x3e,0x09,0x0b,0x3e,0x09,0x0b}; //in clocks.h, this is the unique ID - set it as you want or mac address of the clock (length: 6)
//To subscribe to my sensors: (You can check it with Chrome browser + MQTTlens addon)  MAC_ADDRESS: for example: 3e090b3e090b
// homeassistant/sensor/3e090b3e090b/temperature/value
// homeassistant/sensor/3e090b3e090b/humidity/value
// homeassistant/sensor/3e090b3e090b/pressure/value
// homeassistant/sensor/3e090b3e090b/lux/value
#define MQTT_PREFIX "UniClock32"

//#define USE_MASTER_CLOCK  //if used, this will be the sensor#0
//#define USE_MASTER_TEMP
//#define USE_MASTER_HUMID

char masterTemperatureTopic[] = "homeassistant/sensor/10521c5e14c4/temperature/state";
char masterHumidityTopic[] =    "homeassistant/sensor/10521c5e14c4/humidity/state";
char masterRadarTopic[] =       "homeassistant/sensor/10521c5e14c4/radar/state";
byte MASTERtempPtr=0;
byte MASTERhumidPtr=0;

WiFiClient client;
HADevice device(mac, sizeof(mac));
HAMqtt mqtt(client, device);

HASensor mqttTemp("temperature");
HASensor mqttHumid("humidity");
HASensor mqttPress("pressure");
HASensor mqttLux("lux");
HASensor mqttRadar("radar");

void onMqttMessage(const char* topic, const uint8_t* payload, uint16_t length) {
  float tempTMP,humidTMP;
  char tmp[100];
    // This callback is called when message from MQTT broker is received.
    // Please note that you should always verify if the message's topic is the one you expect.
    // For example: if (memcmp(topic, "myCustomTopic") == 0) { ... }
    
    DPRINT("New message on topic: ");    DPRINT(topic);

    memcpy(tmp,payload,length);
    DPRINT("  Data: ");    DPRINTLN(tmp);
    if (strncmp(topic,masterTemperatureTopic,sizeof(masterTemperatureTopic)) ==0) {
      tempTMP = atof(tmp);
      temperature[MASTERtempPtr] = tempTMP;
      DPRINT("MASTER clock's temperature:"); DPRINTLN(tempTMP);
    }
    if (strncmp(topic,masterHumidityTopic,sizeof(masterHumidityTopic)) ==0) {
      humidTMP = atof(tmp);
      DPRINT("MASTER clock's humidity:"); DPRINTLN(humidTMP);
      humid[MASTERhumidPtr] = humidTMP;
    }
    if (strncmp(topic,masterRadarTopic,sizeof(masterRadarTopic)) ==0) {
       DPRINT("MASTER radar:"); DPRINTLN(tmp[0]);
       if (tmp[0] == '0')  {
         mqttRadarON = false;
         radarON = mqttRadarON;
       }
       else {
         mqttRadarON = true;
         radarON = mqttRadarON;
         radarLastOn = millis();
       }
       DPRINT("mqttRadarON:"); DPRINTLN(mqttRadarON);
 
    }
    //mqtt.publish("myPublishTopic", "hello");
}

void onMqttConnected() {
    DPRINTLN("Connected to the broker!");

    // You can subscribe to custom topic if you need
    #ifdef USE_MASTER_CLOCK
      DPRINTLN(F("MASTER_CLOCK enabled! Subscribe to topics:"));
      #ifdef USE_MASTER_TEMP
        mqtt.subscribe(masterTemperatureTopic);
        DPRINTLN(masterTemperatureTopic);
      #endif
      #ifdef USE_MASTER_HUMID
        mqtt.subscribe(masterHumidityTopic);
        DPRINTLN(masterHumidityTopic);
      #endif
      if (RADAR_PIN>=0) {
         DPRINTLN(F("Local radar enabled! MQTT MASTER radar disabled!"));  
      }
      else {
        DPRINTLN(masterRadarTopic);
        mqtt.subscribe(masterRadarTopic);
      }
    #endif  
}

void setupMqtt() {
  char mactmp[20];
    sprintf(mactmp,"%02x%02x%02x%02x%02x%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    
    DPRINT("Starting MQTT CLient: "); DPRINT(MQTT_PREFIX); DPRINT("/sensor/"); DPRINTLN(mactmp);
    mqtt.setDiscoveryPrefix(MQTT_PREFIX);
    device.setName(FW);
    device.setSoftwareVersion("3.0");
    if (useTemp>0) {
      mqttTemp.setUnitOfMeasurement("°C");
      mqttTemp.setDeviceClass("sensor");
      mqttTemp.setName("Inside temperature");
    }
    if (useHumid>0) {
      mqttHumid.setUnitOfMeasurement("%");
      mqttHumid.setDeviceClass("sensor");
      mqttHumid.setName("Inside humidity");
    }
    if (usePress>0) {
      mqttPress.setUnitOfMeasurement("hPa");
      mqttPress.setDeviceClass("sensor");
      mqttPress.setName("Air pressure");
    }
    if (useLux>0) {
      mqttLux.setUnitOfMeasurement("lux");
      mqttLux.setDeviceClass("sensor");
      mqttLux.setName("Inside lux");
    }
    #if RADAR_PIN>=0
      mqttRadar.setUnitOfMeasurement("on/off");
      mqttRadar.setDeviceClass("sensor");
      mqttRadar.setName("Radar sensor");
    #endif

    mqtt.onMessage(onMqttMessage);
    mqtt.onConnected(onMqttConnected);
    
    IPAddress tmp;
    tmp.fromString(String(prm.mqttBrokerAddr));
    if (strlen(prm.mqttBrokerUser)==0)
      mqtt.begin(tmp);
    else  
      mqtt.begin(tmp, prm.mqttBrokerUser, prm.mqttBrokerPsw);
      
    //mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);

    device.enableLastWill();
    #ifdef USE_MASTER_CLOCK
      //DPRINTLN("MQTT MASTER_CLOCK added as temperature&humidity sensor!");
      #ifdef USE_MASTER_TEMP
        MASTERtempPtr = useTemp;  //remember my ID-s
         useTemp++;   //increase the number of sensors
      #endif
      #ifdef USE_MASTER_TEMP  
        MASTERhumidPtr = useHumid;
        useHumid++;
      #endif  
    #endif
}

void mqttSend() {
  static unsigned long lastRun = millis();
  //if (!mqtt.isConnected()) return;
  mqtt.loop();
    
  if ((millis() - lastRun) < prm.mqttBrokerRefresh*1000) return;
  lastRun = millis();  
  if (mqtt.isConnected()) DPRINT("MQTT Connected."); 
  else {
    DPRINTLN("MQTT Failed to connect.");
    return;
  }
  DPRINT("MQTT send:");
  if (useTemp>0)  {mqttTemp.setValue(temperature[0]+ prm.corrT0);  DPRINT(temperature[0]+ prm.corrT0); DPRINT(", ");}
  if (useHumid>0) {mqttHumid.setValue(humid[0]+ prm.corrH0);  DPRINT(humid[0]+ prm.corrH0); DPRINT(", ");}
  if (usePress>0) {mqttPress.setValue(pressur[0]);  DPRINT(pressur[0]);  DPRINT(", ");}
  if (useLux>0)   {mqttLux.setValue(lx);   DPRINT(lx);  DPRINT(", ");}
  #if RADAR_PIN>=0
    mqttRadar.setValue(radarON);   DPRINT(radarON);
  #endif
  DPRINTLN(" ");
}
  
#else
  void mqttSend() {}
#endif

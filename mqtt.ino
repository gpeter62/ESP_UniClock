#ifdef USE_MQTT
//Clock can receive data from a MASTER_CLOCK and send data to other clocks
//_______________________________________________________________________________________________________________________
//To use MQTT services the following settings should be defined in clocks.h
//#define USE_MQTT
//#define MQTT_PREFIX "UniClock32"
//#define USE_MASTER_CLOCK  //enable it, if you want to get any data from MASTER CLOCK. This will be the sensor#0
//#define USE_MASTER_TEMP   //enable it, if you want to get temperature from MASTER CLOCK
//#define USE_MASTER_HUMID  //enable it, if you want to get humidity from MASTER CLOCK
//#define USE_MASTER_RADAR  //enable it, if you want to get radar from MASTER CLOCK
//#define MASTER_TEMPERATURE_TOPIC "homeassistant/sensor/10521c5e14c4/temperature/state"
//#define MASTER_HUMIDITY_TOPIC    "homeassistant/sensor/10521c5e14c4/humidity/state"
//#define MASTER_RADAR_TOPIC       "homeassistant/sensor/10521c5e14c4/radar/state"
//_______________________________________________________________________________________________________________________

//To subscribe to my sensors: You can check it with Chrome browser + MQTTlens addon)  MAC_ADDRESS: for example: 3e090b3e090b
// homeassistant/sensor/3e090b3e090b/temperature/value
// homeassistant/sensor/3e090b3e090b/humidity/value
// homeassistant/sensor/3e090b3e090b/pressure/value
// homeassistant/sensor/3e090b3e090b/lux/value

// old solution, now moved to prm.mqttBrokerAddr, available on clock's web page
//#define BROKER_ADDR      IPAddress(192,168,1,241) 
//#define BROKER_USERNAME  ""        //moved to prm.mqttBrokerUser
//#define BROKER_PASSWORD  ""        //moved to mqttBrokerPsw
//_______________________________________________________________________________________________________________________

#include <ArduinoHA.h>   //https://github.com/dawidchyrzynski/arduino-home-assistant
//https://pubsubclient.knolleary.net/     https://github.com/knolleary/pubsubclient

#if !defined(MQTT_PREFIX)
  #define MQTT_PREFIX "UniClock32"
#endif

byte mac[6];
char masterTemperatureTopic[80] = "";
char masterHumidityTopic[80] = "";
char masterRadarTopic[80] = "";

byte MASTERtempPtr=0;
byte MASTERhumidPtr=0;

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
//Sensors of local clock, to send data:
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
    if ((strlen(masterTemperatureTopic)>0) && (strncmp(topic,masterTemperatureTopic,sizeof(masterTemperatureTopic)) ==0)) {
      tempTMP = atof(tmp);
      temperature[MASTERtempPtr] = tempTMP;
      DPRINT("MASTER clock's temperature:"); DPRINTLN(tempTMP);
    }
    if ((strlen(masterHumidityTopic)>0) && (strncmp(topic,masterHumidityTopic,sizeof(masterHumidityTopic)) ==0)) {
      humidTMP = atof(tmp);
      DPRINT("MASTER clock's humidity:"); DPRINTLN(humidTMP);
      humid[MASTERhumidPtr] = humidTMP;
    }
    if ((strlen(masterRadarTopic)>0) && (strncmp(topic,masterRadarTopic,sizeof(masterRadarTopic)) ==0)) {
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
      DPRINTLN(F("MASTER_CLOCK enabled! Subscribe to following topics:"));
      #ifdef USE_MASTER_TEMP
        if (strlen(masterTemperatureTopic)==0) {
            DPRINTLN("Please define MASTER_TEMPERATURE_TOPIC in clocks.h'");
        }
        else {
          DPRINT(F("Master temperature topic:"));  DPRINTLN(masterTemperatureTopic);
          mqtt.subscribe(masterTemperatureTopic);
        }
      #endif
      #ifdef USE_MASTER_HUMID
      if (strlen(masterHumidityTopic)==0) {
            DPRINTLN("Please define MASTER_HUMIDITY_TOPIC in clocks.h'");
        }
        else {
          DPRINT(F("Master humidity topic:"));  DPRINTLN(masterHumidityTopic);
          mqtt.subscribe(masterHumidityTopic);
        }
      #endif
      #ifdef USE_MASTER_RADAR
        if (RADAR_PIN>=0) {
         DPRINTLN(F("Local radar enabled! MQTT MASTER radar disabled!"));  
        }
        else {
          if (strlen(masterRadarTopic)==0) {
            DPRINTLN("Please define MASTER_RADAR_TOPIC in clocks.h'");
          }
          else {
            DPRINT(F("Master radar topic:"));  DPRINTLN(masterRadarTopic);
            mqtt.subscribe(masterRadarTopic);
          }
        }
      #endif
    #endif  
}

void setupMqtt() {
  char mactmp[20];

    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));  //set device MAC, as unique ID
    sprintf(mactmp,"%02x%02x%02x%02x%02x%02x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);    
    #if defined(MASTER_TEMPERATURE_TOPIC)
      strncpy(masterTemperatureTopic,MASTER_TEMPERATURE_TOPIC,sizeof(masterTemperatureTopic));
    #endif
    #if defined(MASTER_HUMIDITY_TOPIC)
      strncpy(masterHumidityTopic,MASTER_HUMIDITY_TOPIC,sizeof(masterHumidityTopic));
    #endif  
    #if defined(MASTER_RADAR_TOPIC)
      strncpy(masterRadarTopic,MASTER_RADAR_TOPIC,sizeof(masterRadarTopic));
    #endif
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
      DPRINTLN("MQTT MASTER_CLOCK enabled!");
      DPRINT("MASTER_TEMPERATURE_TOPIC:"); DPRINTLN(masterTemperatureTopic);
      DPRINT("MASTER_HUMIDITY_TOPIC:"); DPRINTLN(masterHumidityTopic);
      DPRINT("MASTER_RADAR_TOPIC:"); DPRINTLN(masterRadarTopic);
      #ifdef USE_MASTER_TEMP
        DPRINTLN("  - MQTT MASTER_TEMP sensor added!");
        MASTERtempPtr = useTemp;  //remember my ID-s
        useTemp++;   //increase the number of sensors
      #endif
      #ifdef USE_MASTER_HUMID  
        DPRINTLN("  - MQTT MASTER_HUMID sensor added!");
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

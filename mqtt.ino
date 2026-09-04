#ifdef USE_MQTT
//=========================================================================================================================
// mqtt.ino - Home Assistant / MQTT integration (compiled only if USE_MQTT is defined for the active CLOCK_NN in clocks.h)
//
// Library: ArduinoHA (https://github.com/dawidchyrzynski/arduino-home-assistant), which wraps PubSubClient
//          (https://pubsubclient.knolleary.net/) and adds Home Assistant MQTT Discovery on top of it. Because ArduinoHA
//          is used, this file never hand-builds HA discovery JSON payloads itself - HADevice/HAMqtt/HASensor below
//          publish the discovery config topics automatically the first time mqtt.begin()/mqtt.loop() connects.
//
// MASTER / SLAVE sensor-sharing architecture (the distinctive part of this subsystem):
//   Every clock that defines USE_MQTT connects to the same broker (prm.mqttBrokerAddr/User/Psw, set on the web UI) and
//   PUBLISHES its own local sensors (temperature/humidity/pressure/lux/radar, whichever are physically present - see
//   HASensor objects below) under a "<MQTT_PREFIX>/sensor/<mac>/..." style topic, discovered via HA MQTT Discovery.
//   A clock that does NOT have its own sensor (or wants to reuse another clock's reading) can instead be built with
//   USE_MASTER_CLOCK, and SUBSCRIBE to another clock's already-published topics:
//     USE_MASTER_TEMP  + MASTER_TEMPERATURE_TOPIC  -> mirrors the master's temperature into local temperature[] array
//     USE_MASTER_HUMID + MASTER_HUMIDITY_TOPIC     -> mirrors the master's humidity into local humid[] array
//     USE_MASTER_RADAR + MASTER_RADAR_TOPIC        -> mirrors the master's PIR/radar presence state into mqttRadarON
//   This lets one "master" clock's physical sensor (e.g. a dedicated sensor box, or a clock with a real radar/PIR
//   sensor wired to RADAR_PIN) feed several "slave" clocks that have no sensor of their own, purely over MQTT.
//   IMPORTANT: if a "slave" clock DOES also have its own local RADAR_PIN wired, the local pin always wins - the radar
//   topic subscription is skipped entirely in onMqttConnected() (see RADAR_PIN>=0 check there), and the core loop's
//   checkTubePowerOnOff() also prefers digitalRead(RADAR_PIN) over the MQTT-derived mqttRadarON when both exist.
//   As of the current clocks.h: CLOCK_99 ("Dummy clock, sensor box with MQTT connector") is a pure MASTER/publisher
//   (USE_MQTT on, USE_MASTER_CLOCK off, has its own RADAR_PIN). CLOCK_26 has USE_MQTT + USE_MASTER_CLOCK +
//   USE_MASTER_RADAR all on, so it both publishes its own temperature/humidity sensors AND subscribes to another
//   clock's radar topic. CLOCK_23/CLOCK_23A define the USE_MASTER_* macros too but currently have USE_MQTT commented
//   out in clocks.h, so for those configs this whole file is compiled out and the master settings are presently inert.
//
// Call pattern from the core .ino: setupMqtt() is called once from setup() when USE_MQTT is defined; mqttSend() is
// called every loop() iteration (but only while WiFi is connected and prm.mqttEnable is set) - it internally rate-limits
// actual publishes to once every prm.mqttBrokerRefresh seconds, while still pumping mqtt.loop() (which also drives
// PubSubClient's/ArduinoHA's own reconnect handling) on every call.
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
// (the block above is documentation only, shown here as a template of what to copy into a CLOCK_NN section of clocks.h -
//  none of these #define lines are active in this file; the real values come from clocks.h at compile time.)

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
// Credential handling: broker IP/user/password are NOT compile-time constants (see the commented-out BROKER_* macros
// above, kept only as historical documentation) - they live in prm.mqttBrokerAddr/mqttBrokerUser/mqttBrokerPsw, which
// are persisted settings editable from the clock's own web UI (and read here in setupMqtt()/mqttSend()). This lets a
// user point any already-flashed clock at a different broker without recompiling.
// SECURITY: factoryReset() in the core .ino seeds these with a hardcoded default
// (strcpy(prm.mqttBrokerAddr,"10.10.0.202"); strcpy(prm.mqttBrokerUser,"mqtt"); strcpy(prm.mqttBrokerPsw,"mqtt")) -
// i.e. a real broker IP and a trivial user/password baked into the firmware as the out-of-the-box default. Not this
// file's code, but directly relevant here since it is exactly the credential pair mqttSend()/setupMqtt() use below.

#include <ArduinoHA.h>   //https://github.com/dawidchyrzynski/arduino-home-assistant
//https://pubsubclient.knolleary.net/     https://github.com/knolleary/pubsubclient

#if !defined(MQTT_PREFIX)
  #define MQTT_PREFIX "UniClock32"   //used both as the HA discovery prefix and as the first path segment of this device's own published topics
#endif

byte mac[6];                          //this device's WiFi MAC, used as a per-device unique ID/topic segment (see setupMqtt())
char masterTemperatureTopic[80] = ""; //resolved from MASTER_TEMPERATURE_TOPIC (clocks.h) if USE_MASTER_CLOCK/USE_MASTER_TEMP is used - topic this clock SUBSCRIBES to
char masterHumidityTopic[80] = "";    //resolved from MASTER_HUMIDITY_TOPIC (clocks.h) if USE_MASTER_CLOCK/USE_MASTER_HUMID is used - topic this clock SUBSCRIBES to
char masterRadarTopic[80] = "";       //resolved from MASTER_RADAR_TOPIC (clocks.h) if USE_MASTER_CLOCK/USE_MASTER_RADAR is used - topic this clock SUBSCRIBES to

// Index into the shared temperature[]/humid[] arrays (declared in the core .ino, up to 6 slots) reserved for the
// MASTER clock's mirrored reading, so the rest of the firmware (display rotation, web UI, etc.) can treat the
// MQTT-sourced master reading exactly like one more local sensor. Assigned in setupMqtt() as "next free slot"
// (useTemp/useHumid BEFORE incrementing) - despite the doc comment above saying "this will be the sensor#0", it is
// only actually sensor#0 if this clock has no other local temperature/humidity sensor already counted.
byte MASTERtempPtr=0;
byte MASTERhumidPtr=0;

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);   //ArduinoHA wrapper around PubSubClient; mqtt.loop() also drives PubSubClient's own reconnect/keepalive handling
//Sensors of local clock, to send data (each one auto-publishes its HA MQTT Discovery config the first time it connects):
HASensor mqttTemp("temperature");
HASensor mqttHumid("humidity");
HASensor mqttPress("pressure");
HASensor mqttLux("lux");
HASensor mqttRadar("radar");

// This is the SLAVE side of the master/slave architecture: called by ArduinoHA/PubSubClient whenever a message
// arrives on any topic this clock is subscribed to (see onMqttConnected() below for what gets subscribed - only
// the MASTER_* topics selected via USE_MASTER_TEMP/USE_MASTER_HUMID/USE_MASTER_RADAR in clocks.h). The payload is
// the raw value published by the MASTER clock's own mqttSend()/HASensor.setValue() (a plain ASCII number for
// temperature/humidity, "0"/"1" for radar - matching HASensor's default text encoding).
void onMqttMessage(const char* topic, const uint8_t* payload, uint16_t length) {
  float tempTMP,humidTMP;
  char tmp[100];
    // This callback is called when message from MQTT broker is received.
    // Please note that you should always verify if the message's topic is the one you expect.
    // For example: if (memcmp(topic, "myCustomTopic") == 0) { ... }

    DPRINT("New message on topic: ");    DPRINT(topic);

    // POSSIBLE BUG: `length` (the broker-reported payload size) is copied into the fixed 100-byte `tmp` buffer with
    // no bounds check - a payload of 100 bytes or more (e.g. a misconfigured/malicious publisher on the same topic)
    // overflows `tmp` on the stack. Also, memcpy() does NOT null-terminate `tmp`: it is used afterwards as a C string
    // (DPRINTLN(tmp), atof(tmp), tmp[0]) with whatever byte happens to follow the copied data in memory, which is
    // undefined behavior if that byte isn't already '\0'. A `if (length >= sizeof(tmp)) length = sizeof(tmp)-1;`
    // guard plus `tmp[length] = '\0';` after the memcpy would fix both issues.
    memcpy(tmp,payload,length);
    DPRINT("  Data: ");    DPRINTLN(tmp);
    // Topic match: compares up to sizeof(masterTemperatureTopic) (80, the fixed buffer size) bytes rather than the
    // actual topic length. This still behaves like strcmp() in practice because strncmp() stops as soon as it hits
    // the '\0' terminator on either side, but it is fragile - if MASTER_TEMPERATURE_TOPIC in clocks.h were ever 80
    // characters or longer, strncpy() in setupMqtt() would not null-terminate masterTemperatureTopic, and this
    // comparison could silently misbehave. Using strcmp(topic, masterTemperatureTopic) would be more robust.
    if ((strlen(masterTemperatureTopic)>0) && (strncmp(topic,masterTemperatureTopic,sizeof(masterTemperatureTopic)) ==0)) {
      tempTMP = atof(tmp);
      temperature[MASTERtempPtr] = tempTMP;   //overwrite the reserved "master" slot in the shared temperature[] array (see MASTERtempPtr above)
      DPRINT("MASTER clock's temperature:"); DPRINTLN(tempTMP);
    }
    if ((strlen(masterHumidityTopic)>0) && (strncmp(topic,masterHumidityTopic,sizeof(masterHumidityTopic)) ==0)) {
      humidTMP = atof(tmp);
      DPRINT("MASTER clock's humidity:"); DPRINTLN(humidTMP);
      humid[MASTERhumidPtr] = humidTMP;       //overwrite the reserved "master" slot in the shared humid[] array (see MASTERhumidPtr above)
    }
    if ((strlen(masterRadarTopic)>0) && (strncmp(topic,masterRadarTopic,sizeof(masterRadarTopic)) ==0)) {
       // Master clock's radar/PIR presence state, mirrored into mqttRadarON (and radarON) so the rest of the
       // firmware (night animation, tube power on/off, etc.) reacts to it exactly like a local sensor reading.
       // We only ever get here if this clock did NOT subscribe to the radar topic due to having its own local
       // RADAR_PIN (see the RADAR_PIN>=0 guard in onMqttConnected() below) - local hardware always wins.
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

// Called by ArduinoHA once the broker connection (re)establishes - this is where the SLAVE side actually subscribes
// to the MASTER clock's topics. Runs again on every reconnect (not just the first connect), so subscriptions survive
// broker restarts/WiFi drops without any extra logic needed in mqttSend().
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
        // WHY: if this clock has its own physical radar/PIR sensor wired to RADAR_PIN, that local reading always
        // takes priority (see checkTubePowerOnOff() in the core .ino, which prefers digitalRead(RADAR_PIN) over
        // mqttRadarON when both exist). So there is no point subscribing to the master's radar topic at all in that
        // case - onMqttMessage()'s radar branch would just be mirrored into a value nothing ever reads.
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

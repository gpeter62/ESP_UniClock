#include "ESP8266WiFi.h"
#include "ESPAsyncTCP.h"
#include "FS.h"
#include "ESPAsyncWebServer.h"
 
const char* ssid = "farm32";
const char* password =  "birka12345";
 
AsyncWebServer server(80);
 
void setup(){
  Serial.begin(115200);
  delay(200);
  Serial.println("--");
  if(!SPIFFS.begin()){
     Serial.println("An Error has occurred while mounting SPIFFS");
     return;
  }
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
 
  Serial.println(WiFi.localIP());
 
  server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
 
  server.on("/jquery_351.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/jquery_351.js", "text/js");
  });
   server.on("/page.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/page.js", "text/js");
  });
   server.on("/site.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/site.css", "text/css");
  });
  server.on("/arduino/msg", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.print("Kaptam valamit:"); Serial.println(request->url());
  });

  server.begin();
}
 
void loop(){}

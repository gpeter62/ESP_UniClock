/*
void wifiCode() {
static char txt[200];
WiFiClient client;   //static ???
bool mod = false;  
int tmp = 0;

  client = server.available();
  //char clientline[BUFSIZ];
  //int index = 0;
  if (client) {
    boolean currentLineIsBlank = true;
    boolean currentLineIsGet = true;
    int tCount = 0;
    char tBuf[512];
    char *pch;
    DPRINTLN("WiFi Client request...");
    while (client.connected()) {
       //DPRINTLN(txt);
       while (client.available()) {
        char c = client.read();
        header += c;
        if (currentLineIsGet && tCount < sizeof(tBuf)) {
          tBuf[tCount] = c;
          tCount++;
          tBuf[tCount] = 0;
        }
        if (c == '\n' && currentLineIsBlank) {
          while (client.available()) client.read();
          pch = strtok(tBuf, "?");
          mod = false;
          while (pch != NULL) {
            if (strncmp(pch, "utc_offset=", 11) == 0) {
              tmp = atoi(pch + 11);
              if (tmp > 12) tmp = 12;
              if (tmp < -12) tmp = -12;
              if (tmp != prm.utc_offset) {  
                prm.utc_offset = tmp;
                mod = true;
                calcTime();
              }
            }
            else if (strncmp(pch, "anim=", 5) == 0) {
              tmp = atoi(pch + 5); 
              if (tmp < 0) tmp = 0;
              if (tmp > 6) tmp = 6;
              if (tmp != prm.animMode) {
                prm.animMode = tmp;
                mod = true;              
              }
            }
            else if (strncmp(pch, "interval=", 9) == 0) {
              tmp = atoi(pch + 9); 
              if (tmp < 0) tmp = 0;
              if (tmp > 240) tmp = 240;
              if (tmp != prm.interval) {
                prm.interval = tmp;
                mod = true;              
              }
            }
            else if (strncmp(pch, "dayBright=", 10) == 0) {
              tmp = atoi(pch + 10); 
              if (tmp < 1) tmp = 1;
              if (tmp > MAXBRIGHTNESS) tmp = MAXBRIGHTNESS;
              if (tmp != prm.dayBright) {
                prm.dayBright = tmp;
                mod = true;              
              }
            }    
            else if (strncmp(pch, "nightBright=", 12) == 0) {
              tmp = atoi(pch + 12); 
              if (tmp < 1) tmp = 0;
              if (tmp > MAXBRIGHTNESS) tmp = MAXBRIGHTNESS;
              if (tmp != prm.nightBright) {
                prm.nightBright = tmp;
                mod = true;              
              }  
            }
            else if (strncmp(pch, "dayHour=", 8) == 0) {
              tmp = atoi(pch + 8); 
              if (tmp <0) tmp = 0;
              if (tmp>23) tmp = 23;
              if (tmp != prm.dayHour) {
                prm.dayHour = tmp;
                mod = true;              
              }              
            }
            else if (strncmp(pch, "dayMin=", 7) == 0) {
              tmp = atoi(pch + 7); 
              if (tmp <0) tmp = 0;
              if (tmp>59) tmp = 59;
              if (tmp != prm.dayMin) {
                prm.dayMin = tmp;
                mod = true;              
              }              
            } 
            else if (strncmp(pch, "nightHour=", 10) == 0) {
              tmp = atoi(pch + 10); 
              if (tmp <0) tmp = 0;
              if (tmp>23) tmp = 23;
              if (tmp != prm.nightHour) {
                prm.nightHour = tmp;
                mod = true;              
              }              
            }
            else if (strncmp(pch, "nightMin=", 9) == 0) {
              tmp = atoi(pch + 9); 
              if (tmp <0) tmp = 0;
              if (tmp>59) tmp = 59;
              if (tmp != prm.nightMin) {
                prm.nightMin = tmp;
                mod = true;              
              }              
            }
            else if (strncmp(pch, "rgbEffect=", 10) == 0) {
              tmp = atoi(pch + 10); 
              if (tmp <0) tmp = 0;
              if (tmp>3) tmp = 3;
              if (tmp != prm.rgbEffect) {
                prm.rgbEffect = tmp;
                mod = true;              
              }              
            }
            else if (strncmp(pch, "rgbFixColor=", 12) == 0) {
              tmp = atoi(pch + 12); 
              if (tmp <0) tmp = 0;
              if (tmp>256) tmp = 256;
              if (tmp != prm.rgbFixColor) {
                prm.rgbFixColor = tmp;
                mod = true;              
              }              
            }            
            else if (strncmp(pch, "rgbBrightness=", 14) == 0) {
              tmp = atoi(pch + 14); 
              if (tmp <0) tmp = c_MinBrightness;
              if (tmp>c_MaxBrightness) tmp = c_MaxBrightness;
              if (tmp != prm.rgbBrightness) {
                prm.rgbBrightness = tmp;
                mod = true;              
              }              
            }
            else if (strncmp(pch, "rgbSpeed=", 9) == 0) {
              tmp = atoi(pch + 9); 
              if (tmp <0) tmp = 20;
              if (tmp>255) tmp = 255;
              if (tmp != prm.rgbSpeed) {
                prm.rgbSpeed = tmp;
                mod = true;              
              }              
            }
           pch = strtok(NULL, "& ");
          }  //end while pch...
          
          if (header.indexOf("GET /DAY") >= 0)        {displayON = true; manualOverride = true;} 
          else if (header.indexOf("GET /NIGHT") >= 0) {displayON = false; manualOverride = true;} 
          else if (header.indexOf("GET /DNON") >= 0)  {prm.enableAutoShutoff = true; mod = true;} 
          else if (header.indexOf("GET /DNOFF") >= 0) {prm.enableAutoShutoff = false; mod = true;}           
          else if (header.indexOf("GET /BLINKON") >= 0) {prm.enableBlink = true; mod = true;} 
          else if (header.indexOf("GET /BLINKOFF") >= 0) {prm.enableBlink = false; mod = true;} 
          else if (header.indexOf("GET /DSTON") >= 0) {prm.enableDST = true; mod = true;}
          else if (header.indexOf("GET /DSTOFF") >= 0) {prm.enableDST = false; mod = true;}
          else if (header.indexOf("GET /SET12") >= 0) {prm.set12_24 = false; mod = true;}
          else if (header.indexOf("GET /SET24") >= 0) {prm.set12_24 = true; mod = true;}
          else if (header.indexOf("GET /ZERO") >= 0) {prm.showZero = true; mod = true;}
          else if (header.indexOf("GET /NOZERO") >= 0) {prm.showZero = false; mod = true;}
          else if (header.indexOf("GET /factReset") >= 0) { factoryReset();  }

          if (mod) {  //Save modified parameters
            DPRINTLN("Saved to EEPROM!");
            saveEEPROM();    
            }
           if (useTemp>0)   
              sprintf(txt,"<H1>%02d/%02d/%4d %02d:%02d      T:%2.1fC</H1>",month(),day(),year(),hour(),minute(),temperature[0]);
          else 
              sprintf(txt,"<H1>%02d/%02d/%4d %02d:%02d </H1>",month(),day(),year(),hour(),minute());
          client.print("HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><H1>"); client.print(webName);
          client.print("</H1><form method=GET>");
          client.println(txt);  
          client.print("<br>UTC offset (-12..12):<input type=text  maxlength=3 size=3 name=utc_offset value=");  client.print(prm.utc_offset);
          client.print("><br>ANIMATE (0..6): <input type=text maxlength=1 size=3 name=anim value=");    client.print(prm.animMode);
          client.print(">  SHOW (0..240min): <input type=text maxlength=3 size=3 name=interval value=");    client.print(prm.interval);
          client.print("><br>DAY &#160;&#160;&#160;&#160;Brightness (1.."); client.print(MAXBRIGHTNESS);
          client.print("): <input type=text maxlength=2 size=1 name=dayBright value=");    client.print(prm.dayBright);
          client.print("><br>NIGHT Brightness (0.."); client.print(MAXBRIGHTNESS);
          client.print("): <input type=text maxlength=2 size=1 name=nightBright value=");    client.print(prm.nightBright);
          if (prm.enableAutoShutoff) {  
            client.print("><br>DAY &#160;&#160;&#160;&#160;hour (0..23): <input type=text maxlength=2 size=2 name=dayHour value=");  client.print(prm.dayHour);
            client.print("> min (0..59): <input type=text maxlength=2 size=2 name=dayMin value=");             client.print(prm.dayMin);
            client.print("><br>NIGHT hour (0..23): <input type=text maxlength=2 size=2 name=nightHour value=");  client.print(prm.nightHour);
            client.print("> min (0..59): <input type=text maxlength=2 size=2 name=nightMin value=");           client.print(prm.nightMin);  
          }
//#if defined(USE_NEOPIXEL_MAKUNA) && defined(USE_NEOPIXEL_ADAFRUIT)
#ifdef USE_NEOPIXEL_MAKUNA
            client.print("><br>RGB effect:(0-3): <input type=text maxlength=1 size=1 name=rgbEffect value=");         client.print(prm.rgbEffect);  
            client.print("> fixColor: <input type=text maxlength=3 size=3 name=rgbFixColor value=");                  client.print(prm.rgbFixColor); 
            client.print("><br>Max bright(0-256): <input type=text maxlength=3 size=3 name=rgbBrightness value=");    client.print(prm.rgbBrightness); 
            client.print("> speed: <input type=text maxlength=3 size=3 name=rgbSpeed value=");                        client.print(prm.rgbSpeed);  
#endif
          client.print("><br><input type=submit value=Submit></form>");
          
          client.println("<!DOCTYPE html><html>");
          client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
          client.println("<link rel=\"icon\" href=\"data:,\">");
          client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: left;}");
          client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 8px 20px;");
          client.println("text-decoration: none; font-size: 15px; margin: 2px; cursor: pointer;}");
          client.println(".button2 {background-color: #555555;}</style></head>");
//   ---------- body  -  Buttons ... --------------------          
          //client.println("<body><h1>Control buttons</h1>");
          if (!displayON)       client.println("<p><a href=\"/DAY\"><button class=\"button button2\">DAY!</button></a>");
          if (displayON)        client.println("<p><a href=\"/NIGHT\"><button class=\"button button2\">NIGHT!</button></a>");
          if (!prm.enableAutoShutoff) client.println("<a href=\"/DNON\"><button class=\"button button2\">AUTO DAY/NIGHT ON</button></a>");
          if (prm.enableAutoShutoff)  client.println("<a href=\"/DNOFF\"><button class=\"button button2\">AUTO DAY/NIGHT OFF</button></a>");          
          if (!prm.enableBlink) client.println("<p><a href=\"/BLINKON\"><button class=\"button button2\">BLINK ON</button></a>");
          if (prm.enableBlink)  client.println("<p><a href=\"/BLINKOFF\"><button class=\"button button2\">BLINK OFF</button></a>");          
          if (!prm.enableDST)   client.println("<a href=\"/DSTON\"><button class=\"button button2\">DST ON</button></a>");
          if (prm.enableDST)    client.println("<a href=\"/DSTOFF\"><button class=\"button button2\">DST OFF</button></a>");
          if (prm.set12_24)     client.println("<p><a href=\"/SET12\"><button class=\"button button2\">SET 12h</button></a>");
          if (!prm.set12_24)    client.println("<p><a href=\"/SET24\"><button class=\"button button2\">SET 24h</button></a>");
          if (!prm.showZero)    client.println("<a href=\"/ZERO\"><button class=\"button button2\">Show Zero</button></a></p>");
          if (prm.showZero)     client.println("<a href=\"/NOZERO\"><button class=\"button button2\">NO Zero</button></a></p>");
          client.println("<p><a href=\"/factReset\"><button class=\"button button2\">Factory Reset</button></a></p>");
          client.println("<br>(c) 2020 Peter Gautier<br>gautier.p62@gmail.com<br>");
          client.write("</body></html>\r\n\r\n");
          header = "";
          client.stop();
        }
        else if (c == '\n') {currentLineIsBlank = true;  currentLineIsGet = false;  }
        else if (c != '\r') { currentLineIsBlank = false;  }
      } //endwhile client available
    timeProgram();
    if (checkWifiMode()) return;   //clock mode changed!
    } //endwhile client connected
  } //endif (client)
}  
*/

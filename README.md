
# ESP_UniClock v2.3

##Universal WifiClock with Nixie, VFD, Numitron tubes or LED display
	- Number of digits: 4, 6 or 8
	- Adjustable day and night brightness
	- Tube animation: 5 different animation modes, mode6 means random animation
	- Hardver: (Schematics and board design by Zoltan Unferdorben are in "schematics" subfolder)
	- Wemos D1 Mini (or any other ESP8266 modul) 160MHz or ESP32 
	- Drivers: SN74141 (multiplex or latch), MAX6921, MAX7219, MM5450

##OPTIONAL COMPONENTS:

	- DS18B20 temperature sensor  (1 or 2 sensor), 
	- DHT11/DHT22 temperature/humidity sensor
	- DS3231 RTC Clock with Wifi/Manual mode switch and pushbuttons to set the date/time - 
	standalone mode possible without WiFi
	- GPS modul (to use instead of Wifi sync  - standalone mode, only web setup screen available)
	- WS2812 adressable LEDS, adjustable for fix color or different animation effects
	- User defined pin numbers for digits and segments
	- Web page for setup. See: Uniclock_v2.jpg

-- External libraries used ------------------------------------------------------------------------------
Time, Timezone, NtpClient, ESPAsyncWifiManager, ArduinoJson6.0       
Optional: DS3231, TinyGPSPlus, NeoPixelBus_by_Makuna
--------------------------------------------------------------------------------------------------------
Web page files are found in "dat" subfolder must upload to SPIFFS !

	- ESP8266:  https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide/
	- ESP32: https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
--------------------------------------------------------------------------------------------------------
More info & support:

Peter Gautier
 Email: gautier.p62@gmail.com

The old version backup is in ESP_UniClock_1_4_final.zip!!!

# ESP_UniClock v2.3

## Universal WifiClock with Nixie, VFD, Numitron tubes or LED display
	- Any ESP8266 or ESP32 modul is supported
	- Clock digits (tubes): 4, 6 or 8
	- Time sync from WiFi, GPS or RTC
	- WiFi Client or AP mode, Clock Web Page: (https://github.com/gpeter62/ESP_UniClock/Webpage.jpg)
	- Temperature / Hygrometer support
	- Adjustable Day and Night brightness
	- Tube animation: 5 different animation modes (Mode-6 is random animation)
	- Tube backlightning with WS2812 addressable leds, many full color animations 
	- Alarm function with buzzer and LED lightning
	- Hardver: (Schematics and board design by Zoltan Unferdorben are in "schematics" subfolder)
	- Wemos D1 Mini (or any other ESP8266 modul) 160MHz or ESP32 
	- Drivers: SN74141 (multiplex or latch), MAX6921, MAX7219, MM5450
	- Clock setup can be defined in clocks.h file (https://github.com/gpeter62/ESP_UniClock/clocks.h)
## Hardver schematics and PCB plans are found in schematics folder 
	- ESP32 Clock, 6x IN-11 VFD tubes
	- ESP32 Clock, 6x Z573M/Z574M Nixie tubes
	- ESP8266 Numitron Clock 4x IV-16 tubes
	- and many others

## Optional Components:
	- DS18B20 temperature sensor  (1 or 2 sensor), 
	- DHT11/DHT22 temperature/humidity sensor
	- DS3231 RTC Clock with Wifi/Manual mode switch and pushbuttons to set the date/time - 
	standalone mode possible without WiFi
	- GPS modul (to use instead of Wifi sync  - standalone mode, only web setup screen available)
	- WS2812 adressable LEDS, adjustable for fix color or different animation effects
	- User defined pin numbers for digits and segments
	- Web page for setup. See: Uniclock_v2.jpg

## External libraries used (available zipped in libraries folder)
	- ESP8266: Time, Timezone, NtpClient, ESPAsyncWifiManager, ESPAsyncTCP, ArduinoJson6.0, AsyncTCP-ESP32 
	- Optional: ds3231, TinyGPSPlus, NeoPixelBus_by_Makuna

	- Web page files are found in "dat" subfolder must upload to SPIFFS !
		- ESP8266:	(https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide/
		- ESP32:	(https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/)
--------------------------------------------------------------------------------------------------------
More info & support:

Peter Gautier
 Email: gautier.p62@gmail.com

The very old version backup is in ESP_UniClock_1_4_final.zip!!!
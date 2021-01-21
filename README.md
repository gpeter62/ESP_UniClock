# ESP_UniClock v2.3
 
## Universal Wi-Fi Clock with Nixie, VFD and Numitron tubes, or LED display
                - Works with any ESP8266 or ESP32 based module (via the Arduino IDE)
                - # of clock digits (tubes): 4, 6 or 8
                - Time sync using Wi-Fi, GPS or RTC chip
                - Wi-Fi, both infrastructure and standalone (AP) modes, see /Webpage.jpg
                - Temperature / humidity sensor support
                - Adjustable day and night brightness
		- Radar or infra sensor for tube power off feature
                - Tube animation: 5 different animation modes (also Mode 6 for random selected)
                - Tube backlight using WS2812 LEDs, with different full-color animations 
                - Alarm function with buzzer and LED light
                - SN74141 (mpx or latch), MAX6921, MAX7219, MM5450, HV5122, PT6355, SN75512 drivers
                - Single file setup via /clocks.h
 
## Schematics and PCB designs in /schematics :
                - ESP32 Clock, 6x IN-11 VFD tubes
                - ESP32 Clock, 6x Z573M/Z574M Nixie tubes
                - ESP8266 Numitron Clock, 4x IV-16 tubes
                - many more...
 
## Optional components:
                - DS18B20 temperature sensor (1 or 2 sensors) 
                - DHT11/DHT22 temperature/humidity sensor
		- BME280/BMP280/AHTX0/SHT21 temperature/humidity sensor on I2C bus
                - DS3231 RTC clock, set via with Wi-Fi, or manually using pushbuttons
		- Radar or infra motion sensor
                - GPS module for time sync - Wi-Fi is in AP (standalone) mode
                - WS2812 adressable LEDS, selectable fixed color or multiple animation effects
                - User defined pin numbers for digits and segments
                - Setup via web page: /Uniclock_v2.jpg
 
## External libraries used (available zipped in libraries folder)
- ESP8266: Time, Timezone, NtpClient, ESPAsyncWifiManager, ESPAsyncTCP, ArduinoJson6.0, AsyncTCP-ESP32 
- Optional: ds3231, TinyGPSPlus, NeoPixelBus_by_Makuna, Adafruit sensor libraries
- Web page files are found in "dat" subfolder must upload to SPIFFS with uploader tool!
	- [ESP8266 Tool](https://randomnerdtutorials.com/install-esp8266-filesystem-uploader-arduino-ide)
	- [ESP32 Tool](https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide)

		
--------------------------------------------------------------------------------------------------------
# More info & support:
## Before uploading to the ESP board don't forget to unzip the libary files to the Arduino IDE's \libraries folder and RENAME the main folder to ESP32_UniClock2!
Peter Gautier [Email: gautier.p62@gmail.com]

# ESP_UniClock v1.4

Universal WifiClock with Nixie, VFD, Numitron tubes or LED display
Number of digits: 4, 6 or 8

Adjustable day and night brightness

ANIMATE: 5 animation modes, 6 means random animation

Hardver:

Wemos D1 Mini (or any other ESP8266 modul) 160MHz

Drivers: SN74141 (multiplex or latch), MAX6921, MAX7219, MM5450

Optional: DS18B20 temperature sensor  (1 or 2 sensor)
 
Optional: DHT11/DHT22 temperature/humidity sensor

Optional: DS3231 RTC Clock with Wifi/Manual mode switch and pushbuttons to set the date/time - standalone mode possible without WiFi

Optional: GPS modul (to use instead of Wifi sync  - standalone mode, only web setup screen available)

User defined pin numbers for digits and segments

Web page for setup. See: Screenshot.png

-- External libraries used ------------------------------------------------------------------------------

Time, Timezone, NtpClient, Wifimanager       Optional: DS3231, TinyGPSPlus, 
--------------------------------------------------------------------------------------------------------

More info:

Peter Gautier
 Email: gautier.p62@gmail.com
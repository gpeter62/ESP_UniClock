# ESP_UniClock2 – Fejlesztői dokumentáció

Ez a dokumentum a teljes rendszer architektúráját írja le. A haladás/státusz külön fájlban van:
[PROGRESS.md](PROGRESS.md). Az összesített hibalista/optimalizálási javaslatok: [CODE_REVIEW.md](CODE_REVIEW.md).

Utolsó frissítés: 2026-09-03.

## Executive summary

A teljes kódbázis (~13 000 sor, minden .ino/.h fájl a "kétes" – dummy.ino, mqtt.old, rtc.ino.old,
samsung.ino.sav, "ZM1500 .sav" – fájlok kivételével) át lett olvasva, dokumentálva és kikommentezve.
A legfontosabb megállapítások:

1. ✅ **ZM1500.ino lezárva**: a korábbi, félbehagyott fejlesztésű driver törölve, az utolsó ismert
   működő verzió (`ZM1500 .sav`) visszaállítva `ZM1500.ino` néven. Nincs ismert hiba benne.
2. ✅ **`enableDisplay()`/`disableDisplay()` eltávolítva**: mindkét függvény évek óta hatástalan volt
   (feltétel nélküli `return` az elején) – a user kérésére teljesen törölve, minden hívási helyükkel
   (~24 db) és a hozzájuk tartozó, szintén feleslegessé vált `dState`/`lastDisable` globálisokkal együtt.
3. **🔴 Biztonsági megjegyzés**: `mqtt.ino` – hardkódolt gyári MQTT broker-cím/felhasználó/jelszó a
   `factoryReset()`-ben, és egy határellenőrzés nélküli `memcpy` az MQTT-üzenet feldolgozásában
   (potenciális stack-túlcsordulás hosszú payload esetén). **Nem javítva** – felhasználói döntés kérdése.
4. ✅ **Összes ismert túlindexelési (out-of-bounds) hiba javítva** – lásd [CODE_REVIEW.md](CODE_REVIEW.md)
   a pontos listáért és a javítások részleteiért.
5. Számos 🟡/⚪ szintű, még nem javított hiba, holt kód és inkonzisztencia – teljes lista: [CODE_REVIEW.md](CODE_REVIEW.md).
6. **Két driver-fájl jelenleg árva** (egyetlen aktív CLOCK_NN konfiguráció sem használja): `HV5122.ino`
   (mindenki a `HV5122new.ino`-t/`NEWHV5122`-t használja) és `SN75512.ino`.
7. A rendszer architektúrája összességében jól strukturált és konzisztens a driver-interfész mentén –
   a hibák túlnyomó többsége lokális, izolált, könnyen javítható probléma, nem architekturális hiba.
8. ✅ **Angol nyelvi lektorálás lezárva**: minden .ino/.h fájl kommentje átnézve angol helyesség
   szempontjából, elgépelések javítva, a maradék magyar szórészletek lefordítva (főleg `clocks.h`-ban).
   Ellenőrizve, hogy nem maradt véletlenül otthagyott magyar szöveg kommentben.

**Ez egy élő dokumentum** – ahogy további javítások történnek, érdemes frissíteni.

---

## 1. Mi ez a projekt?

Egyetlen Arduino-forráskód-bázis, amely ESP8266 vagy ESP32 mikrovezérlőn fut, és **~65 fajta,
ténylegesen megépített, fizikailag különböző órát** tud kiszolgálni: Nixie csöves, VFD (vákuum
fluoreszcens), Numitron (izzószálas), LED mátrix/szegmens és "szóóra" (word clock) kijelzős
változatokat egyaránt. A különbség kizárólag **fordítási időben, a `clocks.h` fájlban** dől el – a
program logikája (időkezelés, WiFi, webes admin felület, animációk, riasztás, szenzorok, MQTT) közös,
csak a "hogyan írjuk ki a számjegyet a hardverre" réteg (a *driver*) cserélődik.

## 2. Build-modell – ezt kell először megérteni

Az Arduino IDE/CLI egy `.ino` sketch-mappa **összes** `.ino` fájlját egyetlen fordítási egységbe
fűzi össze (a `.ino` kiterjesztésű fájlok gyakorlatilag "include"-olva vannak, ábécésorrendben, a fő
`ESP32_UniClock2.ino` elé/mögé). Ez a projekt ezt így használja ki:

- **`clocks.h`** egy "kapcsolótábla": legfelül ~65 sor `//#define CLOCK_NN` van felsorolva, ebből
  pontosan **egy van kikommentezve** (jelenleg: `CLOCK_72`, sor 64). Minden `CLOCK_NN` blokk
  (`#ifdef CLOCK_NN ... #endif`) definiálja:
  - egy **driver-választó makrót** (pl. `MAX6921`, `MAX6921_ESP32`, `HV5122`, `NEWHV5122`, `PT6311`,
    `PT6355`, `ZM1500_ESP32`, `MULTIPLEX74141_ESP32`, `VQC10`, `WORDCLOCK`, stb.)
  - a fizikai GPIO-kiosztást (pl. `digitEnablePins[]`, `segmentEnablePins[]`, `ABCDPins[]`, `PIN_LE`,
    `PIN_CLK`, stb. – drivertől függően más és más nevekkel)
  - opcionális funkciókat be/kikapcsoló makrókat (`USE_NEOPIXEL`, `USE_RTC`, `USE_GPS`,
    `USE_DALLAS_TEMP`, `USE_MQTT`, `ALARMSPEAKER_PIN`, stb.)
  - a fájl végén (2360–2534. sor) egy nagy `#ifndef X #define X <default> #endif` blokk-sorozat
    gondoskodik róla, hogy minden opcionális paraméternek legyen értelmes alapértéke, ha az adott
    `CLOCK_NN` nem definiálta explicit.
- **Minden driver `.ino` fájl a teljes tartalmát `#ifdef <SAJÁT_MAKRÓJA> ... #endif` közé zárja.**
  Mivel egyszerre csak egy driver-makró van definiálva (az aktív `CLOCK_NN` blokkból), a többi
  driver-fájl teljes törzse *lefordítás előtt* kiesik a preprocesszorban – gyakorlatilag "üres" lesz.
  Ezért lehet a repóban egyszerre ~25 különböző, egymást kizáró driver-implementációt tartani: csak
  az fordul be ténylegesen, amelyiket a `clocks.h` aktivál.
- Emiatt **egyetlen kódmódosítás új óratípus hozzáadásához**: egy új `#define CLOCK_NN` blokk a
  `clocks.h`-ba (meglévő hasonló blokk lemásolásával), és ha a hardver driver-chipje még nincs
  meg, egy új `driver.ino` fájl a repó gyökerében.

## 3. A közös driver-interfész (szerződés)

Minden driver `.ino` fájlnak a következő függvényeket/globálisokat kell megvalósítania, mert a core
fájl (`ESP32_UniClock2.ino`) ezeket `extern`-ként hívja, driver-függetlenül:

| Név | Szerep |
|---|---|
| `void setup_pins()` | GPIO-k `pinMode()`-olása, `regPin(pin,"név")` minden lábra (ütközés-ellenőrzéshez és a webes pin-listához), jellemzően a végén `startTimer()` hívás |
| `void writeDisplay()` | `IRAM_ATTR` (ESP32) / `ICACHE_RAM_ATTR` (8266) – **időzítő-megszakítás (ISR)** kézbesítő, ez multiplexeli folyamatosan a kijelzőt háttérben. Néhány "okos" chip-driver (PT6311, PT6355, MAX7219, VQC10), ahol a chip maga tud multiplexelni/scannelni, ezt üresen hagyja, és nem hívja a `startTimer()`-t sem. |
| `void writeDisplaySingle()` | Blokkoló, egyszeri, nem-ISR kiírás a kijelzőre |
| `void clearTubes()` | Kijelző elsötétítése |
| `int maxDigits` | Hány csöves/digites a konfiguráció |
| `char tubeDriver[]` | Emberi olvasásra szánt driver-név (logban, weblapon) |

### Közös globális állapot (a core fájlban deklarálva, a driverek ezt olvassák/írják)

- `byte digit[BUFSIZE], newDigit[BUFSIZE], oldDigit[BUFSIZE]` (BUFSIZE=12) – a ténylegesen
  kirakandó érték csövenként. Kódolás: **0-9** = számjegy, **10** = ÜRES (blank), **11** = mínusz/kötőjel,
  **12** = önálló tizedespont-glyph (néhány drivernél), **13+** = driver-specifikus extra karakterek
  (pl. 'A', 'P', 'C', '°', '%', kör-alakzatok, 'I', 'F' – minden driver saját `charDefinition[]`/
  `charTable[]` táblával fordítja ezeket szegmensmintává).
- `boolean digitDP[BUFSIZE]` – az adott pozíció tizedespontja/elválasztó pontja világítson-e.
- `byte animMask[BUFSIZE]` – a "wipe" animációhoz (5-ös animMode, lásd lent) használt maszk, 0=nincs.
- `prm` struct (EEPROM-ban perzisztált beállítások – ld. 6. fejezet).
- `displayON` (nappal/éjszaka fényerő-mód), `autoBrightness`, `lx` (aktuális lux-érték),
  `MAXBRIGHTNESS` (fordítási idejű maximum).
- `DPRINT/DPRINTLN/DPRINTF` makrók – no-op, ha `DEBUG` nincs definiálva.
- `regPin(byte pin, const char* label)` – minden GPIO-t regisztrálni kell vele.

## 4. Core program felépítése (`ESP32_UniClock2.ino`, ~2930 sor)

### 4.1 `setup()` (1574. sor)
Sorrend: brownout-detektor kikapcsolása (opcionális) → EEPROM init → `setup_pins()` (driver-specifikus)
→ opcionális perifériák (`ALARMSPEAKER_PIN`, `COLON_PIN`, `RADAR_PIN`, `TUBE_POWER_PIN`,
`LIGHT_SENSOR_PIN`, PWM LED-ek) → hőmérséklet/I2C-szenzorok/RTC/GPS setup → `loadEEPROM()` (ha a
"magic number" nem egyezik, `factoryReset()`) → NeoPixel setup → csőteszt-animáció → SPIFFS mount
(weblap fájljainak forrása) → WiFi csatlakozás (STA vagy AP mód, ld. 4.3) → webszerver indítás → MQTT
setup → IP-cím kijelzése a csöveken → időszinkron.

### 4.2 Fő ciklus – `loop()` (2883. sor)
Nincs `delay()`-alapú várakozás a fő cikluson belül (kivéve a segédfüggvényeken belüli `Fdelay()`
hívásokat, amik maguk sem blokkolnak teljesen, mert közben `writeDisplay2()`-t, animációkat,
`dnsServer`-t stb. is kiszolgálnak). Minden ciklusban:
`writeDisplay2()` (csak a nem-ISR-alapú drivereknél csinál valamit) → DNS (AP módban) →
`timeProgram()` (max 200ms-onként fut ténylegesen, ld. lent) → animációk (NeoPixel/PWM) →
`alarmSound()` → `checkTubePowerOnOff()` (radar) → `getLightSensor()` → MQTT küldés/WiFi
újracsatlakozás → `editor()` (RTC gombkezelés, ha van) → esetleges firmware-frissítés / cathode-protect
kérés végrehajtása.

### 4.3 WiFi-kezelés
Két üzemmód (`prm.wifiMode`): **Infrastruktúra (STA)** – csatlakozás egy meglévő routerhez
(`startWifiMode()`, sikertelen esetnél `wifiManager()`/`ESPAsyncWiFiManager` konfigportál, ha
`USE_WIFIMANAGER` be van kapcsolva), vagy **Standalone (AP)** – az óra saját hotspotot nyit
(`startStandaloneMode()`, `192.168.4.1`, DNS-capture-portál). WiFi-kapcsolat elvesztésekor
`resetWiFi()` 10 percenként próbál újracsatlakozni.

### 4.4 Időforrás-prioritás – `calcTime()` (1768. sor)
1. Ha van WiFi: `updateTimefromTimeserver()` (NTP, 2 óránként frissít – `TIMESERVER_REFRESH`).
2. Ha van GPS (`GPSexist`): `getGPS()`.
3. Ha van RTC (`RTCexist`): ha az előző kettő közül bármelyik frissített, `updateRTC()` (RTC-t
   frissíti az internetes/GPS idővel), egyébként `getRTC()` (RTC-ből olvas – ez a fallback, ha nincs
   se WiFi, se GPS-frissítés).

Ezt a `timeProgram()` (1788. sor) hívja legfeljebb 5 másodpercenként (a tényleges kijelzés csak
másodpercváltáskor frissül: `if (now() != prevTime)`), és ez hívja a hőmérséklet-/páratartalom-
lekérdezéseket is (Dallas, DHT, I2C-szenzorok), majd eldönti, hogy a képernyőn épp óra, dátum,
hőmérséklet vagy páratartalom jelenjen-e meg (`showClock/showDate/showTemp0/showTemp1/showHumid0/
showHumid1` – ciklikusan váltakoznak a `prm.tempStart/tempEnd/humidStart/humidEnd/dateStart/dateEnd`
másodperc-ablakok szerint), végül meghívja a driver-független `changeDigit()`-et (animáció) és
`printDigits()`-et (debug-konzol kiírás).

### 4.5 Kijelző-csővezeték (pipeline)
`displayTime4()/displayTime6()/displayTime8()` (a `maxDigits` szerint) → beírja az aktuális
óra/perc/másodperc (vagy dátum/hőmérséklet/páratartalom, ha épp az van soron) értékeit
`newDigit[]`-be → `changeDigit()` összehasonlítja `newDigit[]`-et `oldDigit[]`-tel, és ha eltérés
van, lejátssza a beállított animációt (`prm.animMode`, 0-7):
- 0: nincs animáció (azonnali váltás)
- 1: "gördülő" számlálás minden változó csövön egyenként (0→9→cél)
- 2: minden változó cső egyszerre pörög fel
- 3/4: balra/jobbra "becsúszó" digit-sor animáció (`----` elválasztóval)
- 5: "wipe" – `animMask[]`-kel vezérelt letörlés/felfedés (driverenként eltérő implementáció –
  SN74141-családnál és néhány másiknál speciális, szegmensenkénti verzió van)
- 6: random választás 1-5 közül minden váltáskor
- 7: random választás 1-4 közül

Majd `writeDisplaySingle()`-lel (driver-specifikus) azonnal kiírja a végeredményt, és
`oldDigit[]`/`digit[]` frissül `newDigit[]`-re.

### 4.6 EEPROM / `prm` struct
A `prm` struct (317. sor) tartalmazza az ÖSSZES perzisztens felhasználói beállítást (riasztás,
RGB-effekt, WiFi SSID/jelszó, MQTT broker adatok, fényerő nappal/éjjel, dátum/hőmérséklet
megjelenítési ablakok, kalibrációs korrekciók, stb.) – ez egyben van `EEPROM.put()`-olva/
`EEPROM.get()`-elve (`saveEEPROM()`/`loadEEPROM()`), egy `magic` mezővel verziózva (`MAGIC_VALUE`,
jelenleg 304) – ha ez nem egyezik (pl. új firmware más struct-elrendezéssel), `factoryReset()` fut le
automatikusan induláskor.

### 4.7 Webes admin felület
`startServer()` (948. sor) egy `ESPAsyncWebServer`-t indít a 80-as porton, statikus fájlokat szolgál
ki SPIFFS-ből (`/index.html`, `/site.css`, `/page.js`, `/jquery_*.js` – forrásuk a repó `data/`
mappájában, ezeket kell külön SPIFFS-feltöltő eszközzel felmásolni a kártyára). REST-szerű API:
- **`POST /configChanged`** (`key`, `value` form-paraméterekkel) – `handleConfigChanged()` (1124. sor):
  egy hosszú `if/else if` lánc kulcsnév szerint írja a megfelelő `prm.*` mezőt, majd `saveEEPROM()`.
  ~60 lehetséges kulcs van (időzóna, fényerő, riasztás, RGB, WiFi, MQTT, dátum/hőmérséklet-ablakok,
  kalibráció, stb.) – ez az **egyetlen belépési pont** minden beállítás-módosításhoz a weblapról.
- **`GET` valamilyen végpont** → `handleSendConfig()` (1364. sor) – a teljes `prm` állapotot JSON-ként
  visszaküldi a kliensnek (kezdeti betöltéskor).
- `handleSendClockDetails()` (1501. sor) – statikus infó (firmware ID, driver név, MAC-cím, használt
  GPIO-k listája – ld. `regPin()`/`listPins()`).
- `handleSendCurrentInfos()` (1521. sor) – gyakran frissülő élő adatok (dátum, idő, hőmérséklet,
  páratartalom, légnyomás, lux, WiFi RSSI) JSON-ban, a weblap AJAX-pollingjához.

## 5. Perifériák és opcionális alrendszerek

| Alrendszer | Fájl(ok) | Aktiválás |
|---|---|---|
| DS3231 RTC + kézi gombos időállítás | `rtc.ino` | `USE_RTC` |
| GPS időszinkron (önálló órákhoz) | `gps.ino` | `USE_GPS` |
| DS18B20 hőmérséklet (1-wire, max 2 db) | `dallas_temp.ino` | `USE_DALLAS_TEMP` |
| DHT11/22 hőmérséklet+pára | `DHT_temp.ino` | `USE_DHT_TEMP` |
| I2C szenzorok (BME280/BMP280/AHTX0/SHT21/BH1750) | `I2Csensors.ino` | `USE_BME280` stb. (a `USE_I2CSENSORS` automatikusan levezetve, ha bármelyik + `PIN_SDA`/`PIN_SCL` be van állítva) |
| WS2812 (NeoPixel) csőháttérvilágítás, animációkkal | `z_neopixel.ino` | `USE_NEOPIXEL` |
| PWM (RGB, 3 láb) háttérvilágítás | `pwmleds.ino` | `USE_PWMLEDS` (csak ESP32) |
| Home Assistant / MQTT integráció | `mqtt.ino` | `USE_MQTT` |

### 5.1 MQTT master/slave architektúra
Néhány konfiguráció (`USE_MASTER_CLOCK`, `USE_MASTER_TEMP`, `USE_MASTER_HUMID`, `USE_MASTER_RADAR` +
`MASTER_*_TOPIC` makrók, pl. CLOCK_23/23A/26) nem a saját helyi szenzorát használja, hanem egy másik,
"master" óra által MQTT-n publikált hőmérséklet/páratartalom/radar-állapotot **feliratkozza** –
így egy fizikai szenzor több "néma" (csak kijelző) klón-óra kiszolgálására is elég lehet. Részletek:
[CODE_REVIEW.md](CODE_REVIEW.md) mqtt.ino szakasza (a review folyamatban).

## 6. `clocks.h` gyors-referencia (a legfontosabb makrók)

*(Ez a szakasz bővül – az összes ~65 CLOCK_NN blokk teljes felsorolása külön mellékletben lesz, ha kész.)*

**Driver-választó makrók** (pontosan egy legyen aktív): `MULTIPLEX74141` / `MULTIPLEX74141_ESP32` /
`NO_MULTIPLEX74141` / `NO_MULTIPLEX_ESP32` / `PCF_74141` — SN74141-Nixie család (ld. `multiplex74141*.ino`,
`no_multiplex*.ino`, `pcf_multiplex74141.ino`); `MAX6921` / `MAX6921_ESP32` — VFD (`MAX6921*.ino`);
`HV5122` / `NEWHV5122` — Nixie HV-driver (`HV5122.ino`/`HV5122new.ino` – **csak `NEWHV5122` aktív
bármelyik CLOCK_NN-ben, a sima `HV5122.ino` jelenleg árva kód**); `PT6311`, `PT6355`, `SN75512`,
`SN75518_ESP32`, `MM5450`, `MAX7219CNG`, `Numitron_4511N`, `VQC10`, `TUBE1CLOCK` (`1tube.ino`),
`MULE_V2` (`mulev2.ino`), `ZM1500_ESP32` (`ZM1500.ino` – **jelenleg aktív, CLOCK_72**), `WORDCLOCK`
(`wordclock.ino`), `DUMMY` (`dummy.ino` – jelenleg nem review-zott, ld. PROGRESS.md).

**Opcionális funkció-makrók**: `DEBUG`, `USE_NEOPIXEL`, `USE_PWMLEDS`, `USE_RTC`, `USE_GPS`,
`USE_DALLAS_TEMP`, `USE_DHT_TEMP`, `USE_BME280`, `USE_BMP280`, `USE_AHTX0`, `USE_SHT21`, `USE_BH1750`,
`USE_MQTT`, `USE_WIFIMANAGER`, `USE_MASTER_CLOCK` (+`_TEMP`/`_HUMID`/`_RADAR`), `DISABLE_NIGHT_ANIMATION`,
`DISABLE_BROWNOUT`, `SHIFT_TUBES_LEFT_BY_1`, `LEFTDECIMAL`, `THERMOMETER_CLOCK`, `ENABLE_CLOCK_DISPLAY`,
`SIMULATE_TEMPERATURE_SENSOR` (csak teszthez), `DOUBLE_BLINK`, `MAKE_BLINKING_DOTS`.

**Pin-paraméterek** (drivertől függően eltérő névvel, de a gyakoriak): `PIN_SDA`/`PIN_SCL` (I2C),
`COLON_PIN`, `DP_PIN`/`DECIMALPOINT_PIN`, `ALARMSPEAKER_PIN`/`ALARMBUTTON_PIN`, `RADAR_PIN`,
`TUBE_POWER_PIN`, `LIGHT_SENSOR_PIN`, `LED_SWITCH_PIN`, `NEOPIXEL_PIN`, `PWM1_PIN`/`PWM2_PIN`/`PWM3_PIN`,
`TEMP_DALLAS_PIN`, `TEMP_DHT_PIN`.

**Megjelenítési paraméterek**: `MAXBRIGHTNESS`, `TEMP_START/END`, `HUMID_START/END`, `DATE_START/END`,
`DATE_REPEAT_MIN`, `ANIMSPEED`, `TEMP_CHARCODE`, `GRAD_CHARCODE`, `PERCENT_CHARCODE`,
`UPPER_CIRCLE_CHARCODE`/`LOWER_CIRCLE_CHARCODE`, `PLUS_CHARCODE`/`MINUS_CHARCODE`.

## 7. Driver-katalógus

| Driver-makró | Fájl | Chip / elv | CLOCK_NN (minta) |
|---|---|---|---|
| `MULTIPLEX74141`/`_ESP32` | multiplex74141*.ino | SN74141 BCD-dekóder, közös/multiplexelt | CLOCK_1,2,6A(nem),31,35 stb. |
| `NO_MULTIPLEX74141`/`_ESP32` | no_multiplex*.ino | SN74141, csövenként egy chip, nincs multiplex | CLOCK_6,36 |
| `PCF_74141` | pcf_multiplex74141.ino | SN74141 + PCF8574 I2C port-bővítő | CLOCK_7 |
| `MAX6921`/`_ESP32` | MAX6921*.ino | MAXIM MAX6921, 20-csat. VFD shift-register | CLOCK_3,4,5,14,15,15a,16,18,26,30,32,40-42,47 |
| `HV5122`(árva)/`NEWHV5122` | HV5122.ino / HV5122new.ino | HV5122 32-csat. HV shift-register, Nixie | CLOCK_50-57A |
| `PT6311` | PT6311.ino | Princeton PT6311, saját scan-generátor VFD | CLOCK_17 |
| `PT6355` | PT6355.ino | Princeton PT6355, saját scan-generátor VFD | CLOCK_12 |
| `SN75512` | SN75512.ino | TI SN75512, 2× kaszkád VFD driver | (review alatt) |
| `SN75518_ESP32` | SN75518.ino | TI SN75518, 32-csat. VFD driver | (review alatt) |
| `MM5450` | MM5450.ino | MM5450 34-csat. LED driver, saját ISR | CLOCK_11 |
| `MAX7219CNG` | MAX7219CNG.ino | MAX7219, saját belső multiplex+fényerő | CLOCK_10 |
| `Numitron_4511N` | Numitron_4511N.ino | CD4511/74HC4511 BCD→7seg, Numitron | CLOCK_8,9,13,46 |
| `VQC10` | VQC10.ino | VQC10 5×7 pontmátrix, 74HC595 lánc | CLOCK_71 |
| `TUBE1CLOCK` | 1tube.ino | 1 db Nixie cső, 74141-класс | (egycsöves óra) |
| `MULE_V2` | mulev2.ino | "Mule V2" board, SA40 LED kijelző | CLOCK_34 |
| `ZM1500_ESP32` | ZM1500.ino | 4×74HC595 + optocsatoló | **CLOCK_72 (AKTÍV)** |
| `WORDCLOCK` | wordclock.ino | WS2812 mátrix, szó-alapú időkijelzés (EN/DE/HU) | CLOCK_80,81,82 |
| `DUMMY` | dummy.ino | Csak szenzorbox, nincs fizikai kijelző | CLOCK_99 |

Minden sor részletei (interfész-implementáció, talált hibák) a [CODE_REVIEW.md](CODE_REVIEW.md)-ban,
ahogy az adott fájl review-ja elkészül.

## 8. Ismert, még nyitott architekturális kérdések

- **ZM1500.ino kritikus hiba** – az aktív driver jelenleg egy debug-célból bekerült sorral felülírja
  a kijelzendő számjegyeket, lásd [PROGRESS.md](PROGRESS.md) és [CODE_REVIEW.md](CODE_REVIEW.md).
  Felhasználói megerősítésre vár.
- A `HV5122.ino` fájl jelenleg egyetlen aktív `CLOCK_NN` konfiguráció által sem használt (mindenki a
  `NEWHV5122`-t/`HV5122new.ino`-t definiálja) – érdemes lehet DEPRECATED-jelölést kapnia.

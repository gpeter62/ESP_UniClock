# ESP_UniClock2 – Összesített code review

Ez a fájl gyűjti össze az egyes fájlok review-ja során talált hibákat, gyanús kódrészeket és
optimalizálási javaslatokat. A forráskódban a pontos helyükön `// POSSIBLE BUG:` / `// SECURITY:`
komment jelzi őket is.

Súlyossági jelölés: 🔴 kritikus (funkciót tör), 🟡 közepes, ⚪ apró/kozmetikai.

## Core (ESP32_UniClock2.ino) ✅ (2026-09-03, teljesen kommentezve, ~2932→~3337 sor)

- ✅ **JAVÍTVA (2026-09-03)** – `enableDisplay()` / `disableDisplay()` – mindkét függvény teljesen
  eltávolítva a core fájlból (a user kérésére), az összes hívási helyükkel együtt (ESP32_UniClock2.ino-ban
  22 db, `DHT_temp.ino`-ban 2 db), mivel mindkettő évek óta csak egy feltétel nélküli `return`-t
  tartalmazott ("NOT USED ANYMORE"／"DO NOT USE ANYMORE!!!"), a mögöttük lévő kód pedig sosem futott le.
  Ezzel együtt a kizárólag ezek által írt/olvasott `dState` és `lastDisable` globális változók is törölve
  lettek (teljesen feleslegessé váltak). Az `EEPROMsaving` flaget és a driver fájlokban lévő
  `if (EEPROMsaving) {...}` védelmeket NEM érintettük – azok önmagukban maradtak, csak jelenleg sosem
  aktiválódnak, mert semmi nem állítja `true`-ra (ez egy külön, nem javított megfigyelés, lásd lent).
  A `clearTubes()` driver-interfész függvényt a core mostantól sehonnan nem hívja (korábban is csak
  a most törölt `disableDisplay()`-en keresztül hívódott volna) – minden driverben megmaradt no-op-ként,
  az interfész-szerződés részeként, a kommentek frissítve lettek ennek megfelelően.
- ⚪ **Kapcsolódó, még nyitott megfigyelés**: mivel `disableDisplay()` törölve lett, és az `EEPROMsaving`
  változó az egyetlen élő kódágban sosem kapott `true` értéket (a korábbi `disableDisplay()` halott
  ágában lett volna beállítva), az `EEPROMsaving` mostantól garantáltan mindig `false` — vagyis az
  összes driver fájlban szereplő `if (EEPROMsaving) {...}` védelem (kb. 20 helyen, pl. `MAX6921.ino`,
  `HV5122.ino`, `multiplex74141*.ino`, `Numitron_4511N.ino`, `pwmleds.ino`, `z_neopixel.ino` stb.)
  gyakorlatilag holt kódág. Ez nem lett javítva/eltávolítva (nem volt kifejezett kérés rá), csak
  jelezve – ha az EEPROM-írás alatti kijelző-felfüggesztés valós funkció volt, ezt érdemes lenne
  pótolni valahol máshol (pl. `saveEEPROM()`-ban explicit `EEPROMsaving = true/false` beállítással).
- 🟡 `resetWiFi()` (~2913. sor) – korai `return` után holt kód (3 órás restart logika).
- 🟡 **`handleConfigChanged()` `"firmware"` ág** (~1440. sor) – a webes felületen a firmware-szerver URL
  puszta megváltoztatása AZONNAL valódi OTA-frissítést indít (`makeFirmwareUpdate = true`), a kód
  szerinti "for testing only" megjegyzéssel – fejlesztői/teszt-célú maradvány lehet, ami élesben
  meglepő mellékhatás (URL mentése = firmware-csere indítása).
- ⚪ `evalShutoffTime()` (~2830. sor) – korai `return` után elérhetetlen debug sorok.
- ⚪ `newCathodeProtect()` (~2140. sor) – a `switch(dir)` blokk kiszámolt
  `nextStoppedDigit` értékét egy közvetlenül utána következő feltétel nélküli értékadás felülírja,
  emiatt a `dir==0` (random) eset kezdő csöve valójában sosem random induláskor.
- ⚪ `testTubes()` (~3095. sor) – `digit[i] = i;` valószínűleg elgépelés,
  `digit[j] = i;` lett volna a szándék; gyakorlati hatása nincs (changeDigit() felülírja).
- ⚪ **Teljesen hívatlan (holt) függvények**, egyik sincs sehonnan meghívva élő kódágból:
  `findBestWifi()` (~665. sor, mindkét hívási helye ki van kommentezve), `startNewWifiMode()`
  (~700. sor), `checkWifiMode()` (~3010. sor, hívója is ki van kommentezve a `loop()`-ban), a régi
  `cathodeProtect()` (~2295. sor, a `newCathodeProtect()` váltotta le), `fifteenMinToHM()` (~2963. sor),
  `mod()` (~3006. sor), `playTubes()` (~3043. sor), `checkPrm()` (~3025. sor). Biztonságosan
  eltávolíthatók lennének kód-tisztítás során, ha nincs tervezett jövőbeli felhasználásuk.

## clocks.h ✅ (2026-09-03, saját kézzel kommentezve)
Header-blokk jelentősen kibővítve (a build-modell + driver-makró→fájl kereszttáblázat), a defaults
("fallback") szakasz elé magyarázó komment, a jelenleg aktív CLOCK_72 blokk elé figyelmeztető komment.
- ⚪ `clocks.h` ~2466-2470. sor (`TUBE_POWER_PIN`/`TUBE_POWER_ON` defaults) – a második `#ifndef` tévesen
  `TUBE_POWER_PIN`-t ellenőrzi `TUBE_POWER_ON` helyett, emiatt a `TUBE_POWER_ON` fallback-értéke soha nem
  lép életbe. Jelenleg ártalmatlan (minden CLOCK_NN blokk, amelyik `TUBE_POWER_PIN`-t hagyja
  definiálatlanul, vagy `TUBE_POWER_ON`-t is explicit definiálja, vagy sosem hivatkozik rá – a core
  kódban minden `TUBE_POWER_ON` használat `#if TUBE_POWER_PIN >= 0` mögé van zárva), de egy jövőbeli
  konfiguráció, amelyik csak `TUBE_POWER_PIN`-t állítana be `TUBE_POWER_ON` nélkül, fordítási hibát kapna.

## Driver fájlok (ügynökök jelentései alapján, ahogy megérkeznek)

### font5x7.h ✅ (2026-09-03)
Csak adattábla + felhasználó-felderítés, nincs funkcionális hiba. Kizárólag `wordclock.ino` használja
(`_font[]`, 256 karakter × 5 byte, column-major, LSB=felső pixel). Nem árva fájl.

### MAX6921.ino / MAX6921_ESP32.ino ✅ (2026-09-03)
MAXIM MAX6921, 20-kimenetű VFD driver, 3-vezetékes shift-register protokoll (DATA/CLK/LE) + BL (blank,
PWM fényerő). Használják: `MAX6921` → CLOCK_3,4,5,14,15,15a,16,18 (8266); `MAX6921_ESP32` →
CLOCK_26,30,32,40,41,42,47 (ESP32).
- ✅ **JAVÍTVA (2026-09-03)**: `digitEnableBits[10]` fix méretű tömb volt mindkét fájlban, míg
  `maxDigits` dinamikus – egy jövőbeli &gt;10 pozíciós config csendben túlindexelte volna
  `generateBitTable()`-ben. Javítva: mindkét fájlban `[12]`-re növelve (a `BUFSIZE`-hoz igazítva,
  ami a rendszer egyéb cső-puffereinek is a felső korlátja).
- ⚪ `MAX6921_ESP32.ino:38` `int DRAM_ATTR maxDig;` deklarálva, de sosem használva (holt kód).
- ⚪ `delayMS()` mindkét fájlban definiálva, de sehol nem hívva (minden hívási hely ki van kommentezve).
- ⚪ ESP8266 verzió `EEPROMsaving` alatt BL=HIGH-ra kényszeríti (elsötétít) a kijelzőt mentés közben,
  az ESP32 verzióban ez a sor ki van kommentezve → platformok között inkonzisztens viselkedés (nyitott kérdés: szándékos-e).

### Numitron_4511N.ino ✅ (2026-09-03)
CD4511/74HC4511 BCD→7-szegmens dekóder, csövenként egy chip, közös BCD-busz + cím-latch (LE) csövenként,
közös `LTBIpin` a teljes panel PWM-fényerejéhez (nincs pozíció-multiplex – minden cső folyamatosan világít).
Használják: CLOCK_8, CLOCK_9, CLOCK_13 (8266, IV-16), CLOCK_46 (ESP32, IV-9).
- ✅ **JAVÍTVA (2026-09-03) – off-by-one tömbtúlindexelés**: `writeDisplaySingle()`-ben mindkét ágban
  (ESP32 és ESP8266) `for (int pos=0; pos<=maxDig; pos++)` volt `pos<maxDig` helyett –
  `digitEnablePins[maxDig]` tömbön túli olvasás, meghatározatlan GPIO-számmal digitalWrite() hívás.
  Javítva mindkét helyen. (A kikommentezett legacy `clearTubes()`-blokkban ugyanez a minta megmaradt
  érintetlenül, mert az egy `/* ... */` közé zárt, sosem fordított kód – nem hiba.)
- 🟡 **Gyanús LE strobe-sorrend**: a kód LE=LOW→adat→LE=HIGH sorrendben strobol, a 4511 datasheet szerint
  LE=HIGH="átlátszó", LE=LOW="zárolt" – a szokásos hiba-mentes sorrend LE=HIGH→adat→LE=LOW lenne. Lehet,
  hogy a fizikai panelen (inverter vagy más bekötés miatt) mégis jól működik – **hardveres ellenőrzést igényel, nem javítottam.**
- ⚪ `PCB_VERSION` (1/3/32) ma sehol nincs kiolvasva élő kódágban – hatástalan paraméter, csak a törölt
  legacy blanking-logika használta.
- ⚪ Holt `int num;` változó mindkét writeDisplay()-ben.
- Megjegyzés: ez a driver NEM támogat tizedespontot/elválasztó pontot (nincs kiolvasva `digitDP[]`).

### PT6311.ino / PT6355.ino ✅ (2026-09-03)
Két KÜLÖNBÖZŐ Princeton Technology "okos" VFD-vezérlő chip, mindkettő saját belső display-RAM-mal és
scan-generátorral (ezért nincs ISR-alapú `writeDisplay()` – az no-op, a tényleges frame-küldés
`writeDisplaySingle()`-ben történik, `startTimer()` sincs hívva). PT6311 → CLOCK_17 (8266, IV-22 orosz óra).
PT6355 → CLOCK_12 (8266 VFD). A két fájl felépítése gyanúsan hasonló (copy-paste), pedig eltérő parancskészletű/keretezésű chipekről van szó.
- 🟡 `clearTubes()` mindkét fájlban üres no-op, a core `disableDisplay()` azonnali törlést várna – csak a következő `writeDisplaySingle()` hívásnál ürül ki ténylegesen.
- 🟡 PT6355.ino:321-327 – a decimálpont-kezelés ki van kommentezve, PT6311.ino-ban ugyanez a sor aktív → PT6355-ös órákon a `digitDP[]` sosem jelenik meg. (Szándékos-e, nyitott kérdés.)
- 🟡 PT6311.ino:44-63 `DISPLAY_MODE[]` fényerő-tábla – az 5./6. és a 8./9. index azonos parancsbájtot tartalmaz, ezek a fényerő-szintek megkülönböztethetetlenek a hardveren.
- ✅ **JAVÍTVA (2026-09-03) – PT6311.ino**: hiányos határellenőrzés `asciiConvert[]` indexelésnél –
  a `dispChar -= 32` feltétel nélkül lefutott a bounds-check után is, ami sikertelen leképezés esetén
  negatív indexet adott volna a `charDefinition[]`-be. Javítva: a kivonás csak sikeres lookup esetén
  fut le, egyébként `dispChar=0` (blank glyph) az eredmény.
- ✅ **JAVÍTVA (2026-09-03) – PT6355.ino**: ugyanez a minta (a kódja szinte szó szerint megegyezett a
  PT6311-ével ezen a ponton) itt is jelen volt, ugyanúgy javítva.
- ⚪ PT6355.ino:257-266 `TESTMODE` – holt kód (végtelen `while(true)` után elérhetetlen rész), csak debug buildet érint.

### HV5122.ino / HV5122new.ino ✅ (2026-09-03)
Mindkettő HV5122 32-csatornás HV shift-register/latch Nixie-katód-driver, 2 chip kaszkádolva (64 kimenet).
**Fontos**: a `clocks.h`-ban egyetlen aktív CLOCK_NN sem használja a sima `HV5122` makrót – mind a 9
HV5122-alapú config (CLOCK_50,51,52,53,54,55,56,57,57A) a `NEWHV5122` makrót definiálja, tehát
**`HV5122.ino` jelenleg árva/legacy fájl, semmi nem fordítja be**. A `HV5122new.ino` az aktuális, funkcionálisan
bővebb verzió (3-fázisú animáció/wipe-támogatás `animMask`-kal, memcmp-cache), a régi csak egyszerű ON/OFF PWM-et tud, animáció nélkül.
- 🟡 `HV5122new.ino:241-253` – a `writeDisplaySingle()` korai-return cache-ellenőrzése (`memcmp`) nem veszi figyelembe a `digitDP[]` változását → ha csak a tizedespont/villogó pont állapota változik (pl. `MAKE_BLINKING_DOTS`, CLOCK_52/54), a pont-frissítés elmaradhat, "beragadhat".
- ⚪ Mindkét fájlban a DP-ciklus csak `maxDigits-1`-ig megy a `digitPins[maxDigits][]` 7 oszlopos sorából – szándékos-e (utolsó cső nem kap pontot) vagy off-by-one, nyitott kérdés.
- 💡 Javaslat: mivel `HV5122.ino`-t semmi nem használja élesben, érdemes lenne DEPRECATED-jelöléssel ellátni vagy törölni, nehogy véletlenül azon dolgozzon valaki a `HV5122new.ino` helyett.

### MM5450.ino / MAX7219CNG.ino ✅ (2026-09-03)
MM5450 (34-kimenetű LED driver, saját ISR-alapú multiplex+szoftver-PWM, `writeDisplay()` valódi ISR) és
MAX7219CNG (klasszikus MAX7219, saját belső multiplex+HW fényerő-regiszter, nincs ISR, `writeDisplaySingle()`
pusszol mindent egyszerre). Használat: `MM5450`→CLOCK_11, `MAX7219CNG`→CLOCK_10 (mindkettő jelenleg
kikommentezve, egyik sincs aktívan fordítva). Mindkettő csak 8266-ra (`#error` guard ESP32 ellen).
- 🟡 MM5450.ino `writeDisplay()` – `pos = 7-(4*side+i)` mindig 0-7 közé indexel `digit[]`-be, de `maxDigits=6` –
  a `digit[6]`/`digit[7]` sosem íródik a core által (nem inicializált 0 érték → mindig '0' minta), ártalmatlan ha csak 6 fizikai digit van bekötve, de tisztázandó.
- ⚪ MM5450.ino – duplikált `bitBuffer[sideEnablePins[side]] = 0;` sor, feltehetően felesleges copy-paste.
- ⚪ MAX7219CNG.ino `sendBits()` – digitenként `delay(3)` (3ms), miközben a chip ~50ns CS impulzust igényelne; kb. 18ms felesleges blokkolás egy teljes kijelzőfrissítésnél.
- ⚪ MM5450.ino – az animMask/wipe-animáció támogatás ki van kommentezve és a hivatkozott tömb nincs is definiálva ebben a fájlban → nincs wipe-animáció ennél a drivernél.

### VQC10.ino ✅ (2026-09-03)
5×7 pontmátrixos VQC10 modul, teljes ASCII-támogatással (saját `_font[]`), 74HC595 shift-out lánc,
soronkénti (nem digitenkénti) multiplex. `writeDisplay()` (ISR) szándékosan üres no-op – a frissítés a
kooperatív `writeDisplay2()`-ben történik (mert soronkénti PWM-hez `delayMicroseconds()` kell, ami ISR-ben
nem biztonságos), ezért `startTimer()` sosem hívódik ennél a drivernél. Egyetlen használó: CLOCK_71.
- ✅ **JAVÍTVA (2026-09-03)**: `writeDisplay2()` – `loopMax = maxDigits` (=8) volt a sor-multiplex
  ciklusban `NUMROWS` (=7) helyett – a ciklus egy 8. (nem létező) sort is kiírt, ami az utolsó digitnél
  ténylegesen a `dat[]` tömbön túli memóriaolvasáshoz vezetett (`oldDat[0][0]`-ba nyúlt) és extra
  sorválasztó vonalat aktivált → villódzás/ghosting kockázat és elvesztegetett PWM-idő minden
  frissítésnél. Javítva: `loopMax = NUMROWS;`.
- ⚪ Ehhez kapcsolódóan az `offState` számítás `(iRow == maxDigits)` feltétele még mindig sosem igaz –
  ez holt, de ártalmatlan (nem tömbindexelési) kódág, szándékosan nem nyúltam hozzá.
- ⚪ `cDigit[]` és `PWMrefresh` makró sehol nem használt holt kód; `_upsidedown` sosem kap `true` értéket sehonnan (sem itt, sem wordclock.ino-ban).
- ⚪ `D5pin` bemenetnek van konfigurálva és regisztrálva, de sehol nincs kiolvasva – célja tisztázatlan.

### gps.ino ✅ (2026-09-03)
NMEA GPS időszinkron (TinyGPS++), csak RX (nem küld parancsot a modulnak), WiFi nélküli önálló órákhoz.
Aktívan használja: CLOCK_1, CLOCK_13 (8266). Opcióként elérhető, de kikommentezve: CLOCK_50, 71, 80, 81, 82, 99 (ESP32).
- 🟡 A GPS-ágon a nyári/téli időszámítás **fix +1 órás eltolással** megy (`prm.enableDST` kapcsoló alapján),
  NEM naptári dátum szerint automatikusan, szemben a WiFi/NTP ággal (ami a `Timezone` könyvtár szabályai
  szerint automatikusan vált) – egy tisztán GPS-es, internet nélküli órán a DST-váltás emiatt nem történik
  meg magától a naptári határnapon, kézi átállítás kell.
- ⚪ `GPSexist` a soros port megnyitása után rögtön `true`-ra áll, függetlenül attól, hogy érkezett-e valaha
  érvényes NMEA adat – inkább "GPS funkció be van fordítva" jelentésű, mint tényleges hardver-detektálás.
- ⚪ Nincs fix-minőség/műholdszám ellenőrzés, csak `time.isValid()`/`isUpdated()` – lehet szándékos.
- 💡 Nyitott kérdés: a `CLOCK_1`-ben szereplő `SPEEDOMETER`/`SPEEDOMETER_SWITCH` makrók sehol a kódbázisban
  nincsenek ténylegesen felhasználva – befejezetlen/elavult funkciónak tűnik.

### I2Csensors.ino ✅ (2026-09-03)
BME280/BMP280/AHTX0/SHT21/BH1750 közös I2C-buszos kezelése, egyszerre több szenzor is aktív lehet – három
független számláló (`useTemp`/`useHumid`/`usePress`) osztja ki a slotokat a `temperature[]`/`humid[]`/`pressur[]`
tömbökben, nincs kollízió. `USE_I2CSENSORS` automatikusan levezetett makró (clocks.h), egyetlen CLOCK_NN sem
állítja be közvetlenül.
- 🟡 SHT21 detektálás a helyes 0x40 (7-bit) cím mellett a 0x80-at is próbálja, ami valószínűleg 8-bit/7-bit
  címzés-keveredésből eredő felesleges/értelmetlen ellenőrzés (ártalmatlan, mert az `&&` logika miatt a helyes
  találat is beállítja a flaget).
- ⚪ `BMP280humidPtr` deklarálva, de sosem használt holt kód (BMP280-nak nincs páratartalom-mérése).
- 💡 Fontos platform-szintű korlát (nem ebben a fájlban): a core `displayTemp()`/`displayHumid()` csak a 0. és
  1. szenzor-slothoz alkalmazza a `prm.corrT0/corrT1/corrH0/corrH1` kalibrációt – egy 3. vagy 4. egyidejű
  szenzor (pl. CLOCK_26 SHT21-je a temp[3] slotban) korrekció nélkül jelenik meg.

### multiplex74141.ino / multiplex74141_ESP32.ino ✅ (2026-09-03)
Közös/multiplexelt SN74141 Nixie driver, 8266 ill. ESP32 változat. Használat: CLOCK_1,2,20,21,22 (8266);
CLOCK_23,23A,23B,31,33,43,44,45 (ESP32).
- ⚪ ESP32 változat: `setup_pins()`-ből hiányzik a `regPin(DP_PIN,"DP_PIN")` hívás (a 8266 testvérnél megvan) → DP_PIN kimarad a diagnosztikai pin-listából, pedig ténylegesen meghajtott láb.
- ⚪ `PWMrefresh`/`PWM_min`/`PWM_max` kommentjei "msec"-et írnak, valójában nyers timer-tick érték – csak elnevezési pontatlanság.

### no_multiplex74141.ino ✅ (2026-09-03)
Nem multiplexelt SN74141 driver (csövenként egy chip, közös 74HC595 lánc), 8266-only. Használat: CLOCK_6, CLOCK_6A.
- 🟡 Az egyes csövek `animMask[i]` (wipe-animáció progresszió) értékeit egyetlen globális `animM` változóba tömöríti – ha egyszerre több cső animálna eltérő maszk-értékkel, csak az egyik időzítése lesz helyes (a multiplex-driverekkel ellentétben, ahol minden csőnek saját ISR-passzusa van).
- ⚪ `clearTubes()` üres no-op (szándékosan, a fejléc szerint a blanking `writeDisplay()`-en belül történik).

### wordclock.ino ✅ (2026-09-03)
WS2812 szódíszóra, 11×11 LED-mátrix, kígyózó (boustrophedon) bekötés, `font5x7.h` újrahasznosítva
szám/overlay kijelzéshez. Használat: CLOCK_80 (EN), CLOCK_81 (DE), CLOCK_82 (HU) – közös driver, csak a
`matrix[][]`/`sequence[][]`/`words[]`/`hours[]` nyelvi táblák és `START_CORNER` térnek el clocks.h-ban.
- 🟡 `convertXYtoPOS()` – a RIGHTUP ág XMAX-ot használ ugyanahhoz a sor-tükrözési számításhoz, ahol a
  LEFTUP ág YMAX-ot, és a zig-zag (`y%2`) teszt polaritása is fordított a két ág között, nyilvánvaló ok
  nélkül. Jelenlegi négyzet panelen (XMAX==YMAX==11) ártalmatlan, de nem-négyzet panelen hibás lehet.
  **Fizikai RIGHTUP-bekötésű hardveren (CLOCK_81/82) igazolást igényel.**
- ⚪ `disp()` – egy `tmp` statikus buffer + `strncpy`/`strcmp` logika mára maradványkód (egyetlen felhasználása, egy DPRINT, ki van kommentezve) – kockázat nélkül törölhető.

### mqtt.ino ✅ (2026-09-03)
Home Assistant/MQTT integráció (ArduinoHA), master/slave szenzor-megosztás (egy "master" óra publikálja a
szenzorait, mások feliratkoznak rá `USE_MASTER_CLOCK`/`_TEMP`/`_HUMID`/`_RADAR`-ral). Használat: CLOCK_99 (master), CLOCK_26 (publikál is, feliratkozik is).
- 🔴 **SECURITY**: a core `factoryReset()` hardkódolt gyári MQTT broker-hitelesítést állít be:
  `prm.mqttBrokerAddr="10.10.0.202"`, user/pass = `"mqtt"`/`"mqtt"` – valós IP és triviális jelszó a firmware-be égetve.
- 🔴 **`onMqttMessage()`**: `memcpy(tmp,payload,length)` a bróker által jelentett `length`-et HATÁRELLENŐRZÉS
  NÉLKÜL másolja egy fix 100 bájtos `tmp` bufferbe → ≥100 bájtos payload esetén stack-túlcsordulás; a buffer
  emellett nincs null-terminálva, mielőtt C-stringként használnák (`atof(tmp)` stb.) – definiálatlan viselkedés kockázata.
- 🟡 A topic-összehasonlítás `strncmp(...,sizeof(masterXTopic))`-mal fut, ami gyakorlatban strcmp-ként viselkedik, de törékeny 80 karakternél hosszabb topic esetén.

### rtc.ino ✅ (2026-09-03)
DS3231 RTC (Rtc_by_Makuna), NTP/GPS tartalék időforrás, opcionális kézi gombos beállító állapotgép.
Használat: CLOCK_5,15a,17,35,52,71 (gombkezelés RTC nélkül is), CLOCK_80/81/82 (RTC igen, gombok nem).
- ⚪ `scanButSET()` – `if (fld>MAXFLD) fld = 0;` valószínűleg hibás/másolt maradványkód (a mezőválasztó
  értékét állítja vissza a mező-érték helyett; a feltétel gyakorlatilag sosem igaz).
- ⚪ Nincs szökőév/hónaphossz-ellenőrzés kézi dátumbeállításnál (pl. Feb 30 is beállítható és elmenthető).
- ⚪ `I2C_ClearBus()` élő, de gyakorlatilag holt kód – egyetlen hívási helye (I2Csensors.ino) ki van kommentezve, az I2C-busz-helyreállítás sosem fut le ma.
- ⚪ `MENU_UNSELECT_TIMEOUT` definiálva, de sehol nem használt.

### dallas_temp.ino ✅ (2026-09-03)
DS18B20 1-wire driver (OneWire+DallasTemperature), max 2 szenzor, nem-blokkoló kérés/lekérdezés minta.
Használat: nagyon sok CLOCK_NN (CLOCK_3,5,6A,7,12,14,15,15a,16,20,26,30,31,35,40-49,51,53-57A,71).
- 🟡 Sikeres olvasásnál a kód `temperature[0]`-ba ír `temperature[d1Ptr]` helyett – ma egyenértékű (mert
  a Dallas-driver mindig elsőként inicializálódik, `d1Ptr`=0 mindig), de törékeny, ha valaha megváltozna a
  setup-sorrend vagy más szenzor is 0-ás slotot kapna.
- 🟡 A "szenzor hiányzik" ágban `temperature[d2Ptr] = 0;` FELTÉTEL NÉLKÜL fut akkor is, ha csak 1 szenzor
  van konfigurálva (`d2Ptr` ilyenkor alapértelmezett 0-n marad) – ha épp egy másik szenzor-driver írja a 0.
  slotot, ez felülírhatja azt. A sikeres-ág helyesen védi ezt `if (useDallasTemp>1)`-fel, a hiba-ág nem.
- ⚪ `setupDallasTemp()` végi `requestDallasTemp(true); delay(200); getTemp();` csak 200ms-et vár, de a
  konverzióhoz 800ms kell – a látszattal ellentétben NEM tölti fel előre a hőmérsékletet setup() végére.
- ⚪ `SENSOR1`/`SENSOR2` makrók, `lastTemperature`/`lastTemperatureStr` – holt/maradvány kód.

### DHT_temp.ino ✅ (2026-09-03)
DHT11/22/21 driver (Adafruit DHT lib), 1 szenzor, blokkoló olvasás (~250ms), csak hívási gyakorisággal
korlátozva. Használat: CLOCK_18,21,22,23A,23B,32,50,99. Nem talált hiba – tiszta, kis fájl.

### z_neopixel.ino ✅ (2026-09-03)
WS2812/WS2812B címezhető RGB háttérvilágítás (NeoPixelBrightnessBus), `tubePixels[]` (clocks.h)
alapján rendeli a fizikai LED-eket logikai cső-pozíciókhoz. Nagyon sok CLOCK_NN használja (a `USE_NEOPIXEL`
konfigurációk többsége). `doAnimationMakuna()` minden `loop()`/`Fdelay()` iterációban hívódik.
- ✅ **JAVÍTVA (2026-09-03)**: `PixelCount = sizeof(tubePixels)+2` (2 extra "tartalék", mindig sötét pixel
  hely), de mind a 6 érintett függvény (`alarmLight()`, `rainbow()`, `effect1()`, `effect2()`,
  `setPixels()`, `fixColor()`) közvetlenül `tubePixels[i]`-vel indexelt `i` egészen `PixelCount-1`-ig –
  vagyis a 2 tartalék pozíción a `tubePixels[]` tömbön TÚLI memóriát olvasta (undefined behavior),
  és csak szerencséből "működött" (ha a szomszédos memória épp `>=maxDigits` értéket tartalmazott).
  Javítás: bevezetve egy `tubeForPixel(i)` segédfüggvény, ami a tömbön túli pozíciókra biztonságos
  255-ös sentinel-t ad vissza (mindig `>=maxDigits`, megőrizve az eredeti "mindig sötét" viselkedést)
  – az összes 6 hívási hely erre lett átállítva.
- ⚪ `cnt` változó sosem inkrementálódik (`effect1()`-ben), így a `cnt%3==0` feltétel mindig igaz – nem
  hiba a jelenlegi kimenetre nézve (a másik OR-ág amúgy is szinte mindig igaz), de valószínűleg hiányos/
  befejezetlen logika.
- ⚪ `colorGamma` (NeoGamma) objektum deklarálva, de sosem használva (`.Correct()` sehol nincs hívva) –
  a színek lineárisan, gamma-korrekció nélkül mennek ki, valószínűleg egy be nem fejezett fejlesztés maradványa.

### mulev2.ino ✅ (2026-09-03)
"Mule V2" ESP32 board, 6× SA40 7-szegmenses LED kijelző. CD4543 (BCD→7seg) + CD4028 (BCD→decimál,
digit-select) multiplex, PWM fényerő az ON-idő szabályozásával. Egyetlen használó: **CLOCK_34, jelenleg
kikommentezve (nincs aktívan bekapcsolva egyik konfigurációban sem)**.
- ✅ **JAVÍTVA (2026-09-03)**: `setup_pins()` – a pin-inicializáló ciklus korábban `maxDigits` (=6)
  hosszan futott a `digitEnablePins[]` tömbön, ami a clocks.h-ban valójában csak **3 elemű** (CD4028
  3 select-vonala, nem digitenkénti enable-pin) → i=3,4,5-nél tömbön túli olvasás, `pinMode()`/`regPin()`
  szemét byte-értékkel hívódott. Javítva: a ciklus most `sizeof(digitEnablePins)`-t használ, konzisztensen
  a fájl többi helyével.

### pwmleds.ino ✅ (2026-09-03)
Nem-címezhető RGB háttérvilágítás, ESP32 LEDC PWM 3 lábon (R/G/B), egyetlen közös szín/fényerő (nem
pixelenkénti, ellentétben a z_neopixel.ino-val). Egyetlen használó: CLOCK_54. A pin-setup (`ledcAttachPin`/
`ledcSetup`) NEM ebben a fájlban, hanem a core `setup()`-jában történik.
- 🟡 **Architektúra-védelem hiánya**: a LEDC API tisztán ESP32-specifikus, se ez a fájl, se a core
  PWM-setup blokkja nincs `#if defined(ESP32)` védelemmel ellátva, csak `#ifdef USE_PWMLEDS` – ha valaha
  egy ESP8266-os confighoz kapcsolnák be `USE_PWMLEDS`-t, fordítási hiba lenne.
- ⚪ Holt kód: `pwmColorStep` sosem használt (mindig 1-es lépésköz), globális `pwmRed/Green/Blue` mindig
  elfedve egy azonos nevű lokális változó által, `COLORSATURATION` makró sehol nem hivatkozott.
- 💡 Nyitott kérdés: `USE_NEOPIXEL` és `USE_PWMLEDS` egyidejű bekapcsolása makróütközést okozna (`FPS`,
  `FPS_MSEC`, `COLORSATURATION`, `WHITE_INDEX` mindkét fájlban azonos néven) – jelenleg nincs erre explicit védelem.

### SN75512.ino ✅ (2026-09-03)
2× kaszkádolt TI SN75512 VFD shift-register+driver, bit-bangelt DATA/CLK+LE, STROBE-alapú fényerő-dimmelés
(digitenkénti ki/be-kapcsolással). **Egyik CLOCK_NN sem használja jelenleg** – árva/legacy driver (a
core fájl driver-választó listájában is csak kikommentezve szerepel).
- 🟡 `clearTubes()` és `writeDisplaySingle()` üres stub-ok, semmit nem csinálnak – ha valaha aktiválnák ezt a drivert, csendben hibásan működne (a közös interfész-szerződést nem teljesíti).
- ⚪ `charTable[]` indexelés (`digit[pos]`) nincs explicit határellenőrzéssel védve (MAXCHARS=21) – csak látens kockázat, ISR-teljesítmény miatt szándékosan nincs védve.
- ⚪ `VFDrefresh` komment "msec"-et ír, valójában timer1-tick.

### SN75518.ino ✅ (2026-09-03)
TI SN75518 (32-kimenetű) VFD driver, bit-bang DATA/CLK+LE+STROBE, valódi változó-periódusú (mark-space)
PWM fényerőszabályzás ESP32 hw timerrel. Kétféle glyph-tábla (`SEGMENT8` 7-szeg+DP vagy 16-szeg+DP ASCII).
Használat: **CLOCK_48** (6×IV-17, 16-szeg mód), **CLOCK_49** (6×IV-11, SEGMENT8 mód).
- ✅ **JAVÍTVA (2026-09-03)**: az ASCII-mód glyph-konverziójában a `dispChar -= 32;` korábban feltétel
  nélkül lefutott a bounds-check után is → sikertelen leképezés esetén negatív indexszel olvasta volna
  a `charTable[]`-t. Javítva: a kivonás csak sikeres lookup esetén fut le, egyébként `dispChar=0` (blank glyph).
- ⚪ A 16-szegmenses animációs maszk-építésnél nincs "-1" korrekció a `segmentEnablePins[]`-re, szemben a fájl többi részével (Q1..Q32 1-alapú számozás miatt máshol mindenhol van) – működik, de inkonzisztens, törékeny jövőbeli pin-átszervezésnél.
- ⚪ `writeDisplaySingle()` itt is üres stub (az SN75512-vel ellentétben `clearTubes()` viszont ténylegesen letiltja a STROBE-ot, tehát működik).
- ⚪ `delayMS()` holt kód (senki nem hívja élőben, és a `d` paramétert is figyelmen kívül hagyná).
- ⚪ clocks.h `PIN_STB` makró (CLOCK_48/49) sehol nincs ténylegesen felhasználva – a driver saját `PIN_STROBE`-ot definiál ugyanazzal az értékkel, `PIN_STB` redundáns/holt config.

### no_multiplex_ESP32.ino ✅ (2026-09-03)
`no_multiplex74141.ino` ESP32 portja (nem multiplexelt, csövenként saját 74141, közös 74HC595 lánc).
Használat: CLOCK_35, CLOCK_36 (mindkettő jelenleg kikommentezve – nincs aktívan bekapcsolva).
- 🟡 Ugyanaz a globális `animM` korlát, mint az ESP8266 testvérnél (per-cső animáció-időzítés nem lehetséges).
- ⚪ `maxDigits` nincs `DRAM_ATTR`-ral jelölve, míg az ISR közvetlenül olvassa – eltér a család többi ESP32-tagjától (ott külön `DRAM_ATTR maxDig` másolat van), valószínűleg ártalmatlan, de inkonzisztens.

### pcf_multiplex74141.ino ✅ (2026-09-03)
Multiplexelt SN74141 driver + PCF8574 I2C port-bővítő a tubusválasztáshoz (saját, kézzel bit-bangelt
I2C, mert `Wire.h` nem hívható biztonságosan ISR-ből). Egyetlen használó: **CLOCK_7, jelenleg
kikommentezve** – árva/nem tesztelt, a család legrégebbi, legkevésbé karbantartott tagja.
- 🟡 A blank-ágban a tubus enable-vonal (`p`) SOSEM kerül vissza LOW-ra – közvetlen ESP GPIO-s
  enable-vonalaknál (`p<100`, amit a `setup_pins()` kevert konfigurációhoz kifejezetten támogat) ez azt
  jelentené, hogy az első bekapcsolás után a tubus örökre bekapcsolva marad ("szellemkép" más csövek
  digitjeinél). PCF-hosztolt csöveknél (`p>=100`) ártalmatlan, mert a `sendBits()` mindig a teljes
  portbájtot felülírja. A jelenlegi (kizárólag PCF-es) config ezt nem futtatja meg, ezért maradt észrevétlen.
- 🟡 `PWMtimeBrightness` a `brightness`-ből számolódik, MIELŐTT a `brightness` frissülne – mindig egy
  ISR-ciklussal késik; ráadásul minden egyes híváskor újraszámol, nem csak a ciklus elején (`pos==0`),
  szemben a család többi tagjával – elméletben csövenként eltérő fényerőt okozhatna egy cikluson belül.
- ⚪ Egy felesleges, párosítatlan záró `}` a DECIMALPOINT_PIN ág `else`-je után – jelenleg ártalmatlan (a blokk valószínűleg sosem fordul be ennél a drivernél), de látens fordítási kockázat.
- ⚪ `shiftout()` – az I2C ACK-bitet sosem olvassa vissza ténylegesen (a mintavételező rész ki van kommentezve), vakon feltételezi a sikeres ACK-ot.

**Az 5 SN74141-családbeli fájl viszonya** (multiplex74141*.ino, no_multiplex*.ino, pcf_multiplex74141.ino):
három tengely mentén térnek el – (1) multiplexelt (1 közös 74141, időosztásos, kisebb átlagfényerő,
kevesebb chip) vs. nem-multiplexelt (csövenként 1 chip, folyamatos fény, nincs villódzás, de nincs
per-cső animáció-időzítés sem); (2) ESP8266 vs. ESP32 platform (funkcionálisan ekvivalens párok, kivéve
a PCF-es változat, aminek nincs ESP32 portja); (3) PCF8574 I2C-bővítő használata, ha elfogynak a szabad
GPIO-k sok cső multiplexeléséhez. Részletes döntési szempontok: lásd a family-agent teljes jelentését
(a `git log`/session-transcriptben, illetve DEVELOPER_GUIDE.md driver-katalógusában összefoglalva).

### ZM1500.ino ✅ (2026-09-03, saját kézzel kommentezve; 2026-09-03-án lecserélve)
4× 74HC595N shift-register + optocsatolók, egyszerű 3-vezetékes (data/clk/latch) bitbang protokoll.
**Jelenleg AKTÍV driver (CLOCK_72).** A korábbi `ZM1500.ino` egy félbehagyott, befejezetlen fejlesztési
kísérlet volt (kettős PCB-huzalozás támogatás, `SEGMENTS_FIRST` kapcsolóval) – a user tisztázta ezt, a
fájl törölve lett, és a `ZM1500 .sav` (az utolsó ismert működő verzió) lett `ZM1500.ino` névre átnevezve.
A `clocks.h` CLOCK_72 blokkjából a `SEGMENTS_FIRST` definíció is eltávolítva, mivel az új/visszaállított
driver nem használja. Nincs ismert hiba ebben a verzióban.

(a többi fájl jelentése ide kerül, ahogy az ügynökök végeznek – lásd docs/PROGRESS.md "Dispatched agent batches" táblázatát a státuszért)

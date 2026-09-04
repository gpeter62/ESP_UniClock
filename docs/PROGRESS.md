# ESP_UniClock2 – Fejlesztői dokumentáció készítésének státusza

Ez a fájl a review/kommentezés/dokumentálás előrehaladtát követi. Ha a feldolgozás megszakad, ebből a
fájlból lehet folytatni: nézd meg a "Jelenleg folyamatban" és "Hátralévő teendők" szakaszokat.

Utolsó frissítés: 2026-09-03

## Összefoglaló állapot

A teljes kódbázis (minden .ino/.h fájl, a lent listázott "kétes" fájlok kivételével) át van olvasva,
kikommentezve és review-zva. Részletek: [DEVELOPER_GUIDE.md](DEVELOPER_GUIDE.md) (architektúra),
[CODE_REVIEW.md](CODE_REVIEW.md) (összes talált hiba/javaslat).

**A ZM1500.ino kérdés lezárva**: a user tisztázta, hogy a korábbi `ZM1500.ino` egy félbehagyott,
befejezetlen fejlesztési kísérlet volt (a "SEGMENTS_FIRST" kettős-huzalozás támogatás). Ez törölve lett,
és a `ZM1500 .sav` (az utolsó ismert működő verzió) lett átnevezve `ZM1500.ino`-ra. A `clocks.h`
CLOCK_72 blokkjából a most már felesleges `SEGMENTS_FIRST` definíció és a hibára figyelmeztető
kommentek eltávolítva. Az összes ezzel kapcsolatos hiba-bejegyzés törölve a dokumentációból.

## ✅ 2026-09-03 késő délután: minden kért utómunka elkészült
1. ✅ ZM1500.ino csere (törlés + .sav átnevezése) + clocks.h hivatkozások javítása + doksi-tisztítás
2. ✅ Összes azonosított túlindexelési (out-of-bounds) hiba ténylegesen javítva a kódban (nem csak jelölve):
   `Numitron_4511N.ino` (2 hely), `VQC10.ino`, `mulev2.ino`, `MAX6921.ino`/`MAX6921_ESP32.ino`,
   `PT6311.ino`, `PT6355.ino`, `SN75518.ino`, `z_neopixel.ino` (6 hívási hely egy közös segédfüggvénnyel).
   Részletek soronként: [CODE_REVIEW.md](CODE_REVIEW.md).
3. ✅ `enableDisplay()`/`disableDisplay()` teljesen eltávolítva (a user külön kérésére) – a két függvény,
   minden hívási helyük (~24 db, `ESP32_UniClock2.ino` + `DHT_temp.ino`), és a hozzájuk tartozó, szintén
   feleslegessé vált `dState`/`lastDisable` globális változók törölve. Kapcsolódó felfedezés: mivel
   `EEPROMsaving` egyetlen élő kódágban sem kapott valaha `true` értéket, az összes driverben lévő
   `if (EEPROMsaving) {...}` védelem gyakorlatilag holt kódág – ezt NEM javítottuk, csak dokumentáltuk.
4. ✅ Minden .ino/.h fájl kommentjeinek angol nyelvi lektorálása megtörtént (5 párhuzamos ügynökkel,
   fájlcsoportonként). Talált és javított problémák: elgépelések, hiányos mondatok, néhány valódi magyar
   szórészlet (főleg `clocks.h`-ban, pl. "átkötés", "előjel", "tizedfok", "fok10", "fok1", "századfok",
   illetve a core fájlban a "datum"→"date" elgépelés 3 helyen). Ellenőrizve: a kódbázisban nem maradt
   véletlenül otthagyott magyar szöveg kommentekben (csak szándékos, funkcionális elemek: pl. `WEBNAME`
   string-literálok, amik a weblapon jelennek meg, a wordclock.ino szándékos magyar szó-példái, és a
   font5x7.h/VQC10.ino betűkészlet-táblák német ékezetes karaktereinek dokumentációja).

## Legenda
- ✅ Kész | 🔶 Folyamatban | ⬜ Nincs elkezdve | ⚠️ Kész, de nyitott kérdés van vele kapcsolatban

## Alapfájlok

| Fájl | Státusz | Megjegyzés |
|---|---|---|
| readme.md | ✅ | Elavult (v3.1), a repó azóta sokat fejlődött |
| clocks.h | ✅ | Konfigurációs mátrix, ~65 klóndefiníció – katalogizálva (DEVELOPER_GUIDE.md 6. fejezet) |
| ESP32_UniClock2.ino | ✅ | Fő program (setup/loop, WiFi, webszerver, animációk) – dokumentálva + kikommentezve |
| font5x7.h | ✅ | Karakterkészlet – 256×5 byte column-major bitmap font, kizárólag wordclock.ino használja |

## Driver / periféria fájlok

| Fájl | Státusz | Megjegyzés |
|---|---|---|
| MAX6921.ino / MAX6921_ESP32.ino | ✅ | VFD driver (8266 / ESP32) |
| MAX7219CNG.ino | ✅ | LED mátrix driver |
| MM5450.ino | ✅ | LED driver |
| HV5122.ino | ✅ | Nixie HV driver – **jelenleg árva, semmi nem használja (mind NEWHV5122-t definiál)** |
| HV5122new.ino | ✅ | Nixie HV driver (aktuális változat) |
| PT6311.ino / PT6355.ino | ✅ | VFD driver |
| SN75512.ino | ✅ | VFD driver – **jelenleg árva, semmi nem használja** |
| SN75518.ino | ✅ | VFD driver – CLOCK_48, CLOCK_49 |
| Numitron_4511N.ino | ✅ | Numitron (CD4511) driver |
| ZM1500.ino | ✅ | Jelenleg AKTÍV driver (CLOCK_72) – lásd fent, a korábbi hibás verzió lecserélve |
| multiplex74141.ino / _ESP32.ino | ✅ | SN74141 multiplexelt driver (8266 / ESP32) |
| no_multiplex74141.ino / _ESP32.ino | ✅ | SN74141 nem-multiplexelt driver (8266 / ESP32) |
| pcf_multiplex74141.ino | ✅ | SN74141 + PCF8574 tube-select driver |
| VQC10.ino | ✅ | VQC10 LED driver |
| 1tube.ino | ✅ | Egy csöves óra változat |
| mulev2.ino | ✅ | Mule V2 LED driver |
| z_neopixel.ino | ✅ | WS2812 háttérvilágítás animációk |
| pwmleds.ino | ✅ | PWM LED háttérvilágítás |
| wordclock.ino | ✅ | Szóóra (word clock) logika |

## Szenzor / kommunikáció fájlok

| Fájl | Státusz | Megjegyzés |
|---|---|---|
| rtc.ino | ✅ | RTC (DS3231) + gombkezelés |
| gps.ino | ✅ | GPS időszinkron |
| dallas_temp.ino | ✅ | DS18B20 hőmérséklet |
| DHT_temp.ino | ✅ | DHT11/22 hő+pára |
| I2Csensors.ino | ✅ | BME280/BMP280/AHTX0/SHT21/BH1750 |
| mqtt.ino | ✅ | MQTT / Home Assistant integráció |

Részletes hibalista minden fájlhoz: [CODE_REVIEW.md](CODE_REVIEW.md).

## Egyéb / kétes állapotú fájlok
**A user kérésére ezekkel NEM foglalkozunk.**

| Fájl | Megjegyzés |
|---|---|
| dummy.ino | 20 sor – placeholder/teszt? |
| mqtt.old | Régi verzió, valószínűleg nem fordul be |
| rtc.ino.old | Régi verzió |
| samsung.ino.sav | .sav kiterjesztés, nem .ino – nem fordul be |

## ✅ readme.md frissítve (2026-09-03)
A GitHub-on elsőként megjelenő readme.md teljesen átdolgozva: tartalomjegyzék, pontos és teljes
driver-táblázat (minden ~15 driver-chip felsorolva, a review során feltárt tényleges lista alapján),
Requirements szakasz kiemelve a tesztelt verziókkal (**Arduino IDE 1.8.19**, **ESP32 board package
2.0.17**), Getting Started lépések, hivatkozás a docs/ fejlesztői dokumentációra, Known Issues szakasz
(mqtt.ino biztonsági megjegyzésére mutatva), License (GPL-3.0, a forráskód fejléce alapján), teljes
könyvtár-lista a libraries/ mappa alapján.

## ✅ Manuálok frissítve és bővítve (2026-09-04)
`UniClock Manual ENG v3.1.docx` és `UniClock Manual HUN v3.1.docx` (átnevezve a régi `...v2_5.docx`
nevekről, mert a user jelezte, hogy a valós aktuális verzió 3.1, nem 2.5). XML-szintű szerkesztéssel
készült (ebben a környezetben nincs LibreOffice/pandoc renderer), a séma-validáció mindkét fájlnál
hibátlanul lefutott, és a végleges tartalmat egy tényleges XML-parserrel (nem csak regex-szel) is
visszaellenőriztem – az ékezetes karakterek, kötőjelek, idézőjelek helyesen jelennek meg.

**1. kör – hibajavítások (mindkét nyelven):**
- `/dat` mappahivatkozás javítva `/data`-ra (a tényleges mappanév).
- A driver-választó listában `HV5122` → `NEWHV5122`-re javítva (a sima `HV5122`/`HV5122.ino` jelenleg
  árva, senki nem használja), és eltávolítva a HV5122/PT6355 mellől a mára téves "development in
  progress" jelzés (mindkettő stabil, production driver).
- `PCF_MULTIPLEX74141PCF` elgépelés javítva `PCF_74141`-re (a valós makró neve).
- "Samsung" driver bejegyzés megjelölve, hogy jelenleg nem elérhető (samsung.ino nincs a buildben).
- Verzió-hivatkozások frissítve 2.5/2.2c-ről 3.1-re (pl. a mintakódban lévő `webName[]` string és a
  soros monitor minta-log).

**2. kör – a user kérésére a tudásbázis érdemben bővítve** a projekt-review során szerzett ismeretek
alapján, ~33 új bekezdéssel mindkét nyelven:
- **Teljes, friss driver-katalógus** (a rövid "lásd a docs-ot" utaló mondat helyett): PT6311, SN75518,
  SN75512, NEWHV5122, ZM1500_ESP32, MULE_V2, VQC10, WORDCLOCK, TUBE1CLOCK, DUMMY – mindegyik rövid
  leírással, mikor melyiket érdemes használni.
- **Új "Home Assistant / MQTT integráció" szakasz**: USE_MQTT bekapcsolása, bróker-beállítás a
  weboldalról, a gyári alapértelmezett mqtt/mqtt jelszóra vonatkozó biztonsági figyelmeztetés, és a
  master/slave szenzor-megosztás koncepciója (USE_MASTER_CLOCK stb.) – ez korábban EGYÁLTALÁN nem
  szerepelt a manuálban, pedig valós, jelentős funkció.
- **Új "További I2C szenzorok" szakasz**: BH1750 luxmérő (korábban csak az analóg LDR-t írta le a
  manuál), és tisztázva, hogy több szenzortípus is kombinálható egy órán.
- **Új "Szódíszórák" szakasz**: a WORDCLOCK (WS2812-mátrixos, szavakban kiírt idő) teljesen más
  kijelzési paradigmájának rövid bemutatása (EN/DE/HU nyelvi változatok).
- **Új "Gyári alapállapot és firmware-frissítés" szakasz**: FACTORYRESET_PIN gombos reset, és az OTA
  (over-the-air) firmware-frissítés lehetősége a weboldalról.
- **Új "Hibajavítások és ismert problémák" szakasz**: rövid utalás a 2026-os code review-ra és a
  docs/CODE_REVIEW.md-re (benne az MQTT biztonsági megjegyzéssel).
- Bővített felülvizsgálati dátum-jegyzet: "...a 3.1-es firmware-verzióhoz..." szöveggel.

**Egyéb, a manuálokkal összefüggő tennivalók, amik megtörténtek:**
- A .pdf verziókat NEM tudtuk újragenerálni (nincs helyi renderer) – a user kézzel törölte a két
  elavult .pdf fájlt. A readme.md "Manuals" szakasza a .docx fájlokra mutat (az új v3.1 nevekkel).
- A readme.md-ben talált törött linkek (space-t tartalmazó fájlnevek %20-as kódolással) javítva –
  CommonMark `<fájlnév szóközzel>` szögletes-zárójeles szintaxisra váltva, ami szélesebb körben működik.

## ✅ Utólagos javítás: fejléc/lábjegyzet verziószám (2026-09-04, később ugyanaznap)
A user telepítette a Claude in Chrome bővítményt és a poppler-t (`C:\poppler-26.07.0`), és kiderült,
hogy helyi Word (COM automatizálás elérhető, verzió 16.0) is van – ezekkel végre valódi vizuális
ellenőrzés történt (Word megnyitotta a .docx-eket hiba nélkül → PDF exportálva → PDF oldalak képként
megnézve). Ez feltárt egy korábban kimaradt hibát: **a futó fejléc és lábjegyzet** (külön
`header*.xml`/`footer*.xml` fájlok a docx-ben, nem a `document.xml`-ben, ezért maradt ki az első
körben) minden oldalon még mindig "v2.5" / "2021.01.23." dátumot mutatott – ez volt, amit a user
észrevett. Javítva mindkét manuálban:
- EN fejléc: "...software (v2.5)" → "...software (v3.1)"
- EN lábjegyzet: "Document version: 2021.01.23." → "...  (reviewed 2026.09, firmware v3.1)" hozzáfűzve
- HU fejléc: "ESP32_UniClock v2.5 program használata" → "...v3.1..."
- HU lábjegyzet: "2021.01.21." → "...  (felülvizsgálva 2026.09, v3.1 firmware)" hozzáfűzve

A .pdf fájlok is újragenerálva Word COM automatizálással (`ExportAsFixedFormat`), és több oldal
vizuálisan is ellenőrizve (poppler `pdftoppm`-mel renderelve, majd képként megnézve) – minden korábbi
tartalmi javítás (driver-katalógus, "What's new" szakasz, stb.) helyesen, jól formázva jelenik meg.

**A poppler PATH-hoz adása** (`C:\poppler-26.07.0\Library\bin`) megtörtént a User-szintű környezeti
változóban is, de csak Claude Code újraindítása után lesz élő ebben a munkamenetben (addig a
teljes elérési úttal hívtuk közvetlenül).

## ✅ Harmadik kör: teljes clocks.h paraméter-referencia hozzáadva (2026-09-04)
A user jelezte, hogy a clocks.h sokkal több paramétert kezel, mint amennyi a manuálban szerepel – ez
így is volt: a manuál eleji "USABLE PARAMETERS" idézett blokk csak a v2.5-ös állapotot tükrözte. Új,
~40 bekezdéses "Additional clocks.h parameters" / "További clocks.h paraméterek" szakasz került
mindkét manuálba (a "What's new"/"Újdonságok" blokk után, "General remarks"/"Általános megjegyzés"
elé), témakör szerint csoportosítva:
- MQTT/Home Assistant (MQTT_PREFIX, USE_MASTER_CLOCK/_TEMP/_HUMID/_RADAR, MASTER_*_TOPIC)
- PWM RGB háttérvilágítás (PWM1/2/3_PIN)
- WiFi kiegészítők (USE_WIFIMANAGER, MDNSNAME)
- RTC kézi nyomógombok (PIN_MODE_SWITCH, PIN_FLD_BUTTON, PIN_SET_BUTTON)
- Numitron specifikumok (PCB_VERSION, LTBIpin)
- Extra kijelzőkarakterek (PLUS/MINUS_CHARCODE, UPPER/LOWER_CIRCLE_CHARCODE)
- Kompakt hőmérséklet/páratartalom kijelzés (DISPLAYTEMP/HUMID_ONLY_2DIGITS, TEMP/HUMID_START_POSITION)
- Egyéb (DATE_REPEAT_MIN, THERMOMETER_CLOCK, DISABLE_NIGHT_ANIMATION, DISABLE_BROWNOUT, FW,
  SIMULATE_TEMPERATURE_SENSOR, MAXDIGITS)
- Szódíszóra-specifikus (WORD_LENGTH, XMAX/YMAX, START_CORNER, WEEKDAY_POSX/POSY, SENSOR_POSX/POSY)

A driver-specifikus GPIO lábneveket (pl. PIN_CS/PIN_SIN, PIN_DIN/PIN_LOAD, PIN_STB, dataPin/clkPin/
latchPin stb.) szándékosan NEM soroltuk fel egyenként – ezek driverenként eltérnek, a meglévő
per-driver példa-szakaszok és a docs/DEVELOPER_GUIDE.md már lefedik őket, felesleges lenne duplikálni.

**Ezúttal is teljes Word COM + poppler vizuális ellenőrzés történt** (immár rutinszerűen): mindkét
.docx hiba nélkül megnyílt Wordben, PDF-be exportálva, majd a releváns oldalak képként megnézve –
minden helyesen, jól formázva jelenik meg mindkét nyelven.

**readme.md**: a "Manuals" szakasz kiegészítve, most már a .docx MELLETT a .pdf verziókra is mutat.

## Hátralévő teendők
Nincs nyitott tétel. A projekt review+dokumentálás+kommentezés+a kért javítások mind elkészültek.
Az egyetlen fennmaradó, felhasználói döntést igénylő pont: az `mqtt.ino`-ban talált hardkódolt gyári
MQTT broker-hitelesítés (SECURITY) és a `memcpy` buffer-túlcsordulás – ezeket a user kifejezett kérésére
NEM javítottuk automatikusan (nem "túlindexelés", és nem volt rá külön utasítás), csak dokumentálva
vannak: lásd [CODE_REVIEW.md](CODE_REVIEW.md) mqtt.ino szakasza.

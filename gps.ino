// ============================================================================
// gps.ino - GPS-based time synchronization
// ----------------------------------------------------------------------------
// Purpose : for STANDALONE clocks that have no WiFi/internet connectivity
//           (see "//#define USE_GPS //use for standalone clock, without wifi
//           internet access" in the parameter reference block at the top of
//           ESP32_UniClock2.ino). Where WiFi IS available, the core program
//           normally gets time from an NTP timeserver instead (see
//           updateTimefromTimeserver() in ESP32_UniClock2.ino); GPS and WiFi
//           time sync are not mutually exclusive, but GPS exists specifically
//           to cover the no-internet case.
// Hardware: any generic NMEA-0183 serial GPS receiver module (e.g. Ublox
//           NEO-6M/NEO-8M or similar). Only the module's TX output needs to
//           be wired to the MCU's RX pin - we only ever READ NMEA sentences
//           from the module, we never send configuration commands to it, so
//           TX/TXPin on the MCU side is intentionally unused (-1).
// Library : TinyGPS++ (TinyGPSPlus) - see libraries/TinyGPSPlus-1.0.2.zip.
//           It incrementally parses NMEA sentences (GPRMC/GPGGA/... checksum
//           validated) character by character via gps.encode(), and exposes
//           decoded fields (time, date, location, etc.) with isValid()/
//           isUpdated() freshness flags.
// Wiring  : ESP8266  -> SoftwareSerial on GPIO3 (RX only, bit-banged, no HW
//                        UART needed). GPIO3 is also the chip's native
//                        RXD0/USB-serial pin, so the GPS wire must be
//                        disconnected while flashing firmware.
//           ESP32    -> HardwareSerial(1) on GPIO25 (RX only).
// Used by : CLOCK_1 (ESP8266, IN-16 Nixie tubes) and CLOCK_13 (ESP8266,
//           IV-16 Numitron tubes) in clocks.h have USE_GPS active by
//           default. CLOCK_50, CLOCK_71, CLOCK_80/81/82 and CLOCK_99 carry
//           a commented-out "//#define USE_GPS" as an optional switch.
// Time    : GPS always provides time in UTC. This file converts to local
//           time itself using a flat prm.utc_offset/enableDST arithmetic
//           offset (see getGPS() below) - unlike the WiFi/NTP path in
//           ESP32_UniClock2.ino, which uses the Timezone library (myTZ/
//           myDST/mySTD) to resolve the exact calendar-based DST transition
//           date. See the note near the setTime() calls below.
// ============================================================================
#ifdef USE_GPS

#include <TinyGPS++.h>                                  // Tiny GPS Plus Library

#if defined(ESP8266)
  #include "SoftwareSerial.h"     // Software Serial Library so we can use other Pins for communication with the GPS module
  static int RXPin = 3;     //RX pin  -  remove wire, when uploading program to 8266
  static int TXPin = -1;    //not used
  SoftwareSerial ss(RXPin, TXPin);
#else
  #include <HardwareSerial.h>   //on ESP32 no SoftwareSerial is needed
  static int RXPin = 25;     //RX pin
  static int TXPin = -1;    //not used  (we only read the GPS, never write to it, so TX is not wired/used)
  HardwareSerial ss(1);
#endif

// The serial connection to the GPS device
static uint32_t GPSBaud = 9600;                   // Ublox GPS default Baud Rate is 9600 (NMEA default for most modules; if a module was reconfigured to a different baud rate, this must be changed to match, otherwise all sentences will fail checksum validation)
TinyGPSPlus gps;                                  // Create an Instance of the TinyGPS++ object called gps

void setupGPS() {
  delay(1500);     //give the GPS module time to power up / boot before we start talking to it
  DPRINTLN("Starting GPS...");
  if (RXPin>=0) regPin(RXPin,"RXPin");    //bookkeeping only (for the pin-usage diagnostics/webpage), doesn't affect GPS operation
  if (TXPin>=0) regPin(TXPin,"TXPin");    //TXPin is -1 (unused) on both platforms, so this branch never actually runs
  #if defined(ESP8266)
    ss.begin(GPSBaud);
    //ss.begin(GPSBaud, SWSERIAL_8N1, RXPin, TXPin, false);
  #else
    ss.begin(GPSBaud, SERIAL_8N1, RXPin);    // ESP32 HardwareSerial::begin(baud, config, rxPin, txPin=-1) - txPin left at its default (-1/unused) since we don't transmit to the GPS
  #endif
  delay(200);
  GPSexist = true;   // POSSIBLE BUG / design note: this is set unconditionally right after opening the serial port, regardless of whether a GPS module is actually attached, powered, or has ever produced a valid NMEA sentence. calcTime() uses GPSexist to decide whether to call getGPS() every cycle; if no module is present this flag stays true forever and getGPS() will just keep returning false harmlessly, but GPSexist does NOT mean "GPS is present/working", only "USE_GPS was compiled in and setupGPS() ran". Compare with how RTCexist is determined elsewhere (presumably via a presence check) for a stricter pattern.
  smartDelay(500);   //feed the parser for 500ms so gps.encode() has a chance to see the first sentences before we try to read anything from it
  getGPS();
}

boolean getGPS(){
static unsigned long lastRun = 0;
boolean res = false;

   // Throttle: only attempt a refresh once every 10s. lastRun==0 lets the very first call
   // (from setupGPS()) through immediately without waiting for the first 10s window.
   if ((lastRun!=0) && (millis() - lastRun) < 10000) //refresh rate
      return(res);

   lastRun = millis();
   smartDelay(500);   //block for 500ms while feeding gps.encode(), giving the parser a chance to complete a fresh RMC/GGA/ZDA sentence since the last call
   // NOTE: this checks gps.time.isValid()/isUpdated() (sentence was parsed correctly and is new since last read),
   // but deliberately does NOT require gps.location.isValid() or any fix-quality/satellite-count check.
   // Many GPS chipsets can decode time/date from the almanac/ephemeris before achieving a full position fix,
   // so this allows time sync slightly earlier than a location fix would. It does mean the reported time could
   // in theory be used even with a very weak/no fix, as long as the receiver has parsed a valid time field.
   // gps.date.isUpdated() is intentionally/incidentally not checked here - time and date normally arrive
   // together in the same NMEA sentence (e.g. RMC), so checking only time.isUpdated() is normally sufficient.
   if (gps.time.isValid() && gps.date.isValid() && gps.time.isUpdated())  {
    if (gps.date.year()>2020) {   //sanity check: many GPS modules report a bogus default date (e.g. from firmware build date or epoch start) until they have decoded a real almanac; reject anything before this fixed threshold. NOTE: this threshold is a fixed constant and will need to be bumped periodically as time passes.
      DPRINTLN("Time refreshed from GPS");
      setTime(gps.time.hour(),gps.time.minute(),gps.time.second(),gps.date.day(),gps.date.month(),gps.date.year());  //set the time (hr,min,sec,day,mnth,yr)  -- GPS time is always UTC, so system time is UTC at this point
      // Convert UTC -> local time. NOTE: unlike updateTimefromTimeserver() in ESP32_UniClock2.ino (which builds a
      // Timezone object from myDST/mySTD TimeChangeRule's and lets the Timezone library pick the correct offset
      // for the current calendar date, i.e. it auto-detects whether "today" falls inside the DST period), this GPS
      // path just adds a flat prm.utc_offset hour, plus one more hour if prm.enableDST is currently true - it does
      // NOT look at gps.date to determine whether the DST period has actually started/ended this year. Since
      // enableDST is only ever changed manually (web UI / MQTT, see key=="enableDST" handling in
      // ESP32_UniClock2.ino) and is never auto-toggled by date, a standalone GPS-only clock (no WiFi to run the
      // calendar-aware Timezone conversion) will NOT automatically switch to/from summer time on the correct
      // calendar day unless the user (or some other mechanism) flips enableDST at the right moment.
      setTime(now()+(prm.utc_offset + (prm.enableDST ? 1 : 0)) * 3600);
      lastTimeUpdate = millis();
    }
    #ifdef DEBUG
      printGPS();
    #endif

    res = true;
   } //endif valid date&time
   return (res);
}

void printGPS() {                                  // debug-only helper (only called when DEBUG is defined, see getGPS() above); values printed here are NOT freshness-checked, so location/satellites may be stale or invalid if no fix has ever been acquired
  DPRINT("Latitude  : ");  DPRINT(gps.location.lat(), 5);  DPRINT("  Longitude : ");  DPRINTLN(gps.location.lng(), 4);
  DPRINT("Satellites: ");  DPRINT(gps.satellites.value());  DPRINT("  Elevation : ");  DPRINT(gps.altitude.feet()); DPRINTLN("ft");
  DPRINT("GPS Time UTC  : ");  DPRINT(gps.time.hour());   // GPS time UTC
  DPRINT(":");  DPRINT(gps.time.minute());                // Minutes
  DPRINT(":");  DPRINTLN(gps.time.second());              // Seconds
}


// This custom version of delay() ensures the TinyGPS++ object keeps being "fed" bytes while we're blocked.
// TinyGPSPlus has no internal buffering of its own: gps.encode() must be called for every single byte read from
// the serial port, or that byte's contribution to the current NMEA sentence is lost. The GPS module streams NMEA
// sentences continuously (typically once per second) regardless of whether anyone is listening, and the serial
// RX FIFO (HW UART on ESP32, or the SoftwareSerial ring buffer on ESP8266) is small, so if this isn't called
// often enough for long enough, incoming bytes get silently dropped/overwritten and whole sentences are lost -
// which is exactly why Fdelay() in ESP32_UniClock2.ino calls smartDelay(1) on every iteration of its busy-loop
// (see "feed GPS" there): it's not really about producing a 1ms delay, it's about draining ss.available() often
// enough during any other blocking/animation work so the GPS UART buffer never has a chance to overflow.
void smartDelay(unsigned long ms)  {
  unsigned long start = millis();
  do {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

//------------------------------------------------------------------------------------------

#else
// USE_GPS not defined for this CLOCK_NN config: provide no-op stand-ins so the rest of the core program
// (setup()/calcTime() in ESP32_UniClock2.ino) can call setupGPS()/getGPS() unconditionally without more #ifdef's.
void setupGPS() {GPSexist = false;}
boolean getGPS() {return(false);}
#endif

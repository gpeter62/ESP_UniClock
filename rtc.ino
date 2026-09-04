#ifdef USE_RTC
//====================================================================================================
// rtc.ino - DS3231 realtime clock driver, with optional manual clock-setup pushbuttons
//====================================================================================================
// Chip:      Maxim/Analog DS3231 I2C RTC (temperature-compensated crystal oscillator, +-2ppm accuracy).
//            Talked to via the Makuna "Rtc_by_Makuna" library (RtcDS3231<TwoWire>), see
//            libraries/Rtc_by_Makuna.zip. Uses the Arduino `Wire` (I2C) object, so it shares the
//            I2C bus with any other I2C sensors defined for the clock (see I2Csensors.ino).
// Wiring:    I2C bus only - PIN_SDA / PIN_SCL, set per-CLOCK_NN in clocks.h (any GPIO pair works,
//            it's not necessarily the chip's "default" I2C pins). Presence is auto-detected at
//            startup by I2Csensors.ino (probes address 0x68) and reported in the RTCexist flag.
// Role in the system: the DS3231 is a battery-backed FALLBACK time source, not the primary one.
//            NTP (WiFi) and/or GPS are preferred whenever available; the RTC is only read to set
//            the running clock when neither NTP nor GPS refreshed the time on that update cycle,
//            and it is written to (kept in sync) whenever NTP/GPS DID refresh the time - so after
//            a power loss/reset with no network, the clock still starts from a decent time.
//            See calcTime() in ESP32_UniClock2.ino:
//              if (RTCexist) { if (refreshed || refreshed2) updateRTC(); else getRTC(); }
// Manual set: on clocks that define PIN_FLD_BUTTON/PIN_SET_BUTTON (and optionally
//            PIN_MODE_SWITCH), a long-press on the FLD button enters/leaves a manual date/time
//            editor (editor()/showValue()); the SET button changes the value of the currently
//            selected field. See scanButFLD()/scanButSET() below for the button state machine.
// Used by:   CLOCK_5   (IVL-2 VFD clock, MAX6921, has RTC+buttons)
//            CLOCK_15a (IV-6 VFD clock RGB)
//            CLOCK_17  (PT6311 IV-22 russian clock)
//            CLOCK_35  (Z566M/IN-1 tubes, non-multiplex driver)
//            CLOCK_52  (2xHV5122 IN18 clock, PROTOTYPE)
//            CLOCK_71  (2x VQC10 clock - USE_RTC defined here only to also enable the pushbutton
//                       code below; this build has no physical DS3231, RTCexist stays false)
//            CLOCK_80/81/82 (word clocks EN/DE/HU - RTC only, PIN_FLD_BUTTON/PIN_SET_BUTTON are
//                       commented out on these builds, so the manual editor is never entered)
//            (CLOCK_50 and CLOCK_99 have USE_RTC available but commented out by default)
//====================================================================================================
#include <RtcDS3231.h>   //Makuna/RTC
RtcDS3231<TwoWire> Rtc(Wire);
RtcDateTime rtcNow;
RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);   //firmware build timestamp, used as a sane fallback date if the RTC ever loses confidence (e.g. dead backup battery)
#define EPOCH_OFFSET 946684800l   //offset from 1970->2000 epoch start; the Makuna library's RtcDateTime counts seconds since 2000-01-01, while TimeLib's now() is a standard Unix epoch (1970) - subtract this constant to convert Unix-epoch -> DS3231 epoch
//------ Mode Switch and Push Buttons ---------------------
#ifndef PIN_MODE_SWITCH
  #define PIN_MODE_SWITCH  -1   //-1 = "not wired on this clock", all buttoncode below is compiled out via the #if PIN_xxx>=0 guards
#endif

#ifndef PIN_FLD_BUTTON
  #define PIN_FLD_BUTTON -1
#endif
#ifndef PIN_SET_BUTTON
  #define PIN_SET_BUTTON -1
#endif

boolean pushedFldButtonValue = LOW;   //which logic level means "button pushed"
boolean pushedSetButtonValue = LOW;   //which logic level means "button pushed"

//------- I2C bus definition   Any pins are usable  Please, SET in clocks.h --------
//#define PIN_SDA 4           //D2   original general setup for 8266
//#define PIN_SCL 5           //D1

#define MENU_UNSELECT_TIMEOUT 30000   //NOTE: defined but not actually read/used anywhere in this file (LastModify is set on every keypress but never compared against this timeout) - the editor only exits via a long FLD press, not by an idle timeout. Left in place for a possible future auto-exit-on-idle feature.

int monthdays[13] = {0,31,29,31,30,31,30,31,31,30,31,30,31};   //days-per-month lookup (index 1-12, Feb hardcoded to 29/leap); NOTE: not currently referenced anywhere in this file - no leap-year/day-overflow validation is actually performed when editing the Day field (see scanButSET())

struct Menu {
   short lowlimit;        // lower limit for this field's value
   short uplimit;         // upper limit for this field's value
   char name[5];          // short label shown on the display while editing this field
};

// Editable fields for manual time setting, indexed by `fld` (1..MAXFLD). Index 0 is a dummy/unused
// "no field selected" placeholder so `fld==0` can mean "editor not active" without needing -1 checks.
const struct Menu m1[]= {
  {0,0,"    "},
  {2022,2099,"Year"},
  {1,12,"Mon "},
  {1,31,"Day "},   //NOTE: fixed 1..31 range regardless of month/leap-year - editing Feb 30 or Apr 31 is not prevented, see scanButSET()
  {0,23,"Hour"},
  {0,59,"Min "},
};

#define MAXFLD 5

int mvar[MAXFLD+1];   //working copy of the field being edited: mvar[1]=year, [2]=month, [3]=day, [4]=hour, [5]=minute (seconds are always reset to 0 on save, see saveRTC())

// Pushbutton debounce/press-length state machine (used identically by both FLD and SET buttons):
// IS_OPEN            idle, button released
// IS_PUSHING/PUSHED   button just went down (one extra state to timestamp the press)
// IS_LONGPRESSING/-ED  button has been held past the long-press threshold (~2s) - action fires once, on entry
// IS_OPENING          button just released - if this happened quickly (<500ms since push) it's a "short press"
// IS_LONGOPENING       button released after a long press - swallow the release so it isn't also seen as a short press
enum switchStates {IS_OPEN, IS_OPENING, IS_PUSHED, IS_LONGPRESSING, IS_LONGPRESSED, IS_PUSHING, IS_LONGOPENING};

int fld = 0;              //0 = manual editor not active; 1..MAXFLD = index into m1[]/mvar[] of the field currently being edited
unsigned long LastModify = 0;   //timestamp of the last field change while editing; set by scanButFLD/scanButSET, only used as a "was anything changed?" flag by editor() to decide whether saveRTC() is needed on exit (NOT used for the MENU_UNSELECT_TIMEOUT idle-exit that its name might suggest)

void setupRTC() {
  #if PIN_MODE_SWITCH>=0
    pinMode(PIN_MODE_SWITCH,INPUT_PULLUP);
    regPin(PIN_MODE_SWITCH,"PIN_MODE_SWITCH");
  #endif
  #if PIN_FLD_BUTTON>=0
    pinMode(PIN_FLD_BUTTON,INPUT_PULLUP);
    regPin(PIN_FLD_BUTTON,"PIN_FLD_BUTTON");
    // Auto-detect button polarity at boot: with INPUT_PULLUP the idle (unpressed) line should read
    // HIGH. If it reads LOW here instead, the wiring/hardware inverts the logic (e.g. external
    // pull-down, or the button is wired to the pin differently than the internal-pullup default),
    // so remember that for this board "pressed" actually reads HIGH, not LOW.
    // NOTE: this assumes the button is NOT physically held down at power-on - if it were, polarity
    // detection here would be wrong for the rest of the session.
    if (digitalRead(PIN_FLD_BUTTON) == LOW)  //inverted button?
      pushedFldButtonValue = HIGH;
  #endif
  #if PIN_SET_BUTTON>=0
    pinMode(PIN_SET_BUTTON,INPUT_PULLUP);
    regPin(PIN_SET_BUTTON,"PIN_SET_BUTTON");
    if (digitalRead(PIN_SET_BUTTON) == LOW)  //inverted button?
      pushedSetButtonValue = HIGH;
  #endif

  if (RTCexist) {
    DPRINTLN("Connecting to RTC clock...");
    Rtc.Begin();
    if (!Rtc.IsDateTimeValid()) {
        // IsDateTimeValid() also surfaces DS3231-specific fault conditions (oscillator stop flag,
        // invalid register values) via LastError(), distinct from a plain I2C communication failure.
        if (Rtc.LastError() != 0) {
            // A nonzero LastError() here means the I2C transaction itself failed (bus error / chip
            // not responding) - do NOT touch the RTC's stored time in that case, since we can't
            // trust what we'd be reading or writing over a broken bus.
            DPRINT("RTC communications error = ");  DPRINTLN(Rtc.LastError());
        }
        else {
            // I2C is fine, but the chip reports its own datetime as invalid - typically means the
            // DS3231's Oscillator Stop Flag (OSF) is set, i.e. the coin-cell backup battery died (or
            // is missing/depleted) at some point and the chip lost power, so its date/time can no
            // longer be trusted. Reseed it with the firmware's own compile timestamp as a "better
            // than nothing" starting point until NTP/GPS can correct it.
            DPRINTLN("RTC lost confidence in the DateTime!");
            Rtc.SetDateTime(compiled);  //Set RTC time to firmware compile time
        }
    }

    if (!Rtc.GetIsRunning())  {
        // The DS3231 has an Enable Oscillator (EOSC) bit that can leave the clock halted even
        // though the chip itself answers on I2C - make sure the oscillator/counting is actually on.
        DPRINTLN("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    // Disable the 32kHz output and the SQW/INT pin's square-wave function - this firmware never
    // uses the DS3231 as an interrupt/alarm source, so both are turned off to save a little power
    // and avoid an unused, possibly-floating/toggling pin on the board.
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  }
/*
while (true) {
  DPRINT(digitalRead(PIN_MODE_SWITCH)); DPRINT(" / ");
  DPRINT(digitalRead(PIN_FLD_BUTTON));  DPRINT(" / ");
  DPRINTLN(digitalRead(PIN_SET_BUTTON));
  delay(1000);
  }
*/
}


// Called once per main loop() iteration (unconditionally, whenever USE_RTC is defined). Most of the
// time this is a cheap no-op: it just polls the FLD button once (scanButFLD) and returns immediately
// because fld==0 (editor not active). Only when a long FLD-press flips fld to nonzero does it drop
// into the inner while(true) loop below - and while that loop runs, it BLOCKS the rest of loop() (no
// WiFi/animation/sensor/etc. processing happens) until the user long-presses FLD again to save & exit.
void editor() {
  static unsigned long lastDisplay = 0;
  if (RTCexist) {rtcNow = Rtc.GetDateTime();}   //note: read but not otherwise used here - editor() re-reads current time into mvar[] via getRTC() only at the moment edit mode is entered (see scanButFLD's IS_LONGPRESSING case)
  LastModify = 0;
  #if PIN_FLD_BUTTON >= 0 && PIN_SET_BUTTON >= 0   //Are buttons installed?
    editorRunning = true;   //flag read elsewhere (e.g. wordclock.ino) to show an "editing" indicator on the display
    while (true) {
      scanButFLD(100);   //100ms poll interval for both buttons - also doubles as the blocking loop's own pacing/refresh rate
      if (fld == 0) {
        editorRunning = false;
        break;
      }
      scanButSET(100);
      showValue();
      #if defined(WORDCLOCK)
        if ((millis()-lastDisplay)>100) {
          lastDisplay = millis();
          writeDisplay2();
        }
      #else
        writeDisplaySingle();
      #endif
      printDigits(1000);
      Fdelay(100);
    }
  #endif
  if (LastModify != 0) saveRTC();   //only persist if at least one field was actually changed during this edit session
}

// Renders the value of the field currently being edited (mvar[fld]) into the shared `digit[]`
// display buffer (digit 10 = blank, used by memset to clear unused positions first).
void showValue() {
  memset(digit,10,sizeof(digit));   //clear array
  memset(digitDP,0,sizeof(digitDP));
  if(fld==1) {
    //Year is the only 4-digit field (e.g. 2026) - split into thousands/hundreds/tens/units across digit[0..3]
    digit[3] = mvar[1] / 1000;
    digit[2] = (mvar[1] % 1000) / 100;
    digit[1] = (mvar[1] % 100) / 10;
    digit[0] = mvar[1] % 10;
  }
  else {
    //Month/Day/Hour/Minute are all 1-2 digit values - tens/units into digit[0..1]
    digit[1] = mvar[fld] / 10;
    digit[0] = mvar[fld] % 10;
  }
  #if defined(MAX6921) || defined(MAX6921_ESP32)
    //character set not available yet   (MAX6921-driven displays can't render the alphabetic field-name label below, only digits)
  #else
  // On displays with more than 4 digit positions, show the field's short text label (e.g. "Year",
  // "Hour") in the upper digit positions alongside the numeric value, so the user knows which field
  // they're editing. Needs >=6 digits to fit 3 label characters + a ':' separator, or >=8 to fit the
  // full 4-character label without the ':' filler.
  if (!digitsOnly && (maxDigits>4)) {
    if (maxDigits>=8) digit[7] = m1[fld].name[0];
    if (maxDigits>=8) digit[6] = m1[fld].name[1];
    if (maxDigits>=6) digit[5] = m1[fld].name[2];
    else digit[5] = ':';   //not enough digit positions for the 3rd label character - show a separator instead
    digit[4] = m1[fld].name[3];

  }
  #endif
}

//------------------------------------- RTC Clock functions --------------------------------------------------------------------
// Called from calcTime() (ESP32_UniClock2.ino) ONLY on cycles where the system time (TimeLib's
// now()) was just refreshed from NTP and/or GPS this same cycle - i.e. "we just got a trustworthy
// time from the network/satellites, so push it into the battery-backed RTC to keep it in sync as
// the fallback source for the next time we DON'T have network/GPS (e.g. after a reboot)."
void updateRTC() {
  DPRINTLN("Update RTC?");
  if (!RTCexist) return;
  if (year()<2020) return;    //invalid system date?? sanity guard so a not-yet-synced/garbage system clock can never be written into the RTC

  rtcNow = Rtc.GetDateTime();
  // Only actually write the RTC if it disagrees with the system time by more than the noise you'd
  // expect from normal read/processing latency (>2s on seconds, or any mismatch on minute/hour/
  // day/month/year) - avoids needless I2C writes to the RTC on every single refresh cycle.
  if (abs(rtcNow.Second() - second())>2 || (rtcNow.Minute() != minute()) || (rtcNow.Hour() != hour())
      || (rtcNow.Day() != day()) || (rtcNow.Month() != month()) || (rtcNow.Year() != year()) )   {
      DPRINTLN("Updating RTC from TimeServer...");
      DPRINT("- DS3231  date:"); DPRINT(rtcNow.Year()); DPRINT("/"); DPRINT(rtcNow.Month()); DPRINT("/"); DPRINT(rtcNow.Day());
      DPRINT(" time:");  DPRINT(rtcNow.Hour()); DPRINT(":"); DPRINT(rtcNow.Minute()); DPRINT(":"); DPRINTLN(rtcNow.Second());
      DPRINT("- Tserver date:"); DPRINT(year()); DPRINT("/"); DPRINT(month()); DPRINT("/"); DPRINT(day());
      DPRINT(" time:");   DPRINT(hour()); DPRINT(":"); DPRINT(minute()); DPRINT(":"); DPRINTLN(second());
      Rtc.SetDateTime(now()-EPOCH_OFFSET);   //update RTC with UNIX epoch timestamp to 2000year epoch
      }  //endif
}


// Called from calcTime() on cycles where NTP/GPS did NOT refresh the time this cycle - the DS3231
// is the fallback: read its current date/time and push it into TimeLib via setTime() so the rest
// of the firmware (display, animations, etc., all of which read time via now()/hour()/etc.) sees a
// continuously-updated clock even with no network/GPS. Also refreshes mvar[] with the (now current)
// date/time fields, which is what the manual editor uses as its starting point when entered next
// (see scanButFLD's IS_LONGPRESSING case calling getRTC() to seed mvar[] with "now").
void getRTC() {
  if (RTCexist) {
    if (!Rtc.IsDateTimeValid()) {
        if (Rtc.LastError() != 0) {
            // I2C communication problem - do not touch system time this cycle; whatever TimeLib
            // already has (from the last successful update) is left running/ticking on its own.
            DPRINT("RTC communications error = "); DPRINTLN(Rtc.LastError());
        }
        else {
            // Oscillator Stop Flag set (dead/missing backup battery, or never-yet-set RTC) - the
            // chip's stored date/time can't be trusted, so it is deliberately NOT read into
            // setTime() here; system time is simply left as-is for this cycle.
            DPRINTLN("RTC lost confidence in the DateTime!");
        }
    }
    else {
      rtcNow = Rtc.GetDateTime();
      //DPRINT("\nGet DS3231 data&time: ");  DPRINT(rtcNow.Year()); DPRINT("/"); DPRINT(rtcNow.Month()); DPRINT("/"); DPRINT(rtcNow.Day());
      //DPRINT(" ");  DPRINT(rtcNow.Hour()); DPRINT(":"); DPRINT(rtcNow.Minute()); DPRINT(":"); DPRINTLN(rtcNow.Second());
      setTime(rtcNow.Hour(),rtcNow.Minute(),rtcNow.Second(),rtcNow.Day(),rtcNow.Month(),rtcNow.Year());  //set the time (hr,min,sec,day,mnth,yr)
    }
  }
  // NOTE: mvar[] is refreshed from now()/year()/month()/etc. below unconditionally, even when
  // RTCexist is false (or the RTC read above was skipped due to an invalid/error state) - in that
  // case it simply reflects whatever TimeLib's current system time already is (e.g. last NTP time,
  // or millis()-derived free-running time), which is a reasonable "best current guess" starting
  // point for the manual editor on hardware with no RTC chip (see CLOCK_71 in clocks.h).
  mvar[1] = year(); mvar[2] = month(); mvar[3] = day();
  mvar[4] = hour(); mvar[5] = minute();
}

// Commits the values edited via the manual FLD/SET buttons (mvar[1..5]) back into the running
// system clock, and - if present - into the DS3231 as well. Called once, when the editor is exited
// (long FLD press while fld>0), only if at least one field was actually changed (see editor()).
void saveRTC() {
  setTime(mvar[4],mvar[5],0,mvar[3],mvar[2],mvar[1]);  //set the time (hr,min,sec,day,mnth,yr) - seconds are always reset to :00 since there's no seconds field in the editor

    if (RTCexist) {
      DPRINTLN("Updating RTC from Manual settings...");
      Rtc.SetDateTime(now()-EPOCH_OFFSET);
    }
    DPRINT("New Date & Time:"); DPRINT(year()); DPRINT("/"); DPRINT(month()); DPRINT("/"); DPRINT(day());
    DPRINT(" time:");  DPRINT(hour()); DPRINT(":"); DPRINT(minute()); DPRINT(":"); DPRINTLN(second());
}

//-------------------------- check buttons  ------------------------------------------------
// FLD ("field") button:
//  - Long press (>2s) toggles the editor: OFF->ON (loads current time into mvar[] via getRTC(),
//    fld becomes 1 = "Year"); ON->OFF (saves mvar[] back via saveRTC(), fld becomes 0).
//  - Short press (<500ms) while already editing (fld>0) advances to the next field, wrapping
//    Year->Month->Day->Hour->Minute->Year... Has no effect while the editor is closed (fld==0),
//    since a short press there does nothing but a long press is needed to open it.
// Debounce: state changes are only evaluated once per `mill` ms (called with 100ms from editor()),
// so a button transition is only "seen" after at least one 100ms tick, which is enough to reject
// mechanical contact bounce without any extra filtering.
void scanButFLD(unsigned long mill) {
static unsigned long lastRun = millis();
static unsigned long lastPush = millis();;
static switchStates butState;
byte sw;

  if (PIN_FLD_BUTTON<0) return;
  if ((millis() - lastRun) < mill) return;   //refresh rate
  lastRun = millis();

  sw = digitalRead(PIN_FLD_BUTTON);
  if (pushedFldButtonValue == HIGH) sw = !sw;   //inverted button logic - normalize so downstream code can always treat LOW as "pressed"

  switch (butState) {
    case IS_OPEN:
      if(sw == LOW) butState = IS_PUSHING;
      break;
    case IS_PUSHING:
      butState = IS_PUSHED;
      lastPush = millis();   //timestamp the press; used both for the 2s long-press threshold and the 500ms short-press classification on release
      DPRINTLN(" PUSHED");
      break;
    case IS_PUSHED:
      if(sw == HIGH)  butState = IS_OPENING;
      if((sw == LOW) && ((millis()-lastPush)>2000))  butState = IS_LONGPRESSING;
      break;
    case IS_LONGPRESSING:
      // Entered exactly once per long press (state immediately advances to IS_LONGPRESSED below),
      // so this toggle fires a single time per press-and-hold, not repeatedly while held.
      butState = IS_LONGPRESSED;
      DPRINTLN(" IS_LONGPRESSED");
      if (fld>0) {
        fld = 0;
        saveRTC();       //leaving edit mode: commit mvar[] to system clock + RTC chip
      }
      else {
        fld = 1;
        getRTC();        //entering edit mode: seed mvar[] with the current date/time (field 1 = Year)
      }
      break;
    case IS_LONGPRESSED:
      if (sw == HIGH) butState = IS_LONGOPENING;
      break;
    case IS_OPENING:
      butState = IS_OPEN;
      if ((millis()-lastPush)<500) {
           //short press (press+release completed within 500ms of the initial push) - only meaningful while editing (fld>0), since fld is unaffected when already 0
           LastModify = millis();
          fld++;  if (fld>MAXFLD) fld = 1;   //wraps back to field 1 (Year), never to 0 - the editor can only be exited via a long press, not by cycling fields
          DPRINT(m1[fld].name); DPRINTLN(fld);
       } //endif millis()
      break;
    case IS_LONGOPENING:
      // Swallow the release-after-long-press here so it isn't misread as an additional short press
      // once control returns to IS_OPEN.
      butState = IS_OPEN;
      break;
  } //end switch
}

// SET button: changes the value of whichever field FLD currently has selected (mvar[fld]).
//  - Short press (<500ms): increments the value by 1 (with wrap-around at m1[fld].lowlimit/uplimit).
//  - Held past the long-press threshold (2s): auto-repeats the +1 increment on every subsequent
//    100ms poll tick for as long as the button stays down (see IS_LONGPRESSED case below) - lets
//    the user "run" a field up quickly (e.g. scrolling through years) instead of clicking 60 times
//    to advance the minute field.
void scanButSET(unsigned long mill) {
static unsigned long lastRun = millis();
static unsigned long lastPush = millis();
static switchStates butState;
byte sw;

  if (PIN_SET_BUTTON<0) return;
  if ((millis() - lastRun) < mill) return;   //refresh rate
  lastRun = millis();

  sw = digitalRead(PIN_SET_BUTTON);
  if (pushedSetButtonValue == HIGH) sw = !sw;   //inverted button logic

  switch (butState) {
    case IS_OPEN:
      if(sw == LOW) butState = IS_PUSHING;
      break;
    case IS_PUSHING:
      butState = IS_PUSHED;
      lastPush = millis();
      DPRINTLN(" PUSHED");
      break;
    case IS_PUSHED:
      if(sw == HIGH)  butState = IS_OPENING;
      if((sw == LOW) && ((millis()-lastPush)>2000))  butState = IS_LONGPRESSING;
      break;
    case IS_LONGPRESSING:
      butState = IS_LONGPRESSED;
      DPRINTLN(" IS_LONGPRESSED");
      //if (noZero !=0) noZero =0; else noZero = 1;      //dead code (leftover from another clock variant?) - no noZero/EEPROM_NOZERO symbol exists in this codebase
      //EEPROM.update(EEPROM_NOZERO, noZero);
      break;
    case IS_LONGPRESSED:
      // Stays in this state (re-entered every poll) for as long as the button remains held, which
      // is what produces the auto-repeat effect: mvar[fld] is bumped by 1 on every ~100ms tick.
      if (sw == HIGH) butState = IS_LONGOPENING;
       // if ((millis()-lastPush)>10000)  {factoryReset(); resetFunc();}   //reset to factory settings   (dead code - factory-reset-via-long-hold was never wired up here)
       LastModify = millis();
       mvar[fld]+=1;
       if (mvar[fld] < m1[fld].lowlimit)  mvar[fld] = m1[fld].uplimit;
       if (mvar[fld] > m1[fld].uplimit)  mvar[fld] = m1[fld].lowlimit;
       DPRINT(fld); DPRINT(" : "); DPRINTLN(mvar[fld]);
       break;
    case IS_OPENING:
      butState = IS_OPEN;
      if ((millis()-lastPush)<500) {
          LastModify = millis();
          mvar[fld]++;
          if (mvar[fld] < m1[fld].lowlimit)  mvar[fld] = m1[fld].uplimit;
          if (mvar[fld] > m1[fld].uplimit)  mvar[fld] = m1[fld].lowlimit;
          //if (fld <= 2) mvar[2] = monthday_check(mvar[RTC_Ev], mvar[RTC_Ho], mvar[RTC_Nap]);    //leap-year check (szökőév ellenőrzés)   -- dead code: monthday_check()/RTC_Ev/RTC_Ho/RTC_Nap are not defined anywhere in this codebase, so a Day value like 30 in February can currently be set and saved without any leap-year/month-length validation
          DPRINT(fld); DPRINT(" : "); DPRINTLN(mvar[fld]);
          if (fld>MAXFLD) fld = 0;   // POSSIBLE BUG: this resets fld (the FIELD SELECTOR), not mvar[fld] (the field's VALUE). fld is only ever changed by scanButFLD(), which already clamps it to 1..MAXFLD, so this condition should never be true in practice - looks like leftover/copy-pasted logic (or was meant to operate on something else) rather than an intentional part of the SET-button behavior.
          //flash(fld);
       } //endif millis()
      break;
    case IS_LONGOPENING:
      butState = IS_OPEN;
      break;
  } //end switch
}


// Manual I2C bus-recovery routine: bit-bangs SCL/SDA as plain GPIOs (bypassing the Wire/TWI
// peripheral) to unstick an I2C bus where a slave device is stuck holding a line low (e.g. a
// device that was reset/powered mid-transaction and is waiting for clocks it will never see from
// a normal Wire.begin()). Follows the standard "clock out up to 9 pulses, then issue a STOP"
// recovery procedure from the I2C spec / Adafruit/NXP application notes.
// NOTE: currently DEAD CODE from this file's own call graph - not invoked anywhere in rtc.ino; the
// only call site in the whole project is commented out (I2Csensors.ino, near where RTCexist gets
// probed on the bus) rather than being wired into the normal startup/error-recovery path.
#if defined(PIN_SDA) && defined(PIN_SCL)
int I2C_ClearBus() {
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif
  DPRINTLN("I2C_ClearBus() started.");
  pinMode(PIN_SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(PIN_SCL, INPUT_PULLUP);

  Fdelay(2500);  // Wait 2.5 secs. This is strictly only necessary on the first power
  // up of the DS3231 module to allow it to initialize properly,
  // but is also assists in reliable programming of FioV3 boards as it gives the
  // IDE a chance to start uploaded the program
  // before existing sketch confuses the IDE by sending Serial data.

  boolean SCL_LOW = (digitalRead(PIN_SCL) == LOW); // Check if SCL is Low.
  if (SCL_LOW) { //If it is held low, Arduino cannot become the I2C master.
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(PIN_SDA) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
    clockCount--;
  // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
    pinMode(PIN_SCL, INPUT); // release SCL pullup so that when made output it will be LOW
    pinMode(PIN_SCL, OUTPUT); // then clock SCL Low
    delayMicroseconds(10); //  for >5uS
    pinMode(PIN_SCL, INPUT); // release SCL LOW
    pinMode(PIN_SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5uS
    // The >5uS is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(PIN_SCL) == LOW); // Check if SCL is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(PIN_SCL) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
    }
    SDA_LOW = (digitalRead(PIN_SDA) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    return 3; // I2C bus error. Could not clear. SDA data line held low
  }

  // else pull SDA line low for Start or Repeated Start
  pinMode(PIN_SDA, INPUT); // remove pullup.
  pinMode(PIN_SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5uS
  pinMode(PIN_SDA, INPUT); // remove output low
  pinMode(PIN_SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5uS
  pinMode(PIN_SDA, INPUT); // and reset pins as tri-state inputs which is the default state on reset
  pinMode(PIN_SCL, INPUT);
  return 0; // all ok
}
#else
int I2C_ClearBus() {return 1;}   //PIN_SDA/PIN_SCL not defined for this build - can't bit-bang pins that don't exist, report a generic "bus error" instead
#endif

//------------------------------------------------------------------------------------------
#else
  // USE_RTC not defined for this clock build: provide empty stand-in implementations so the rest
  // of the codebase (ESP32_UniClock2.ino's setup()/calcTime()/loop()) can call these unconditionally
  // without needing #ifdef USE_RTC guards at every call site.
  void updateRTC() {}
  void setupRTC() {}
  void getRTC() {}
  void editor() {}
#endif

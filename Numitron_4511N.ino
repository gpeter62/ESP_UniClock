// ============================================================================
// Numitron_4511N driver
// ----------------------------------------------------------------------------
// Drives IV-16 (and similar) Numitron filament (resistive, incandescent-style
// 7-segment) tubes through one CD4511/74HC4511 BCD-to-7-segment latch/decoder
// chip PER TUBE. Numitron tubes are NOT gas-discharge (like Nixies) or
// vacuum-fluorescent (like VFDs): each segment is a tiny resistive filament,
// so "brightness" here is achieved purely by PWM-blanking the whole panel's
// power rail (via LTBIpin), not by varying per-segment current the way a
// current-driven display might.
//
// Wiring / protocol:
//  - ABCDPins[4]        : 4 GPIOs = the shared BCD data bus (bits A..D),
//                          wired in parallel to the BCD inputs of every
//                          4511 chip on the panel (one chip per digit).
//  - digitEnablePins[]  : one GPIO per digit/tube position, each wired to
//                          the LE (Latch Enable) pin of that digit's own
//                          4511. Because the BCD bus is shared, only ONE
//                          digit's latch may be allowed to "see" the bus
//                          at a time while the bus is being changed - see
//                          the per-chip strobe sequence in
//                          writeDisplaySingle() below.
//  - LTBIpin            : "Lamp Test / Blanking Input" - shared across the
//                          whole panel. Driving it LOW blanks all digits
//                          instantly regardless of latched BCD data; this
//                          is exploited as a cheap panel-wide PWM dimming
//                          switch (see writeDisplay()) instead of touching
//                          each digit's latch content.
//  - PCB_VERSION        : identifies which physical GP Numitron carrier PCB
//                          revision (1, 3, or 32) the code is running on;
//                          see the panelVersion note near the ESP8266
//                          writeDisplay() below - currently this only
//                          affects code that is commented out, so on the
//                          live code path PCB_VERSION has no effect.
//
// Used by (see clocks.h, search "Numitron_4511N"):
//  - CLOCK_8  : ESP8266, GP PCB v3, 4x IV-16 Numitron tubes (plexi box)
//  - CLOCK_9  : ESP8266, GP PCB v1, 4x IV-16 Numitron tubes (brown box)
//  - CLOCK_13 : ESP8266, GP PCB v1/v3, 4x IV-16 Numitron tubes + GPS timesync
//  - CLOCK_46 : ESP32,   UNFI board, 6x IV-9 Numitron tubes
// ============================================================================

#ifdef Numitron_4511N
char tubeDriver[] = "Numitron_4511N";
//used by GP Numitron v3 panel

// digitEnablePins[], ABCDPins[] and LTBIpin are actually #defined/declared
// per-clock in clocks.h (see CLOCK_8/9/13/46 blocks) - the lines below are
// only a commented-out reference/example of what those definitions look like.
//#define LTBIpin 5
//byte digitEnablePins[] = {13,12,14,16};    //define here the digit enable pins from 4 to 8
//byte ABCDPins[4] = {4,0,2,15};

// digitEnablePins[] is a byte array, so sizeof() here correctly yields the
// element count (number of tube positions), not a byte size in a larger type.
int maxDigits = sizeof(digitEnablePins);

// NOTE: unlike position-multiplexed displays, this driver does not cycle
// through digit positions in the timer ISR (see writeDisplay() below) - all
// digits are latched simultaneously and stay lit continuously. PWMrefresh/
// PWM_min/PWM_max instead define a slow (sub-kHz) panel-wide ON/OFF blanking
// cycle (via LTBIpin) used purely for brightness dimming, not for scanning
// digits. DRAM_ATTR pins these vars in internal RAM on ESP32 so the ISR
// (running from IRAM) can access them without touching flash-backed memory.
#if defined(ESP32)
  int DRAM_ATTR PWMrefresh=5500;   //msec, Multiplex time period. Greater value => slower multiplex frequency
  //int DRAM_ATTR PWMtiming[11] = {0,500,800,1200,2000,2500,3000,3500,4000,4500,5000};
  int DRAM_ATTR PWM_min = 500;
  int DRAM_ATTR PWM_max = 5000;
  int DRAM_ATTR maxDig=maxDigits;
#else
  int PWMrefresh=5500;   //msec, Multiplex time period. Greater value => slower multiplex frequency
  int PWM_min = 500;
  int PWM_max = 5000;
  int maxDig=maxDigits;
  //int PWMtiming[11] = {0,500,800,1200,2000,2500,3000,3500,4000,4500,5000};
#endif

void setup_pins() {
  char tmp[30];

  DPRINTLN("Numitron Clock - setup pins");
  delay(1000);
  pinMode(LTBIpin, OUTPUT);  regPin(LTBIpin,"LTBIpin");
  digitalWrite(LTBIpin,HIGH);  //panel enabled (not blanked) by default until the first PWM tick decides otherwise
  for (int i=0;i<maxDig;i++) {
    pinMode(digitEnablePins[i], OUTPUT);
    sprintf(tmp,"digitEnablePins[%d]",i);
    regPin(digitEnablePins[i],tmp);
  }
  for (int i=0;i<4;i++) {
    // ABCDPins is always 4 wide (one GPIO per BCD bit, A=LSB..D=MSB) regardless
    // of how many tube positions maxDig has - it is the shared bus, not per-digit.
    pinMode(ABCDPins[i], OUTPUT);
    sprintf(tmp,"Pin[%c]",char('A'+i));
    regPin(ABCDPins[i],tmp);
  }
  startTimer();  //arms the hardware timer ISR; writeDisplay() re-arms itself with a new period on every call (see below)
}

#if defined(ESP32)
// Hardware-timer ISR. IMPORTANT: unlike most other drivers in this project,
// this ISR does NOT scan/multiplex individual tube positions and does NOT
// touch digit[]/ABCDPins/digitEnablePins at all - the 4511 latches hold their
// last-written BCD value continuously (see writeDisplaySingle()), so all
// digits stay lit simultaneously with no scanning artifacts. Instead this ISR
// implements a slow square-wave "mark-space" PWM purely on the shared
// LTBIpin (panel blanking) to control overall brightness, and re-arms its own
// timer with a different period every call - i.e. it is a self-rescheduling
// software PWM, not a fixed-frequency multiplex tick.
void IRAM_ATTR writeDisplay() { //void IRAM_ATTR  writeDisplay(){
  static DRAM_ATTR volatile int timer = PWMrefresh;
  static DRAM_ATTR volatile int dpCounter = 0; //decimal point stepper -- NOTE: currently unused; this driver has no per-position decimal-point/dot output (no DP_PIN wiring), so digitDP[] is never consulted here.
  static DRAM_ATTR boolean state=true;
  int PWMtimeBrightness;
  int brightness;
  int num;  // POSSIBLE BUG (minor): declared but never used in this function - likely leftover from an earlier version, or copy-paste from writeDisplaySingle(). Harmless, just dead.

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    return;
  }

  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();
  intCounter++;
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  if (autoBrightness && (displayON )) {
    // Ambient-light (lux) driven brightness: scale the ON-time fraction of the
    // PWMrefresh period by measured lux against MAXIMUM_LUX, clamped to PWM_min
    // so the panel is never driven at a 0% duty cycle (would look "dead").
    PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
    }
  else
    PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);

  if ((!autoBrightness) && (brightness==MAXBRIGHTNESS))
    state = true;  // at full fixed brightness, force the next phase to start as ON so the panel doesn't sit in a leftover OFF phase

  if (state) {  //ON state
    timer = PWMtimeBrightness;
    timerON = timer;
    timerOFF = PWMrefresh-PWMtimeBrightness;
  }
  else {  //OFF state
    timer = PWMrefresh-PWMtimeBrightness;
  }
  if (timer<500) timer = 500;  //safety only... avoids re-arming the hardware timer with too short/zero a period, which could stall the ISR cadence
  if ( (brightness == 0) || (!state) || (!radarON)) {  //OFF state, blank digit
        digitalWrite(LTBIpin,LOW); //disable display -- global blank: LTBIpin overrides all latched BCD data on every 4511 at once, regardless of what's latched
        #if COLON_PIN >=0
          digitalWrite(COLON_PIN,LOW);  // Blink colon pin
        #endif
    }
  else {  //ON state
        digitalWrite(LTBIpin,HIGH); //enable display
        #if COLON_PIN >=0
          digitalWrite(COLON_PIN,colonBlinkState);
        #endif
  }
  state = !state;  //flip ON/OFF phase for the next ISR invocation
  portEXIT_CRITICAL_ISR(&timerMux);
  timerAlarmWrite(ESP32timer, timer, true);   //re-arm with the just-computed (variable) period - this is what makes the PWM period self-scheduling instead of a fixed tick
  timerAlarmEnable(ESP32timer);
  interrupts();
}

// Blocking, one-shot write of the whole digit[] buffer into the tube latches.
// Because ABCDPins is a single bus shared by every digit's 4511, digits must
// be written one at a time: pulse that digit's own LE line while the bus
// carries its value, so only that digit's latch captures it.
void IRAM_ATTR writeDisplaySingle() {
  int num;

  // FIXED (was "pos<=maxDig", an off-by-one that read digitEnablePins[maxDig] - one element past
  // the end of that array, from clocks.h - and called digitalWrite() with whatever garbage byte
  // happened to sit there in memory). Valid indices are 0..maxDig-1 (maxDig == maxDigits == the
  // actual array length, e.g. 4 or 6).
  for (int pos=0; pos<maxDig; pos++) {
    num = digit[pos];
    // POSSIBLE BUG: strobe order looks inverted for CD4511/74HC4511 latch
    // semantics. On that chip LE (Latch Enable) is active HIGH = "transparent"
    // (Q outputs follow BCD inputs live) and LE LOW = "latched/frozen" (holds
    // last value regardless of later BCD changes). The expected safe sequence
    // to update one digit without disturbing others on the shared bus is:
    // LE HIGH (transparent) -> drive ABCD with the new value -> LE LOW (latch
    // it in, freezing it against subsequent bus changes for the next digit).
    // This code instead does LE LOW -> drive ABCD -> LE HIGH, which leaves
    // every already-processed digit's latch in the *transparent* state, so it
    // may keep following the shared bus as later positions are written -
    // potentially causing all/most tubes to end up showing the last digit's
    // value, or visible cross-talk between positions. If this matches the
    // actual physical panel behavior it may be intentional/compensated in
    // hardware, but it contradicts the datasheet's normal usage - flagging
    // for hardware verification rather than "fixing" blind.
    digitalWrite(digitEnablePins[pos],LOW);   //latch enable
    for (int j=0;j<4;j++) {digitalWrite(ABCDPins[j],num  & (1<<j)); }  //bit j of num (0=A/LSB .. 3=D/MSB) drives ABCDPins[j]; num is expected to be a BCD digit 0-9 (values 10=BLANK/11=dash/12=DP from digit[] are NOT valid BCD and will decode to whatever the 4511 maps undefined codes to - typically blank/garbage segments)
    digitalWrite(digitEnablePins[pos],HIGH);
  } //end for
}
//__________________ ESP8266 driver __________________________
#else
// panelVersion reflects the PCB_VERSION #define set per-clock in clocks.h
// (1, 2 or 3 for the ESP8266 GP Numitron boards). NOTE: on the current live
// code path below, panelVersion is computed but never actually read - it is
// only consulted inside the commented-out legacy implementation further
// down, so today PCB_VERSION has no effect on ESP8266 runtime behavior
// (brightness/blanking is handled uniformly for all panel versions via the
// PWM+LTBIpin scheme, same as the ESP32 branch above).
#ifdef PCB_VERSION
  byte panelVersion = PCB_VERSION;
#else
  byte panelVersion = 1;
#endif

void ICACHE_RAM_ATTR writeDisplay() {       //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  static int timer = PWMrefresh;
  static int brightness;
  static int PWMtimeBrightness = PWM_min;
  static boolean state = true;
  int num;  // POSSIBLE BUG (minor): declared but only used inside the commented-out block below; dead in the active code path.

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
      timer1_write(PWMrefresh);
  return;
  }

  intCounter++;
// --- Legacy ESP8266 implementation (kept for reference, NOT compiled) ---
// This older approach did per-tick brightness stepping by re-writing every
// digit's BCD value each ISR call and blanking individual digits (num=10)
// once a rolling brightCounter exceeded the configured brightness - i.e. a
// true duty-cycle PWM done by rewriting the tube data itself, gated by
// panelVersion>=3 (only v3 panels apparently supported blanking this way).
// It also multiplexed the decimal-point/dot output (dpCounter cycling
// through digitDP[]) and inverted the colon blink polarity relative to the
// active implementation below. All of this was superseded by the simpler
// LTBIpin-based panel-wide PWM blanking used today, which is why dpCounter,
// brightCounter, DP_PIN and digitDP[] are effectively unused by this driver
// now - decimal points/dots are NOT currently displayed by Numitron_4511N.
/*
  if (((panelVersion>=3) && ((displayON ?  prm.dayBright : prm.nightBright)<brightCounter)) || (!radarON)) {
    digitalWrite(LTBIpin,LOW);   //Blank display
  }
  else {
    digitalWrite(LTBIpin,HIGH); //enable display
    for (int pos=0; pos<=maxDig; pos++) {
      num = digit[pos];
      if ((displayON ?  prm.dayBright : prm.nightBright)<brightCounter)  num = 10;   //clear digit
      digitalWrite(digitEnablePins[pos],LOW);   //latch enable
      for (int j=0;j<4;j++) {digitalWrite(ABCDPins[j],num  & (1<<j)); }
      digitalWrite(digitEnablePins[pos],HIGH);
    } //end for
  } //end else
  brightCounter++;   if (brightCounter>MAXBRIGHTNESS) brightCounter = 1;
  if (brightCounter == 1) {
    dpCounter++;   if (dpCounter>maxDig) dpCounter = 0;
    #if defined(DP_PIN) && (DP_PIN>=0)
      digitalWrite(DP_PIN,digitDP[dpCounter]);
    #endif
  }
  #if COLON_PIN >=0
    digitalWrite(COLON_PIN,!colonBlinkState);  // Blink colon pin
  #endif
*/

  // Same panel-wide PWM-blanking scheme as the ESP32 writeDisplay() above:
  // compute the ON-time (PWMtimeBrightness) out of the PWMrefresh period from
  // either lux (auto) or the configured day/night brightness, then toggle
  // LTBIpin and reschedule the ESP8266 hardware timer (timer1) for the next
  // phase's duration. See the ESP32 branch for the fuller rationale.
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  if (autoBrightness && (displayON )) {
    PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
    }
  else
    PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);

  if ((!autoBrightness) && (brightness==MAXBRIGHTNESS))
    state = true;

  if (state) {  //ON state
    timer = PWMtimeBrightness;
    timerON = timer;
    timerOFF = PWMrefresh-PWMtimeBrightness;
  }
  else {  //OFF state
    timer = PWMrefresh-PWMtimeBrightness;
  }
  if (timer<500) timer = 500;  //safety only... avoid a near-zero/zero timer1 reload value
  if ( (brightness == 0) || (!state) || (!radarON)) {  //OFF state, blank digit
        digitalWrite(LTBIpin,LOW); //disable display
        #if COLON_PIN >=0
          digitalWrite(COLON_PIN,LOW);  // Blink colon pin
        #endif
  }
  else {  //ON state
        digitalWrite(LTBIpin,HIGH); //enable display
        #if COLON_PIN >=0
          digitalWrite(COLON_PIN,colonBlinkState);
        #endif
  }
  state = !state;

  timer1_write(timer);  //ESP8266 hardware timer1 reload, analogous to timerAlarmWrite() on the ESP32 branch
}

// Blocking, one-shot write of the whole digit[] buffer into the tube latches
// (ESP8266 twin of the ESP32 writeDisplaySingle() above - see that copy for
// the detailed strobe-order/latch-semantics discussion, which applies here
// identically).
void writeDisplaySingle() {
  int num;
  // FIXED: same off-by-one as the ESP32 writeDisplaySingle() above was ("pos<=maxDig", one element
  // past the end of digitEnablePins[]) - see that copy for the detailed explanation.
  for (int pos=0; pos<maxDig; pos++) {
    num = digit[pos];
    // POSSIBLE BUG: same LE strobe-order concern as the ESP32 twin above
    // (LOW->set bus->HIGH leaves the latch transparent instead of latching
    // it) - see the detailed comment in the ESP32 writeDisplaySingle().
    digitalWrite(digitEnablePins[pos],LOW);   //latch enable
    for (int j=0;j<4;j++) {digitalWrite(ABCDPins[j],num  & (1<<j)); }
    digitalWrite(digitEnablePins[pos],HIGH);
  } //end for
}
#endif  //esp8266 end

void clearTubes() {
// Body is entirely commented out: currently a no-op. Comment says "not
// necessary to use", presumably because writeDisplay()'s LTBIpin blanking
// (brightness==0 case) already visually blanks the panel without needing to
// rewrite every digit's latch to a blank BCD code. Kept here only as
// reference for how a real digit-level clear would be done (set all ABCD
// lines HIGH == BCD 15, an invalid/blank code on the 4511) if LTBIpin ever
// becomes unavailable/unsuitable for a given panel revision.
/*  //not necessary to use
  if (panelVersion==3) {
    digitalWrite(LTBIpin,LOW);
  }
  else {
  for (int pos=0; pos<=maxDigits; pos++) {
    digitalWrite(digitEnablePins[pos],LOW);
    for (int i=0;i<4;i++) digitalWrite(ABCDPins[i],HIGH);
    digitalWrite(digitEnablePins[pos],HIGH);
  }
  }
*/
}


#endif

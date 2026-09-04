// ============================================================================
// Nixie driver: NON-multiplexed, one 74141 BCD decoder LATCHED per tube (ESP32)
// ----------------------------------------------------------------------------
// Hardware: 4 (or 6) digit Nixie Clock, 2x 74HC595N shift register + 4x (or 6x)
// 74141. Identical hardware/wiring strategy to no_multiplex74141.ino (every
// tube has its own dedicated 74141, all fed from one shared 74HC595 shift-
// register chain via dataPin/clkPin/latchPin; ALL tubes are shifted+latched
// together every ISR call so they update SIMULTANEOUSLY - there is no
// per-tube time-division multiplexing here, unlike the multiplex74141*.ino /
// pcf_multiplex74141.ino drivers). Brightness is still faked with the same
// trick: the whole chain is periodically re-latched with an all-blank pattern
// (0xF nibbles) for part of each refresh cycle, turning a single shared
// ON/OFF duty cycle into perceived dimming across all tubes at once.
//
// This file is the ESP32 port of no_multiplex74141.ino - kept as a separate
// file (rather than merged behind #ifdefs) for the same reason as
// multiplex74141_ESP32.ino vs multiplex74141.ino: the hardware-timer APIs
// differ (timer1_write single-shot vs. timerAlarmWrite auto-reload,
// ICACHE_RAM_ATTR vs IRAM_ATTR/DRAM_ATTR placement) and ESP32 is dual-core,
// so the ISR here additionally needs portENTER/EXIT_CRITICAL_ISR(&timerMux)
// around the shared-state update, which the single-core ESP8266 doesn't need.
//
// Platform: ESP32 ONLY. For the ESP8266 original see no_multiplex74141.ino.
// Used by these clocks.h configs (search for "#define NO_MULTIPLEX_ESP32"):
//   CLOCK_35 (6x Z566M tubes, 3x74595/6x74141), CLOCK_36 (4x IN-1 tubes,
//   4x74595/4x74141) - both currently commented out (inactive) in clocks.h.
// dataPin/latchPin/clkPin and tubes[] are defined per-CLOCK_NN in clocks.h.
// ============================================================================

#ifdef NO_MULTIPLEX_ESP32

char tubeDriver[] = "NO_MULTIPLEX_ESP32";
#if defined(ESP32)
#else
  #error "Only ESP32 Board is supported!"
#endif

 //change it, if needed for the correct tube sequence
//byte tubes[] = {3,2,1,0};         //4 tubes,   old clock...        use in clocks.h
//byte tubes[] = {5,4,3,2,1,0};   //6 tubes, reverse order            use in clocks.h
// tubes[] maps PHYSICAL shift-register output position -> LOGICAL digit[] index,
// exactly as in no_multiplex74141.ino, so tube wiring order is corrected in
// clocks.h alone without touching this driver's logic.

// NOTE: unlike multiplex74141_ESP32.ino (which keeps a separate DRAM_ATTR maxDig
// copy of maxDigits specifically for the ISR to read), writeDisplay() below reads
// this plain maxDigits directly. In practice this is harmless: ordinary non-const
// globals like this one already live in .bss (real internal RAM), not in a
// flash-cached region, so no DRAM_ATTR is strictly required here - but it is an
// inconsistency with the pattern established elsewhere in this driver family, so
// flagging it for consistency.
int maxDigits = sizeof(tubes);
// NOTE: PWMrefresh/PWM_min/PWM_max here are noticeably larger than the ESP8266
// sibling's (12000/1000/12000) because the ESP32 hardware timer used by
// startTimer() runs at a different effective tick rate (PRESCALER=15 against the
// APB clock, see startTimer() in the core .ino) than the ESP8266's TIM_DIV16
// timer1 - these were re-tuned for ESP32 rather than being real milliseconds
// on either platform.
int DRAM_ATTR PWMrefresh=25000;   //msec, Multiplex time period. Greater value => slower multiplex frequency
int DRAM_ATTR PWM_min = 5000;
int DRAM_ATTR PWM_max = 25000;

//#define dataPin  14  //D5
//#define latchPin 27 //12  //D6
//#define clkPin   26 //13  //D7

// Not implemented for this driver: brightness/blanking is handled entirely inside
// the timer ISR (writeDisplay) by periodically re-shifting an all-blank pattern, so
// there is no separate blocking "write once" path. Kept only for interface parity.
void IRAM_ATTR writeDisplaySingle() { }

void setup_pins(){
  DPRINTLN("Nixie clock - setup pins NON-MULTIPLEX 4(or 6) x 74141");
  pinMode(dataPin, OUTPUT); regPin(dataPin,"dataPin");
  pinMode(latchPin,OUTPUT); regPin(latchPin,"latchPin");
  pinMode(clkPin,OUTPUT);   regPin(clkPin,"clkPin");
  startTimer();
}

// Bit-bangs one 4-bit BCD nibble (one tube's worth of data) into the 74HC595 shift
// register chain, MSB-first - same protocol/timing as no_multiplex74141.ino's
// writeBits(). Uses ICACHE_RAM_ATTR (aliased to IRAM_ATTR for ESP32 builds, see
// "#define ICACHE_RAM_ATTR IRAM_ATTR" in the core .ino) purely so this file stays
// close to its ESP8266 sibling; functionally identical to marking it IRAM_ATTR
// directly. Runs from inside the timer ISR, so it cannot use delayMicroseconds()
// - the "for t<10 nop" loops are a crude but deterministic busy-wait instead, to
// satisfy the 74HC595's minimum clock pulse-width/setup-time requirements.
void ICACHE_RAM_ATTR writeBits(byte num) {   //shift out 4 bits
  for (int i=3;i>=0;i--) {
    digitalWrite(dataPin,num  & (1<<i)); 
    for (int t=0; t<10;t++) asm volatile ("nop");   //100nsec
    digitalWrite(clkPin,HIGH);
    for (int t=0; t<10;t++) asm volatile ("nop");   //100nsec
    digitalWrite(clkPin,LOW);
    for (int t=0; t<10;t++) asm volatile ("nop");   //100nsec
    }
}

void IRAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  // All static locals are DRAM_ATTR for the same reason as in multiplex74141_ESP32.ino:
  // an IRAM_ATTR ISR must be able to run even while flash is briefly inaccessible
  // (Wi-Fi / EEPROM-OTA activity), which requires every byte of data it touches -
  // not just the function's own code - to be forced into internal DRAM.
  static DRAM_ATTR volatile byte state=0;
  static DRAM_ATTR int timer = PWMrefresh;
  static DRAM_ATTR int brightness;
  static DRAM_ATTR int PWMtimeBrightness=PWM_min;
  byte animM;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    // Unlike the ESP8266 sibling (which re-arms timer1 at an idle rate before
    // returning), this simply returns without touching the timer. That's fine
    // because startTimer() configured ESP32timer as auto-reload (true flag), so
    // the alarm keeps firing on its own at whatever period was last programmed;
    // only the shift-register update is skipped while EEPROM is being written.
    return;
  }

  // Dual-core ESP32: portENTER/EXIT_CRITICAL_ISR takes a spinlock so the other core
  // can't preempt this ISR mid-update, while noInterrupts()/interrupts() additionally
  // blocks other interrupt sources on THIS core - together they keep the
  // writeBits()+latchPin sequence below atomic (a partial shift-out latched early
  // would flash the wrong digits on every tube at once).
  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();

  // POSSIBLE BUG / design limitation (same as no_multiplex74141.ino): this collapses
  // every tube's per-position animMask[i] (0..20 wipe-progress) into a SINGLE global
  // animM, keeping only the last nonzero value found. Because every tube is re-shifted
  // and re-latched together in one ISR pass, the actual wipe TIMING computed from animM
  // further down is necessarily shared by all tubes; only the per-tube choice of old vs.
  // new digit (via animMask[i]==0 below) is still individual. So two tubes mid-wipe with
  // different animMask values at the same time can't both be timed correctly - true
  // per-tube staggered animation isn't achievable with this non-multiplexed design.
  animM = 0;
  for (int i=0;i<maxDigits;i++) {
    if (animMask[i] > 0) animM = animMask[i];    //find, if any animation is wanted?
  }

  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  digitalWrite(latchPin, LOW);  //keep the 74141 outputs frozen on their OLD latched values while new data is shifted in

  if ((brightness ==0) || !radarON) {
    for (int i=0;i<maxDigits;i++) writeBits(0xA);  //blank display  (0xA is an invalid BCD code -> every 74141 output stays off)

  #if DECIMALPOINT_PIN>=0
    digitalWrite(DECIMALPOINT_PIN,LOW);
  #endif
  #if COLON_PIN >= 0
    digitalWrite(COLON_PIN,LOW);  // colon pin OFF
  #endif
  digitalWrite(latchPin, HIGH);  //commit the all-blank pattern to the 74141 outputs
  portEXIT_CRITICAL_ISR(&timerMux);
  timerAlarmWrite(ESP32timer, 100*PWMrefresh, true);   //display fully off (radar/night-off) - just idle-poll slowly instead of running the PWM state machine
  timerAlarmEnable(ESP32timer);
  interrupts();
  return;
  }
  else {
    // Shift out one BCD nibble per tube, all in a single ISR pass - this is what makes
    // the driver "non-multiplexed": every tube gets fresh data on every call, rather
    // than only the currently-selected tube as in the multiplex74141*.ino drivers.
    for (int i=0;i<maxDigits;i++) {
      if (state==0) {
        if (animMask[i]==0) writeBits(digit[tubes[i]]);
        else writeBits(oldDigit[tubes[i]]);
      }
      else if (state==1) writeBits(newDigit[tubes[i]]);
      else writeBits(0xF);   //state 2 (blank phase of the brightness duty cycle): 0xF is any non-BCD code, forces the 74141 output off
    } //end for

  //for (int i=0;i<maxDigits;i++) writeBits(digit[tubes[i]]);   //simple test, without animation

  // 3-state duty-cycle machine (global, applies to the whole tube bank at once - see the
  // animM note above): state 0 = show current/old digits for the brightness ON time
  // (optionally starting a wipe if animM>0); state 1 = show the "new" half of a wipe for
  // the ON time; state 2 = blank all tubes for the remaining OFF time (dimming). At
  // MAXBRIGHTNESS, state 2 is skipped entirely (stays at/returns to state 0) since
  // there's no need to ever blank at 100% duty cycle.
  switch (state) {   //state machine...
    case 0:
      #if DECIMALPOINT_PIN>=0
        if (decimalpointON) digitalWrite(DECIMALPOINT_PIN,HIGH);
      #endif  
      #if COLON_PIN>=0
        digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
      #endif  
      if (autoBrightness && displayON) {
          PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
      }
      else {
        PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);
      }      
      if (animM > 0) { //Animation?
        timer =  (PWMtimeBrightness * (20-animM))/20;
        state = 1;  //next state is: show newDigits
      }
      else {
        timer = PWMtimeBrightness;  
        if (brightness>=MAXBRIGHTNESS) state = 0;
        else state = 2;  //default next state is: BLANK display
      }
      break;
    case 1:  //show new character, if animation
      timer = (PWMtimeBrightness * animM)/20;      
      if (brightness>=MAXBRIGHTNESS) state = 0;
      else state = 2;  //default next state is: BLANK display
      break;
    case 2:  //blank display
      #if DECIMALPOINT_PIN>=0 
        digitalWrite(DECIMALPOINT_PIN,LOW);
      #endif  
      #if COLON_PIN>=0 
        digitalWrite(COLON_PIN,LOW);  // colon pin OFF
      #endif  
      state = 0;
      timer = PWMrefresh-PWMtimeBrightness;
      break;
  }  //end switch
  } //end else

  digitalWrite(latchPin, HIGH);    //commit the freshly shifted-in data to all 74141 outputs at once (this is the actual visible update instant)

  if ((brightness == 0) || (!radarON)) {timer = PWM_max; state = 0;}  //no time sharing is needed
  if (timer<500) timer = 500;  //safety only...
  portEXIT_CRITICAL_ISR(&timerMux);
  // Even though startTimer() set the alarm to auto-reload, the alarm value is still
  // rewritten on every call: `timer` changes call-to-call (it encodes the current
  // state's ON/OFF duration), so this is how the variable-period software PWM is
  // actually implemented on top of a fixed-period hardware auto-reload timer.
  timerAlarmWrite(ESP32timer, timer, true);
  timerAlarmEnable(ESP32timer);
  interrupts();
}

// Not implemented for this driver: there's no persistent "enable" line to blank, since
// blanking is achieved by re-shifting an all-off (0xA/0xF) pattern from within
// writeDisplay() itself; a real clearTubes() would need to bit-bang a blank frame here too.
void IRAM_ATTR clearTubes() {}
#endif

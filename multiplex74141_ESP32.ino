// ============================================================================
// Nixie driver: shared/multiplexed BCD-to-decimal decoder (SN74141 / 7441 / K155ID1)
// ----------------------------------------------------------------------------
// ESP32 port of multiplex74141.ino - identical hardware/multiplex strategy
// (one shared BCD decoder driving all tubes, one digitEnablePins[] transistor
// per tube selects which tube's cathodes are actually connected at any given
// instant; see the header comment in multiplex74141.ino for the full write-up
// of the wiring/timing rationale). The two files are kept separate rather
// than merged behind #ifdefs because the ESP8266 and ESP32 hardware-timer
// APIs are different enough (timer1_write single-shot vs. timerAlarmWrite
// auto-reload, ICACHE_RAM_ATTR vs IRAM_ATTR/DRAM_ATTR placement, and the
// dual-core critical-section requirements on ESP32) that sharing one file
// would need almost as many #ifdefs as duplicating it.
//
// Platform: ESP32 ONLY.
// Used by these clocks.h configs (search for "#define MULTIPLEX74141_ESP32"):
//   CLOCK_23, CLOCK_23A, CLOCK_23B, CLOCK_31, CLOCK_33, CLOCK_43, CLOCK_44, CLOCK_45
// digitEnablePins[], ABCDPins[4] and DP_PIN are defined per-CLOCK_NN in clocks.h.
// ============================================================================
#ifdef MULTIPLEX74141_ESP32
char tubeDriver[] = "MULTIPLEX74141_ESP32";

//define here the digit enable pins from 4 to 8

//byte digitEnablePins[] = {15,13,12,14};   //for example... But SET in clocks.h !!!
//byte ABCDPins[4] =  {2,4,5,0};
//int DP_PIN = -1; // decimalPoint inside Nixie tube, set -1, if not used!

int maxDigits = sizeof(digitEnablePins);  //digitEnablePins is byte[], so sizeof == element count
int DRAM_ATTR maxDig = maxDigits;   //memory variable version; DRAM_ATTR forces it into internal RAM (never flash-cached) so the IRAM ISR can safely read it

//byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, is needed (for example: bad tube pin layout)
// NOTE: despite the "msec" comment these are raw hardware-timer tick counts (see
// PRESCALER in the core .ino / startTimer()), not real milliseconds - they only
// control the ON/OFF ratio of each tube's multiplex slot (= brightness).
int DRAM_ATTR PWMrefresh=11000;   //msec, Multiplex time period. Greater value => slower multiplex frequency
int DRAM_ATTR PWM_min = 1000;
int DRAM_ATTR PWM_max = 10000;
//int DRAM_ATTR PWMtiming[11] = {0,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000};

  #if defined(ESP32)
  #else
    #error "Only ESP32 board is supported by this driver!"
  #endif

void setup_pins() {
  char tmp[30];
    
  DPRINTLN("Nixie clock - setup pins -  Multiplex 74141 mode...");
  DPRINTLN("digitEnablePins: ");
  for (int i=0;i<maxDigits;i++) {
    pinMode(digitEnablePins[i], OUTPUT);
    sprintf(tmp,"digitEnPin[%d]",i);
    regPin(digitEnablePins[i],tmp); 
  }
  DPRINTLN(" ");
  DPRINTLN("ABCDPins: ");
  for (int i=0;i<4;i++) {
    pinMode(ABCDPins[i], OUTPUT);
    sprintf(tmp,"Pin[%c]",char('A'+i));
    regPin(ABCDPins[i],tmp); 
  }
  DPRINTLN(" ");
  #if DP_PIN>=0
    pinMode(DP_PIN,OUTPUT);
    // POSSIBLE BUG: no regPin(DP_PIN,"DP_PIN") call here, unlike the ESP8266 sibling
    // (multiplex74141.ino) and the other drivers in this family. DP_PIN will therefore
    // be silently missing from the "USED CLOCK PINS" diagnostic list even though it is
    // actively driven below in writeDisplay().
    DPRINT("DP_PIN:"); DPRINTLN(DP_PIN);
  #endif
  startTimer();
}

void IRAM_ATTR writeDisplay(){  //void IRAM_ATTR  writeDisplay(){
  // All static locals are DRAM_ATTR: code and read-only data placed by the linker in
  // flash can become briefly inaccessible while the ESP32 is reading/writing SPI flash
  // (e.g. Wi-Fi, EEPROM/OTA). An IRAM_ATTR ISR is safe to run during that time, but only
  // if every byte of data it touches - not just the function code - is forced into
  // internal DRAM instead of flash-backed memory. Hence DRAM_ATTR on every static here
  // and on the module-level PWM constants above.

  static DRAM_ATTR byte pos = 0;
  static DRAM_ATTR byte state=0;
  static DRAM_ATTR int timer = PWMrefresh;
  static DRAM_ATTR int num,brightness;
  static DRAM_ATTR byte DPpos;
  static DRAM_ATTR int PWMtimeBrightness=PWM_min;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    // Note: unlike the ESP8266 sibling, this does NOT re-arm the hardware timer before
    // returning. That's fine here because startTimer() configured ESP32timer with
    // autoreload=true (see timerAlarmWrite(...,true) in startTimer()), so the timer
    // keeps firing on its own at whatever period was last programmed; only the display
    // content update is skipped while EEPROM is being written.
    return;
  }

  // ESP32 is dual-core: portENTER/EXIT_CRITICAL_ISR takes a spinlock so the other core
  // (e.g. running Wi-Fi/loop() code) cannot preempt this ISR mid-update of shared state,
  // while noInterrupts()/interrupts() additionally blocks other interrupt sources on
  // THIS core. Both are used together defensively so the digitalWrite() sequence below
  // (which must stay atomic - partial BCD + digit-enable writes would flash a wrong
  // digit) can't be interleaved with anything else.
  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();

  intCounter++;
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  timer = PWMrefresh;

   // Same 4-state per-tube multiplex/animation state machine as the ESP8266 version
   // (multiplex74141.ino) - see the detailed comments there. In short: state 0 advances
   // to the next tube and (if a wipe animation is active for it) shows the OLD digit for
   // part of the slot; state 1 shows the NEW digit for the rest; state 2 blanks the tube
   // for the remainder of the refresh period (this ON/OFF ratio is what implements
   // brightness); state 3 is a one-tick marker consumed immediately below.
   switch (state) {   //state machine...
    case 0:
      pos++;  if (pos>maxDig-1)  {
        pos = 0;  //go to the first tube
        // Brightness/lux is only resampled once per full refresh cycle (all tubes), not per-tube.
        if (autoBrightness && displayON) {
          PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
        }
        else
          PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);
      }
      if (animMask[pos] > 0) { //Animation?
        num = oldDigit[pos];  //show old character
        timer = (PWMtimeBrightness * (20-animMask[pos]))/20;  //older digit gets the (20-mask)/20 share of ON time
        state = 1;  //next state is: show newDigit
      }
      else {
        num = digit[pos];  //show active character
        timer = PWMtimeBrightness;
        state = 2;  //next state is: BLANK display
      }
      break;
    case 1:  //show new character, if animation
      num =   newDigit[pos];
      timer = (PWMtimeBrightness * animMask[pos])/20;      //new digit gets the mask/20 share -> "wipe" transition within one tube's slot
      state = 2;  //default next state is: BLANK display
      break;
    case 2:  //blank display
      timer = PWMrefresh-PWMtimeBrightness;
      state = 3;
      break;
   }  //end switch
   if (timer<500) timer = 500;  //safety only... (too-short alarm periods risk missed/late reloads)

   //  if ((pos>0) && (num<=9)) num = convert[num];   //tube character conversion, if needed... (maybe bad pin numbering)

  // num>9 (BLANK/MINUS/DP-glyph/... codes >=10) is treated as "blank" here purely as a
  // software shortcut; the 74141 hardware itself also outputs nothing for BCD 10-15.
  if ((brightness == 0) || (state == 3) || (num >9) || (!radarON)) {  //blank digit
    state = 0;
    digitalWrite(digitEnablePins[pos],LOW);  //switch off anode
    #if DP_PIN >=0
      digitalWrite(DP_PIN,LOW);
    #endif
    #if COLON_PIN >= 0
      digitalWrite(COLON_PIN,LOW);      // Colon pin OFF
    #endif
    #if DECIMALPOINT_PIN >=0
        digitalWrite(DECIMALPOINT_PIN,LOW);
    #endif
    }
  else {
      for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }  //push BCD bits (bit i -> ABCDPins[i]) to the shared decoder
      digitalWrite(digitEnablePins[pos],HIGH);    //switch on the new digit
      #if DP_PIN >=0
        // LEFTDECIMAL: some tube types have the DP cathode physically positioned to the
        // left of the digit glyph, so the DP for logical position `pos` must be lit
        // together with the NEXT tube's slot instead of this one's.
        if (LEFTDECIMAL) DPpos = min(maxDig-1,pos+1); else DPpos = pos;
        if (digitDP[DPpos]) digitalWrite(DP_PIN,HIGH); //switch ON decimal point, if needed
		else digitalWrite(DP_PIN,LOW);
      #endif
      #if COLON_PIN >= 0
        digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
      #endif
      #if DECIMALPOINT_PIN >=0
        digitalWrite(DECIMALPOINT_PIN,decimalpointON);
      #endif
  }


  portEXIT_CRITICAL_ISR(&timerMux);
  //ESP32timer = timerBegin(0, PRESCALER, true);  //set prescaler, true = edge generated signal
  //timerAttachInterrupt(ESP32timer, &writeDisplay, true);
  // Even though startTimer() configured the alarm as auto-reload, we still rewrite the
  // alarm value + re-enable every call: `timer` changes call-to-call (it encodes the
  // current state's ON/OFF duration), so this is how the variable-period software PWM
  // is actually implemented on top of a fixed-period hardware auto-reload timer.
  timerAlarmWrite(ESP32timer, timer, true);
  timerAlarmEnable(ESP32timer);
  interrupts();
}

void clearTubes() {

  for (int i=0;i<maxDig;i++) digitalWrite(digitEnablePins[i],LOW);  //drop every digit-enable line so no tube anode remains active

    #if (DP_PIN>=0)
      digitalWrite(DP_PIN,LOW);
    #endif
}

// Not implemented for this driver: writeDisplay() (the timer ISR) already refreshes the
// display continuously on its own; kept only for interface parity with drivers that need it.
void writeDisplaySingle() {}
#endif

// ============================================================================
// MULE_V2 driver
//
// Drives the "Mule V2" ESP32 board: 6 x SA40 7-segment LED digit modules,
// used by CLOCK_34 ("Mule V2 ESP32, board, 6 x SA40 LED Display", see
// clocks.h around line 972). This is a BCD-multiplexed 7-segment LED clock,
// NOT a Nixie/VFD tube driver, even though several comments/variable names
// below (DP_PIN, "Nixie clock", tube terminology) are inherited boilerplate
// copied from the tube-driver files this codebase is built from.
//
// Hardware / wiring protocol (see clocks.h CLOCK_34 block for actual pins):
//   - ABCDPins[4]      -> CD4543 BCD-to-7-segment latch/decoder/driver IC.
//                          4 bits carry the digit VALUE (0-9) for whichever
//                          digit position is currently selected; the 4543
//                          decodes it to segments and drives the LED digit
//                          directly (latching, so no extra data hold time is
//                          required).
//   - digitEnablePins[3] -> CD4028 BCD-to-Decimal decoder ("A,B,C" inputs).
//                          Only 3 bits are used (not per-digit enable lines!)
//                          to binary-encode which ONE of the 6 SA40 digits
//                          is active right now; the 4028's decoded output
//                          lines 1-6 are wired to the 6 digit commons, output
//                          0 and 7 are intentionally left unconnected and are
//                          used as a "blank all digits" state (see writeDisplay()).
//   - This is a classic BCD multiplex scheme: only one digit is illuminated
//     at any instant, cycling fast enough (via hardware timer ISR) that
//     persistence of vision makes all 6 digits appear lit simultaneously.
//   - Digit brightness is done by PWM-ing the "on" time of each digit within
//     its multiplex slot (see PWMrefresh/PWM_min/PWM_max and PWMtimeBrightness
//     below), not by varying LED current.
//
// Board also has optional peripherals declared in clocks.h but NOT touched
// by this file directly: NeoPixel backlight (USE_NEOPIXEL/NEOPIXEL_PIN) and
// a Dallas temperature sensor pin - both handled by the core .ino.
// ============================================================================
#ifdef MULE_V2
char tubeDriver[] = "MULE_V2";

//define here the digit enable pins from 4 to 8

//byte digitEnablePins[] = {15,13,12,14};   //for example... But SET in clocks.h !!!
//byte ABCDPins[4] =  {2,4,5,0};
//int DP_PIN = -1; // decimalPoint inside Nixie tube, set -1, if not used!

int maxDigits = 6;  //sizeof(digitEnablePins);
// NOTE: maxDigits (6) is the number of physical SA40 digits, NOT the size of
// digitEnablePins (which is only 3 - see CLOCK_34 in clocks.h: {5,16,23}).
// digitEnablePins holds the 3 CD4028 select lines, so its correct element
// count is always 3 regardless of maxDigits. Do not assume they are equal
// (setup_pins() below used to get this wrong - now fixed, see the note there).
int DRAM_ATTR maxDig = maxDigits;   //memory variable version
// Local DRAM_ATTR copy of maxDigits, used inside the IRAM_ATTR ISR
// (writeDisplay). Keeping a dedicated DRAM-resident copy guarantees the
// value stays reachable from the ISR even in situations where flash-cache
// backed data access would be unsafe (e.g. during flash/EEPROM writes).

//byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, if needed (e.g. for a non-standard tube pin layout)
int DRAM_ATTR PWMrefresh=11000;   //msec, Multiplex time period. Greater value => slower multiplex frequency
int DRAM_ATTR PWM_min = 1000;
int DRAM_ATTR PWM_max = 10000;
//int DRAM_ATTR PWMtiming[11] = {0,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000};
// PWMrefresh is the total time budget (in hardware-timer ticks) for one full
// digit's multiplex slot: part of it is "digit ON" (length set by
// PWMtimeBrightness, scaled between PWM_min/PWM_max according to
// brightness/ambient light) and the remainder is "digit OFF" (blanking), see
// case 2 in writeDisplay(). This is how per-digit brightness/dimming is
// achieved without a dedicated PWM peripheral per digit.

  #if defined(ESP32) 
  #else
    #error "Only ESP32 board is supported by this driver!"  
  #endif

void setup_pins() {
  char tmp[30];

  DPRINTLN("Nixie clock - setup pins -  Multiplex BCD mode (MULE_V2)...");
  // "Nixie clock" text is inherited boilerplate from the tube-driver
  // templates this file was cloned from; this board drives LED digits.
  DPRINTLN("BCD digitEnablePins: ");
  // FIXED (was "i<maxDigits", i.e. 0..5): digitEnablePins[] is declared in clocks.h with only 3
  // elements ({5,16,23} - the CD4028 3-bit select lines, see header comment above), not one entry
  // per digit, so looping to maxDigits(=6) read past the end of the array for i=3,4,5 and called
  // pinMode()/regPin() with whatever garbage byte followed it in memory. Every other place in this
  // file that iterates digitEnablePins (writeDisplay(), clearTubes()) already correctly uses
  // sizeof(digitEnablePins) (=3) instead of maxDigits - this loop now matches them.
  for (int i=0;i<(int)sizeof(digitEnablePins);i++) {
    pinMode(digitEnablePins[i], OUTPUT);
    sprintf(tmp,"digitEnPin[%d]",i);
    regPin(digitEnablePins[i],tmp);
  }
  DPRINTLN(" ");
  DPRINTLN("ABCDPins: ");
  // ABCDPins[4]: the 4 BCD data lines feeding the CD4543 7-segment decoder.
  for (int i=0;i<4;i++) {
    pinMode(ABCDPins[i], OUTPUT);
    sprintf(tmp,"Pin[%c]",char('A'+i));
    regPin(ABCDPins[i],tmp);
  }
  DPRINTLN(" ");
  // DP_PIN (decimal point) is not wired for the Mule V2 board (defaults to
  // -1 in clocks.h for CLOCK_34), so this whole block is compiled out for
  // this board; kept for source compatibility with boards that do use it.
  #if DP_PIN>=0
    pinMode(DP_PIN,OUTPUT);
    DPRINT("DP_PIN:"); DPRINTLN(DP_PIN);
  #endif
  startTimer();  // arms the hardware timer that drives writeDisplay() as an ISR
}

void IRAM_ATTR writeDisplay(){  //void IRAM_ATTR  writeDisplay(){
// Timer-interrupt multiplex refresh handler. Runs once per "phase" of a
// single digit's multiplex slot (see state machine below) and re-arms the
// hardware timer itself with the next phase's duration - i.e. this is a
// self-rescheduling one-shot timer, not a fixed-period tick. Because it is
// IRAM_ATTR it (and everything it touches, hence the DRAM_ATTR statics) must
// not depend on code/data that lives in flash, since it can fire while the
// SPI flash cache is disabled.

  static DRAM_ATTR byte pos = 0;
  static DRAM_ATTR byte state=0;
  static DRAM_ATTR int timer = PWMrefresh;
  static DRAM_ATTR int num,brightness;
  static DRAM_ATTR byte DPpos;
  static DRAM_ATTR int PWMtimeBrightness=PWM_min;

  if (EEPROMsaving) {  //stop refresh while an EEPROM write is in progress!
    return;
  }

  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();

  intCounter++;
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  timer = PWMrefresh;

   switch (state) {   //state machine...
    // 4-phase cycle per digit position, advancing `pos` only in case 0:
    //   0: decide which digit to show next (advance pos, recompute
    //      brightness once per full 6-digit sweep) and pick ON-phase timing
    //   1: (only reached when animMask[pos]>0, i.e. a digit-change animation
    //      is in progress) show the NEW character for the remaining
    //      fraction of the ON time, crossfading old->new digit by splitting
    //      the ON slot into two sub-phases weighted by animMask (0..20)
    //   2: blank phase - digit output stays on the last-set value but the
    //      remaining PWMrefresh budget elapses with the digit dark, giving
    //      the dimming effect
    //   3: sentinel reached via case 2's fallthrough-by-timer only, forces
    //      the blank branch below and resets state to 0 next call
    case 0:
      pos++;  if (pos>maxDig-1)  {
        pos = 0;  //go to the first tube
        if (autoBrightness && displayON) {
          PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);  //ambient-light-driven auto brightness
        }
        else
          PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);  //manual day/night brightness setting
      }
      if (animMask[pos] > 0) { //Animation?
        num = oldDigit[pos];  //show old character
        timer = (PWMtimeBrightness * (20-animMask[pos]))/20;
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
      timer = (PWMtimeBrightness * animMask[pos])/20;
      state = 2;  //default next state is: BLANK display
      break;
    case 2:  //blank display
      timer = PWMrefresh-PWMtimeBrightness;
      state = 3;
      break;
   }  //end switch
   if (timer<500) timer = 500;  //safety only... (avoid re-arming the HW timer with an unreasonably short/zero alarm)

   //  if ((pos>0) && (num<=9)) num = convert[num];   //tube character conversion, if needed... (maybe bad pin numbering)

  // Blank the currently-selected digit when: display globally off/zero
  // brightness, we're in the "blank" phase (state==3), the character code
  // isn't a real digit (10=BLANK,11=dash,12=DP,13+=custom glyph - none of
  // these have a 7-segment BCD encoding this driver understands), or the
  // radar/PIR presence sensor says nobody is around.
  if ((brightness == 0) || (state == 3) || (num >9) || (!radarON)) {  //blank digit
    state = 0;
    for (int i=0;i<sizeof(digitEnablePins);i++)
      digitalWrite(digitEnablePins[i],HIGH);  //switch off anode
      // All 3 CD4028 select lines HIGH = binary 111 = decoder output 7,
      // which is intentionally NOT wired to any digit common (only outputs
      // 1-6 are used, see file header) - this is how "no digit selected"
      // is achieved without a separate enable/blank signal.
    #if DP_PIN >=0
      digitalWrite(DP_PIN,LOW);
    #endif
    }
  else {
      // Push the digit VALUE (0-9) as 4 bits onto the CD4543 (ABCDPins),
      // which latches and decodes it to 7-segment outputs for whichever
      // digit is currently selected below.
      for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }
      // Binary-encode (pos+1) across the 3 CD4028 select lines to choose
      // which of the 6 SA40 digits is active (decoder outputs 1..6); "+1"
      // is because output 0 is reserved/unused (see header comment).
      for (int i=0;i<sizeof(digitEnablePins);i++) {digitalWrite(digitEnablePins[i],(pos+1)  & 1<<i);} //switch on the new digit

      #if DP_PIN >=0
        if (LEFTDECIMAL) DPpos = min(maxDig-1,pos+1); else DPpos = pos;
        if (digitDP[DPpos]) digitalWrite(DP_PIN,HIGH); //switch ON decimal point, if needed
      #endif
  }

  // COLON_PIN / DECIMALPOINT_PIN are both -1 (unused) for the Mule V2 board
  // (CLOCK_34 in clocks.h does not define them), so these blocks compile out
  // entirely on this board; kept for source compatibility with other boards
  // that reuse this driver file.
  #if COLON_PIN >= 0
    if (num==10) digitalWrite(COLON_PIN,LOW);      // Colon pin OFF
    else digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
  #endif

  #if DECIMALPOINT_PIN >=0
        if (num==10) {digitalWrite(DECIMALPOINT_PIN,LOW);}
        else {digitalWrite(DECIMALPOINT_PIN,decimalpointON); }
  #endif

  portEXIT_CRITICAL_ISR(&timerMux);
  //ESP32timer = timerBegin(0, PRESCALER, true);  //set prescaler, true = edge generated signal
  //timerAttachInterrupt(ESP32timer, &writeDisplay, true);
  timerAlarmWrite(ESP32timer, timer, true);   // re-arm the one-shot HW timer for the next phase's duration (see state machine above)
  timerAlarmEnable(ESP32timer);
  interrupts();
}

void clearTubes() {
  // Blank all digits immediately (outside the ISR/multiplex cycle), using
  // the same "select CD4028 output 7" trick as the blanking branch in
  // writeDisplay() above.
  for (int i=0;i<sizeof(digitEnablePins);i++) digitalWrite(digitEnablePins[i],HIGH);
    #if (DP_PIN>=0)
      digitalWrite(DP_PIN,LOW);
    #endif
}

// Intentionally empty: unlike non-multiplexed drivers, this board's display
// is refreshed continuously by the writeDisplay() timer ISR, which reads the
// live digit[]/newDigit[]/animMask[] state on every phase. Callers elsewhere
// in the core .ino invoke writeDisplaySingle() to force an immediate
// blocking redraw after changing digit values (e.g. during animations); for
// MULE_V2 no explicit action is needed since the next ISR tick (at most
// PWM_min..PWMrefresh ticks away) will already pick up the new state.
void writeDisplaySingle() {}
#endif

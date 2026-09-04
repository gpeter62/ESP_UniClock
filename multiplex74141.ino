// ============================================================================
// Nixie driver: shared/multiplexed BCD-to-decimal decoder (SN74141 / 7441 / K155ID1)
// ----------------------------------------------------------------------------
// Hardware: ONE (or a few) 74141-family BCD driver chip(s) whose 4 BCD inputs
// (ABCDPins[4]) are wired in parallel to ALL Nixie tubes, while each tube's
// high-voltage anode is switched individually through a transistor connected
// to digitEnablePins[]. Only one digitEnablePin is HIGH at any instant, so
// only that tube's cathodes actually light up with the BCD value currently
// present on ABCDPins - this is classic time-division multiplexing: the
// digits are shown one tube at a time, fast enough (via the hardware timer
// ISR below) that persistence of vision makes them look simultaneously lit.
// This trades tube count for 74141-chip count (cheap: 1 decoder for N tubes)
// at the cost of average brightness per tube (each tube is only ON 1/N of
// the time) and of needing careful ISR timing to avoid visible flicker.
//
// Platform: ESP8266 ONLY (uses the 8266 hardware timer1 API: timer1_write /
// ICACHE_RAM_ATTR). For the ESP32 port of the same multiplex strategy see
// multiplex74141_ESP32.ino (uses timerAlarmWrite/IRAM_ATTR instead).
//
// Used by these clocks.h configs (search for "#define MULTIPLEX74141"):
//   CLOCK_1, CLOCK_2, CLOCK_20, CLOCK_21, CLOCK_22
// digitEnablePins[], ABCDPins[4] and DP_PIN are actually defined per-CLOCK_NN
// in clocks.h (the declarations commented out below are just examples of the
// shape clocks.h must provide).
// ============================================================================
#ifdef MULTIPLEX74141   //Nixie driver for 8266 only!
char tubeDriver[] = "MULTIPLEX74141";
//define here the digit enable pins from 4 to 8
//byte digitEnablePins[] = {15,13,12,14};   //for example... But SET in clocks.h !!!
//byte ABCDPins[4] =  {2,4,5,0};
//#define DP_PIN -1             // decimalPoint inside Nixie tube, set -1, if not used!

#if defined(ESP8266)
#else
  #error "Board is not supported! For ESP32 use MULTIPLEX74141_ESP32 !"
#endif

int maxDigits = sizeof(digitEnablePins);  //digitEnablePins is byte[], so sizeof == element count
int maxDig = maxDigits;   //memory variable version (kept as a separate copy so it can be read cheaply from the ISR)

//byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, is needed (for example: bad tube pin layout)
// NOTE: these "PWM..." values are NOT milliseconds despite the comment - they are raw
// timer1 tick counts (timer1 on ESP8266 runs at TIM_DIV16, i.e. 5MHz/16 ~= 312.5kHz,
// so 1 tick ~= 3.2us). They set how long (in ticks) each tube stays lit / blanked
// within one full multiplex refresh cycle, which is what actually controls brightness.
int PWMrefresh=11000;   //msec, Multiplex time period. Greater value => slower multiplex frequency
int PWM_min = 1000;
int PWM_max = 10000;
//int PWMtiming[11] = {0,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000};

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
    pinMode(DP_PIN,OUTPUT);   regPin(DP_PIN,"DP_PIN");
    DPRINT("DP_PIN:"); DPRINTLN(DP_PIN);
  #endif  
  startTimer();
}

void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  // ESP8266 timer1 is a SINGLE-SHOT (TIM_SINGLE) timer, so unlike the ESP32
  // version, it must be explicitly re-armed with timer1_write(timer) at the
  // END of every single call below, otherwise the ISR would never fire again.
  static byte pos = 0;
  static volatile byte state=0;
  static int timer = PWMrefresh;
  static int num,brightness;
  static byte DPpos;
  static int PWMtimeBrightness=PWM_min;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
      digitalWrite(digitEnablePins[pos],LOW);  //blank current tube so it doesn't stay lit "stuck" during the (possibly long) flash write
      timer1_write(PWMrefresh);  //re-arm with a "slow idle" period; state machine resumes normally once EEPROMsaving clears
    return;
  }

  intCounter++;
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  timer = PWMrefresh;

   // 4-state per-tube state machine, re-entered once per ISR call:
   //  state 0: advance to next tube position; if animating, show the OLD digit for a
   //           fraction of the slot proportional to animMask[pos] (0..20 => wipe progress)
   //  state 1: show the NEW digit for the remaining fraction of the slot (only reached
   //           when animating)
   //  state 2: blank the tube for the rest of the PWMrefresh period - this "off time"
   //           is what implements brightness dimming (shorter ON / longer OFF = dimmer)
   //  state 3: transient marker consumed immediately below to force digitalWrite LOW
   switch (state) {   //state machine...
    case 0:
      pos++;  if (pos>maxDig-1)  {
        pos = 0;  //go to the first tube
        // Brightness/lux is only re-sampled once per FULL refresh cycle (when wrapping
        // back to tube 0), not on every tube - this keeps brightness visually uniform
        // across all tubes within one cycle even if lx/brightness changes mid-cycle.
        if (autoBrightness && displayON) {
          PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
        }
        else
          PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);
        }

      if (animMask[pos] > 0) { //Animation?
        num = oldDigit[pos];  //show old character
        timer = (PWMtimeBrightness * (20-animMask[pos]))/20;  //animMask range is 0..20; older digit gets the (20-mask) share of the ON time
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
      timer = (PWMtimeBrightness * animMask[pos])/20;      //new digit gets the mask/20 share - together with case 0 this "wipes" old->new within one tube's ON slot
      state = 2;  //default next state is: BLANK display
      break;
    case 2:  //blank display
      timer = PWMrefresh-PWMtimeBrightness;  //remaining OFF portion of the cycle = dimming
      state = 3;
      break;
   }  //end switch
   if (timer<500) timer = 500;  //safety only... (a too-short timer1 reload can miss the interrupt entirely on 8266)

    //  if ((pos>0) && (num<=9)) num = convert[num];   //tube character conversion, if needed... (maybe bad pin numbering)

  // num>9 (BLANK=10, MINUS=11, DP glyph=12, ...) intentionally falls through to "blank digit":
  // the 74141 itself also blanks all outputs for BCD inputs 10-15, so this check is really a
  // software-side shortcut that avoids driving ABCDPins at all for non-numeric glyphs.
  if ((brightness == 0) || (state == 3) || (num >9) || (!radarON)) {  //blank digit
    state = 0;
    digitalWrite(digitEnablePins[pos],LOW);  //switch off anode
    #if DP_PIN >=0
      digitalWrite(DP_PIN,LOW);
    #endif
    }
  else {
      for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }  //push BCD bits to the shared 74141 inputs (LSB first, bit i <-> ABCDPins[i])
      digitalWrite(digitEnablePins[pos],HIGH);    //switch on the new digit
      #if DP_PIN >=0
        // Some tube types (Z574M) have their decimal-point cathode wired so it visually
        // sits to the LEFT of the printed digit; LEFTDECIMAL compensates by lighting the
        // DP together with the NEXT tube position instead of the current one.
        if (LEFTDECIMAL) DPpos = min(maxDig-1,pos+1); else DPpos = pos;
        if (digitDP[DPpos]) digitalWrite(DP_PIN,HIGH); //switch ON decimal point, if needed
      #endif
  }

  #if COLON_PIN >= 0
    if (num==10) digitalWrite(COLON_PIN,LOW);      // Colon pin OFF
    else digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
  #endif

  #if DECIMALPOINT_PIN >=0
        if (num==10) {digitalWrite(DECIMALPOINT_PIN,LOW);}
        else {digitalWrite(DECIMALPOINT_PIN,decimalpointON); }
  #endif

  timer1_write(timer);  //re-arm the single-shot timer1 for the next phase (see note at top of function)
}

void clearTubes() {
      for (int i=0;i<maxDig;i++) digitalWrite(digitEnablePins[i],LOW);  //drop every digit-enable line so no tube anode is active
      #if DP_PIN >= 0
        digitalWrite(DP_PIN,LOW);
      #endif
}

// Not implemented for this driver: the multiplex ISR (writeDisplay) already refreshes
// continuously on its own, so there's no separate "blocking single write" path here
// (unlike the no_multiplex*.ino drivers, which are not otherwise refreshed by an ISR loop
// in the same way and rely more on this being a real no-op too, kept only for interface parity).
void writeDisplaySingle() {}
#endif

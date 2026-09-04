// ============================================================================
// Nixie driver: NON-multiplexed, one 74141 BCD decoder LATCHED per tube
// ----------------------------------------------------------------------------
// Hardware: 4 digit Nixie Clock, 2x 74HC595N shift register + 4x 74141
// (or 6 tubes with more 74595/74141 chips, depending on clocks.h). Unlike the
// multiplex74141*.ino drivers, there is NO time-division sharing of a single
// decoder across tubes: EVERY tube has its own 74141, and all of them are fed
// from a shared serial shift-register chain (dataPin/clkPin/latchPin, classic
// 74HC595 bit-bang protocol - MSB-of-nibble first, see writeBits()). All tube
// digits are pushed out serially and then latched (latchPin pulse) so that
// ALL tubes display their digit SIMULTANEOUSLY and permanently (no flicker,
// no per-tube ON/OFF multiplexing) - at the cost of needing one 74141 chip
// per tube instead of one shared chip.
// Because there is no per-tube ON slot to shorten/lengthen for dimming, the
// SAME hardware-timer ISR mechanism as the multiplex drivers is reused here
// only to fake brightness control: the whole shift-register chain is
// periodically re-latched with either real digits or an all-blank pattern
// (0xF nibbles) for a portion of each refresh cycle, so perceived brightness
// is still a global ON-time/OFF-time duty cycle, just applied to all tubes at
// once rather than tube-by-tube.
//
// Platform: ESP8266 ONLY. For the ESP32 port see no_multiplex_ESP32.ino.
// Used by these clocks.h configs (search for "#define NO_MULTIPLEX74141"):
//   CLOCK_6, CLOCK_6A
// ============================================================================

#ifdef NO_MULTIPLEX74141

char tubeDriver[] = "NO_MULTIPLEX74141";
#if defined(ESP8266)
#else
  #error "Only 8266 Board is supported!"
#endif

 //change it, if needed for the correct tube sequence
//byte tubes[] = {3,2,1,0};         //4 tubes,   old clock...   in clocks.h
//byte tubes[] = {5,4,3,2,1,0};   //6 tubes, reverse order      in clocks.h
// tubes[] maps PHYSICAL shift-register output position -> LOGICAL digit[] index,
// so a clock's tube wiring order (which may not match the natural digit order)
// can be corrected purely in clocks.h without touching this driver's logic.

int maxDigits = sizeof(tubes);  //tubes is byte[], so sizeof == tube count
// NOTE: despite the "msec" comment, these are raw timer1 tick counts (see startTimer():
// TIM_DIV16 => ~312.5kHz => ~3.2us/tick), not real milliseconds.
int PWMrefresh=12000;   //msec, Multiplex time period. Greater value => slower multiplex frequency
int PWM_min = 1000;
int PWM_max = 12000;
//int PWMtiming[11] = {1000,1000,2000,3000,4000,5000,6000,7000,8000,10000,12000};

//#define dataPin  14  //D5
//#define latchPin 12  //D6
//#define clkPin   13  //D7

// Not implemented for this driver: brightness/blanking is handled entirely inside the
// timer ISR (writeDisplay) by periodically re-shifting an all-blank pattern, so there is
// no separate blocking "write once" path. Kept only for interface parity.
void writeDisplaySingle() { }

void setup_pins(){
  DPRINTLN("Nixie clock - setup pins NON-MULTIPLEX 4(or 6) x 74141");
  pinMode(dataPin, OUTPUT); regPin(dataPin,"dataPin");
  pinMode(latchPin,OUTPUT); regPin(latchPin,"latchPin");
  pinMode(clkPin,OUTPUT);   regPin(clkPin,"clkPin");
  startTimer();
}

// Bit-bangs one 4-bit BCD nibble (one tube's worth of data) into the 74HC595 shift
// register chain, MSB-first. Runs inside the timer ISR (ICACHE_RAM_ATTR keeps it in IRAM
// so it stays callable even while flash is busy), so it cannot use delayMicroseconds()
// (which is not ISR-safe / not guaranteed available) - the "for t<10 nop" loops are a
// crude but deterministic ~100ns busy-wait to satisfy the 74HC595's minimum clock
// pulse-width/setup-time requirements.
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

void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  static volatile byte state=0;
  static int timer = PWMrefresh;
  static int brightness;
  static int PWMtimeBrightness=PWM_min;
  byte animM;
  
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    timer1_write(PWMrefresh);
    return;
  }

  // POSSIBLE BUG / design limitation: this collapses each tube's per-position animMask[i]
  // (0..20 wipe-progress value) into a SINGLE global `animM`, keeping only the last
  // nonzero one found while scanning left to right. Because ALL tubes are re-latched
  // together every ISR call (see the loop below), the actual ON/OFF *timing* of the wipe
  // transition (computed further down from `animM`) is necessarily shared by every tube,
  // even though the choice of WHICH digit to show per tube (old vs new, via animMask[i]==0)
  // is still made individually. So if two tubes are mid-wipe with different animMask
  // values at the same moment, only one of them will actually be timed correctly; unlike
  // the multiplex74141*.ino drivers (where each tube gets its own ISR pass and thus its
  // own independent timing), true per-tube staggered animation isn't achievable here.
  animM = 0;
  for (int i=0;i<maxDigits;i++) {
    if (animMask[i] > 0) animM = animMask[i];    //find, if any animation is wanted?
  }

  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  digitalWrite(latchPin, LOW);  //keep 74141 outputs frozen on their OLD latched values while we shift in new data

  if ((brightness ==0) || !radarON){
    for (int i=0;i<maxDigits;i++) writeBits(0xA);  //blank display  (0xA..0xF are all "invalid BCD" -> 74141 outputs nothing, i.e. blank)

  #if DECIMALPOINT_PIN>=0
    digitalWrite(DECIMALPOINT_PIN,LOW);
  #endif
  #if COLON_PIN >= 0
    digitalWrite(COLON_PIN,LOW);  // colon pin OFF
  #endif
  digitalWrite(latchPin, HIGH);  //commit the all-blank pattern to the 74141 outputs
  timer1_write(100*PWMrefresh);  //display fully off (radar/night-off) - just idle-poll slowly instead of running the PWM state machine
  return;
  }
  else {
    // Shift out one BCD nibble per tube, all in a single ISR pass (this is what makes
    // the driver "non-multiplexed": every tube gets its data on every call, rather than
    // only the currently-selected tube as in the multiplex74141*.ino drivers).
    for (int i=0;i<maxDigits;i++) {
      if (state==0) {
        if (animMask[i]==0) writeBits(digit[tubes[i]]);
        else writeBits(oldDigit[tubes[i]]);
      }
      else if (state==1) writeBits(newDigit[tubes[i]]);
      else writeBits(0xF);  //state 2 (blank phase of the brightness duty cycle): 0xF is any non-BCD code, forces the 74141 output off
    } //end for

  //for (int i=0;i<maxDigits;i++) writeBits(digit[tubes[i]]);   //simple test, without animation
  
  // 3-state duty-cycle machine (global, applies to the whole tube bank at once - see the
  // note above about animM): state 0 = show current/old digits for the brightness ON
  // time (optionally starting a wipe if animM>0); state 1 = show the "new" half of a wipe
  // for the ON time; state 2 = blank all tubes for the remaining OFF time (dimming).
  // When brightness is already MAXBRIGHTNESS, state 2 is skipped entirely (goes straight
  // back to state 0) since there's no need to ever blank at 100% duty cycle.
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
  timer1_write(timer);
}

// Not implemented for this driver: there's no persistent "enable" line to blank, since
// blanking is achieved by re-shifting an all-off (0xA-0xF) pattern from within
// writeDisplay() itself; a real clearTubes() would need to bit-bang a blank frame here too.
void clearTubes() {}
#endif

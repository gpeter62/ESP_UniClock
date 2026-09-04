//============================================================================
// HV5122 Nixie tube driver (CURRENT / PREFERRED VERSION - "NEW")
//----------------------------------------------------------------------------
// Chip:     Microchip/Supertex HV5122 - 32-channel high-voltage (up to ~170V)
//           serial-in shift register / latch, used here to drive Nixie tube
//           cathodes directly (no external cathode-driver transistors needed).
// Wiring:   Two HV5122 chips are cascaded (bit-banged, SPI-like, 3-wire bus):
//             PIN_DIN -> chip0 DIN, chip0 DOUT -> chip1 DIN (daisy-chained),
//             PIN_CLK -> both chips' CLK (shared),
//             PIN_OE  -> both chips' output-enable (shared, active HIGH here
//                        i.e. energizes/blanks the tubes - see writeDisplay()).
//           Together the two chips give 64 open-drain HV outputs, addressed
//           via the digitPins[][] table (pin numbers 1-32 = chip0, 101-132 =
//           chip1; see clocks.h for the actual per-clock wiring tables).
// Status:   This is the CURRENT driver: every HV5122-based config in
//           clocks.h (CLOCK_50, 51, 52, 53, 54, 55, 56, 57, 57A) defines
//           NEWHV5122, so this file is the one that actually gets compiled.
//           The plain "HV5122" macro (see HV5122.ino) is no longer selected
//           by any active config - that file is legacy/orphaned.
// What's new vs. HV5122.ino: writeDisplay() (the timer ISR) now runs a
//           3-phase state machine (old digit -> new digit -> blank) that
//           implements the digit-change "wipe" animation (animMask[] /
//           newDigit[] / oldDigit[], see changeDigit() mode 5 in the core
//           .ino) by time-slicing how long the OLD vs. NEW cathode pattern
//           is shown within one PWM refresh period. writeDisplaySingle() also
//           now only recomputes the shift-out bit buffers when digit/newDigit/
//           animMask actually changed (memcmp-cached), instead of
//           unconditionally re-shifting every 50ms.
//============================================================================
#ifdef NEWHV5122  //Nixie driver
//P.S. 6x tubes PCB version

char tubeDriver[] = "HV5122NEW";
//HV5122 control pins
//#define PIN_DIN  17   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
//#define PIN_CLK  22   // Clock
//#define PIN_OE   21   // OutputEnable

//int maxDigits = 6;

//Data pin numbers 100+ means: chip1 pins are used. Chip0-s DOUT is connected to chip1's DIN
/* Example:
  byte digitPins[maxDigits+1][10] = {
    {2,10,9,8,7,6,5,4,3,1},                      //sec  1 , chip0  (tube#0)
    {11,32,20,19,18,17,16,15,14,13},             //sec  10 , chip0 (tube#1)
    {22,31,29,30,27,28,25,26,23,24},             //min   1 , chip0 (tube#2)
    {101,131,110,109,108,107,106,105,104,103},   //min  10 , chip1 (tube#3)
    {111,132,120,119,118,117,116,115,114,113},   //hour  1 , chip1 (tube#4)
    {122,129,130,127,128,125,126,123,124,121},   //hour 10 , chip1 (tube#5)
    {0,12,21,102,112,0,    0,0,0,0}              //extra decimal point (tube0...tube6)
    };   
*/

// DRAM_ATTR: forces these globals to live in internal DRAM rather than being
// placed somewhere the ISR might reach via the flash cache. On ESP32, if a
// flash-cache miss/disable happens to coincide with a SPIFFS/OTA/flash write
// while this IRAM_ATTR ISR (writeDisplay(), below) touches these variables,
// an un-cached access could stall or crash; DRAM_ATTR keeps them reliably
// accessible from the ISR at all times.
uint32_t  DRAM_ATTR bitBuffer0 = 0;      //new digits - bit pattern for chip0, prepared by writeDisplaySingle(), consumed by the writeDisplay() ISR
uint32_t  DRAM_ATTR bitBuffer1 = 0;      //new digits - chip1
uint32_t  DRAM_ATTR oldBitBuffer0 = 0;   //old (pre-change) digits - chip0, shown briefly during the wipe/crossfade animation
uint32_t  DRAM_ATTR oldBitBuffer1 = 0;   //old (pre-change) digits - chip1
int DRAM_ATTR animPtr = 0;               //0 = no animation in progress; otherwise the animMask[] value driving how the old/new time split is weighted (see writeDisplaySingle())

#define SHIFT_LSB_FIRST false  //true= LSB first, false= MSB first

//Brightness PWM timing. Greater value => slower brightness PWM frequency
// Suggested values: 100000 / 20000, for faster PWM: 50000 / 10000
uint32_t DRAM_ATTR PWMrefresh = 100000;  //full PWM period length, in timer ticks; also the time-budget shared between old-digit/new-digit/blank phases (see writeDisplay())
uint32_t DRAM_ATTR PWM_min = 6000;       //floor for the ON portion of the cycle, so very low brightness settings still produce a visible/stable flash instead of a near-zero-width pulse

uint32_t DRAM_ATTR time1 = 2000;    //how long (ticks) to show the OLD digit pattern this refresh cycle (state 0 below)
uint32_t DRAM_ATTR time2 = 2000;    //how long (ticks) to show the NEW digit pattern this refresh cycle (state 1 below)
uint32_t DRAM_ATTR offTime = 2000;  //how long (ticks) the display stays blanked (OE off) this refresh cycle (state 2 below) - this is what actually implements the brightness dimming
uint32_t DRAM_ATTR brightness = 0;
uint32_t DRAM_ATTR PWMtimeBrightness;  //time1+time2 combined, i.e. total "lit" time per refresh cycle, derived from brightness/lux

// shift(): bit-bangs one 32-bit word into the HV5122 shift-register chain.
// The ~20-NOP delays around each CLK/DIN edge are required because the
// HV5122 is a high-voltage part (internal level-shifters to the ~170V rail)
// and is much slower than a typical 5V logic shift register - driving it at
// full CPU speed without these delays can corrupt shifted-in data.
// Data is latched into the HV5122 on the falling edge of CLK.
// Runs from the ISR context here (called by writeDisplay() below), unlike in
// HV5122.ino where it's only called from the main-loop writeDisplaySingle().
#if defined(ESP32)
void IRAM_ATTR shift(uint32_t Data) {
#else
void ICACHE_RAM_ATTR shift(uint32_t Data) {
#endif
  static boolean b;

  for (uint32_t i = 0; i < 32; i++) {
    digitalWrite(PIN_CLK, HIGH);
    for (int t=0;t<20;t++) asm volatile ("nop");
    if (SHIFT_LSB_FIRST)
      b = ((Data & (uint32_t(1) << i))) > 0;  //LSB first
    else
      b = (Data & (uint32_t(1) << (31 - i))) > 0; //MSB first
    digitalWrite(PIN_DIN, b);
    for (int t=0;t<20;t++) asm volatile ("nop");
    digitalWrite(PIN_CLK, LOW);  //falling CLK  to store DIN
    for (int t=0;t<20;t++) asm volatile ("nop");
  }
  digitalWrite(PIN_CLK, HIGH);
  for (int t=0;t<20;t++) asm volatile ("nop");
}

void clearTubes() {
  shift(0);
  shift(0);
}

void setup_pins() {
  DPRINTLN("Setup pins -  HV5122 Nixie driver...");
  DPRINT("- CLK   : GPIO"); DPRINTLN(PIN_CLK);
  DPRINT("- DATAIN: GPIO"); DPRINTLN(PIN_DIN);
  DPRINT("- OUTPUT_ENABLE: GPIO"); DPRINTLN(PIN_OE);
  pinMode(PIN_CLK, OUTPUT);  regPin(PIN_CLK, "PIN_CLK");
  pinMode(PIN_DIN, OUTPUT);  regPin(PIN_DIN, "PIN_DIN");
  pinMode(PIN_OE, OUTPUT);   regPin(PIN_OE, "PIN_OE");
  digitalWrite(PIN_CLK, HIGH);
  digitalWrite(PIN_OE, LOW);  //keep outputs blanked until clearTubes() below has pushed a known (all-zero) pattern into the shift registers
  driverSetupStr = "<br>HV5122 pin settings:<br>";
  for (int i=0;i<maxDigits+1;i++) {
    driverSetupStr += String("{");
    for (int j=0;j<10;j++) {
      driverSetupStr += String(digitPins[i][j]); 
      driverSetupStr += (j==9) ? "}," : ",";
    }
    driverSetupStr += String("<br>");
  }
  clearTubes();
  startTimer();
}


// writeDisplay(): TIMER-ISR implementing a 3-phase state machine per refresh
// cycle, re-armed with a different timer delay for each phase:
//   state 0: show the OLD (pre-change) digit pattern for time1 ticks
//   state 1: show the NEW digit pattern for time2 ticks
//   state 2: blank (OE off) for offTime ticks - this is the actual dimming mechanism
//   (state 3 is not a real switch case: it's set inline within state 2's
//    handling and consumed immediately, in the same ISR call, by the
//    "state==3" check below - it never survives to the next ISR invocation)
// time1/time2/offTime/bitBuffer*/oldBitBuffer* are all precomputed once by
// writeDisplaySingle() (main-loop side) whenever the digit/animation state
// changes; this ISR just plays them back every refresh cycle. Showing the OLD
// pattern briefly before the NEW one is what produces the digit-change wipe/
// crossfade animation (animMask[]-driven) - when no animation is active,
// writeDisplaySingle() sets oldBitBuffer==bitBuffer so phases 0 and 1 render
// the identical (new) pattern and no visible transition occurs.
// Each state's "if" falls through to the next case when its time is too short
// (<1000 ticks) to bother with - e.g. if time1<1000 the old-digit phase is
// skipped entirely and state 0 falls straight into state 1's code within the
// same ISR call.
#if defined(ESP32)
void IRAM_ATTR writeDisplay() { //void IRAM_ATTR  writeDisplay(){
#else
void ICACHE_RAM_ATTR writeDisplay() {       //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
#endif

  static DRAM_ATTR byte state=0;
  int timer = PWMrefresh;  //fallback delay if neither state's time threshold is met below (e.g. both time1 and time2 < 1000 at very low brightness)

  intCounter++;
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    #if defined(ESP8266)
      timer1_write(PWMrefresh);
    #endif
    return;
  }

  #if defined(ESP32)
    portENTER_CRITICAL_ISR(&timerMux);
    noInterrupts();
  #endif
  digitalWrite(PIN_OE, LOW); //OFF - blank while shifting new data in, to avoid a brief flash of garbage/transitional segment patterns on the tubes
  switch (state) {   //state machine...
    case 0:   //show old character
      if (time1>=1000) {
        timer = time1;
        shift(oldBitBuffer1);
        shift(oldBitBuffer0);
        state = 1;
        break;
      }
      //time1 too short: fall through to state 1 without a break, skipping the old-digit phase for this cycle
    case 1:  //show new character
      if (time2>=1000) {
        timer = time2;
        shift(bitBuffer1);
        shift(bitBuffer0);
        if (offTime<500) state = 0;      //offTime negligible: skip the blank phase too, go straight back to old-digit next cycle
        else state = 2;
        break;
      }
      //time2 too short: fall through to state 2 without a break
    case 2:  //blank display
      timer = offTime;
      state = 3;  //state 3 exists only to be caught by the "state==3" check just below, within this same ISR call - see comment above
      break;
   }  //end switch

  if (timer<500) timer = 500;  //safety only... (timerAlarmWrite/timer1_write with 0 or a very small value can hang/misfire)

  if ( (state==3) || (!radarON) || (brightness == 0) ) {  //OFF state, blank digit
    digitalWrite(PIN_OE, LOW); //OFF
    state = 0;  //resets the state machine to restart the old->new->blank cycle from state 0 on the next ISR call
    }
  else {  //ON state
    digitalWrite(PIN_OE, HIGH);   //ON  -- NOTE: active-HIGH OE on this board, opposite of the active-low /OE found on many shift-register ICs
  }

#if defined(ESP8266)
  timer1_write(timer);
#elif defined(ESP32)
  portEXIT_CRITICAL_ISR(&timerMux);
  timerAlarmWrite(ESP32timer, timer, true);   
  timerAlarmEnable(ESP32timer);
  interrupts();
#endif
}

// writeDisplaySingle(): recomputes the shift-out bit buffers (bitBuffer0/1,
// oldBitBuffer0/1) and the per-phase timing (time1/time2/offTime) from the
// current digit[]/newDigit[]/animMask[]/digitDP[] state, then hands them off
// (via the shared globals above) to the writeDisplay() ISR to actually shift
// out and time-multiplex. Called from the main loop; guarded by the memcmp
// check below so the (relatively expensive) recompute only runs when
// something actually changed, instead of unconditionally every call like
// HV5122.ino's writeDisplaySingle() does.
void writeDisplaySingle() {
  static byte lastAnimMask[BUFSIZE];
  static byte lastDigit[BUFSIZE];
  static unsigned long lastRun = 0;
  uint32_t b0,b1,ob0,ob1;  //temp buffers
  byte num;

  // POSSIBLE BUG: this early-return only compares digit[]/newDigit[]/animMask[]
  // against their cached values - it does NOT compare digitDP[] (the decimal
  // point / separator-dot buffer). If only digitDP[] changes between calls
  // (e.g. a blinking colon toggled by prm.enableBlink, or MAKE_BLINKING_DOTS
  // mode - both used by this driver's configs, see clocks.h CLOCK_52/54)
  // while digit/newDigit/animMask stay the same, this function returns early
  // and bitBuffer0/1 are never recomputed with the new DP state, so the
  // blink would not actually appear on the tubes. In most normal clock
  // screens this is masked because the seconds digit (or minute digit) also
  // changes in the same tick as the blink, but it could manifest as a
  // frozen/stuck blinking dot on screens where only the dot toggles
  // (e.g. while a static date/temperature reading is shown with blink enabled).
  if ( (memcmp(digit,lastDigit,maxDigits) == 0) && (memcmp(lastDigit,newDigit,maxDigits) == 0) && (memcmp(animMask,lastAnimMask,maxDigits)==0) ) return;
  //if ((millis()-lastRun)<5) return;
  lastRun = millis();

  memcpy(lastAnimMask,animMask,maxDigits);
  memcpy(lastDigit,digit,maxDigits);

  animPtr = 0;
  b0 = 0; b1 = 0;
  for (int i = 0; i < maxDigits; i++) { //Set the clock digits new values
    num = digit[i];
    if (animMask[i]>0) {  //this tube position is mid-animation: use the incoming NEW value here (digit[i] is presumably still the transitional/old value at this point) and remember the mask so we know to build an "old" buffer below too
      animPtr = animMask[i];
      num = newDigit[i];
    }
    if (num < 10) {  //codes 10 (BLANK), 11 (dash), 12 (dp glyph) and 13+ (extra glyphs) have no cathode mapping on a Nixie tube -> leave bits cleared = tube blank
      if (digitPins[i][num] < 100) {
        b0 |= (uint32_t)(1 << (digitPins[i][num]-1)); 
        //DPRINT(digitPins[i][num]); DPRINT(":");
        } //chip0
      else {
        b1 |= (uint32_t)(1 << (digitPins[i][num] - 101)); 
        //DPRINT(digitPins[i][num]);DPRINT(":");} //chip1
      }
    }
  }  //end for i

if (animPtr>0) {  //at least one position is animating -> also build the OLD-pattern buffer, shown during the "state 0" phase in writeDisplay() for the crossfade/wipe effect
  ob0 = 0; ob1 = 0;
  for (int i = 0; i < maxDigits; i++) { //Set the clock digits old values
    num = oldDigit[i];
    if (num < 10) {
      if (digitPins[i][num] < 100) {
        ob0 |= (uint32_t)(1 << (digitPins[i][num]-1)); 
        //DPRINT(digitPins[i][num]); DPRINT(":");
        } //chip0
      else {
        ob1 |= (uint32_t)(1 << (digitPins[i][num] - 101)); 
        //DPRINT(digitPins[i][num]);DPRINT(":");} //chip1
      }
    }
  }  //end for i
}

  #ifdef  MAKE_BLINKING_DOTS //it means, the extra datapins are used as 4 blinking dot instead of decimal points!
    if (showClock) {
      digitDP[1] = digitDP[2]; //left two dots
      digitDP[3] = digitDP[4]; //right two dots
    }
  #endif
  
  for (int i = 0; i < maxDigits-1; i++) { //Set the extra decimal point dots
    if (digitDP[i] && digitPins[maxDigits][i]>0) {
      //DP bits are ORed into BOTH the new (b0/b1) and old (ob0/ob1) buffers, so
      //decimal points/separator dots stay lit continuously through the old->new
      //wipe animation instead of blinking off during the old-digit phase.
      if (digitPins[maxDigits][i] < 100) {
        b0  |= (uint32_t)(1 << (digitPins[maxDigits][i]-1)); //chip0
        ob0 |= (uint32_t)(1 << (digitPins[maxDigits][i]-1)); //chip0
      }
      else {
        b1  |= (uint32_t)(1 << (digitPins[maxDigits][i] - 101)); //chip1
        ob1 |= (uint32_t)(1 << (digitPins[maxDigits][i] - 101)); //chip1
      }
    }
  }  //end for i



  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  if (autoBrightness && displayON)
    PWMtimeBrightness = max(PWM_min, (PWMrefresh*lx)/MAXIMUM_LUX);   //day mode + ALS: scale total lit-time by measured lux instead of the stored brightness setting
  else
    PWMtimeBrightness = max(PWM_min, (PWMrefresh*brightness)/MAXBRIGHTNESS);

  //Below a certain lit-time (PWMtimeBrightness<15000, i.e. brightness/lux is
  //low), skip the wipe animation altogether: the old/new split would be too
  //fast to see anyway, and at very short lit-times splitting it further into
  //old+new sub-phases risks pulses too short for the HV5122/tubes to respond
  //to cleanly. animPtr=10 forces an even old:new time split further below,
  //but since ob0/ob1==b0/b1 here, phase 0 and phase 1 render identically
  //(no visible transition) regardless of the split.
  if ((animPtr==0) || (PWMtimeBrightness<15000)) {  //no animation #5
    ob0 = b0;
    ob1 = b1;
    animPtr = 10;  //to make equal timing
  }

  //Split the total lit time (PWMtimeBrightness) between the OLD-digit phase
  //(time1) and NEW-digit phase (time2) according to animPtr, which walks from
  //a low value (just-started animation, mostly OLD digit still shown) up to
  //~20 (animation complete, mostly/only NEW digit) as the wipe animation
  //progresses across successive changeDigit() calls - see animMask[] in the
  //core .ino. animPtr==10 (forced above when not animating) gives an even
  //50/50 split, which is harmless there since ob0/ob1==b0/b1 already.
  time1 = PWMtimeBrightness * (20-animPtr)/20;
  if (time1<1000) time1=0;                              //too short to bother with a separate old-digit phase -> writeDisplay() will fall through and skip it
  if (time1 > (PWMtimeBrightness-1000)) time1 = PWMtimeBrightness;  //leave less than 1000 ticks for time2 -> just give the old phase the whole lit budget instead
  time2 = PWMtimeBrightness - time1;
  offTime = PWMrefresh - time1 - time2;  //remaining budget of the refresh period is spent blanked - this (not time1/time2) is what actually produces the brightness dimming

  bitBuffer0 = b0;
  bitBuffer1 = b1;
  oldBitBuffer0 = ob0;
  oldBitBuffer1 = ob1;

//  DPRINT("Br:"); DPRINT(brightness);  DPRINT(" A:"); DPRINT(animPtr);  
//  DPRINT(" PTB"); DPRINT(PWMtimeBrightness);
//  DPRINT(" t1:"); DPRINT(time1); DPRINT(" t2:"); DPRINT(time2); DPRINT(" off:"); DPRINT(offTime);
//  DPRINT(" "); DPRINT(b0,HEX); DPRINT(" ");   DPRINT(b1,HEX); DPRINT(" "); 
//  DPRINT(ob0,HEX); DPRINT(" ");   DPRINTLN(ob1,HEX);   

} 


//For testing -------------------
/*
void test() {
  digitalWrite(PIN_OE, HIGH);
  clearTubes();
  for (int j=0;j<2;j++)
    for (uint32_t i=0;i<32;i++) {
      DPRINT("D"); DPRINT(i+1); DPRINT(": ");
      shift((uint32_t)(1 << (i))); 
      DPRINTLN((uint32_t)(1 << (i)),HEX);
      delay(3000);
    }
}

void testTubes2() {
while (true) {
   
  DPRINTLN("Testing tubes: ");
  for (int i = 0; i < 10; i++) {
    DPRINT(i); DPRINT(" ");
    for (int j = 0; j < maxDigits; j++) {
      digit[j] = i;
      digitDP[j] = i % 2;
    }
    //changeDigit();
 
    writeDisplaySingle();
    delay(3000);
    yield();
  }
  }  //end while
}

void showBits(uint32_t bits) {
  boolean b;

  for (uint32_t i = 0; i < 32; i++) {
     if (SHIFT_LSB_FIRST)
      b = ((bits & (uint32_t(1) << i))) > 0;  //LSB first
    else
      b = (bits & (uint32_t(1) << (31 - i))) > 0; //MSB first
    if (b) {
      DPRINT("1");
    }
    else {
      DPRINT("0");
    }
  }  //end for
  DPRINT("-");
}

*/
#endif

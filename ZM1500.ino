// ============================================================================
// ZM1500 Clock driver by UNFI
// ----------------------------------------------------------------------------
// Hardware: 4x 74HC595N serial-in/parallel-out shift registers (32 bits total)
// feeding a bank of optocouplers ("opto PCB" wiring), which in turn switch the
// actual ZM1500 (12-position) display element grid/segments. Data is bit-banged
// out over a plain 3-wire interface (dataPin/clkPin/latchPin - classic 74HC595
// SPI-like protocol: shift bits in on clkPin rising edges while dataPin holds
// the current bit, then pulse latchPin to move the whole shift-register content
// to the (glitch-free) output latches in one go).
//
// This is the driver currently active in clocks.h (CLOCK_72 defines
// ZM1500_ESP32 - see clocks.h). The 32-bit value sent out each refresh packs
// the 8 segment-enable bits (from segmentEnablePins[], bits 0-7) and the
// 12 possible digit/tube-select bits (from digitEnablePins[], each shifted up
// by +8, so bits 8-19) into one word, of which only the low 20 bits are
// actually clocked out (see the 20-iteration loop in writeDisplay() below) -
// this only supports the single "opto PCB" pin layout (no alternate
// "transistor PCB" wiring mode), unlike an earlier work-in-progress revision
// of this file that attempted to support both via a SEGMENTS_FIRST toggle.
// That revision was abandoned half-finished and has been removed; this is the
// last known-working version, restored from a backup (ZM1500 .sav).
// ============================================================================

#ifdef ZM1500_ESP32

char tubeDriver[] = "ZM1500_ESP32";
#if defined(ESP32)
#else
  #error "Only ESP32 Board is supported!"
#endif

// NOTE: despite the "msec" wording in the original comment, these are raw ESP32 hardware-timer
// tick counts passed straight into timerAlarmWrite() below, not real milliseconds - the actual
// tick period depends on the timer's prescaler (set up once, elsewhere, when the timer object is
// created in the core .ino's startTimer()). Treat these as "bigger number = longer phase", not as
// an absolute time unit.
int DRAM_ATTR PWMrefresh=11000;   //msec, Multiplex time period. Greater value => slower multiplex frequency
int DRAM_ATTR PWM_min = 2000;
int DRAM_ATTR PWM_max = 10000;

//#define dataPin  14  //D5
//#define latchPin 27 //12  //D6
//#define clkPin   26 //13  //D7

// Segment bit patterns per displayable glyph code (index = the value stored in digit[]/newDigit[]).
// Each byte is a bitmask over 8 abstract "segment slots" (bit7=a .. bit0=DP by convention elsewhere in
// this codebase); generateBitTable() below translates these abstract bits into the actual physical
// shift-register output bits via segmentEnablePins[] (defined per-CLOCK_NN in clocks.h). Codes 0-9 are
// digits, 10=blank, 11=minus, 12=a standalone decimal-point glyph, 13+ are extra characters (A, P, C,
// degree symbol, %, etc.) used for date/temperature/humidity/IP-address display - see clocks.h's
// TEMP_CHARCODE/GRAD_CHARCODE/PERCENT_CHARCODE and the core file's displayTemp()/displayHumid()/writeIpTag().
byte DRAM_ATTR charDefinition[] = {
                   B11111100,   //0: abcdef
                   B01100000,   //1: bc 
                   B11011010,   //2: abdeg
                   B11110010,   //3: abcdg
                   B01100110,   //4: bcfg
                   B10110110,   //5: acdfg
                   B10111110,   //6: acdefg
                   B11100000,   //7: abc
                   B11111110,   //8: abcdefg
                   B11110110,   //9: abcdfg
                   B00000000,   // : BLANK (10)
                   B00000010,   //-: minus (11)
                   B00000001,   // decimal point (12)
                   B11101110,   // A  abcefg (13)
                   B11001110,   // P  abefg (14)
                   B10011100,   // C  adef (15)
                   B11000110,   //grad  (upper circle) abfg (16)
                   B10110100,   //%  acdf  (17)
                   B00111010,   //lower circle cdeg  (18)                  
                   B01100000,   //I  bc    (19)
                   B10001110    //F  aefg  (20)
};

#define MAXCHARS sizeof(charDefinition)
#define MAXSEGMENTS sizeof(segmentEnablePins)
int maxDigits = sizeof(digitEnablePins);

uint32_t DRAM_ATTR charTable[MAXCHARS];    //generated pin table from segmentDefinitions
uint32_t DRAM_ATTR segmentEnableBits[MAXSEGMENTS];   //bitmaps, generated from EnablePins tables
uint32_t DRAM_ATTR animationMaskBits[5];
uint32_t DRAM_ATTR digitEnableBits[12];

// This driver refreshes the whole display purely from the writeDisplay() ISR below (hardware-timer
// driven multiplex), so there is nothing left to do for a "blocking, one-shot" write - required by
// the common driver interface, but intentionally a no-op here.
void IRAM_ATTR writeDisplaySingle() { }

void setup_pins(){
  DPRINT("ZM1500 clock ");  DPRINT(maxDigits); DPRINTLN(" digits - setup pins");
  pinMode(dataPin, OUTPUT); regPin(dataPin,"dataPin");
  pinMode(latchPin,OUTPUT); regPin(latchPin,"latchPin");
  pinMode(clkPin,OUTPUT);   regPin(clkPin,"clkPin");
  digitalWrite(latchPin,LOW);
  digitalWrite(clkPin,LOW);
  generateBitTable();   //pre-compute charTable[]/digitEnableBits[]/animationMaskBits[] from clocks.h's pin arrays
  startTimer();         //arms the ESP32 hardware timer that drives writeDisplay() below
}

// Hardware-timer ISR: multiplexes one digit position per "ON" call, using a two-phase (ON/OFF)
// duty-cycle so the *ratio* of ON time to OFF time controls perceived brightness (a software PWM),
// then re-arms itself (timerAlarmWrite/timerAlarmEnable) with however long the next phase should
// last. Runs continuously in the background once startTimer() has been called from setup_pins().
void IRAM_ATTR writeDisplay(){  //void IRAM_ATTR  writeDisplay(){

  static DRAM_ATTR int pos = maxDigits;
  static DRAM_ATTR boolean state=0;
  static DRAM_ATTR int brightness;
  static DRAM_ATTR uint32_t PWMtimeBrightness=PWM_min;
  uint32_t timer = PWMrefresh;
  uint32_t val = 0;

  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();
  intCounter++;

  if (state) {  //ON state: advance to the next digit position and compute its bit pattern
    pos++;
    if (pos>maxDigits-1)  {   //go to the tube#0
      pos = 0;
      // Brightness (and the resulting PWMtimeBrightness ON-duration) is only recomputed once per
      // full multiplex sweep (i.e. when we wrap back to position 0), not on every single digit -
      // this keeps all digits at the same, consistently-updated brightness within one sweep.
      brightness = displayON ? prm.dayBright : prm.nightBright;
      if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety
      if (autoBrightness && displayON) {   //change brightness only on the tube#0
        PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
        }
      else
        PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);
    }
    // Look up this position's live glyph, apply the wipe-animation mask (changeDigit()'s animMode 5,
    // in the core .ino) and OR in a standalone decimal-point glyph (index 12) if this tube's dot
    // should be lit - digit[]/animMask[]/digitDP[] are all written by the core file's display
    // pipeline (displayTimeX()/changeDigit()) before this ISR ever reads them.
    val = uint32_t(charTable[digit[pos]]);  //the full bitmap to send to the shift-register chain
    if (animMask[pos]>0) val &= uint32_t(animationMaskBits[animMask[pos]-1]);  //animationMode 6, mask characters from up to down and back
    if (digitDP[pos]) val = val | uint32_t(charTable[12]);    //Decimal Point

    timer = PWMtimeBrightness;
    timerON = timer;   //debug only
    timerOFF = PWMrefresh-PWMtimeBrightness;  //debug only
  }
  else {  //OFF state: blank the shift-register output for the remainder of this digit's time-slice
    timer = PWMrefresh-PWMtimeBrightness;
    val = 0;
  }

  if (timer<500) timer = 500;  //safety only...
  if ( (brightness == 0) || (!radarON))  val = 0; //blank digit
  val = val | digitEnableBits[pos];   //combine segment bits (val) with this tube's select bit

  // Bit-bang val out LSB-first over 20 clock pulses - only the low 20 bits of the 32-bit val
  // actually reach the hardware (8 segment bits + up to 12 digit-select bits, see the file header).
  for (int i=0; i<20; i++)  {
    if (val & uint32_t(1 << i))   //reverse: (1 << (19 - i)))
      digitalWrite(dataPin, HIGH);
    else
      digitalWrite(dataPin, LOW);
    digitalWrite(clkPin,HIGH);  //asm volatile ("nop");  //Clock impulse
    digitalWrite(clkPin,LOW);   //asm volatile ("nop");
  } //end for

  // Pulse latchPin to atomically move the just-shifted-in 20 bits from the 74HC595's internal shift
  // register to its output latches - this is the instant the physical display actually changes.
  digitalWrite(latchPin,HIGH);  //asm volatile ("nop");
  digitalWrite(latchPin,LOW);
  state = !state;

  portEXIT_CRITICAL_ISR(&timerMux);
  timerAlarmWrite(ESP32timer, timer, true);   //re-arm the hardware timer for the just-computed phase duration
  timerAlarmEnable(ESP32timer);
  interrupts();
}


// One-time setup helper (called from setup_pins()): pre-computes the lookup tables writeDisplay()'s
// ISR needs every cycle, so the ISR itself never has to do bit-shifting arithmetic based on the raw
// clocks.h pin arrays - just plain table lookups, keeping the interrupt handler fast.
void generateBitTable() {
uint32_t out;

  DPRINTLN("--- Generating segment pins bitmap ---");
  for (int i=0;i<MAXSEGMENTS;i++) {
    segmentEnableBits[i] = uint32_t(1<<segmentEnablePins[i]);
    DPRINT(i); DPRINT(": "); DPRINTLN(segmentEnableBits[i],BIN);
  }
  animationMaskBits[0] = uint32_t(1<<segmentEnablePins[0]);  //a
  animationMaskBits[1] = animationMaskBits[0] | uint32_t(1<<segmentEnablePins[1]) | uint32_t(1<<segmentEnablePins[5]);  //bf
  animationMaskBits[2] = animationMaskBits[1] | uint32_t(1<<segmentEnablePins[6]);  //g
  animationMaskBits[3] = animationMaskBits[2] | uint32_t(1<<segmentEnablePins[4]) | uint32_t(1<<segmentEnablePins[2]);  //ec
  animationMaskBits[4] = animationMaskBits[3] | uint32_t(1<<segmentEnablePins[3]);  //d
  DPRINTLN("--- Animation mask ---");
  for (int i=0;i<5;i++) {
    animationMaskBits[i] = ~animationMaskBits[i]; //invert bits
    DPRINTLN(animationMaskBits[i],HEX);
  }

DPRINTLN("---- Generated Character / Pins table -----");
  for (int i=0;i<MAXCHARS;i++) {
    out = 0;
    DPRINT(i); DPRINT(":  ");
    DPRINT(charDefinition[i],BIN);  DPRINT(" = ");
    for (int j=0;j<=7;j++)   //from a to g
      if ((charDefinition[i] & 1<<(7-j)) != 0) {
        out = out | segmentEnableBits[j]; //DPRINT("1"); 
        }
    //else        DPRINT("0");
    //DPRINT("  >> ");  
    
    charTable[i] = out;
    DPRINTLN(charTable[i],BIN);
  }  //end for

  // "+8" shifts the digit-select bit up past the 8 segment-enable bit positions (0-7, from
  // segmentEnablePins[] above), so a combined value (segment bits | digitEnableBits[pos]) never
  // collides: segment data occupies bits 0-7, tube selection occupies bits 8-19 (up to 12 tubes).
  DPRINTLN("--- Generating digit pins bitmap ---");
  for (int i=0;i<maxDigits;i++) {
    digitEnableBits[i] = uint32_t(1 << uint32_t(digitEnablePins[i]+8));
    DPRINT(i); DPRINT(": "); DPRINT(digitEnableBits[i],HEX); DPRINTLN(" ");
  }

  // Debug-only sanity dump: shows what writeDisplay() would send for each tube if it always
  // displayed its own character-table index (not the real runtime digit[] value) - lets a developer
  // eyeball the generated bitmaps against the datasheet/wiring before trusting the live display.
  DPRINTLN("--- DEMO ---");
  for (int i=0;i<maxDigits;i++) {
    uint32_t val = uint32_t(charTable[i]);  //the full bitmap to send to the shift-register chain
    val = val | digitEnableBits[i];
    DPRINT(i); DPRINT(": "); DPRINTLN(val,HEX);
  }
}

// Empty no-op: nothing in the core .ino calls clearTubes() any more (its only former caller,
// disableDisplay(), has been removed entirely as dead code), so this is purely for interface
// compliance. If an active blank is ever needed, this would have to bit-bang an all-zero frame
// out to the shift-register chain instead of just returning.
void IRAM_ATTR clearTubes() {}
#endif

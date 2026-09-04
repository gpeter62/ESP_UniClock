#ifdef MAX6921_ESP32
//===========================================================================================
// Driver for the MAXIM MAX6921 20-output VFD/vacuum-tube driver chip -- ESP32 variant.
// This is the ESP32 port of MAX6921.ino; the protocol/chip behaviour is identical, only the
// platform-specific timer/interrupt plumbing differs (ESP32 hardware timer + spinlock instead
// of ESP8266 timer1; IRAM_ATTR/DRAM_ATTR instead of ICACHE_RAM_ATTR). See MAX6921.ino's header
// for the full protocol writeup (SPI-like DATA/CLK/LE shift register + BLANK).
//
// Hardware / protocol summary:
//   20-bit serial shift register + latch, MSB (OUT19) first on DATA, one CLK pulse per bit,
//   LE (Latch Enable) pulses HIGH->LOW after the 20 bits to atomically transfer the shift
//   register into the output latch, BL (BLANK, active HIGH) forces all outputs off. Which
//   output pin drives a segment vs. a tube's digit-select grid is purely PCB wiring, described
//   by segmentEnablePins[8]/digitEnablePins[] in clocks.h.
//
// Used by (see clocks.h, search for "#define MAX6921_ESP32"):
//   CLOCK_26 - ESP32 D1 mini, UNFI 6x IV-11 VFD tubes clock, SHT21 sensor
//   CLOCK_30 - ESP32, UNFI PCB clock, 6x IV-11 VFD tubes
//   CLOCK_32 - ESP32, UNFI 6x IV-11 VFD tubes clock, DHT22 sensor
//   CLOCK_40 - V1 ESP32, UNFI 6x IV-11 VFD tubes clock
//   CLOCK_41 - V2 ESP32, UNFI 6x IV-11 VFD tubes clock (jumper wire)
//   CLOCK_42 - V3 ESP32, UNFI 6x IV-11 VFD tubes clock
//   CLOCK_47 - V3 ESP32, UNFI 10-LT-50G VFD tubes clock
//   (all ESP32 boards -- the ESP8266 counterpart configs use MAX6921.ino instead, see that
//   file's header for their CLOCK_NN list.)
//===========================================================================================
//VFD driver for ESP32
  char tubeDriver[] = "MAX6921_ESP32";
  #if defined(ESP32)
  #else
    #error "Only ESP32 board is supported by this driver!"
  #endif

// POSSIBLE BUG (minor / dead code): declared but never assigned or referenced anywhere else in
// this file (or the project, unlike the same-named `maxDig` pattern used by e.g.
// multiplex74141_ESP32.ino / Numitron_4511N.ino, where it actually caches maxDigits and is used
// in the ISR). Here it is vestigial and can be removed; `maxDigits` is used directly instead.
int DRAM_ATTR maxDig;   //memory variable version of maxDigits
//------------------abcdefgDP----------------   definition of different characters
// Each byte below is one glyph's segment pattern, bit7..bit0 = a,b,c,d,e,f,g,DP (a=MSB).
// See generateBitTable(): these abstract a..g/DP bits get remapped through segmentEnablePins[]
// (the PCB-specific wiring) into the actual MAX6921 OUTx bits, producing charTable[].
// Indices 0-9 are digits; 10-20 are special glyphs referenced via the digit[] buffer.
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

uint32_t DRAM_ATTR animationMaskBits[5];   //precomputed AND-masks for the "wipe" transition animation (animMask[]), see generateBitTable(). DRAM_ATTR pins these in internal RAM so the IRAM-resident ISR can access them without touching flash-cached memory.

// NOTE: MAXCHARS/MAXSEGMENTS/maxDigits rely on sizeof() over `byte` arrays (1 byte/element),
// so sizeof() == element count here. segmentEnablePins[] must always have exactly 8 entries
// (a,b,c,d,e,f,g,DP) per the clocks.h convention; digitEnablePins[] length = number of tubes.
#define MAXCHARS sizeof(charDefinition)
#define MAXSEGMENTS sizeof(segmentEnablePins)
int maxDigits =  sizeof(digitEnablePins);

uint32_t DRAM_ATTR charTable[MAXCHARS];              //generated pin table from segmentDefinitions
uint32_t DRAM_ATTR segmentEnableBits[MAXSEGMENTS];   //bitmaps, generated from EnablePins tables
// FIXED (was fixed-size [10] regardless of the actual maxDigits): sized to BUFSIZE (12, see the
// digit[]/newDigit[]/animMask[] buffers in ESP32_UniClock2.ino), the same ceiling the rest of the
// codebase uses for tube-position buffers, so this can no longer silently overflow in
// generateBitTable() for any config the system otherwise supports.
uint32_t DRAM_ATTR digitEnableBits[12];

// Multiplex/brightness timing. Despite the variable names, these are NOT milliseconds -- they
// are raw ESP32 hardware-timer tick counts (see startTimer() in ESP32_UniClock2.ino: PRESCALER=15
// divides the 80MHz APB clock down to roughly a few MHz, so each tick is on the order of 0.2us).
// So PWMrefresh=5500 ticks is roughly ~1ms per ON/OFF half-cycle, PWM_min/PWM_max similarly small.
// Greater PWMrefresh => slower multiplex/refresh frequency (more visible flicker risk).
// All DRAM_ATTR here for the same reason as above: read from inside an IRAM-resident ISR.
int DRAM_ATTR PWMrefresh=5500;   //Multiplex time period, in timer ticks (not milliseconds, despite the variable name -- see note above). Greater value => slower multiplex frequency
int DRAM_ATTR PWM_min = 500;
int DRAM_ATTR PWM_max = 5000;
//int DRAM_ATTR PWMtiming[11] = {0,500,800,1200,2000,2500,3000,3500,4000,4500,5000};

//-----------------------------------------------------------------------------------------

//https://sub.nanona.fi/esp8266/timing-and-ticks.html
//One tick is 1us / 80 = 0.0125us = 12.5ns on 80MHz
//One tick is 1us / 80 = 0.0125us = 6.25ns on 160MHz
// 1 NOP is 1 tick

// Dead/no-op helper: `d` is ignored, so this always executes exactly three NOPs regardless of
// the requested delay. Not called anywhere in this file (see the commented-out delayMS(1) call
// sites in writeDisplay() below) -- relies instead on digitalWrite()'s own call overhead to
// satisfy MAX6921 minimum pulse-width timing.
void inline delayMS(int d) {
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
}


void setup_pins() {
  DPRINTLN("VFD Clock - setup MAX6921 pins");
  pinMode(PIN_LE,  OUTPUT);  regPin(PIN_LE,"PIN_LE");
  pinMode(PIN_BL,  OUTPUT);  regPin(PIN_BL,"PIN_BL");
  digitalWrite(PIN_BL,LOW);  //brightness
  pinMode(PIN_DATA,OUTPUT);  regPin(PIN_DATA,"PIN_DATA");
  pinMode(PIN_CLK, OUTPUT);  regPin(PIN_CLK,"PIN_CLK");

  // Build a human-readable dump of the pin mapping for the web UI's diagnostics page (not
  // used by the driver logic itself).
  driverSetupStr = "MAX6921 segmentEnablePins:";
  for (int j=0;j<sizeof(segmentEnablePins);j++) {
    driverSetupStr += String(segmentEnablePins[j]) + ",";
  }
  driverSetupStr += "<br>MAX6921 digitEnablePins:";
  for (int j=0;j<sizeof(digitEnablePins);j++) {
    driverSetupStr += String(digitEnablePins[j]) + ",";
  }
  driverSetupStr += String("<br>");

  generateBitTable();     //precompute charTable/digitEnableBits/animationMaskBits once, so the ISR (writeDisplay) never has to do this math
  digitsOnly = false;     //this driver can show letters/symbols too (A,P,C,deg,%,I,F...), not just 0-9
  startTimer();           //arms the ESP32 hardware timer and attaches writeDisplay() as its ISR -- multiplexing runs entirely in the background from here on
}

// Hardware-timer ISR (ESP32). Unlike the ESP8266 timer1 (one-shot, must be manually
// rearmed every call), timerAlarmWrite(..., true) below enables auto-reload, so the timer
// keeps firing periodically on its own; the repeated timerAlarmWrite()/timerAlarmEnable()
// calls at the end of every invocation just update the *next* period's length (for the
// following ON or OFF phase) rather than being strictly required to keep it running.
// Implements brightness via a simple two-phase (ON/OFF) time-slicing scheme rather than true
// PWM: each call toggles `state` between an "ON phase" (tube lit, duration proportional to
// brightness) and an "OFF phase" (tube dark, remaining time of the PWMrefresh period).
void IRAM_ATTR writeDisplay(){  //void IRAM_ATTR  writeDisplay(){
  static DRAM_ATTR int timer = PWMrefresh;
  static DRAM_ATTR uint32_t val;
  static DRAM_ATTR byte pos = 0;
  static DRAM_ATTR boolean state=true;
  static DRAM_ATTR int brightness;
  static DRAM_ATTR int PWMtimeBrightness=PWM_min;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress! (EEPROM writes are blocking/slow, and touching them concurrently with flash-adjacent SPI activity risks corruption)
      //digitalWrite(PIN_BL,HIGH);    //OFF
      // NOTE: unlike MAX6921.ino (ESP8266), BL is NOT forced HIGH here, so the display is left
      // showing whatever frame was last latched (frozen, not blanked) for the duration of the
      // EEPROM write. Also, no timerAlarmWrite()/timerAlarmEnable() call is needed here (unlike
      // the ESP8266 timer1_write() call in the sibling file) because this hardware timer's
      // auto-reload keeps it firing on its own even while this ISR body returns early.
    return;
  }

  // portENTER_CRITICAL_ISR + noInterrupts()/interrupts() together ensure this whole bit-bang
  // sequence (and the shared `val`/`pos`/`state` statics) cannot be interrupted or preempted
  // by the other CPU core or a higher-priority ISR mid-frame, which would otherwise risk
  // shifting out a half-updated/torn frame to the MAX6921.
  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();
  intCounter++;

  brightness = displayON ? prm.dayBright : prm.nightBright;   //day/night dimming level, selected by the core's displayON flag
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  if ((!autoBrightness) && (brightness==MAXBRIGHTNESS)) state = true;  //at max fixed brightness, always stay in the "ON" phase (skip the OFF/dimming phase entirely)

  if (state) {  //ON state: advance to the next tube position and compute what to shift out
    pos++;  if (pos>maxDigits-1)  {   //go to the tube#0
      pos = 0;
      // Brightness (PWM ON time) is only recalculated once per full multiplex sweep (at tube#0),
      // not per-tube, so all tubes share the same duty cycle within one refresh cycle.
      if (autoBrightness && displayON) {   //change brightness only on the tube#0
        PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);   //ambient-light-driven brightness (auto-dimming), scaled by measured lux vs MAXIMUM_LUX
        }
      else
        PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);   //manual brightness setting, scaled 0..MAXBRIGHTNESS into PWM_min..PWM_max ticks
    }

    // MAX6921 has no separate "digit select" vs "segment" concept -- both are just output
    // bits. Combine the bit for this tube's grid/anode-enable output with the bits for the
    // segments that should light up for the current character into one 20-bit frame.
    val = (digitEnableBits[pos] | charTable[digit[pos]]);  //the full bitmap to send to MAX chip
    if (digitDP[pos]) val = val | charTable[12];    //Decimal Point (charTable[12] = the DP-only glyph, OR'd in independently of the digit glyph)

    timer = PWMtimeBrightness;
    //if (pos==2) timer = 3*timer;  //Weak IV11 tube#2 brightness compensation -- see the CLOCK_4-guarded equivalent in MAX6921.ino; no ESP32 clocks.h config currently needs this hack, left disabled here
    timerON = timer;                              //exposed for debug/diagnostics only (see ESP32_UniClock2.ino)
    timerOFF = PWMrefresh-PWMtimeBrightness;
  }
  else {  //OFF state: nothing to shift out this call, just wait out the rest of the duty cycle
    timer = PWMrefresh-PWMtimeBrightness;
  }

  if (timer<500) timer = 500;  //safety only... avoid an excessively short/zero timer reload that could starve the CPU with ISR re-entry

  if ( (brightness == 0) || (!state) || (!radarON)) {  //OFF state, blank digit  (radarON gates the whole display off when a presence/radar sensor says nobody is around)
    digitalWrite(PIN_BL,HIGH);    //OFF
  }
  else {  //ON state
    if (animMask[pos]>0) val &= animationMaskBits[animMask[pos]-1];  //animationMode 6, mask characters from up to down and back -- animMask[] holds a 1-based progressive-reveal stage per tube; the precomputed masks blank segments in a..g order to fake a vertical wipe
    // Bit-bang 20 bits out MSB-first (OUT19 first). No explicit delay is inserted between
    // digitalWrite() calls: the GPIO register write + digitalWrite() call overhead already
    // exceeds the MAX6921's minimum DIN setup/CLK pulse-width requirement, so the
    // commented-out nop/delayMS() calls below are intentionally left disabled.
    for (int i=0; i<20; i++)  {
      if (val & uint32_t(1 << (19 - i)))
        {digitalWrite(PIN_DATA, HIGH);   //asm volatile ("nop");
        }
      else
        {digitalWrite(PIN_DATA, LOW);    //asm volatile ("nop");
        }

      digitalWrite(PIN_CLK,HIGH);  //asm volatile ("nop");  //delayMS(1);  //rising edge shifts the current DATA bit into the MAX6921 internal shift register
      digitalWrite(PIN_CLK,LOW);   //asm volatile ("nop"); //delayMS(1);
      } //end for

    // Latch pulse: transfers the 20-bit shift register into the MAX6921's output latch in one
    // shot, so the outputs (and thus the tube) change atomically rather than rippling bit by
    // bit while still shifting (which would show visible garbage/flicker on the tube).
    digitalWrite(PIN_LE,HIGH );  asm volatile ("nop");
    digitalWrite(PIN_LE,LOW);
    digitalWrite(PIN_BL,LOW );   //ON -- unblank now that the new frame is safely latched
  }  //end else

  portEXIT_CRITICAL_ISR(&timerMux);
  state = !state;
  //ESP32timer = timerBegin(0, PRESCALER, true);  //set prescaler, true = edge generated signal
  //timerAttachInterrupt(ESP32timer, &writeDisplay, true);
  timerAlarmWrite(ESP32timer, timer, true);   //schedule the next phase's (ON or OFF) duration; `true` keeps auto-reload enabled
  timerAlarmEnable(ESP32timer);
  interrupts();
}

// One-time setup helper (called from setup_pins()): translates the PCB-specific pin-index
// tables from clocks.h (segmentEnablePins[], digitEnablePins[]) and the abstract a..g/DP
// glyph patterns (charDefinition[]) into ready-to-OR 32-bit MAX6921 output bitmaps. Doing
// this heavy lifting once at boot keeps the writeDisplay() ISR itself lightweight (just a
// couple of array lookups + OR + bit-shift loop).
void generateBitTable() {
uint32_t out;

  DPRINTLN("--- Generating segment pins bitmap ---");
  for (int i=0;i<MAXSEGMENTS;i++) {
    segmentEnableBits[i] = uint32_t(1<<segmentEnablePins[i]);   //one-hot bit for each of the 8 segment output pins (a..g,DP)
    //DPRINT(i); DPRINT(": "); DPRINTLN(segmentEnableBits[i],BIN);
  }
  // Precompute cumulative "wipe" animation masks (animationMode 6 / the "up->down->up" reveal
  // used when a digit changes). Each stage progressively adds more segments in a fixed visual
  // order (a -> b+f -> g -> e+c -> d), i.e. top segment, then the two middle-height segments,
  // then the center bar, then the two lower-middle segments, then the bottom segment --
  // roughly mimicking a wipe sweeping down the 7-segment glyph.
  animationMaskBits[0] = uint32_t(1<<segmentEnablePins[0]);  //a
  animationMaskBits[1] = animationMaskBits[0] | uint32_t(1<<segmentEnablePins[1]) | uint32_t(1<<segmentEnablePins[5]);  //bf
  animationMaskBits[2] = animationMaskBits[1] | uint32_t(1<<segmentEnablePins[6]);  //g
  animationMaskBits[3] = animationMaskBits[2] | uint32_t(1<<segmentEnablePins[4]) | uint32_t(1<<segmentEnablePins[2]);  //ec
  animationMaskBits[4] = animationMaskBits[3] | uint32_t(1<<segmentEnablePins[3]);  //d
  for (int i=0;i<5;i++) {
    animationMaskBits[i] = ~animationMaskBits[i]; //invert bits -- stored inverted so writeDisplay() can apply them with a plain AND (val &= mask) to blank out the not-yet-revealed segments
    //DPRINTLN(animationMaskBits[i],HEX);
  }
  DPRINTLN("--- Generating digit pins bitmap ---");
  for (int i=0;i<maxDigits;i++) {
    digitEnableBits[i] = uint32_t(1 << digitEnablePins[i]);   //one-hot bit for each tube's grid/anode-enable output
    //DPRINT(i); DPRINT(": "); DPRINTLN( digitEnableBits[i],BIN);
  }

DPRINTLN("---- Generated Character / Pins table -----");
  for (int i=0;i<MAXCHARS;i++) {
    out = 0;
    //DPRINT(i); DPRINT(":  ");
    //DPRINT(charDefinition[i],BIN);  //DPRINT(" = ");
    for (int j=0;j<=7;j++)   //from a to g  (j=0..6 are segments a..g, j=7 is DP; bit(7-j) walks charDefinition MSB->LSB to match that ordering)
      if ((charDefinition[i] & 1<<(7-j)) != 0) {
        out = out | segmentEnableBits[j]; //DPRINT("1");
        }
    //else        DPRINT("0");
    //DPRINT("  >> ");

    charTable[i] = out;   //final per-glyph MAX6921 output bitmap, ready to OR with digitEnableBits[pos] in writeDisplay()
    //DPRINTLN(charTable[i],BIN);
  }  //end for
}


void clearTubes() {
  // Only asserts BLANK (forces all MAX6921 outputs off) -- it does NOT clear the shift
  // register/latch contents or the digit[]/newDigit[] buffers. The stale frame stays latched
  // in the chip and will reappear instantly the moment BLANK is deasserted again (e.g. next
  // writeDisplay() ISR tick), so this is a cheap/fast "go dark" rather than a true reset.
  digitalWrite(PIN_BL,HIGH);
}

// Intentionally empty: this driver's entire refresh/multiplex logic runs inside the
// hardware-timer ISR (writeDisplay()), so there is nothing left for a synchronous single-shot
// write to do. The function still needs to exist because the core file (ESP32_UniClock2.ino)
// calls writeDisplaySingle() unconditionally from several places (e.g. after changing a setting).
void writeDisplaySingle() {}
#endif

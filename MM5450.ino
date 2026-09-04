/*==============================================================================
  MM5450.ino - driver for the MM5450 (National Semi / clone) LED segment driver IC.

  Hardware: MM5450 is a 34-output constant-current LED sink driver with a
  simple synchronous serial input (start bit + 34 data bits, latched with a
  LE/latch-enable pulse). It has NO on-chip digit multiplexing and NO
  brightness/intensity register - unlike MAX7219. In this board design a
  single MM5450 is time-shared between TWO groups of 4x 7-segment+DP digits
  (32 segment lines + 2 "side enable" lines = 34 outputs), and brightness is
  produced entirely in software via PWM (see writeDisplay()/PWMrefresh below),
  driven from the periodic hardware-timer ISR started by startTimer().

  Wiring/protocol: DATA/CLK/LE are bit-banged (not hardware SPI) directly via
  ESP8266 GPIO-set/clear peripheral registers for speed, because writeBits()
  runs inside the timer ISR. Bits are shifted out in ascending bitBuffer[]
  index order (index 0 first), each element corresponding 1:1 to one MM5450
  output (index = output pin number as defined by the MM5450 datasheet,
  1..34), with bitBuffer[0] used as the fixed "start bit" required by the
  chip's serial protocol to frame the following 34 data bits.

  Used by (see clocks.h): CLOCK_11 - "8266 LED Clock with MM5450 chip"
  (clocks.h defines MM5450 only inside the #ifdef CLOCK_11 block).
==============================================================================*/
#ifdef MM5450
char tubeDriver[] = "MM5450";
int maxDigits = 6;

//Fill this table with the pin numbers of the MM5450 chip!
// Maps [digit-group 0..3][segment a..g,DP] -> MM5450 output pin number (1..34).
// Digit groups 0,1 belong to display "side" 0, groups 2,3 belong to "side" 1
// (see sideEnablePins[] and the side-switching logic in writeDisplay()).
byte segmentEnablePins[4][8] =  {
  {16, 15, 14, 13, 12, 11, 10, 9}, //segment enable pins of MM5450 (a,b,c,d,e,f,g,DP)
  {8, 7, 6, 5, 4, 3, 2, 1},
  {17, 18, 19, 20, 21, 22, 23, 24},
  {25, 26, 27, 28, 29, 30, 31, 32}
};


/* Old version - kept for reference only, NOT compiled (different pin-to-output mapping,
   presumably from an earlier PCB revision with different MM5450 output wiring).
  byte segmentEnablePins[4][8] =  {
                 {25,26,27,28,29,30,31,32},   //segment enable pins of MM5450 (a,b,c,d,e,f,g,DP)
                 {17,18,19,20,21,22,23,24},
                 {9,10,11,12,13,14,15,16},
                 {1,2,3,4,5,6,7,8}
                 };
*/


byte sideEnablePins[] = {34, 33}; //enable pins for the two groups of 4 digits (2x4 LEDs)
// sideEnablePins[0] = MM5450 output 34 = enable line for "side 0" (digit groups 0,1)
// sideEnablePins[1] = MM5450 output 33 = enable line for "side 1" (digit groups 2,3)
// Only one side's enable line is driven HIGH at a time -> the two 4-digit
// groups share the same 32 segment lines and are multiplexed in time.

//MM5450 control pins (GPIO numbers - see clocks.h D5pin/D6pin/etc for the NodeMCU Dx aliases:
// GPIO14=D5, GPIO12=D6, GPIO13=D7, GPIO15=D8)
#define PIN_LE    14  // D5 Shift Register Latch Enable
#define PIN_CLK   12  // D6 Shift Register Clock
#define PIN_DATA  13  // D7 Shift Register Data
#define PIN_BR    15  // D8 Shift Register Brightness (chip-enable/blank output, active HIGH)

// Bitmasks for the same pins, used with WRITE_PERI_REG(PIN_OUT_SET/CLEAR, ...) for fast
// direct GPIO register access inside the ISR (digitalWrite() is too slow/has too much
// call overhead to meet the tight bit-banging timing below).
#define PIN_LE_BIT   1<<14  // D5 Shift Register Latch Enable
#define PIN_CLK_BIT  1<<12  // D6 Shift Register Clock
#define PIN_DATA_BIT 1<<13  // D7 Shift Register Data
#define PIN_BR_BIT   1<<15  // D8 Shift Register Brightness

//------------------abcdefgDP----------------   definition of different characters
byte charDefinition[] = {
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
  B00000000,   // : BLANK   (10)
  B00000010,   //-: minus (11)
  B00000001,   // decimal point (12)
  B11101110,   // A  abcefg  (13)
  B11001110,   // P  abefg (14)
  B10011100,   // C  adef (15)
  B11000110,   //grad (upper circle) abfg  (16)
  B10110100,   //%  acdf  (17)
  B00111010,   //lower circle cdeg  (18)
  B01100000,   //I  bc    (19)
  B10001110    //F  aefg  (20)
};

#define MAXCHARS sizeof(charDefinition)  // (currently unused in this file - kept for reference/consistency with other drivers)

int PWMrefresh = 10000; //usec, Multiplex time period (one full ON+OFF brightness cycle). Greater value => slower multiplex/PWM frequency
// PWM_min/PWM_max bound the ON-time portion of PWMrefresh: since the MM5450 has no
// hardware intensity register, brightness is entirely software PWM - writeDisplay()
// re-arms the hw timer with a shorter or longer "ON" interval depending on the
// requested brightness (see PWMtimeBrightness below).
int PWM_min = 500;
int PWM_max = 10000;
//int PWMtiming[11] = {0,500,800,1200,2000,2500,3000,4500,6000,8000,10000};

// bitBuffer holds the full 34-bit MM5450 serial frame plus its leading start bit
// (bitBuffer[0] = start bit, bitBuffer[1..34] = one byte per output pin, 0/1 = LED off/on).
// One array element is used per bit (not packed) to make indexing by MM5450 pin number
// trivial for writeBits(); size 36 leaves a little headroom beyond the 35 bits actually sent.
byte bitBuffer[36];

void setup_pins() {
#if defined(ESP8266)
#else
#error "Only 8266 Board is supported!"
#endif

  DPRINTLN("MM5450 - Setup pins...");
  pinMode(PIN_LE,  OUTPUT);  regPin(PIN_LE, "PIN_LE");
  pinMode(PIN_BR,  OUTPUT);  regPin(PIN_BR, "PIN_BR");
  pinMode(PIN_DATA, OUTPUT);  regPin(PIN_DATA, "PIN_DATA");
  pinMode(PIN_CLK, OUTPUT);  regPin(PIN_CLK, "PIN_CLK");
  digitalWrite(PIN_BR, HIGH); //brightness

  digitsOnly = false;  // this driver's charDefinition[] also supports letters (A,P,C,deg,%,I,F) besides 0-9
  startTimer();  // arms the periodic hw timer -> writeDisplay() becomes the multiplex/PWM ISR

}

// Bit-bangs one full 35-bit frame (start bit + 34 output bits) into the MM5450 shift
// register and latches it. Runs inside the timer ISR (called from writeDisplay()), so it
// uses direct GPIO SET/CLEAR peripheral-register writes instead of digitalWrite() - the
// latter's call/pin-lookup overhead would blow the tight clock pulse widths needed here.
void ICACHE_RAM_ATTR writeBits() {
  //https://sub.nanona.fi/esp8266/timing-and-ticks.html
  //One tick is 1us / 80 = 0.0125us = 12.5ns on 80MHz
  //One tick is 1us / 80 = 0.0125us = 6.25ns on 160MHz
  // 1 NOP is 1 tick

  //noInterrupts();  // not needed: this code already runs inside the timer ISR itself
  WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_LE_BIT );   //DATA_ENABLE - LE must be LOW while shifting data in
  for (int t = 0; t < 5; t++) asm volatile ("nop"); //100nsec

  for (int i = 0; i < 36; i++)  {  // shift bitBuffer out LSB-index-first (index 0 = start bit, sent first)
    if (bitBuffer[i]) {
      WRITE_PERI_REG( PIN_OUT_SET, PIN_DATA_BIT );  //digitalWrite(PIN_DATA,HIGH);
    }
    else {
      WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_DATA_BIT ); //digitalWrite(PIN_DATA,LOW);
    }

    for (int t = 0; t < 12; t++) asm volatile ("nop"); //300nsec - data setup time before clock edge

    WRITE_PERI_REG( PIN_OUT_SET, PIN_CLK_BIT );  // MM5450 samples DATA on this rising CLK edge
    for (int t = 0; t < 12; t++) asm volatile ("nop"); //300nsec
    WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_CLK_BIT );
    for (int t = 0; t < 12; t++) asm volatile ("nop"); //300nsec
  } //end for

  WRITE_PERI_REG( PIN_OUT_SET, PIN_LE_BIT );  // rising edge on LE latches the shifted-in frame to the outputs
  //interrupts();
}

// Main multiplex/PWM ISR, attached to the ESP8266 hw timer1 by startTimer().
// timer1 is configured TIM_SINGLE (one-shot), so this handler must re-arm itself via
// timer1_write() at the very end of every call - the display refresh rate/PWM duty
// cycle is entirely driven by how long each re-armed interval is (see 'timer' below).
// Each call does ONE of: (a) blank the display for the computed OFF duration (brightness
// PWM low phase), or (b) blank for the ON duration but first shift out a fresh frame for
// whichever "side" (group of 4 digits) is up next, alternating sides on every ON call.
void ICACHE_RAM_ATTR writeDisplay() {       // Writes to the MM5450 driver for LEDS
  static volatile int timer = PWMrefresh;
  static volatile boolean state = true;  // true = ON phase of the PWM cycle, false = OFF phase
  static volatile int brightness;
  static int PWMtimeBrightness;
  static volatile uint32_t val;
  static volatile byte pos = 0;
  static byte side = 0;
  byte num = 0;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    // Flash writes on ESP8266 briefly disable interrupts/stall the CPU; touching the
    // display bus during that window could corrupt the SPI-like bit-bang timing, so we
    // just keep re-arming the timer with a fixed period and skip the actual refresh.
    timer1_write(PWMrefresh);
    return;
  }

  intCounter++;
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness > MAXBRIGHTNESS) brightness = MAXBRIGHTNESS; //only for safety

  if (autoBrightness && displayON) {
    // Ambient-light (lx) driven brightness: brighter room -> longer ON time within PWMrefresh
    PWMtimeBrightness = max(PWM_min, PWM_max * lx / MAXIMUM_LUX);
  }
  else
    // Manual brightness setting (0..MAXBRIGHTNESS) mapped linearly onto the ON-time window
    PWMtimeBrightness = max(PWM_min, PWM_max * brightness / MAXBRIGHTNESS);

  if ((!autoBrightness) && (brightness == MAXBRIGHTNESS))
    // At max manual brightness PWMtimeBrightness == PWMrefresh, so the OFF phase would be
    // ~0us; forcing state=true here keeps the ISR permanently in the ON branch instead of
    // wastefully toggling into a near-zero-length OFF phase every other call.
    state = true;

  if (state) {  //ON state
    timer = PWMtimeBrightness;
    timerON = timer;
    timerOFF = PWMrefresh - PWMtimeBrightness;
  }
  else {  //OFF state
    timer = PWMrefresh - PWMtimeBrightness;
  }
  if (timer < 500) timer = 500; //safety only... (avoid re-arming the hw timer with too short/zero an interval)

  if ( (brightness == 0) || (!state) || (!radarON)) {  //OFF state, blank digit
    WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_BR_BIT );    //OFF - BR pin blanks all MM5450 outputs regardless of shift-register content
    // radarON reflects presence-detection (radar/PIR): when nobody is around the display
    // is blanked here (power saving / tube-life) without touching the multiplex state.
  }
  else {  //ON state
    WRITE_PERI_REG( PIN_OUT_SET, PIN_BR_BIT );   //ON

    memset(bitBuffer, 0, sizeof(bitBuffer)); //clear array
    bitBuffer[0] = 1;   //starting bit required to frame the 34 data bits (see MM5450 datasheet)

    bitBuffer[sideEnablePins[side]] = 0;
    writeBits();  // 1st transfer: push an all-blank frame (except start bit) so BOTH sides are
                   // momentarily off before the new side's segments are shifted in - avoids
                   // ghosting/cross-talk between the outgoing and incoming digit group.
    for (int t = 0; t < 100; t++) asm volatile ("nop"); //clear display and wait between switching the two sides
    bitBuffer[sideEnablePins[side]] = 0;   //disable old side..
    // NOTE: bitBuffer was just memset to 0 a few lines above and this index was already
    // cleared before the previous writeBits() call, so this re-assignment is a redundant
    // no-op (POSSIBLE dead/duplicated code - likely meant to clear something else, e.g.
    // the *other* side, but 'side' has not been toggled yet at this point).

    if (side == 0) side = 1; else side = 0;   //change side...

    for (int i = 0; i < 4; i++) {
      pos = 7 - (4 * side + i);
      // POSSIBLE BUG: pos always ranges 0..7 (8 digit positions) regardless of maxDigits
      // (=6 above). digit[6] and digit[7] are never written by the core display-update
      // logic when maxDigits==6, and being plain (non-initialized-to-BLANK) globals they
      // default to 0 -> segment pattern for '0'. If the physical PCB for CLOCK_11 really
      // only has 6 digits wired, positions 6/7 are presumably just unused MM5450 outputs
      // (harmless); if it has 8 digits, the top 2 would never be updated by the clock logic.
      num = digit[pos];
      //if (animMask[pos]>0) val &= animationMaskBits[animMask[pos]-1];  //animationMode 6, mask characters from up to down and back
      // (wipe/mask animation support - referenced 'val' and 'animationMaskBits' are unused/
      // undefined in this file, so this feature is not implemented for the MM5450 driver)
      for (int j = 0; j <= 7; j++) //from a to g
        if ((charDefinition[num] & 1 << (7 - j)) != 0) {
          bitBuffer[segmentEnablePins[i][j]] = 1;
        }
      if (digitDP[pos]) bitBuffer[segmentEnablePins[i][7]] = 1;   //Decimal Point
    }  //end for

    bitBuffer[sideEnablePins[side]] = 1;   //enable new side
    writeBits();  // 2nd transfer: actual segment data + new side's enable bit
  }  //end else ON

  state = !state;
  timer1_write(timer);  // re-arm the one-shot hw timer for the next ON or OFF phase
}


// Both required by the common driver interface but intentionally no-ops here: this driver
// does all display updates from the writeDisplay() ISR (brightness PWM + side multiplex
// depend on continuous timer-driven refresh), so there is nothing useful for a one-shot
// blank (clearTubes) or a one-shot blocking full-frame write (writeDisplaySingle) to do -
// the next ISR tick will simply pick up the current digit[]/digitDP[] contents.
void clearTubes() {}
void writeDisplaySingle() {}
#endif

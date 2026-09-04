#ifdef SN75512
//=============================================================================================
// Driver for VFD (Vacuum Fluorescent Display) tubes driven by 2x cascaded Texas Instruments
// SN75512 serial-in / high-voltage parallel-out shift-register + segment/grid driver ICs.
//
// Wiring / protocol:
//   - DATA/CLK: plain serial shift-register bit-banging, MSB first (see writeDisplay()).
//   - The two SN75512 chips are daisy-chained (12 outputs each) into one 24-bit (DATABITS)
//     shift register: shift all 24 bits in, then pulse LE (Latch Enable) once to transfer the
//     shifted-in bits to the chips' parallel high-voltage outputs (glitch-free update - the
//     outputs only change on the LE pulse, not while shifting).
//   - STROBE is the chip's display blank/enable input (per the pin comment below: 1=display
//     OFF, 0=display ON) and is toggled once per digit refresh to implement brightness dimming
//     (see the brightCounter[] dithering trick in writeDisplay()).
//   - Platform: ESP8266 only - uses direct WRITE_PERI_REG() GPIO register access instead of
//     digitalWrite() because bit-banging 24 bits per digit, at the multiplex refresh rate,
//     needs to be much faster than the Arduino digitalWrite() call overhead allows.
//
// NOTE: no CLOCK_NN block currently active in clocks.h defines SN75512 (grep of clocks.h for
// "SN75512" / "#define SN75512" comes up empty) - it only appears as a commented-out option in
// the driver-selection list in ESP32_UniClock2.ino. This driver therefore currently looks like
// a legacy/orphan module not wired to any active clock configuration.
//=============================================================================================
// 2 x SN75512
char tubeDriver[] = "SN75512";
#define VFDrefresh 1200    //msec, Multiplex time period. Greater value => slower multiplex frequency
                            //NOTE: comment says "msec" but this value is passed straight to timer1_write() (ESP8266
                            //hw timer1 in TIM_DIV16/TIM_EDGE/TIM_SINGLE mode, i.e. ~5MHz tick rate => ~0.2us/tick,
                            //see startTimer() in ESP32_UniClock2.ino) - so the real unit is timer ticks (~0.2us
                            //each), not milliseconds. The label is misleading but functionally harmless.

#define DATABITS 24        //total length of the shift registers (2 x 12-bit SN75512, daisy-chained)

//Fill this table with the bit positions of the shift registers!
//Bit position = index (0-based) of that output within the 24-bit combined (2-chip) shift
//register, in the order the bits are shifted out (see writeDisplay(): bit DATABITS-1 goes
//out first). segmentEnablePins order must match the a,b,c,d,e,f,g,DP order of charDefinition[].
byte segmentEnablePins[] =  {16,17,18,19,20,21,22,23};  //segment enable bits (a,b,c,d,e,f,g,DP)  (You MUST define always 8 bits!!!)
byte digitEnablePins[] = {0,1,2,3,4,5,6,7,8};           //digit enable bits   (You may define any number of digits between 1 and 10 )

//SN75512 control pins
#define PIN_LE      13  // D7 Shift Register Latch Enable
#define PIN_CLK     12  // D6 Shift Register Clock
#define PIN_DATA    14  // D5 Shift Register Data
#define PIN_STROBE  5   // D1 Shift Register Strobe (1=display off     0=display on)

//Precomputed GPIO bitmasks (1<<pin) for use with the low-level ESP8266 WRITE_PERI_REG()/
//PIN_OUT_SET/PIN_OUT_CLEAR register access in writeDisplay() - much faster than digitalWrite(),
//needed because the whole 24-bit frame must be bit-banged out within one multiplex refresh slot.
#define PIN_LE_BIT     1<<PIN_LE
#define PIN_CLK_BIT    1<<PIN_CLK
#define PIN_DATA_BIT   1<<PIN_DATA
#define PIN_STROBE_BIT 1<<PIN_STROBE



//------------------abcdefgDP----------------   definition of different characters
//Each byte is a 7-segment+DP mask, MSB=a ... LSB=DP (bit7=a, bit6=b, ... bit1=g, bit0=DP).
//generateBitTable() below expands these into the actual 24-bit shift-register bitmaps
//(charTable[]) using segmentEnablePins[] as the a..DP -> shift-register-bit-position map.
//Indices 0-9 = digits, 10=BLANK, 11=minus, 12=decimal point, 13-20 = extra glyphs
//(A,P,C,deg,%,lower-circle,I,F) - these indices must line up with the digit[]/newDigit[]
//buffer value convention documented in the core .ino file.
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

#define MAXCHARS sizeof(charDefinition)
#define MAXSEGMENTS sizeof(segmentEnablePins)
int maxDigits =  sizeof(digitEnablePins);

uint32_t charTable[MAXCHARS];              //generated pin table from segmentDefinitions
uint32_t segmentEnableBits[MAXSEGMENTS];   //bitmaps, generated from EnablePins tables
uint32_t digitEnableBits[10];
//-----------------------------------------------------------------------------------------

void ICACHE_RAM_ATTR writeDisplay(){
static volatile uint32_t val = 0;
static volatile byte pos = 0;
//Per-digit dithering/duty-cycle phase counters, one per tube position (up to 10 supported here).
//Each entry free-runs 1..MAXBRIGHTNESS and is compared against the target brightness to decide
//ON/OFF for that digit's current refresh slot (see below) - a cheap "N-cycle duty modulation"
//dimmer rather than true PWM within one frame. The staggered starting phases {0,9,2,8,4,7,6,5,3,1}
//(instead of all starting at 0) spread each digit's ON pulses across different points in the
//brightness cycle, which avoids all digits blinking on/off in visible lock-step and reduces
//perceived flicker/beating between adjacent tubes.
static volatile int brightCounter[] = {0,9,2,8,4,7,6,5,3,1};

//https://sub.nanona.fi/esp8266/timing-and-ticks.html
//One tick is 1us / 80 = 0.0125us = 12.5ns on 80MHz
//One tick is 1us / 80 = 0.0125us = 6.25ns on 160MHz
// 1 NOP is 1 tick

  //noInterrupts();


  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    timer1_write(VFDrefresh);  //TIM_SINGLE (one-shot) mode requires re-arming the timer every call,
    return;                    //even when skipping the refresh, or the ISR would never fire again.
  }

  //Brightness gating for THIS digit position (pos), applied BEFORE this position's segment data
  //is shifted out below. STROBE is the chip's global blank input (LOW=display on, HIGH=off, per
  //the pin comment above), so this briefly leaves the PREVIOUS digit's already-latched pattern
  //on screen at the new brightness-derived on/off state until the new data is latched via LE
  //further down. With a ~1200-tick (see VFDrefresh) refresh period this overlap is a small
  //fraction of the slot and isn't visually significant.
  if (brightCounter[pos] % MAXBRIGHTNESS < (displayON ?  prm.dayBright : prm.nightBright))
    WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_STROBE_BIT );   //ON
  else
    WRITE_PERI_REG( PIN_OUT_SET, PIN_STROBE_BIT );    //OFF

  for (int t=0; t<4;t++) asm volatile ("nop");

  brightCounter[pos]++;
  if (brightCounter[pos]>MAXBRIGHTNESS) brightCounter[pos] = 1;

  //NOTE: digit[pos] indexes charTable[] (built from charDefinition[], MAXCHARS=21 entries)
  //directly with no bounds check. If digit[pos] were ever >=21 this would read out-of-bounds
  //memory. In practice the rest of the firmware is expected to only ever write valid glyph
  //codes (0-20) into digit[], so this is a latent risk rather than an observed failure.
  val = (digitEnableBits[pos] | charTable[digit[pos]]);  //the full bitmap to send to MAX chip
  if (digitDP[pos]) val = val | charTable[12];    //Decimal Point
  //Shift out all 24 bits, MSB first (bit DATABITS-1 down to bit 0), matching the bit-position
  //convention used by segmentEnablePins[]/digitEnablePins[] (index = bit position, MSB shifted
  //first so it ends up in the "first" chip in the daisy-chain after the final LE latch).
  //Note the asymmetric NOP padding: a '1' bit only pads 4 NOPs before raising CLK, while a '0'
  //bit pads 8 - this compensates for the extra WRITE_PERI_REG() call needed to explicitly clear
  //the DATA line (vs. just letting it stay set) so that DATA has equal setup time before CLK
  //rises in both cases; hand-tuned bit-bang timing, not an accidental asymmetry.
  for (int i=0; i<DATABITS; i++)  {
    if ((val & (uint32_t)1 << (DATABITS -1 - i)) ) {
      WRITE_PERI_REG( PIN_OUT_SET, PIN_DATA_BIT );
          for (int t=0; t<4;t++) asm volatile ("nop");
    }
    else {
      WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_DATA_BIT );
          for (int t=0; t<8;t++) asm volatile ("nop");
    }
    WRITE_PERI_REG( PIN_OUT_SET, PIN_CLK_BIT );
    for (int t=0; t<8;t++) asm volatile ("nop");
    WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_CLK_BIT );
    for (int t=0; t<4;t++) asm volatile ("nop");
    } //end for

  //Latch: pulse LE high then low to transfer the just-shifted 24 bits from the SN75512's
  //internal shift register into its output latch - this is what actually updates the VFD
  //outputs; the outputs stay unchanged (no visible glitching) while bits are still shifting.
  WRITE_PERI_REG( PIN_OUT_SET, PIN_LE_BIT );
  for (int t=0; t<4;t++) asm volatile ("nop");
  WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_LE_BIT );

  pos++; if (pos >= maxDigits) pos = 0;

  timer1_write(VFDrefresh);  //re-arm the one-shot timer1 for the next digit's refresh
  //interrupts();
}

//One-time (setup) precomputation: turns the human-editable pin-index tables
//(segmentEnablePins[]/digitEnablePins[]) and the per-glyph segment masks (charDefinition[])
//into ready-to-OR 24-bit shift-register bitmaps, so writeDisplay() never has to do this bit
//translation work inside the timing-critical ISR - it just OR's digitEnableBits[pos] |
//charTable[digit[pos]] together.
void generateBitTable() {
uint32_t out;

  DPRINTLN("--- Generating segment pins bitmap ---");
  for (int i=0;i<MAXSEGMENTS;i++) {
    segmentEnableBits[i] = (uint32_t)1<<segmentEnablePins[i];
    //DPRINT(i); DPRINT(": "); DPRINTLN(segmentEnableBits[i],BIN);
  }

  DPRINTLN("--- Generating digit pins bitmap ---");
  for (int i=0;i<maxDigits;i++) {
    digitEnableBits[i] = (uint32_t)1 << digitEnablePins[i];
    //DPRINT(i); DPRINT(": "); DPRINTLN( digitEnableBits[i],BIN);
  }

DPRINTLN("---- Generated Character / Pins table -----");
  for (int i=0;i<MAXCHARS;i++) {
    out = 0;
    //DPRINT(i); DPRINT(":  ");
    //DPRINT(charDefinition[i],BIN);  //DPRINT(" = ");
    //j runs 0..7 through charDefinition's bits from MSB(a, bit7) to LSB(DP, bit0); since
    //segmentEnablePins[] is stored in the same a..DP order, j directly indexes it (no reversal
    //needed here, unlike the SN75518 driver which stores segments in the opposite bit order).
    for (int j=0;j<=7;j++)   //from a to g
      if ((charDefinition[i] & (uint32_t)1<<(7-j)) != 0) {
        out = out | segmentEnableBits[j]; //DPRINT("1");
        }
    //else        DPRINT("0");
    //DPRINT("  >> ");

    charTable[i] = out;
    //DPRINTLN(charTable[i],BIN);
  }  //end for
}


void setup_pins() {
#if defined(ESP8266)
#else
  //Enforced at compile time because writeDisplay() bit-bangs via ESP8266-only
  //WRITE_PERI_REG()/PIN_OUT_SET/PIN_OUT_CLEAR register macros and relies on the ESP8266
  //timer1 API (timer1_write, TIM_SINGLE) armed in startTimer() - none of that exists on ESP32.
  #error "Only 8266 Board is supported!"
#endif

  DPRINTLN("Clock with SN75512 - Setup pins...");
  pinMode(PIN_LE,  OUTPUT);   regPin(PIN_LE,"PIN_LE");
  pinMode(PIN_STROBE,OUTPUT); regPin(PIN_STROBE,"PIN_STROBE");
  pinMode(PIN_DATA,OUTPUT);   regPin(PIN_DATA,"PIN_DATA");
  pinMode(PIN_CLK, OUTPUT);   regPin(PIN_CLK,"PIN_CLK");
  digitalWrite(PIN_STROBE,LOW);  //brightness

  generateBitTable();  //must run before the timer/ISR starts, since writeDisplay() reads charTable[]/digitEnableBits[]/segmentEnableBits[]
  digitsOnly = false;

  startTimer();
}

//POSSIBLE ISSUE: both clearTubes() and writeDisplaySingle() are empty stubs - they satisfy the
//common driver interface (see other driver files / ESP32_UniClock2.ino) syntactically, but do
//not actually blank the display or perform a blocking one-shot write. Note that the core .ino no
//longer calls clearTubes() from anywhere at all (its former caller, disableDisplay(), has been
//removed entirely as dead code), so in practice this is moot for now regardless. Given this driver
//is not wired to any active CLOCK_NN (see file header), this looks like an incomplete/legacy
//stub rather than something exercised by current configurations - flagged for confirmation.
void clearTubes() {}
void writeDisplaySingle() {}
#endif

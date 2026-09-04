#ifdef PT6355
//===========================================================================================
// Driver for the Princeton Technology PT6355 VFD controller/driver IC.
//
// Hardware / protocol:
//   Like the PT6311 (see PT6311.ino), the PT6355 is a "smart" VFD grid/segment controller
//   with its own display RAM and internal multiplex/scan generator -- the MCU only pushes a
//   full frame after something changes, the chip keeps scanning the tubes on its own. That is
//   why writeDisplay() (the timer-ISR multiplex hook used by "dumb" shift-register drivers)
//   is an empty stub here and setup_pins() never calls startTimer(): all display updates go
//   through the blocking writeDisplaySingle() instead of an ISR-driven refresh loop.
//   3-wire serial bus (similar to SPI, chip-specific framing):
//     PIN_CS  - LOW for the duration of one whole command or one whole per-frame data burst
//               (see startBits()/stopBits()), HIGH otherwise. Unlike the PT6311 driver, this
//               chip's frame is NOT re-strobed byte-by-byte: PIN_CS stays low across the
//               entire multi-digit data burst in writeDisplaySingle().
//     PIN_CLK - one pulse per bit.
//     PIN_SIN - serial data, LSB-first per byte (see shift(): loop starts at bit 0).
//   Note this file structurally mirrors PT6311.ino very closely (same DISPLAY_MODE-less
//   brightness idea, same 8-segment/16-segment charDefinition tables, same ASCIImode /
//   asciiConvert[] translation) even though the two chips are different parts with different
//   command sets and bus framing -- treat any future edits to the shared-looking tables in
//   one file as NOT automatically applicable to the other without checking its own datasheet.
//
// Used by (see clocks.h, search for "#define PT6355"):
//   CLOCK_12 - ESP8266 VFD clock with PT6355 chip
//===========================================================================================
char tubeDriver[] = "PT6355";
//int maxDigits = 6;
int wt = 5;   //Serial timing

//#define PIN_CS 22 //12 or 15     // D6 CS_
//#define PIN_CLK  16   // D7 Clock
//#define PIN_SIN  21   // D5 DataIN

//#define SEGMENT8
//#define TESTMODE   //for testing the segments of a new type VFD module

//PT6355 Commands
#define C1_DISPLAY_STATE_SETTING   B11000000  // Display duty setting=15 + Display ON
#define C3_PORT_DATA_SETTINGS      B10000000  //Port selection + output data

#ifdef SEGMENT8
//------------------abcdefgDP----------------   definition of different characters
// C0 selects "10 grids, <=16 segments/grid" data-write mode; C2 sets which physical output
// pin the grid (digit-select) scan starts from (D17..D7 range) -- both are chip-mode setup
// commands, sent once per full frame in writeDisplaySingle(), not per digit.
#define C0_DISPLAY_DATA_SETTING    B11101100  //grid=10, segments<=16
#define C2_GRID_SELECTION          B10101000  //Grid start pin setting D17..D7
// 8-bit segment mask per glyph, MSB-first as a,b,c,d,e,f,g,DP (bit7=a .. bit0=DP). Indices
// 0-9 = digits, 10 = blank, 11 = minus, 12 = decimal point, 13+ = extra glyphs (A,P,C,deg,%,
// lower-circle,I,F) -- matches the digit[]/newDigit[] encoding used by the core firmware, so
// digit[] values index straight into this table (see also the identical table in PT6311.ino).
#define MAXSEGMENTS 8
boolean ASCIImode = false;
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

//byte segmentEnablePins[] =  {9,8,5,3,4,7,6,2};   //segment enable bits (a,b,c,d,e,f,g,DP)   (You MUST define always 8 bits!!!)

#else //16 segment + DP  ---------------------------------------------------------------------------------------------
#define C0_DISPLAY_DATA_SETTING    B11101111  //grid=10, segments>17
#define C2_GRID_SELECTION          B10101000  //Grid start pin setting D17..D7
#define MAXSEGMENTS 17
boolean ASCIImode = true;
// Full 16-segment-star + DP font, indexed by ASCII code - 32 (index 0 = space/0x20). digit[]
// values are translated through asciiConvert[] first (see below) since they are not ASCII.
uint32_t charDefinition[96] = {
  0b00000000000000000, /* (space) */ 
  0b10000000000001100, /* ! */   
  0b00000001000000100, /* " */   //   100000001
  0b01010101000111100, /* # */
  0b01010101010111011, /* $ */
  0b01110111010011001, /* % */
  0b01001001101110001, /* & */
  0b00000001000000000, /* ' */
  0b00001010000000000, /* ( */
  0b00100000100000000, /* ) */
  0b01111111100000000, /* * */
  0b01010101000000000, /* + */
  0b00100000000000000, /* , */
  0b01000100000000000, /* - */
  0b10000000000000000, /* . */
  0b00100010000000000, /* / */
  0b00100010011111111, /* 0 */
  0b00000010000001100, /* 1 */
  0b01000100001110111, /* 2 */
  0b00000100000111111, /* 3 */
  0b01000100010001100, /* 4 */
  0b01001000010110011, /* 5 */
  0b01000100011111011, /* 6 */
  0b00000000000001111, /* 7 */
  0b01000100011111111, /* 8 */
  0b01000100010111111, /* 9 */
  0b00010001000000000, /* : */
  0b00100001000000000, /* ; */
  0b01001010000000000, /* < */
  0b01000100000110000, /* = */
  0b00100100100000000, /* > */
  0b10010100000000111, /* ? */
  0b00000101011110111, /* @ */
  0b01000100011001111, /* A */
  0b00010101000111111, /* B */
  0b00000000011110011, /* C */
  0b00010001000111111, /* D */
  0b01000000011110011, /* E */
  0b01000000011000011, /* F */
  0b00000100011111011, /* G */
  0b01000100011001100, /* H */
  0b00010001000110011, /* I */
  0b00000000001111100, /* J */
  0b01001010011000000, /* K */
  0b00000000011110000, /* L */
  0b00000010111001100, /* M */
  0b00001000111001100, /* N */
  0b00000000011111111, /* O */
  0b01000100011000111, /* P */
  0b00001000011111111, /* Q */
  0b01001100011000111, /* R */
  0b01000100010111011, /* S */
  0b00010001000000011, /* T */
  0b00000000011111100, /* U */
  0b00100010011000000, /* V */
  0b00101000011001100, /* W */
  0b00101010100000000, /* X */
  0b01000100010111100, /* Y */
  0b00100010000110011, /* Z */
  0b00010001000010010, /* [ */
  0b00001000100000000, /* \ */
  0b00010001000100001, /* ] */
  0b00101000000000000, /* ^ */
  0b00000000000110000, /* _ */
  0b00000000100000000, /* ` */
  0b01010000001110000, /* a */
  0b01010000011100000, /* b */
  0b01000000001100000, /* c */
  0b00010100000011100, /* d */
  0b01100000001100000, /* e */
  0b01010101000000010, /* f */
  0b01010001010100001, /* g */
  0b01010000011000000, /* h */
  0b00010000000000000, /* i */
  0b00010001001100000, /* j */
  0b00011011000000000, /* k */
  0b00000000011000000, /* l */
  0b01010100001001000, /* m */
  0b01010000001000000, /* n */
  0b01010000001100000, /* o */
  0b01000001011000001, /* p */
  0b01010001010000001, /* q */
  0b01000000001000000, /* r */
  0b01010000010100001, /* s */
  0b01000000011100000, /* t */
  0b00010000001100000, /* u */
  0b00100000001000000, /* v */
  0b00101000001001000, /* w */
  0b00101010100000000, /* x */
  0b00000101000011100, /* y */
  0b01100000000100000, /* z */
  0b01010001000010010, /* { */
  0b00010001000000000, /* | */
  0b00010101000100001, /* } */
  0b01100110000000000, /* ~ */
  0b00000000000000000, /* (del) */
};

  //byte segmentEnablePins[] =  {16,7,0,1,2,3,4,5,6,8,9,11,10,12,13,14,15};    //segment enable bits 
#endif


// Maps the special glyph codes (BLANK, minus, DP, A,P,C,deg,%,circle,I,F -- digit[] values
// 10..20) onto printable ASCII characters, used only when ASCIImode is true (16-segment
// branch), positionally 1:1 with those glyph codes (asciiConvert[10]==' '==BLANK, etc.) --
// see the fuller explanation of this same table in PT6311.ino.
char asciiConvert[] = "0123456789 -.APC~%oIF";

#define MAXCHARS sizeof(charDefinition)/sizeof(charDefinition[0])

// PIN_CS framing helpers: unlike PT6311 (which re-strobes its STB pin every single byte),
// this driver holds PIN_CS low for an entire command or an entire frame -- callers bracket
// one or more shift() calls between startBits()/stopBits() themselves (see sendCommand() and
// writeDisplaySingle() below, where the whole per-frame digit loop sits inside one CS-low
// window).
void inline startBits(byte CS) {
  digitalWrite(CS, LOW);
  delayMicroseconds(wt);
}

void inline stopBits(byte CS) {
  delayMicroseconds(wt);
  digitalWrite(CS, HIGH);
  delayMicroseconds(wt);
}

// Bit-bangs one byte out on SIN/CLK. NOTE: LSB-first (i starts at 0, tests bit 1<<i from the
// low end) -- same bit order as the PT6311 driver, opposite of e.g. the MAX6921 driver.
// SIN/CLK are passed as parameters (not read from #define'd globals like PT6311's shift())
// even though only one SIN/CLK pair is ever used in this file; harmless but slightly redundant.
void inline shift(byte SIN, byte CLK, byte Data) {
  for (int i=0;i<8;i++) {
    digitalWrite(CLK,LOW);
    delayMicroseconds(wt);
    digitalWrite(SIN, Data & (1<<i));
    delayMicroseconds(wt);
    digitalWrite(CLK,HIGH);
    delayMicroseconds(wt);
  }
  delayMicroseconds(wt);
}

void ICACHE_RAM_ATTR sendCommand(byte val){    //for commands C1..C3
  startBits(PIN_CS);
  shift(PIN_SIN, PIN_CLK, val);
  stopBits(PIN_CS);

  //DPRINT(address,HEX); DPRINT(","); DPRINTLN(val,HEX);
}

void setup_pins() {
  DPRINTLN("PT6355 Clock - Setup pins...");
  pinMode(PIN_CS,OUTPUT);     regPin(PIN_CS,"PIN_CS");
  pinMode(PIN_SIN, OUTPUT);   regPin(PIN_SIN,"PIN_SIN");
  pinMode(PIN_CLK, OUTPUT);   regPin(PIN_CLK,"PIN_CLK");
  digitalWrite(PIN_CS, HIGH);
  digitalWrite(PIN_CLK, HIGH);
  // Hardcoded false (i.e. "full ASCII glyph set usable") to match the 16-segment/ASCIImode
  // table currently active above (SEGMENT8 is commented out at the top of this file). As with
  // the analogous line in PT6311.ino, if SEGMENT8 is ever enabled for this driver this value
  // is NOT automatically updated to match -- it would need to become "digitsOnly = false;" or
  // be reconsidered by hand depending on what glyphs the target clock model actually needs.
  digitsOnly = false;
  delay(1);

  sendCommand(C1_DISPLAY_STATE_SETTING);
  sendCommand(C2_GRID_SELECTION);
  #ifdef TESTMODE
    // POSSIBLE BUG (dead code): this first while(true) never returns, so the for-loop and
    // the second while(true) below it can never execute. Presumably the intent was to run the
    // for-loop (fill digit[] with glyph 1) BEFORE looping writeDisplaySingle() forever, i.e.
    // this first while(true) block looks misplaced/leftover from editing. Only affects the
    // TESTMODE debug build, not normal operation.
    while (true) {writeDisplaySingle(); delay(10000);}
    for (int i=0;i<10;i++) digit[i] = 1;   // " char
    while (true) {writeDisplaySingle(); delay(10000);}
  #endif

}

#if defined(ESP32) 
void IRAM_ATTR writeDisplaySingle(){ 
#else 
void ICACHE_RAM_ATTR writeDisplaySingle(){     
#endif
static byte oldBright = 0;
static byte newBright = 0;
byte dispState;
uint32_t bitBuffer;
int dispChar;
static int count = 15;

  #ifdef TESTMODE
    DPRINT("count:"); DPRINTLN(count);
  #endif
  newBright = displayON ?  prm.dayBright : prm.nightBright;
  if (newBright != oldBright) {
    // Coarse 3-level duty mapping (not a per-step 0..10 lookup table like PT6311's
    // DISPLAY_MODE[]): bit0 just turns the display on/off, and bits 1-2 (mask 0b110) are set
    // as one all-or-nothing group once newBright>5. So the effective brightness only has
    // three distinct hardware states (off / low duty / high duty) even though prm.dayBright /
    // prm.nightBright can range 0..10. This may be a deliberate simplification for this
    // chip's duty-setting bits, but it has not been verified against the PT6355 datasheet.
    dispState = C1_DISPLAY_STATE_SETTING;
    if (newBright>0) dispState |= 1;
    if (newBright>5) dispState |= 6;
    sendCommand(dispState);    //Set Brightness
    oldBright = newBright;
  }
  startBits(PIN_CS);
  // Unlike PT6311 (which addresses each digit individually via C3_ADDRESS_SETTING + i*3 and
  // re-strobes CS per digit), this driver sends ONE C0_DISPLAY_DATA_SETTING command and then
  // streams all maxDigits worth of segment data continuously while CS stays low the whole
  // time (stopBits() only happens once, after the loop) -- the chip auto-advances through its
  // internal grid RAM as bytes arrive.
  shift(PIN_SIN, PIN_CLK, C0_DISPLAY_DATA_SETTING);
  for (int i=0;i<maxDigits;i++) {
    dispChar = digit[i];
    if (ASCIImode) {
      // FIXED: same issue PT6311.ino had - "dispChar -= 32" used to run unconditionally, even
      // when the bounds check just below skipped the asciiConvert[] lookup (dispChar >=
      // sizeof(asciiConvert), or dispChar==21 landing on the string's NUL terminator), producing
      // a negative index into charDefinition[] below. Now the subtraction only happens together
      // with a successful lookup; anything unmapped falls back to charDefinition[0] (blank glyph).
      if (dispChar<sizeof(asciiConvert)-1) {  //-1 excludes the string's NUL terminator
        dispChar = asciiConvert[dispChar] - 32;
      } else {
        dispChar = 0;  //unmapped/out-of-range code -> blank glyph (charDefinition[0])
      }
    }
    bitBuffer = 0;
    for (int j=0;j<MAXSEGMENTS;j++)
      if ((charDefinition[dispChar] & (uint32_t)1<<(MAXSEGMENTS-1-j)) != 0) {
        bitBuffer |= (uint32_t)1<<segmentEnablePins[j];
        }
    // POSSIBLE BUG / disabled feature: decimal-point output is commented out here, unlike the
    // equivalent line in PT6311.ino ("if (digitDP[i]) bitBuffer |= ...segmentEnablePins[7]").
    // As written, digitDP[] is completely ignored on this driver -- decimal points/colon dots
    // set by the core firmware will never actually light up on PT6355-based clocks. This may
    // be intentional (e.g. this clock's wiring has no DP output driven through segmentEnablePins[7]),
    // but it is worth confirming since it silently drops a feature other drivers implement.
    //if (digitDP[i]) bitBuffer |= (uint32_t)1<<segmentEnablePins[7];   //Decimal Point
    #ifdef TESTMODE
      bitBuffer = (uint32_t)1<<count;  //walk a single lit segment/grid bit through the frame to identify wiring
    #endif
    //DPRINT(charDefinition[dispChar],BIN); DPRINT(" / "); DPRINTLN(bitBuffer,BIN);
    //bitBuffer = 0x03FFFF;
    shift(PIN_SIN, PIN_CLK, bitBuffer & 0xFF);
    shift(PIN_SIN, PIN_CLK, bitBuffer >>8);
    if (MAXSEGMENTS>16) shift(PIN_SIN, PIN_CLK, bitBuffer >>16);
  }  //end for i
  stopBits(PIN_CS);
  count++;
}

// No-op: this driver has no timer-ISR multiplex loop (see file header) -- the PT6355 refreshes
// its own tubes internally, so there is nothing for the periodic writeDisplay() ISR hook to do.
void writeDisplay() {
}

// clearTubes() is a required part of the common driver interface, but nothing in the core .ino
// calls it any more (its only former caller, disableDisplay(), has been removed entirely as dead
// code) - kept as a no-op stub purely for interface compliance.
void clearTubes() {}
#endif

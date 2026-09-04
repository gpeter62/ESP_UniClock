#ifdef SN75518_ESP32
//=============================================================================================
// Driver for VFD (Vacuum Fluorescent Display) tubes driven by the Texas Instruments SN75518
// serial-in / high-voltage parallel-out VFD grid/segment driver IC (32 outputs).
//
// Wiring / protocol:
//   - DATA/CLK: plain serial shift-register bit-banging, MSB first (bit 31 down to bit 0),
//     see writeDisplay(). LE (Latch Enable) is pulsed high-then-low once per digit after all
//     32 bits are shifted, to atomically transfer the shifted-in bits to the SN75518's
//     high-voltage outputs (glitch-free - outputs don't change mid-shift).
//   - STROBE is the chip's output-enable/blank pin (LOW=display on, HIGH=off, per the pin
//     comment below) and is toggled per-digit to blank the display outside its multiplex slot.
//   - SN75518 outputs are numbered 1..32 (not 0-based) in its datasheet, hence the "-1" applied
//     to segmentEnablePins[]/digitEnablePins[] everywhere they're turned into bitmasks (see
//     generateBitTable()) - unlike the SN75512 driver, which uses 0-based bit positions.
//   - Two glyph-table variants, selected in clocks.h per clock config:
//       * #define SEGMENT8: classic 7-segment+DP tubes (e.g. IV-11/IV-12), reuses the same
//         8-bit a..g+DP charDefinition[] layout as the SN75512/MAX6921 drivers.
//       * default (no SEGMENT8): 16-segment+DP alphanumeric tubes (e.g. IV-17), with a full
//         ASCII (space..DEL) 17-bit charDefinition[] table and ASCIImode glyph lookup via
//         asciiConvert[] (maps the digit[]-buffer glyph codes 0-20 to a printable ASCII char).
//   - Brightness/dimming is done differently from SN75512: instead of a fixed multiplex period
//     with a duty-cycle counter, writeDisplay() runs the ESP32 hardware timer in a genuine
//     variable-period ON/OFF PWM scheme - see the state/timerON/timerOFF logic below.
//   - Platform: ESP32 only (uses the ESP32 hw_timer_t API - timerAlarmWrite/Enable - and
//     portENTER_CRITICAL_ISR/DRAM_ATTR/IRAM_ATTR).
//
// Used by CLOCK_48 (WROOM ESP32, UNFI, 6x IV-17, 16-segment mode) and CLOCK_49 (WROOM ESP32,
// UNFI, 6x IV-11, SEGMENT8/7-segment mode) in clocks.h.
//=============================================================================================
char tubeDriver[] = "SN75518_ESP32";
//int maxDigits = 6;
int wt = 5;   //Serial timing

#define PIN_LE      13  // D7 Shift Register Latch Enable
#define PIN_CLK     12  // D6 Shift Register Clock
#define PIN_DATA    27  // D5 Shift Register Data
#define PIN_STROBE  14  // D1 Shift Register Strobe (1=display off     0=display on)

//#define SEGMENT8   //please set in clocks.h
//#define TESTMODE   //for testing the segments of a new type VFD module - cycles through output
                      //Q1..Q32 one at a time (see "count" below) so each physical segment/grid
                      //wire can be identified by eye when commissioning a new/unknown VFD module.
int count = 15;  //output# to test
unsigned long lastTest = 0;

#ifdef SEGMENT8
  //7-segment+DP tubes (e.g. IV-11): reuse the digit[]-buffer glyph-code convention directly
  //(no ASCII remapping), so ASCIImode stays false and dispChar==digit[pos] in writeDisplay().
  boolean ASCIImode = false;
  #define DP_CHAR 12
  byte charDefinition[] = {
//------------------abcdefgDP----------------   definition of different characters
//Each byte is a 7-segment+DP mask, MSB=a ... LSB=DP. generateBitTable() expands these into
//32-bit shift-register bitmaps (charTable[]) via segmentEnablePins[] (defined in clocks.h).
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

#else
  //16 segment + DP  ---------------------------------------------------------------------------------------------
  //16-segment alphanumeric tubes (e.g. IV-17): full ASCII glyph set is needed (letters, symbols),
  //so instead of the 21-entry digit[]-buffer glyph codes we index this table by ASCII character
  //(space=0x20 .. DEL=0x7F, 96 entries), via the asciiConvert[]+dispChar-=32 lookup below.
  boolean ASCIImode = true;
  #define DP_CHAR 14
  //Each entry is a 17-bit mask, one bit per segment, in the order given by the "Dutsrpnmkhgfedcba"
  //comment below (bit0=a ... bit15=u, bit16=D/decimal point) - note this is the OPPOSITE bit
  //order from the SEGMENT8/charDefinition[] table above (there MSB=a); generateBitTable() below
  //reverses it (segmentEnableBits[MAXSEGMENTS-j-1]) to compensate when building charTable[].
  uint32_t charDefinition[96] = {
//  Dutsrpnmkhgfedcba
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
  0b01000001010000001, /* ~ */
  0b00000000000000000, /* (del) */
  //Dutsrpnmkhgfedcba  
};

  //byte segmentEnablePins[] =  {16,7,0,1,2,3,4,5,6,8,9,11,10,12,13,14,15};    //segment enable bits defined in clocks.h
#endif

#define MAXCHARS sizeof(charDefinition)/sizeof(charDefinition[0])
#define MAXSEGMENTS sizeof(segmentEnablePins)
//Maps the digit[]-buffer glyph codes (0-9=digits,10=blank,11=minus,12=dp,13-20=A,P,C,deg,%,
//lower-circle,I,F - see index comments on the SEGMENT8 charDefinition[] table above) to a
//printable ASCII character, so that in 16-segment/ASCIImode the same digit[]-buffer convention
//used by the rest of the firmware still works: writeDisplay() does
//asciiConvert[digit[pos]] then "-32" to turn that into a 0-based index into the 96-entry
//charDefinition[]/charTable[] (which starts at ASCII space=0x20).
char asciiConvert[] = "0123456789 -.APC~% IF";
int maxDigits =  sizeof(digitEnablePins);
int dispChar;

uint32_t DRAM_ATTR animationMaskBits[5];
uint32_t DRAM_ATTR charTable[MAXCHARS];              //generated pin table from segmentDefinitions
uint32_t DRAM_ATTR segmentEnableBits[MAXSEGMENTS];   //bitmaps, generated from EnablePins tables
uint32_t DRAM_ATTR digitEnableBits[10];

int DRAM_ATTR PWMrefresh=5500;   //Multiplex time period, in ESP32 hardware-timer ticks (not milliseconds, despite the variable name). Greater value => slower multiplex frequency
int DRAM_ATTR PWM_min = 500;
int DRAM_ATTR PWM_max = 5000;
//int DRAM_ATTR PWMtiming[11] = {0,500,800,1200,2000,2500,3000,3500,4000,4500,5000};

//-----------------------------------------------------------------------------------------

//https://sub.nanona.fi/esp8266/timing-and-ticks.html
//One tick is 1us / 80 = 0.0125us = 12.5ns on 80MHz
//One tick is 1us / 80 = 0.0125us = 6.25ns on 160MHz
// 1 NOP is 1 tick

//POSSIBLE BUG / dead code: this function is never actually called (only referenced from
//commented-out lines in writeDisplay(), e.g. "//delayMS(1);"). Also, despite its name and the
//"int d" parameter (presumably intended as a millisecond count), the parameter is unused and
//the body always executes exactly 3 NOPs (a few nanoseconds), not d milliseconds - so even if
//re-enabled it would not delay by the requested amount. Left as-is since it is not on any live
//code path.
void inline delayMS(int d) {
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
}

void setup_pins() {
  DPRINTLN("VFD Clock - setup SN75518 pins");
  pinMode(PIN_LE,  OUTPUT);  regPin(PIN_LE,"PIN_LE");
  pinMode(PIN_STROBE,  OUTPUT);  regPin(PIN_STROBE,"PIN_STROBE");
  digitalWrite(PIN_STROBE,LOW);  //brightness
  pinMode(PIN_DATA,OUTPUT);  regPin(PIN_DATA,"PIN_DATA");
  pinMode(PIN_CLK, OUTPUT);  regPin(PIN_CLK,"PIN_CLK");
  #ifdef TESTMODE
    DPRINT("Testing output Q#:"); DPRINTLN(count);
  #endif
  //Build a human-readable summary of the active pin mapping for display on the config web page
  //(see driverSetupStr usage in ESP32_UniClock2.ino) - purely diagnostic, no functional effect.
  driverSetupStr = "SN75518 segmentEnablePins:";
  for (int j=0;j<sizeof(segmentEnablePins);j++) {
    driverSetupStr += String(segmentEnablePins[j]) + ",";
  }
  driverSetupStr += "<br>SN75518 digitEnablePins:";
  for (int j=0;j<sizeof(digitEnablePins);j++) {
    driverSetupStr += String(digitEnablePins[j]) + ",";
  }
  driverSetupStr += String("<br>");
  
  generateBitTable();  //must run before the timer/ISR starts - writeDisplay() reads charTable[]/digitEnableBits[]/segmentEnableBits[]/animationMaskBits[]
  digitsOnly = false;
  startTimer();
}

//Timer-interrupt multiplex refresh handler. Unlike the fixed-period SN75512 driver (which uses
//a constant multiplex tick and a duty-cycle counter for brightness), this driver re-arms the
//ESP32 hardware timer with a DIFFERENT period every call, alternating between an "ON" phase
//(digit lit, duration = PWMtimeBrightness, proportional to brightness) and an "OFF" phase
//(digit blanked, duration = PWMrefresh - PWMtimeBrightness) via the `state` toggle - i.e. true
//variable-mark-space PWM dimming rather than a fixed-frequency multiplex with sub-cycle gating.
//The digit position (pos) only advances at the start of each "ON" phase.
void IRAM_ATTR writeDisplay(){  //void IRAM_ATTR  writeDisplay(){
  static DRAM_ATTR int timer = PWMrefresh;
  static DRAM_ATTR uint32_t val;
  static DRAM_ATTR byte pos = 0;
  static DRAM_ATTR boolean state=true;
  static DRAM_ATTR int brightness;
  static DRAM_ATTR int PWMtimeBrightness=PWM_min;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
      //digitalWrite(PIN_STROBE,HIGH);    //OFF
    return;   //NOTE: unlike SN75512's ISR, no timerAlarmWrite()/timerAlarmEnable() re-arm is
              //needed here - this driver's ESP32 hw timer is configured with autoreload=true
              //(see the timerAlarmWrite(...,true) calls below), so it keeps firing at its last
              //configured period on its own; simply returning is enough to skip a refresh cycle
              //while an EEPROM write is in progress. (Whatever was last shifted out stays lit,
              //since STROBE is left as-is - the blanking line above is commented out.)
  }
  //Example of how to force specific glyphs onto specific tube positions for bench testing:
  //digit[5] = 15;   digit[4] = 16;     digit[3] = 17;     digit[2] = 18;      digit[1] = 19;       digit[0] = 20;
  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();   //belt-and-braces alongside portENTER_CRITICAL_ISR - protects the whole
                    //shift+latch sequence below from being interleaved with other interrupts/
                    //the other core while the SN75518's serial frame is being clocked out.
  intCounter++;

  brightness = displayON ? prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  //At full manual brightness (auto-brightness off), force state permanently ON so the OFF/blank
  //phase below is skipped entirely - i.e. 100% duty cycle / no blanking gap, for maximum
  //perceived brightness and refresh rate when the user has explicitly asked for max brightness.
  if ((!autoBrightness) && (brightness==MAXBRIGHTNESS)) state = true;

  if (state) {  //ON state: light up tube #pos for PWMtimeBrightness ticks, then go to OFF state
    pos++;  if (pos>maxDigits-1)  {   //go to the tube#0
      pos = 0;
      //Brightness (the ON-phase duration) is only recalculated once per full multiplex cycle,
      //at tube#0, so all tubes share the same brightness within one cycle (avoids visible
      //brightness differences between tubes if the ambient light level changes mid-cycle).
      if (autoBrightness && displayON) {   //change brightness only on the tube#0
        PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
        }
      else
        PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);
    }
    dispChar = digit[pos];
    if (ASCIImode) {
      //16-segment tables are indexed by ASCII code (space=0x20 based), not by the digit[]-buffer
      //glyph code directly, so translate glyph-code -> ASCII char via asciiConvert[] first.
      //FIXED: previously "dispChar -= 32" ran unconditionally, even when the bounds check just
      //below skipped the asciiConvert[] lookup (dispChar >= sizeof(asciiConvert), or dispChar
      //landing on the string's NUL terminator) - that produced a negative index into charTable[]
      //below (out-of-bounds read). Now the subtraction only happens together with a successful
      //lookup; anything unmapped falls back to charTable[0], the blank/space glyph.
      if (dispChar<sizeof(asciiConvert)-1) {  //-1 excludes the string's NUL terminator (see asciiConvert[] above)
        dispChar = asciiConvert[dispChar] - 32;
      } else {
        dispChar = 0;  //unmapped/out-of-range code -> blank glyph (charTable[0])
      }
    }
    //NOTE: as with the SN75512 driver, dispChar indexes charTable[] (MAXCHARS entries) with no
    //explicit bounds check against MAXCHARS itself - relies on upstream code only ever writing
    //valid glyph codes into digit[].
    val = (digitEnableBits[pos] | charTable[dispChar]);  //the full bitmap to send to MAX chip
    if (digitDP[pos]) val = val | charTable[DP_CHAR];    //Decimal Point
    #ifdef TESTMODE
      //Override the normal glyph lookup: light exactly one output (Q<count>) at a time so each
      //physical pin can be identified on the tube; "count-1" because SN75518 outputs are
      //numbered 1-based (Q1..Q32), same convention as segmentEnablePins/digitEnablePins.
      val = digitEnableBits[pos] | ((uint32_t)1<<(count-1));
      animMask[pos] = 0;
      if ((millis()-lastTest)>5000) {
        lastTest=millis();
        count++; if (count>32) count = 0;
      }
    #endif
    timer = PWMtimeBrightness;
    #ifdef CLOCK_xx  //example only
      //Template/example only - "CLOCK_xx" is never actually #defined by any real CLOCK_NN in
      //clocks.h, so this block is permanently dead code as shipped. It documents the intended
      //pattern for compensating individual tubes that appear dimmer than others (e.g. due to
      //VFD wear or grid-wiring differences): give a specific tube position a longer ON time
      //(here 2x/3x) than PWMtimeBrightness would otherwise give it. To use it for a real clock,
      //replace CLOCK_xx with that clock's actual CLOCK_NN macro.
      if (pos==2) timer = 2*timer;  //Weak  tube#2 brightness compensation, some hacking
      if (pos==4) timer = 3*timer;  //Weak  tube#4 brightness compensation, some hacking
    #endif
    timerON = timer;    //debug/telemetry only (see intCounter/timerON/timerOFF in ESP32_UniClock2.ino)
    timerOFF = PWMrefresh-PWMtimeBrightness;
  }
  else {  //OFF state: blank for the remainder of the PWMrefresh period
    timer = PWMrefresh-PWMtimeBrightness;
  }

  if (timer<500) timer = 500;  //safety only... avoids an excessively short/zero hw-timer alarm period (very high ISR rate) from a pathological brightness/PWM combination

  if ( (brightness == 0) || (!state) || (!radarON)) {  //OFF state, blank digit
    digitalWrite(PIN_STROBE,HIGH);    //OFF
  }
  else {  //ON state
    if (animMask[pos]>0) val &= animationMaskBits[animMask[pos]-1];  //animationMode 6, mask characters from up to down and back
    //Shift out all 32 bits, MSB first (Q32 down to Q1). Timing note: unlike the SN75512 driver
    //(which uses raw WRITE_PERI_REG()+NOP padding for a tightly-controlled bit-bang), this uses
    //plain digitalWrite() with no explicit delay between DATA and CLK edges - the delayMS(1)
    //calls that would add a small pad are commented out. The function-call overhead of
    //digitalWrite() itself appears to provide enough setup/hold margin for the SN75518 at the
    //ESP32's clock speed, but that margin is implicit rather than guaranteed by an explicit delay.
    for (int i=0; i<32; i++)  {
      if (val & uint32_t(1 << (31 - i)))
        {digitalWrite(PIN_DATA, HIGH);   //asm volatile ("nop");
        }
      else
        {digitalWrite(PIN_DATA, LOW);    //asm volatile ("nop");
        }

      digitalWrite(PIN_CLK,HIGH);  //asm volatile ("nop");  //delayMS(1);
      digitalWrite(PIN_CLK,LOW);   //asm volatile ("nop"); //delayMS(1);
      } //end for

    //Latch: pulse LE high then low to transfer the just-shifted 32 bits into the SN75518's
    //output latch (glitch-free update), then re-enable STROBE (LOW=on) to actually light the tube.
    digitalWrite(PIN_LE,HIGH );  asm volatile ("nop");
    digitalWrite(PIN_LE,LOW);
    digitalWrite(PIN_STROBE,LOW );   //ON
  }  //end else

  portEXIT_CRITICAL_ISR(&timerMux);
  state = !state;   //flip ON/OFF phase for the next timer interrupt
  //ESP32timer = timerBegin(0, PRESCALER, true);  //set prescaler, true = edge generated signal
  //timerAttachInterrupt(ESP32timer, &writeDisplay, true);
  //Re-arm the hw timer with the period (ON or OFF duration) just computed above - this is what
  //gives this driver its variable-frequency PWM dimming (as opposed to SN75512's fixed period).
  timerAlarmWrite(ESP32timer, timer, true);
  timerAlarmEnable(ESP32timer);
  interrupts();
}

//One-time (setup) precomputation, mirroring SN75512's generateBitTable() but for the SN75518's
//1-based (Q1..Q32) output numbering (note the "-1" on every segmentEnablePins[i]/
//digitEnablePins[i] use below) and its extra animationMaskBits[] table used by the digit-change
//"wipe" animation (mode 5/6 in changeDigit(), core .ino file).
void generateBitTable() {
uint32_t out;

  DPRINT("--- Generating segment pins bitmap:"); DPRINTLN(MAXSEGMENTS);
  for (int i=0;i<MAXSEGMENTS;i++) {
    segmentEnableBits[i] = uint32_t(1)<<(segmentEnablePins[i]-1);
    //DPRINT(i); DPRINT(": "); DPRINTLN(segmentEnableBits[i],BIN);
  }
#ifdef SEGMENT8
  //Progressive top-to-bottom segment groups for the 7-segment wipe animation: each mask ORs in
  //one more "row" of segments (a -> +b/f -> +g -> +c/e -> +d), later inverted below so that
  //ANDing a character's bitmap with animationMaskBits[N] blanks out the segments not yet
  //reached by the wipe.
  animationMaskBits[0] = uint32_t(1<<segmentEnablePins[0]);  //a
  animationMaskBits[1] = animationMaskBits[0] | uint32_t(1)<<segmentEnablePins[1] | uint32_t(1<<segmentEnablePins[5]);  //bf
  animationMaskBits[2] = animationMaskBits[1] | uint32_t(1)<<segmentEnablePins[6];  //g
  animationMaskBits[3] = animationMaskBits[2] | uint32_t(1)<<segmentEnablePins[4] | uint32_t(1<<segmentEnablePins[2]);  //ec
  animationMaskBits[4] = animationMaskBits[3] | uint32_t(1)<<segmentEnablePins[3];  //d
#else
  //Dutsrpnmkhgfedcba
  //DP=16,u=15,t=14,s=13,r=12,p=11,n=10,m=9,k=8,h=7,g=6,f=5,e=4,d=3,c=2,b=1,a=0}
  //Same top-to-bottom progressive wipe idea, but for the 16-segment layout (grouped by the
  //segments' approximate vertical position within the character cell rather than a..g letters).
  //NOTE: unlike the segmentEnablePins[]/charTable[] bit-order handling elsewhere in this file,
  //these use segmentEnablePins[N] directly (no "-1") - i.e. these masks are built with 0-based
  //bit positions offset by one from the rest of the table (a latent inconsistency to be aware
  //of if segmentEnablePins[] pin numbers are ever renumbered/changed).
  animationMaskBits[0] = uint32_t(1<<segmentEnablePins[0]) | uint32_t(1<<segmentEnablePins[1]);  //ab
  animationMaskBits[1] = animationMaskBits[0] | uint32_t(1<<segmentEnablePins[2]) | uint32_t(1<<segmentEnablePins[8]) |
      uint32_t(1<<segmentEnablePins[9]) | uint32_t(1<<segmentEnablePins[10]) | uint32_t(1<<segmentEnablePins[7]);  //hkmnc
  animationMaskBits[2] = animationMaskBits[1] | uint32_t(1<<segmentEnablePins[15]) | uint32_t(1<<segmentEnablePins[11]);  //up
  animationMaskBits[3] = animationMaskBits[2] | uint32_t(1<<segmentEnablePins[6]) | uint32_t(1<<segmentEnablePins[14]) |
      uint32_t(1<<segmentEnablePins[13]) |uint32_t(1<<segmentEnablePins[12]) |uint32_t(1<<segmentEnablePins[3]);  //gtsrd
  animationMaskBits[4] = animationMaskBits[3] | uint32_t(1<<segmentEnablePins[5]) | uint32_t(1<<segmentEnablePins[4]);  //fe
#endif
  DPRINTLN("--- Generating animation mask bitmap:");
  for (int i=0;i<5;i++) {
    animationMaskBits[i] = ~animationMaskBits[i]; //invert bits: AND-ing a char's bitmap with this clears (blanks) the "not yet revealed" segments, keeping everything else unaffected
    //DPRINTLN(animationMaskBits[i],BIN);
  }
  DPRINT("--- Generating digit pins bitmap:"); DPRINTLN(maxDigits);
  for (int i=0;i<maxDigits;i++) {
    digitEnableBits[i] = uint32_t(1 << (digitEnablePins[i]-1));
    //DPRINT(i); DPRINT(": "); DPRINTLN( digitEnableBits[i],BIN);
  }

  DPRINT("--- Generated Character / Pins table:"); DPRINTLN(MAXCHARS);
  for (int i=0;i<MAXCHARS;i++) {
    out = 0;
    //DPRINT(i); DPRINT(":  ");
    //DPRINT(charDefinition[i],BIN);  //DPRINT(" = ");
    //Bit j of charDefinition[i] (0=a in the "Dutsrpnmkhgfedcba" order) maps to
    //segmentEnableBits[MAXSEGMENTS-1-j] - i.e. REVERSED relative to segmentEnablePins[]'s own
    //index order, because charDefinition[] here is written MSB-high-letter-first while
    //segmentEnablePins[] is written a-first; see the file header/table comments above.
    for (int j=0;j<MAXSEGMENTS;j++)   //from a to g
      if ((uint32_t)(charDefinition[i] & (uint32_t)1<<j) != 0) {
        out = out | segmentEnableBits[MAXSEGMENTS-j-1]; //DPRINT("1");
        }
    //else        {DPRINT("0");}
    //DPRINT("  >> ");

    charTable[i] = out;
    //DPRINTLN(charTable[i],BIN);
  }  //end for
}


void clearTubes() {
  digitalWrite(PIN_STROBE,HIGH);
}

void writeDisplaySingle() {}
#endif

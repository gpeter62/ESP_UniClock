/*==============================================================================
  MAX7219CNG.ino - driver for the MAX7219 (or compatible, e.g. MAX7221/CNG clone)
  LED display driver IC.

  Hardware: unlike the MM5450 driver in this project, the MAX7219 has its OWN
  internal digit scan/multiplexing (up to 8 digits) and its own PWM intensity
  control (REG_INTENSITY, 16 steps), so this driver does NOT use the periodic
  hardware-timer ISR / startTimer() at all - there is no writeDisplay() ISR
  here, only a blocking writeDisplaySingle() that pushes one full frame (all
  digits) whenever the displayed content changes. The chip keeps refreshing
  the LEDs by itself between calls.

  Wiring/protocol: standard MAX72xx 3-wire serial bus (DIN/CLK/LOAD-CS), bit-
  banged with shiftOut() (MSBFIRST) rather than hardware SPI. Each 16-bit
  transfer is {8-bit register/digit address, 8-bit data}, latched into the
  chip's internal register on the LOAD/CS_ rising edge. This driver runs the
  chip in "No-Decode" mode (REG_DECODE=0) so it can drive arbitrary custom
  7-segment patterns (including the extra glyphs A,P,C,deg,%,I,F) instead of
  the chip's built-in BCD/font decoder, which only knows digits 0-9 and a few
  symbols.

  Used by (see clocks.h): CLOCK_10 - "8266 LED Clock with MAX7219 chip"
  (clocks.h defines MAX7219CNG only inside the #ifdef CLOCK_10 block).
==============================================================================*/
#ifdef MAX7219CNG
char tubeDriver[] = "MAX7219CNG";
int maxDigits = 6;
byte tubes[] = {0,1,2,3,4,5};    //change it, if needed for the correct tube sequence
// tubes[i] maps logical digit position i (0..maxDigits-1, left to right as wired to the
// MAX7219's DIG0..DIG7 addresses) to the index into the shared digit[]/digitDP[] buffers -
// lets the PCB wire digits in any physical order without changing the core clock logic.
byte brightConvert[] = {0,1,2,3,4,6,8,10,12,14,15};   //convert brightness from 0..10 to 0..15
// Non-linear lookup table: the firmware-wide brightness scale is 0..10 (MAXBRIGHTNESS=10
// for CLOCK_10), but REG_INTENSITY wants 0..15 - this table also biases the mapping
// (skipping value 5/7/etc.) so perceived brightness steps feel more even than a plain x1.5 scale.

//MAX7219CNG control pins
#define PIN_LOAD 15  // D8 LOAD/CS_
#define PIN_CLK  14  // D5 Clock
#define PIN_DIN  13  // D7 DataIN

//MAX7219 Registers (see MAX7219/MAX7221 datasheet "Table 2. Register Address Map")
#define REG_SHUTDOWN   0x0C  // 0 = shutdown (blank, low power), 1 = normal operation
#define REG_SCANLIMIT  0x0B  // how many digits (0..7, i.e. count-1) the chip actively multiplexes
#define REG_DECODE     0x09  // per-digit BCD-decode enable bitmask; 0 = No-Decode (raw segment bits, used here)
#define REG_INTENSITY  0x0A  // global PWM duty cycle for all digits, 0x00 (dimmest) .. 0x0F (brightest)
#define REG_TEST       0x0F  // 1 = display-test mode (forces all segments on at full brightness), 0 = normal

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
                   B11000110,   //grad  (upper circle) abfg  (16)         
                   B10110100,   //%  acdf  (17)    
                   B00111010,   //lower circle cdeg  (18)                   
                   B01100000,   //I  bc    (19)
                   B10001110    //F  aefg  (20)                         
};

//Fill this table with the output-bit numbers of the MAX7219 chip!   Only No-Decode mode is used, for flexible hardware
// segmentEnablePins[j] = which bit position (0..7) within the MAX7219's per-digit data
// byte drives logical segment j (a,b,c,d,e,f,g,DP) - lets the segment-to-pin wiring on the
// PCB differ from the charDefinition[] bit order without touching the font table itself.
byte segmentEnablePins[] =  {6,5,4,3,2,1,0,7};   //segment enable bits of MAX7219 (a,b,c,d,e,f,g,DP)   (You MUST always define 8 bits!!!)

#define MAXCHARS sizeof(charDefinition)  // (currently unused in this file - kept for reference/consistency with other drivers)
#define MAXSEGMENTS 8  // (currently unused in this file)

// Sends one 16-bit MAX7219 command: {address (register or digit 1..8), data byte}.
// LOAD/CS_ must be held LOW for the whole 16-bit transfer and pulsed HIGH afterward to
// latch it - the chip ignores clock edges while LOAD is high, so partial/torn writes
// are not possible even though this is a blocking (non-ISR) call.
void ICACHE_RAM_ATTR sendBits(byte address,byte val){
  digitalWrite(PIN_LOAD, LOW);
  shiftOut(PIN_DIN, PIN_CLK, MSBFIRST, address);
  shiftOut(PIN_DIN, PIN_CLK, MSBFIRST, val);
  digitalWrite(PIN_LOAD, HIGH);
  delay(3);  // conservative settle/latch delay; MAX7219 datasheet only requires ~tCSW ~ 50ns,
             // so this is far more generous than strictly needed (see optimization note in report)
  //DPRINT(address,HEX); DPRINT(","); DPRINTLN(val,HEX);
}

void setup_pins() {
#if defined(ESP8266) 
#else
  #error "Only 8266 Board is supported!"
#endif
  
  DPRINTLN("MAX7219 Clock - setup pins...");
  pinMode(PIN_LOAD,OUTPUT);   regPin(PIN_LOAD,"PIN_LOAD");
  pinMode(PIN_DIN, OUTPUT);   regPin(PIN_DIN,"PIN_DIN");
  pinMode(PIN_CLK, OUTPUT);   regPin(PIN_CLK,"PIN_CLK");
  digitsOnly = false;  // this driver's charDefinition[] also supports letters (A,P,C,deg,%,I,F) besides 0-9
  // Note: no startTimer() call here - this driver relies on the MAX7219's own internal
  // digit multiplexing, so no software multiplex/PWM ISR is needed for this chip.

  sendBits(REG_SHUTDOWN,1);              //Set to Normal (not Shutdown) mode
  sendBits(REG_TEST,0);                  //Set to Normal, (Non-Test) mode
  sendBits(REG_SCANLIMIT,maxDigits-1);   //Set scanlimit (chip multiplexes digits 0..maxDigits-1 only)
  sendBits(REG_DECODE,0 );               //Set Non-Decode Mode - segment data is sent as raw bit patterns, not BCD
  sendBits(REG_INTENSITY,0x0F);          //Set maximum brightness (actual runtime brightness is set per-frame in writeDisplaySingle())
}

// Blocking, one-shot push of the ENTIRE current frame (all digits + brightness) to the
// MAX7219. Called directly from the main sketch (not from an ISR - despite the
// ICACHE_RAM_ATTR tag, likely inherited from other drivers' ISR-based writeDisplay()
// pattern; harmless here, just places the function in IRAM instead of flash).
// Because the MAX7219 keeps displaying the last-latched frame by itself (on-chip
// multiplexing), this only needs to run when digit[]/digitDP[]/brightness actually change -
// there is no periodic refresh ISR for this driver (see setup_pins() note).
void ICACHE_RAM_ATTR writeDisplaySingle() {
static byte oldBright = 0;
static byte newBright = 0;
int bitBuffer;

//https://sub.nanona.fi/esp8266/timing-and-ticks.html
//One tick is 1us / 80 = 0.0125us = 12.5ns on 80MHz
//One tick is 1us / 80 = 0.0125us = 6.25ns on 160MHz
// 1 NOP is 1 tick
// (NOP-timing reference is a leftover from the bit-banged drivers - this file uses
// shiftOut()/delay() instead and doesn't rely on cycle-counted NOPs)


  newBright = brightConvert[displayON ?  prm.dayBright : prm.nightBright];  //convert 0..10 brightness levels to 0..15
  if (!radarON) newBright = 0;  // no presence detected -> force display off (still fully addressed/latched, just at 0 intensity)
  if (newBright != oldBright) {  // only touch the INTENSITY register when the value actually changed, to save a 16-bit transfer + 3ms delay
    sendBits(REG_INTENSITY, newBright);    //Set Brightness
    oldBright = newBright;
  }
  for (int i=0;i<maxDigits;i++) {
    bitBuffer = 0;
    for (int j=0;j<=7;j++)   //from a to g
      if ((charDefinition[digit[tubes[i]]] & 1<<(7-j)) != 0) {
        bitBuffer |= 1<<segmentEnablePins[j];
        }
    if (digitDP[tubes[i]]) bitBuffer |= 1<<segmentEnablePins[7];   //Decimal Point
    sendBits(i+1, bitBuffer);  // MAX7219 digit registers are addressed 1..8 (DIG0..DIG7), hence i+1
  }  //end for i
}

void clearTubes() {}  // no-op: since there's no refresh ISR, "clearing" would require an explicit
                       // blank frame write, which this driver leaves to the next writeDisplaySingle()
                       // call (or REG_SHUTDOWN could be used, but isn't here)
#endif

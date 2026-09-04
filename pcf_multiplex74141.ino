// ============================================================================
// Nixie driver: shared/multiplexed BCD-to-decimal decoder (SN74141), tube
// selection via a PCF8574 I2C GPIO expander
// ----------------------------------------------------------------------------
// Hardware: same multiplex strategy as multiplex74141.ino (ONE shared 74141
// decoder, BCD inputs wired in parallel to all tubes, time-division
// multiplexed one tube at a time via a per-tube anode-enable line - see that
// file's header for the full rationale). The difference here is where the
// per-tube ENABLE lines come from: instead of driving each digitEnablePins[i]
// straight off an ESP GPIO, up to 8 of them can instead live on the port pins
// (P0..P7) of a PCF8574 I2C I/O expander chip, addressed with a hand-rolled
// (bit-banged, NOT Wire-library) I2C implementation. This lets a clock with
// more tubes than spare direct GPIOs (this file supports up to 6-8 tubes)
// still be built with a single shared 74141 and only 2 extra ESP pins
// (PCF_SDA_PIN/PCF_SCL_PIN) instead of one extra GPIO per tube.
// digitEnablePins[i] values 0-99 address a direct ESP GPIO as usual; values
// 100-107 address PCF8574 port P(value-100) instead (see sendBits() below) -
// by convention "add 100 to the port number" when a tube's enable line is on
// the PCF chip. The ABCDPins[4] (shared BCD lines) and DP_PIN always stay on
// direct ESP GPIOs since there are only 4 of them regardless of tube count.
//
// The I2C bit-banging (not Wire.h) is deliberate: writeDisplay()/sendBits()/
// shiftout() all run inside the hardware-timer ISR (ICACHE_RAM_ATTR), and the
// Arduino Wire library is not safe/available to call from an ISR context, so
// the SDA/SCL toggling is done by hand with busy-wait "delayMS()" nop loops
// instead (see the protocol walkthrough further down).
//
// Platform: ESP8266 ONLY (this file has never been ported to ESP32; compare
// with how multiplex74141.ino/multiplex74141_ESP32.ino form a matching pair -
// no *_ESP32 sibling exists for this PCF-expander strategy).
// Used by these clocks.h configs (search for "#define PCF_74141"):
//   CLOCK_7 (8266 UNFI PCB clock, 6x Z574M/Z573M tubes) - currently commented
//   out (inactive) in clocks.h, i.e. this driver is presently orphaned/unused
//   by the active build (CLOCK_72 is the config currently enabled).
// Unlike the other files in this family, digitEnablePins[]/ABCDPins[]/DP_PIN/
// I2C_ADDR/PCF_SDA_PIN/PCF_SCL_PIN are hardcoded directly in THIS file rather
// than left commented-out for clocks.h to define per-config - clocks.h's
// CLOCK_7 block only sets "#define PCF_74141" and does not redefine any of
// these, so every clock built with this driver currently gets this exact
// fixed 6-tube/PCF pinout with no per-config override, unlike every sibling
// driver in this family.
// ============================================================================
#ifdef PCF_74141
char tubeDriver[] = "PCF_74141";
//PCF8574 I/O expander version, 4..8 tube Nixie Clock, driven by 74141
//define here the digit enable pins from 4 to 8 tubes

//if a digitEnablePort is on PCF chip, add 100 to the port number!   (for example  2-->102)

#define I2C_ADDR 0x20     //PCF8574 base 7-bit I2C address (A0/A1/A2 strapped to GND on this board)
#define PCF_SDA_PIN 4
#define PCF_SCL_PIN 5

//If a digitEnablePin is on pcf8574, add 100 to the pin number.   (P0 = 100,... P7 = 107)
byte digitEnablePins[] = {100,101,102,103,104,105};    //6 tube nixie driven by PCF - all 6 enable lines live on the PCF8574 here, none direct on ESP GPIOs
byte ABCDPins[4] = {14,12,13,2};  //D5,D6,D7,D4 on 8266 - shared BCD lines, always direct ESP GPIOs regardless of tube count
#define DP_PIN  16 // decimalPoint on 8266's D0

int maxDigits = sizeof(digitEnablePins);

//byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, is needed (for example: bad tube pin layout)
int PWMrefresh=15000;   //msec, Multiplex time period. Greater value => slower multiplex frequency
int PWM_min = 2000;
int PWM_max = 14000;
//int PWMtiming[11] = {0,2000,3000,4000,5000,6000,7000,8000,10000,12000,14000};

#if defined(ESP8266) 
#else
  #error "Only 8266 Board is supported!"  
#endif

// NOTE: named "delayMS" but the comment says "Delay microsec" - actually neither is
// literally true: it's a plain busy-wait nop loop, roughly proportional to `d` at
// whatever clock speed the 8266 runs at (not a calibrated microsecond/millisecond
// delay). Used instead of delayMicroseconds()/delay() because this runs inside the
// timer ISR (ICACHE_RAM_ATTR) where those aren't safe/appropriate to call.
void ICACHE_RAM_ATTR delayMS(int d) {        //Delay microsec
  for (int i=0;i<d*30;i++) {asm volatile ("nop"); }
}

// Bit-bangs one 8-bit byte onto the I2C bus, MSB first, toggling PCF_SCL_PIN once per
// bit while PCF_SDA_PIN carries the bit value (standard I2C: SDA must be stable
// while SCL is HIGH). After the 8 data bits, pulls SDA LOW and pulses SCL once more
// for the slave's ACK bit - but note the ACK is never actually READ back (the
// pinMode(...,INPUT_PULLUP) sampling code is commented out just above): this
// implementation blindly assumes the PCF8574 acknowledged and will happily continue
// even if the chip is missing or the bus is stuck.
void ICACHE_RAM_ATTR shiftout(byte in) {
  for (int i=0;i<8;i++) {
    digitalWrite(PCF_SDA_PIN,in  & (1<<(7-i)));
    delayMS(1);
    digitalWrite(PCF_SCL_PIN,HIGH);
    //if (i==0) delayMS(8);
    delayMS(4);
    digitalWrite(PCF_SCL_PIN,LOW);
  }

  //ACK is coming
  //pinMode(PCF_SDA_PIN,INPUT_PULLUP);
  digitalWrite(PCF_SDA_PIN,LOW);  //LOW
  delayMS(12);
  digitalWrite(PCF_SCL_PIN,HIGH);  //clock pulse
  delayMS(4);
  digitalWrite(PCF_SCL_PIN,LOW);
  //pinMode(PCF_SDA_PIN,OUTPUT);
    delayMS(4);
 }

// Sends one complete I2C write transaction (START + 7-bit address+W + 1 data byte +
// STOP) to the PCF8574 at `address`, blocking (from the ISR's point of view) for the
// whole bit-banged transfer. `val` is the full 8-bit output pattern for the PCF8574's
// P0..P7 pins - since the PCF8574 has no separate "set one pin" command, every call
// here replaces the ENTIRE port state at once (this matters for how digitEnablePins
// on the PCF get turned on/off in writeDisplay() below: enabling a new tube's PCF
// pin implicitly turns every other PCF-hosted tube off in the same write).
void ICACHE_RAM_ATTR sendBits(byte address,byte val){
  //Serial.print(address,HEX); Serial.print("/"); Serial.println(val);

  // The explicit startCondition()/stopCondition() helpers below are left commented out
  // and inlined by hand instead; the START here works because the bus is left idle
  // (SDA=HIGH, SCL=HIGH) by the previous call's STOP condition (or by setup_pins()),
  // so simply pulling SDA LOW while SCL is still HIGH IS the START condition.
  //void ICACHE_RAM_ATTR startCondition() {
  //digitalWrite(PCF_SDA_PIN,HIGH);   //PCF_SDA_PIN HIGH -> LOW, while PCF_SCL_PIN is HIGH
  //digitalWrite(PCF_SCL_PIN,HIGH);
  //delayMS(4);
  digitalWrite(PCF_SDA_PIN,LOW);
  delayMS(3);
  digitalWrite(PCF_SCL_PIN,LOW);
  delayMS(4);

  shiftout(address<<1);   //shift address one bit left, because 0bit is READ/WRITE mode. 0=WRITE
  shiftout(val);

  // STOP condition: SDA transitions LOW->HIGH while SCL is HIGH.
  //void ICACHE_RAM_ATTR stopCondition() {
  delayMS(4);
  digitalWrite(PCF_SDA_PIN,LOW);   //PCF_SDA_PIN LOW -> HIGH, while PCF_SCL_PIN is HIGH
  delayMS(2);
  digitalWrite(PCF_SCL_PIN,HIGH);
  delayMS(4);
  digitalWrite(PCF_SDA_PIN,HIGH);
  delayMS(4);
}


void setup_pins() {
  char tmp[30];

  DPRINTLN("PCF8574 MULTIPLEX driver");
  for (int i=0;i<maxDigits;i++)  {
    if (digitEnablePins[i]<100) {
      pinMode(digitEnablePins[i], OUTPUT);   //8266 pins setup, if needed
      sprintf(tmp,"digitEnPin[%d]",i);
      regPin(digitEnablePins[i],tmp);
      }
    // NOTE: pins >=100 (PCF-hosted) are intentionally NOT pinMode'd/regPin'd here -
    // they aren't real ESP GPIOs at all, just an index into the PCF8574's output byte
    // sent over I2C via sendBits() below, so there's nothing local to configure.
  }
  for (int i=0;i<4;i++)  {
    pinMode(ABCDPins[i], OUTPUT);
    sprintf(tmp,"Pin[%c]",char('A'+i));
    regPin(ABCDPins[i],tmp);
  }
  pinMode(PCF_SDA_PIN,OUTPUT); regPin(PCF_SDA_PIN,"PCF_SDA_PIN");
  pinMode(PCF_SCL_PIN,OUTPUT); regPin(PCF_SCL_PIN,"PCF_SCL_PIN");
  pinMode(DP_PIN,OUTPUT);  regPin(DP_PIN,"DP_PIN");
  digitalWrite(PCF_SDA_PIN,HIGH);
  digitalWrite(PCF_SCL_PIN,HIGH);  //leave the bus idle (both lines HIGH) before the first transaction
  delay(100);
  sendBits(I2C_ADDR,0);   //clear all 8 PCF8574 outputs (all tubes disabled) at startup

  startTimer();
  // Software watchdog re-armed with a generous 8s timeout: the bit-banged I2C
  // transaction in sendBits() runs with interrupts effectively serialized inside the
  // ISR and can take a comparatively long time (many delayMS() busy-waits per call),
  // so the default (shorter) 8266 watchdog period could otherwise trip during startup.
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);
  }

void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  static byte pos = 0;
  static byte state=0;
  static int timer = PWMrefresh;
  static byte num,brightness;
  static byte p,DPpos;
  static int PWMtimeBrightness = PWM_min;

if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    //digitalWrite(digitEnablePins[pos],LOW);
    timer1_write(PWMrefresh);
    return;
  }

  // POSSIBLE BUG: PWMtimeBrightness is computed here from `brightness` BEFORE
  // `brightness` is (re)assigned a few lines below - so this uses the value left
  // over from the PREVIOUS ISR call, not the current one (a one-call lag whenever
  // brightness/lx changes). Also, unlike multiplex74141.ino/multiplex74141_ESP32.ino
  // (which only recompute PWMtimeBrightness once per FULL multiplex cycle, when pos
  // wraps back to 0, specifically to keep brightness visually uniform across all
  // tubes within one cycle), this recomputes it on EVERY ISR call regardless of
  // state/pos - so if autoBrightness/lx changes mid-cycle, different tubes within
  // the same refresh pass could end up with different PWMtimeBrightness values.
  if (autoBrightness && displayON) {
    PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
  }
  else
    PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);

  intCounter++;
  timer = PWMrefresh;
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

   // Same 4-state per-tube multiplex/animation state machine as multiplex74141.ino -
   // see that file's header for the full write-up. state 0: advance to next tube
   // (old digit during a wipe); state 1: new digit for the rest of a wipe; state 2:
   // blank for the remaining OFF portion of the cycle (brightness dimming); state 3:
   // one-tick marker consumed immediately below.
   switch (state) {   //state machine...
    case 0:
      pos++;  if (pos>maxDigits-1)  pos = 0;  //go to the first tube

      if (animMask[pos] > 0) { //Animation?
        num = oldDigit[pos];  //show old character
        timer = (PWMtimeBrightness * (20-animMask[pos]))/20;
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
      timer = (PWMtimeBrightness * animMask[pos])/20;      
      state = 2;  //default next state is: BLANK display
      break;
    case 2:  //blank display
      timer = PWMrefresh-PWMtimeBrightness;
      state = 3;
      break;
   }  //end switch

   //  if ((pos>0) && (num<=9)) num = convert[num];   //tube character conversion, if needed... (maybe bad pin numbering)

  p  = digitEnablePins[pos];  //resolve this cycle's tube-enable target: <100 = direct ESP GPIO, >=100 = PCF8574 port (p-100)

  if ((brightness == 0) || (state == 3) || (num >9) || (!radarON)) {  //blank digit
    state = 0;
    // Blanking is done purely by pushing an invalid BCD code (1111=15) to the shared
    // 74141 - unlike multiplex74141.ino/multiplex74141_ESP32.ino, which additionally
    // switch the CURRENT tube's digitEnablePins[pos] line LOW here. That matters:
    // POSSIBLE BUG - `p` (the just-selected tube's enable line) is never explicitly
    // turned back off, neither here nor anywhere else in this file. For a PCF-hosted
    // pin (p>=100) this is harmless in practice, because sendBits() below always
    // rewrites the WHOLE PCF8574 output byte with only the newly-selected tube's bit
    // set, which implicitly clears every previously-enabled PCF pin. But for a
    // DIRECT ESP GPIO enable pin (p<100), digitalWrite(p,HIGH) is a single-pin write
    // with no such "whole port" side effect - once such a pin is first driven HIGH it
    // is NEVER driven LOW again anywhere in this file. If a clock config mixes direct
    // and PCF-hosted digitEnablePins (the setup_pins() loop above clearly anticipates
    // this), every direct-GPIO tube's anode would stay permanently powered after its
    // first turn, relying solely on the BCD=1111 blanking to keep it dark the rest of
    // the time - but since ALL tubes share the same 74141/ABCDPins, that direct-GPIO
    // tube would then visibly ghost (light up showing) whatever digit is currently
    // being driven for a DIFFERENT tube's turn. The shipped default config
    // (digitEnablePins[] = {100,101,102,103,104,105}, all PCF-hosted) never exercises
    // this path, which is presumably why it has gone unnoticed.
    for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],HIGH); }
      digitalWrite(DP_PIN,LOW);
    }
  else {
      if (p<100) digitalWrite(p,HIGH);      //switch ON new digit on 8266
      else sendBits(I2C_ADDR,1<<(p-100)) ;  // or on PCF port - single-bit-set byte, see note above
      // LEFTDECIMAL: some tube types (Z574M) have their DP cathode positioned to the
      // LEFT of the printed digit, so its DP is lit together with the NEXT tube's slot
      // instead of the current one (see CLOCK_7's "#define LEFTDECIMAL true" in clocks.h).
      if (LEFTDECIMAL) DPpos = min(maxDigits-1,pos+1); else DPpos = pos;
      if (digitDP[DPpos] && (brightness>0)) digitalWrite(DP_PIN,HIGH); //switch ON decimal point, if needed
      for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }  //push BCD bits (bit i -> ABCDPins[i]) to the shared 74141
  }

  #if COLON_PIN >= 0
    if (num==10) digitalWrite(COLON_PIN,LOW);      // Colon pin OFF
    else digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
  #endif

  #if DECIMALPOINT_PIN >=0
        // POSSIBLE BUG: stray extra "}" at the end of this line - the `else` branch is a
        // single bare statement (no opening brace), so this closing brace has no match.
        // Currently harmless only because DECIMALPOINT_PIN is normally left at -1 for
        // this driver's clock configs, so this whole #if block never actually compiles;
        // it would very likely fail to compile as soon as a PCF_74141-based config ever
        // defined DECIMALPOINT_PIN >=0. Compare with the correctly-braced
        // "else {digitalWrite(DECIMALPOINT_PIN,decimalpointON); }" in multiplex74141.ino.
        if (num==10) {digitalWrite(DECIMALPOINT_PIN,LOW);}
        else digitalWrite(DECIMALPOINT_PIN,decimalpointON); }
  #endif

  if (timer<500) timer = 500;  //safety only...
  timer1_write(timer);
}

void ICACHE_RAM_ATTR clearTubes() {
    digitalWrite(DP_PIN,LOW);
    sendBits(I2C_ADDR,0);   //zero out all PCF8574 outputs - blanks every PCF-hosted tube; note direct-GPIO (p<100) tubes are NOT addressed here (see the ABCDPins-only comment below)
    //for (int i=0;i<4;i++) digitalWrite(ABCDPins[i],HIGH);
}

// Not implemented for this driver: the multiplex ISR (writeDisplay) already refreshes
// continuously on its own, so there's no separate blocking "write once" path here;
// kept only for interface parity with drivers that need it.
void writeDisplaySingle() {}
#endif

//============================================================================
// HV5122 Nixie tube driver (LEGACY / ORIGINAL VERSION)
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
// Status:   THIS IS THE OLDER/LEGACY DRIVER. As of the current clocks.h, no
//           active CLOCK_NN config selects plain "HV5122" anymore - every
//           HV5122-based config (CLOCK_50, 51, 52, 53, 54, 55, 56, 57, 57A)
//           now defines NEWHV5122 and is compiled from HV5122new.ino instead.
//           This file is effectively unused/orphaned code, kept for
//           reference. It lacks the crossfade/wipe digit-change animation
//           (animMask/newDigit/oldDigit) support that HV5122new.ino adds -
//           see HV5122new.ino's header comment for details.
//============================================================================
#ifdef HV5122  //Nixie driver
//P.S. 6x tubes PCB version

char tubeDriver[] = "HV5122";
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

#define SHIFT_LSB_FIRST false  //true= LSB first, false= MSB first
int PWMrefresh = 10000; //Brightness PWM period (microseconds-ish timer ticks). Greater value => slower brightness PWM frequency
//int PWMtiming[11] = {0,500,800,1200,2000,2500,3000,4500,6000,8000,10000};
int PWM_min = 500;   //shortest allowed ON time (floor, avoids a 0-length timer alarm)
int PWM_max = 10000; //longest ON time = full brightness (== PWMrefresh, i.e. tube stays on the whole period)

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


// writeDisplay(): TIMER-ISR. Unlike most other drivers in this project, this
// driver does NOT multiplex individual tube positions from here - the full
// digit pattern for ALL tubes was already shifted out into the HV5122 latches
// by writeDisplaySingle() (called from the main loop). This ISR only toggles
// the shared PIN_OE line on/off to implement brightness as a simple
// square-wave PWM (whole display blinks ON/OFF at a variable duty cycle -
// there is no per-tube time-slicing here since HV5122 has a real static
// latch per output, not a multiplexed matrix).
#if defined(ESP32)
void IRAM_ATTR writeDisplay() { //void IRAM_ATTR  writeDisplay(){
#else
void ICACHE_RAM_ATTR writeDisplay() {       //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
#endif
  static volatile int timer = PWMrefresh;
  static volatile boolean state=true;    //true = about to enter/currently in the ON phase of the PWM cycle
  static volatile int brightness;
  static int PWMtimeBrightness=PWM_min;

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

  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  if (autoBrightness && displayON) {
    PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);  //day mode + ALS: scale ON-time by measured lux instead of the stored brightness setting
  }
  else
    PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);

  //At full brightness, force a steady ON state every call instead of letting the
  //ON/OFF alternation run its course - avoids a needless (and here pointless,
  //since ON-time==PWMrefresh anyway) OFF sliver caused by scheduling/rounding jitter.
  if ((!autoBrightness) && (brightness==MAXBRIGHTNESS))
    state = true;

  if (state) {  //ON state
    timer = PWMtimeBrightness;
    timerON = timer;
  }
  else {  //OFF state
    timer = PWMrefresh-PWMtimeBrightness;
    timerOFF = timer;
  }
  if (timer<500) timer = 500;  //safety only... (timerAlarmWrite/timer1_write with 0 or a very small value can hang/misfire)

  if ( (brightness == 0) || (!state) || (!radarON)) {  //OFF state, blank digit
    digitalWrite(PIN_OE, LOW); //OFF  -- NOTE: on this board OE is active-HIGH (HIGH=tubes lit), opposite of the active-low /OE found on many shift-register ICs
    }
  else {  //ON state
    digitalWrite(PIN_OE, HIGH);   //ON
  }

  state = !state;  //flip for next ISR call (alternate ON phase / OFF phase); irrelevant while the full-brightness override above forces true every time

#if defined(ESP8266)
  timer1_write(timer);
#elif defined(ESP32)
  portEXIT_CRITICAL_ISR(&timerMux);
  //ESP32timer = timerBegin(0, PRESCALER, true);  //set prescaler, true = edge generated signal
  //timerAttachInterrupt(ESP32timer, &writeDisplay, true);   
  timerAlarmWrite(ESP32timer, timer, true);   
  timerAlarmEnable(ESP32timer);
  interrupts();
#endif
}



// shift(): bit-bangs one 32-bit word into the HV5122 shift-register chain.
// The ~20-NOP delays around each CLK/DIN edge are required because the
// HV5122 is a high-voltage part (internal level-shifters to the ~170V rail)
// and is much slower than a typical 5V logic shift register - driving it at
// full CPU speed without these delays can corrupt shifted-in data.
// Data is latched into the HV5122 on the falling edge of CLK.
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

// writeDisplaySingle(): builds the two 32-bit bit-buffers (one per cascaded
// HV5122 chip) from the current digit[]/digitDP[] state and shifts them out.
// Called repeatedly from the main loop (not from the ISR) and self-throttled
// to run at most every 50ms, since the bit-bang shift() above is relatively
// slow (32 bits x ~60 NOPs, twice) and re-shifting on every idle loop
// iteration would be wasteful. NOTE: unlike HV5122new.ino, this version does
// NOT check whether anything actually changed since the last call - it always
// re-shifts full data every 50ms regardless, and it ignores newDigit[]/
// oldDigit[]/animMask[] entirely, so it has no digit-change wipe/crossfade
// animation support.
void writeDisplaySingle() {
  static unsigned long lastRun = 0;
  uint32_t bitBuffer0 = 0;
  uint32_t bitBuffer1 = 0;
  byte num;

  if ((millis() - lastRun) < 50) return; //slow down!!!
  lastRun = millis();

  bitBuffer0 = 0; bitBuffer1 = 0;
  for (int i = 0; i < maxDigits; i++) { //Set the clock digits
    num = digit[i];
    if (num < 10) {  //codes 10 (BLANK), 11 (dash), 12 (dp glyph) and 13+ (extra glyphs) have no cathode mapping on a Nixie tube -> leave bits cleared = tube blank
      if (digitPins[i][num] < 100) {
        bitBuffer0 |= (uint32_t)(1 << (digitPins[i][num]-1));
        //DPRINT(digitPins[i][num]); DPRINT(":");
        } //chip0
      else {
        bitBuffer1 |= (uint32_t)(1 << (digitPins[i][num] - 101));
        //DPRINT(digitPins[i][num]);DPRINT(":");} //chip1
      }
    }
  }  //end for i

  #ifdef  MAKE_BLINKING_DOTS //it means, the extra datapins are used as 4 blinking dot instead of decimal points!
    if (showClock) {
      digitDP[1] = digitDP[2]; //left two dots
      digitDP[3] = digitDP[4]; //right two dots
    }
  #endif

  for (int i = 0; i < maxDigits-1; i++) { //Set the extra decimal point dots
    if (digitDP[i] && digitPins[maxDigits][i]>0) {
      if (digitPins[maxDigits][i] < 100)
        bitBuffer0 |= (uint32_t)(1 << (digitPins[maxDigits][i]-1)); //chip0
      else
        bitBuffer1 |= (uint32_t)(1 << (digitPins[maxDigits][i] - 101)); //chip1
    }
  }  //end for i

  //bitBuffer0 = 1<<cnt;  bitBuffer1 = 1<<cnt;  cnt++;  if (cnt>32) cnt = 0;  DPRINTLN(cnt);
  //Shift chip1 (the far end of the DIN->DOUT daisy-chain) FIRST, then chip0 LAST:
  //bits shifted in earliest travel furthest down the chain, so chip1's data must
  //go in before chip0's, otherwise chip0's own data would get pushed on into chip1.
  shift(bitBuffer1);
  //delayMicroseconds(1);   //for testing on oscilloscope
  shift(bitBuffer0);
  //showBits(bitBuffer1);  showBits(bitBuffer0);   DPRINTLN(" ");
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

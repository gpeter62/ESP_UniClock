/*
 * ======================================================================
 * 1TUBE.INO - driver for the UNFI "1TUBE" single-Nixie-tube clock
 * ======================================================================
 * Hardware: ONE physical Nixie tube (e.g. IN-16/Z-series) fed by a single
 * BCD-to-decimal decoder/driver IC (e.g. 74141/K155ID1-class chip) wired
 * to the 4 "ABCDPins" GPIOs. Because there is only one tube, the classic
 * simultaneous-multiplex approach used by multi-tube drivers cannot show
 * several digits at once; instead this driver walks through the 4 logical
 * digit positions (H,H,M,M held in digit[0..3]) ONE AT A TIME, holding
 * each on the single tube for a while and cross-fading (PWM wipe) between
 * the old and new character of that position before advancing.
 * Board target: ESP8266 (uses the ESP8266 timer1 API: timer1_write(),
 * ICACHE_RAM_ATTR); this driver is NOT built for ESP32.
 * Protocol: 4 parallel GPIO lines (ABCDPins[0..3]) carry the BCD nibble
 * that the decoder IC translates into "light cathode N" on the tube.
 * BCD codes 10 (0xA) and 15 (0xF) are both outside the valid 0-9 range,
 * so the decoder lights no cathode for them - used here as "blank" codes.
 * Used by CLOCK_NN configs (see clocks.h): CLOCK_70 ("1 tube esp8266
 * Nixie Clock by UNFI") is the only config that #defines TUBE1CLOCK.
 * ======================================================================
 */
#ifdef TUBE1CLOCK
char tubeDriver[] = "1TUBE";
//used by UNFI 1TUBE Clock

//byte ABCDPins[4] =  {2,5,4,15};  //defined in clocks.h

int maxDigits = 4;   //4 LOGICAL digit positions (H,H,M,M) even though there is only 1 PHYSICAL tube
int pos = 0;  //pointer of show phase - which of the 4 logical digit positions is currently on the single tube

int PWMrefresh = 15000;   //msec, multiplex time period. Greater value => slower multiplex frequency
int PWM_min = 2000;
int PWM_max = 15000;
int maxDig=maxDigits;

void setup_pins() {
  char tmp[30];
    
  DPRINTLN("1TUBE Clock - setup pins");

  for (int i=0;i<4;i++) {
    pinMode(ABCDPins[i], OUTPUT);
    sprintf(tmp,"Pin[%c]",char('A'+i));
    regPin(ABCDPins[i],tmp); 
  }
  startTimer();
}

void ICACHE_RAM_ATTR writeBits(byte num) {
   //Send the 4-bit BCD code to the decoder IC, LSB first on ABCDPins[0].
   //Note: digitalWrite() treats ANY non-zero value as HIGH, so passing the
   //raw masked bit (0,1,2,4,8) instead of a strict 0/1 is fine here.
   for (int j=0;j<4;j++) {
    digitalWrite(ABCDPins[j],num  & (1<<j));
   }
}

void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  //This is the ESP8266 hardware-timer1 ISR (single-shot mode: it must
  //re-arm itself via timer1_write() at the end of every call - see
  //startTimer() in the core .ino, which sets timer1 to TIM_SINGLE).
  //Because there is only ONE physical tube, "multiplex" here does not mean
  //cycling through several tubes per refresh - it means running a tiny
  //state machine (0/1/2) that, for the CURRENT logical position `pos`
  //only, PWM-blends between the old and new character (cross-fade "wipe"
  //animation) and then blanks the tube for the rest of the PWM period to
  //control brightness. Advancing `pos` itself is NOT done here; it is
  //done by writeDisplay2()/makeAnim(), which run from the main loop.
  static volatile byte state=0;
  static int timer = PWMrefresh;
  static int brightness;
  static int PWMtimeBrightness=PWM_min;
  byte animM;
  byte num;

  if (EEPROMsaving) {  //stop refresh while an EEPROM write is in progress!
    timer1_write(PWMrefresh);
    return;
  }
  //POSSIBLE BUG (ISR race condition): digit[], newDigit[], oldDigit[], animMask[]
  //and `pos` are all read here from the timer ISR while makeAnim()/writeDisplay2()
  //write to the very same globals from the main loop (no lock, no `volatile`
  //on the arrays). On ESP8266 a torn read of an int-sized array element is
  //unlikely but not architecturally guaranteed; worth a closer look if visual
  //glitches during animation are ever reported.
  animM = 0;
  if (animMask[pos] > 0) animM = animMask[pos];
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  if ((brightness ==0) || !radarON){
    writeBits(0xA);  //black display - BCD code 10 is outside 0-9, decoder lights no cathode

  #if DECIMALPOINT_PIN>=0
    digitalWrite(DECIMALPOINT_PIN,LOW);
  #endif
  #if COLON_PIN >= 0
    digitalWrite(COLON_PIN,LOW);  // colon pin OFF
  #endif
  timer1_write(100*PWMrefresh);  //re-check only occasionally while display is fully off (saves ISR overhead)
  return;
  }
  else {
    digit[4] = 0xA;  //extra "5th" slot (blank) used as the makeAnim() end-of-cycle marker/target position - see writeDisplay2()
      if (state==0) {
        if (animMask[pos]==0) num = digit[pos];
        else num = oldDigit[pos];  //mid-animation: still fading OUT the previous character
      }
      else if (state==1) num = newDigit[pos];  //mid-animation: now fading IN the new character
      else num = 0xF;  //BCD code 15 - also outside 0-9, another "blank" pattern (belt-and-braces vs 0xA)

  writeBits(num);
  switch (state) {   //state machine: 0=show current/old char, 1=show new char (animation only), 2=blank (brightness/PWM off-time)
    case 0:
      #if DECIMALPOINT_PIN>=0
        if (decimalpointON) digitalWrite(DECIMALPOINT_PIN,HIGH);
      #endif
      #if COLON_PIN>=0
        digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
      #endif
      if (autoBrightness && displayON) {
          PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);  //ambient-light-driven ON-time within the PWM period
      }
      else {
        PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);  //fixed day/night brightness setting -> ON-time
      }
      if (animM > 0) { //Animation running for this position? animM (1..19) is the wipe progress from animMask[pos]
        timer =  (PWMtimeBrightness * (20-animM))/20;  //old-character share of the ON-time shrinks as animM grows
        state = 1;  //next state is: show newDigits
      }
      else {
        timer = PWMtimeBrightness;
        if (brightness>=MAXBRIGHTNESS) state = 0;  //at max brightness skip the blank phase entirely (always-on, no flicker)
        else state = 2;  //default next state is: BLANK display
      }
      break;
    case 1:  //show new character, if animation
      timer = (PWMtimeBrightness * animM)/20;      //new-character share of the ON-time grows as animM grows
      if (brightness>=MAXBRIGHTNESS) state = 0;
      else state = 2;  //default next state is: BLANK display
      break;
    case 2:  //blank display
      #if DECIMALPOINT_PIN>=0
        digitalWrite(DECIMALPOINT_PIN,LOW);
      #endif
      #if COLON_PIN>=0
        digitalWrite(COLON_PIN,LOW);  // colon pin OFF
      #endif
      state = 0;
      timer = PWMrefresh-PWMtimeBrightness;  //remainder of the fixed-length PWM period -> keeps overall refresh rate constant
      break;
  }  //end switch
  } //end else

  if ((brightness == 0) || (!radarON)) {timer = PWM_max; state = 0;}  //no time sharing is needed
  if (timer<500) timer = 500;  //safety only... (timer1 min practical reload value; too-short reload could starve other ISRs/WiFi)
  timer1_write(timer);
}

//Cooperative/non-blocking-ish delay: since the whole time-showing sequence
//in writeDisplay2()/makeAnim() is written as a straight-line blocking loop
//(one call per animation step), we can't just call delay(d) - that would
//starve WiFi, the captive-portal DNS, the alarm buzzer and GPS parsing for
//the whole animation. So Adelay() busy-waits but keeps servicing them.
void Adelay(unsigned long d) {
  unsigned long dStart = millis();

  doAnimationMakuna();
  while ((millis() - dStart) < d) {
    if (WiFi.getMode() == 2) dnsServer.processNextRequest();  //WiFi.getMode()==2 is WIFI_AP: service the captive-portal DNS while in AP config mode
    alarmSound();
    #ifdef USE_GPS
      smartDelay(1);  //feed GPS
    #endif
    yield();  //let the ESP8266 core service WiFi/SDK background tasks (required to avoid WDT resets)
  }
}

//Drives the crossfade of the SINGLE current position `pos` from its old
//character to its new one. This is only ever meaningful for one tube at a
//time (unlike multi-tube drivers' changeDigit(), which is bypassed
//entirely for TUBE1CLOCK - see core .ino changeDigit()). Called once per
//position from writeDisplay2()'s position-walk loop.
void makeAnim() {
  static byte oldDig = 10;  //remembers the digit that was actually shown last time THIS function ran (persists across positions/cycles)

  if (cathodeProtRunning) {
    //Anti-cathode-poisoning routine: just snap to the target digit, no fade,
    //no PWM animation bookkeeping - doCathodeProtect() drives digit[] itself.
    memset(animMask,0,sizeof(animMask));
    digit[pos] = newDigit[pos];
    return;

  }
  else if ((prm.animMode==0) || ipShowRunning ){
    //Animation disabled by user setting, or showing the boot-time IP address: snap instantly.
    digit[pos] = newDigit[pos];
    memset(animMask,0,sizeof(animMask));
    Adelay(ANIMSPEED*20);
    return;
  }

  //Time showing
  if (oldDig == newDigit[pos]) oldDig = 10; //if the old digit == new digit, use space instead (avoids a pointless fade from a digit onto an identical digit)
  oldDigit[pos] = oldDig;  //last displayed digit
  //DPRINT(oldDigit[pos]); DPRINT("/"); DPRINTLN(newDigit[pos]);
  for (int i = 1; i < 20; i++) {
    animMask[pos] = i;  //drives the writeDisplay() ISR's PWM wipe ratio (old-share shrinks, new-share grows) for this position, step by step
    Adelay(ANIMSPEED);
  }
  oldDig = newDigit[pos];  //save last displayed digit
  digit[pos] = newDigit[pos];
  animMask[pos] = 0;
  Adelay(5*ANIMSPEED);
  if (pos==2) {
    //pos==2 is the "hours ones" digit - digit[3]=hour tens, digit[2]=hour ones,
    //digit[1]=minute tens, digit[0]=minute ones (set in core .ino's time-digit
    //code). The walk in writeDisplay2() goes 3,2,1,0, so after showing the HH
    //part (positions 3 and 2) we insert a blank gap here so the HH/MM halves
    //of the time are visually separated on the single tube instead of running together.
    digit[pos] = 10;
    oldDigit[pos] = 10;
    oldDig = 10;
    Adelay(20*ANIMSPEED);
  }
}

//Required common-interface function ("blocking, non-interrupt, one-shot
//write of the display buffer") - but for this single-tube driver the
//actual pixel/segment output is always produced asynchronously by the
//writeDisplay() timer ISR, driven by `pos`. So this implementation does
//NOT push bits to hardware itself; it only redirects `pos` to the single
//physical tube's slot (index 3, reused as the cathode-protect "digit")
//so that the next ISR tick renders whatever doCathodeProtect() just wrote
//into digit[3]/newDigit[3]. Outside cathode-protect it is a no-op, so
//normal time updates still rely entirely on writeDisplay2() below
//(called from the main loop - see changeDigit() in the core .ino, which
//for TUBE1CLOCK short-circuits to just calling this function and returning).
void writeDisplaySingle() {
  if (cathodeProtRunning) {
    pos = 3;
  }
}

//NOT part of the required driver interface (setup_pins/writeDisplay/
//writeDisplaySingle/clearTubes/maxDigits/tubeDriver) - this is an EXTRA
//function that the core .ino calls directly (only for TUBE1CLOCK/VQC10/
//WORDCLOCK, see the `writeDisplay2` stub at the bottom of the core file)
//from the main loop to drive the single tube's sequential "flying digit"
//display, since a single tube can't be updated purely by changeDigit()'s
//normal per-position-array animation.
void writeDisplay2(void){
  static unsigned long lastShow = 0;

  if (cathodeProtRunning) {  //show cathode protect, only one digit
    pos = 3;
    return;
    }

  if (ipShowRunning) {    //boot-time diagnostic: show the device's IP address, 3 digits at a time, on the single tube
    memset(oldDigit,10,sizeof(oldDigit));
    newDigit[3] = 10;
    digit[3] = 10;
    for (int i=2;i>=0;i--) {
      pos = i;
      makeAnim();
      digit[pos] = 10;
      Adelay(200);
    }
    pos = 3;
    makeAnim();
    pos = 0;
    ipShowRunning = false;
    memset(digit,10,sizeof(digit));
    memset(newDigit,10,sizeof(newDigit));
    memset(oldDigit,10,sizeof(oldDigit));
    memset(animMask,0,sizeof(animMask));
    Adelay(500);
    return;
  }

  if ((millis()-lastShow)<3000) return;   //show time - only start a new HH/MM sweep once every 3s; between sweeps the last-shown digit just stays lit via the ISR
  for (int i=3;i>=0;i--) {
      pos = i;
      makeAnim();
      if (prm.animMode==0) {  //make some pause
        digit[pos] = 10;
        Adelay((i==2)? 400 : 200);  //extra-long pause after the HH half (i==2) to emphasize the HH/MM boundary
      }
  }
  newDigit[4] = 10;
  pos = 4;   //parks the state machine on the unused 5th slot (always 0xA/blank, see digit[4]=0xA in writeDisplay()) so the tube goes dark between sweeps
  makeAnim();
  memcpy(digit,newDigit,sizeof(digit));
  memcpy(oldDigit,newDigit,sizeof(oldDigit));
  memset(animMask,0,sizeof(animMask));
  lastShow = millis();
}

//Required common-interface function, but intentionally a no-op here: the core .ino no longer has
//any caller for clearTubes() at all (its only former call site, disableDisplay(), has been removed
//entirely as dead code), so there is nothing to implement. Actual blanking of the single tube is
//handled inside writeDisplay()'s own brightness==0 / !radarON branch instead.
void clearTubes() {}
#endif

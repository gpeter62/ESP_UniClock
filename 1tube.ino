#ifdef TUBE1CLOCK
char tubeDriver[] = "1TUBE";
//used by UNFI 1TUBE Clock

//byte ABCDPins[4] =  {2,5,4,15};  //defined in clocks.h

int maxDigits = 4;
int pos = 0;  //pointer of show phase

int PWMrefresh = 15000;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
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
   for (int j=0;j<4;j++) {
    digitalWrite(ABCDPins[j],num  & (1<<j)); 
   }
}

void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  static volatile byte state=0;
  static int timer = PWMrefresh;
  static int brightness;
  static int PWMtimeBrightness=PWM_min;
  byte animM;
  byte num;
  
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    timer1_write(PWMrefresh);
    return;  
  }
  animM = 0;
  if (animMask[pos] > 0) animM = animMask[pos]; 
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety
  
  if ((brightness ==0) || !radarON){
    writeBits(0xA);  //black display
    
  #if DECIMALPOINT_PIN>=0 
    digitalWrite(DECIMALPOINT_PIN,LOW);
  #endif  
  #if COLON_PIN >= 0 
    digitalWrite(COLON_PIN,LOW);  // colon pin OFF
  #endif
  timer1_write(100*PWMrefresh);
  return;
  }
  else {
    digit[4] = 0xA;  //empty digit
      if (state==0) {
        if (animMask[pos]==0) num = digit[pos];
        else num = oldDigit[pos];
      }  
      else if (state==1) num = newDigit[pos];
      else num = 0xF;
      
  writeBits(num); 
  switch (state) {   //state machine...
    case 0:
      #if DECIMALPOINT_PIN>=0
        if (decimalpointON) digitalWrite(DECIMALPOINT_PIN,HIGH);
      #endif  
      #if COLON_PIN>=0
        digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
      #endif  
      if (autoBrightness && displayON) {
          PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
      }
      else {
        PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);
      }      
      if (animM > 0) { //Animation?
        timer =  (PWMtimeBrightness * (20-animM))/20;
        state = 1;  //next state is: show newDigits
      }
      else {
        timer = PWMtimeBrightness;  
        if (brightness>=MAXBRIGHTNESS) state = 0;
        else state = 2;  //default next state is: BLANK display
      }
      break;
    case 1:  //show new character, if animation
      timer = (PWMtimeBrightness * animM)/20;      
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
      timer = PWMrefresh-PWMtimeBrightness;
      break;
  }  //end switch
  } //end else
    
  if ((brightness == 0) || (!radarON)) {timer = PWM_max; state = 0;}  //no time sharing is needed
  if (timer<500) timer = 500;  //safety only...
  timer1_write(timer); 
}

void Adelay(unsigned long d) {
  unsigned long dStart = millis();

  doAnimationMakuna();
  while ((millis() - dStart) < d) {
    if (WiFi.getMode() == 2) dnsServer.processNextRequest();
    alarmSound();
    #ifdef USE_GPS
      smartDelay(1);  //feed GPS
    #endif  
    yield();
  }
}

void makeAnim() {
  static byte oldDig = 10;

  if (cathodeProtRunning) {
    memset(animMask,0,sizeof(animMask));
    digit[pos] = newDigit[pos];
    return;
        
  }
  else if ((prm.animMode==0) || ipShowRunning ){
    digit[pos] = newDigit[pos];
    memset(animMask,0,sizeof(animMask));
    Adelay(ANIMSPEED*20);
    return;
  }

  //Time showing
  if (oldDig == newDigit[pos]) oldDig = 10; //if the old digit == new digit, use space instead
  oldDigit[pos] = oldDig;  //last displayed digit
  //DPRINT(oldDigit[pos]); DPRINT("/"); DPRINTLN(newDigit[pos]);
  for (int i = 1; i < 20; i++) {
    animMask[pos] = i;  
    Adelay(ANIMSPEED);
  }  
  oldDig = newDigit[pos];  //save last displayed digit
  digit[pos] = newDigit[pos];
  animMask[pos] = 0;
  Adelay(5*ANIMSPEED);
  if (pos==2) {
    digit[pos] = 10;
    oldDigit[pos] = 10;
    oldDig = 10;
    Adelay(20*ANIMSPEED);
  }
}

void writeDisplaySingle() {
  if (cathodeProtRunning) {
    pos = 3;
  }
}

void writeDisplay2(void){  
  static unsigned long lastShow = 0;

  if (cathodeProtRunning) {  //show cathode protect, only one digit
    pos = 3;
    return;
    }
  
  if (ipShowRunning) {    //show 3digit IP address segment
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

  if ((millis()-lastShow)<3000) return;   //show time 
  for (int i=3;i>=0;i--) {
      pos = i;
      makeAnim();
      if (prm.animMode==0) {  //make some pause
        digit[pos] = 10;
        Adelay((i==2)? 400 : 200);
      }
  }
  newDigit[4] = 10;
  pos = 4;
  makeAnim();
  memcpy(digit,newDigit,sizeof(digit));
  memcpy(oldDigit,newDigit,sizeof(oldDigit));
  memset(animMask,0,sizeof(animMask));
  lastShow = millis();
}

void clearTubes() {}
#endif

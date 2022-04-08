#ifdef TUBE1CLOCK
char tubeDriver[] = "1TUBE";
//used by UNFI 1TUBE Clock

//byte ABCDPins[4] =  {2,5,4,15};  //defined in clocks.h

int maxDigits = 4;
int pos = 0;  //pointer of show phase

int PWMrefresh=5500;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
int PWM_min = 500;
int PWM_max = 5000;
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

void makeAnim() {
  static byte oldDig = 10;
  
  if (oldDig == newDigit[pos]) oldDig = 10; //if the old digit == new digit, use space instead
  oldDigit[pos] = oldDig;  //last displayed digit
  
  for (int i = 1; i < 20; i++) {
    animMask[pos] = i;  
    Fdelay(ANIMSPEED);
  }  
  oldDig = newDigit[pos];  //save last displayed digit
  digit[pos] = newDigit[pos];
  animMask[pos] = 0;
  Fdelay(400);
}

void writeDisplaySingle() { 
  if (cathodeProtRunning) {
    pos = 3;
    makeAnim();
  }
  else if (ipShowRunning) {
    for (int i=2;i>=0;i--) {
      pos = i;
      makeAnim();
    }
    pos = 4;
    newDigit[4] = 10;
    makeAnim();
  }
  else {
    for (int i=3;i>=0;i--) {
      pos = i;
      makeAnim();
    }
    pos = 4;
    newDigit[4] = 10;
    makeAnim();
    Fdelay(2000);  
  }

  memset(animMask, 0, sizeof(animMask));
  memcpy(digit, newDigit, sizeof(digit));
  memcpy(oldDigit, newDigit, sizeof(oldDigit)); 
}
  
void clearTubes() {}
#endif

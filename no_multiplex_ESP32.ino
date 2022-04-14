// 4digit Nixie Clock 
//  2x 74HC595N shift register + 4x 74141

#ifdef NO_MULTIPLEX_ESP32

char tubeDriver[] = "NO_MULTIPLEX_ESP32";
#if defined(ESP32) 
#else
  #error "Only ESP32 Board is supported!"  
#endif

 //change it, if needed for the correct tube sequence
//byte tubes[] = {3,2,1,0};         //4 tubes,   old clock...        use in clocks.h
//byte tubes[] = {5,4,3,2,1,0};   //6 tubes, reverse order            use in clocks.h

int maxDigits = sizeof(tubes);
int DRAM_ATTR PWMrefresh=25000;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
int DRAM_ATTR PWM_min = 5000;
int DRAM_ATTR PWM_max = 25000;

//#define dataPin  14  //D5
//#define latchPin 27 //12  //D6
//#define clkPin   26 //13  //D7

void IRAM_ATTR writeDisplaySingle() { }   

void setup_pins(){
  DPRINTLN("Nixie clock - setup pins NON-MULTIPLEX 4(or 6) x 74141");
  pinMode(dataPin, OUTPUT); regPin(dataPin,"dataPin"); 
  pinMode(latchPin,OUTPUT); regPin(latchPin,"latchPin"); 
  pinMode(clkPin,OUTPUT);   regPin(clkPin,"clkPin"); 
  startTimer();
}

void ICACHE_RAM_ATTR writeBits(byte num) {   //shift out 4 bits
  for (int i=3;i>=0;i--) {
    digitalWrite(dataPin,num  & (1<<i)); 
    for (int t=0; t<10;t++) asm volatile ("nop");   //100nsec
    digitalWrite(clkPin,HIGH);
    for (int t=0; t<10;t++) asm volatile ("nop");   //100nsec
    digitalWrite(clkPin,LOW);
    for (int t=0; t<10;t++) asm volatile ("nop");   //100nsec
    }
}

void IRAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  static DRAM_ATTR volatile byte state=0;
  static DRAM_ATTR int timer = PWMrefresh;
  static DRAM_ATTR int brightness;
  static DRAM_ATTR int PWMtimeBrightness=PWM_min;
  byte animM;
  
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    return;  
  }
  
  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();

  animM = 0;
  for (int i=0;i<maxDigits;i++) {
    if (animMask[i] > 0) animM = animMask[i];    //find, if any animation is wanted?
  }

  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety
  
  digitalWrite(latchPin, LOW);

  if ((brightness ==0) || !radarON) {
    for (int i=0;i<maxDigits;i++) writeBits(0xA);  //black display
    
  #if DECIMALPOINT_PIN>=0 
    digitalWrite(DECIMALPOINT_PIN,LOW);
  #endif  
  #if COLON_PIN >= 0 
    digitalWrite(COLON_PIN,LOW);  // colon pin OFF
  #endif
  digitalWrite(latchPin, HIGH);
  portEXIT_CRITICAL_ISR(&timerMux);
  timerAlarmWrite(ESP32timer, 100*PWMrefresh, true);   
  timerAlarmEnable(ESP32timer);
  interrupts();
  return;
  }
  else {
    for (int i=0;i<maxDigits;i++) {
      if (state==0) {
        if (animMask[i]==0) writeBits(digit[tubes[i]]);
        else writeBits(oldDigit[tubes[i]]);
      }  
      else if (state==1) writeBits(newDigit[tubes[i]]);
      else writeBits(0xF);
    } //end for
   
  //for (int i=0;i<maxDigits;i++) writeBits(digit[tubes[i]]);   //simple test, without animation
  
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
    
  digitalWrite(latchPin, HIGH);    
  
  if ((brightness == 0) || (!radarON)) {timer = PWM_max; state = 0;}  //no time sharing is needed
  if (timer<500) timer = 500;  //safety only...
  portEXIT_CRITICAL_ISR(&timerMux);
  timerAlarmWrite(ESP32timer, timer, true);   
  timerAlarmEnable(ESP32timer);
  interrupts();
}

void IRAM_ATTR clearTubes() {}
#endif

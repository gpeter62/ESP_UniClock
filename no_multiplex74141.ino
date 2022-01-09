// 4digit Nixie Clock 
//  2x 74HC595N shift register + 4x 74141

#ifdef NO_MULTIPLEX74141

char tubeDriver[] = "NO_MULTIPLEX74141";
#if defined(ESP8266) 
#else
  #error "Only 8266 Board is supported!"  
#endif

 //change it, if needed for the correct tube sequence
byte tubes[] = {3,2,1,0};         //4 tubes,   old clock...     
//byte tubes[] = {5,4,3,2,1,0};   //6 tubes, reverse order

int maxDigits = sizeof(tubes);
int PWMrefresh=12000;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
int PWM_min = 1000;
int PWM_max = 12000;
//int PWMtiming[11] = {1000,1000,2000,3000,4000,5000,6000,7000,8000,10000,12000};

#define dataPin  14  //D5
#define latchPin 12  //D6
#define clkPin   13  //D7

void writeDisplaySingle() { }   

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

void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  static volatile byte state=0;
  static int timer = PWMrefresh;
  static int brightness;
  static int PWMtimeBrightness=PWM_min;
  byte animM;
  
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    timer1_write(PWMrefresh);
    return;  
  }

  animM = 0;
  for (int i=0;i<maxDigits;i++) {
    if (animMask[i] > 0) animM = animMask[i];    //find, if any animation is wanted?
  }

  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety
  
  digitalWrite(latchPin, LOW);

  if (brightness ==0) {
    for (int i=0;i<maxDigits;i++) writeBits(0xA);  //black display
    
  #if DECIMALPOINT_PIN>=0 
    digitalWrite(DECIMALPOINT_PIN,LOW);
  #endif  
  #if COLON_PIN >= 0 
    digitalWrite(COLON_PIN,LOW);  // colon pin OFF
  #endif
  digitalWrite(latchPin, HIGH);
  timer1_write(100*PWMrefresh);
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
  timer1_write(timer); 
}

void clearTubes() {}
#endif

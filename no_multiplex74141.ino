// 4digit Nixie Clock (original version with OLED display)
//  2x 74HC595N shift register + 4x 74141

#ifdef NO_MULTIPLEX74141

#define MAXBRIGHT 10

 //change it, if needed for the correct tube sequence
byte tubes[] = {3,2,1,0};         //4 tubes,   old OLED clock...     
//byte tubes[] = {5,4,3,2,1,0};   //6 tubes, reverse order

int maxDigits = sizeof(tubes);
const int PWMrefresh=10000;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
const int PWMtiming[] = {2000,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000};

#define dataPin  14  //D5
#define latchPin 12  //D6
#define clkPin 13    //D7

void writeDisplaySingle() {  
}   

void setup_pins(){
 #if defined(ESP8266) 
#else
  #error "Board is not supported!"  
#endif

  DPRINTLN("Setup pins...");
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin,OUTPUT);
  pinMode(clkPin,OUTPUT);
  timer1_attachInterrupt(writeDisplay);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(PWMrefresh); 
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
  static byte brightness;
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
  if (brightness>MAXBRIGHT) brightness = MAXBRIGHT;  //only for safety
  
  digitalWrite(latchPin, LOW);

  if (brightness ==0) {
    for (int i=0;i<maxDigits;i+=2) writeBits(0xF);  //black display
    if (DECIMALPOINT_PIN>=0) digitalWrite(DECIMALPOINT_PIN,LOW);
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
      if ((DECIMALPOINT_PIN>=0) && decimalpointON) digitalWrite(DECIMALPOINT_PIN,HIGH);
      if (COLON_PIN>=0) digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
      if (animM > 0) { //Animation?
        timer =  (PWMtiming[brightness] * (10-animM))/10;
        state = 1;  //next state is: show newDigits
      }
      else {
        timer = PWMtiming[brightness];  
        if (brightness>=MAXBRIGHT) state = 0;
        else state = 2;  //default next state is: BLANK display
      }
      break;
    case 1:  //show new character, if animation
      timer = (PWMtiming[brightness] * animM)/10;      
      if (brightness>=MAXBRIGHT) state = 0;
      else state = 2;  //default next state is: BLANK display
      break;
    case 2:  //blank display
      if (DECIMALPOINT_PIN>=0) digitalWrite(DECIMALPOINT_PIN,LOW);
      if (COLON_PIN>=0) digitalWrite(COLON_PIN,LOW);  // Blink colon pin
      state = 0;
      timer = PWMrefresh-PWMtiming[brightness];
      break;
  }  //end switch
  } //end else
    
  digitalWrite(latchPin, HIGH);    
  
  if (brightness == 0) {timer = PWMtiming[10]; state = 0;}  //no time sharing is needed
  if (timer<2000) timer = 2000;  //safety only...
  timer1_write(timer); 
}

/* //-----  OLD program version ------
   
void ICACHE_RAM_ATTR writeBits(byte num) {
  for (int i=7;i>=0;i--) {
    digitalWrite(dataPin,num  & (1<<i)); 
    for (int t=0; t<2;t++) asm volatile ("nop");   //100nsec
    digitalWrite(clkPin,HIGH);
    for (int t=0; t<2;t++) asm volatile ("nop");   //100nsec
    digitalWrite(clkPin,LOW);
    for (int t=0; t<2;t++) asm volatile ("nop");   //100nsec
    }
}

void ICACHE_RAM_ATTR writeDisplay() {
static volatile int brightCounter = 1;
static volatile boolean oldState = true;
byte brightness;

  digitalWrite(latchPin, LOW);
  for (int i=0;i<maxDigits;i+=2) {
    if ((displayON ?  prm.dayBright : prm.nightBright) < brightCounter)
      writeBits(0xFF);  //BLANK display
    else  
      writeBits((digit[tubes[i+1]]<<4) + digit[tubes[i]]);

   digitalWrite(latchPin, HIGH);     
  }
  
  brightCounter++; 
  if (brightCounter>MAXBRIGHTNESS) brightCounter = 1; 
  timer1_write(PWMrefresh);  
}   
*/
#endif

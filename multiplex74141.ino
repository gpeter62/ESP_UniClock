#ifdef MULTIPLEX74141
//define here the digit enable pins from 4 to 8

//const byte digitEnablePins[] = {14,12,13,15};   //IN16 clock
//const byte ABCDPins[4] =  {2,4,5,0};   

const byte digitEnablePins[] = {13,12,14,15};    //red tube nixie clock
const byte ABCDPins[4] = {16,5,4,0};

int maxDigits = sizeof(digitEnablePins);

//const byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, is needed (for example: bad tube pin layout)
const int PWMrefresh=12000;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
const int PWMtiming[] = {1000,2000,3500,5000,6000,7000,8000,9000,10000,11000,12000};
#define MAXBRIGHT 10

void setup_pins() {
  DPRINTLN("Setup pins...");
  for (int i=0;i<maxDigits;i++) pinMode(digitEnablePins[i], OUTPUT);
  for (int i=0;i<4;i++) pinMode(ABCDPins[i], OUTPUT);
  timer1_attachInterrupt(writeDisplay);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(PWMrefresh); 
}


void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  static byte pos = 0;
  static volatile byte state=0;
  static int timer = PWMrefresh;
  static byte num,brightness;

  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHT) brightness = MAXBRIGHT;  //only for safety

  switch (state) {   //state machine...
    case 0:
      digitalWrite(digitEnablePins[pos],LOW);     //switch off old digit

      pos++;  if (pos>maxDigits-1) {  pos = 0; }  //go to the first tube
      for (int i=0;i<1000;i++) {asm volatile ("nop"); }   //long delay to switch off the old digit before switch on the new, depends on hardware
      digitalWrite(digitEnablePins[pos],HIGH);    //switch on the new digit
      state = 2;  //default next state is: BLANK display
      if (animMask[pos] > 0) { //Animation?
        num =   oldDigit[pos];  //show old character
        timer =  (PWMtiming[brightness] * (10-animMask[pos]))/10;
        state = 1;  //next state is: show newDigit
      }
      else {
        num = digit[pos];
        timer = PWMtiming[brightness];  
        if (brightness==MAXBRIGHT) state = 0;
      }
      break;
    case 1:  //show new character, if animation
      num =   newDigit[pos];
      timer = (PWMtiming[brightness] * animMask[pos])/10;      
      if (brightness>=MAXBRIGHT) state = 0;
      else state = 2;  //default next state is: BLANK display
      break;
    case 2:  //blank display
      num = 10;  //blank character
      state = 0;
      timer = PWMrefresh-PWMtiming[brightness];
      break;
  }

  //if ((pos>0) && (num<=9)) num = convert[num];   //tube character conversion, if needed... (maybe bad pin numbering)
  
  if (timer<200) timer = 200;  //safety only...
  if (brightness == 0) {num = 10; timer = PWMtiming[10]; state = 0;}
  for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }
  
  if (COLON_PIN>=0) {
    if (num==10) digitalWrite(COLON_PIN,LOW);      // Colon pin OFF
    else digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
  }
  
  if (DECIMALPOINT_PIN>=0) {
        if (num==10) {
          digitalWrite(DECIMALPOINT_PIN,LOW);
        }
        else {
          digitalWrite(DECIMALPOINT_PIN,HIGH);
        }
  }
  timer1_write(timer); 
}

void writeDisplaySingle() {}
#endif

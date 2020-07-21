#ifdef MULTIPLEX74141
//define here the digit enable pins from 4 to 8

byte digitEnablePins[] = {14,12,13,15};   //IN16 clock
byte ABCDPins[4] =  {2,4,5,0};   

//byte digitEnablePins[] = {13,12,14,15};    //red tube nixie clock
//byte ABCDPins[4] = {16,5,4,0};

int maxDigits = sizeof(digitEnablePins);

//const byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, is needed (for example: bad tube pin layout)
int PWMrefresh=2000;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
//const int tubeTime[] = {2000,2000,2000,2000,2000,2000,2000,2000,2000};  //ticks to stay on the same digit to compensate different digit brightness

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
  static byte oldPos = maxDigits-1;
  static volatile int brightCounter = 1;
  byte num,brightness;
  
  brightness = displayON ?  prm.dayBright : prm.nightBright;
 
  if ((animMask[pos] == 0) || (brightness<6)) //no animation, if brightness is below 6
      num = digit[pos];    
  else {                  //transition from old to new character
      num =  (animCounter[pos]>=animMask[pos]) ? oldDigit[pos]: newDigit[pos];
      animCounter[pos]++; if (animCounter[pos]>10) animCounter[pos]=1;
  }
  //if ((pos>0) && (num<=9)) num = convert[num];   //tube character conversion, if needed... (maybe bad pin numbering)
  
  if (brightness<brightCounter)  //dark tube or display digit?
    num = 10;  //blank character
  
  digitalWrite(digitEnablePins[oldPos],LOW);   //switch off old digit
  for (int i=0;i<1500;i++) {asm volatile ("nop"); }   //long delay to switch off the old digit before switch on the new, depends on hardware
  for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }

  digitalWrite(digitEnablePins[pos],HIGH);    //switch on the new digit
  oldPos = pos;
  pos++; 
  if (pos>maxDigits-1) {   //go to the first tube
    pos = 0;  
    brightCounter++; 
    if (brightCounter>MAXBRIGHTNESS) brightCounter = 1;
  }  
  timer1_write(PWMrefresh);   //constand timing
  //timer1_write(tubeTime[oldPos]);   //tube dependent timing
}

void writeDisplaySingle() {}
#endif

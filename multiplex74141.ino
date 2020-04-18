#ifdef MULTIPLEX74141

byte digitEnablePins[] = {13,12,14,15};    //define here the digit enable pins from 4 to 8
byte ABCDPins[4] = {16,5,4,0};
int maxDigits = sizeof(digitEnablePins);
int PWMrefresh=2000;   ////msec, Multiplex time period. Greater value => slower multiplex frequency

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
  num = digit[pos]; 
  
  if (brightness<brightCounter)  num = 10;
  digitalWrite(digitEnablePins[oldPos],LOW);   //switch off old digit

  for (int i=0;i<1000;i++) {asm volatile ("nop"); }   //long delay to switch off the old digits
  for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }

  digitalWrite(digitEnablePins[pos],HIGH);    //switch on the new digit
  oldPos = pos;
  pos++; 
  if (pos>maxDigits-1) {   //go to the first tube
    pos = 0;  
    brightCounter++; 
    if (brightCounter>MAXBRIGHTNESS) brightCounter = 1;
  }  
  timer1_write(PWMrefresh);
}

void writeDisplaySingle() {}
#endif

#ifdef Numitron_4511N
//used by GP Numitron v3 panel

#define LTBIpin 5
byte digitEnablePins[] = {13,12,14,16};    //define here the digit enable pins from 4 to 8
byte ABCDPins[4] = {4,0,2,15};
int maxDigits = sizeof(digitEnablePins);
int PWMrefresh=5000;   ////kb.1000Hz, Multiplex time period. Greater value => slower multiplex frequency

void setup_pins() {
  DPRINTLN("Setup pins...");
  pinMode(LTBIpin, OUTPUT);
  digitalWrite(LTBIpin,HIGH);
  for (int i=0;i<maxDigits;i++) pinMode(digitEnablePins[i], OUTPUT);
  for (int i=0;i<4;i++) pinMode(ABCDPins[i], OUTPUT);
  timer1_attachInterrupt(writeDisplay);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(PWMrefresh); 
}


void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
static volatile int brightCounter = 1;
byte num;
  
  for (int pos=0; pos<=maxDigits; pos++) {
    num = digit[pos]; 
    if ((displayON ?  prm.dayBright : prm.nightBright)<brightCounter)  num = 10;
    digitalWrite(digitEnablePins[pos],LOW);   
    for (int j=0;j<4;j++) {digitalWrite(ABCDPins[j],num  & (1<<j)); }
    digitalWrite(digitEnablePins[pos],HIGH);    
  }
  
  brightCounter++; 
  if (brightCounter>MAXBRIGHTNESS) brightCounter = 1;
  
  if (colonPin>=0) digitalWrite(colonPin,!colonBlinkState);  // Blink colon pin
  timer1_write(PWMrefresh);
}

void writeDisplaySingle() {}
#endif

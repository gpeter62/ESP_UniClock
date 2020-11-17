#ifdef Numitron_4511N
//used by GP Numitron v3 panel
//Flash size: 1MB (FS:160k, OTA:422k)

byte panelVersion = 3;   //1,2 or 3

#define LTBIpin 5
byte digitEnablePins[] = {13,12,14,16};    //define here the digit enable pins from 4 to 8
byte ABCDPins[4] = {4,0,2,15};

const int maxDigits = sizeof(digitEnablePins);
int PWMrefresh=5000;   //Multiplex time period. Greater value => slower multiplex frequency

void setup_pins() {
  #if defined(ESP8266) 
  #else
    #error "Board is not supported!"  
  #endif

  DPRINTLN("Setup pins...");
  pinMode(LTBIpin, OUTPUT);
  digitalWrite(LTBIpin,HIGH);
  for (int i=0;i<maxDigits;i++) pinMode(digitEnablePins[i], OUTPUT);
  for (int i=0;i<4;i++) pinMode(ABCDPins[i], OUTPUT);
  startTimer();
}


void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
static volatile int brightCounter = 1;
byte num;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    timer1_write(PWMrefresh);
    return;  
  }
  
  if ((panelVersion==3) && ((displayON ?  prm.dayBright : prm.nightBright)<brightCounter)) {
    digitalWrite(LTBIpin,LOW);
  }
  else { 
    digitalWrite(LTBIpin,HIGH);
    for (int pos=0; pos<=maxDigits; pos++) {
      num = digit[pos]; 
      if ((displayON ?  prm.dayBright : prm.nightBright)<brightCounter)  num = 10;   //clear digit
      digitalWrite(digitEnablePins[pos],LOW);   
      for (int j=0;j<4;j++) {digitalWrite(ABCDPins[j],num  & (1<<j)); }
      digitalWrite(digitEnablePins[pos],HIGH);    
    } //end for
  } //end else
  brightCounter++; 
  if (brightCounter>MAXBRIGHTNESS) brightCounter = 1;
  
  if (COLON_PIN>=0) digitalWrite(COLON_PIN,!colonBlinkState);  // Blink colon pin
  timer1_write(PWMrefresh);
}

void clearTubes() {
/*  //not necessary to use
  if (panelVersion==3) {
    digitalWrite(LTBIpin,LOW);
  }
  else {
  for (int pos=0; pos<=maxDigits; pos++) {
    digitalWrite(digitEnablePins[pos],LOW);
    for (int i=0;i<4;i++) digitalWrite(ABCDPins[i],HIGH); 
    digitalWrite(digitEnablePins[pos],HIGH); 
  }
  }
*/  
}

void writeDisplaySingle() {}
#endif

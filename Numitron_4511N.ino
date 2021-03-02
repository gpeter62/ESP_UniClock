#ifdef Numitron_4511N
//used by GP Numitron v3 panel
//Flash size: 1MB (FS:160k, OTA:422k)

#ifdef PCB_VERSION
  byte panelVersion = PCB_VERSION;
#else  
  byte panelVersion = 1;
#endif

//#define LTBIpin 5
//byte digitEnablePins[] = {13,12,14,16};    //define here the digit enable pins from 4 to 8
//byte ABCDPins[4] = {4,0,2,15};

const int maxDigits = sizeof(digitEnablePins);
int PWMrefresh=5000;   //Multiplex time period. Greater value => slower multiplex frequency

void setup_pins() {
  DPRINTLN("Numitron Clock - setup pins");
  pinMode(LTBIpin, OUTPUT);  regPin(LTBIpin,"LTBIpin");
  digitalWrite(LTBIpin,HIGH);
  for (int i=0;i<maxDigits;i++) {
    pinMode(digitEnablePins[i], OUTPUT);
    regPin(digitEnablePins[i],"digitEnablePins");
  }
  for (int i=0;i<4;i++) {
    pinMode(ABCDPins[i], OUTPUT);
    regPin(ABCDPins[i],"ABCDPins");
  }
  startTimer();
}

#if defined(ESP32)
void IRAM_ATTR writeDisplay() { //void IRAM_ATTR  writeDisplay(){
#else
void ICACHE_RAM_ATTR writeDisplay() {       //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
#endif
static volatile int timer = PWMrefresh;
static volatile int brightCounter = 1;
static volatile int dpCounter = 0; //decimal point stepper
byte num;

  intCounter++;
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    #if defined(ESP8266)
      timer1_write(PWMrefresh);
    #endif  
    return;
  }
  
  #if defined(ESP32)
    portENTER_CRITICAL_ISR(&timerMux);
    noInterrupts();
  #endif
  
  if ((panelVersion==3) && ((displayON ?  prm.dayBright : prm.nightBright)<brightCounter)) {
    digitalWrite(LTBIpin,LOW);   //Blank display
  }
  else { 
    digitalWrite(LTBIpin,HIGH); //enable display
    for (int pos=0; pos<=maxDigits; pos++) {
      num = digit[pos]; 
      if ((displayON ?  prm.dayBright : prm.nightBright)<brightCounter)  num = 10;   //clear digit
      digitalWrite(digitEnablePins[pos],LOW);   //latch enable 
      for (int j=0;j<4;j++) {digitalWrite(ABCDPins[j],num  & (1<<j)); }
      digitalWrite(digitEnablePins[pos],HIGH);    
    } //end for
  } //end else
  brightCounter++;   if (brightCounter>MAXBRIGHTNESS) brightCounter = 1;
  if (brightCounter == 1) {
    dpCounter++;   if (dpCounter>maxDigits) dpCounter = 0;
    #if DP_PIN>=0
      digitalWrite(DP_PIN,digitDP[dpCounter]); 
    #endif
  } 
  #if COLON_PIN >=0  
    digitalWrite(COLON_PIN,!colonBlinkState);  // Blink colon pin
  #endif  
  
  #if defined(ESP8266)
    timer1_write(timer);
  #elif defined(ESP32)
    portEXIT_CRITICAL_ISR(&timerMux);
    //ESP32timer = timerBegin(0, PRESCALER, true);  //set prescaler, true = edge generated signal
    //timerAttachInterrupt(ESP32timer, &writeDisplay, true);   
    timerAlarmWrite(ESP32timer, timer, true);   
    timerAlarmEnable(ESP32timer);
    interrupts();
  #endif
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

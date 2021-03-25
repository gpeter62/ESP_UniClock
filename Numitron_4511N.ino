#ifdef Numitron_4511N
//used by GP Numitron v3 panel

//#define LTBIpin 5
//byte digitEnablePins[] = {13,12,14,16};    //define here the digit enable pins from 4 to 8
//byte ABCDPins[4] = {4,0,2,15};

const int maxDigits = sizeof(digitEnablePins);
#if defined(ESP32)
  uint32_t DRAM_ATTR digitEnableBits[10];
  #define MAXBRIGHT 10
  int DRAM_ATTR PWMrefresh=5500;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
  //int DRAM_ATTR PWMtiming[11] = {0,500,800,1200,2000,2500,3000,3500,4000,4500,5000};
  int DRAM_ATTR maxDig;
  int DRAM_ATTR PWM_min = 500;
  int DRAM_ATTR PWM_max = 5000;  
#else
  uint32_t digitEnableBits[10];
  #define MAXBRIGHT 10
  int PWMrefresh=5500;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
  int PWM_min = 500;
  int PWM_max = 5000;
  //int PWMtiming[11] = {0,500,800,1200,2000,2500,3000,3500,4000,4500,5000};
  int maxDig;
#endif

void setup_pins() {
  DPRINTLN("Numitron Clock - setup pins");
  maxDig = maxDigits;  //put const to memory var
  pinMode(LTBIpin, OUTPUT);  regPin(LTBIpin,"LTBIpin");
  digitalWrite(LTBIpin,HIGH);
  for (int i=0;i<maxDig;i++) {
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
  static DRAM_ATTR volatile int timer = PWMrefresh;
  static DRAM_ATTR volatile int brightCounter = 1;
  static DRAM_ATTR volatile int dpCounter = 0; //decimal point stepper
  static DRAM_ATTR boolean state=true;
  int PWMtimeBrightness;
  int brightness;
  int num;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    return;
  }
  
  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();
  intCounter++;
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  if (autoBrightness && (displayON )) {   
    PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
    }
  else
    PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);
  
  if ((!autoBrightness) && (brightness==MAXBRIGHTNESS))  
    state = true;
  
  if (state) {  //ON state
    timer = PWMtimeBrightness;
    timerON = timer;
    timerOFF = PWMrefresh-PWMtimeBrightness; 
  }
  else {  //OFF state
    timer = PWMrefresh-PWMtimeBrightness;
  }
  if (timer<500) timer = 500;  //safety only...
  if ( (brightness == 0) || (!state) || (!radarON)) {  //OFF state, blank digit
        digitalWrite(LTBIpin,LOW); //disable display
        #if COLON_PIN >=0  
          digitalWrite(COLON_PIN,!colonBlinkState);  // Blink colon pin
        #endif
    }
  else {  //ON state
        digitalWrite(LTBIpin,HIGH); //enable display
        #if COLON_PIN >=0  
          digitalWrite(COLON_PIN,LOW);
        #endif
  }
  state = !state;  
  portEXIT_CRITICAL_ISR(&timerMux);
  timerAlarmWrite(ESP32timer, timer, true);   
  timerAlarmEnable(ESP32timer);
  interrupts();
}

void IRAM_ATTR writeDisplaySingle() {
  int num;

  for (int pos=0; pos<=maxDig; pos++) {
    num = digit[pos]; 
    digitalWrite(digitEnablePins[pos],LOW);   //latch enable 
    for (int j=0;j<4;j++) {digitalWrite(ABCDPins[j],num  & (1<<j)); }
    digitalWrite(digitEnablePins[pos],HIGH);    
  } //end for}
}
//__________________ ESP8266 driver __________________________
#else   
#ifdef PCB_VERSION
  byte panelVersion = PCB_VERSION;
#else  
  byte panelVersion = 1;
#endif

void ICACHE_RAM_ATTR writeDisplay() {       //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  static volatile int timer = PWMrefresh;
  static volatile int brightCounter = 1;
  static volatile int dpCounter = 0; //decimal point stepper
  int num;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
      timer1_write(PWMrefresh);
  return;
  }
  
  intCounter++;
  if (((panelVersion>=3) && ((displayON ?  prm.dayBright : prm.nightBright)<brightCounter)) || (!radarON)) {
    digitalWrite(LTBIpin,LOW);   //Blank display
  }
  else { 
    digitalWrite(LTBIpin,HIGH); //enable display
    for (int pos=0; pos<=maxDig; pos++) {
      num = digit[pos]; 
      if ((displayON ?  prm.dayBright : prm.nightBright)<brightCounter)  num = 10;   //clear digit
      digitalWrite(digitEnablePins[pos],LOW);   //latch enable 
      for (int j=0;j<4;j++) {digitalWrite(ABCDPins[j],num  & (1<<j)); }
      digitalWrite(digitEnablePins[pos],HIGH);    
    } //end for
  } //end else
  brightCounter++;   if (brightCounter>MAXBRIGHTNESS) brightCounter = 1;
  if (brightCounter == 1) {
    dpCounter++;   if (dpCounter>maxDig) dpCounter = 0;
    #if defined(DP_PIN) && (DP_PIN>=0)
      digitalWrite(DP_PIN,digitDP[dpCounter]); 
    #endif
  } 
  #if COLON_PIN >=0  
    digitalWrite(COLON_PIN,!colonBlinkState);  // Blink colon pin
  #endif  
  
  timer1_write(timer);
}

void writeDisplaySingle() {}
#endif  //esp8266 end

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


#endif

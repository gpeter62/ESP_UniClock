#ifdef MULTIPLEX74141
//define here the digit enable pins from 4 to 8

//const byte digitEnablePins[] = {4,16,17,5,18,19};   //ESP32 6x tube Clock
//const byte ABCDPins[4] =  {12,27,14,13};   

const byte digitEnablePins[] = {14,12,13,15};   //IN16 4x tube clock
const byte ABCDPins[4] =  {2,4,5,0};   

//const byte digitEnablePins[] = {15,13,12,14};   //PinterS clock
//const byte ABCDPins[4] =  {2,4,5,0}; 

//const byte digitEnablePins[] = {15,13,12,14};   //PinterS thermo
//const byte ABCDPins[4] =  {2,4,5,0};  

//const byte digitEnablePins[] = {13,12,14,15};    //red 4x tube nixie clock
//const byte ABCDPins[4] = {16,5,4,0};

const int maxDigits = sizeof(digitEnablePins);

//const byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, is needed (for example: bad tube pin layout)
const int PWMrefresh=11000;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
#define MAXBRIGHT 10
const int PWMtiming[MAXBRIGHT+1] = {0,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000};


void setup_pins() {
  DPRINTLN("Setup pins...");
  for (int i=0;i<maxDigits;i++) pinMode(digitEnablePins[i], OUTPUT);
  for (int i=0;i<4;i++) pinMode(ABCDPins[i], OUTPUT);
  startTimer();
}

#if defined(ESP32)
  void IRAM_ATTR  writeDisplay(){
#else 
  void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
#endif
  
  static byte pos = 0;
  static volatile byte state=0;
  static int timer = PWMrefresh;
  static byte num,brightness;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    digitalWrite(digitEnablePins[pos],LOW);
    #if defined(ESP8266)    
      timer1_write(PWMrefresh);
    #elif defined(ESP32)
      timerAlarmWrite(ESP32timer, PWMrefresh, false);
    #endif    
    return;  
  }

  #if defined(ESP32)
    portENTER_CRITICAL_ISR(&timerMux);
  #endif
  
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHT) brightness = MAXBRIGHT;  //only for safety
  timer = PWMrefresh;

   switch (state) {   //state machine...
    case 0:
      pos++;  if (pos>maxDigits-1)  pos = 0;  //go to the first tube
      
      if (animMask[pos] > 0) { //Animation?
        num = oldDigit[pos];  //show old character
        timer = (PWMtiming[brightness] * (10-animMask[pos]))/10;
        state = 1;  //next state is: show newDigit
      }
      else {
        num = digit[pos];  //show active character
        timer = PWMtiming[brightness];  
        state = 2;  //next state is: BLANK display
      }
      break;
    case 1:  //show new character, if animation
      num =   newDigit[pos];
      timer = (PWMtiming[brightness] * animMask[pos])/10;      
      state = 2;  //default next state is: BLANK display
      break;
    case 2:  //blank display
      timer = PWMrefresh-PWMtiming[brightness];
      state = 3;
      break;
   }  //end switch
   if (timer<200) timer = 200;  //safety only...

 
   //  if ((pos>0) && (num<=9)) num = convert[num];   //tube character conversion, if needed... (maybe bad pin numbering)
   

  if ((brightness == 0) || (state == 3) || (num >9)) {  //blank digit
    state = 0; 
    digitalWrite(digitEnablePins[pos],LOW);  //switch off anode 
    }
  else {
      for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }
      digitalWrite(digitEnablePins[pos],HIGH);    //switch on the new digit
  }
    
  if (COLON_PIN>=0) {
    if (num==10) digitalWrite(COLON_PIN,LOW);      // Colon pin OFF
    else digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
  }
  
  if (DECIMALPOINT_PIN>=0) {
        if (num==10) {
          digitalWrite(DECIMALPOINT_PIN,LOW);
        }
        else {
          if (decimalpointON) digitalWrite(DECIMALPOINT_PIN,HIGH);
        }
  }

  #if defined(ESP8266)    
    timer1_write(timer);
  #elif defined(ESP32)     
    timerAlarmWrite(ESP32timer, PWMrefresh, false);
    portEXIT_CRITICAL_ISR(&timerMux);
  #endif    
}

void clearTubes() {
    for (int i=0;i<maxDigits;i++) digitalWrite(digitEnablePins[i],LOW); 
}

void writeDisplaySingle() {}
#endif

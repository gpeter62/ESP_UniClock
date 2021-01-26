#ifdef MULTIPLEX74141_ESP32
//define here the digit enable pins from 4 to 8

//const byte digitEnablePins[] = {15,13,12,14};   //fox example... But SET in clocks.h !!!
//const byte ABCDPins[4] =  {2,4,5,0};
//const int DP_PIN = -1; // decimalPoint inside Nixie tube, set -1, if not used!

const int maxDigits = sizeof(digitEnablePins);
int DRAM_ATTR maxDig = maxDigits;   //memory variable version

//const byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, is needed (for example: bad tube pin layout)
int DRAM_ATTR PWMrefresh=11000;   //msec, Multiplex time period. Greater value => slower multiplex frequency
#define MAXBRIGHT 10
int DRAM_ATTR PWMtiming[MAXBRIGHT+1] = {0,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000};
int DRAM_ATTR PWMrange = PWMtiming[MAXBRIGHT] - PWMtiming[1];

  #if defined(ESP32) 
  #else
    #error "Only ESP32 board is supported by this driver!"  
  #endif

void setup_pins() {
  DPRINTLN("Nixie clock - setup pins -  Multiplex 74141 mode...");
  DPRINT("digitEnablePins: ");
  for (int i=0;i<maxDigits;i++) {
    pinMode(digitEnablePins[i], OUTPUT);
    regPin(digitEnablePins[i],"DIGIT_ENABLE_PIN"); 
  }
  DPRINTLN(" ");
  DPRINT("ABCDPins: ");
  for (int i=0;i<4;i++) {
    pinMode(ABCDPins[i], OUTPUT);
    regPin(ABCDPins[i],"74141_ABCD_PIN"); 
  }
  DPRINTLN(" ");
  #if DP_PIN>=0
    pinMode(DP_PIN,OUTPUT);  
    DPRINT("DP_PIN:"); DPRINTLN(DP_PIN);
  #endif  
  startTimer();
}

void IRAM_ATTR writeDisplay(){  //void IRAM_ATTR  writeDisplay(){

  static DRAM_ATTR byte pos = 0;
  static DRAM_ATTR byte state=0;
  static DRAM_ATTR int timer = PWMrefresh;
  static DRAM_ATTR byte num,brightness;
  static DRAM_ATTR byte DPpos;
  static DRAM_ATTR int PWMtimeBrightness;
  
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    return;  
  }
  
  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();
  
  intCounter++;
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHT) brightness = MAXBRIGHT;  //only for safety

  if (autoBrightness && displayON)
    PWMtimeBrightness = max(PWMtiming[1],PWMtiming[MAXBRIGHT] * LuxValue / MAXIMUM_LUX);
  else
    PWMtimeBrightness = PWMtiming[brightness];
  
  timer = PWMrefresh;

   switch (state) {   //state machine...
    case 0:
      pos++;  if (pos>maxDig-1)  pos = 0;  //go to the first tube
      
      if (animMask[pos] > 0) { //Animation?
        num = oldDigit[pos];  //show old character
        timer = (PWMtimeBrightness * (10-animMask[pos]))/10;
        state = 1;  //next state is: show newDigit
      }
      else {
        num = digit[pos];  //show active character
        timer = PWMtimeBrightness;  
        state = 2;  //next state is: BLANK display
      }
      break;
    case 1:  //show new character, if animation
      num =   newDigit[pos];
      timer = (PWMtimeBrightness * animMask[pos])/10;      
      state = 2;  //default next state is: BLANK display
      break;
    case 2:  //blank display
      timer = PWMrefresh-PWMtimeBrightness;
      state = 3;
      break;
   }  //end switch
   if (timer<500) timer = 500;  //safety only...
 
   //  if ((pos>0) && (num<=9)) num = convert[num];   //tube character conversion, if needed... (maybe bad pin numbering)

  if ((brightness == 0) || (state == 3) || (num >9) || (!radarON)) {  //blank digit
    state = 0; 
    digitalWrite(digitEnablePins[pos],LOW);  //switch off anode 
    #if DP_PIN >=0
      digitalWrite(DP_PIN,LOW);
    #endif  
    }
  else {
      for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }
      digitalWrite(digitEnablePins[pos],HIGH);    //switch on the new digit
      #if DP_PIN >=0
        if (LEFTDECIMAL) DPpos = min(maxDig-1,pos+1); else DPpos = pos;
        if (digitDP[DPpos]) digitalWrite(DP_PIN,HIGH); //switch ON decimal point, if needed
      #endif
  }
    
  #if COLON_PIN >= 0
    if (num==10) digitalWrite(COLON_PIN,LOW);      // Colon pin OFF
    else digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
  #endif
  
  #if DECIMALPOINT_PIN >=0 
        if (num==10) {digitalWrite(DECIMALPOINT_PIN,LOW);}
        else digitalWrite(DECIMALPOINT_PIN,decimalpointON); }
  #endif

  portEXIT_CRITICAL_ISR(&timerMux);
  //ESP32timer = timerBegin(0, PRESCALER, true);  //set prescaler, true = edge generated signal
  //timerAttachInterrupt(ESP32timer, &writeDisplay, true);   
  timerAlarmWrite(ESP32timer, timer, true);   
  timerAlarmEnable(ESP32timer);
  interrupts();
}

void clearTubes() {
  
  for (int i=0;i<maxDig;i++) digitalWrite(digitEnablePins[i],LOW); 
    if (DP_PIN>=0) digitalWrite(DP_PIN,LOW);
}

void writeDisplaySingle() {}
#endif

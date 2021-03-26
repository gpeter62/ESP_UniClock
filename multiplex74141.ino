#ifdef MULTIPLEX74141   //Nixie driver for 8266 only!
#define TUBEDRIVER MULTIPLEX74141
//define here the digit enable pins from 4 to 8
//const byte digitEnablePins[] = {15,13,12,14};   //fox example... But SET in clocks.h !!!
//const byte ABCDPins[4] =  {2,4,5,0};
//#define DP_PIN -1             // decimalPoint inside Nixie tube, set -1, if not used!

#if defined(ESP8266)
#else
  #error "Board is not supported! For ESP32 use MULTIPLEX74141_ESP32 !"  
#endif

const int maxDigits = sizeof(digitEnablePins);
int maxDig = maxDigits;   //memory variable version

//const byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, is needed (for example: bad tube pin layout)
int PWMrefresh=11000;   //msec, Multiplex time period. Greater value => slower multiplex frequency
int PWM_min = 1000;
int PWM_max = 10000;
//int PWMtiming[11] = {0,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000};

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

void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  static byte pos = 0;
  static volatile byte state=0;
  static int timer = PWMrefresh;
  static int num,brightness;
  static byte DPpos;
  static int PWMtimeBrightness=PWM_min;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
      digitalWrite(digitEnablePins[pos],LOW); 
      timer1_write(PWMrefresh);
    return;  
  }
  
  intCounter++;
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  timer = PWMrefresh;

   switch (state) {   //state machine...
    case 0:
      pos++;  if (pos>maxDig-1)  {
        pos = 0;  //go to the first tube
        if (autoBrightness && displayON) {
          PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
        }
        else
          PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);
        }
      
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
        else {digitalWrite(DECIMALPOINT_PIN,decimalpointON); }
  #endif

  timer1_write(timer);
}

void clearTubes() {
      for (int i=0;i<maxDig;i++) digitalWrite(digitEnablePins[i],LOW); 
      #if DP_PIN >= 0 
        digitalWrite(DP_PIN,LOW);
      #endif  
}

void writeDisplaySingle() {}
#endif

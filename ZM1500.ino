// ZM1500 Clock by UNFI
//  4x 74HC595N shift register + optocouplers

#ifdef ZM1500_ESP32

char tubeDriver[] = "ZM1500_ESP32";
#if defined(ESP32) 
#else
  #error "Only ESP32 Board is supported!"  
#endif

int DRAM_ATTR PWMrefresh=11000;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
int DRAM_ATTR PWM_min = 2000;
int DRAM_ATTR PWM_max = 10000;

//#define dataPin  14  //D5
//#define latchPin 27 //12  //D6
//#define clkPin   26 //13  //D7

byte DRAM_ATTR charDefinition[] = {
                   B11111100,   //0: abcdef
                   B01100000,   //1: bc 
                   B11011010,   //2: abdeg
                   B11110010,   //3: abcdg
                   B01100110,   //4: bcfg
                   B10110110,   //5: acdfg
                   B10111110,   //6: acdefg
                   B11100000,   //7: abc
                   B11111110,   //8: abcdefg
                   B11110110,   //9: abcdfg
                   B00000000,   // : BLANK (10)
                   B00000010,   //-: minus (11)
                   B00000001,   // decimal point (12)
                   B11101110,   // A  abcefg (13)
                   B11001110,   // P  abefg (14)
                   B10011100,   // C  adef (15)
                   B11000110,   //grad  (upper circle) abfg (16)
                   B10110100,   //%  acdf  (17)
                   B00111010,   //lower circle cdeg  (18)                  
                   B01100000,   //I  bc    (19)
                   B10001110    //F  aefg  (20)
};

#define MAXCHARS sizeof(charDefinition)
#define MAXSEGMENTS sizeof(segmentEnablePins)
int maxDigits = sizeof(digitEnablePins);

uint32_t DRAM_ATTR charTable[MAXCHARS];    //generated pin table from segmentDefinitions
uint32_t DRAM_ATTR segmentEnableBits[MAXSEGMENTS];   //bitmaps, generated from EnablePins tables
uint32_t DRAM_ATTR animationMaskBits[5];
uint32_t DRAM_ATTR digitEnableBits[12];

void IRAM_ATTR writeDisplaySingle() { }   

void setup_pins(){
  DPRINT("ZM1500 clock ");  DPRINT(maxDigits); DPRINTLN(" digits - setup pins");
  pinMode(dataPin, OUTPUT); regPin(dataPin,"dataPin"); 
  pinMode(latchPin,OUTPUT); regPin(latchPin,"latchPin"); 
  pinMode(clkPin,OUTPUT);   regPin(clkPin,"clkPin"); 
  digitalWrite(latchPin,LOW); 
  digitalWrite(clkPin,LOW);
  generateBitTable();
  startTimer();
}

void IRAM_ATTR writeDisplay(){  //void IRAM_ATTR  writeDisplay(){

  static DRAM_ATTR int pos = maxDigits;
  static DRAM_ATTR boolean state=0;
  static DRAM_ATTR int brightness;
  static DRAM_ATTR uint32_t PWMtimeBrightness=PWM_min;
  uint32_t timer = PWMrefresh;
  uint32_t val = 0;
  
  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();
  intCounter++;
  
  if (state) {  //ON state
    pos++;  
    if (pos>maxDigits-1)  {   //go to the tube#0
      pos = 0; 
      brightness = displayON ? prm.dayBright : prm.nightBright;
      if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety
      if (autoBrightness && displayON) {   //change brightness only on the tube#0
        PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
        }
      else
        PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);
    }  
    val = uint32_t(charTable[digit[pos]]);  //the full bitmap to send to MAX chip
    if (animMask[pos]>0) val &= uint32_t(animationMaskBits[animMask[pos]-1]);  //animationMode 6, mask characters from up to down and back
    if (digitDP[pos]) val = val | uint32_t(charTable[12]);    //Decimal Point
        
    timer = PWMtimeBrightness;
    timerON = timer;   //debug only
    timerOFF = PWMrefresh-PWMtimeBrightness;  //debug only
  }
  else {  //OFF state
    timer = PWMrefresh-PWMtimeBrightness;
    val = 0;
  }
  
  if (timer<500) timer = 500;  //safety only...
  if ( (brightness == 0) || (!radarON))  val = 0; //blank digit
  val = val | digitEnableBits[pos];
  
  for (int i=0; i<20; i++)  {
    if (val & uint32_t(1 << i))   //reverse: (1 << (19 - i))) 
      digitalWrite(dataPin, HIGH);  
    else 
      digitalWrite(dataPin, LOW);    
    digitalWrite(clkPin,HIGH);  //asm volatile ("nop");  //Clock impulse
    digitalWrite(clkPin,LOW);   //asm volatile ("nop");    
  } //end for      

  digitalWrite(latchPin,HIGH);  //asm volatile ("nop");
  digitalWrite(latchPin,LOW);
  state = !state;  
    
  portEXIT_CRITICAL_ISR(&timerMux);   
  timerAlarmWrite(ESP32timer, timer, true);   
  timerAlarmEnable(ESP32timer);
  interrupts();
}


void generateBitTable() {
uint32_t out;

  DPRINTLN("--- Generating segment pins bitmap ---");
  for (int i=0;i<MAXSEGMENTS;i++) {
    segmentEnableBits[i] = uint32_t(1<<segmentEnablePins[i]);
    DPRINT(i); DPRINT(": "); DPRINTLN(segmentEnableBits[i],BIN);
  }
  animationMaskBits[0] = uint32_t(1<<segmentEnablePins[0]);  //a
  animationMaskBits[1] = animationMaskBits[0] | uint32_t(1<<segmentEnablePins[1]) | uint32_t(1<<segmentEnablePins[5]);  //bf
  animationMaskBits[2] = animationMaskBits[1] | uint32_t(1<<segmentEnablePins[6]);  //g
  animationMaskBits[3] = animationMaskBits[2] | uint32_t(1<<segmentEnablePins[4]) | uint32_t(1<<segmentEnablePins[2]);  //ec
  animationMaskBits[4] = animationMaskBits[3] | uint32_t(1<<segmentEnablePins[3]);  //d
  DPRINTLN("--- Animation mask ---");
  for (int i=0;i<5;i++) {
    animationMaskBits[i] = ~animationMaskBits[i]; //invert bits
    DPRINTLN(animationMaskBits[i],HEX);
  }

DPRINTLN("---- Generated Character / Pins table -----");
  for (int i=0;i<MAXCHARS;i++) {
    out = 0;
    DPRINT(i); DPRINT(":  ");
    DPRINT(charDefinition[i],BIN);  DPRINT(" = ");
    for (int j=0;j<=7;j++)   //from a to g
      if ((charDefinition[i] & 1<<(7-j)) != 0) {
        out = out | segmentEnableBits[j]; //DPRINT("1"); 
        }
    //else        DPRINT("0");
    //DPRINT("  >> ");  
    
    charTable[i] = out;
    DPRINTLN(charTable[i],BIN);
  }  //end for

  DPRINTLN("--- Generating digit pins bitmap ---");
  for (int i=0;i<maxDigits;i++) {
    digitEnableBits[i] = uint32_t(1 << uint32_t(digitEnablePins[i]+8));
    DPRINT(i); DPRINT(": "); DPRINT(digitEnableBits[i],HEX); DPRINTLN(" ");
  }

  DPRINTLN("--- DEMO ---");
  for (int i=0;i<maxDigits;i++) {
    uint32_t val = uint32_t(charTable[i]);  //the full bitmap to send to MAX chip
    val = val | digitEnableBits[i];
    DPRINT(i); DPRINT(": "); DPRINTLN(val,HEX);
  }  
}

void IRAM_ATTR clearTubes() {}
#endif

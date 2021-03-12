#ifdef MAX6921_ESP32   
//VFD driver driver for ESP32
  
  #if defined(ESP32) 
  #else
    #error "Only ESP32 board is supported by this driver!"  
  #endif
  
int DRAM_ATTR maxDig;   //memory variable version of maxDigits
//------------------abcdefgDP----------------   definition of different characters
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

uint32_t DRAM_ATTR animationMaskBits[5];

#define MAXCHARS sizeof(charDefinition)
#define MAXSEGMENTS sizeof(segmentEnablePins)
const int maxDigits =  sizeof(digitEnablePins);

uint32_t DRAM_ATTR charTable[MAXCHARS];              //generated pin table from segmentDefinitions
uint32_t DRAM_ATTR segmentEnableBits[MAXSEGMENTS];   //bitmaps, generated from EnablePins tables
uint32_t DRAM_ATTR digitEnableBits[10];

#define MAXBRIGHT 10

int DRAM_ATTR PWMrefresh=5500;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
int DRAM_ATTR PWMtiming[MAXBRIGHT+1] = {0,500,800,1200,2000,2500,3000,3500,4000,4500,5000};

//-----------------------------------------------------------------------------------------

//https://sub.nanona.fi/esp8266/timing-and-ticks.html
//One tick is 1us / 80 = 0.0125us = 12.5ns on 80MHz
//One tick is 1us / 80 = 0.0125us = 6.25ns on 160MHz
// 1 NOP is 1 tick

void inline delayMS(int d) {
  asm volatile ("nop"); 
  asm volatile ("nop");
  asm volatile ("nop");
}


void setup_pins() {
  DPRINTLN("VFD Clock - setup MAX6921 pins");
  pinMode(PIN_LE,  OUTPUT);  regPin(PIN_LE,"PIN_LE");
  pinMode(PIN_BL,  OUTPUT);  regPin(PIN_BL,"PIN_BL");
  digitalWrite(PIN_BL,LOW);  //brightness
  pinMode(PIN_DATA,OUTPUT);  regPin(PIN_DATA,"PIN_DATA");
  pinMode(PIN_CLK, OUTPUT);  regPin(PIN_CLK,"PIN_CLK");
  
  maxDig = maxDigits;  //put const to memory var
  generateBitTable();
  digitsOnly = false;
  startTimer();
}  

void IRAM_ATTR writeDisplay(){  //void IRAM_ATTR  writeDisplay(){
  static DRAM_ATTR int timer = PWMrefresh;
  static DRAM_ATTR uint32_t val;
  static DRAM_ATTR byte pos = 0;
  static DRAM_ATTR boolean state=true;
  static DRAM_ATTR byte brightness;
  static DRAM_ATTR int PWMtimeBrightness=PWMtiming[1];

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
      //digitalWrite(PIN_BL,HIGH);    //OFF
    return;
  }

  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();
  intCounter++;

  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHT) brightness = MAXBRIGHT;  //only for safety
  
  if ((!autoBrightness) && (brightness==MAXBRIGHT)) state = true;
  
  if (state) {  //ON state
    pos++;  if (pos>maxDig-1)  {   //go to the tube#0
      pos = 0; 
      if (autoBrightness && displayON) {   //change brightness only on the tube#0
        PWMtimeBrightness = max(PWMtiming[2],PWMtiming[MAXBRIGHT]*lx/MAXIMUM_LUX);
        }
      else
        PWMtimeBrightness = PWMtiming[brightness];
    }  
    
    val = (digitEnableBits[pos] | charTable[digit[pos]]);  //the full bitmap to send to MAX chip
    if (digitDP[pos]) val = val | charTable[12];    //Decimal Point
        
    timer = PWMtimeBrightness;
    //if (pos==2) timer = 3*timer;  //Weak IV11 tube#2 brightness compensation
    timerON = timer;
    timerOFF = PWMrefresh-PWMtimeBrightness;
  }
  else {  //OFF state
    timer = PWMrefresh-PWMtimeBrightness;
  }
  
  if (timer<500) timer = 500;  //safety only...

  if ( (brightness == 0) || (!state) || (!radarON)) {  //OFF state, blank digit
    digitalWrite(PIN_BL,HIGH);    //OFF
  }
  else {  //ON state
    if (animMask[pos]>0) val &= animationMaskBits[animMask[pos]-1];  //animationMode 6, mask characters from up to down and back
    for (int i=0; i<20; i++)  {
      if (val & uint32_t(1 << (19 - i)))
        {digitalWrite(PIN_DATA, HIGH);   //asm volatile ("nop");
        }
      else
        {digitalWrite(PIN_DATA, LOW);    //asm volatile ("nop");
        }
      
      digitalWrite(PIN_CLK,HIGH);  //asm volatile ("nop");  //delayMS(1);
      digitalWrite(PIN_CLK,LOW);   //asm volatile ("nop"); //delayMS(1);
      } //end for      
 
    digitalWrite(PIN_LE,HIGH );  asm volatile ("nop");
    digitalWrite(PIN_LE,LOW);
    digitalWrite(PIN_BL,LOW );   //ON
  }  //end else
  
  portEXIT_CRITICAL_ISR(&timerMux);   
  state = !state;  
  //ESP32timer = timerBegin(0, PRESCALER, true);  //set prescaler, true = edge generated signal
  //timerAttachInterrupt(ESP32timer, &writeDisplay, true);   
  timerAlarmWrite(ESP32timer, timer, true);   
  timerAlarmEnable(ESP32timer);
  interrupts();
}

void generateBitTable() {
uint32_t out;

  DPRINTLN("--- Generating segment pins bitmap ---");
  for (int i=0;i<MAXSEGMENTS;i++) {
    segmentEnableBits[i] = uint32_t(1<<segmentEnablePins[i]);
    //DPRINT(i); DPRINT(": "); DPRINTLN(segmentEnableBits[i],BIN);
  }
  animationMaskBits[0] = uint32_t(1<<segmentEnablePins[0]);  //a
  animationMaskBits[1] = animationMaskBits[0] | uint32_t(1<<segmentEnablePins[1]) | uint32_t(1<<segmentEnablePins[5]);  //bf
  animationMaskBits[2] = animationMaskBits[1] | uint32_t(1<<segmentEnablePins[6]);  //g
  animationMaskBits[3] = animationMaskBits[2] | uint32_t(1<<segmentEnablePins[4]) | uint32_t(1<<segmentEnablePins[2]);  //ec
  animationMaskBits[4] = animationMaskBits[3] | uint32_t(1<<segmentEnablePins[3]);  //d
  for (int i=0;i<5;i++) {
    animationMaskBits[i] = ~animationMaskBits[i]; //invert bits
    //DPRINTLN(animationMaskBits[i],HEX);
  }
  DPRINTLN("--- Generating digit pins bitmap ---");
  for (int i=0;i<maxDigits;i++) {
    digitEnableBits[i] = uint32_t(1 << digitEnablePins[i]);
    //DPRINT(i); DPRINT(": "); DPRINTLN( digitEnableBits[i],BIN);
  }

DPRINTLN("---- Generated Character / Pins table -----");
  for (int i=0;i<MAXCHARS;i++) {
    out = 0;
    //DPRINT(i); DPRINT(":  ");
    //DPRINT(charDefinition[i],BIN);  //DPRINT(" = ");
    for (int j=0;j<=7;j++)   //from a to g
      if ((charDefinition[i] & 1<<(7-j)) != 0) {
        out = out | segmentEnableBits[j]; //DPRINT("1"); 
        }
    //else        DPRINT("0");
    //DPRINT("  >> ");  
    
    charTable[i] = out;
    //DPRINTLN(charTable[i],BIN);
  }  //end for
}


void clearTubes() {
  digitalWrite(PIN_BL,HIGH);
}

void writeDisplaySingle() {}
#endif

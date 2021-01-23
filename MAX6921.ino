#ifdef MAX6921   
//VFD driver driver for ESP8266

//------------------abcdefgDP----------------   definition of different characters
byte charDefinition[] = {
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
                   B11000110,   //grad (upper circle) abfg (16)
                   B10110100,   //%  acdf  (17)
                   B00111010,   //lower circle cdeg  (18)
                   B01100000,   //I  bc    (19)
                   B10001110    //F  aefg  (20)

};

uint32_t animationMaskBits[5];

#define MAXCHARS sizeof(charDefinition)
#define MAXSEGMENTS sizeof(segmentEnablePins)
const int maxDigits =  sizeof(digitEnablePins);

uint32_t charTable[MAXCHARS];              //generated pin table from segmentDefinitions
uint32_t segmentEnableBits[MAXSEGMENTS];   //bitmaps, generated from EnablePins tables
uint32_t digitEnableBits[10];

int PWMrefresh=5500;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
#define MAXBRIGHT 10
int PWMtiming[MAXBRIGHT+1] = {0,500,800,1200,2000,2500,3000,3500,4000,4500,5000};
//-----------------------------------------------------------------------------------------

//https://sub.nanona.fi/esp8266/timing-and-ticks.html
//One tick is 1us / 80 = 0.0125us = 12.5ns on 80MHz
//One tick is 1us / 80 = 0.0125us = 6.25ns on 160MHz
// 1 NOP is 1 tick


void inline delayMS(int d) {
    //for (int i=0;i<d;i++) 
      asm volatile ("nop"); 
}


void setup_pins() {
  
  #if defined(ESP8266)
  #else
    #error "Board is not supported! For ESP32 use MAX6921_ESP32 !"  
  #endif
  
  DPRINTLN("Setup MAX6921 pins for VFD Clock...");
  pinMode(PIN_LE,  OUTPUT);  regPin(PIN_LE,"PIN_LE");
  pinMode(PIN_BL,  OUTPUT);  regPin(PIN_BL,"PIN_BL");
  digitalWrite(PIN_BL,LOW);  //brightness
  pinMode(PIN_DATA,OUTPUT);  regPin(PIN_DATA,"PIN_DATA");
  pinMode(PIN_CLK, OUTPUT);  regPin(PIN_CLK,"PIN_CLK");
  
  generateBitTable();
  digitsOnly = false;
  startTimer();
}  

void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  static volatile int timer = PWMrefresh;
  static volatile uint32_t val;
  static volatile byte pos = 0;
  static volatile boolean state=true;
  static volatile byte brightness;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
      digitalWrite(PIN_BL,HIGH);    //OFF
      timer1_write(PWMrefresh);
    return;
  }
  
  intCounter++;
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHT) brightness = MAXBRIGHT;  //only for safety
  if ((!autoBrightness) && (brightness==MAXBRIGHT))  
    state = true;

  if (state) {  //ON state
    pos++;  if (pos>maxDigits-1)  pos = 0;  //go to the first tube
    val = (digitEnableBits[pos] | charTable[digit[pos]]);  //the full bitmap to send to MAX chip
    if (digitDP[pos]) val = val | charTable[12];    //Decimal Point
    
  if (autoBrightness && displayON)
      timer = max(PWMtiming[1],PWMtiming[MAXBRIGHT] * LuxValue / MAXIMUM_LUX);
    else
      timer = PWMtiming[brightness];
    //if (pos==2) timer = 3*timer;  //Weak IV11 tube#2 brightness compensation
    timerON = timer;
  }
  else {  //OFF state
    if (autoBrightness && displayON)
      timer = PWMrefresh - max(PWMtiming[1],PWMtiming[MAXBRIGHT] * LuxValue / MAXIMUM_LUX);
    else  
      timer = PWMrefresh-PWMtiming[brightness];
    timerOFF = timer;  
  }

  if (timer<500) timer = 500;  //safety only...

  if ( (brightness == 0) || (!state) ) {  //OFF state, blank digit
    digitalWrite(PIN_BL,HIGH);    //OFF
    }
  else {  //ON state

    if (animMask[pos]>0) val &= animationMaskBits[animMask[pos]-1];  //animationMode 6, mask characters from up to down and back
    for (int i=0; i<20; i++)  {
      if (val & uint32_t(1 << (19 - i))) {
        digitalWrite(PIN_DATA, HIGH);   
        //asm volatile ("nop");
        }
      else {
        digitalWrite(PIN_DATA, LOW);    
        //asm volatile ("nop");
        }
      
      digitalWrite(PIN_CLK,HIGH);  
      //asm volatile ("nop");  //delayMS(1);
      digitalWrite(PIN_CLK,LOW);   
      //asm volatile ("nop"); //delayMS(1);
      } //end for      
 
    digitalWrite(PIN_LE,HIGH );  asm volatile ("nop");
    digitalWrite(PIN_LE,LOW);
    digitalWrite(PIN_BL,LOW );   //ON
  }  //end else

  state = !state;  
  timer1_write(timer);
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

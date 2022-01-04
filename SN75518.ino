#ifdef SN75518_ESP32
char tubeDriver[] = "SN75518_ESP32";
//int maxDigits = 6;
int wt = 5;   //Serial timing

#define PIN_LE      13  // D7 Shift Register Latch Enable
#define PIN_CLK     12  // D6 Shift Register Clock
#define PIN_DATA    27  // D5 Shift Register Data
#define PIN_STROBE  14  // D1 Shift Register Strobe (1=display off     0=display on)

//#define SEGMENT8   //please set in clocks.h
//#define TESTMODE   //for testing the segments of a new type VFD modul
int count = 15;  //output# to test
unsigned long lastTest = 0;

#ifdef SEGMENT8
  boolean ASCIImode = false;
  #define DP_CHAR 12
  byte charDefinition[] = {
//------------------abcdefgDP----------------   definition of different characters
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
                   B00000000,   // : BLANK   (10)
                   B00000010,   //-: minus (11)
                   B00000001,   // decimal point (12)
                   B11101110,   // A  abcefg  (13)
                   B11001110,   // P  abefg (14)
                   B10011100,   // C  adef (15)
                   B11000110,   //grad (upper circle) abfg  (16)         
                   B10110100,   //%  acdf  (17) 
                   B00111010,   //lower circle cdeg  (18)                   
                   B01100000,   //I  bc    (19)
                   B10001110    //F  aefg  (20)                            
};

//byte segmentEnablePins[] =  {9,8,5,3,4,7,6,2};   //segment enable bits (a,b,c,d,e,f,g,DP)   (You MUST define always 8 bits!!!)

#else
  //16 segment + DP  ---------------------------------------------------------------------------------------------
  boolean ASCIImode = true;
  #define DP_CHAR 14
  uint32_t charDefinition[96] = {
//  Dutsrpnmkhgfedcba   
  0b00000000000000000, /* (space) */ 
  0b10000000000001100, /* ! */   
  0b00000001000000100, /* " */   //   100000001
  0b01010101000111100, /* # */
  0b01010101010111011, /* $ */
  0b01110111010011001, /* % */
  0b01001001101110001, /* & */
  0b00000001000000000, /* ' */
  0b00001010000000000, /* ( */
  0b00100000100000000, /* ) */
  0b01111111100000000, /* * */
  0b01010101000000000, /* + */
  0b00100000000000000, /* , */
  0b01000100000000000, /* - */
  0b10000000000000000, /* . */
  0b00100010000000000, /* / */
  0b00100010011111111, /* 0 */
  0b00000010000001100, /* 1 */
  0b01000100001110111, /* 2 */
  0b00000100000111111, /* 3 */
  0b01000100010001100, /* 4 */
  0b01001000010110011, /* 5 */
  0b01000100011111011, /* 6 */
  0b00000000000001111, /* 7 */
  0b01000100011111111, /* 8 */
  0b01000100010111111, /* 9 */
  0b00010001000000000, /* : */
  0b00100001000000000, /* ; */
  0b01001010000000000, /* < */
  0b01000100000110000, /* = */
  0b00100100100000000, /* > */
  0b10010100000000111, /* ? */
  0b00000101011110111, /* @ */
  0b01000100011001111, /* A */
  0b00010101000111111, /* B */
  0b00000000011110011, /* C */
  0b00010001000111111, /* D */
  0b01000000011110011, /* E */
  0b01000000011000011, /* F */
  0b00000100011111011, /* G */
  0b01000100011001100, /* H */
  0b00010001000110011, /* I */
  0b00000000001111100, /* J */
  0b01001010011000000, /* K */
  0b00000000011110000, /* L */
  0b00000010111001100, /* M */
  0b00001000111001100, /* N */
  0b00000000011111111, /* O */
  0b01000100011000111, /* P */
  0b00001000011111111, /* Q */
  0b01001100011000111, /* R */
  0b01000100010111011, /* S */
  0b00010001000000011, /* T */
  0b00000000011111100, /* U */
  0b00100010011000000, /* V */
  0b00101000011001100, /* W */
  0b00101010100000000, /* X */
  0b01000100010111100, /* Y */
  0b00100010000110011, /* Z */
  0b00010001000010010, /* [ */
  0b00001000100000000, /* \ */
  0b00010001000100001, /* ] */
  0b00101000000000000, /* ^ */
  0b00000000000110000, /* _ */
  0b00000000100000000, /* ` */
  0b01010000001110000, /* a */
  0b01010000011100000, /* b */
  0b01000000001100000, /* c */
  0b00010100000011100, /* d */
  0b01100000001100000, /* e */
  0b01010101000000010, /* f */
  0b01010001010100001, /* g */
  0b01010000011000000, /* h */
  0b00010000000000000, /* i */
  0b00010001001100000, /* j */
  0b00011011000000000, /* k */
  0b00000000011000000, /* l */
  0b01010100001001000, /* m */
  0b01010000001000000, /* n */
  0b01010000001100000, /* o */
  0b01000001011000001, /* p */
  0b01010001010000001, /* q */
  0b01000000001000000, /* r */
  0b01010000010100001, /* s */
  0b01000000011100000, /* t */
  0b00010000001100000, /* u */
  0b00100000001000000, /* v */
  0b00101000001001000, /* w */
  0b00101010100000000, /* x */
  0b00000101000011100, /* y */
  0b01100000000100000, /* z */
  0b01010001000010010, /* { */
  0b00010001000000000, /* | */
  0b00010101000100001, /* } */
  0b01000001010000001, /* ~ */
  0b00000000000000000, /* (del) */
  //Dutsrpnmkhgfedcba  
};

  //byte segmentEnablePins[] =  {16,7,0,1,2,3,4,5,6,8,9,11,10,12,13,14,15};    //segment enable bits defined in clocks.h
#endif

#define MAXCHARS sizeof(charDefinition)/sizeof(charDefinition[0])
#define MAXSEGMENTS sizeof(segmentEnablePins)
char asciiConvert[] = "0123456789 -.APC~% IF";
int maxDigits =  sizeof(digitEnablePins);
int dispChar;

uint32_t DRAM_ATTR animationMaskBits[5];
uint32_t DRAM_ATTR charTable[MAXCHARS];              //generated pin table from segmentDefinitions
uint32_t DRAM_ATTR segmentEnableBits[MAXSEGMENTS];   //bitmaps, generated from EnablePins tables
uint32_t DRAM_ATTR digitEnableBits[10];

int DRAM_ATTR PWMrefresh=5500;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
int DRAM_ATTR PWM_min = 500;
int DRAM_ATTR PWM_max = 5000;
//int DRAM_ATTR PWMtiming[11] = {0,500,800,1200,2000,2500,3000,3500,4000,4500,5000};

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
  DPRINTLN("VFD Clock - setup SN75518 pins");
  pinMode(PIN_LE,  OUTPUT);  regPin(PIN_LE,"PIN_LE");
  pinMode(PIN_STROBE,  OUTPUT);  regPin(PIN_STROBE,"PIN_STROBE");
  digitalWrite(PIN_STROBE,LOW);  //brightness
  pinMode(PIN_DATA,OUTPUT);  regPin(PIN_DATA,"PIN_DATA");
  pinMode(PIN_CLK, OUTPUT);  regPin(PIN_CLK,"PIN_CLK");
  #ifdef TESTMODE
    DPRINT("Testing output Q#:"); DPRINTLN(count);
  #endif
  driverSetupStr = "SN75518 segmentEnablePins:";
  for (int j=0;j<sizeof(segmentEnablePins);j++) {
    driverSetupStr += String(segmentEnablePins[j]) + ",";
  }
  driverSetupStr += "<br>SN75518 digitEnablePins:";
  for (int j=0;j<sizeof(digitEnablePins);j++) {
    driverSetupStr += String(digitEnablePins[j]) + ",";
  }
  driverSetupStr += String("<br>");
  
  generateBitTable();
  digitsOnly = false;
  startTimer();
}  

void IRAM_ATTR writeDisplay(){  //void IRAM_ATTR  writeDisplay(){
  static DRAM_ATTR int timer = PWMrefresh;
  static DRAM_ATTR uint32_t val;
  static DRAM_ATTR byte pos = 0;
  static DRAM_ATTR boolean state=true;
  static DRAM_ATTR int brightness;
  static DRAM_ATTR int PWMtimeBrightness=PWM_min;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
      //digitalWrite(PIN_STROBE,HIGH);    //OFF
    return;
  }
  //digit[5] = 15;   digit[4] = 16;     digit[3] = 17;     digit[2] = 18;      digit[1] = 19;       digit[0] = 20;
  portENTER_CRITICAL_ISR(&timerMux);
  noInterrupts();
  intCounter++;

  brightness = displayON ? prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety
  
  if ((!autoBrightness) && (brightness==MAXBRIGHTNESS)) state = true;
  
  if (state) {  //ON state
    pos++;  if (pos>maxDigits-1)  {   //go to the tube#0
      pos = 0; 
      if (autoBrightness && displayON) {   //change brightness only on the tube#0
        PWMtimeBrightness = max(PWM_min,PWM_max*lx/MAXIMUM_LUX);
        }
      else
        PWMtimeBrightness = max(PWM_min,PWM_max*brightness/MAXBRIGHTNESS);
    }  
    dispChar = digit[pos];
    if (ASCIImode) {
      if (dispChar<sizeof(asciiConvert)) dispChar = asciiConvert[dispChar];       
      dispChar -= 32;                  
    }
    val = (digitEnableBits[pos] | charTable[dispChar]);  //the full bitmap to send to MAX chip
    if (digitDP[pos]) val = val | charTable[DP_CHAR];    //Decimal Point
    #ifdef TESTMODE
      val = digitEnableBits[pos] | ((uint32_t)1<<(count-1));
      animMask[pos] = 0;
      if ((millis()-lastTest)>5000) {
        lastTest=millis();
        count++; if (count>32) count = 0;
      }
    #endif       
    timer = PWMtimeBrightness;
    #ifdef CLOCK_xx  //example only
      if (pos==2) timer = 2*timer;  //Weak  tube#2 brightness compensation, some hacking
      if (pos==4) timer = 3*timer;  //Weak  tube#4 brightness compensation, some hacking
    #endif 
    timerON = timer;
    timerOFF = PWMrefresh-PWMtimeBrightness;
  }
  else {  //OFF state
    timer = PWMrefresh-PWMtimeBrightness;
  }
  
  if (timer<500) timer = 500;  //safety only...

  if ( (brightness == 0) || (!state) || (!radarON)) {  //OFF state, blank digit
    digitalWrite(PIN_STROBE,HIGH);    //OFF
  }
  else {  //ON state
    if (animMask[pos]>0) val &= animationMaskBits[animMask[pos]-1];  //animationMode 6, mask characters from up to down and back
    for (int i=0; i<32; i++)  {
      if (val & uint32_t(1 << (31 - i)))
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
    digitalWrite(PIN_STROBE,LOW );   //ON
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

  DPRINT("--- Generating segment pins bitmap:"); DPRINTLN(MAXSEGMENTS);
  for (int i=0;i<MAXSEGMENTS;i++) {
    segmentEnableBits[i] = uint32_t(1)<<(segmentEnablePins[i]-1); 
    //DPRINT(i); DPRINT(": "); DPRINTLN(segmentEnableBits[i],BIN);
  }
#ifdef SEGMENT8
  animationMaskBits[0] = uint32_t(1<<segmentEnablePins[0]);  //a
  animationMaskBits[1] = animationMaskBits[0] | uint32_t(1)<<segmentEnablePins[1] | uint32_t(1<<segmentEnablePins[5]);  //bf
  animationMaskBits[2] = animationMaskBits[1] | uint32_t(1)<<segmentEnablePins[6];  //g
  animationMaskBits[3] = animationMaskBits[2] | uint32_t(1)<<segmentEnablePins[4] | uint32_t(1<<segmentEnablePins[2]);  //ec
  animationMaskBits[4] = animationMaskBits[3] | uint32_t(1)<<segmentEnablePins[3];  //d
#else  
  //Dutsrpnmkhgfedcba  
  //DP=16,u=15,t=14,s=13,r=12,p=11,n=10,m=9,k=8,h=7,g=6,f=5,e=4,d=3,c=2,b=1,a=0}
  animationMaskBits[0] = uint32_t(1<<segmentEnablePins[0]) | uint32_t(1<<segmentEnablePins[1]);  //ab
  animationMaskBits[1] = animationMaskBits[0] | uint32_t(1<<segmentEnablePins[2]) | uint32_t(1<<segmentEnablePins[8]) |
      uint32_t(1<<segmentEnablePins[9]) | uint32_t(1<<segmentEnablePins[10]) | uint32_t(1<<segmentEnablePins[7]);  //hkmnc
  animationMaskBits[2] = animationMaskBits[1] | uint32_t(1<<segmentEnablePins[15]) | uint32_t(1<<segmentEnablePins[11]);  //up
  animationMaskBits[3] = animationMaskBits[2] | uint32_t(1<<segmentEnablePins[6]) | uint32_t(1<<segmentEnablePins[14]) |
      uint32_t(1<<segmentEnablePins[13]) |uint32_t(1<<segmentEnablePins[12]) |uint32_t(1<<segmentEnablePins[3]);  //gtsrd
  animationMaskBits[4] = animationMaskBits[3] | uint32_t(1<<segmentEnablePins[5]) | uint32_t(1<<segmentEnablePins[4]);  //fe
#endif  
  DPRINTLN("--- Generating animation mask bitmap:");
  for (int i=0;i<5;i++) {
    animationMaskBits[i] = ~animationMaskBits[i]; //invert bits
    //DPRINTLN(animationMaskBits[i],BIN);
  }
  DPRINT("--- Generating digit pins bitmap:"); DPRINTLN(maxDigits);
  for (int i=0;i<maxDigits;i++) {
    digitEnableBits[i] = uint32_t(1 << (digitEnablePins[i]-1));
    //DPRINT(i); DPRINT(": "); DPRINTLN( digitEnableBits[i],BIN);
  }

  DPRINT("--- Generated Character / Pins table:"); DPRINTLN(MAXCHARS);
  for (int i=0;i<MAXCHARS;i++) {
    out = 0;
    //DPRINT(i); DPRINT(":  ");
    //DPRINT(charDefinition[i],BIN);  //DPRINT(" = ");
    for (int j=0;j<MAXSEGMENTS;j++)   //from a to g
      if ((uint32_t)(charDefinition[i] & (uint32_t)1<<j) != 0) {
        out = out | segmentEnableBits[MAXSEGMENTS-j-1]; //DPRINT("1"); 
        }
    //else        {DPRINT("0");}
    //DPRINT("  >> ");  
    
    charTable[i] = out;
    //DPRINTLN(charTable[i],BIN);
  }  //end for
}


void clearTubes() {
  digitalWrite(PIN_STROBE,HIGH);
}

void writeDisplaySingle() {}
#endif

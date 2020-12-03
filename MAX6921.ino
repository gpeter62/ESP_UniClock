#ifdef MAX6921   
//VFD driver driver for ESP8266 os ESP32

#if defined(ESP32)
  //Fill this table with the OUT positions of the MAX6921 chip!   
  byte segmentEnablePins[] =  {19,17,15,12,13,16,18,14};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
  byte digitEnablePins[] = {9,8,7,2,1,0};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6)  (You may define any number)
  
  //MAX6921 pins
  #define PIN_LE    14  // Shift Register Latch Enable
  #define PIN_CLK   13  // Shift Register Clock
  #define PIN_DATA  27  // Shift Register Data
  #define PIN_BL    12  // Shift Register Blank (1=display off     0=display on)

#else  //Any 8266 clock
  //#define IV18
  //#define IVL2
  #define IV11

  #ifdef IV18
  //Fill this table with the OUT positions of the MAX6921 chip!   
    byte segmentEnablePins[] =  {0,2,5,6,4,1,3,7};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
    byte digitEnablePins[] = {18,11,17,12,16,13,14,15}; //19};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  (You may define any number)
  //MAX6921 pins
    #define PIN_LE    12  // D6 Shift Register Latch Enable
    #define PIN_CLK   13  // D7 Shift Register Clock
    #define PIN_DATA  14  // D5 Shift Register Data
    #define PIN_BL    15  // D8 Shift Register Blank (1=display off     0=display on)
  #endif

  #ifdef IVL2
  //Fill this table with the OUT positions of the MAX6921 chip!   
    byte segmentEnablePins[] =  {11,13,5,8,3,12,2,1};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
    byte digitEnablePins[] = {10,9,4,0};  //segment enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  
  //MAX6921 pins
    #define PIN_LE    13  // D6 Shift Register Latch Enable
    #define PIN_CLK   12  // D7 Shift Register Clock
    #define PIN_DATA  14  // D5 Shift Register Data
    #define PIN_BL    15  // D8 Shift Register Blank (1=display off     0=display on)
    #define TEMP_SENSOR_PIN RX
  #endif

    #ifdef IV11  //Custom made 4 x IV11 clock
  //Fill this table with the OUT positions of the MAX6921 chip!   
    byte segmentEnablePins[] =  {7,5,1,0,2,4,6,3};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
    byte digitEnablePins[] = {19,18,17,16};  //digit enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  (You may define any number)
  //MAX6921 pins
    #define PIN_LE    12  // D6 Shift Register Latch Enable
    #define PIN_CLK   13  // D7 Shift Register Clock
    #define PIN_DATA  14  // D5 Shift Register Data
    #define PIN_BL    15  // D8 Shift Register Blank (1=display off     0=display on)
  #endif
#endif

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
                   B00000000,   // : BLANK   (10)
                   B00000010,   //-: minus (11)
                   B00000001,   // decimal point (12)
                   B11101110,   // A  abcefg  (13)
                   B11001110,   // P  abefg (14)
                   B10011100,   // C  adef (15)
                   B11000110    //grad  abfg  (16)
};

uint32_t animationMaskBits[5];

#define MAXCHARS sizeof(charDefinition)
#define MAXSEGMENTS sizeof(segmentEnablePins)
const int maxDigits =  sizeof(digitEnablePins);

uint32_t charTable[MAXCHARS];              //generated pin table from segmentDefinitions
uint32_t segmentEnableBits[MAXSEGMENTS];   //bitmaps, generated from EnablePins tables
uint32_t digitEnableBits[10];

const int PWMrefresh=5500;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
#define MAXBRIGHT 10
const int PWMtiming[MAXBRIGHT+1] = {0,250,500,1000,2000,2500,3000,3500,4000,4500,5000};
//-----------------------------------------------------------------------------------------

//https://sub.nanona.fi/esp8266/timing-and-ticks.html
//One tick is 1us / 80 = 0.0125us = 12.5ns on 80MHz
//One tick is 1us / 80 = 0.0125us = 6.25ns on 160MHz
// 1 NOP is 1 tick

#if defined(ESP8266)   
  void inline delayMS(int d) {
    for (int i=0;i<d;i++) {asm volatile ("nop"); }
  }
#else
  void inline delayMS(int d) {
    for (int i=0;i<d*7;i++) {asm volatile ("nop"); }
  }
#endif

void setup_pins() {
  
  #if defined(ESP8266) || defined(ESP32)
  #else
    #error "Board is not supported!"  
  #endif
  
  pinMode(PIN_LE,  OUTPUT);
  pinMode(PIN_BL,  OUTPUT);   digitalWrite(PIN_BL,LOW);  //brightness
  pinMode(PIN_DATA,OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
    
  DPRINTLN("Setup MAX6921 pins...");
  DPRINT("- CLK   : GPIO"); DPRINTLN(PIN_CLK);
  DPRINT("- DATAIN: GPIO"); DPRINTLN(PIN_DATA);
  DPRINT("- LE    : GPIO"); DPRINTLN(PIN_LE);
  DPRINT("- BLANK : GPIO"); DPRINTLN(PIN_BL);
  
  generateBitTable();
  digitsOnly = false;
  startTimer();
}  

#if defined(ESP32) 
void writeDisplay(){  //void IRAM_ATTR  writeDisplay(){
#else 
void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
#endif
  
  static volatile int timer = PWMrefresh;
  static volatile uint32_t val;
  static volatile byte pos = 0;
  static volatile boolean state=true;
  static volatile byte brightness;

  #if defined(ESP32)
    portENTER_CRITICAL_ISR(&timerMux);
  #endif

  intCounter++;
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    #if defined(ESP8266)    
      digitalWrite(PIN_BL,HIGH);    //OFF
      timer1_write(PWMrefresh);
    #elif defined(ESP32)
      portEXIT_CRITICAL(&timerMux);
    #endif    
    return;
  }

  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHT) brightness = MAXBRIGHT;  //only for safety
  if (brightness==MAXBRIGHT) state = true;

  if (state) {  //ON state
    pos++;  if (pos>maxDigits-1)  pos = 0;  //go to the first tube
    timer = PWMtiming[brightness];
  }
  else {  //OFF state
    timer = PWMrefresh-PWMtiming[brightness];
  }

  if (timer<500) timer = 500;  //safety only...

  if ( (brightness == 0) || (!state) ) {  //OFF state, blank digit
    digitalWrite(PIN_BL,HIGH);    //OFF
    }
  else {  //ON state
    val = (digitEnableBits[pos] | charTable[digit[pos]]);  //the full bitmap to send to MAX chip
    if (digitDP[pos]) val = val | charTable[12];    //Decimal Point
    if (animMask[pos]>0) val &= animationMaskBits[animMask[pos]-1];  //animationMode 6, mask characters from up to down and back
    for (int i=0; i<20; i++)  {
      if (val & uint32_t(1 << (19 - i)))
        {digitalWrite(PIN_DATA, HIGH);   asm volatile ("nop");}
      else
        {digitalWrite(PIN_DATA, LOW);    asm volatile ("nop");}
      
      digitalWrite(PIN_CLK,HIGH);  delayMS(3);
      digitalWrite(PIN_CLK,LOW);   delayMS(3);
      } //end for      
 
    digitalWrite(PIN_LE,HIGH );  delayMS(8);
    digitalWrite(PIN_LE,LOW);
    digitalWrite(PIN_BL,LOW );   //ON
  }  //end else
    
  if (COLON_PIN>=0) {
    digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
  }
  
  state = !state;  
  #if defined(ESP8266)    
    timer1_write(timer);
  #elif defined(ESP32)     
    ESP32timer = timerBegin(0, PRESCALER, true);  //set prescaler, true = edge generated signal
    timerAttachInterrupt(ESP32timer, &writeDisplay, true);   
    timerAlarmWrite(ESP32timer, timer, false);   
    timerAlarmEnable(ESP32timer);
    portEXIT_CRITICAL_ISR(&timerMux);
  #endif    
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

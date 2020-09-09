#ifdef MAX6921
// for example usable with IV18 VFD CLock
// Pulse Filament Drive (High Frequency RMS) is supported, if PIN_HEAT_A and PIN_HEAT_B are set!!!

//#define VFDrefresh 1200    //msec, Multiplex time period. Greater value => slower multiplex frequency


//Choose only 1 option from the following:
#define IV18
//#define IVL2


#ifdef IV18
//Fill this table with the OUT positions of the MAX6921 chip!   
byte segmentEnablePins[] =  {0,2,5,6,4,1,3,7};   //segment enable OUTbits of MAX6921 (a,b,c,d,e,f,g,DP)  (You MUST define always 8 Pins!!!)
byte digitEnablePins[] = {18,11,17,12,16,13,14,15}; //19};  //segment enable OUTbits of MAX6921 (1,2,3,4,5,6,7,8)  (You may define any numb
const int tubeTime[] = {1200,1200,1200,1200,1200,1200,1200,1200,1200};      //ticks to stay on the same digit to compensate different digit brightness
                                                  
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
const int tubeTime[] = {1200,1200,1200,1200,1200,1200,1200,1200,1200};      //ticks to stay on the same digit to compensate different digit brightness
                                               
//MAX6921 pins
#define PIN_LE    13  // D6 Shift Register Latch Enable
#define PIN_CLK   12  // D7 Shift Register Clock
#define PIN_DATA  14  // D5 Shift Register Data
#define PIN_BL    15  // D8 Shift Register Blank (1=display off     0=display on)
#endif

#define PIN_HEAT_A -1   //VFD heater signalA  (if not used, set to -1)
#define PIN_HEAT_B -1   //VFD heater signalB  (if not used, set to -1)

#define PIN_LE_BIT     1<<PIN_LE    
#define PIN_CLK_BIT    1<<PIN_CLK    
#define PIN_DATA_BIT   1<<PIN_DATA  
#define PIN_BL_BIT     1<<PIN_BL    
#define PIN_HEAT_A_BIT 1<<PIN_HEAT_A
#define PIN_HEAT_B_BIT 1<<PIN_HEAT_B

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
int maxDigits =  sizeof(digitEnablePins);

uint32_t charTable[MAXCHARS];              //generated pin table from segmentDefinitions
uint32_t segmentEnableBits[MAXSEGMENTS];   //bitmaps, generated from EnablePins tables
uint32_t digitEnableBits[10];
boolean useHeater = false;                 //Is heater driver signal used?
//-----------------------------------------------------------------------------------------

//https://sub.nanona.fi/esp8266/timing-and-ticks.html
//One tick is 1us / 80 = 0.0125us = 12.5ns on 80MHz
//One tick is 1us / 80 = 0.0125us = 6.25ns on 160MHz
// 1 NOP is 1 tick

void setup_pins() {
#if defined(ESP8266) 
#else
  #error "Board is not supported!"  
#endif
  
  pinMode(PIN_LE,  OUTPUT);
  pinMode(PIN_BL,  OUTPUT);   digitalWrite(PIN_BL,LOW);  //brightness
  pinMode(PIN_DATA,OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  
  if ((PIN_HEAT_A >=0) && (PIN_HEAT_B>=0)) {
    useHeater = true;
    pinMode(PIN_HEAT_A, OUTPUT);  digitalWrite(PIN_HEAT_A,HIGH);
    pinMode(PIN_HEAT_B, OUTPUT);  digitalWrite(PIN_HEAT_B,LOW);
    DPRINT("Filament Driver: GPIO"); DPRINT(PIN_HEAT_A);
    DPRINT(" / GPIO");      DPRINTLN(PIN_HEAT_B);
  }
  
  DPRINTLN("Setup MAX6921 pins...");
  DPRINT("- CLK   : GPIO"); DPRINTLN(PIN_CLK);
  DPRINT("- DATAIN: GPIO"); DPRINTLN(PIN_DATA);
  DPRINT("- LE    : GPIO"); DPRINTLN(PIN_LE);
  DPRINT("- BLANK : GPIO"); DPRINTLN(PIN_BL);
  
  generateBitTable();
  digitsOnly = false;
    
  timer1_attachInterrupt(writeDisplay);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(tubeTime[0]); 
}  


void ICACHE_RAM_ATTR writeDisplay(){        // Writes to the MAX6921 driver for IV-18
static volatile uint32_t val = 0;
static volatile byte pos = 0;
static volatile int brightCounter[] = {0,9,2,8,4,7,6,5,3,1};
static volatile boolean heatState = false;

  //noInterrupts();
  
  if (useHeater) {
    heatState = !heatState;
    if (heatState) {
      WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_HEAT_A_BIT );
      WRITE_PERI_REG( PIN_OUT_SET,   PIN_HEAT_B_BIT );
    }
    else {
      WRITE_PERI_REG( PIN_OUT_SET,   PIN_HEAT_A_BIT );
      WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_HEAT_B_BIT );
    }
  }
  
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    timer1_write(tubeTime[0]);
    return;  
  }
  
  if (brightCounter[pos] % MAXBRIGHTNESS < (displayON ?  prm.dayBright : prm.nightBright))
    WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_BL_BIT );   //ON
  else 
    WRITE_PERI_REG( PIN_OUT_SET, PIN_BL_BIT );    //OFF

  
  brightCounter[pos]++;  
  if (brightCounter[pos]>MAXBRIGHTNESS) brightCounter[pos] = 1;
  
  val = (digitEnableBits[pos] | charTable[digit[pos]]);  //the full bitmap to send to MAX chip
  if (digitDP[pos]) val = val | charTable[12];    //Decimal Point
  if (animMask[pos]>0) val &= animationMaskBits[animMask[pos]-1];  //animationMode 6, mask characters from up to down and back
  
  for (int i=0; i<20; i++)  {
    if ((val & uint32_t(1 << (19 - i))) ) {
      WRITE_PERI_REG( PIN_OUT_SET, PIN_DATA_BIT );
      asm volatile ("nop");
    }
    else {
      WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_DATA_BIT );
      asm volatile ("nop");
    }
    WRITE_PERI_REG( PIN_OUT_SET, PIN_CLK_BIT );
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_CLK_BIT );
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    asm volatile ("nop");
    } //end for      
 
  WRITE_PERI_REG( PIN_OUT_SET, PIN_LE_BIT );
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  asm volatile ("nop");
  WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_LE_BIT );
  timer1_write(tubeTime[pos]);
  pos++; if (pos >= maxDigits) pos = 0; 
  //interrupts();
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



void writeDisplaySingle() {}
#endif

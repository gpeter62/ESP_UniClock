#ifdef MM5450

#define MAXBRIGHTNESS 15
const int maxDigits = 6;

//Fill this table with the pin numbers of MM5450 chip!
byte segmentEnablePins[4][8] =  {
                 {25,26,27,28,29,30,31,32},   //segment enable pins of MM5450 (a,b,c,d,e,f,g,DP)
                 {17,18,19,20,21,22,23,24},
                 {9,10,11,12,13,14,15,16},
                 {1,2,3,4,5,6,7,8}
                 };
                 
byte sideEnablePins[] = {34,33};  //segment enable pins of 2x4 LEDS  

//MM5450 control pins
#define PIN_LE    14  // D6 Shift Register Latch Enable
#define PIN_CLK   12  // D7 Shift Register Clock
#define PIN_DATA  13  // D5 Shift Register Data
#define PIN_BR    15  // D8 Shift Register Brightness 
#define PIN_LE_BIT   1<<14  // D6 Shift Register Latch Enable
#define PIN_CLK_BIT  1<<12  // D7 Shift Register Clock
#define PIN_DATA_BIT 1<<13  // D5 Shift Register Data
#define PIN_BR_BIT   1<<15  // D8 Shift Register Brightness 

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
                   B11000110,   //grad  abfg  (16)      
                   B10110100    //%  acdf  (17)
};

#define MAXCHARS sizeof(charDefinition)
#define MAXSEGMENTS 8

#define VFDrefresh 3000   //msec, Multiplex time period. Greater value => slower multiplex frequency

byte bitBuffer[36];

void setup_pins() {
#if defined(ESP8266) 
#else
  #error "Board is not supported!"  
#endif
  
  DPRINTLN("MM5450 Clock - Setup pins...");
  pinMode(PIN_LE,  OUTPUT);
  pinMode(PIN_BR,  OUTPUT); 
  pinMode(PIN_DATA,OUTPUT);
  pinMode(PIN_CLK, OUTPUT);
  digitalWrite(PIN_BR,HIGH);  //brightness
  digitsOnly = false;
  startTimer();
  
}

void ICACHE_RAM_ATTR writeBits() {
//https://sub.nanona.fi/esp8266/timing-and-ticks.html
//One tick is 1us / 80 = 0.0125us = 12.5ns on 80MHz
//One tick is 1us / 80 = 0.0125us = 6.25ns on 160MHz
// 1 NOP is 1 tick

  //noInterrupts();  
  WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_LE_BIT );   //DATA_ENABLE
  for (int t=0; t<5;t++) asm volatile ("nop");   //100nsec
  
  for (int i=0; i<36; i++)  {
    if (bitBuffer[i]) {
         WRITE_PERI_REG( PIN_OUT_SET, PIN_DATA_BIT );  //digitalWrite(PIN_DATA,HIGH);
    }
    else {
         WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_DATA_BIT ); //digitalWrite(PIN_DATA,LOW);
      }

    for (int t=0; t<12;t++) asm volatile ("nop");   //300nsec

    WRITE_PERI_REG( PIN_OUT_SET, PIN_CLK_BIT );
    for (int t=0; t<12;t++) asm volatile ("nop");   //300nsec
    WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_CLK_BIT );
    for (int t=0; t<12;t++) asm volatile ("nop");   //300nsec
    } //end for      
 
  WRITE_PERI_REG( PIN_OUT_SET, PIN_LE_BIT );
  //interrupts();
}

void ICACHE_RAM_ATTR writeDisplay(){        // Writes to the MM5450 driver for LEDS
static volatile byte brightCounter = 0;
static byte side = 0;
byte num = 0;

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    timer1_write(VFDrefresh);
    return;  
  }

  if (brightCounter % MAXBRIGHTNESS <= (displayON ?  prm.dayBright : prm.nightBright))
    WRITE_PERI_REG( PIN_OUT_SET, PIN_BR_BIT );   //ON
  else 
    WRITE_PERI_REG( PIN_OUT_CLEAR, PIN_BR_BIT );    //OFF

  brightCounter++;  
  if (brightCounter>MAXBRIGHTNESS) brightCounter = 0;

  memset(bitBuffer,0,sizeof(bitBuffer));   //clear array
  bitBuffer[0] = 1;   //starting bit
  //
  bitBuffer[sideEnablePins[side]] = 0;
  writeBits();
  for (int t=0; t<100;t++) asm volatile ("nop");   //clean display and wait between changing te two sides
  bitBuffer[sideEnablePins[side]] = 0;   //disable old side..
  
  if (side == 0) side = 1; else side = 0;   //change side...
  
  for (int i=0;i<4;i++) {
    num = digit[7-(4*side+i)];
    for (int j=0;j<=7;j++)   //from a to g
      if ((charDefinition[num] & 1<<(7-j)) != 0) {
        bitBuffer[segmentEnablePins[i][j]] = 1; 
        }
    if (digitDP[7-(4*side+i)]) bitBuffer[segmentEnablePins[i][7]] = 1;   //Decimal Point    
  }  //end for

  bitBuffer[sideEnablePins[side]] = 1;   //enable new side
  writeBits();  

  timer1_write(VFDrefresh);
}

void clearTubes() {}
void writeDisplaySingle() {}
#endif

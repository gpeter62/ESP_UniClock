#ifdef MAX7219CNG
char tubeDriver[] = "MAX7219CNG";
int maxDigits = 6;
byte tubes[] = {0,1,2,3,4,5};    //change it, if needed for the correct tube sequence
byte brightConvert[] = {0,1,2,3,4,6,8,10,12,14,15};   //convert brightness from 0..10 to 0..15

//MAX7219CNG control pins
#define PIN_LOAD 15  // D8 LOAD/CS_
#define PIN_CLK  14  // D5 Clock
#define PIN_DIN  13  // D7 DataIN

//MAX7219 Registers
#define REG_SHUTDOWN   0x0C
#define REG_SCANLIMIT  0x0B  
#define REG_DECODE     0x09 
#define REG_INTENSITY  0x0A 
#define REG_TEST       0x0F 

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
                   B11000110,   //grad  (upper circle) abfg  (16)         
                   B10110100,   //%  acdf  (17)    
                   B00111010,   //lower circle cdeg  (18)                   
                   B01100000,   //I  bc    (19)
                   B10001110    //F  aefg  (20)                         
};

//Fill this table with the OUT bits numbers of MAX7219 chip!   I use only No-Decode mode for flexible hardware
byte segmentEnablePins[] =  {6,5,4,3,2,1,0,7};   //segment enable bits of MAX7219 (a,b,c,d,e,f,g,DP)   (You MUST define always 8 bits!!!)

#define MAXCHARS sizeof(charDefinition)
#define MAXSEGMENTS 8

void ICACHE_RAM_ATTR sendBits(byte address,byte val){ 
  digitalWrite(PIN_LOAD, LOW);
  shiftOut(PIN_DIN, PIN_CLK, MSBFIRST, address);
  shiftOut(PIN_DIN, PIN_CLK, MSBFIRST, val);
  digitalWrite(PIN_LOAD, HIGH);
  delay(3);
  //DPRINT(address,HEX); DPRINT(","); DPRINTLN(val,HEX);
}

void setup_pins() {
#if defined(ESP8266) 
#else
  #error "Only 8266 Board is supported!"
#endif
  
  DPRINTLN("MAX7219 Clock - setup pins...");
  pinMode(PIN_LOAD,OUTPUT);   regPin(PIN_LOAD,"PIN_LOAD");
  pinMode(PIN_DIN, OUTPUT);   regPin(PIN_DIN,"PIN_DIN");
  pinMode(PIN_CLK, OUTPUT);   regPin(PIN_CLK,"PIN_CLK");
  digitsOnly = false; 
  
  sendBits(REG_SHUTDOWN,1);              //Set to Normal (not Shutdown) mode
  sendBits(REG_TEST,0);                  //Set to Normal, (Non-Test) mode
  sendBits(REG_SCANLIMIT,maxDigits-1);   //Set scanlimit 
  sendBits(REG_DECODE,0 );               //Set Non-Decode Mode
  sendBits(REG_INTENSITY,0x0F);          //Set maximum brightness
}

void ICACHE_RAM_ATTR writeDisplaySingle() {
static byte oldBright = 0;
static byte newBright = 0;
int bitBuffer;  

//https://sub.nanona.fi/esp8266/timing-and-ticks.html
//One tick is 1us / 80 = 0.0125us = 12.5ns on 80MHz
//One tick is 1us / 80 = 0.0125us = 6.25ns on 160MHz
// 1 NOP is 1 tick


  newBright = brightConvert[displayON ?  prm.dayBright : prm.nightBright];  //convert 0..10 brightness levels to 0..15
  if (!radarON) newBright = 0;
  if (newBright != oldBright) {
    sendBits(REG_INTENSITY, newBright);    //Set Brightness
    oldBright = newBright;
  }
  for (int i=0;i<maxDigits;i++) {
    bitBuffer = 0;
    for (int j=0;j<=7;j++)   //from a to g
      if ((charDefinition[digit[tubes[i]]] & 1<<(7-j)) != 0) {
        bitBuffer |= 1<<segmentEnablePins[j]; 
        }
    if (digitDP[tubes[i]]) bitBuffer |= 1<<segmentEnablePins[7];   //Decimal Point  
    sendBits(i+1, bitBuffer);
  }  //end for i
}

void clearTubes() {}
#endif

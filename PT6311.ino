#ifdef PT6311
//CLOCK_17 
char tubeDriver[] = "PT6311";
int maxDigits = sizeof(digitEnablePins);
int wt = 1;   //Serial timing

//#define PIN_CLK  14   //  Clock
//#define PIN_DIN  13   //  DataIN
//#define PIN_LOAD 15   //  STB pin 

#define SEGMENT8   //if not used, 17 segments or more
//#define TESTMODE   //for testing the segments of a new type VFD modul

//PT6311 Commands

byte DISPLAY_MODE[11] = {
	0B00000000,	//OFF
	0B10001000,	//1
	0B10001001,	//2
	0B10001010,	//3
	0B10001011,	//4
	0B10001100,	//5
	0B10001100,	//6
	0B10001101,	//7
	0B10001110,	//8
	0B10001110,	//9 
	0B10001111,	//10
};

#define C1_DISPLAY_MODE_SETTING     B00000000  //8 digit, 20 segments
#define C2_DATA_SETTING             B01000000  //write data to display, addr increment
#define C2_DATA_SETTING_TESTMODE    B01001000  //write data to display, addr increment
#define C3_ADDRESS_SETTING          B11000000  //Address setting to 0
#define C4_DISPLAY_CONTROL          B10001111  //max brightness, ON

#ifdef SEGMENT8
//------------------abcdefgDP----------------   definition of different characters

#define MAXSEGMENTS 8
boolean ASCIImode = false;
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
                   B11000110,   //grad (upper circle) abfg  (16)         
                   B10110100,   //%  acdf  (17) 
                   B00111010,   //lower circle cdeg  (18)                   
                   B01100000,   //I  bc    (19)
                   B10001110    //F  aefg  (20)                            
};

//byte segmentEnablePins[] =  {9,8,5,3,4,7,6,2};   //segment enable bits (a,b,c,d,e,f,g,DP)   (You MUST define always 8 bits!!!)

#else //16 segment + DP  ---------------------------------------------------------------------------------------------
#define C1_DISPLAY_MODE_SETTING    B00000000  //8 digit, 20 segments
#define MAXSEGMENTS 17
boolean ASCIImode = true;
uint32_t charDefinition[96] = {
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
  0b01100110000000000, /* ~ */
  0b00000000000000000, /* (del) */
};

  //byte segmentEnablePins[] =  {16,7,0,1,2,3,4,5,6,8,9,11,10,12,13,14,15};    //segment enable bits 
#endif


char asciiConvert[] = "0123456789 -.APC~%oIF";

#define MAXCHARS sizeof(charDefinition)/sizeof(charDefinition[0])

void inline shift(byte Data) {
  for (int i=0;i<8;i++) {
    digitalWrite(PIN_CLK,LOW);
    delayMicroseconds(2);
    digitalWrite(PIN_DIN, Data & (1<<i));
    delayMicroseconds(5);
    digitalWrite(PIN_CLK,HIGH);
    delayMicroseconds(5);
    digitalWrite(PIN_CLK,LOW);
    delayMicroseconds(2);
  }
}

void ICACHE_RAM_ATTR sendCommand(byte val){    //for commands C1..C3
  digitalWrite(PIN_LOAD,LOW);
  delayMicroseconds(2);
  shift(val);
  digitalWrite(PIN_LOAD,HIGH);
  delayMicroseconds(2);

  //DPRINT(address,HEX); DPRINT(","); DPRINTLN(val,HEX);
}

void setup_pins() {
  DPRINTLN("PT6311 Clock - Setup pins...");
  pinMode(PIN_LOAD,OUTPUT);   regPin(PIN_LOAD,"PIN_LOAD"); 
  pinMode(PIN_DIN, OUTPUT);   regPin(PIN_DIN,"PIN_DIN"); 
  pinMode(PIN_CLK, OUTPUT);   regPin(PIN_CLK,"PIN_CLK"); 
  digitalWrite(PIN_CLK, HIGH);
  digitalWrite(PIN_LOAD, HIGH);
  digitsOnly = true;
  delay(10);
  sendCommand(C1_DISPLAY_MODE_SETTING); 
}

#if defined(ESP32) 
void IRAM_ATTR writeDisplaySingle(){ 
#else 
void ICACHE_RAM_ATTR writeDisplaySingle(){     
#endif
byte newBright = 0;
uint32_t bitBuffer;  
int dispChar;
static int count = 15;

  newBright = displayON ?  prm.dayBright : prm.nightBright;
  if (newBright>10) newBright = 10;

  digitalWrite(PIN_LOAD, LOW);  //safety only
  delayMicroseconds(2);

  sendCommand(C2_DATA_SETTING); 
  sendCommand(DISPLAY_MODE[newBright]);
  
for (int i=0;i<maxDigits;i++) {
    dispChar = digit[i];
    if (ASCIImode) {
      if (dispChar<sizeof(asciiConvert)) dispChar = asciiConvert[dispChar];       
      dispChar -= 32;                  
    }
    bitBuffer = 0;
    digitalWrite(PIN_LOAD,LOW);
    delayMicroseconds(2);
    shift(C3_ADDRESS_SETTING + i*3);  //set display address
    for (int j=0;j<MAXSEGMENTS;j++)   
      if ((charDefinition[dispChar] & (uint32_t)1<<(MAXSEGMENTS-1-j)) != 0) {
        bitBuffer |= (uint32_t)1<<segmentEnablePins[j]; 
        }
    if (digitDP[i]) bitBuffer |= (uint32_t)1<<segmentEnablePins[7];   //Decimal Point  
    //DPRINT(charDefinition[dispChar],BIN); DPRINT(" / "); DPRINTLN(bitBuffer,BIN);
    if (newBright==0) bitBuffer = 0;  //switch OFF display
    shift(bitBuffer & 0xFF);
    shift(bitBuffer >>8);
    if (MAXSEGMENTS>16) shift(bitBuffer >>16);
    delayMicroseconds(2);
    digitalWrite(PIN_LOAD,HIGH);
    delayMicroseconds(2);
  }  //end for i

/*  //TEST  mode
  digitalWrite(PIN_LOAD, LOW);  //upload data
  delayMicroseconds(2);
  shift(C3_ADDRESS_SETTING); 
  for (int i=0;i<48;i++) {
    shift(0xff); // Data to fill table 5*16 = 80 bits
    //shift(0x00); // Data to fill table 5*16 = 80 bits
    //shift(0xff); // Data to fill table 5*16 = 80 bits
  }
  digitalWrite(PIN_LOAD, HIGH);
  delay(2);
*/  
}

/*********************** Send command without strobe/chip select******************************************/

void writeDisplay() {}
void clearTubes() {}
#endif

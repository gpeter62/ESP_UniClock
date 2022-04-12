#ifdef NEWHV5122  //Nixie driver
//P.S. 6x tubes PCB version

char tubeDriver[] = "HV5122NEW";
//HV5122 control pins
//#define PIN_DIN  17   // DataIn  - chip0 DOUT pin is connected to chip1 DIN pin!
//#define PIN_CLK  22   // Clock
//#define PIN_OE   21   // OutputEnable

//int maxDigits = 6;

//Data pin numbers 100+ means: chip1 pins are used. Chip0-s DOUT is connected to chip1's DIN
/* Example:
  byte digitPins[maxDigits+1][10] = {
    {2,10,9,8,7,6,5,4,3,1},                      //sec  1 , chip0  (tube#0)
    {11,32,20,19,18,17,16,15,14,13},             //sec  10 , chip0 (tube#1)
    {22,31,29,30,27,28,25,26,23,24},             //min   1 , chip0 (tube#2)
    {101,131,110,109,108,107,106,105,104,103},   //min  10 , chip1 (tube#3)
    {111,132,120,119,118,117,116,115,114,113},   //hour  1 , chip1 (tube#4)
    {122,129,130,127,128,125,126,123,124,121},   //hour 10 , chip1 (tube#5)
    {0,12,21,102,112,0,    0,0,0,0}              //extra decimal point (tube0...tube6)
    };   
*/

uint32_t  DRAM_ATTR bitBuffer0 = 0;      //new digits
uint32_t  DRAM_ATTR bitBuffer1 = 0;
uint32_t  DRAM_ATTR oldBitBuffer0 = 0;   //old digits
uint32_t  DRAM_ATTR oldBitBuffer1 = 0;
int DRAM_ATTR animPtr = 0;

#define SHIFT_LSB_FIRST false  //true= LSB first, false= MSB first

//Brightness PWM timing. Greater value => slower brightness PWM frequency  
// Suggested values: 100000 / 20000, for faster PWM: 50000 / 10000
uint32_t DRAM_ATTR PWMrefresh = 100000; 
uint32_t DRAM_ATTR PWM_min = 6000;

uint32_t DRAM_ATTR time1 = 2000;
uint32_t DRAM_ATTR time2 = 2000; 
uint32_t DRAM_ATTR offTime = 2000;
uint32_t DRAM_ATTR brightness = 0;
uint32_t DRAM_ATTR PWMtimeBrightness;
  
#if defined(ESP32)
void IRAM_ATTR shift(uint32_t Data) {
#else
void ICACHE_RAM_ATTR shift(uint32_t Data) {
#endif
  static boolean b;

  for (uint32_t i = 0; i < 32; i++) {
    digitalWrite(PIN_CLK, HIGH);
    for (int t=0;t<20;t++) asm volatile ("nop");
    if (SHIFT_LSB_FIRST)
      b = ((Data & (uint32_t(1) << i))) > 0;  //LSB first
    else
      b = (Data & (uint32_t(1) << (31 - i))) > 0; //MSB first
    digitalWrite(PIN_DIN, b);
    for (int t=0;t<20;t++) asm volatile ("nop");
    digitalWrite(PIN_CLK, LOW);  //falling CLK  to store DIN
    for (int t=0;t<20;t++) asm volatile ("nop");
  }
  digitalWrite(PIN_CLK, HIGH);
  for (int t=0;t<20;t++) asm volatile ("nop");
}

void clearTubes() {
  shift(0);
  shift(0);
}

void setup_pins() {
  DPRINTLN("Setup pins -  HV5122 Nixie driver...");
  DPRINT("- CLK   : GPIO"); DPRINTLN(PIN_CLK);
  DPRINT("- DATAIN: GPIO"); DPRINTLN(PIN_DIN);
  DPRINT("- OUTPUT_ENABLE: GPIO"); DPRINTLN(PIN_OE);
  pinMode(PIN_CLK, OUTPUT);  regPin(PIN_CLK, "PIN_CLK");
  pinMode(PIN_DIN, OUTPUT);  regPin(PIN_DIN, "PIN_DIN");
  pinMode(PIN_OE, OUTPUT);   regPin(PIN_OE, "PIN_OE");
  digitalWrite(PIN_CLK, HIGH);
  digitalWrite(PIN_OE, LOW);
  driverSetupStr = "<br>HV5122 pin settings:<br>";
  for (int i=0;i<maxDigits+1;i++) {
    driverSetupStr += String("{");
    for (int j=0;j<10;j++) {
      driverSetupStr += String(digitPins[i][j]); 
      driverSetupStr += (j==9) ? "}," : ",";
    }
    driverSetupStr += String("<br>");
  }
  clearTubes();
  startTimer();
}


#if defined(ESP32)
void IRAM_ATTR writeDisplay() { //void IRAM_ATTR  writeDisplay(){
#else
void ICACHE_RAM_ATTR writeDisplay() {       //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
#endif
  
  static DRAM_ATTR byte state=0;
  int timer = PWMrefresh;

  intCounter++;
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    #if defined(ESP8266)
      timer1_write(PWMrefresh);
    #endif  
    return;
  }
  
  #if defined(ESP32)
    portENTER_CRITICAL_ISR(&timerMux);
    noInterrupts();
  #endif
  digitalWrite(PIN_OE, LOW); //OFF
  switch (state) {   //state machine...
    case 0:   //show old character
      if (time1>=1000) {
        timer = time1;
        shift(oldBitBuffer1);
        shift(oldBitBuffer0);
        state = 1;
        break;
      }
    case 1:  //show new character
      if (time2>=1000) {
        timer = time2;
        shift(bitBuffer1);
        shift(bitBuffer0);
        if (offTime<500) state = 0;      
        else state = 2;
        break;
      }
    case 2:  //blank display
      timer = offTime;
      state = 3;
      break;
   }  //end switch
 
  if (timer<500) timer = 500;  //safety only...
  
  if ( (state==3) || (!radarON) || (brightness == 0) ) {  //OFF state, blank digit
    digitalWrite(PIN_OE, LOW); //OFF
    state = 0;
    }
  else {  //ON state
    digitalWrite(PIN_OE, HIGH);   //ON
  }
   
#if defined(ESP8266)
  timer1_write(timer);
#elif defined(ESP32)
  portEXIT_CRITICAL_ISR(&timerMux);
  timerAlarmWrite(ESP32timer, timer, true);   
  timerAlarmEnable(ESP32timer);
  interrupts();
#endif
}

void writeDisplaySingle() {
  static byte lastAnimMask[BUFSIZE];
  static byte lastDigit[BUFSIZE];
  static unsigned long lastRun = 0;  
  uint32_t b0,b1,ob0,ob1;  //temp buffers
  byte num;

  if ( (memcmp(digit,lastDigit,maxDigits) == 0) && (memcmp(lastDigit,newDigit,maxDigits) == 0) && (memcmp(animMask,lastAnimMask,maxDigits)==0) ) return;
  //if ((millis()-lastRun)<5) return;
  lastRun = millis();

  memcpy(lastAnimMask,animMask,maxDigits);
  memcpy(lastDigit,digit,maxDigits);

  animPtr = 0;
  b0 = 0; b1 = 0; 
  for (int i = 0; i < maxDigits; i++) { //Set the clock digits new values
    num = digit[i];
    if (animMask[i]>0) {
      animPtr = animMask[i];
      num = newDigit[i];
    }
    if (num < 10) {
      if (digitPins[i][num] < 100) {
        b0 |= (uint32_t)(1 << (digitPins[i][num]-1)); 
        //DPRINT(digitPins[i][num]); DPRINT(":");
        } //chip0
      else {
        b1 |= (uint32_t)(1 << (digitPins[i][num] - 101)); 
        //DPRINT(digitPins[i][num]);DPRINT(":");} //chip1
      }
    }
  }  //end for i

if (animPtr>0) {  
  ob0 = 0; ob1 = 0;
  for (int i = 0; i < maxDigits; i++) { //Set the clock digits old values
    num = oldDigit[i];
    if (num < 10) {
      if (digitPins[i][num] < 100) {
        ob0 |= (uint32_t)(1 << (digitPins[i][num]-1)); 
        //DPRINT(digitPins[i][num]); DPRINT(":");
        } //chip0
      else {
        ob1 |= (uint32_t)(1 << (digitPins[i][num] - 101)); 
        //DPRINT(digitPins[i][num]);DPRINT(":");} //chip1
      }
    }
  }  //end for i
}

  #ifdef  MAKE_BLINKING_DOTS //it means, the extra datapins are used as 4 blinking dot instead of decimal points!
    if (showClock) {
      digitDP[1] = digitDP[2]; //left two dots
      digitDP[3] = digitDP[4]; //right two dots
    }
  #endif
  
  for (int i = 0; i < maxDigits-1; i++) { //Set the extra decimal point dots
    if (digitDP[i] && digitPins[maxDigits][i]>0) {
      if (digitPins[maxDigits][i] < 100) {
        b0  |= (uint32_t)(1 << (digitPins[maxDigits][i]-1)); //chip0
        ob0 |= (uint32_t)(1 << (digitPins[maxDigits][i]-1)); //chip0
      }
      else {
        b1  |= (uint32_t)(1 << (digitPins[maxDigits][i] - 101)); //chip1
        ob1 |= (uint32_t)(1 << (digitPins[maxDigits][i] - 101)); //chip1
      }
    }
  }  //end for i



  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHTNESS) brightness = MAXBRIGHTNESS;  //only for safety

  if (autoBrightness && displayON) 
    PWMtimeBrightness = max(PWM_min, (PWMrefresh*lx)/MAXIMUM_LUX);
  else
    PWMtimeBrightness = max(PWM_min, (PWMrefresh*brightness)/MAXBRIGHTNESS);
   
  if ((animPtr==0) || (PWMtimeBrightness<15000)) {  //no animation #5
    ob0 = b0;
    ob1 = b1;
    animPtr = 10;  //to make equal timing
  }

  time1 = PWMtimeBrightness * (20-animPtr)/20;
  if (time1<1000) time1=0;
  if (time1 > (PWMtimeBrightness-1000)) time1 = PWMtimeBrightness;
  time2 = PWMtimeBrightness - time1;
  offTime = PWMrefresh - time1 - time2;
  
  bitBuffer0 = b0;
  bitBuffer1 = b1;
  oldBitBuffer0 = ob0;
  oldBitBuffer1 = ob1;

  DPRINT("Br:"); DPRINT(brightness);  DPRINT(" A:"); DPRINT(animPtr);  
  DPRINT(" PTB"); DPRINT(PWMtimeBrightness);
  DPRINT(" t1:"); DPRINT(time1); DPRINT(" t2:"); DPRINT(time2); DPRINT(" off:"); DPRINT(offTime);
  DPRINT(" "); DPRINT(b0,HEX); DPRINT(" ");   DPRINT(b1,HEX); DPRINT(" "); 
  DPRINT(ob0,HEX); DPRINT(" ");   DPRINTLN(ob1,HEX);   

} 


//For testing -------------------
/*
void test() {
  digitalWrite(PIN_OE, HIGH);
  clearTubes();
  for (int j=0;j<2;j++)
    for (uint32_t i=0;i<32;i++) {
      DPRINT("D"); DPRINT(i+1); DPRINT(": ");
      shift((uint32_t)(1 << (i))); 
      DPRINTLN((uint32_t)(1 << (i)),HEX);
      delay(3000);
    }
}

void testTubes2() {
while (true) {
   
  DPRINTLN("Testing tubes: ");
  for (int i = 0; i < 10; i++) {
    DPRINT(i); DPRINT(" ");
    for (int j = 0; j < maxDigits; j++) {
      digit[j] = i;
      digitDP[j] = i % 2;
    }
    //changeDigit();
 
    writeDisplaySingle();
    delay(3000);
    yield();
  }
  }  //end while
}

void showBits(uint32_t bits) {
  boolean b;

  for (uint32_t i = 0; i < 32; i++) {
     if (SHIFT_LSB_FIRST)
      b = ((bits & (uint32_t(1) << i))) > 0;  //LSB first
    else
      b = (bits & (uint32_t(1) << (31 - i))) > 0; //MSB first
    if (b) {
      DPRINT("1");
    }
    else {
      DPRINT("0");
    }
  }  //end for
  DPRINT("-");
}

*/
#endif

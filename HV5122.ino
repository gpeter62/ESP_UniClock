#ifdef HV5122  //Nixie driver
//P.S. 6x tubes PCB version

//MAX7219CNG control pins
#define PIN_DIN  12   // DataIn
#define PIN_CLK  13   // Clock
#define PIN_OE   14   // OutputEnable

#define SHIFT_LSB_FIRST true  //true= LSB first, false= MSB first
//chip0 DOUT pin is connected to chip1 DIN pin!

/*
const int maxDigits = 6;
byte digitPins[maxDigits+1][10] = {
  {121,122,123,124,125,126,127,128,129,130},  //sec   1 , chip1
  {111,112,113,114,115,116,117,118,119,120},  //sec  10 , chip1
  {101,102,103,104,105,106,107,108,109,110},  //min   1 , chip1
  {21,22,23,24,25,26,27,28,29,30},            //min  10 , chip0
  {11,12,13,14,15,16,17,18,19,20},            //hour  1 , chip0
  {1,2,3,4,5,6,7,8,9,10},                     //hour 10 , chip0
  {31,32,131,132,0,0,0,0,0,0}                 //extra GL dots
  };    
*/

int PWMrefresh=10000;   //Brightness PWM period. Greater value => slower multiplex frequency

void setup_pins() {
  DPRINTLN("Setup pins -  HV5122 Nixie driver...");
  DPRINT("- CLK   : GPIO"); DPRINTLN(PIN_CLK);
  DPRINT("- DATAIN: GPIO"); DPRINTLN(PIN_DIN);
  DPRINT("- OUTPUT_ENABLE: GPIO"); DPRINTLN(PIN_OE);
  pinMode(PIN_CLK,OUTPUT);  regPin(PIN_CLK,"PIN_CLK");
  pinMode(PIN_DIN,OUTPUT);  regPin(PIN_DIN,"PIN_CLK");
  pinMode(PIN_OE,OUTPUT);   regPin(PIN_OE,"PIN_CLK");
  digitalWrite(PIN_CLK,HIGH);
  digitalWrite(PIN_OE,LOW);
  clearTubes();
  startTimer();
}


#if defined(ESP32)
void IRAM_ATTR writeDisplay(){  //void IRAM_ATTR  writeDisplay(){
#else 
void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
#endif

  static volatile int brightCounter = 1;
  
  #if defined(ESP32)
    portENTER_CRITICAL_ISR(&timerMux);
  #endif
  
  intCounter++;
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    #if defined(ESP8266)  
      digitalWrite(digitEnablePins[pos],LOW); 
      timer1_write(PWMrefresh);
    #elif defined(ESP32)
      portEXIT_CRITICAL(&timerMux);
    #endif    
    return;  
  }
  
  if ((displayON ?  prm.dayBright : prm.nightBright)<brightCounter)  digitalWrite(PIN_OE,HIGH);
  else  digitalWrite(PIN_OE,LOW);
  
  brightCounter++;   if (brightCounter>MAXBRIGHTNESS) brightCounter = 1;
  
  #if defined(ESP8266)    
    timer1_write(timer);
  #elif defined(ESP32)     
    ESP32timer = timerBegin(0, PRESCALER, true);  //set prescaler, true = edge generated signal
    timerAttachInterrupt(ESP32timer, &writeDisplay, true);   
    timerAlarmWrite(ESP32timer, PWMrefresh, false);   
    timerAlarmEnable(ESP32timer);
    portEXIT_CRITICAL_ISR(&timerMux);
  #endif    
}


#if defined(ESP32)
void IRAM_ATTR shift(uint32_t Data){  
#else 
void ICACHE_RAM_ATTR shift(uint32_t Data){      
#endif
  static boolean b;

  for (uint32_t i=0;i<32;i++) {
    digitalWrite(PIN_CLK,HIGH);
    if (SHIFT_LSB_FIRST) 
      b = ((Data & (uint32_t(1)<<i)))>0;      //LSB first
    else                 
      b = (Data & (uint32_t(1)<<(31-i)))>0;   //MSB first
    digitalWrite(PIN_DIN, b);
    digitalWrite(PIN_CLK,LOW);   //falling CLK  to store DIN
  }
  digitalWrite(PIN_CLK,HIGH);
}

void clearTubes() {
  shift(0);
  shift(0);
}

void writeDisplaySingle() {
  static unsigned long lastRun = 0;
  uint32_t bitBuffer0=0;
  uint32_t bitBuffer1=0;
  byte num;

  if ((millis()-lastRun)<500) return;   //for test only, to slow down!!!
  lastRun = millis();
  
  for (int i=0;i<maxDigits;i++) {  //Set the clock digits
    num = digit[i]; 
    if (num<10) {
        if (digitPins[i][num] <100) bitBuffer0 |= (uint32_t)(1<<digitPins[i][num]);  //chip0
        else                        bitBuffer1 |= (uint32_t)(1<<(digitPins[i][num]-100));  //chip1  
      }
  }  //end for i

  for (int i=0;i<4;i++) {   //Set the extra GL dots
    if (colonBlinkState || i<2) {
        if (digitPins[maxDigits][i] <100) bitBuffer0 |= (uint32_t)(1<<digitPins[maxDigits][i]); //chip0
        else                              bitBuffer1 |= (uint32_t)(1<<(digitPins[maxDigits][i]-100)); //chip1  
      }  
  }  //end for i
  
  shift(bitBuffer1);   
  //delayMicroseconds(1);   //for testing on oscilloscope
  shift(bitBuffer0);   
  showBits(bitBuffer1);  showBits(bitBuffer0);   DPRINTLN(" "); 
}
  
void showBits(uint32_t bits) {  
  for (uint32_t i=0;i<32;i++) 
    if (bits & (1<<i)) DPRINT("1"); else DPRINT("0");
  DPRINT("-");
}
#endif

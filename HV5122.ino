#ifdef HV5122  //Nixie driver
//P.S. 6x tubes PCB version

//MAX7219CNG control pins
#define PIN_DIN  12   // DataIn
#define PIN_CLK  13   // Clock
#define PIN_OE   14   // OutputEnable

const int wt = 5;   //Serial timing

const int maxDigits = 6;
byte digitPins[maxDigits][10] = {
  {121,122,123,124,125,126,127,128,129,130},  //sec   1 , chip1
  {111,112,113,114,115,116,117,118,119,120},  //sec  10 , chip1
  {101,102,103,104,105,106,107,108,109,110},           //min   1 , chip1
  {21,22,23,24,25,26,27,28,29,30},  //min  10 , chip0
  {11,12,13,14,15,16,17,18,19,20},  //hour  1 , chip0
  {1,2,3,4,5,6,7,8,9,10}            //hour 10 , chip0
  };    //define here the digit enable pins from 4 to 8

byte GL[] = {31,32,31,32};

int PWMrefresh=10000;   //Multiplex time period. Greater value => slower multiplex frequency

void setup_pins() {
  DPRINTLN("Setup pins -  HV5122 Nixie driver...");
  DPRINT("- CLK   : GPIO"); DPRINTLN(PIN_CLK);
  DPRINT("- DATAIN: GPIO"); DPRINTLN(PIN_DIN);
  DPRINT("- OUTPUT_ENABLE: GPIO"); DPRINTLN(PIN_OE);
  pinMode(PIN_CLK,OUTPUT);
  pinMode(PIN_DIN,OUTPUT);
  pinMode(PIN_OE,OUTPUT);
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
  
  if (COLON_PIN>=0) digitalWrite(COLON_PIN,!colonBlinkState);  // Blink colon pin
  
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


void inline shift(uint32_t Data) {
  for (int i=0;i<32;i++) {
    digitalWrite(PIN_CLK,HIGH);
    digitalWrite(PIN_DIN, Data & (1<<i));
    digitalWrite(PIN_CLK,LOW);
  }
}

void clearTubes() {
  shift(0);
}


void writeDisplaySingle() {
  uint32_t bitBuffer=0;
  byte num;

  for (int i=0;i<maxDigits;i++) {
    num = digit[i]; 
    if (num<10) {
        if (digitPins[i][num] <100) bitBuffer |= 1<<digitPins[i][num];      //chip0
        else                        bitBuffer |= 1<<(digitPins[i][num]+16); //chip1  
      }  

  // write GL pins... ???
  }  //end for i
  shift(bitBuffer);   //DPRINTLN(bitBuffer,BIN);
}
#endif

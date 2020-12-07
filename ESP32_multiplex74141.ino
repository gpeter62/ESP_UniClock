#ifdef ESP32_MULTIPLEX74141
//define here the digit enable pins from 4 to 8
#define LEFTDECIMAL false   //set true, if decimal point is on the left side on the tube. Else set false!

const byte digitEnablePins[] = {4,16,17,5,18,19};   //ESP32 6x tube Clock
const byte ABCDPins[4] =  {12,27,14,13};   
const int DpPin = 15; // decimalPoint in Nixie tube, set -1, if not used!

const int maxDigits = sizeof(digitEnablePins);

//const byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, is needed (for example: bad tube pin layout)
const int PWMrefresh=11;   //msec, Multiplex time period. Greater value => slower multiplex frequency
#define MAXBRIGHT 10
const int PWMtiming[MAXBRIGHT+1] = {0,1,2,3,4,5,6,7,8,9,10};

void setup_pins() {
  DPRINTLN("Setup pins -  Multiplex 74141 mode...");
  for (int i=0;i<maxDigits;i++) pinMode(digitEnablePins[i], OUTPUT);
  for (int i=0;i<4;i++) pinMode(ABCDPins[i], OUTPUT);
  if (DpPin>=0)  pinMode(DpPin,OUTPUT);  

  //https://savjee.be/2020/01/multitasking-esp32-arduino-freertos/
  
  xTaskCreate(
    writeDisplay,    // Function that should be called
    "writeDisplay",   // Name of the task (for debugging)
    1000,            // Stack size (bytes)
    NULL,            // Parameter to pass
    2,               // Task priority
    NULL             // Task handle
  );
  
   //startTimer();
}


void  writeDisplay(void * pvParameters ){  //void IRAM_ATTR  writeDisplay(){

  static byte pos = 0;
  static volatile byte state=0;
  static int timer = PWMrefresh;
  static byte num,brightness;
  byte DPpos;
  //TickType_t xLastWakeTime;
  //const TickType_t xFrequency = 10000;
  while(true) {
  
  intCounter++;  
/*  
  #if defined(ESP32)
    portENTER_CRITICAL_ISR(&timerMux);
  #endif
*/  

  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    vTaskDelay(100);
      //portEXIT_CRITICAL(&timerMux);
  }

  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHT) brightness = MAXBRIGHT;  //only for safety
  timer = PWMrefresh;

   switch (state) {   //state machine...
    case 0:
      pos++;  if (pos>maxDigits-1)  pos = 0;  //go to the first tube
      
      if (animMask[pos] > 0) { //Animation?
        num = oldDigit[pos];  //show old character
        timer = (PWMtiming[brightness] * (10-animMask[pos]))/10;
        state = 1;  //next state is: show newDigit
      }
      else {
        num = digit[pos];  //show active character
        timer = PWMtiming[brightness];  
        state = 2;  //next state is: BLANK display
      }
      break;
    case 1:  //show new character, if animation
      num =   newDigit[pos];
      timer = (PWMtiming[brightness] * animMask[pos])/10;      
      state = 2;  //default next state is: BLANK display
      break;
    case 2:  //blank display
      timer = PWMrefresh-PWMtiming[brightness];
      state = 3;
      break;
   }  //end switch
   if (timer<1) timer = 1;  //safety only...

 
   //  if ((pos>0) && (num<=9)) num = convert[num];   //tube character conversion, if needed... (maybe bad pin numbering)
   

  if ((brightness == 0) || (state == 3) || (num >9)) {  //blank digit
    state = 0; 
    digitalWrite(digitEnablePins[pos],LOW);  //switch off anode 
    if (DpPin>=0) digitalWrite(DpPin,LOW);
    }
  else {
      for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }
      digitalWrite(digitEnablePins[pos],HIGH);    //switch on the new digit
      if (DpPin>=0) {
        if (LEFTDECIMAL) DPpos = min(maxDigits-1,pos+1); else DPpos = pos;
        if (digitDP[DPpos]) digitalWrite(DpPin,HIGH); //switch ON decimal point, if needed
      }
  }
    
  if (COLON_PIN>=0) {
    if (num==10) digitalWrite(COLON_PIN,LOW);      // Colon pin OFF
    else digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
  }

  /*
  if (DECIMALPOINT_PIN>=0) {
        if (num==10) {
          digitalWrite(DECIMALPOINT_PIN,LOW);
        }
        else {
          if (decimalpointON) digitalWrite(DECIMALPOINT_PIN,HIGH);
        }
  }
*/
  vTaskDelay(timer); 

  } //end_while(true)
}

void clearTubes() {
  for (int i=0;i<maxDigits;i++) digitalWrite(digitEnablePins[i],LOW); 
  if (DpPin>=0) digitalWrite(DpPin,LOW);
}

void writeDisplaySingle() {}
#endif

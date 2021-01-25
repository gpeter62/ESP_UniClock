#ifdef PCF_74141
//PCF8574 I/O expander version, 4..8 tube Nixie Clock, driven by 74141
//define here the digit enable pins from 4 to 8 tubes

//if a digitEnablePort is on PCF chip, add 100 to the port number!   (for example  2-->102)

#define I2C_ADDR 0x20
#define PCF_SDA_PIN 4
#define PCF_SCL_PIN 5

//If a digitEnablePin is on pcf8574, add 100 to the pin number.   (P0 = 100,... P7 = 107)
byte digitEnablePins[] = {100,101,102,103,104,105};    //6 tube nixie driven by PCF 
byte ABCDPins[4] = {14,12,13,2};  //D5,D6,D7,D4 on 8266
#define DP_PIN  16 // decimalPoint on 8266's D0

const int maxDigits = sizeof(digitEnablePins);
#define MAXBRIGHT 10

//const byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, is needed (for example: bad tube pin layout)
int PWMrefresh=15000;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
int PWMtiming[MAXBRIGHT+1] = {0,2000,3000,4000,5000,6000,7000,8000,10000,12000,14000};
int PWMrange = PWMtiming[MAXBRIGHT] - PWMtiming[1];

#if defined(ESP8266) 
#else
  #error "Only 8266 Board is supported!"  
#endif

void ICACHE_RAM_ATTR delayMS(int d) {        //Delay microsec
  for (int i=0;i<d*30;i++) {asm volatile ("nop"); }
}

void ICACHE_RAM_ATTR shiftout(byte in) {
  for (int i=0;i<8;i++) {
    digitalWrite(PCF_SDA_PIN,in  & (1<<(7-i)));
    delayMS(1);
    digitalWrite(PCF_SCL_PIN,HIGH);
    //if (i==0) delayMS(8);
    delayMS(4);
    digitalWrite(PCF_SCL_PIN,LOW);
  }  

  //ACK is coming
  //pinMode(PCF_SDA_PIN,INPUT_PULLUP); 
  digitalWrite(PCF_SDA_PIN,LOW);  //LOW
  delayMS(12);
  digitalWrite(PCF_SCL_PIN,HIGH);  //clock pulse
  delayMS(4); 
  digitalWrite(PCF_SCL_PIN,LOW);
  //pinMode(PCF_SDA_PIN,OUTPUT);
    delayMS(4); 
 }

void ICACHE_RAM_ATTR sendBits(byte address,byte val){ 
  //Serial.print(address,HEX); Serial.print("/"); Serial.println(val);
  
  //void ICACHE_RAM_ATTR startCondition() {
  //digitalWrite(PCF_SDA_PIN,HIGH);   //PCF_SDA_PIN HIGH -> LOW, while PCF_SCL_PIN is HIGH
  //digitalWrite(PCF_SCL_PIN,HIGH);
  //delayMS(4); 
  digitalWrite(PCF_SDA_PIN,LOW);
  delayMS(3); 
  digitalWrite(PCF_SCL_PIN,LOW);
  delayMS(4); 
  
  shiftout(address<<1);   //shift address one bit left, because 0bit is READ/WRITE mode. 0=WRITE
  shiftout(val);
  
  //void ICACHE_RAM_ATTR stopCondition() {
  delayMS(4);
  digitalWrite(PCF_SDA_PIN,LOW);   //PCF_SDA_PIN LOW -> HIGH, while PCF_SCL_PIN is HIGH
  delayMS(2); 
  digitalWrite(PCF_SCL_PIN,HIGH);
  delayMS(4);
  digitalWrite(PCF_SDA_PIN,HIGH);
  delayMS(4); 
}


void setup_pins() {
  DPRINTLN("PCF8574 MULTIPLEX driver");
  for (int i=0;i<maxDigits;i++)  {
    if (digitEnablePins[i]<100) {
      pinMode(digitEnablePins[i], OUTPUT);   //8266 pins setup, if needed
      regPin(digitEnablePins[i],"DIGIT_ENABLE_PIN");
    }
  }
  for (int i=0;i<4;i++)  {
    pinMode(ABCDPins[i], OUTPUT);
    regPin(ABCDPins[i],"74141_ABCD_PIN");
  }  
  pinMode(PCF_SDA_PIN,OUTPUT); regPin(PCF_SDA_PIN,"PCF_SDA_PIN");
  pinMode(PCF_SCL_PIN,OUTPUT); regPin(PCF_SCL_PIN,"PCF_SCL_PIN");
  pinMode(DP_PIN,OUTPUT);  regPin(DP_PIN,"DP_PIN");
  digitalWrite(PCF_SDA_PIN,HIGH);
  digitalWrite(PCF_SCL_PIN,HIGH);
  delay(100); 
  sendBits(I2C_ADDR,0);
  
  startTimer();
  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);
  }

void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  
  static byte pos = 0;
  static byte state=0;
  static int timer = PWMrefresh;
  static byte num,brightness;
  static byte p,DPpos;
  
if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    //digitalWrite(digitEnablePins[pos],LOW); 
    timer1_write(PWMrefresh);
    return;  
  }
  
  intCounter++;
  timer = PWMrefresh;
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHT) brightness = MAXBRIGHT;  //only for safety

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
 
   //  if ((pos>0) && (num<=9)) num = convert[num];   //tube character conversion, if needed... (maybe bad pin numbering)
   
  p  = digitEnablePins[pos];

  if ((brightness == 0) || (state == 3) || (num >9) || (!radarON)) {  //blank digit
    state = 0; 
    for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],HIGH); }
      digitalWrite(DP_PIN,LOW);
    }
  else {
      if (p<100) digitalWrite(p,HIGH);      //switch ON new digit on 8266
      else sendBits(I2C_ADDR,1<<(p-100)) ;  // or on PCF port
      if (LEFTDECIMAL) DPpos = min(maxDigits-1,pos+1); else DPpos = pos;
      if (digitDP[DPpos] && (brightness>0)) digitalWrite(DP_PIN,HIGH); //switch ON decimal point, if needed
      for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }
  }
    
  #if COLON_PIN >= 0
    if (num==10) digitalWrite(COLON_PIN,LOW);      // Colon pin OFF
    else digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
  #endif
  
  #if DECIMALPOINT_PIN >=0 
        if (num==10) {digitalWrite(DECIMALPOINT_PIN,LOW);}
        else digitalWrite(DECIMALPOINT_PIN,decimalpointON); }
  #endif
  
  if (timer<500) timer = 500;  //safety only...
  timer1_write(timer); 
}

void ICACHE_RAM_ATTR clearTubes() {
    digitalWrite(DP_PIN,LOW);
    sendBits(I2C_ADDR,0);
    //for (int i=0;i<4;i++) digitalWrite(ABCDPins[i],HIGH); 
}

void writeDisplaySingle() {}
#endif

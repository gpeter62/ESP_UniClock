#ifdef PCF_MULTIPLEX74141
//PCF8574 I/O expander version, 4..8 tube Nixie Clock, driven by 74141
//define here the digit enable pins from 4 to 8 tubes

//if a digitEnablePort is on PCF chip, add 100 to the port number!   (for example  2-->102)

#define I2C_ADDR 0x20
#define LEFTDECIMAL true   //set true, if decimal point is on the left side on the tube. Else set false!

//If a digitEnablePin is on pcf8574, add 100 to the pin number.   (P0 = 100,... P7 = 107)
const byte digitEnablePins[] = {100,101,102,103,104,105};    //6 tube nixie driven by PCF 
const byte ABCDPins[4] = {14,12,13,2};  //D5,D6,D7,D4 on 8266
const byte DpPin = 16; // decimalPoint on 8266's D0

const int maxDigits = sizeof(digitEnablePins);

//const byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, is needed (for example: bad tube pin layout)
const int PWMrefresh=15000;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
const int PWMtiming[] = {0,2000,3000,4000,5000,6000,7000,8000,10000,12000,14000};
#define MAXBRIGHT 10

#if defined(ESP8266) 
#else
  #error "Board is not supported!"  
#endif

void inline delayMS(int d) {
  for (int i=0;i<d*7;i++) {asm volatile ("nop"); }
}

void ICACHE_RAM_ATTR shiftout(byte in) {
  for (int i=0;i<8;i++) {
    digitalWrite(SDA,in  & (1<<(7-i)));
    delayMS(1);
    digitalWrite(SCL,HIGH);
    if (i==0) delayMS(4);
    delayMS(1);
    digitalWrite(SCL,LOW);
  }  

  //ACK is coming
  //pinMode(SDA,INPUT_PULLUP); 
  digitalWrite(SDA,LOW);
  delayMS(1);
  digitalWrite(SCL,HIGH);  //clock pulse
  delayMS(1); 
  digitalWrite(SCL,LOW);
  //pinMode(SDA,OUTPUT);
 }

void ICACHE_RAM_ATTR sendBits(byte address,byte val){ 
  //Serial.print(address,HEX); Serial.print("/"); Serial.println(val);
  
  //void ICACHE_RAM_ATTR startCondition() {
  digitalWrite(SDA,HIGH);
  delayMS(1); 
  digitalWrite(SCL,HIGH);
  delayMS(1); 
  digitalWrite(SDA,LOW);
  delayMS(3); 
  digitalWrite(SCL,LOW);
  delayMS(2); 
  
  shiftout(address<<1);   //shift address one bit left, because 0bit is READ/WRITE mode. 0=WRITE
  shiftout(val);
  
  //void ICACHE_RAM_ATTR stopCondition() {
  delayMS(2); 
  digitalWrite(SDA,LOW);
  digitalWrite(SCL,HIGH);
  delayMS(2); 
  digitalWrite(SDA,HIGH);
  //delayMS(4); 
}


void setup_pins() {
  DPRINTLN("PCF8574 MULTIPLEX driver: Setup pins...");
  DPRINT("I2C SDA: GPIO"); DPRINTLN(SDA);
  DPRINT("I2C SCL: GPIO"); DPRINTLN(SCL);
  for (int i=0;i<maxDigits;i++)  if (digitEnablePins[i]<100) pinMode(digitEnablePins[i], OUTPUT);   //8266 pins setup, if needed
  for (int i=0;i<4;i++)  pinMode(ABCDPins[i], OUTPUT);
  pinMode(SCL,OUTPUT);
  pinMode(SDA,OUTPUT);
  pinMode(DpPin,OUTPUT);  
  digitalWrite(SDA,HIGH);
  digitalWrite(SCL,HIGH);
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
  byte num,brightness;
  byte p,DPpos;
  
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    return;
  }
  
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
   if (timer<200) timer = 200;  //safety only...
 
   //  if ((pos>0) && (num<=9)) num = convert[num];   //tube character conversion, if needed... (maybe bad pin numbering)
   
  p  = digitEnablePins[pos];

  if ((brightness == 0) || (state == 3) || (num >9)) {  //blank digit
    state = 0; 
    for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],HIGH); }
      //if (p<100) digitalWrite(p,LOW); //switch OFF old digit on 8266
      //else sendBits(I2C_ADDR,0);      // or on PCF port  
      digitalWrite(DpPin,LOW);
    }
  else {
      if (p<100) digitalWrite(p,HIGH);      //switch ON new digit on 8266
      else sendBits(I2C_ADDR,1<<(p-100)) ;  // or on PCF port
      if (LEFTDECIMAL) DPpos = min(maxDigits-1,pos+1); else DPpos = pos;
      if (digitDP[DPpos] && (brightness>0)) digitalWrite(DpPin,HIGH); //switch ON decimal point, if needed
      for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }
  }
    
  if (COLON_PIN>=0) {
    if (num==10) digitalWrite(COLON_PIN,LOW);      // Colon pin OFF
    else digitalWrite(COLON_PIN,colonBlinkState);  // Blink colon pin
  }
  
  if (DECIMALPOINT_PIN>=0) {
        if (num==10) {
          digitalWrite(DECIMALPOINT_PIN,LOW);
        }
        else {
          if (decimalpointON) digitalWrite(DECIMALPOINT_PIN,HIGH);
        }
  }
  if (timer<200) timer = 200;  //safety only...
 
  timer1_write(timer); 
}

void ICACHE_RAM_ATTR clearTubes() {
    digitalWrite(DpPin,LOW);
    sendBits(I2C_ADDR,0);
    //for (int i=0;i<4;i++) digitalWrite(ABCDPins[i],HIGH); 
}

void writeDisplaySingle() {}
#endif

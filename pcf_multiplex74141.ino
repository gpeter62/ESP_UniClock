#ifdef PCF_MULTIPLEX74141
//PCF8574 I/O expander version, 4..8 tube Nixie Clock, driven by 74141
//define here the digit enable pins from 4 to 8 tubes

//if a digitEnablePort is on PCF chip, add 100 to the port number!   (for example  2-->102)

#define I2C_ADDR 0x20
//If a digitEnablePin is on pcf8574, add 100 to the pin number.   (P0 = 100,... P7 = 107)
const byte digitEnablePins[] = {100,101,102,103,104,105};    //6 tube nixie driven by PCF 
const byte ABCDPins[4] = {14,12,13,2};  //D5,D6,D7,D4 on 8266
const byte DpPin = 16; // decimalPoint on 8266's D0

int maxDigits = sizeof(digitEnablePins);

//const byte convert[] = {1,0,9,8,7,6,5,4,3,2};   //tube pin conversion, is needed (for example: bad tube pin layout)
const int PWMrefresh=10000;   ////msec, Multiplex time period. Greater value => slower multiplex frequency
const int PWMtiming[] = {1000,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000};
#define MAXBRIGHT 10

void ICACHE_RAM_ATTR delayMS(int d) {
  for (int i=0;i<d*20;i++) {asm volatile ("nop"); }
}

void ICACHE_RAM_ATTR shiftout(byte in) {
  for (int i=0;i<8;i++) {
    digitalWrite(SDA,in  & (1<<(7-i)));
    delayMS(1);
    digitalWrite(SCL,HIGH);
    if (i==0) delayMS(4);
    delayMS(4);
    digitalWrite(SCL,LOW);
  }  

  //ACK is coming
  pinMode(SDA,INPUT_PULLUP); 
  delayMS(4);
  digitalWrite(SCL,HIGH);  //clock pulse
  delayMS(4); 
  digitalWrite(SCL,LOW);
  pinMode(SDA,OUTPUT);
 }

void ICACHE_RAM_ATTR sendBits(byte address,byte val){ 
  //Serial.print(address,HEX); Serial.print("/"); Serial.println(val);
  
  //void ICACHE_RAM_ATTR startCondition() {
  digitalWrite(SDA,HIGH);
  delayMS(2); 
  digitalWrite(SCL,HIGH);
  delayMS(2); 
  digitalWrite(SDA,LOW);
  delayMS(10); 
  digitalWrite(SCL,LOW);
  delayMS(4); 
  
  shiftout(address<<1);   //shift address one bit left, because 0bit is READ/WRITE mode. 0=WRITE
  shiftout(val);
  
  //void ICACHE_RAM_ATTR stopCondition() {
  delayMS(4); 
  digitalWrite(SDA,LOW);
  digitalWrite(SCL,HIGH);
  delayMS(4); 
  digitalWrite(SDA,HIGH);
  delayMS(4); 
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
  timer1_attachInterrupt(writeDisplay);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(PWMrefresh); 
}

void ICACHE_RAM_ATTR writeDisplay(){        //https://circuits4you.com/2018/01/02/esp8266-timer-ticker-example/
  static byte pos = 0;
  static volatile byte state=0;
  static int timer = PWMrefresh;
  static byte num,brightness;
  byte p;
  
  if (EEPROMsaving) {  //stop refresh, while EEPROM write is in progress!
    timer1_write(PWMrefresh);
    return;  
  }
  
  brightness = displayON ?  prm.dayBright : prm.nightBright;
  if (brightness>MAXBRIGHT) brightness = MAXBRIGHT;  //only for safety
  p = digitEnablePins[pos];
  switch (state) {   //state machine...
    case 0:
      if (p<100) digitalWrite(p,LOW); //switch OFF old digit on 8266
      else sendBits(I2C_ADDR,0);      // or on PCF port 
      digitalWrite(DpPin,LOW);        //Switch OFF old decimal point
      
      pos++;  if (pos>maxDigits-1) pos = 0;   //go to the first tube
      for (int i=0;i<1500;i++) {asm volatile ("nop"); }   //long delay to switch off the old digit before switch on the new, depends on hardware
      
      p = digitEnablePins[pos];
      if (p<100) digitalWrite(p,HIGH);      //switch ON new digit on 8266
      else sendBits(I2C_ADDR,1<<(p-100)) ;  // or on PCF port
      if (digitDP[pos] && (brightness>0)) digitalWrite(DpPin,HIGH); //switch ON decimal point, if needed
      state = 2;  //default next state is: BLANK display
      if (animMask[pos] > 0) { //Animation?
        num =   oldDigit[pos];  //show old character
        timer =  (PWMtiming[brightness] * (10-animMask[pos]))/10;
        state = 1;  //next state is: show newDigit
      }
      else {
        num = digit[pos];
        timer = PWMtiming[brightness];  
        if (brightness==MAXBRIGHT) state = 0;
      }
      break;
    case 1:  //show new character, if animation
      num =   newDigit[pos];
      timer = (PWMtiming[brightness] * animMask[pos])/10;      
      if (brightness>=MAXBRIGHT) state = 0;
      else state = 2;  //default next state is: BLANK display
      break;
    case 2:  //blank display
      num = 10;  //blank character
      state = 0;
      timer = PWMrefresh-PWMtiming[brightness];
      digitalWrite(DpPin,LOW);
      break;
  }

  //if ((pos>0) && (num<=9)) num = convert[num];   //tube character conversion, if needed... (maybe bad pin numbering)
  
  if (timer<200) timer = 200;  //safety only...
  if (brightness == 0) {num = 10; timer = PWMtiming[10]; state = 0;}
  for (int i=0;i<4;i++) {digitalWrite(ABCDPins[i],num  & 1<<i); }
  
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
  timer1_write(timer); 
}

void writeDisplaySingle() {}
#endif

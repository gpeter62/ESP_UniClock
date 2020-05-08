// 4digit Nixie Clock (original version with OLED display)
//  2x 74HC595N shift register + 4x 74141

#ifdef NO_MULTIPLEX74141

 //change it, if needed for the correct tube sequence
byte tubes[] = {0,1,2,3};         //4 tubes,   old OLED clock...     
//byte tubes[] = {5,4,3,2,1,0};   //6 tubes, reverse order

int maxDigits = sizeof(tubes);
int PWMrefresh=5000;   // 1000Hz interrupt frequency

#define dataPin  14 
#define latchPin 12 
#define clkPin 13 

void ICACHE_RAM_ATTR writeBits(byte num) {
  
  for (int i=7;i>=0;i--) {
    digitalWrite(dataPin,num  & (1<<i)); 
    for (int t=0; t<2;t++) asm volatile ("nop");   //100nsec
    digitalWrite(clkPin,HIGH);
    for (int t=0; t<2;t++) asm volatile ("nop");   //100nsec
    digitalWrite(clkPin,LOW);
    for (int t=0; t<2;t++) asm volatile ("nop");   //100nsec
    }
}

void ICACHE_RAM_ATTR writeDisplay() {
static volatile int brightCounter = 1;
static volatile boolean oldState = true;
byte brightness;

  digitalWrite(latchPin, LOW);
  for (int i=0;i<maxDigits;i+=2) {
    if ((displayON ?  prm.dayBright : prm.nightBright) < brightCounter)
      writeBits(0xFF);  //BLANK display
    else  
      writeBits((digit[tubes[i+1]]<<4) + digit[tubes[i]]);

   digitalWrite(latchPin, HIGH);     
  }
  
  brightCounter++; 
  if (brightCounter>MAXBRIGHTNESS) brightCounter = 1; 
  timer1_write(PWMrefresh);  
}   

void writeDisplaySingle() {   //if hour or min == 255, BLANK display
}   

void setup_pins(){
  DPRINTLN("Setup pins...");
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin,OUTPUT);
  pinMode(clkPin,OUTPUT);
  timer1_attachInterrupt(writeDisplay);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(PWMrefresh); 
}

#endif

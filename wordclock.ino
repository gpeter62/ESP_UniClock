// NeoPixel Word Clock

#ifdef WORDCLOCK

//#define TEST_WORDCLOCK

#include "font5x7.h"
char tubeDriver[] = "WORDCLOCK";
#define NUMDIGITS 4
int maxDigits = NUMDIGITS;
long neoBrightness;
int colorStep = 1;
int maxPos;
int offsetX = 0; 
int offsetY = 0;
int animOffsetX = 0;
int animOffsetY = 0;
  
enum LEDSTART {LEFTUP, RIGHTUP, LEFTDOWN, RIGHTDOWN };
enum NEO_RGB_FEATURE {RGB, GRB, BRG, BGR};

#define NUMCOLS 5
#define NUMROWS 7
boolean _upsidedown = false;
byte row[NUMROWS];

char asciiConvert[] = "0123456789 -.APC~%oIF";
char DPchar = 0xFF;  //decimal point
byte  dat[NUMDIGITS][NUMROWS];

#define FPS 60 //frame per sec
#define FPS_MSEC 1000/FPS
#define COLORSATURATION 255
#define WHITE_INDEX 192
#define RANDOM_WHEEL_DISTANCE  30  //how far colors will get in random mode
#define RANDOM_MAX_COUNTER 100      //maximum how many times try to found a new color
#define RANDOM_FROM_ALL_PIXELS true  //true or false: when generating new colors, the distance must be calculated from all pixels or only from the actual pixel's color 

RgbColor red(COLORSATURATION, 0, 0);
RgbColor red2(COLORSATURATION/2, 0, 0);
RgbColor green(0, COLORSATURATION, 0);
RgbColor blue(0, 0, COLORSATURATION);
RgbColor purple(COLORSATURATION, 0, COLORSATURATION);
RgbColor white(COLORSATURATION/2,COLORSATURATION/2,COLORSATURATION/2);
RgbColor black(0,0,0);
RgbColor yellow(255, 255, 0);
RgbColor orange(255,140,0);
RgbColor turqouise(0xB1,0xFF,0xEC);
RgbColor pink(0xFF,0xA0,0xB0);

RgbColor dispColor;

//some definitions are in clocks.h   CLOCK_80 and CLOCK_81

#define WORD_MAX sizeof(words)/sizeof(wordDef)  //number of words
#define HOUR_MAX sizeof(hours)/sizeof(wordDef)  //number of words

const int PixelCount = XMAX*YMAX+4; 


//NeoGrbFeature give me BRGW (g and b swapped)
//NeoRgbFeature give me RBGW (g and b swapped)
//NeoBrgFeature give me BGRW (g and r swapped)

#if defined(ESP32)
  byte PixelPin = NEOPIXEL_PIN;  //on ESP32 usable any pin below 32 
  NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s1800KbpsMethod> strip(PixelCount+10,PixelPin);  //instead of NeoEsp32Rmt7Ws2812xMethod the  NeoEsp32I2s1800KbpsMethod is better!
#else
  #define NEOPIXEL_PIN 3
  byte PixelPin = 3;  // on 8266 it MUST use GPIO3 (RX pin)    
  NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount+10);   
#endif
                                                                              
NeoGamma<NeoGammaTableMethod> colorGamma;

RgbColor Wheel(byte WheelPos) {
  if (WheelPos == WHITE_INDEX) return white;
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85)  {
    return RgbColor(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
    return RgbColor(0, WheelPos * 3, 255 - WheelPos * 3);
  } else  {
    WheelPos -= 170;
    return RgbColor(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

int inline convertXYtoPOS(int x, int y) {   //calculate position, depending the first led position
  int pos;
  //DPRINT("X:"); DPRINT(x); DPRINT(" Y:"); DPRINT(y);
  if ((x>XMAX) || (y>YMAX) || (x<1) || (y<1)) {
    DPRINT(x); DPRINT("/"); DPRINTLN(y);
    return(maxPos+5);   //out of area
  }
  x--; y--;   //move position 1,1 to 0,0
   
  switch (START_CORNER) {
    case LEFTUP:
      y = YMAX-y-1;
      if (y%2==0) pos = (y+1)*XMAX - x-1;
      else pos = x + y*XMAX;
      pos = maxPos - pos;  //*prm.ledsPerHold;
      break;
    case RIGHTUP:
      x = XMAX-x-1;
      y = XMAX-y-1;
      if (y%2) pos = (y+1)*XMAX - x-1;
      else pos = x + y*XMAX;
      pos = maxPos - pos;  //*prm.ledsPerHold;
      break;
    case LEFTDOWN:
      if (y%2) pos = (y+1)*XMAX - x-1;
      else pos = x + y*XMAX;
      //pos = pos*prm.ledsPerHold;
      break;
    case RIGHTDOWN:
      x = XMAX-x-1;
      if (y%2) pos = (y+1)*XMAX - x-1;
      else pos = x + y*XMAX;
      //pos = pos*prm.ledsPerHold;    
      break;
    default:
      pos = 0;
  }  //end switch    
  //pos += prm.ledOffset;
  //DPRINT(" POS:"); DPRINTLN(pos); DPRINT(" ");
  return(pos);
} 


//Fill the struct's fields with X,Y and len values
void generateTable(struct wordDef *w,int maxLine) {
  DPRINT("\nNumber of lines:"); DPRINTLN(maxLine);
    char* ret;
    boolean found;
    for (int num=0;num<maxLine;num++) {  //find all words
      found = false;
      DPRINT(w[num].txt);
      for (int j=0;j<YMAX;j++) {    //scan all matrix lines
        ret = strstr(matrix[j],w[num].txt);
        if (ret!=NULL){
          found = true;
          w[num].startX = ret-matrix[j]+1;
          w[num].startY = j+1;
          w[num].len = strlen(w[num].txt);
          DPRINT(" x:"); DPRINT(w[num].startX); DPRINT(" y:"); DPRINT(w[num].startY); DPRINT(" len:"); DPRINTLN(w[num].len);
          break;
        }
      }//endfor j
      if (!found) {
        DPRINTLN(" not found:"); 
      }
    } //endfor i
}

void testSequences() {  //print all hour:min
    DPRINTLN("------------ Testing Expressions --------------------");
    for (int i=0;i<=23;i++) {
      for (int j=0;j<=59;j++) {
        showTime(i,j);
      }
    }
}
void testPanel(){
  int n = 0;
  for (int i=0;i<PixelCount;i++) {strip.SetPixelColor(i,black);} 
  strip.Show();
  for (int i=0;i<PixelCount;i++) {
    strip.SetPixelColor(i,red);
    strip.Show();
    delay(50);
    n++;
    if (n>=XMAX) {
      n = 0;
      for (int i=0;i<PixelCount;i++) {strip.SetPixelColor(i,black);} 
    }
  }
  delay(2000);
}


void setup_pins() {
    DPRINTLN("Setup Word Clock's NeoPixel LEDS");  
    regPin(PixelPin,"NEOPIXEL_PIN");
    maxPos = XMAX * YMAX  - 1;
    DPRINT("Pixel count: "); DPRINTLN(PixelCount);
    DPRINT("Brightness:"); DPRINT(c_MinBrightness); DPRINT(" - "); DPRINTLN(c_MaxBrightness);
    neoBrightness = 50;
    strip.Begin();
    strip.Show();
    strip.SetBrightness(neoBrightness);
    testPanel();
    for (int i=0;i<PixelCount;i++) {strip.SetPixelColor(i,black);} 
    dispColor = Wheel(prm.rgbFixColor);
    int pos;
    for (int i=1;i<=YMAX;i++) {
      pos = convertXYtoPOS(i, i);
      strip.SetPixelColor(pos,red);
      pos = convertXYtoPOS(XMAX-i+1, i);
      strip.SetPixelColor(pos,red);
      strip.Show();
      delay(50);
    }
    for (int i=1;i<=XMAX;i++) {
      pos = convertXYtoPOS(XMAX-i+1, YMAX);
      strip.SetPixelColor(pos,red);
      pos = convertXYtoPOS(i, 1);
      strip.SetPixelColor(pos,red);
      strip.Show();
      delay(50);
    }
    for (int i=1;i<=YMAX;i++) {
      pos = convertXYtoPOS(XMAX, i);
      strip.SetPixelColor(pos,red);
      pos = convertXYtoPOS(1, YMAX-i+1);
      strip.SetPixelColor(pos,red);
      strip.Show();
      delay(50);
    }    
    DPRINTLN("\n--- MATRIX ---");
    for (int i=0;i<YMAX;i++) {
      DPRINTLN(matrix[i]);
    }
    DPRINTLN("\n------ SEQUENCE ------");
    for (int i=0;i<12;i++) {
      DPRINTLN(sequence[i]);
    }
    DPRINTLN("--------------------\n");
    generateTable(words,WORD_MAX);
    generateTable(hours,HOUR_MAX);
    #ifdef TEST_WORDCLOCK
      testSequences();   //print all hour:min
    #endif
}

int clock24to12(int hr) {  
  if (hr==0) 
    hr=12;
  else 
    if(hr>12) hr-=12;
  return(hr);  
}

void firstRun() {   //switch off unused (tube) settings
	prm.enableBlink = false;
	prm.showZero = false;
	prm.enableTimeDisplay = true;  //disable date display
	prm.dateRepeatMin = 0;
	prm.enableDoubleBlink = false;
  prm.interval = 0; //disable cathode protect
}



/*
RgbColor getLedColor(RgbColor in) {
  RgbColor out;
  
  switch (prm.neoRgbFeature) {
    case RGB:
      out = in;
      break;
    case GRB:
      out.R = in.G;    
      out.G = in.R;    
      out.B = in.B;    
      break;
    case BRG:
      out.R = in.B;    
      out.G = in.R;    
      out.B = in.G;    
      break;      
    case BGR:
      out.R = in.B;    
      out.G = in.G;    
      out.B = in.R;    
      break;               
    }
  return (out);
}
*/

void clearScr(){
  if (prm.animMode==0) {
    for (int i=0;i<PixelCount;i++) {
      strip.SetPixelColor(i,black);
    } 
  }
  else {
    for (int y=1;y<=YMAX;y++){
      for (int x=1;x<=XMAX;x++) {strip.SetPixelColor(convertXYtoPOS(x,y),black);}
      strip.Show();
      delay(25);
    } //endfor y
  } //end else
}

void disp(struct wordDef *w) {
  int x1,y1;
  static char tmp[20] = " ";
  
  if (strcmp(tmp,w->txt)!=0)  {
    //DPRINT(w->txt); DPRINT(" ");
    strncpy(tmp,w->txt,sizeof(tmp));
  }
  
  for (int i=0; i<w->len; i++) {
    x1 = w->startX + i + offsetX;
    y1 = w->startY + offsetY;
    if ((x1>=1) && (x1<=XMAX) && (y1>=1) && (y1<=YMAX)) {
      strip.SetPixelColor(convertXYtoPOS(x1,y1),dispColor);  
    }
  } //endfor
}


void dispNum(int num){
  #ifdef TEST_WORDCLOCK
    if ((num<0) || (num>12)) {
      DPRINT("dispNumError:"); DPRINT(num); DPRINT(" ");
    }
    else {
      DPRINT(num); DPRINT(" >> "); DPRINT(hours[num].txt); DPRINT(" ");
    }
  #endif
  disp(&hours[num]);
}

void dispWord(char* w){
  int num=0;
  boolean found = false;
  do {
    if (strcmp(w,words[num].txt)==0) {
      found = true;
      #ifdef TEST_WORDCLOCK
        DPRINT(words[num].txt); DPRINT(" ");
      #endif
      disp(&words[num]);
      return;
    }
    num++;
  } while (num<WORD_MAX);
  #ifdef TEST_WORDCLOCK
    DPRINT("NOT_FOUND:"); DPRINTLN(w);
  #endif
}

void resetOffset(){
  offsetX = -1 * animOffsetX * XMAX;
  offsetY = -1 * animOffsetY * YMAX;
}

void setAnimOffset(){
 switch (prm.animMode) {
    case 0:
    default:
      animOffsetX = 0;    
      animOffsetY = 0;
      offsetX = 0;
      offsetY = 0;
      break;
    case 1:
      animOffsetX = 1;    
      animOffsetY = 0;
      break;
    case 2:
      animOffsetX = -1;    
      animOffsetY = 0;
      break;        
    case 3:
      animOffsetX = 0;    
      animOffsetY = 1;
      break;
    case 4:
      animOffsetX = 0;    
      animOffsetY = -1;
      break;
    case 5:
      animOffsetX = 1;    
      animOffsetY = 1;
      break;
    case 6:
      animOffsetX = -1;    
      animOffsetY = -1;
      break;                          
  }
}  

void displayCathodeProtectShow() {
    offsetX = 0; offsetY = 0;
    dispColor = Wheel(prm.rgbFixColor);
    for (int i=0;i<PixelCount;i++) {strip.SetPixelColor(i,black);} 
    //dispNum(digit[1]);
    writeNumericDisplay(digit[1],-1,true);
    strip.Show();
}

void displayWifiConnecting(){
  char txt[] = "wifi";
  
  dispColor = yellow;
  for (int i=0;i<PixelCount;i++) {strip.SetPixelColor(i,black);} 
  dispWord(txt);
  strip.Show();
}

void displayIpShow() {
    char txt[] = "wifi";
    
      offsetX = 0; offsetY = 0;
      for (int i=2;i>=0;i--) {
        for (int i=0;i<PixelCount;i++) {strip.SetPixelColor(i,black);} 
        dispColor = yellow;
        dispWord(txt);
        strip.Show();
        delay(200);
        for (int j=0;j<=(2-i);j++) {
          strip.SetPixelColor(extraDots[j], blue);  //switch on 1..4 minute leds
        } //endfor
        dispColor = red;
        writeNumericDisplay(digit[i],-1,true);
        //dispNum(digit[i]);
        strip.Show();
        delay(1000);
      } //endfor
      for (int i=0;i<PixelCount;i++) {strip.SetPixelColor(i,black);}
      strip.Show();
      delay(1000);
      resetOffset();  //reset animation position offset
}


void displayClock(){
  static int oldH = 99;
  static int oldM = 99;
  static byte oldAnimMode = 0;
    
  setAnimOffset();
  if ((hour() != oldH) || (minute() != oldM) || (prm.animMode != oldAnimMode)) {
    oldH = hour(); oldM = minute(); oldAnimMode = prm.animMode;
    resetOffset();
    clearScr();
  }
  if ((animOffsetX>0) && (offsetX<0))  offsetX += animOffsetX;  
  else if ((animOffsetX<0) && (offsetX>0))  offsetX += animOffsetX;  
  else offsetX = 0;
 
  if ((animOffsetY>0) && (offsetY<0))  offsetY += animOffsetY;  
  else if ((animOffsetY<0) && (offsetY>0))  offsetY += animOffsetY;  
  else offsetY = 0;        
  //DPRINT("\no:"); DPRINT(offsetX); DPRINT("/"); DPRINT(offsetY); DPRINT("  a:");DPRINT(animOffsetX); DPRINT("/"); DPRINT(animOffsetY);
  showTime(hour(), minute());          
}

void writeNumericDisplay(int num,int dot,boolean lowerDigitOnly) {   //write big numbers to the  led matrix display  (temp,humidity)
  char dispChar;
  int p,pos;
  int st,offset;

  dispColor = askColor(num,dot);
  num = abs(num);
  for (int i=0;i<=1;i++) {   //generate new line
    if (i==0) {
      dispChar = num/10;  //upper digit
      if (dispChar==0) dispChar = ' ';  //space instead of leading zero
    }
    else
      dispChar = num%10;  //lower digit
      
    if (dispChar<sizeof(asciiConvert)) dispChar = asciiConvert[(byte)dispChar]; 
    show(dispChar); 
    for (int k=0;k<NUMROWS;k++) { dat[i][k] = row[k]; }
  } //endfor
  
  //printBitmap();
  if (lowerDigitOnly) {   //move lower digit to center
    offset = 3;
    st = 1;
  }
  else {
    offset = 6;
    st = 0;
  }
  for (int j=0;j<NUMROWS;j++) {
    for (int dig=st;dig<=1;dig++) {
      p = 5;
      for (int b=3;b<=7;b++) {
        if (dat[dig][j] & (1<<b))  {
          int pos =convertXYtoPOS(dig*offset+p,j+3);
          strip.SetPixelColor(pos,dispColor); 
        } 
        p--;
      } //endfor b
    } //endfor i
  } //endfor j

  if (dot>=0) {
    pos = convertXYtoPOS(SENSOR_POSX+dot, SENSOR_POSY);
    strip.SetPixelColor(pos,dispColor);  //switch on sensor led;
  }
  strip.Show();
  delay(50);
}

void displayAlarmShow() {
  static boolean phase = true;
  static unsigned long lastRun = 0;
  int pos;

  if ((millis()-lastRun)<500) {
    return;
  }
  lastRun = millis();
    
  neoBrightness = MAXBRIGHTNESS;
  for (int i=0;i<PixelCount;i++) {strip.SetPixelColor(i,black);} 

  for (int i=1;i<=YMAX;i++) {
      pos = convertXYtoPOS(i, i);
      strip.SetPixelColor(pos,phase ? white : red);
      pos = convertXYtoPOS(XMAX-i+1, i);
      strip.SetPixelColor(pos,phase ? white : red);
  }
  for (int i=1;i<=XMAX;i++) {
      pos = convertXYtoPOS(XMAX-i+1, YMAX);
      strip.SetPixelColor(pos,phase ? white : red);
      pos = convertXYtoPOS(i, 1);
      strip.SetPixelColor(pos,phase ? white : red);
  }
  strip.Show();
  phase = !phase;
}
      
void writeDisplay2() {
  int dot = -1;  
  if (alarmON) {
    displayAlarmShow();
  }
  else if (cathodeProtRunning) {
    displayCathodeProtectShow();
  }
  else if (ipShowRunning) {   //WIFI IP address display
    neoBrightness = prm.dayBright;
    displayIpShow();
  }
  else if (wifiConnectRunning) {
    neoBrightness = prm.dayBright;
    displayWifiConnecting();
  }
  else {   //sensor or clock display
    neoBrightness = displayON ?  prm.dayBright : prm.nightBright;
    if (autoBrightness && (neoBrightness>0)) {
      neoBrightness = (neoBrightness*lx)/long(MAXIMUM_LUX);
      if (neoBrightness< c_MinBrightness) 
            neoBrightness = c_MinBrightness;
    }

    neoBrightness = min(neoBrightness,long(RGB_MAX_BRIGHTNESS));  //safety only   
    if (!radarON) neoBrightness = 0;
    
    if (editorRunning)                         dot = 3;  //editor mode
    else if (newDigit[0] == TEMP_CHARCODE)     dot = 0;  //Celsius
    else if (newDigit[0] == (TEMP_CHARCODE+5)) dot = 1;  //Fahrenheit
    else if (newDigit[0] == PERCENT_CHARCODE)  dot = 2;  //Humidity %
    //DPRINT(newDigit[0]); DPRINT("/"); DPRINT(dot); DPRINT(" ");
    
    if (dot>=0) {
      for (int i=0;i<PixelCount;i++) {strip.SetPixelColor(i,black);} 
      strip.SetBrightness(neoBrightness);
      if (dot==3) writeNumericDisplay(digit[1]*10+digit[0],dot,false);   //date/time editor display
      else if (dot==2) writeNumericDisplay(humid[0],dot,false);    //humidity sensor display
      else writeNumericDisplay(temperature[0],dot,false);          //temperature sensor display
      resetOffset();  //reset animation position offset
    }
    else {
      delay(258 - prm.rgbSpeed);
      displayClock();
    }
  }
}


//--------------    character display  -----------------------------------
void show(byte c){
  byte loccol[NUMCOLS];
  for (byte i=0; i < NUMCOLS; i++)
    loccol[i] = _font[c*NUMCOLS+i];
  transposePattern(loccol,row);
}

void transposePattern(byte cols[], volatile byte rows[]){
  byte cmask;
  cmask = (_upsidedown ? 0x40 : 1);
  for (byte r=0; r < NUMROWS; r++) {
    rows[r] = 0;
    for (byte c=0; c < NUMCOLS; c++) {
      if (cmask&cols[(_upsidedown ? (NUMCOLS-c-1) : c)]) rows[r] |= 1;
      rows[r] = rows[r] << 1;
    }
    if (_upsidedown) cmask = cmask >> 1;
    else cmask = cmask << 1;
    rows[r] = rows[r] << (7-NUMCOLS);
  }
}

void bPrint(byte in) {
  for (int i=7;i>2;i--) {
    boolean b = in & (1<<i);
    DPRINT(b ? '1' : '0');
  }
}

void printBitmap() {  //print bitmap for testing
  for (int j=0;j<NUMROWS;j++) {
    for (int i=0;i<maxDigits;i++) {
      bPrint(dat[i][j]); Serial.print(" ");
    }
    DPRINTLN(" ");
  }
  DPRINTLN("__________________________________");
}

RgbColor askColor(int num, int dot){    //calculate tghe color of the numbers
RgbColor col;

  col = green;

  switch (dot) {
    case 0:  //celsius
      if (num<=0) col = blue;
      else if (num>28) col = red;
      break;
    case 1: //fahrenheit
      if (num<=32) col = blue;
      else if (num>82) col = red;
      break;
    case 2: //humidity
      if (num<40) col = yellow;
      else if (num>70) col = blue;
      break;
    default: 
      col = white;
      break;
  }
  return(col);
}


void showTime(int h,int m){           //write word clock to led display
  static unsigned long lastRun = 0;
  int ptrSeq,ptrWord,pos;
  char ch;
  char tmp[20];
  int m5;
  
  if (lastRun==0) {firstRun(); lastRun = 1;}
  //if ((millis()-lastRun)>200)  {DPRINT(h); DPRINT(":"); DPRINT(m); DPRINT(" ");lastRun = millis();}

  if ((h>23) || (m>59)) return;  //safety only

  dispColor = Wheel(prm.rgbFixColor);

  strip.SetBrightness(neoBrightness);
  #ifdef TEST_WORDCLOCK   
    DPRINTLN(""); DPRINT(h); DPRINT(":"); DPRINT(m); DPRINT(" ");  
  #endif
  memset(tmp,0,sizeof(tmp));
  m5 = m / 5;  //5min step
  ptrSeq = 0;    //pointer to the sequence chars
  ptrWord = 0;  //pointer to the found word chars
  for (int i=0;i<PixelCount;i++) {strip.SetPixelColor(i,black);} 
  
  for (ptrSeq=0; ptrSeq<=strlen(sequence[m5]); ptrSeq++) {  //process sequence of current minute
    ch = sequence[m5][ptrSeq];
    //DPRINTLN(ch);
    tmp[ptrWord] = ch;
    if ((ch == ' ') || (ch == 0)) {  //end of word??
      tmp[ptrWord] = 0; //end char
      //DPRINT("Word:"); DPRINTLN(tmp);
      if (tmp[0]=='#') {   //is it HOUR to show ???
        if (strncmp(tmp,"#2",2)==0) h++;
        h = clock24to12(h);
        dispNum(h);
      }
      else {
          dispWord(tmp);
      }
      ptrWord = 0;
      memset(tmp,0,sizeof(tmp));
    } //endif: end of word found
    else {
      ptrWord++;
    }
  }  //endfor
  if ((offsetX==0) && (offsetY==0)) {  //animation finished???
    if (m>0) {   //valid minute?
      pos = convertXYtoPOS(WEEKDAY_POSX+weekday()-1, WEEKDAY_POSY);
      strip.SetPixelColor(pos,dispColor);  //switch on weekday led

      int dot = m % 5;   //extra dot for 1..4 minutes
      if (dot>0) {
        for (int i=0;i<dot;i++) {
           strip.SetPixelColor(extraDots[i], dispColor);  //switch on 1..4 minute leds
        } //endfor
      } //endif dot
    } //endif m>0
    if (WiFi.status() == WL_CONNECTED) {
      strip.SetPixelColor(convertXYtoPOS(SENSOR_POSX+3, SENSOR_POSY), dispColor);  //  'W' sign for wifi
    }
  } //endif animation finished
  strip.Show();
}



//-------------------------------------------------------------

void clearTubes() {}
void writeDisplaySingle() {}
#endif

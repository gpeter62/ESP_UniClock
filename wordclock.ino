// NeoPixel Word Clock
//
// ============================================================================
// This driver is fundamentally different from the other (tube/VFD/Numitron/
// LED segment) drivers in this codebase: instead of multiplexing digit tubes,
// it lights up individual letters/words on a WS2812 ("NeoPixel") LED matrix
// mounted behind a laser-cut/printed acrylic panel, so the time is spelled
// out as a sentence (e.g. "IT IS HALF PAST TEN") rather than shown as digits.
//
// Hardware: an XMAX x YMAX (currently 11x11, see clocks.h) grid of WS2812
// LEDs wired in a single serpentine ("boustrophedon") strip, plus a handful
// of extra individually-addressed LEDs (weekday row, minute dots, wifi/temp
// indicator) appended after the grid. Driven via NeoPixelBrightnessBus.
// A 5x7 pixel font (font5x7.h) is reused from the tube drivers to render
// small numeric overlays (temperature/humidity/date-time editor) on top of
// the same matrix.
//
// Language variants: the actual word layout (the `matrix[][]` character grid
// used to look up each word's on-screen position) and the HH:MM -> sentence
// templates (`sequence[][]`) are NOT in this file -- they live in clocks.h,
// one block per clock model:
//   CLOCK_80 = English  ("it is quarter past ten")
//   CLOCK_81 = German   ("es ist viertel nach zehn" style, uses "vor/nach halb")
//   CLOCK_82 = Hungarian ("negyed tiz" / "harom negyed tiz" style)
// All three define WORDCLOCK and share this single .ino as their driver;
// only the word/sequence/matrix tables (and START_CORNER wiring direction)
// differ per config. Hungarian time phrasing is grammatically irregular
// (quarters count towards the NEXT hour, e.g. "negyed" = quarter of the
// hour to come, "haromnegyed" = three-quarters/quarter-to) -- see showTime()
// below for how the generic #1/#2 token mechanism copes with that.
//
// Note: wordclock_hun.txt in the repo root is NOT read by this code at
// runtime -- it is a captured serial debug log (DPRINT output) from a past
// firmware build, kept only as human-readable reference documentation of
// the Hungarian matrix/sequence layout, not a data file this .ino loads.
// ============================================================================

#ifdef WORDCLOCK

//#define TEST_WORDCLOCK

#include "font5x7.h"
char tubeDriver[] = "WORDCLOCK";  //identifies this driver in status/web page; "tubeDriver" name is generic/shared across all drivers
#define NUMDIGITS 4
int maxDigits = NUMDIGITS;  //read by the core (changeDigit(), etc.) even though this driver has no physical digit tubes; keeps digit[]/newDigit[] sized for the numeric overlay (temp/humidity/IP/editor)
long neoBrightness;
int colorStep = 1;
int maxPos;   //highest linear pixel index inside the XMAX*YMAX word grid (excludes the extra indicator LEDs appended after it)
int offsetX = 0;   //current animation scroll offset (in grid cells), used to slide the whole sentence in/out of view
int offsetY = 0;
int animOffsetX = 0;  //target scroll direction/step for the current prm.animMode (see setAnimOffset())
int animOffsetY = 0;

enum LEDSTART {LEFTUP, RIGHTUP, LEFTDOWN, RIGHTDOWN };  //physical corner where LED index 0 of the serpentine strip starts -- set per clock model via START_CORNER in clocks.h
enum NEO_RGB_FEATURE {RGB, GRB, BRG, BGR};

#define NUMCOLS 5
#define NUMROWS 7
boolean _upsidedown = false;
byte row[NUMROWS];

char asciiConvert[] = "0123456789 -.APC~%oIF";  //maps a raw digit/code value (used as index) to the matching glyph in font5x7.h, for the numeric overlay display
char DPchar = 0xFF;  //decimal point
byte  dat[NUMDIGITS][NUMROWS];  //rasterized glyph rows for up to NUMDIGITS characters; in practice only dat[0] and dat[1] (the two digits of a number) are ever filled by writeNumericDisplay()

#define FPS 60 //frames per second
#define FPS_MSEC 1000/FPS
#define COLORSATURATION 255
#define WHITE_INDEX 192
#define RANDOM_WHEEL_DISTANCE  30  //minimum wheel-distance a new random color must keep from the existing color(s) to be accepted, in random mode
#define RANDOM_MAX_COUNTER 100      //maximum number of attempts to find a sufficiently distinct new random color before giving up
#define RANDOM_FROM_ALL_PIXELS true  //true or false: when generating a new random color, measure its distance from all pixels' colors, or only from the current pixel's own color

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
//(also CLOCK_82, the Hungarian variant -- see: struct wordDef, XMAX/YMAX, matrix[][],
// sequence[][], hours[], words[], extraDots[], WEEKDAY_POSX/Y, SENSOR_POSX/Y, START_CORNER)

#define WORD_MAX sizeof(words)/sizeof(wordDef)  //number of words
#define HOUR_MAX sizeof(hours)/sizeof(wordDef)  //number of words

//Total addressable pixel count: the XMAX*YMAX word-matrix grid plus 4 extra
//individually-addressed LEDs wired after it (see extraDots[] in clocks.h) used
//for the 1..4 "minute remainder" dots (word clocks only resolve to 5-minute
//steps in words, see showTime()/m5 below -- these dots show the 0..4 leftover minutes).
const int PixelCount = XMAX*YMAX+4;


//NeoGrbFeature gives BRGW (G and B swapped)
//NeoRgbFeature gives RBGW (G and B swapped)
//NeoBrgFeature gives BGRW (G and R swapped)

#if defined(ESP32)
  byte PixelPin = NEOPIXEL_PIN;  //on ESP32 usable any pin below 32 
  NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s1800KbpsMethod> strip(PixelCount+10,PixelPin);  //NeoEsp32I2s1800KbpsMethod is used here because it is more reliable than NeoEsp32Rmt7Ws2812xMethod!
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

//Converts a logical (column,row) letter position in the word matrix -- 1-based,
//x in [1..XMAX], y in [1..YMAX] -- into the linear NeoPixel strip index.
//This is necessary because the physical strip is wired as a single serpentine
//("boustrophedon": row 0 left-to-right, row 1 right-to-left, row 2 left-to-right, ...)
//starting from whichever physical corner START_CORNER says (set per clock model
//in clocks.h, since different panel builds route the data-in wire to a different
//corner). x/y are first normalized to 0-based, then mirrored/flipped as needed for
//the chosen starting corner, then folded into the zig-zag row order, and finally
//(for the "up" corners) the whole index is mirrored via maxPos-pos because index 0
//of the physical strip is at the BOTTOM of the panel even when the logical/visual
//starting corner is at the top.
int inline convertXYtoPOS(int x, int y) {   //calculate position, depending on the first LED's position
  int pos;
  //DPRINT("X:"); DPRINT(x); DPRINT(" Y:"); DPRINT(y);
  if ((x>XMAX) || (y>YMAX) || (x<1) || (y<1)) {
    DPRINT(x); DPRINT("/"); DPRINTLN(y);
    return(maxPos+5);   //out of area: park it just past the real grid+extraDots range (still inside the strip's PixelCount+10 safety buffer) so an out-of-range coordinate can't corrupt another LED nor crash
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
      // POSSIBLE BUG: uses XMAX here where LEFTUP (above) uses YMAX for the same
      // row-flip computation on y, and the y%2 zig-zag test's polarity is also
      // inverted vs. LEFTUP (here "if (y%2)", there "if (y%2==0)") without an
      // obvious reason tied to the extra x-mirroring done for this corner.
      // With the current hardware XMAX==YMAX==11 this is silently harmless,
      // but it looks like a copy/paste-and-tweak mistake from the LEFTUP case
      // rather than an intentional difference, and would break on a
      // non-square panel (XMAX != YMAX). Left unchanged; needs verification
      // against a real RIGHTUP-wired panel (CLOCK_81/CLOCK_82 hardware).
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
//For every entry in a word table (words[] or hours[], defined in clocks.h), find
//where that literal text substring first occurs inside the `matrix[][]` character
//grid (also in clocks.h, one row of text per matrix row) and record its start
//column/row + length. This is how the human-readable word list gets turned into
//pixel coordinates without having to hand-enter X/Y for every word.
//Note: some hour words rely on this "find literal substring" approach in a
//deliberately hacky way to dodge false-positive matches -- e.g. the Hungarian
//hours[] table (clocks.h, CLOCK_82) uses placeholder text like "__1" and
//"TIZEN__1" instead of the real letters "EGY"/"TIZENEGY" ("one"/"eleven"),
//because "EGY" is also a substring of other words in that matrix row (e.g.
//"NEGYED") and strstr() would otherwise lock onto the wrong occurrence. The
//placeholder is unique in the matrix text but still highlights the correct
//physical letters when displayed.
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
    //Debug-only self-test (enabled via #define TEST_WORDCLOCK near the top of
    //this file): walks every possible 24h time (00:00..23:59) through showTime(),
    //which under TEST_WORDCLOCK dumps each resolved word sequence over serial --
    //useful for proofreading every generated sentence in a new language/layout
    //without waiting for the clock to actually reach that minute.
    DPRINTLN("------------ Testing Expressions --------------------");
    for (int i=0;i<=23;i++) {
      for (int j=0;j<=59;j++) {
        showTime(i,j);
      }
    }
}
void testPanel(){  //boot-time self-test: lights each pixel red in strip order, one at a time, wrapping every XMAX pixels -- lets you visually confirm the serpentine wiring/START_CORNER mapping matches the physical panel
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


void setup_pins() {   //this driver's equivalent of the tube drivers' pin/hardware init; called once from the core's setup()
    DPRINTLN("Setup Word Clock's NeoPixel LEDS");
    regPin(PixelPin,"NEOPIXEL_PIN");  //register the GPIO with the core's pin-conflict tracker, as every driver must
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
    //Boot animation: trace the two diagonals, then the outer border, in red,
    //as a further visual wiring/orientation check (see testPanel() above) --
    //purely cosmetic/diagnostic, no effect on later clock operation.
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
    generateTable(words,WORD_MAX);  //resolve X/Y/len for every entry in the general word list (see generateTable() above)
    generateTable(hours,HOUR_MAX);  //resolve X/Y/len for every entry in the hour-number word list (ONE..TWELVE / EIN..ZWOLF / __1..TIZENKETTO)
    #ifdef TEST_WORDCLOCK
      testSequences();   //print all hour:min
    #endif
}

int clock24to12(int hr) {  //convert a (possibly out-of-range, see showTime()'s h++ for "#2" tokens) 24h hour into the 1..12 range used to index hours[]
  if (hr==0)
    hr=12;
  else
    if(hr>12) hr-=12;
  return(hr);
}

void firstRun() {   //switch off (tube-only) settings that don't apply to a word-clock display, e.g. no digit blink/leading-zero-blank/double-blink animation and no cathode-protection exercise cycle exist on a word matrix; called once on first showTime() invocation
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

void clearScr(){   //blank the whole panel; either instantly (static display, animMode 0) or as a row-by-row wipe (any scroll animation mode), so a transition between two times has a visible "clean slate" moment
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

//Light up one word: walk its `len` characters starting at (startX,startY) --
//both filled in by generateTable() from the matrix[][] text -- offset by the
//current scroll animation position (offsetX/offsetY), skipping any character
//that has scrolled outside the visible XMAX x YMAX grid.
void disp(struct wordDef *w) {
  int x1,y1;
  static char tmp[20] = " ";  //NOTE: WORD_LENGTH (clocks.h) is 15 (+1 for NUL) so this 20-byte buffer is safe for all current word tables; if a future language ever needs longer words than 19 chars, this fixed-size buffer (and the one in showTime() below) would need to grow too.

  if (strcmp(tmp,w->txt)!=0)  {
    //DPRINT(w->txt); DPRINT(" ");
    strncpy(tmp,w->txt,sizeof(tmp));  //vestigial: this comparison/copy only fed the (now commented-out) DPRINT above; with that print disabled, tmp no longer affects the drawing below, which always executes unconditionally
  }

  for (int i=0; i<w->len; i++) {
    x1 = w->startX + i + offsetX;
    y1 = w->startY + offsetY;
    if ((x1>=1) && (x1<=XMAX) && (y1>=1) && (y1<=YMAX)) {
      strip.SetPixelColor(convertXYtoPOS(x1,y1),dispColor);
    }
  } //endfor
}


void dispNum(int num){   //display one hour-number word (num must already be in the 1..12 range hours[] is indexed by, see clock24to12())
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

void dispWord(char* w){   //find a word by its literal text in the words[] table (linear search -- table is tiny, so no need for anything smarter) and light it up
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

void resetOffset(){   //place the sentence one full panel-width/height off-screen in the direction it will scroll IN from, so displayClock() can animate it sliding back to offset (0,0)
  offsetX = -1 * animOffsetX * XMAX;
  offsetY = -1 * animOffsetY * YMAX;
}

//Map the generic prm.animMode setting (shared EEPROM setting across all
//drivers) to a scroll direction for this word-clock's slide-in transition.
//animOffsetX/Y only encode the *direction* (-1/0/1 per axis); the actual
//per-frame step size/speed is handled by how often displayClock() is called
//(see writeDisplay2()'s `delay(258 - prm.rgbSpeed)`).
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

//Cathode-protection is a tube-driver concept (periodically cycling digits to
//avoid uneven wear on Nixie/VFD cathodes); a word-clock LED matrix has no such
//wear mechanism, so this word-clock version is a no-op cosmetically: it simply
//shows the current minute's ones-digit as a big number using the numeric-overlay
//font, reusing the same code path/hook the tube drivers use (`cathodeProtRunning`,
//set by the core), for interface parity rather than an actual physical need.
void displayCathodeProtectShow() {
    offsetX = 0; offsetY = 0;
    dispColor = Wheel(prm.rgbFixColor);
    for (int i=0;i<PixelCount;i++) {strip.SetPixelColor(i,black);}
    //dispNum(digit[1]);
    writeNumericDisplay(digit[1],-1,true);
    strip.Show();
}

void displayWifiConnecting(){   //shown while connecting to WiFi (wifiConnectRunning, set by the core): lights the word "wifi" in yellow
  char txt[] = "wifi";

  dispColor = yellow;
  for (int i=0;i<PixelCount;i++) {strip.SetPixelColor(i,black);}
  dispWord(txt);
  strip.Show();
}

//Shown once connected (ipShowRunning, set by the core) to read out the
//assigned IP address: alternates the "wifi" word with each of the last 3
//digit[] entries (populated by the core's showMyIp()) rendered as big numbers,
//one per second, i=2..0 (most- to least-significant of the octet shown).
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
          strip.SetPixelColor(extraDots[j], blue);  //here the 4 "minute" indicator dots are repurposed as a digit-position progress marker (1 dot for the 1st digit shown, up to 3 dots for the 3rd) -- comment text is copy-pasted from showTime()'s actual per-minute usage, this call site's meaning is different
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

void writeNumericDisplay(int num,int dot,boolean lowerDigitOnly) {   //write big numbers to the LED matrix display (temp, humidity)
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

RgbColor askColor(int num, int dot){    //calculate the color of the numbers
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


void showTime(int h,int m){           //write the word-clock sentence to the LED display
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

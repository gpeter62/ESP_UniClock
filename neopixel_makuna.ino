// NeoPixel Tube Lightning

#ifdef USE_NEOPIXEL_MAKUNA

byte neoBrightness;
#define COLORSATURATION 255

RgbColor red(COLORSATURATION, 0, 0);
RgbColor green(0, COLORSATURATION, 0);
RgbColor blue(0, 0, COLORSATURATION);
RgbColor purple(COLORSATURATION, 0, COLORSATURATION);
RgbColor white(COLORSATURATION/2,COLORSATURATION/2,COLORSATURATION/2);
RgbColor black(0,0,0);

int cnt = 0;        // counter for pixel to set to new colour

unsigned int rno = 0x45; // random9 seed
unsigned int cno;       // used to set colours, range 0-0x1FF
unsigned int indexx;     // used to set colours, range 0-0x1FF

int8_t direction; // current direction of dimming

const uint16_t PixelCount = maxDigits; // make sure to set this to the number of pixels in your strip
const uint8_t PixelPin = 3;  // for Esp8266 it MUST be GPIO3 (RX pin)

NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount+2);   //NeoGrbFeature give me BRGW (g and b swapped)
                                                                            //NeoRgbFeature give me RBGW (g and b swapped)
                                                                            //NeoBrgFeature give me BGRW (g and r swapped)
NeoGamma<NeoGammaTableMethod> colorGamma;

void setupNeopixelMakuna() {
    DPRINTLN("Setup NeoPixel LEDS");
    DPRINT("Pixel count: "); DPRINTLN(PixelCount);
    DPRINT("Brightness:"); DPRINT(c_MinBrightness); DPRINT(" - "); DPRINTLN(c_MaxBrightness);
    neoBrightness = prm.rgbBrightness;
    strip.Begin();
    strip.Show();
    strip.SetBrightness(neoBrightness);
    fixColor(prm.rgbFixColor);
    strip.Show();
}

RgbColor Wheel(uint8_t WheelPos) {
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


void rainbow() {
  static uint16_t j=0;
  uint16_t i;    
  
  for(i=0; i<PixelCount; i++) {
    strip.SetPixelColor(i, Wheel((i+j) & 255));
  }
  if (prm.rgbDir) {
    j++; if (j>=256) j=0;
  }
 else {
     j--; if (j<0) j=256;
 }
}

void rainbow2() {
  static int16_t j=0;
  static int16_t i=0;   
  static unsigned long lastRun = 0;
  const int steps = 15;
  unsigned long spd = max(0,steps* maxDigits / PixelCount * (258-prm.rgbSpeed));
  
   if ((millis()-lastRun)<spd) return;
   lastRun = millis();
   
  if (i>=PixelCount) {
    if (j>256) j=0; 
    i=0;
    j+=steps; 
  } //endif

  if (i<0) {
    if (j>256) j=0; 
    i=PixelCount-1;
    j+=steps; 
  } //endif   

  //DPRINT(i); DPRINT("/"); DPRINTLN(j);    
  if (j<256) 
      strip.SetPixelColor(i, Wheel(j));
  else 
      strip.SetPixelColor(i,white);
  
  if (prm.rgbDir)i++;
  else i--;
}



void effect1() {  //color dimmer
  static uint16 j=0;
  static int c = 0;  //actual color
  static int dir = 1;  //direction
  static int counter = 0;
  static byte cnt = 0;
 
  strip.SetBrightness(max(0,prm.rgbBrightness - counter));
  
  for(uint16_t i=0; i<PixelCount; i++) {
    if (c<256) 
      strip.SetPixelColor(i, Wheel(c));
    else 
      strip.SetPixelColor(i,white);
  }

  if (((prm.rgbBrightness - counter)>20) || (cnt%3 == 0)) {
    counter += dir;
  //DPRINTLN(counter);
  }
  
  if (counter <= 0) {
    dir = 1;
  }
  if (counter >= (prm.rgbBrightness-c_MinBrightness)) {
    c = random(0,256);
    dir = -1;
  }
}


void effect2() {   //random color picker
  static int c = 0;  //actual color
  static unsigned long lastRun = random(0,256);
  
  if ((millis()-lastRun)>100*max(0,(258-prm.rgbSpeed))) {
    lastRun = millis();
    c = random(0,256);
  }
  
  for(uint16_t i=0; i<PixelCount; i++) {
    if (c<256) 
      strip.SetPixelColor(i, Wheel(c));
    else 
      strip.SetPixelColor(i,white);
  }
}

void effect3() {
  static const int c[] = {255,5,12,22,30,40,54,62,78,85,100,110,122,137,177,190,210,227,240,256};
  static const int cMax = sizeof(c) / sizeof(c[0]);  //size of array
  static unsigned long lastRun = 0;
  static int newColor = 100;
  static int oldColor = 155;
  static int actColor = 0;
  static int i = 2;
  static int step = 0;
  unsigned long spd = max(0,22* maxDigits / PixelCount * (258-prm.rgbSpeed));

  if ((millis()-lastRun)>spd) {
    if (prm.rgbDir) i++;  else i--;     //goto next pixel

    if (i>=PixelCount) { 
      i=0;
      oldColor = newColor;
      do {newColor = c[random(0,cMax)];   //get a new random color
      } while (newColor == oldColor);
      //DPRINT("***************oldColor:"); DPRINTLN(oldColor);
      //DPRINT("***************newColor:"); DPRINTLN(newColor);
    }
    if (i<0) { 
      i=PixelCount-1; 
      oldColor = newColor;
      do {newColor = c[random(0,cMax)];   //get a new random color
      } while (newColor == oldColor);
      //DPRINT("***************oldColor:"); DPRINTLN(oldColor);
      //DPRINT("***************newColor:"); DPRINTLN(newColor);
    }
    actColor = oldColor;  //starting color} 
    step = max(1,abs(newColor-oldColor)/20);
    lastRun = millis();
  }

  //DPRINT(i); DPRINT("/"); DPRINTLN(j);    
  if (actColor<256) 
      strip.SetPixelColor(i, Wheel(actColor));
  else 
      strip.SetPixelColor(i,white);

  if (newColor != actColor) {   //next rainbow color
    if (oldColor<newColor) actColor = min(newColor,actColor+step);
    else actColor = max(newColor,actColor-step);
    
   }
      //DPRINT("Pix:"); DPRINT(i); DPRINT(" ActCol:"); DPRINTLN(actColor); 
}

void fixColor(int col) {

  for (int i=0;i<PixelCount;i++) {
      if (col==-1) strip.SetPixelColor(i,black);
      else if (col>=256) strip.SetPixelColor(i,white);
      else strip.SetPixelColor(i,Wheel(col));
    } 
}

void doAnimationMakuna() {
static unsigned long lastRun = 0;

  if ((prm.rgbEffect <=1) && ((millis()-lastRun)<1000)) return;  //fix color
  if ((millis()-lastRun)<(258-prm.rgbSpeed)) return;
  lastRun = millis();

  if ((prm.rgbEffect == 0) || !displayON) {   //Night: no RGB lightning
    neoBrightness = 0;
    fixColor(-1);
    strip.Show();
    return;
  }

  neoBrightness = prm.rgbBrightness;
  strip.SetBrightness(neoBrightness);

  if (prm.rgbEffect==1) fixColor(prm.rgbFixColor);
  else if (prm.rgbEffect==2) rainbow(); //flow
  else if (prm.rgbEffect==3) rainbow2();  //stepper
  else if (prm.rgbEffect==4) effect1();  //color dimmer
  else if (prm.rgbEffect==5) effect2();  //color stepper
  else if (prm.rgbEffect==6) effect3();  //color stepflow
  strip.Show();
}

#else
void setupNeopixelMakuna() {}
void doAnimationMakuna() {}
#endif

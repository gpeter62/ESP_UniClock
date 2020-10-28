// NeoPixel Tube Lightning

#ifdef USE_NEOPIXEL_MAKUNA

byte neoBrightness;
#define COLORSATURATION 255

RgbColor red(COLORSATURATION, 0, 0);
RgbColor green(0, COLORSATURATION, 0);
RgbColor blue(0, 0, COLORSATURATION);
RgbColor purple(COLORSATURATION, 0, COLORSATURATION);
RgbColor white(COLORSATURATION);
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
    DPRINTLN("Setup NeoPixel LEDS (Makuna lib):");
    DPRINT("Pixel count: "); DPRINTLN(PixelCount);
    DPRINT("Brightness:"); DPRINT(c_MinBrightness); DPRINT(" - "); DPRINTLN(c_MaxBrightness);
    neoBrightness = prm.dayBright * ((prm.rgbBrightness - c_MinBrightness)/MAXBRIGHTNESS);
    strip.Begin();
    strip.Show();
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

   if ((millis()-lastRun)<10*(255-prm.rgbSpeed)) return;
   lastRun = millis();
   
  if (i>=PixelCount) {
    i=0;
    j+=15; 
  } //endif

  if (i<0) {
    i=PixelCount-1;
    j+=15; 
  } //endif   

  if (j>256) j=0; 

  //DPRINT(i); DPRINT("/"); DPRINTLN(j);    
  if (j<256) 
      strip.SetPixelColor(i, Wheel(j));
  else 
      strip.SetPixelColor(i,white);
      
   if (prm.rgbDir)i++;
   else i--;
}


void fixColor(int col) {

  for (int i=0;i<PixelCount;i++) {
      if (col==-1) strip.SetPixelColor(i,black);
      else if (col==256) strip.SetPixelColor(i,white);
      else strip.SetPixelColor(i,Wheel(col));
    } 
}

void doAnimationMakuna() {
static unsigned long lastRun = 0;

  if ((prm.rgbEffect <=1) && ((millis()-lastRun)<1000)) return;  //fix color
  if ((millis()-lastRun)<(255-prm.rgbSpeed)) return;
  lastRun = millis();
  
  neoBrightness = (displayON ?  prm.dayBright : 0) * ((prm.rgbBrightness - c_MinBrightness)/MAXBRIGHTNESS);
  strip.SetBrightness(neoBrightness);
  if (prm.rgbEffect==0) fixColor(-1);
  else if (prm.rgbEffect==1) fixColor(prm.rgbFixColor);
  else if (prm.rgbEffect==2) rainbow(); 
  else if (prm.rgbEffect==3) rainbow2();
  
  strip.Show();    // This sends the updated pixel colors to the string.
}

#else
void setupNeopixelMakuna() {}
void doAnimationMakuna() {}
#endif

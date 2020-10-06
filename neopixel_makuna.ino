// NeoPixel Tube Lightning


#ifdef USE_NEOPIXEL_MAKUNA

uint8_t c_MinBrightness = 8; 
uint8_t c_MaxBrightness = 80;
byte neoBrightness;

#define COLORSATURATION 255
#define DO_ANIMATION true   //true or false
#define LED_ANIM_SPEED 100

RgbColor red(COLORSATURATION, 0, 0);
RgbColor green(0, COLORSATURATION, 0);
RgbColor blue(0, 0, COLORSATURATION);
RgbColor white(COLORSATURATION);
RgbColor black(0);

int cnt = 0;        // counter for pixel to set to new colour

unsigned int rno = 0x45; // random9 seed
unsigned int cno;       // used to set colours, range 0-0x1FF
unsigned int indexx;     // used to set colours, range 0-0x1FF

int8_t direction; // current direction of dimming

const uint16_t PixelCount = maxDigits; // make sure to set this to the number of pixels in your strip
const uint8_t PixelPin = 3;  // for Esp8266 it MUST be GPIO3 (RX pin)

NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount);   //NeoGrbFeature give me BRGW (g and b swapped)
                                                                            //NeoRgbFeature give me RBGW (g and b swapped)
                                                                            //NeoBrgFeature give me BGRW (g and r swapped)
NeoGamma<NeoGammaTableMethod> colorGamma;

void setupNeopixelMakuna() {
    DPRINTLN("Setup NeoPixel LEDS (Makuna lib):");
    DPRINT("Pixel count: "); DPRINTLN(PixelCount);
    DPRINT("Brightness:"); DPRINT(c_MinBrightness); DPRINT(" - "); DPRINTLN(c_MaxBrightness);
    neoBrightness = prm.dayBright * ((c_MaxBrightness - c_MinBrightness)/MAXBRIGHTNESS);
    strip.Begin();
    strip.Show();
    doAnimation();
    doAnimation();
}

RgbColor Bow(int n) {
byte red,blue,green;

  cno = int(n/64);       // find LED colour (0-6)
  rno = n - (cno*64);    // indexx from LED colour to next colour (0-63)

  switch (cno) {          // update LED colours
      case 0:   // red
      red = 255;
      blue = 0;
      green = rno>>1;     // use shift for div 2
      break;
      
      case 1:   // orange
      red = 255;
      blue = 0;
      green = 32 + ((rno*3)>>1);
      break;
      
      case 2:     // yellow
      red = 255;
      blue = 0;
      green = 128 + (rno*6);

      if(green>255) {
        red -= (green-255);
        green = 255;
      }      
      break;
      
      case 3:     // green
      red = 0;
      green = 255;
      blue = rno*8;
      
      if(blue>255) {
        green -= (blue-255);
        blue = 255;
      }
      break;
      
      case 4:     // blue
      green = 0;
      blue = 255;
      red = rno;
      break;
      
      case 5:     // indigo
      green = 0;
      blue = 255;
      red = 64 + ((11*rno)>>1);

      if(red>255) {
        blue -= (red-255);
        red = 255;
      } 
      break;
      case 6:     // violet
      red = 255;
      green = 0;
      blue = 96 - ((rno*3)>>1);
      break;
      
      default:                  // should not get here
      red = 0;
      green = 0;
      blue = 0;
    }

  return RgbColor(red,green,blue);
}


void rainbow() {
RgbColor c;  

  indexx = cnt;    // set indexx for 1st LED
  //for(int i=0;i<PixelCount;i++){
  for(int i=PixelCount-1;i>=0;i--){
    c = Bow(indexx);
    strip.SetPixelColor(i, colorGamma.Correct(c)); // set colour
    indexx += 4;           // gives 7 steps between LEDS - so 7 colours over 49 LEDs range on string.
    if(indexx>=(7*64)) {indexx =0; }
  }

  cnt ++;   // next step
  if(cnt>=(7*64)) {cnt =0; }
}

void doAnimationMakuna() {
static unsigned long lastRun = 0;

  if ((millis()-lastRun)<LED_ANIM_SPEED) return;
  lastRun = millis();
  
  neoBrightness = (displayON ?  prm.dayBright : 0) * ((c_MaxBrightness - c_MinBrightness)/MAXBRIGHTNESS);
  strip.SetBrightness(neoBrightness);

  if (DO_ANIMATION) {    
    rainbow();
  }
  else {   
    for (int i=0;i<PixelCount;i++) {
      strip.SetPixelColor(i,colorGamma.Correct(red));
    } 
  }
  strip.Show();    // This sends the updated pixel colors to the string.
}

#else
void setupNeopixelMakuna() {}
void doAnimationMakuna() {}
#endif

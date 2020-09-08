// NeoPixel Tube Lightning


#ifdef USE_NEOPIXEL

uint8_t c_MinBrightness = 8; 
uint8_t c_MaxBrightness = 255;
byte neoBrightness = 100;

#define colorSaturation 255

RgbColor red(colorSaturation, 0, 0);
RgbColor green(0, colorSaturation, 0);
RgbColor blue(0, 0, colorSaturation);
RgbColor white(colorSaturation);
RgbColor black(0);

int8_t direction; // current direction of dimming

const uint16_t PixelCount = maxDigits; // make sure to set this to the number of pixels in your strip
const uint8_t PixelPin = 3;  // for Esp8266 it MUST be GPIO3 (RX pin)

NeoPixelBrightnessBus<NeoRgbFeature, Neo800KbpsMethod> strip(PixelCount);
NeoGamma<NeoGammaTableMethod> colorGamma;

void setupNeopixel() {
    DPRINTLN("Setup NeoPixel LEDS:");
    DPRINT("Pixel count: "); DPRINTLN(PixelCount);
    DPRINT("Brightness:"); DPRINT(c_MinBrightness); DPRINT(" - "); DPRINTLN(c_MaxBrightness);
    neoBrightness = prm.dayBright * ((c_MaxBrightness - c_MinBrightness)/MAXBRIGHTNESS);
    strip.Begin();
    strip.Show();
    strip.SetBrightness(neoBrightness);
    for (int i=0;i<PixelCount;i++)
      strip.SetPixelColor(i,colorGamma.Correct(red));
    // fade all pixels providing a tail that is longer the faster
    // the pixel moves.
    strip.Show();
}


void doAnimation() {
if (direction < 0 && neoBrightness <= c_MinBrightness)
    {
      direction = 1;
    }
    else if (direction > 0 && neoBrightness >= c_MaxBrightness)
    {
      direction = -1;
    }
    // apply dimming
    neoBrightness += direction;
    strip.SetBrightness(neoBrightness);

    // show the results
    strip.Show();
}
#else
void setupNeopixel() {}
void doAnimation() {}
#endif

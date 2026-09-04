// ============================================================================
// NeoPixel Tube Backlight subsystem
// ----------------------------------------------------------------------------
// Drives addressable WS2812/WS2812B RGB LEDs mounted behind/around the tubes
// (Nixie/VFD/Numitron/etc.) using the NeoPixelBrightnessBus library (Makuna's
// NeoPixelBus). Each clock config maps its physical LEDs to logical tube
// positions via the `byte tubePixels[]` array defined in clocks.h (see the
// commented examples below), so the same effect code works regardless of how
// many LEDs are wired per tube or in which physical order.
//
// Enabled per-config via #define USE_NEOPIXEL in clocks.h. As of writing this
// is used by the large majority of CLOCK_NN configs that have RGB backlight
// hardware (e.g. CLOCK_4, 6, 8, 15, 17, 18, 22, 26, 30-32, 34-36, 40-46, 48-52,
// 55-57, 57A and others) - grep clocks.h for "#define USE_NEOPIXEL" (not the
// commented-out "//#define" ones) for the authoritative, current list.
// Mutually exclusive in practice with the simpler PWM-based backlight in
// pwmleds.ino (USE_PWMLEDS), which is used instead by configs that only wire
// 3 plain R/G/B PWM pins (e.g. CLOCK_54).
//
// Called from the core program (ESP32_UniClock2.ino):
//   - setupNeopixel()     : once from setup(), after pins/config are known.
//   - doAnimationMakuna() : every loop() iteration, AND from inside Fdelay()'s
//                           busy-wait loop, so animations keep advancing even
//                           while the tube digits are mid-transition (Fdelay
//                           blocks for the tube-switch duration). It rate-limits
//                           itself internally (see FPS_MSEC / prm.rgbSpeed).
//   - darkenNeopixels()   : once from alarmSound(), when the alarm's ring
//                           period naturally times out (not when the alarm is
//                           silenced early via the button).
// ============================================================================

#ifdef USE_NEOPIXEL

long neoBrightness;
int colorStep = 1;

#define FPS 60 //frames per second
#define FPS_MSEC 1000/FPS               //ms per animation frame, used as the minimum interval between doAnimationMakuna() updates (frame-rate cap)
#define COLORSATURATION 255
#define WHITE_INDEX 192                 //reserved "color wheel" position (out of 0..255) that Wheel() special-cases to return pure white instead of a hue
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

//Definition: which pixels are near which tube? Make sure to set all pixels of the strip!
// Valid: 0..(maxDigits-1). If a number is equal to maxDigits or higher, that pixel always stays dark!
//byte tubePixels[] = {0,1,2,3};        //4 tubes, single leds
//byte tubePixels[] = {3,2,1,0};        //4 tubes, single leds, reverse direction
//byte tubePixels[] = {0,9,1,9,2,9,3};  //4 tubes, single leds, 3 leds not used
//byte tubePixels[] = {0,0,1,2,3,3};    //4 tubes, 6 leds
//byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
//byte tubePixels[] = {5,4,3,2,1,0};    //6 tubes, single leds, reverse direction
//byte tubePixels[] = {0,1,2,3,4,5,6,7};    //8 tubes, single leds
//byte tubePixels[] = {3,2,6,1,0};    //Numitron 4 tubes, 4 x single leds + 1. The extra led in the middle is unused and always stays dark!
//byte tubePixels[] = {0,1,2,3,3,2,1,0};  //4 tubes, double row, 8 leds
//byte tubePixels[] = {0,0,1,1,2,2,3,3};  //4 tubes, double row, 8 leds
//byte tubePixels[] = {3,3,2,2,1,1,0,0, 0,0,1,1,2,2,3,3};  //4 tubes, double row, 16 leds (GB)
//byte tubePixels[] = {0,0,1,1,2,2,3,3,3,3,2,2,1,1,0,0,0};  //4 tubes, double row, 17 leds (GP)
//byte tubePixels[] = {0,0,0,1,1,2,2,3,3,3,3,  3,3,2,2,2,1,1,0,0};  //4 tubes, double row, 20 leds (Robi)

// tubePixels is a byte[] so sizeof(tubePixels) already equals its element count; PixelCount is
// set 2 *higher* than that count on purpose, as headroom for a couple of always-dark "spare"
// pixels at the tail of the strip (see the further +4 used when sizing `strip` below). Every loop
// in this file iterates `for (i=0; i<PixelCount; i++)`; use tubeForPixel(i) below instead of
// indexing tubePixels[i] directly, so those last 2 "spare" positions (i == length and
// i == length+1, past the real end of tubePixels[]) resolve to a safe sentinel instead of reading
// past the array. See alarmLight(), rainbow(), effect1(), effect2(), setPixels(), fixColor().
const int PixelCount = sizeof(tubePixels)+2;

// FIXED: centralizes what used to be a direct "tubePixels[i]" array read at 6 call sites (alarmLight(),
// rainbow(), effect1(), effect2(), setPixels(), fixColor()) - those all loop i up to PixelCount-1,
// which is 2 past the end of the real tubePixels[] array (see the PixelCount note above), so a raw
// tubePixels[i] there used to read undefined/out-of-bounds memory for the "spare" positions. This
// helper instead returns a sentinel (255, always >= any real maxDigits/tube-index value) for those
// positions, preserving the original "always dark / never matches a real tube" intent safely.
inline byte tubeForPixel(int i) {
  return (i < (int)sizeof(tubePixels)) ? tubePixels[i] : 255;
}

//NeoGrbFeature gives BRGW (G and B swapped)
//NeoRgbFeature gives RBGW (G and B swapped)
//NeoBrgFeature gives BGRW (G and R swapped)

#if defined(ESP32)
  byte PixelPin = NEOPIXEL_PIN;  //on ESP32 usable any pin below 32
  //Strip is allocated with PixelCount+4 physical pixels (a few more than
  //PixelCount, which is itself already tubePixels-length+2 - see note above)
  //as headroom so the driver never writes past its own frame buffer, even
  //though every loop only ever indexes strip positions 0..PixelCount-1.
  //NeoEsp32I2s1800KbpsMethod drives the strip via the ESP32's I2S peripheral
  //(DMA-fed) instead of RMT; this is more tolerant of interrupts/WiFi jitter
  //than NeoEsp32Rmt7Ws2812xMethod, which can glitch the strict WS2812 bit
  //timing (~1.25us/bit) if an ISR delays the RMT hardware mid-frame.
  NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s1800KbpsMethod> strip(PixelCount+4,PixelPin);  //NeoEsp32I2s1800KbpsMethod is used here because it is more reliable than NeoEsp32Rmt7Ws2812xMethod!
#else
  #define NEOPIXEL_PIN 3
  byte PixelPin = 3;  // on 8266 it MUST use GPIO3 (RX pin)
  //8266 has no I2S/RMT WS2812 helper here; Neo800KbpsMethod bit-bangs the
  //protocol with cycle-counted delays, which is why GPIO3/RX is mandated -
  //it's one of the few pins whose timing isn't disturbed by the UART/boot ROM.
  NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount+4);
#endif

//Gamma-correction lookup table object - NOTE: currently declared but never
//applied (colorGamma.Correct(...) is not called anywhere in this file), so
//colors are sent to the strip linearly, not perceptually corrected. Possible
//leftover from an earlier version / an intended-but-unfinished improvement.
NeoGamma<NeoGammaTableMethod> colorGamma;

void setupNeopixel() {
    DPRINTLN("Setup NeoPixel LEDS");
    regPin(PixelPin,"NEOPIXEL_PIN");
    DPRINT("Pixel count: "); DPRINTLN(PixelCount);
    DPRINT("Brightness:"); DPRINT(c_MinBrightness); DPRINT(" - "); DPRINTLN(c_MaxBrightness);
    neoBrightness = prm.rgbBrightness;
    strip.Begin();     //initialize the underlying I2S/RMT/bit-bang driver and GPIO
    strip.Show();       //push the (still all-black, default-constructed) buffer out so the strip starts dark, not showing stale/garbage pixel data
    strip.SetBrightness(neoBrightness);
    fixColor(prm.rgbFixColor);   //apply the persisted fixed color immediately so the backlight is correct even before the first doAnimationMakuna() tick
}

//Classic Adafruit/NeoPixel "color wheel": maps a single byte (0..255) onto a
//position around the RGB hue circle, producing a smooth rainbow as WheelPos
//increases. Split into 3 arcs of 85 steps (255/3) - red->blue, blue->green,
//green->red - each linearly interpolating between the two arc endpoints
//(hence the `* 3` scaling: 85*3 = 255, so each arc's channel spans the full
//0..255 range). WHITE_INDEX (192) is carved out of the wheel as a special
//sentinel value that instead returns solid white, since a pure hue rotation
//can never itself produce white (R=G=B).
RgbColor Wheel(byte WheelPos) {

  if (WheelPos == WHITE_INDEX)
    return white;

  WheelPos = 255 - WheelPos;   //invert so increasing WheelPos visually rotates the same direction as prm.rgbDir expects
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

//Flashes all tube-backlight pixels white on/off at 2Hz (500ms half-period)
//while the alarm is ringing, overriding whatever normal rgbEffect was active.
//Brightness is hardcoded to 200 here (ignores prm.rgbBrightness/autoBrightness)
//so the alarm flash is always clearly visible regardless of the user's normal
//backlight/dimming settings. Called every doAnimationMakuna() tick while
//alarmON is true (see that function), so the 500ms gate below is what actually
//paces the blink - doAnimationMakuna() itself may be called much more often.
void alarmLight() {
  static unsigned long lastRun = 0;
  static byte counter;

  if ((millis()-lastRun)<500) return;
  lastRun = millis();
  counter++;
  strip.SetBrightness(200);
  for(volatile int i=0; i<PixelCount; i++) {
    if ((counter%2) && tubeForPixel(i)<maxDigits)   //>=maxDigits marks a "no tube here" filler pixel - always kept dark, even during the flash
      strip.SetPixelColor(i, RgbColor(255,255,255));
    else
      strip.SetPixelColor(i,black);
  }
  strip.Show();
}

//Called once from the core's alarmSound() when the alarm's ring period times
//out naturally, to make sure the alarmLight() flashing doesn't get left mid-
//flash (i.e. stuck lit white) - forces brightness to 0 and every pixel black,
//then pushes it out immediately rather than waiting for the next
//doAnimationMakuna() tick to restore the normal effect.
void darkenNeopixels() {
    strip.SetBrightness(0);
    for (int i=0;i<PixelCount;i++) {
      strip.SetPixelColor(i,black);
    }
  strip.Show();
}

//"Flow" rainbow: each physical pixel gets a color offset by its own index i
//plus a global phase j, so a continuous rainbow gradient appears to slide
//across the whole strip. `& 255` wraps the wheel position instead of using
//modulo (index is built from small ints, so this is just a cheap wraparound).
//index==WHITE_INDEX is nudged by +1 to skip the special "solid white" wheel
//position - otherwise a pixel would randomly flash white as the gradient
//scrolls past it, which would look like a glitch rather than part of the flow.
//Speed is controlled purely by how often doAnimationMakuna() calls this
//function (via its own millis() gate), not by anything in here - j advances
//exactly one step per call.
void rainbow() {
 static byte j=0;
 volatile int i;
 int index;

  for(i=0; i<PixelCount; i++) {
    index = (i+j) & 255;
    if (index == WHITE_INDEX) index++;
    if (tubeForPixel(i)<maxDigits) strip.SetPixelColor(i, Wheel(index));
    else strip.SetPixelColor(i,black);
  }
  if (prm.rgbDir) { j++; }
  else { j--; }
  strip.Show();
}

//"Rainbow Stepper": unlike rainbow() which shifts every pixel together every
//frame, this lights one tube (i) at a time with the current wheel color (j),
//then advances to the next tube - so the rainbow visibly steps tube-by-tube
//instead of flowing continuously. Uses its own millis()-based rate limiter
//(spd) on top of doAnimationMakuna()'s, because prm.rgbSpeed here controls the
//per-tube step delay directly rather than the frame rate: `steps * (258-speed)`
//means higher rgbSpeed -> smaller multiplier -> shorter delay -> faster
//stepping. max(0, ...) is defensive: rgbSpeed is expected to stay in 1..255
//per the settings JSON, so (258-speed) should never go negative, but this
//guards against underflow/overflow if it ever does.
void rainbow2() {   //Rainbow Stepper
  static int16_t j=0;
  static int16_t i=0;
  static unsigned long lastRun = 0;
  int steps = 15;
  unsigned long spd = max(0,steps * (258-prm.rgbSpeed));

   if ((millis()-lastRun)<spd) return;
   lastRun = millis();

  if (i>=maxDigits) {   //walked off the end of the tubes -> wrap back to the first tube and advance the color phase by a full "steps" jump
    if (j>255) j=0;
    i=0;
    j+=steps;
  } //endif

  if (i<0) {   //same wrap, but for the reverse (prm.rgbDir==0) direction
    if (j>255) j=0;
    i=maxDigits-1;
    j+=steps;
  } //endif

  //DPRINT(i); DPRINT("/"); DPRINTLN(j);

  setPixels(i, Wheel((j==192) ? j+1 : j));   //skip WHITE_INDEX (192) here too, same reasoning as in rainbow()

  if (prm.rgbDir)i++;
  else i--;

  strip.Show();
}

//All tubes show the same solid color (c), while the overall strip brightness
//ramps down from neoBrightness to c_MinBrightness and back up (a "breathing"
//dimmer), picking a new random color each time it bottoms out. `counter` is
//the amount subtracted from neoBrightness; dir flips its sign at each end of
//the ramp. The `(cnt%3==0)` half of the OR is dead in practice: cnt is a
//static byte that is declared but never incremented anywhere in this
//function, so it is always 0 and `cnt%3==0` is always true - meaning the
//`(neoBrightness-counter)>20` guard is effectively bypassed and counter
//advances by `dir` (i.e. 1) every single call regardless. Net effect: the
//breathing speed is fixed at 1 brightness step per doAnimationMakuna() tick,
//not throttled as the "> 20" condition alone would suggest.
void effect1() {  //color dimmer
  static int c = 0;  //actual color
  static int dir = 1;  //direction
  static long  counter = 0;
  static byte cnt = 0;

  strip.SetBrightness(max(0l,neoBrightness - counter));

  for(volatile int i=0; i<PixelCount; i++) {
    if (tubeForPixel(i)>=maxDigits)
      strip.SetPixelColor(i,black);
    else
      strip.SetPixelColor(i, Wheel(c));
  }

  // POSSIBLE BUG: cnt is never incremented (nor reset), so `cnt%3==0` is
  // always true and this condition always fires - see function comment above.
  if (((neoBrightness - counter)>20) || (cnt%3 == 0)) {
    counter += dir;
  //DPRINTLN(counter);
  }

  if (counter <= 0) {
    dir = 1;   //hit the bright end of the ramp -> start dimming again
  }
  if (counter >= (neoBrightness-c_MinBrightness)) {
    c = random(0,256);   //hit the dim floor -> pick a new random hue and start brightening back up
    dir = -1;
  }
  strip.Show();
}


//All tubes show the same solid color, but the color itself jumps to a new
//random hue every `100*(258-rgbSpeed)` ms - i.e. higher rgbSpeed means shorter
//wait, so colors change faster. Unlike effect1, there's no fade between colors
//here, just an instant switch once the timer elapses.
void effect2() {   //random color picker
  static int c = random(0,256);  //actual color
  static unsigned long lastRun = 0;

  if ((millis()-lastRun)>100*max(0,(258-prm.rgbSpeed))) {
    lastRun = millis();
    c = random(0,256);
  }

  for(volatile int i=0; i<PixelCount; i++) {
    if (tubeForPixel(i)>=maxDigits)
      strip.SetPixelColor(i, black);
    else
      strip.SetPixelColor(i, Wheel(c));
  }
  strip.Show();
}

//Distance between two positions on the 0..255 color wheel, accounting for
//wraparound (the wheel is circular: color 255 is adjacent to color 0). d1 is
//the naive linear distance; d2 is the distance going "the other way around"
//through the wrap point. Returning the smaller of the two gives the true
//shortest hue distance, which the random-color pickers below use to make sure
//a freshly rolled random color is visually distinct from the current one
//(not just numerically different, since e.g. 250 and 5 are close in hue
//despite being far apart as raw numbers).
int colorDistance(int c1,int c2) {
  int d1,d2;
  d1 = abs(c1-c2);
  d2 = (c1<c2) ? c1 + 255-c2 : c2 + 255-c1;
  //DPRINT("c1:"); DPRINT(c1); DPRINT("  c2:"); DPRINT(c2); DPRINT("  d1:"); DPRINT(d1); DPRINT("  d2:"); DPRINTLN(d2);
  return min(d1,d2);
}

//Shared engine behind rgbEffect 6/7/8 ("color stepflow" variants), selected by
//the doAnimationMakuna() dispatcher via two flags:
//  enableRandom=false             -> cycle through the fixed palette table c[]
//  enableRandom=true              -> pick random colors (rejecting ones too
//                                     close to the current color, via
//                                     colorDistance()/RANDOM_WHEEL_DISTANCE)
//  eachPixelRandom=true (implies enableRandom) -> every tube gets its own
//                                     independent target color instead of all
//                                     tubes chasing a single shared color
//For each tube, actColor[] is faded step-by-step from oldColor[] towards
//newColor[] (see the fade loop near the bottom); once a tube's actColor
//catches up to its newColor, the "current tube" pointer i advances (direction
//per prm.rgbDir) and, once it wraps around all tubes (or immediately, in
//eachPixelRandom mode), a new target color is picked. `step` controls how many
//wheel-positions actColor moves per call, chosen so the fade from old to new
//color takes roughly 20 calls regardless of how far apart they are.
void effect3(boolean enableRandom,boolean eachPixelRandom) {
  static const int c[] = {255,5,12,22,30,40,54,62,78,85,100,110,122,137,177,WHITE_INDEX,210,227,240};
  static const int cMax = sizeof(c) / sizeof(c[0]);  //size of array
  //Fixed-size [10] per-tube state arrays, indexed 0..maxDigits-1: only safe as
  //long as no config's maxDigits exceeds 10 (all loops below use maxDigits as
  //the bound, never sizeof(newColor)/sizeof(...), so this isn't re-checked at
  //runtime - a config with more than 10 tubes would silently overrun these).
  static int newColor[10];
  static int oldColor[10];
  static int actColor[10];
  static int i = 2;
  static int step = 1;
  static int idx = 0;
  static boolean firstRun = true;
  static int counter = 0;
  static int dir = 1;
  int newC = 0;
  boolean changeColor = false;
  boolean colorOK;
  
  if (firstRun) {
    firstRun = false;
    for (volatile int k=0;k<maxDigits;k++) {
      oldColor[k] = 0;
      newColor[k] = 100;
      actColor[k] = 0;
    }
    step = max(1,abs(newColor[2]-oldColor[2])/20);
  }
  
  if (newColor[i] == actColor[i]) {      //newColor reached... 
    if (prm.rgbDir) i++;  else i--;   //goto next pixel
    
    if (i>=maxDigits) { i=0; changeColor = true;}
    else if (i<0) {i=maxDigits-1; changeColor = true;}
    
    if (eachPixelRandom) {   //each pixel is random color
      changeColor = true;
    }
    
    if (changeColor) {
      changeColor = false;
      if (enableRandom) {
        counter = 0;
        while (true) {  //random color
          newC = random(0,256);   //get a new random color
          colorOK = true;
          if (RANDOM_FROM_ALL_PIXELS) {
            //require the candidate to be far enough (RANDOM_WHEEL_DISTANCE) from
            //every tube's current target color, not just this tube's - avoids two
            //tubes converging on near-identical hues at the same time
            for (int j=0;j<maxDigits;j++) {
              if (colorDistance(newC,newColor[j]) < RANDOM_WHEEL_DISTANCE)
                colorOK = false;   //note: at this point newColor[j] still holds each tube's previous (not yet reassigned) color
            }
          }
          else {
            if (colorDistance(newC,newColor[i]) < RANDOM_WHEEL_DISTANCE)    //check random only from actual pixel
                colorOK = false;   //note: at this point newColor[i] still holds this tube's previous (not yet reassigned) color
          }
          counter++;
          //escape valve: if no sufficiently-distinct color turns up within
          //RANDOM_MAX_COUNTER tries, give up and use the last candidate anyway
          //rather than risk looping forever (can happen when maxDigits is large
          //relative to 255/RANDOM_WHEEL_DISTANCE, leaving few "free" hues)
          if (colorOK || (counter>RANDOM_MAX_COUNTER))  break;
        } //end while
        //DPRINT("Pix:"); DPRINT(i); DPRINT("  old:"); DPRINT(newColor[i]); DPRINT("  / newC:"); DPRINT(newC); DPRINT("  No of tries:"); DPRINTLN(counter);
      }
      else {  //new color from table
        idx++; if (idx>=cMax) idx = 0;
        newC =  c[idx]; 
      }
      
      if (eachPixelRandom) {
        oldColor[i] = newColor[i]; 
        newColor[i] = newC;
      }
      else { 
        for (volatile int k=0;k<maxDigits;k++) {
          oldColor[k] = newColor[k]; 
          newColor[k] = newC;
          }
      }   
      
    step = max(1,abs(newColor[i]-oldColor[i])/20);
    if (newColor[i] > oldColor[i])  
      dir = 1;
    else
      dir = -1;  
      
    //DPRINT("Old-New simple distance:"); DPRINT(abs(newC-oldColor[i]));  DPRINT("  step:"); DPRINTLN(step);      
    } //endif changeColor
    
    actColor[i] = oldColor[i];  //starting color
    }  //endif (newColor[i] == actColor[i])

  
  if (actColor[i] != newColor[i]) {   //next rainbow color
    if (dir==1) actColor[i] = min(newColor[i],actColor[i]+step);
    else actColor[i] = max(newColor[i],actColor[i]-step);
   }

  for (int j=0;j<maxDigits;j++) 
    setPixels(j, Wheel(actColor[j])); 
     
  strip.Show();     
  //DPRINT("Pix:"); DPRINT(i); DPRINT(" Old:"); DPRINT(oldColor[i]); DPRINT(" ActCol:"); DPRINT(actColor[i]); DPRINT(" New:"); DPRINT(newColor[i]); DPRINT("  Step:"); DPRINTLN(step);
}


//Independent per-tube random color fader: each tube runs its own old->new
//color fade on its own schedule (no shared "current tube" pointer like
//effect3), so all tubes are constantly, independently drifting between random
//hues. Same [10]-element fixed-array-size caveat as effect3 above applies here.
void effect4() {    //every pixel is random changer
  static int newColor[10];
  static int oldColor[10];
  static int actColor[10];
  static int step[10];
  static boolean firstRun = true;
  //unsigned long spd = max(0, 25*(258-prm.rgbSpeed));


  //if ((millis()-lastRun)<spd return;

  if (firstRun) {
    firstRun = false;
    for (int i=0;i<maxDigits;i++) {
      newColor[i] = random(0,256);
      oldColor[i] = random(0,256);
      actColor[i] = oldColor[i];
      step[i] = 1;
    }
  }
  
  for (int t=0;t<maxDigits;t++) {

  if (actColor[t] == newColor[t]) {  //change color
    oldColor[t] = newColor[t];
    do {
      newColor[t] = random(0,256);   //get a new random color
    } while (colorDistance(newColor[t],oldColor[t])<RANDOM_WHEEL_DISTANCE);
    
    actColor[t] = oldColor[t];  //starting color
    step[t] = 1;   //max(1,colorDistance(newColor[t],oldColor[t])/20);
  } //endif changeColor

  //DPRINT(i); DPRINT("/"); DPRINTLN(j);    
  setPixels(t, Wheel(actColor[t]));  

  if (newColor[t] != actColor[t]) {   //next rainbow color
    if (oldColor[t] < newColor[t]) actColor[t] = min(newColor[t],actColor[t]+step[t]);
    else actColor[t] = max(newColor[t],actColor[t]-step[t]);
    
   }
     //DPRINT("Pix:"); DPRINT(i); DPRINT(" ActCol:"); DPRINTLN(actColor); 
  }  //end for t
  strip.Show();
}

//Sets the color of every physical pixel that belongs to logical tube `tubeNo`
//(a tube can be backed by more than one LED - see the tubePixels[] mapping
//examples near the top of this file - so this scans the whole strip rather
//than writing a single index). Does NOT call strip.Show() itself; callers
//batch up several setPixels() calls and Show() once for efficiency.
void setPixels(byte tubeNo, RgbColor c) {
  for (volatile int i=0;i<PixelCount;i++)  {
    if (tubeNo == tubeForPixel(i)) {
      strip.SetPixelColor(i,c);
    }
  }
}

//Sets every tube to a single solid wheel color (rgbEffect==1), or blanks the
//whole strip when col==-1 (used as an "off"/error sentinel - see
//doAnimationMakuna()'s off-branch and its final fallback for invalid
//rgbEffect values).
void fixColor(int col) {

  for (int i=0;i<PixelCount;i++) {
      if ((col == -1) || (tubeForPixel(i)>=maxDigits))
        strip.SetPixelColor(i,black);
      else
        strip.SetPixelColor(i,Wheel(col));
    }
  strip.Show();
}

//Knight Rider "KITT" scanner: a single lit tube (using the fixed
//prm.rgbFixColor, not the rainbow wheel) bounces back and forth across all
//tubes. counter2 divides doAnimationMakuna()'s call rate by 10 so the scanner
//advances one tube every 10 ticks instead of every tick (prm.rgbSpeed is not
//consulted here, unlike most other effects - the bounce speed is fixed).
void kitt() {
  static int dir = 1;  //direction
  static int counter = 0;
  static int counter2 = 0;

  counter2 ++;
  if (counter2 >=10) {
    counter2 = 0;
    counter += dir;
    if (counter >= maxDigits-1) dir = -1;   //bounce off the last tube
    else if (counter <=0 ) dir = 1;   //bounce off the first tube
  }
  for (int i=0;i<PixelCount;i++) {
    strip.SetPixelColor(i,black);   //clear every frame since only one tube is ever lit
  }

  setPixels(counter,Wheel(prm.rgbFixColor));
  strip.Show();
}


void doAnimationMakuna() {
static unsigned long lastRun = 0;

  if (EEPROMsaving) return;
    
  if (alarmON) {
    alarmLight();
    return;
  }
  
  if ((prm.rgbEffect <=1) && ((millis()-lastRun)<1000)) return;  //fix color
  if ((millis()-lastRun)<max(FPS_MSEC,258-prm.rgbSpeed)) return;
  lastRun = millis();

  if ((prm.rgbEffect == 0) || !displayON || !radarON) {   //switch RGB backlight OFF
    neoBrightness = 0;
    fixColor(-1);
    return;
  }
  
  colorStep = max(1,prm.rgbSpeed/5);
  neoBrightness = prm.rgbBrightness;
  if (autoBrightness) {
    neoBrightness = (neoBrightness*lx)/long(MAXIMUM_LUX/2);
    if (neoBrightness< c_MinBrightness) 
            neoBrightness = c_MinBrightness;
  }
  neoBrightness = min(neoBrightness,long(RGB_MAX_BRIGHTNESS));  //safety only
  strip.SetBrightness(neoBrightness);
  //DPRINTLN("  NeoBrightness:"); DPRINT(neoBrightness);
  
  if (prm.rgbEffect==1) fixColor(prm.rgbFixColor);
  else if (prm.rgbEffect==2) rainbow(); //flow
  else if (prm.rgbEffect==3) rainbow2();  //stepper
  else if (prm.rgbEffect==4) effect1();  //color dimmer
  else if (prm.rgbEffect==5) effect2();  //color stepper
  else if (prm.rgbEffect==6) effect3(false,false);  //color stepflow table
  else if (prm.rgbEffect==7) effect3(true,false);  //color stepflow random
  else if (prm.rgbEffect==8) effect3(true,true);  //color stepflow each pixel random
  else if (prm.rgbEffect==9) effect4();  //color stepflow random all pixels
  else if (prm.rgbEffect==10) kitt();  //Knight Rider's KITT car
  else fixColor(-1);   //darken leds, if any error happens
}

#else
void setupNeopixel() {}
void doAnimationMakuna() {}
void darkenNeopixels() {}
#endif

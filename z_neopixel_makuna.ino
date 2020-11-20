// NeoPixel Tube Lightning

#ifdef USE_NEOPIXEL_MAKUNA

byte neoBrightness;
#define COLORSATURATION 255
#define RANDOM_WHEEL_DISTANCE  25  //how far colors will get in random mode
#define RANDOM_MAX_COUNTER 50 //maximum how many times try to found a new color
#define RANDOM_FROM_ALL_PIXELS true  //true or false: when generating new colors, the distance must be calculated from all pixels or only from the actual pixel's color 

RgbColor red(COLORSATURATION, 0, 0);
RgbColor red2(COLORSATURATION/2, 0, 0);
RgbColor green(0, COLORSATURATION, 0);
RgbColor blue(0, 0, COLORSATURATION);
RgbColor purple(COLORSATURATION, 0, COLORSATURATION);
RgbColor white(COLORSATURATION/2,COLORSATURATION/2,COLORSATURATION/2);
RgbColor black(0,0,0);

//Definition: wich pixels are near wich tube?  Make sure to set all pixels of the stripe! 
// Valid: 0..(maxDigits-1) If a number is equal maxDigits or higher, it will stay always dark!
//byte tubePixels[] = {0,1,2,3};        //4 tubes, single leds
//byte tubePixels[] = {0,0,1,2,3,3};    //4 tubes, 6 leds
//byte tubePixels[] = {0,1,2,3,4,5};    //6 tubes, single leds
byte tubePixels[] = {0,1,2,3,4,5,6,7};    //8 tubes, single leds
//byte tubePixels[] = {3,2,6,1,0};    //Numitron 4 tubes, 4 x single leds + 1. The extra led in the middle is not used, is always dark!
//byte tubePixels[] = {0,1,2,3,3,2,1,0};  //4 tubes, double row, 8 leds
//byte tubePixels[] = {0,0,1,1,2,2,3,3};  //4 tubes, double row, 8 leds
//byte tubePixels[] = {3,3,2,2,1,1,0,0, 0,0,1,1,2,2,3,3};  //4 tubes, double row, 16 leds (GB)
//byte tubePixels[] = {0,0,1,1,2,2,3,3,3,3,2,2,1,1,0,0,0};  //4 tubes, double row, 17 leds (GP)
//byte tubePixels[] = {0,0,0,1,1,2,2,3,3,3,3,  3,3,2,2,2,1,1,0,0};  //4 tubes, double row, 20 leds (Robi)

const int PixelCount = sizeof(tubePixels); 

//NeoGrbFeature give me BRGW (g and b swapped)
//NeoRgbFeature give me RBGW (g and b swapped)
//NeoBrgFeature give me BGRW (g and r swapped)

#if defined(ESP32)
  const byte PixelPin = 2;  //on ESP32 usable any pin below 32 
  NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp32I2s1800KbpsMethod> strip(PixelCount+2,PixelPin);
#else
  const byte PixelPin = 3;  // on 8266 it MUST use GPIO3 (RX pin)    
  NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> strip(PixelCount+2);   
#endif
                                                                              
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
}

RgbColor Wheel(byte WheelPos) {
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

void alarmLight() {
  static unsigned long lastRun = 0;   
  static byte counter;
  
  if ((millis()-lastRun)<200) return;
  lastRun = millis();
  counter++;
  strip.SetBrightness(200);
  for(int i=0; i<PixelCount; i++) {
    if (counter%2 && tubePixels[i]<maxDigits)
      strip.SetPixelColor(i, RgbColor(255,255,255));
    else  
      strip.SetPixelColor(i,black);
  }
  strip.Show();
}

void darkenNeopixels() {
    strip.SetBrightness(0);
    for (int i=0;i<PixelCount;i++) {
      strip.SetPixelColor(i,black);
    } 
  strip.Show(); 
}

void rainbow() {
  static int j=0;
  int i;    
  
  for(i=0; i<PixelCount; i++) {
    if (tubePixels[i]<maxDigits) strip.SetPixelColor(i, Wheel((i+j) & 255));
    else strip.SetPixelColor(i,black);
  }
  if (prm.rgbDir) {
    j++; if (j>=256) j=0;
  }
 else {
     j--; if (j<0) j=256;
 }
 strip.Show();
}

void rainbow2() {   //rainbow stepper
  static int16_t j=0;
  static int16_t i=0;   
  static unsigned long lastRun = 0;
  const int steps = 15;
  unsigned long spd = max(0,steps * (258-prm.rgbSpeed));
  
   if ((millis()-lastRun)<spd) return;
   lastRun = millis();
   
  if (i>=maxDigits) {
    if (j>256) j=0; 
    i=0;
    j+=steps; 
  } //endif

  if (i<0) {
    if (j>256) j=0; 
    i=maxDigits-1;
    j+=steps; 
  } //endif   

  //DPRINT(i); DPRINT("/"); DPRINTLN(j);    
  if (j<256) 
      setPixels(i, Wheel(j));
  else 
      setPixels(i,white);
  
  if (prm.rgbDir)i++;
  else i--;
  strip.Show();
}



void effect1() {  //color dimmer
  static int c = 0;  //actual color
  static int dir = 1;  //direction
  static int counter = 0;
  static byte cnt = 0;
 
  strip.SetBrightness(max(0,prm.rgbBrightness - counter));
  
  for(int i=0; i<PixelCount; i++) {
    if (tubePixels[i]>=maxDigits)
      strip.SetPixelColor(i,black);
    else if (c<256) 
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
    c = random(0,257);
    dir = -1;
  }
  strip.Show();
}


void effect2() {   //random color picker
  static int c = random(0,257);  //actual color
  static unsigned long lastRun = 0;
  
  if ((millis()-lastRun)>100*max(0,(258-prm.rgbSpeed))) {
    lastRun = millis();
    c = random(0,257);
  }
  
  for(int i=0; i<PixelCount; i++) {
    if (tubePixels[i]>=maxDigits)
      strip.SetPixelColor(i, black);
    else if (c<256) 
      strip.SetPixelColor(i, Wheel(c));
    else 
      strip.SetPixelColor(i,white);
  }
  strip.Show();
}

void effect3(boolean enableRandom,boolean eachPixelRandom) {
  static const int c[] = {255,5,12,22,30,40,54,62,78,85,100,110,122,137,177,190,210,227,240,256};
  static const int cMax = sizeof(c) / sizeof(c[0]);  //size of array
  static int newColor[maxDigits];
  static int oldColor[maxDigits];
  static int actColor = 0;
  static int i = 2;
  static int step = 1;
  static int idx = 0;
  static boolean firstRun = true;
  static int counter = 0;
  int newC = 0;
  boolean changeColor = false;
  boolean colorOK;
  
  if (firstRun) {
    firstRun = false;
    for (int i=0;i<maxDigits;i++) {
      oldColor[i] = 0;
      newColor[i] = 100;
      actColor = 0;
    }
    step = max(1,abs(newColor[2]-oldColor[2])/20);
  }
  
  if (newColor[i] == actColor) {      //newColor reached... 
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
        while (true) {
          newC = random(0,257);   //get a new random color
          colorOK = true;
          if (RANDOM_FROM_ALL_PIXELS) {
            for (int j=0;j<maxDigits;j++) {
              if (abs(newC-newColor[j])<RANDOM_WHEEL_DISTANCE) 
                colorOK = false;   //here the oldColor is just stored in the newColor... :)
            }    
          }   
          else {  
            if (abs(newC-newColor[i])<RANDOM_WHEEL_DISTANCE)    //check random only from actual pixel
                colorOK = false;   //here the oldColor is just 
          }               
          counter++;
          if (colorOK || (counter>RANDOM_MAX_COUNTER))  break;
        } //end while 
        //DPRINT(abs(actColor-newColor[i])); DPRINT(":"); DPRINTLN(counter);
      }
      else {
        idx++; if (idx>=cMax) idx = 0;
        newC =  c[idx]; 
      }
      //DPRINT("Old-New distance:"); DPRINTLN(abs(newC-newColor[i]));
      if (eachPixelRandom) {
        oldColor[i] = newColor[i]; 
        newColor[i] = newC;
      }
      else { 
        for (int i=0;i<maxDigits;i++) {
          oldColor[i] = newColor[i]; 
          newColor[i] = newC;
          }
      }   
    } //endif changeColor
    
    actColor = oldColor[i];  //starting color} 
    step = max(1,abs(newColor[i]-oldColor[i])/20);
  }  //endif (newColor[i] == actColor)

  if (actColor<256) 
      setPixels(i, Wheel(actColor));
  else 
      setPixels(i,white);

  if (newColor[i] != actColor) {   //next rainbow color
    if (oldColor[i]<newColor[i]) actColor = min(newColor[i],actColor+step);
    else actColor = max(newColor[i],actColor-step);
   }
   //DPRINT("Pix:"); DPRINT(i); DPRINT(" Old:"); DPRINT(oldColor[i]); DPRINT(" ActCol:"); DPRINT(actColor); DPRINT(" New:"); DPRINT(newColor[i]); DPRINT("  Step:"); DPRINTLN(step);
   strip.Show();
}


void effect4() {    //every pixel is random changer
  static unsigned long lastRun = 0;
  static int newColor[maxDigits];
  static int oldColor[maxDigits];
  static int actColor[maxDigits];
  static int step[maxDigits];
  static boolean firstRun = true;
  //unsigned long spd = max(0, 25*(258-prm.rgbSpeed));


  //if ((millis()-lastRun)<spd return;

  if (firstRun) {
    firstRun = false;
    for (int i=0;i<maxDigits;i++) {
      newColor[i] = random(0,257);
      oldColor[i] = random(0,257);
      actColor[i] = oldColor[i];
      step[i] = 1;
    }
  }
  
  for (int t=0;t<maxDigits;t++) {

  if (actColor[t] == newColor[t]) {  //change color
    oldColor[t] = newColor[t];
    do {
      newColor[t] = random(0,257);   //get a new random color
    } while (abs(newColor[t]-oldColor[t])<RANDOM_WHEEL_DISTANCE);
    
    actColor[t] = oldColor[t];  //starting color} 
    step[t] = 1;   //max(1,abs(newColor[t]-oldColor[t])/20);
  } //endif changeColor

  //DPRINT(i); DPRINT("/"); DPRINTLN(j);    
  if (actColor[t]<256) 
      setPixels(t, Wheel(actColor[t]));
  else 
      setPixels(t,white);

  if (newColor[t] != actColor[t]) {   //next rainbow color
    if (oldColor[t] < newColor[t]) actColor[t] = min(newColor[t],actColor[t]+step[t]);
    else actColor[t] = max(newColor[t],actColor[t]-step[t]);
    
   }
     //DPRINT("Pix:"); DPRINT(i); DPRINT(" ActCol:"); DPRINTLN(actColor); 
  }  //end for t
  strip.Show();
}

void setPixels(byte tubeNo, RgbColor c) {
  for (int i=0;i<PixelCount;i++) 
    if (tubeNo == tubePixels[i]) 
      strip.SetPixelColor(i, c);
}

void fixColor(int col) {

  for (int i=0;i<PixelCount;i++) {
      if ((col == -1) || (tubePixels[i]>=maxDigits)) strip.SetPixelColor(i,black);
      else if (col>=256) strip.SetPixelColor(i,white);
      else strip.SetPixelColor(i,Wheel(col));
    } 
  strip.Show();  
}

void kitt() {
  static int dir = 1;  //direction
  static int counter = 0;
  static unsigned long lastRun = 0;

  if ((millis()-lastRun) < (258-prm.rgbSpeed)) return;
  lastRun = millis();
  
  for (int i=0;i<PixelCount;i++) strip.SetPixelColor(i,black);
  setPixels(counter,Wheel(prm.rgbFixColor));
  counter += dir;
  if (counter >= maxDigits-1) dir = -1;
  else if (counter <=0 ) dir = 1; 
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
  if ((millis()-lastRun)<(258-prm.rgbSpeed)) return;
  lastRun = millis();

  if ((prm.rgbEffect == 0) || !displayON) {   //Night: no RGB lightning
    neoBrightness = 0;
    fixColor(-1);
    return;
  }

  neoBrightness = prm.rgbBrightness;
  strip.SetBrightness(neoBrightness);
  
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
void setupNeopixelMakuna() {}
void doAnimationMakuna() {}
void darkenNeopixels() {}
#endif

#ifdef USE_PWMLEDS

#define FPS 60 //frame per sec
#define FPS_MSEC 1000/FPS
#define COLORSATURATION 255
#define WHITE_INDEX 192

long pwmBrightness;
int pwmColorStep = 1;
int pwmRed,pwmGreen,pwmBlue = 0;

void setPWMcolor(int WheelPos, byte brightness) {
#ifdef USE_PWMLEDS

  int pwmRed,pwmGreen,pwmBlue;

  if (WheelPos == WHITE_INDEX) {
    pwmRed = 255; pwmGreen = 255; pwmBlue = 255;
  }
  else if (WheelPos<0) {
    pwmRed =0; pwmGreen = 0; pwmBlue = 0;
  }
  else {  
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85)  {
      pwmRed =255 - WheelPos * 3; pwmGreen = 0; pwmBlue = WheelPos * 3;
    } else if(WheelPos < 170) {
      WheelPos -= 85;
      pwmRed = 0, pwmGreen = WheelPos * 3; pwmBlue = 255 - WheelPos * 3;
    } else  {
      WheelPos -= 170;
      pwmRed = WheelPos * 3; pwmGreen = 255 - WheelPos * 3; pwmBlue = 0;
    }
  }

  #if PWM1_PIN>=0
    ledcWrite(0, pwmRed * brightness / 255);
  #endif
  #if PWM2_PIN>=0
    ledcWrite(1, pwmGreen * brightness / 255);
  #endif
  #if PWM3_PIN>=0
    ledcWrite(2, pwmBlue * brightness / 255);
  #endif
#endif  
}


void pwmRainbow() {
 static byte j=0;
   
  setPWMcolor(j,pwmBrightness);
  if (prm.rgbDir) { j++; }
  else { j--; }
}

void pwmAlarmLight() {
  static unsigned long lastRun = 0;   
  static byte counter;
  
  if ((millis()-lastRun)<500) return;
  lastRun = millis();
  counter++;
  if (counter%2)
    setPWMcolor(WHITE_INDEX,255);
  else  
    setPWMcolor(-1,0);
}


void doAnimationPWM() {
static unsigned long lastRun = 0;

  if (EEPROMsaving) return;
  if (alarmON) {
    pwmAlarmLight();
    return;
  }
  
  if ((prm.rgbEffect <=1) && ((millis()-lastRun)<1000)) return;  //fix color
  if ((millis()-lastRun)<max(FPS_MSEC,258-prm.rgbSpeed)) return;
  lastRun = millis();

  if ((prm.rgbEffect == 0) || !displayON || !radarON) {   //switch RGB backlight OFF
    pwmBrightness = 0;
    setPWMcolor(-1,0);
    return;
  }
  
  pwmColorStep = max(1,prm.rgbSpeed/5);
  pwmBrightness = prm.rgbBrightness;
  if (autoBrightness) {
    pwmBrightness = (pwmBrightness * lx) /(long) MAXIMUM_LUX/2;
    if (pwmBrightness< c_MinBrightness) 
            pwmBrightness = c_MinBrightness;
  }
  pwmBrightness = min(pwmBrightness,long(RGB_MAX_BRIGHTNESS));  //safety only

  //DPRINTLN("  NeoBrightness:"); DPRINT(neoBrightness);
  
  if (prm.rgbEffect==1) setPWMcolor(prm.rgbFixColor,pwmBrightness);
  else pwmRainbow(); //flow
}

#else
void doAnimationPWM() {}
#endif

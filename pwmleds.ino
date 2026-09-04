// ============================================================================
// PWM Tube Backlight subsystem (plain R/G/B PWM, no addressable LEDs)
// ----------------------------------------------------------------------------
// Drives a single RGB backlight (or 3 separate R/G/B LED strips wired in
// parallel) behind/around the tubes using the ESP32 LEDC hardware PWM
// peripheral directly on 3 GPIOs (PWM1_PIN/PWM2_PIN/PWM3_PIN = R/G/B). This
// is the simpler alternative to the addressable-WS2812 backlight implemented
// in z_neopixel.ino (USE_NEOPIXEL): here there is only ONE logical color for
// the whole backlight (no per-tube pixel control), computed in software and
// pushed out as 3 duty-cycle values instead of a serial LED protocol.
//
// Enabled per-config via #define USE_PWMLEDS in clocks.h. Currently the only
// config using it is CLOCK_54 (D1 R32 ESP32 + NCS312 Nixie clock KIT) - grep
// clocks.h for "#define USE_PWMLEDS" for the authoritative, current list.
// USE_NEOPIXEL and USE_PWMLEDS are meant to be mutually exclusive per clock
// config (CLOCK_54 keeps USE_NEOPIXEL commented out) - see the ESP8266/ESP32
// and "both defined" caveats in the POSSIBLE BUG notes below.
//
// LEDC is an ESP32-only peripheral (esp32-hal-ledc.h: ledcAttachPin/
// ledcSetup/ledcWrite have no ESP8266 equivalent). Pin setup (pinMode +
// ledcAttachPin + ledcSetup, 200Hz/8-bit per channel) happens once in
// ESP32_UniClock2.ino's setup(), NOT in this file - this file only ever
// calls ledcWrite() to update duty cycle. See setup()'s "#ifdef USE_PWMLEDS"
// block (channels 0/1/2 = R/G/B) for the actual pin/peripheral configuration.
//
// Called from the core program (ESP32_UniClock2.ino):
//   - doAnimationPWM() : every loop() iteration, AND from inside Fdelay()'s
//                        busy-wait loop, so the backlight keeps animating
//                        even while tube digits are mid-transition (Fdelay
//                        blocks for the tube-switch duration). It rate-limits
//                        itself internally (see FPS_MSEC / prm.rgbSpeed),
//                        mirroring doAnimationMakuna() in z_neopixel.ino.
//
// Only a subset of prm's RGB settings is used here compared to the fuller
// z_neopixel.ino: prm.rgbEffect (0=off, 1=fixed color, anything else=
// rainbow flow), prm.rgbBrightness, prm.rgbFixColor, prm.rgbSpeed and
// prm.rgbDir. There is no equivalent here of z_neopixel's effect1/effect2/
// random-color modes - this file only ever shows "off", a single fixed
// color, or one continuous rainbow sweep.
// ============================================================================

#ifdef USE_PWMLEDS

#define FPS 60 //frames per second, animation frame-rate cap (matches z_neopixel.ino)
#define FPS_MSEC 1000/FPS        //ms per animation frame = minimum interval between doAnimationPWM() updates
#define COLORSATURATION 255      //POSSIBLE BUG (minor/dead code): defined but never referenced in this file - leftover from z_neopixel.ino where it sizes RgbColor constants
#define WHITE_INDEX 192          //reserved "color wheel" position (out of 0..255) that setPWMcolor() special-cases to pure white instead of a hue

long pwmBrightness;
int pwmColorStep = 1;            //POSSIBLE BUG (dead code, same pattern already noted in z_neopixel.ino's colorStep): computed from prm.rgbSpeed in doAnimationPWM() but never actually used to step the rainbow index - pwmRainbow() always advances j by exactly 1 per call, so animation speed is controlled only via the frame-timing gate below, not via this variable
int pwmRed,pwmGreen,pwmBlue = 0; //POSSIBLE BUG (dead code): these globals are always shadowed by the same-named local variables declared in setPWMcolor() below, so they are never actually read or written - harmless (ints default-init to 0) but misleading, and the "= 0" here only initializes pwmBlue, not pwmRed/pwmGreen (equivalent for globals, which zero-init anyway, but worth noting the syntax doesn't do what it looks like)

// Converts a "color wheel" position (0..255 hue, or the special WHITE_INDEX,
// or <0 for "off") plus a brightness (0..255) into R/G/B PWM duty cycles and
// writes them out via LEDC. Same hue math as z_neopixel.ino's Wheel(), just
// inlined here and applied directly to 3 PWM channels instead of an RgbColor.
void setPWMcolor(int WheelPos, byte brightness) {
#ifdef USE_PWMLEDS

  int pwmRed,pwmGreen,pwmBlue;   //local color components (deliberately shadow the unused globals above)

  if (WheelPos == WHITE_INDEX) {
    pwmRed = 255; pwmGreen = 255; pwmBlue = 255;
  }
  else if (WheelPos<0) {
    pwmRed =0; pwmGreen = 0; pwmBlue = 0;   //WheelPos<0 is the "backlight off" sentinel used by doAnimationPWM()/pwmAlarmLight()
  }
  else {
    // Standard 3-band color wheel: 0..255 hue split into 3 rising/falling
    // ramps (R->B, B->G, G->R) of 85 steps each, *3 to scale 0..85 up to 0..255.
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

  // Scale each color channel by brightness (0..255) and push out as PWM duty
  // cycle. Channel numbers (0/1/2) must match the ledcAttachPin() channel
  // assignment done once in ESP32_UniClock2.ino's setup() - this file does
  // not own pin/channel setup, only per-frame duty-cycle updates.
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


// Continuous rainbow sweep ("flow" effect, prm.rgbEffect >= 2): walks the
// color wheel index j by exactly 1 step per call (see pwmColorStep note
// above - prm.rgbSpeed only affects how often doAnimationPWM() calls this,
// not the step size), wrapping via byte overflow at 0/255. Direction is set
// by prm.rgbDir.
void pwmRainbow() {
 static byte j=0;

  setPWMcolor(j,pwmBrightness);
  if (prm.rgbDir) { j++; }
  else { j--; }
}

// Alarm indicator: while alarmON, ignore the normal effect/color settings and
// blink the backlight white/off at full brightness every 500ms (2 calls per
// on/off cycle = 1Hz blink), regardless of prm.rgbBrightness, so the alarm is
// clearly visible even if the user configured a dim or off backlight.
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


// Main entry point, called every loop()/Fdelay() iteration (see file header).
// Self-throttles internally so it's safe to call at arbitrarily high
// frequency - the actual color update rate is capped well below the caller's
// call frequency.
void doAnimationPWM() {
static unsigned long lastRun = 0;

  if (EEPROMsaving) return;          //don't touch PWM/SPI-ish timing while an EEPROM write is in progress
  if (alarmON) {
    pwmAlarmLight();
    return;
  }

  // Fixed color (or OFF) doesn't need 60fps updates - only re-check once a
  // second (e.g. in case displayON/radarON toggled) instead of every frame.
  if ((prm.rgbEffect <=1) && ((millis()-lastRun)<1000)) return;  //fix color
  // Animated (rainbow) effect: gate to a frame rate derived from prm.rgbSpeed
  // (0..255), from ~60fps (FPS_MSEC, fast/high speed) down to ~3ms floor at
  // max speed and up to ~258ms at speed=0 (slowest). max() with FPS_MSEC caps
  // the update rate at 60fps even for high rgbSpeed values; 258-rgbSpeed can't
  // go negative since rgbSpeed is a byte (0..255).
  if ((millis()-lastRun)<max(FPS_MSEC,258-prm.rgbSpeed)) return;
  lastRun = millis();

  if ((prm.rgbEffect == 0) || !displayON || !radarON) {   //switch RGB backlight OFF: user disabled it, display is manually off, or radar/presence timed out
    pwmBrightness = 0;
    setPWMcolor(-1,0);
    return;
  }

  pwmColorStep = max(1,prm.rgbSpeed/5);  //see POSSIBLE BUG note above - computed but currently unused
  pwmBrightness = prm.rgbBrightness;
  if (autoBrightness) {
    // Scale brightness down proportionally to ambient light (lx) read from
    // the light sensor, floored at c_MinBrightness so the backlight never
    // goes fully invisible in auto-dim mode; halved (/2) vs. the raw
    // lx/MAXIMUM_LUX ratio so the backlight dims more aggressively than the
    // tube brightness curve (matches the same /2 in z_neopixel.ino).
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
// USE_PWMLEDS not defined for this clock config: provide a no-op stub so the
// unconditional doAnimationPWM() calls in ESP32_UniClock2.ino's loop()/
// Fdelay() still link even when this subsystem is compiled out entirely.
void doAnimationPWM() {}
#endif

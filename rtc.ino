#ifdef USE_RTC
//DS3231 realtime clock driver
//with manual clock setup buttons
#include <RtcDS3231.h>   //Makuna/RTC
RtcDS3231<TwoWire> Rtc(Wire);
RtcDateTime rtcNow;
RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
#define EPOCH_OFFSET 946684800l   //offset from 1970->2000 epoch start
//------ Mode Switch and Push Buttons ---------------------
#ifndef PIN_MODE_SWITCH
  #define PIN_MODE_SWITCH  -1  
#endif

#ifndef PIN_FLD_BUTTON
  #define PIN_FLD_BUTTON -1
#endif
#ifndef PIN_SET_BUTTON
  #define PIN_SET_BUTTON -1    
#endif

boolean pushedFldButtonValue = LOW;   //wich state means pushed button
boolean pushedSetButtonValue = LOW;   //wich state means pushed button

//------- I2C bus definition   Any pins are usable  Please, SET in clocks.h --------
//#define PIN_SDA 4           //D2   original general setup for 8266
//#define PIN_SCL 5           //D1

#define MENU_UNSELECT_TIMEOUT 30000

int monthdays[13] = {0,31,29,31,30,31,30,31,31,30,31,30,31};

struct Menu {
   short lowlimit;        // alsó határ
   short uplimit;         // felső határ
   char name[5];
};

const struct Menu m1[]= {
  {0,0,"    "},
  {2022,2099,"Year"},
  {1,12,"Mon "},
  {1,31,"Day "},
  {0,23,"Hour"},
  {0,59,"Min "},
};

#define MAXFLD 5 

int mvar[MAXFLD+1];

enum switchStates {IS_OPEN, IS_OPENING, IS_PUSHED, IS_LONGPRESSING, IS_LONGPRESSED, IS_PUSHING, IS_LONGOPENING};

int fld = 0;
unsigned long LastModify = 0;

void setupRTC() {
  #if PIN_MODE_SWITCH>=0
    pinMode(PIN_MODE_SWITCH,INPUT_PULLUP);   
    regPin(PIN_MODE_SWITCH,"PIN_MODE_SWITCH"); 
  #endif
  #if PIN_FLD_BUTTON>=0  
    pinMode(PIN_FLD_BUTTON,INPUT_PULLUP);    
    regPin(PIN_FLD_BUTTON,"PIN_FLD_BUTTON"); 
    if (digitalRead(PIN_FLD_BUTTON) == LOW)  //inverted button?
      pushedFldButtonValue = HIGH;
  #endif    
  #if PIN_SET_BUTTON>=0  
    pinMode(PIN_SET_BUTTON,INPUT_PULLUP);    
    regPin(PIN_SET_BUTTON,"PIN_SET_BUTTON"); 
    if (digitalRead(PIN_SET_BUTTON) == LOW)  //inverted button?
      pushedSetButtonValue = HIGH;
  #endif    
  
  if (RTCexist) {
    DPRINTLN("Connecting to RTC clock...");
    Rtc.Begin();
    if (!Rtc.IsDateTimeValid()) {
        if (Rtc.LastError() != 0) {
            DPRINT("RTC communications error = ");  DPRINTLN(Rtc.LastError());
        }
        else {
            DPRINTLN("RTC lost confidence in the DateTime!");
            Rtc.SetDateTime(compiled);  //Set RTC time to firmware compile time
        }
    }

    if (!Rtc.GetIsRunning())  {
        DPRINTLN("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);     
  }
/*  
while (true) {
  DPRINT(digitalRead(PIN_MODE_SWITCH)); DPRINT(" / "); 
  DPRINT(digitalRead(PIN_FLD_BUTTON));  DPRINT(" / ");
  DPRINTLN(digitalRead(PIN_SET_BUTTON));
  delay(1000);
  }
*/  
}


void editor() {
  static unsigned long lastDisplay = 0;
  if (RTCexist) {rtcNow = Rtc.GetDateTime();}
  LastModify = 0;
  #if PIN_FLD_BUTTON >= 0 && PIN_SET_BUTTON >= 0   //Are buttons installed?
    editorRunning = true;
    while (true) {
      scanButFLD(100); 
      if (fld == 0) {
        editorRunning = false;
        break;
      }
      scanButSET(100);
      showValue();
      #if defined(WORDCLOCK)
        if ((millis()-lastDisplay)>100) {
          lastDisplay = millis();
          writeDisplay2();
        }
      #else
        writeDisplaySingle();
      #endif
      printDigits(1000);
      Fdelay(100);
    }
  #endif
  if (LastModify != 0) saveRTC();
}

void showValue() {
  memset(digit,10,sizeof(digit));   //clear array
  memset(digitDP,0,sizeof(digitDP));
  if(fld==1) {
    digit[3] = mvar[1] / 1000;
    digit[2] = (mvar[1] % 1000) / 100;    
    digit[1] = (mvar[1] % 100) / 10;
    digit[0] = mvar[1] % 10;
  }      
  else {
    digit[1] = mvar[fld] / 10;
    digit[0] = mvar[fld] % 10;
  }
  #if defined(MAX6921) || defined(MAX6921_ESP32)
    //character set not available yet
  #else
  if (!digitsOnly && (maxDigits>4)) {
    if (maxDigits>=8) digit[7] = m1[fld].name[0];
    if (maxDigits>=8) digit[6] = m1[fld].name[1];
    if (maxDigits>=6) digit[5] = m1[fld].name[2];
    else digit[5] = ':';
    digit[4] = m1[fld].name[3];
    
  }
  #endif
}

//------------------------------------- RTC Clock functions --------------------------------------------------------------------
void updateRTC() {
  DPRINTLN("Update RTC?");
  if (!RTCexist) return;
  if (year()<2020) return;    //invalid system date??

  rtcNow = Rtc.GetDateTime();
  if (abs(rtcNow.Second() - second())>2 || (rtcNow.Minute() != minute()) || (rtcNow.Hour() != hour())  
      || (rtcNow.Day() != day()) || (rtcNow.Month() != month()) || (rtcNow.Year() != year()) )   {
      DPRINTLN("Updating RTC from TimeServer...");
      DPRINT("- DS3231  date:"); DPRINT(rtcNow.Year()); DPRINT("/"); DPRINT(rtcNow.Month()); DPRINT("/"); DPRINT(rtcNow.Day()); 
      DPRINT(" time:");  DPRINT(rtcNow.Hour()); DPRINT(":"); DPRINT(rtcNow.Minute()); DPRINT(":"); DPRINTLN(rtcNow.Second());    
      DPRINT("- Tserver date:"); DPRINT(year()); DPRINT("/"); DPRINT(month()); DPRINT("/"); DPRINT(day());      
      DPRINT(" time:");   DPRINT(hour()); DPRINT(":"); DPRINT(minute()); DPRINT(":"); DPRINTLN(second());  
      Rtc.SetDateTime(now()-EPOCH_OFFSET);   //update RTC with UNIX epoch timestamp to 2000year epoch
      }  //endif
}


void getRTC() {
  if (RTCexist) {
    if (!Rtc.IsDateTimeValid()) {
        if (Rtc.LastError() != 0) {
            DPRINT("RTC communications error = "); DPRINTLN(Rtc.LastError());
        }
        else {
            DPRINTLN("RTC lost confidence in the DateTime!");
        }
    }
    else {
      rtcNow = Rtc.GetDateTime();
      //DPRINT("\nGet DS3231 data&time: ");  DPRINT(rtcNow.Year()); DPRINT("/"); DPRINT(rtcNow.Month()); DPRINT("/"); DPRINT(rtcNow.Day()); 
      //DPRINT(" ");  DPRINT(rtcNow.Hour()); DPRINT(":"); DPRINT(rtcNow.Minute()); DPRINT(":"); DPRINTLN(rtcNow.Second());  
      setTime(rtcNow.Hour(),rtcNow.Minute(),rtcNow.Second(),rtcNow.Day(),rtcNow.Month(),rtcNow.Year());  //set the time (hr,min,sec,day,mnth,yr)
    }
  }
  mvar[1] = year(); mvar[2] = month(); mvar[3] = day();
  mvar[4] = hour(); mvar[5] = minute(); 
}

void saveRTC() {
  setTime(mvar[4],mvar[5],0,mvar[3],mvar[2],mvar[1]);  //set the time (hr,min,sec,day,mnth,yr)

    if (RTCexist) {
      DPRINTLN("Updating RTC from Manual settings...");
      Rtc.SetDateTime(now()-EPOCH_OFFSET);
    }
    DPRINT("New Date & Time:"); DPRINT(year()); DPRINT("/"); DPRINT(month()); DPRINT("/"); DPRINT(day()); 
    DPRINT(" time:");  DPRINT(hour()); DPRINT(":"); DPRINT(minute()); DPRINT(":"); DPRINTLN(second());  
}

//-------------------------- check buttons  ------------------------------------------------

void scanButFLD(unsigned long mill) {
static unsigned long lastRun = millis(); 
static unsigned long lastPush = millis();; 
static switchStates butState;
byte sw;

  if (PIN_FLD_BUTTON<0) return;
  if ((millis() - lastRun) < mill) return;   //refresh rate
  lastRun = millis(); 

  sw = digitalRead(PIN_FLD_BUTTON);
  if (pushedFldButtonValue == HIGH) sw = !sw;   //inverted button logic
  
  switch (butState) {
    case IS_OPEN:   
      if(sw == LOW) butState = IS_PUSHING;  
      break; 
    case IS_PUSHING: 
      butState = IS_PUSHED;  
      lastPush = millis();
      DPRINTLN(" PUSHED");
      break; 
    case IS_PUSHED:   
      if(sw == HIGH)  butState = IS_OPENING; 
      if((sw == LOW) && ((millis()-lastPush)>2000))  butState = IS_LONGPRESSING; 
      break; 
    case IS_LONGPRESSING:  
      butState = IS_LONGPRESSED; 
      DPRINTLN(" IS_LONGPRESSED");
      if (fld>0) {
        fld = 0;
        saveRTC();
      }
      else {
        fld = 1;
        getRTC();
      }
      break; 
    case IS_LONGPRESSED:  
      if (sw == HIGH) butState = IS_LONGOPENING; 
      break;      
    case IS_OPENING:
      butState = IS_OPEN;
      if ((millis()-lastPush)<500) {
           LastModify = millis();
          fld++;  if (fld>MAXFLD) fld = 1;
          DPRINT(m1[fld].name); DPRINTLN(fld);
       } //endif millis()
      break; 
    case IS_LONGOPENING: 
      butState = IS_OPEN;
      break;    
  } //end switch
}

void scanButSET(unsigned long mill) {
static unsigned long lastRun = millis(); 
static unsigned long lastPush = millis(); 
static switchStates butState;
byte sw;

  if (PIN_SET_BUTTON<0) return;
  if ((millis() - lastRun) < mill) return;   //refresh rate
  lastRun = millis(); 

  sw = digitalRead(PIN_SET_BUTTON);
  if (pushedSetButtonValue == HIGH) sw = !sw;   //inverted button logic  
  
  switch (butState) {
    case IS_OPEN:   
      if(sw == LOW) butState = IS_PUSHING;  
      break; 
    case IS_PUSHING: 
      butState = IS_PUSHED;  
      lastPush = millis();
      DPRINTLN(" PUSHED");
      break; 
    case IS_PUSHED:   
      if(sw == HIGH)  butState = IS_OPENING; 
      if((sw == LOW) && ((millis()-lastPush)>2000))  butState = IS_LONGPRESSING; 
      break; 
    case IS_LONGPRESSING:  
      butState = IS_LONGPRESSED; 
      DPRINTLN(" IS_LONGPRESSED");
      //if (noZero !=0) noZero =0; else noZero = 1;
      //EEPROM.update(EEPROM_NOZERO, noZero);  
      break; 
    case IS_LONGPRESSED:  
      if (sw == HIGH) butState = IS_LONGOPENING; 
       // if ((millis()-lastPush)>10000)  {factoryReset(); resetFunc();}   //reset to factory settings
       LastModify = millis();
       mvar[fld]+=1;
       if (mvar[fld] < m1[fld].lowlimit)  mvar[fld] = m1[fld].uplimit;
       if (mvar[fld] > m1[fld].uplimit)  mvar[fld] = m1[fld].lowlimit;
       DPRINT(fld); DPRINT(" : "); DPRINTLN(mvar[fld]);
       break;
    case IS_OPENING:
      butState = IS_OPEN;
      if ((millis()-lastPush)<500) {
          LastModify = millis();
          mvar[fld]++;
          if (mvar[fld] < m1[fld].lowlimit)  mvar[fld] = m1[fld].uplimit;
          if (mvar[fld] > m1[fld].uplimit)  mvar[fld] = m1[fld].lowlimit;
          //if (fld <= 2) mvar[2] = monthday_check(mvar[RTC_Ev], mvar[RTC_Ho], mvar[RTC_Nap]);    //szökőév ellenőrzés           
          DPRINT(fld); DPRINT(" : "); DPRINTLN(mvar[fld]);
          if (fld>MAXFLD) fld = 0;
          //flash(fld);
       } //endif millis()
      break; 
    case IS_LONGOPENING: 
      butState = IS_OPEN;
      break;    
  } //end switch
}


#if defined(PIN_SDA) && defined(PIN_SCL)
int I2C_ClearBus() {
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif
  DPRINTLN("I2C_ClearBus() started.");
  pinMode(PIN_SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
  pinMode(PIN_SCL, INPUT_PULLUP);

  Fdelay(2500);  // Wait 2.5 secs. This is strictly only necessary on the first power
  // up of the DS3231 module to allow it to initialize properly,
  // but is also assists in reliable programming of FioV3 boards as it gives the
  // IDE a chance to start uploaded the program
  // before existing sketch confuses the IDE by sending Serial data.

  boolean SCL_LOW = (digitalRead(PIN_SCL) == LOW); // Check is SCL is Low.
  if (SCL_LOW) { //If it is held low Arduno cannot become the I2C master. 
    return 1; //I2C bus error. Could not clear SCL clock line held low
  }

  boolean SDA_LOW = (digitalRead(PIN_SDA) == LOW);  // vi. Check SDA input.
  int clockCount = 20; // > 2x9 clock

  while (SDA_LOW && (clockCount > 0)) { //  vii. If SDA is Low,
    clockCount--;
  // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
    pinMode(PIN_SCL, INPUT); // release SCL pullup so that when made output it will be LOW
    pinMode(PIN_SCL, OUTPUT); // then clock SCL Low
    delayMicroseconds(10); //  for >5uS
    pinMode(PIN_SCL, INPUT); // release SCL LOW
    pinMode(PIN_SCL, INPUT_PULLUP); // turn on pullup resistors again
    // do not force high as slave may be holding it low for clock stretching.
    delayMicroseconds(10); //  for >5uS
    // The >5uS is so that even the slowest I2C devices are handled.
    SCL_LOW = (digitalRead(PIN_SCL) == LOW); // Check if SCL is Low.
    int counter = 20;
    while (SCL_LOW && (counter > 0)) {  //  loop waiting for SCL to become High only wait 2sec.
      counter--;
      delay(100);
      SCL_LOW = (digitalRead(PIN_SCL) == LOW);
    }
    if (SCL_LOW) { // still low after 2 sec error
      return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
    }
    SDA_LOW = (digitalRead(PIN_SDA) == LOW); //   and check SDA input again and loop
  }
  if (SDA_LOW) { // still low
    return 3; // I2C bus error. Could not clear. SDA data line held low
  }

  // else pull SDA line low for Start or Repeated Start
  pinMode(PIN_SDA, INPUT); // remove pullup.
  pinMode(PIN_SDA, OUTPUT);  // and then make it LOW i.e. send an I2C Start or Repeated start control.
  // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
  /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
  delayMicroseconds(10); // wait >5uS
  pinMode(PIN_SDA, INPUT); // remove output low
  pinMode(PIN_SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
  delayMicroseconds(10); // x. wait >5uS
  pinMode(PIN_SDA, INPUT); // and reset pins as tri-state inputs which is the default state on reset
  pinMode(PIN_SCL, INPUT);
  return 0; // all ok
}
#else
int I2C_ClearBus() {return 1;}
#endif

//------------------------------------------------------------------------------------------
#else
  void updateRTC() {}
  void setupRTC() {}
  void getRTC() {}
  void editor() {}
#endif

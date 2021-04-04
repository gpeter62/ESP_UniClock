#ifdef USE_RTC
//DS3231 realtime clock driver
//with manual clock setup buttons
#include <ds3231.h>    //https://github.com/rodan/ds3231
  
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

//------- I2C bus definition   Any pins are usable  Please, SET in clocks.h --------
//#define PIN_SDA 4           //D2   original general setup for 8266
//#define PIN_SCL 5           //D1

#define MENU_UNSELECT_TIMEOUT 30000

//#include <ds3231.h>    //https://github.com/rodan/ds3231
struct ts tim;

int monthdays[13] = {0,31,29,31,30,31,30,31,31,30,31,30,31};

struct Menu {
   short lowlimit;        // alsó határ
   short uplimit;         // felső határ
   char _name[10];
};

const struct Menu m1[]= {
  {0,0,"not used"},
  {2018,2099,"year"},
  {1,12,"month"},
  {1,31,"day"},
  {0,23,"hour"},
  {0,59,"minute"},
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
  #endif    
  #if PIN_SET_BUTTON>=0  
    pinMode(PIN_SET_BUTTON,INPUT_PULLUP);    
    regPin(PIN_SET_BUTTON,"PIN_SET_BUTTON"); 
  #endif    
  
  if (RTCexist) {
    DS3231_init(DS3231_CONTROL_INTCN);
    DS3231_get(&tim);
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
  DS3231_get(&tim);
  LastModify = 0;
  #if PIN_FLD_BUTTON >= 0 && PIN_SET_BUTTON >= 0   //Are buttons installed?
    while (true) {
      scanButFLD(100); if (fld == 0) break;
      scanButSET(100);
      showValue();
      writeDisplaySingle();
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
    digit[2] = (mvar[1]%1000) / 100;    
    digit[1] = (mvar[1]%100) / 10;
    digit[0] = mvar[1] % 10;
  }      
  else {
    digit[1] = mvar[fld] / 10;
    digit[0] = mvar[fld] % 10;
  }
}

//------------------------------------- RTC Clock functions --------------------------------------------------------------------
void updateRTC() {
  DPRINTLN("Update RTC?");
  if (!RTCexist && year()<2020) return;    //invalid system date??
  DS3231_get(&tim);
  if (abs(tim.sec - second())>2 || (tim.min != minute()) || (tim.hour != hour())  
      || (tim.mday != day()) || (tim.mon != month()) || (tim.year != year()) )   {
      DPRINTLN("Updating RTC from TimeServer...");
      DPRINT("- DS3231  date:"); DPRINT(tim.year); DPRINT("/"); DPRINT(tim.mon); DPRINT("/"); DPRINT(tim.mday); 
      DPRINT(" time:");  DPRINT(tim.hour); DPRINT(":"); DPRINT(tim.min); DPRINT(":"); DPRINTLN(tim.sec);    
      DPRINT("- Tserver date:"); DPRINT(year()); DPRINT("/"); DPRINT(month()); DPRINT("/"); DPRINT(day());      
      DPRINT(" time:");   DPRINT(hour()); DPRINT(":"); DPRINT(minute()); DPRINT(":"); DPRINTLN(second());  
      tim.sec = second();
      tim.min = minute();
      tim.hour = hour();
      tim.mday = day();
      tim.mon = month();
      tim.year = year(); 
      DS3231_set(tim); 
      }  //endif
}


void getRTC() {
  if (!RTCexist) return;
  DS3231_get(&tim);
  setTime(tim.hour,tim.min,tim.sec,tim.mday,tim.mon,tim.year);  //set the time (hr,min,sec,day,mnth,yr)
  mvar[1] = tim.year; mvar[2] = tim.mon; mvar[3] = tim.mday;
  mvar[4] = tim.hour; mvar[5] = tim.min; 
}

void saveRTC() {
    if (!RTCexist) return;
    DPRINTLN("Updating RTC from Manual settings...");
    tim.sec = 0;
    tim.min = mvar[5];
    tim.hour = mvar[4];
    tim.mday = mvar[3];
    tim.mon = mvar[2];
    tim.year = mvar[1]; 
    DPRINT("New RTC date:"); DPRINT(tim.year); DPRINT("/"); DPRINT(tim.mon); DPRINT("/"); DPRINT(tim.mday); 
    DPRINT(" time:");  DPRINT(tim.hour); DPRINT(":"); DPRINT(tim.min); DPRINT(":"); DPRINTLN(tim.sec);  
    DS3231_set(tim); 
}

//-------------------------- check buttons  ------------------------------------------------

void scanButFLD(unsigned long mill) {
static unsigned long lastRun = millis(); 
static unsigned long lastPush = millis();; 
static boolean lastState = false;
static switchStates butState;
byte sw;

  if (PIN_FLD_BUTTON<0) return;
  if ((millis() - lastRun) < mill) return;   //refresh rate
  lastRun = millis(); 

  sw = digitalRead(PIN_FLD_BUTTON);
  switch (butState) {
    case IS_OPEN:   
      if(sw == LOW) butState = IS_PUSHING;  
      break; 
    case IS_PUSHING: 
      butState = IS_PUSHED;  
      lastState = true;  
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
      fld = 0;
      break;      
    case IS_OPENING:
      butState = IS_OPEN;
      lastState = false;      
      if ((millis()-lastPush)<500) {
           LastModify = millis();
          fld++;  if (fld>MAXFLD) fld = 0;
          DPRINT(m1[fld]._name); DPRINTLN(fld);
       } //endif millis()
      break; 
    case IS_LONGOPENING: 
      butState = IS_OPEN;
      lastState = false;  
      break;    
  } //end switch
}

void scanButSET(unsigned long mill) {
static unsigned long lastRun = millis(); 
static unsigned long lastPush = millis(); 
static boolean lastState = false;
static switchStates butState;
byte sw;

  if (PIN_SET_BUTTON<0) return;
  if ((millis() - lastRun) < mill) return;   //refresh rate
  lastRun = millis(); 

  sw = digitalRead(PIN_SET_BUTTON);
  switch (butState) {
    case IS_OPEN:   
      if(sw == LOW) butState = IS_PUSHING;  
      break; 
    case IS_PUSHING: 
      butState = IS_PUSHED;  
      lastState = true;  
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
      lastState = false;      
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
      lastState = false;  
      break;    
  } //end switch
}

int I2C_ClearBus() {
#if defined(TWCR) && defined(TWEN)
  TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif

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


//------------------------------------------------------------------------------------------
#else
  void updateRTC() {}
  void setupRTC() {}
  void getRTC() {}
  void editor() {}
#endif

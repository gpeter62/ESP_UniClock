#ifdef samsung

int maxDigits = 8;
#include <Samsung_16LF01_VFD.h>

byte  SCLKpin = 7;
byte  RSTpin  = 6;
byte  DATApin = 5;

char charTable[] = "0123456789 -.APC°";

Samsung_16LF01_VFD vfd(SCLKpin, DATApin, RSTpin);


void setup_pins() {
  vfd.begin(maxDigits, prm.dayBright);
  vfd.print("Hello World");
  delay(3000);
  vfd.clear();
}

void writeDisplaySingle() {
  vfd.setCursor(0);
  for (int i=maxDigits-1;i>0;i--) {
    vfd.write(charTable[digit[i]]);
  }
}

#endif

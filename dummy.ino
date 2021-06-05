#ifdef DUMMY

//Dummy clock driver for sensor box / mqtt sensors

char tubeDriver[] = "DUMMY";
int maxDigits = 6;

void setup_pins() {
  DPRINTLN("Dummy display: no pins are used");
}

#if defined(ESP32)
void IRAM_ATTR writeDisplay() {} 
#else
void ICACHE_RAM_ATTR writeDisplay(){}
#endif

void clearTubes() {}
void writeDisplaySingle() {}
#endif

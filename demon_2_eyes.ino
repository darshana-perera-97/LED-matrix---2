#include <LedControl.h>

// SPI pins on NodeMCU
#define DATA_PIN    13   // D7
#define CLK_PIN     14   // D5
#define CS_PIN      12   // D6
#define NUM_DEVICES 2

LedControl lc(DATA_PIN, CLK_PIN, CS_PIN, NUM_DEVICES);

// A simple 8×8 “eye” shape (1 = LED on; bit7=left, bit0=right)
static const byte eyeShape[8] = {
  B00111100,
  B01000010,
  B10011001,
  B10000001,
  B10000001,
  B10011001,
  B01000010,
  B00111100
};

// Draws the eye on one module, applying the proper rotate+mirror
void drawEye(int dev) {
  for (int colP = 0; colP < 8; colP++) {
    byte val = 0;
    for (int rowP = 0; rowP < 8; rowP++) {
      bool pixel;
      if (dev == 0) {
        // Module 0: rotate 90° CW → (r,c)→(c,7–r)
        // so origRow = 7–colP, origCol = rowP
        int origRow = 7 - colP;
        int origCol =  rowP;
        pixel = (eyeShape[origRow] >> (7 - origCol)) & 0x01;
      } else {
        // Module 1: rotate 90° CCW → (r,c)→(7–c,r)
        // so origRow = colP, origCol = 7–rowP
        int origRow =  colP;
        int origCol = 7 - rowP;
        pixel = (eyeShape[origRow] >> (7 - origCol)) & 0x01;
      }
      if (pixel) {
        // pack into bit (7–rowP) so bit7=top rowP=0
        val |= 1 << (7 - rowP);
      }
    }
    // mirror horizontally by writing into column (7–colP)
    lc.setColumn(dev, 7 - colP, val);
  }
}

void setup() {
  // wake and clear both modules
  for (int d = 0; d < NUM_DEVICES; d++) {
    lc.shutdown(d, false);
    lc.setIntensity(d, 0);
    lc.clearDisplay(d);
  }
}

void loop() {
  // pulse brightness up…
  for (int bri = 0; bri <= 15; bri++) {
    for (int d = 0; d < NUM_DEVICES; d++) {
      lc.setIntensity(d, bri);
      drawEye(d);
    }
    delay(40);
  }
  // …and down again
  for (int bri = 15; bri >= 0; bri--) {
    for (int d = 0; d < NUM_DEVICES; d++) {
      lc.setIntensity(d, bri);
      drawEye(d);
    }
    delay(40);
  }
}

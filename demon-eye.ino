#include <LedControl.h>

// hardware SPI pins on NodeMCU
#define DATA_PIN    13   // D7
#define CLK_PIN     14   // D5
#define CS_PIN      12   // D6
#define NUM_DEVICES 2    // two cascaded 8×8 modules

LedControl lc(DATA_PIN, CLK_PIN, CS_PIN, NUM_DEVICES);

// ——— filled-ellipse eye shape (16×8) ———
// Each uint16_t is one row: bit15=leftmost … bit0=rightmost.
static const uint16_t eyeFill[8] = {
  0b0000011111100000,
  0b0000111111110000,
  0b0001111111111000,
  0b0011111111111100,
  0b0011111111111100,
  0b0001111111111000,
  0b0000111111110000,
  0b0000011111100000
};

// pupil size & vertical position
const int PUP_W   = 3;
const int PUP_H   = 3;
const int PUP_Y   = 2;   // draws on rows 2,3,4

// how far the pupil can slide (x coordinates on the 16-wide canvas)
const int PUP_MIN = 4;
const int PUP_MAX = 9;

// map a “canvas” pixel (x,y) to the correct module & LED
//  • module 0 (x<8):   rotate 90° CW
//  • module 1 (x>=8):  rotate 180° + additional 90° CW (i.e. 270° CW)
void setPixel(int x, int y, bool on) {
  if (x < 8) {
    // left module (device 0), 90° CW:
    //   row' = x
    //   col' = 7 - y
    lc.setLed(0, x, 7 - y, on);
  } else {
    // right module (device 1), 270° CW:
    //   local x = x - 8
    //   row' = 7 - local x
    //   col' = y
    int lx = x - 8;
    lc.setLed(1, 7 - lx, y, on);
  }
}

// draw one frame: fill the eye, then carve out the pupil
void drawFrame(int px) {
  // clear both modules
  for (int d = 0; d < NUM_DEVICES; d++) {
    lc.clearDisplay(d);
  }
  // 1) paint the eye
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 16; x++) {
      if (eyeFill[y] & (1u << (15 - x))) {
        setPixel(x, y, true);
      }
    }
  }
  // 2) carve out the pupil
  for (int dy = 0; dy < PUP_H; dy++) {
    for (int dx = 0; dx < PUP_W; dx++) {
      setPixel(px + dx, PUP_Y + dy, false);
    }
  }
}

void setup() {
  // wake modules & set intensity
  for (int d = 0; d < NUM_DEVICES; d++) {
    lc.shutdown(d, false);
    lc.setIntensity(d, 8);
    lc.clearDisplay(d);
  }
}

void loop() {
  // sweep pupil left→right then back
  for (int x = PUP_MIN; x <= PUP_MAX; x++) {
    drawFrame(x);
    delay(150);
  }
  for (int x = PUP_MAX; x >= PUP_MIN; x--) {
    drawFrame(x);
    delay(150);
  }
}

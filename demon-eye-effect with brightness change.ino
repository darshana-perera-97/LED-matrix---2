#include <LedControl.h>

// hardware SPI pins on NodeMCU
#define DATA_PIN    13   // D7
#define CLK_PIN     14   // D5
#define CS_PIN      12   // D6
#define NUM_DEVICES 2    // two cascaded 8×8 modules

LedControl lc(DATA_PIN, CLK_PIN, CS_PIN, NUM_DEVICES);

// ——— filled-ellipse eye shape (16×8) ———
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

// pupil size (w×h)
const int PUP_W   = 3;
const int PUP_H   = 3;
// pupil “home” Y (we’ll randomly add +1)
const int PUP_Y   = 2;
// horizontal range inside the 16×8 shape
const int PUP_MIN = 4;
const int PUP_MAX = 9;

// global eye offset on the 16×8 canvas
//  ➜ shift your entire shape: change these to move eye left/right/up/down
const int EYE_OFFSET_X = 2;
const int EYE_OFFSET_Y = 0;

// brightness range (0…15)
const int BR_MIN = 0;
const int BR_MAX = 15;

// pause between each micro-step (ms). 
// Increase to slow movement, decrease to speed up.
const unsigned long MOVE_DELAY = 100;

// current pupil position (absolute coords on 16×8 + offsets)
int currentPx, currentPy;

// map a “canvas” pixel (x,y) to the correct module & LED
void setPixel(int x, int y, bool on) {
  if (x < 0 || x >= 16 || y < 0 || y >= 8) return;
  if (x < 8) {
    // left module (device 0), 90° CW
    lc.setLed(0, x, 7 - y, on);
  } else {
    // right module (device 1), 270° CW
    int lx = x - 8;
    lc.setLed(1, 7 - lx, y, on);
  }
}

// draw one frame: paint the eye, then carve out the pupil
void drawFrame(int px, int py) {
  for (int d = 0; d < NUM_DEVICES; d++) lc.clearDisplay(d);

  // 1) paint eye shape with global offset
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 16; x++) {
      if (eyeFill[y] & (1u << (15 - x))) {
        setPixel(x + EYE_OFFSET_X, y + EYE_OFFSET_Y, true);
      }
    }
  }
  // 2) carve out pupil
  for (int dy = 0; dy < PUP_H; dy++) {
    for (int dx = 0; dx < PUP_W; dx++) {
      setPixel(px + dx, py + dy, false);
    }
  }
}

void setup() {
  for (int d = 0; d < NUM_DEVICES; d++) {
    lc.shutdown(d, false);
    lc.clearDisplay(d);
    lc.setIntensity(d, BR_MAX);
  }
  // start pupil in the middle
  currentPx = (PUP_MIN + PUP_MAX) / 2 + EYE_OFFSET_X;
  currentPy = PUP_Y + EYE_OFFSET_Y;
  randomSeed(analogRead(A0));
}

void loop() {
  // 1) pick a new random destination (within eye + offset)
  int relX   = random(PUP_MIN, PUP_MAX + 1);
  int targetPx = relX + EYE_OFFSET_X;
  int relY   = (random(0, 2) == 0 ? PUP_Y : PUP_Y + 1);
  int targetPy = relY + EYE_OFFSET_Y;

  // 2) step‐by‐step move from current → target
  while (currentPx != targetPx || currentPy != targetPy) {
    // step X toward target
    if (currentPx < targetPx) currentPx++;
    else if (currentPx > targetPx) currentPx--;
    // step Y toward target
    if (currentPy < targetPy) currentPy++;
    else if (currentPy > targetPy) currentPy--;

    // random brightness each micro-step
    int br = random(BR_MIN, BR_MAX + 1);
    for (int d = 0; d < NUM_DEVICES; d++) {
      lc.setIntensity(d, br);
    }

    drawFrame(currentPx, currentPy);
    delay(MOVE_DELAY);
  }

  // small pause once you arrive
  delay(MOVE_DELAY * 3);
}

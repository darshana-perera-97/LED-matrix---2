#include <LedControl.h>

// hardware SPI pins on NodeMCU
#define DATA_PIN    13   // D7
#define CLK_PIN     14   // D5
#define CS_PIN      12   // D6
#define NUM_DEVICES 2    // two cascaded 8×8 modules

LedControl lc(DATA_PIN, CLK_PIN, CS_PIN, NUM_DEVICES);

// your message and font parameters
const char message[] = "DARSHANA & DULANGI";
const int  msgLength = sizeof(message) - 1;
const int  charWidth = 5;       // each glyph is 5 columns
const int  spacing   = 1;       // blank column between glyphs
const int  bufferLen = msgLength * (charWidth + spacing);
byte       buffer[bufferLen];   // will hold the entire stream of columns

// timing
const int scrollSpeed = 150;  // ms per column shift
const int clearSpeed  = 50;   // ms per column during the wipe

void setup() {
  // initialize both MAX7219s
  for (int dev = 0; dev < NUM_DEVICES; dev++) {
    lc.shutdown(dev, false);
    lc.setIntensity(dev, 8);   // brightness (0–15)
    lc.clearDisplay(dev);
  }

  // 5×7 font data for just the characters we need
  static const byte fontD[5]   = {0x7F,0x41,0x41,0x22,0x1C};
  static const byte fontA[5]   = {0x7C,0x12,0x11,0x12,0x7C};
  static const byte fontR[5]   = {0x7F,0x09,0x19,0x29,0x46};
  static const byte fontS[5]   = {0x46,0x49,0x49,0x49,0x31};
  static const byte fontH[5]   = {0x7F,0x08,0x08,0x08,0x7F};
  static const byte fontN[5]   = {0x7F,0x04,0x08,0x10,0x7F};
  static const byte fontU[5]   = {0x3F,0x20,0x20,0x20,0x3F};
  static const byte fontL[5]   = {0x7F,0x40,0x40,0x40,0x40};
  static const byte fontG[5]   = {0x3E,0x42,0x41,0x51,0x3E};
  static const byte fontI[5]   = {0x00,0x41,0x7F,0x41,0x00};
  static const byte fontAmp[5] = {0x36,0x49,0x56,0x20,0x50};  // ‘&’
  static const byte fontSp[5]  = {0,0,0,0,0};

  // build the scroll buffer: for each char, copy its 5 cols + 1 blank
  int idx = 0;
  for (int i = 0; i < msgLength; i++) {
    const byte* glyph = fontSp;
    switch (message[i]) {
      case 'D': glyph = fontD;   break;
      case 'A': glyph = fontA;   break;
      case 'R': glyph = fontR;   break;
      case 'S': glyph = fontS;   break;
      case 'H': glyph = fontH;   break;
      case 'N': glyph = fontN;   break;
      case 'U': glyph = fontU;   break;
      case 'L': glyph = fontL;   break;
      case 'G': glyph = fontG;   break;
      case 'I': glyph = fontI;   break;
      case '&': glyph = fontAmp; break;
      case ' ': glyph = fontSp;  break;
    }
    for (int c = 0; c < charWidth; c++) {
      buffer[idx++] = glyph[c];
    }
    buffer[idx++] = 0x00;  // one-column spacer
  }
}

void loop() {
  const int totalCols = NUM_DEVICES * 8;  // 16
  byte window[totalCols];

  // scroll the 16-col window from off-right through the buffer
  for (int offset = -totalCols; offset <= bufferLen; offset++) {
    // 1) fetch 16 columns (or blank if out of range)
    for (int x = 0; x < totalCols; x++) {
      int bi = offset + x;
      window[x] = (bi >= 0 && bi < bufferLen) ? buffer[bi] : 0x00;
    }

    // 2) draw Module 0: rotate 90° CW, then mirror horizontally
    //    rotation CW: (r0,c0)→(c0,7−r0)
    //    mirror: write to column 7−colP
    for (int colP = 0; colP < 8; colP++) {
      byte val = 0;
      for (int rowP = 0; rowP < 8; rowP++) {
        int origRow = 7 - colP;
        int origCol =  rowP;
        bool pixel = (window[origCol] >> (7 - origRow)) & 0x01;
        if (pixel) val |= 1 << (7 - rowP);
      }
      lc.setColumn(0, 7 - colP, val);  // mirror by flipping column index
    }

    // 3) draw Module 1: rotate 90° CCW, then mirror horizontally
    //    rotation CCW: (r0,c0)→(7−c0,r0)
    for (int colP = 0; colP < 8; colP++) {
      byte val = 0;
      for (int rowP = 0; rowP < 8; rowP++) {
        int origRow =  colP;
        int origCol = 7 - rowP;
        bool pixel = (window[8 + origCol] >> (7 - origRow)) & 0x01;
        if (pixel) val |= 1 << (7 - rowP);
      }
      lc.setColumn(1, 7 - colP, val);
    }

    delay(scrollSpeed);
  }

  // 4) end-of-cycle “wipe” clear left→right
  for (int c = 0; c < totalCols; c++) {
    int dev = c / 8;
    int col = c % 8;
    lc.setColumn(dev, col, 0x00);
    delay(clearSpeed);
  }
}

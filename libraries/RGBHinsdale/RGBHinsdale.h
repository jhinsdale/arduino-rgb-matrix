/*
 * RGBHinsdale -- modified RGBmatrixPanel implementing fast
 * horizontal scrolling.
 *
 * This should be 100% compatible with RGBmatrixPanel and not incur
 * any significant loss of performance if none of the added features
 * are used.
 *
 * Adapted from the original work by AdaFruit (See RGBHinsdale.cpp
 * for complete notice)
 *
 * Portions Copyright 2014 by John K. Hinsdale <hin@alma.com>
 * Use under BSD license with above copyright notice(s) retained.
 */

#ifndef _RGB_HINSDALE_H
#define _RGB_HINSDALE_H

#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
 #include "pins_arduino.h"
#endif
#include "Adafruit_GFX.h"

class RGBHinsdale : public Adafruit_GFX {

 public:

  // Constructor for 16x32 panel:
  RGBHinsdale(uint8_t a, uint8_t b, uint8_t c,
    uint8_t sclk, uint8_t latch, uint8_t oe, boolean dbuf);

  // Constructor for 32x32 panel (adds 'd' pin):
  RGBHinsdale(uint8_t a, uint8_t b, uint8_t c, uint8_t d,
    uint8_t sclk, uint8_t latch, uint8_t oe, boolean dbuf);

  void
    begin(void),
    drawPixel(int16_t x, int16_t y, uint16_t c),
    fillScreen(uint16_t c),
    updateDisplay(void),
    swapBuffers(boolean),
    dumpMatrix(void);
  uint8_t
    *backBuffer(void);
  uint16_t
    Color333(uint8_t r, uint8_t g, uint8_t b),
    Color444(uint8_t r, uint8_t g, uint8_t b),
    Color888(uint8_t r, uint8_t g, uint8_t b),
    Color888(uint8_t r, uint8_t g, uint8_t b, boolean gflag),
    ColorHSV(long hue, uint8_t sat, uint8_t val, boolean gflag);

  // Proportional text: draw 8-high column of bits at text cursor,
  // in text color, advancing cursor, and wrapping
  void drawTextBitColumn(uint8_t colbits);
  
  // SCROLLING
  
  // Update horizontal scroll offset, also shifting in data
  // if any was set
  void setScrollOffset(uint8_t);
  
  // Draw a pixel to the scroll buffer column
  void drawPixelToScrollColumn(int16_t y, uint16_t c);

  // Mark scroll column data as ready for use on next call to
  // setScrollOffset()
  void setScrollDataAvailable();

  // ENABLE/DISABLE
  void enableUpdate(boolean);

  // Timer delays
  void setTimerDelays(uint16_t *);
  void initTimerDelays();

 private:

  uint8_t         *matrixbuff[2];
  uint8_t          nRows;
  volatile uint8_t backindex;
  volatile boolean swapflag;

  // Init/alloc code common to both constructors:
  void init(uint8_t rows, uint8_t a, uint8_t b, uint8_t c,
    uint8_t sclk, uint8_t latch, uint8_t oe, boolean dbuf);

  // PORT register pointers, pin bitmasks, pin numbers:
  volatile uint8_t
    *latport, *oeport, *addraport, *addrbport, *addrcport, *addrdport;
  uint8_t
    sclkpin, latpin, oepin, addrapin, addrbpin, addrcpin, addrdpin,
    _sclk, _latch, _oe, _a, _b, _c, _d;

  // Counters/pointers for interrupt handler:
  volatile uint8_t row, plane;
  volatile uint8_t *buffptr;

  // ENABLE/DISABLE
  boolean update_enabled;
  boolean update_enabled_next; // Requested value to set at next update

  // SCROLLING
  void initScroll(boolean);

  // Offset - how many pixels to shift left.  0 ... width-1
  // Note that a "shift-right" of N is achieved by doing
  // a shift-left of width-N.
  uint8_t          horiz_scroll;

  // Next scroll offset, and flag that we need to set it
  boolean          scroll_flag;
  uint8_t          horiz_scroll_next;

  // If "dbuf" given at construction time: Optional column of data to
  // copy "atomically" at shift/refresh time Each pixel uses a 16-bit
  // color value, == 32 extra bytes for the 16x32 and 64 additional
  // bytes for the 32x32.  This is an increase of about 4% of the main
  // pixel buffer.
  uint16_t          *scroll_buff;
  boolean       	scroll_data_available;

  // Timer delays
  uint16_t timer_delays[4];
};

#endif // _RGB_HINSDALE_H

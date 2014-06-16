/*
  "Universal" serial-port driven, multi-modal sketch.  Hosted program
  (which must agree with this code) sends command over the serial port
  that call the matrix graphics library.
       John K. Hinsdale

  Copyright 2014 by John K. Hinsdale <hin@alma.com>
  Use under BSD license with above copyright notice(s) retained.
*/

// Need to include ALL these so that the IDE passes each's respective
// include dir to the compiler, and object code to the linker.
// See http://provideyourown.com/2011/advanced-arduino-including-multiple-libraries/
// All of them need to have include guards since they are included in
// their dependents' headers as well(!)
#include "Adafruit_GFX.h"   // Core graphics library
// Uncomment appropriate line to use the new class
// #include "RGBmatrixPanel.h" // Hardware-specific library
#include "RGBHinsdale.h" // Scrolling version of RBmatrixPanel

// If your 32x32 matrix has the SINGLE HEADER input,
// use this pinout:
#define CLK 8  // MUST be on PORTB! (Use pin 11 on Mega)
#define OE  9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
#define D   A3
// If your matrix has the DOUBLE HEADER input, use:
//#define CLK 8  // MUST be on PORTB! (Use pin 11 on Mega)
//#define LAT 9
//#define OE  10
//#define A   A3
//#define B   A2
//#define C   A1
//#define D   A0

// Swap to use the new class
RGBHinsdale matrix(A, B, C, D, CLK, LAT, OE, false);
// RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false);

/*****************************************/
// Operations

// RESET - Reset (draining serial buffer) ... Send lots of these to
// eventually trigger use as op and re-sync
#define OP_RESET              0

// BLANK_SCREEN - Blank the screen
#define OP_BLANK_SCREEN       1

// DEMO - Run demo
#define OP_DEMO               2

// FILL-SCREEN - Fill screen (color)
#define OP_FILL_SCREEN        3

// SET_TEXT_PARAMS - Set text params (cursor_x, cursor_y, size, wrap)
#define OP_SET_TEXT_PARAMS    4

// SET_TEXT_COLOR - Set text color (fg, bg) bg==fg -> "transparent"
#define OP_SET_TEXT_COLOR     5

// WRITE_TEXT - Write text at cursor (count, char, char ...)
#define OP_WRITE_TEXT         6

// DRAW_PIXEL - Draw pixel (x, y, color)
#define OP_DRAW_PIXEL         7

// DRAW_LINE - Draw line (x0, y0, x1, y1, color)
#define OP_DRAW_LINE          8

// DRAW_RECT - Draw rect (x, y, w, h, color, is_filled, round_radius)
#define OP_DRAW_RECT          9

// DRAW_TRIANGLE - Draw triangle (x0, y0, x1, y1, x2, y2, color, is_filled)
#define OP_DRAW_TRIANGLE     10

// DRAW_CIRCLE - Draw circle (x, y, r, color, is_filled)
#define OP_DRAW_CIRCLE       11

// DRAW_BITMAP - Draw bitmap  (x, y, w, h, color, bytes ...)
#define OP_DRAW_BITMAP       12

// XOR_RECT - XOR region (x, y, w, h, color)
#define OP_XOR_RECT          13

// WRITE_BITCOLS - Draw bit columns at cursor (count, bits1, bits2 ...)
#define OP_WRITE_BITCOLS     14

// COPY_RECT - Copy rectangle (x, y, w, h, new_x, new_y)
#define OP_COPY_RECT         15

// WIPE - Wipe (erase) the display (wipe_type, color, delay_ms)
#define OP_WIPE              16

// SLEEP - Sleep for ms
#define OP_SLEEP             30

// PING - Request to send a character ("!") back, useful for flow control
#define OP_PING              31

// SET_SCROLL_OFFSET - Set the horizontal scroll offset (n)
#define OP_SET_HORIZ_SCROLL  50

// SET_TIMER_DELAYS - Set clock-tick durations by plane d0, d1, d2, d3
#define OP_SET_TIMER_DELAYS  60

// DUMP_MATRIX - Dump out the matrix to serial port
#define OP_DUMP_MATRIX      250
#define OP_DUMP_MATRIX_RECT 251

// More ops (draw image, scroll, etc.)
// ...

// Wipe types, name refers to "direction of erasure," e.g., "DOWN"
// blanks out lines from top to bottom.
#define WIPE_DOWN        1
#define WIPE_UP          2
#define WIPE_LEFT        3
#define WIPE_RIGHT       4
#define WIPE_RADIAL_OUT  5
#define WIPE_RADIAL_IN   6
#define WIPE_DISSOLVE    7

// Set bit rate
#define BIT_RATE 115200

// Track the current operation
unsigned char cur_op = OP_BLANK_SCREEN;
/*****************************************/

// Button state
#define BUTTON_PIN 11
unsigned char button_down = 0;

// Forward declarations - need these if want to compile outside the IDE, e.g.
// with arduino-sketch.py
void blank();
void button_init();
unsigned char button_pressed();
void demo_loop();
void flush_input();
void handle_op(unsigned char);
uint8_t read_byte();
uint16_t read_int();
void write_byte(uint8_t);
void write_int_hex(uint16_t);
void write_nibble(uint8_t);
void handle_RESET();
void handle_BLANK_SCREEN();
void handle_FILL_SCREEN();
void handle_SET_TEXT_PARAMS();
void handle_SET_TEXT_COLOR();
void handle_WRITE_TEXT();
void copyPixel();
void handle_DRAW_PIXEL();
void handle_DRAW_LINE();
void handle_DRAW_RECT();
void handle_XOR_RECT();
void handle_WRITE_BITCOLS();
void handle_COPY_RECT();
void handle_WIPE();
void handle_DRAW_TRIANGLE();
void handle_DRAW_CIRCLE();
void handle_DRAW_BITMAP();
void handle_DUMP_MATRIX();
void handle_DUMP_MATRIX_RECT();
void handle_SET_HORIZ_SCROLL();
void handle_SET_TIMER_DELAYS();
void handle_SLEEP();
uint16_t read_color();
void getPixel(uint8_t, uint8_t, uint8_t *, uint8_t *, uint8_t *);
void dump_matrix_to_serial(uint8_t, uint8_t, uint8_t, uint8_t);

/* setup() **************************************************/
void setup() {
  matrix.begin();
  Serial.begin(BIT_RATE);
  // Init the button
  button_init();

  // Blank the screen
  blank();
}

/* Main loop */
void loop() {
  // See if get a mode change, either via button or serial port
  if ( button_pressed() ) {
    // Serial.println("Button press ...");
    flush_input(); // Drain the serial any time button is pressed
    if ( cur_op == OP_DEMO ) {
      cur_op = OP_RESET;
      handle_op(cur_op);
    }
    else
      cur_op = OP_DEMO;
  }
  else if ( Serial.available() ) {
    cur_op = read_byte();
    handle_op(cur_op);
  }
  // Loop based on current mode: if a "continuously updating" mode, call its loop function
  // Else do nothing and display static content
  if ( cur_op == OP_DEMO )
    demo_loop();
}

// Handlers
// RESET
void handle_RESET() {
  // Flush input, flash green and blank screen
  flush_input();
  matrix.setScrollOffset(0);
  matrix.initTimerDelays();
  matrix.fillScreen(matrix.Color888(0, 50, 0));
  delay(250);
  blank();
  cur_op = OP_BLANK_SCREEN;
}
// BLANK_SCREEN
void handle_BLANK_SCREEN() {
  blank();
}
// FILL_SCREEN (color)
void handle_FILL_SCREEN() {
  uint16_t color = read_color();
  matrix.fillScreen(color);
}
// SET_TEXT_PARAMS (cursor_x, cursor_y, size, wrap)
void handle_SET_TEXT_PARAMS() {
  uint8_t curs_x = read_byte();
  uint8_t curs_y = read_byte();
  uint8_t txt_size = read_byte();
  uint8_t wrap_flag = read_byte();
  matrix.setCursor(curs_x, curs_y);
  matrix.setTextSize(txt_size);
  matrix.setTextWrap(wrap_flag);
}
// SET_TEXT_COLOR (fg, bg) bg==fg -> "transparent"
void handle_SET_TEXT_COLOR() {
  uint16_t fg = read_color();
  uint16_t bg = read_color();
  matrix.setTextColor(fg, bg);
}
// WRITE_TEXT at cursor (count, char, char ...)
void handle_WRITE_TEXT() {
  uint8_t n = read_byte();
  int i;
  for ( i=0; i < n; i++ ) {
    uint8_t c = read_byte();
    matrix.write(c);
  }
}
// DRAW_PIXEL (x, y, color)
void handle_DRAW_PIXEL() {
  uint8_t x = read_byte();
  uint8_t y = read_byte();
  uint16_t color = read_color();
  matrix.drawPixel(x, y, color);
}
// DRAW_LINE (x0, y0, x1, y1, color)
void handle_DRAW_LINE() {
  uint8_t x0 = read_byte();
  uint8_t y0 = read_byte();
  uint8_t x1 = read_byte();
  uint8_t y1 = read_byte();
  uint16_t color = read_color();
  matrix.drawLine(x0, y0, x1, y1, color);
}
// DRAW_RECT (x, y, w, h, color, is_filled, round_radius)
void handle_DRAW_RECT() {
  uint8_t x = read_byte();
  uint8_t y = read_byte();
  uint8_t w = read_byte();
  uint8_t h = read_byte();
  uint16_t color = read_color();
  uint8_t is_filled = read_byte();
  uint8_t r = read_byte();
  if ( r > 0 ) {
    if ( is_filled )
      matrix.fillRoundRect(x, y, w, h, r, color);
    else
      matrix.drawRoundRect(x, y, w, h, r, color);
  }
  else {
    if ( is_filled )
      matrix.fillRect(x, y, w, h, color);
    else
      matrix.drawRect(x, y, w, h, color);
  }
}
// XOR_RECT (x, y, w, h, color_mask)
void handle_XOR_RECT() {
  uint8_t x = read_byte();
  uint8_t y = read_byte();
  uint8_t w = read_byte();
  uint8_t h = read_byte();
  uint16_t color_mask = read_color();
  uint8_t row = y, row_limit = y + h, col = x, col_limit = x + w;
  uint8_t r, g, b;
  for ( row = y; row < row_limit; row++ ) {
    for ( col = x; col < col_limit; col++ ) {
      uint16_t new_color;
      getPixel(col, row, &r, &g, &b);
      uint16_t current_color = matrix.Color444(r, g, b);
      // As a special case, swap pixels exactly equal to the color_mas
      // with black This will provide better "reverse-video" in the
      // common case where text that happens to be the same as the
      // temporary background, when the permanent background is black.
      if ( current_color == 0 )
        new_color = color_mask;
      else if ( current_color == color_mask )
        new_color = 0;
      else
        new_color = current_color ^ color_mask;
      matrix.drawPixel(col, row, new_color);
    }
  }
}
// WRITE_BITCOLS (count, bits1, bits2 ...)
void handle_WRITE_BITCOLS() {
  uint8_t n = read_byte();
  while ( n-- ) {
    uint8_t bc = read_byte();
    matrix.drawTextBitColumn(bc);
  }
}
// copyPixel
void copyPixel(uint8_t src_x, uint8_t src_y, uint8_t dst_x, uint8_t dst_y) {
  uint8_t r, g, b;
  getPixel(src_x, src_y, &r, &g, &b);
  matrix.drawPixel(dst_x, dst_y, matrix.Color444(r, g, b));
}

// COPY_RECT (x, y, w, h, new_x, new_y)
/* Copy a rectangle to destination.  If moved to the right and/or down
   so that the rectangle does not fit, it will clip.  Both source and
   destination origins must be in range, as must the source rectangle.
*/   
void handle_COPY_RECT() {
  uint8_t x = read_byte();
  uint8_t y = read_byte();
  uint8_t w = read_byte();
  uint8_t h = read_byte();
  uint8_t new_x = read_byte();
  uint8_t new_y = read_byte();

  // Validate inputs
  if ( x >= matrix.width() || y >= matrix.height()
       || (x + w) >= matrix.width() || (y + h) >= matrix.height()
       || new_x >= matrix.width() || new_y >= matrix.height() )
    return;

  // Clip width and height if will not fit at destination
  if ( (new_x + w) > matrix.width() )
    w = matrix.width() - new_x;
  if ( (new_y + h) > matrix.height() )
    h = matrix.height() - new_y;
  
  /* Copy row-wise or column-wise, and direction that will not clobber
     data when source and destination regions overlap
  */
  if ( new_y < y ) {
    // copy row-wise top-to-bottom
    // [y ... y+h-1] -> [new_y ... new_y+h-1]
  }
  else if ( new_y > y ) {
    // copy row-wise bottom-to-top
    // [y+h-1 ... y] -> [new_y+h-1 ... y]
  }
  else if ( new_x < x ) {
    // copy column-wise left-to-right
    // [x ... x+w-1] -> [new_x ... new_x+w-1]
  }
  else if ( new_x > x ) {
    // copy column-wise right-to-left
    // [x+w-1 ... x] -> [new_x+w-1 ... new_x]
  }
  // Else: source and dest coords are equal - do nothing
}

// WIPE (wipe_type, color, delay_ms)
// Note that delay_ms is time between operations, the number of which depend
// on the wipe type (32 for "straightedge", 16 for radial, 1024 for dissolve).
// Adjust accordingly to achieve total desired wipe time.
void handle_WIPE() {
  uint8_t wipe_type = read_byte();
  uint16_t color = read_color();
  uint16_t delay_ms = read_int();

  // Row-wise and column-wise - one pass, 32 operations
  if ( wipe_type == WIPE_UP || wipe_type == WIPE_DOWN || wipe_type == WIPE_LEFT || wipe_type == WIPE_RIGHT ) {
    uint8_t x0_start, y0_start, x1_start, y1_start;
    int8_t x_incr, y_incr;
    uint8_t count;
    if ( wipe_type == WIPE_UP ) {
      x0_start = 0;
      y0_start = x1_start = y1_start = 31;
      x_incr = 0;
      y_incr = -1;
      count = matrix.height();
    }
    else if ( wipe_type == WIPE_DOWN ) {
      x0_start = y0_start = y1_start = 0;
      x1_start = 31;
      x_incr = 0;
      y_incr = 1;
      count = matrix.height();
    }
    else if ( wipe_type == WIPE_LEFT ) {
      x0_start = x1_start = y1_start = 31;
      y0_start = 0;
      x_incr = -1;
      y_incr = 0;
      count = matrix.width();
    }
    else if ( wipe_type == WIPE_RIGHT ) {
      x0_start = y0_start = x1_start = 0;
      y1_start = 31;
      x_incr = 1;
      y_incr = 0;
      count = matrix.width();
    }

    // Run the loop for row-wise or column-wise wipe
    uint8_t x0 = x0_start, y0 = y0_start, x1 = x1_start, y1 = y1_start;
    for (int i=0; i < count; i++) {
      matrix.drawLine(x0, y0, x1, y1, color);
      if ( delay_ms && i < (count - 1) )
        delay(delay_ms);
      x0 += x_incr;
      x1 += x_incr;
      y0 += y_incr;
      y1 += y_incr;
    }
  }
  // Radial - one pass, 16 operations.  Draw squares in the background color.
  else if ( wipe_type == WIPE_RADIAL_OUT || wipe_type == WIPE_RADIAL_IN ) {
    uint8_t xy_start, s_start;
    int xy_incr, s_incr;
    if ( wipe_type == WIPE_RADIAL_OUT ) {
      xy_start = 15;
      s_start = 2;
      xy_incr = -1;
      s_incr = 2;
    }
    else if ( wipe_type == WIPE_RADIAL_IN ) {
      xy_start = 0;
      s_start = 32;
      xy_incr = 1;
      s_incr = -2;
    }

    // Run the loop for radial wipe
    uint8_t xy = xy_start, s = s_start;
    int count = matrix.height() / 2;
    for (int i=0; i < count; i++) {
      matrix.drawRect(xy, xy, s, s, color);
      if ( delay_ms && i < (count - 1) )
        delay(delay_ms);
      xy += xy_incr;
      s += s_incr;
    }
  }
  // Dissolve - random iteration
  /* To iterate randomly over the array of 32x32=1024 pixels with full
     coverage, using no memory, we use a special version of a linear
     congruential number generator that is guaranteed to cover every
     element of the interval [0, 1023]

     Linear congruential generator, uses (a, c, m):
         x' = (a*x + c) % m
     Where
         0 < a
         c < m

     By the Hull-Dobell theorem, full coverage of all integers
     [0...m-1] is guaranteed if we have all of:
         c and m relatively prime (e.g. for m == 1024, c = 3^n will do)
         a-1 divisible by every prime factor of m.  For m == 1024: a-1 is even, i.e., a is odd)
         if m divisible by 4, then a-1 is also divisible by four. For m == 1024: a = 4*n + 1 for n > 0

     So here, we will use:
         m = 1024
         a = 4*2 + 1 = 9
         c = 3^4 = 81
  */
  else if ( wipe_type == WIPE_DISSOLVE ) {
    uint16_t x = 0;
    uint8_t row, col;
    int i;
    for ( i=0; i < 1024; i++ ) {
      // x = (a * x + c) % m
      // x = (9*x + 81) % 1024
      x = (9*x + 81) & 0x3FF;
      col = x & 0x1F; // x % 32
      row = x >> 5; // floor(x/32)
      matrix.drawPixel(col, row, color);
      // Clip delay_ms at 10240 (10 seconds so it doesn't take forever)
      if ( delay_ms > 0 && delay_ms <= 10240 && i < 1023 )
        delay(delay_ms);
    }
  }
}
// DRAW_TRIANGLE (x0, y0, x1, y1, x2, y2, color, is_filled)
void handle_DRAW_TRIANGLE() {
  uint8_t x0 = read_byte();
  uint8_t y0 = read_byte();
  uint8_t x1 = read_byte();
  uint8_t y1 = read_byte();
  uint8_t x2 = read_byte();
  uint8_t y2 = read_byte();
  uint16_t color = read_color();
  uint8_t is_filled = read_byte();
  if ( is_filled )
    matrix.fillTriangle(x0, y0, x1, y1, x2, y2, color);
  else
    matrix.drawTriangle(x0, y0, x1, y1, x2, y2, color);
}
// DRAW_CIRCLE (x, y, r, color, is_filled)
void handle_DRAW_CIRCLE() {
  uint8_t x = read_byte();
  uint8_t y = read_byte();
  uint8_t r = read_byte();
  uint16_t color = read_color();
  uint8_t is_filled = read_byte();
  if ( is_filled )
    matrix.fillCircle(x, y, r, color);
  else
    matrix.drawCircle(x, y, r, color);
}
// DRAW_BITMAP
void handle_DRAW_BITMAP() {
}
// DUMP_MATRIX
void handle_DUMP_MATRIX() {
  dump_matrix_to_serial(0, matrix.height() - 1, 0, matrix.width() - 1);
}
// DUMP_MATRIX_RECT
void handle_DUMP_MATRIX_RECT() {
  uint8_t row_start = read_byte();
  uint8_t row_end = read_byte();
  uint8_t col_start = read_byte();
  uint8_t col_end = read_byte();
  dump_matrix_to_serial(row_start, row_end, col_start, col_end);  
}
// SET_HORIZ_SCROLL
void handle_SET_HORIZ_SCROLL() {
  uint8_t n = read_byte();
  matrix.setScrollOffset(n);
}
// SET_TIMER_DELAYS
void handle_SET_TIMER_DELAYS() {
  uint16_t d[4];
  d[0] = read_int();
  d[1] = read_int();
  d[2] = read_int();
  d[3] = read_int();
  matrix.setTimerDelays(d);
}
// SLEEP
void handle_SLEEP() {
  uint16_t ms = read_int();
  delay(ms);
}
// PING
void handle_PING() {
  // Send a byte back so the client driver can wait to read it.  This
  // is a crude "command level" flow control.
  write_byte('!');
}
void dump_matrix_to_serial(uint8_t row_start, uint8_t row_end, uint8_t col_start, uint8_t col_end) {
  uint16_t last_color = 0;
  uint16_t count = 0;
  boolean first = true;
  uint8_t row, col;
  uint8_t r = 0, g = 0, b = 0;

  write_byte('>');
  write_byte('\n');
  for (row = row_start; row >= 0 && row <= row_end && row < matrix.height(); row++) {
    for (col = col_start; col >= 0 && col <= col_end && col < matrix.width(); col++) {
      getPixel(col, row, &r, &g, &b);
      uint16_t color = matrix.Color444(r, g, b);
      if ( first ) {
        last_color = color;
        count = 1;
        first = false;
      }
      else {
        if ( color == last_color )
          count++;
        else {
          write_int_hex(last_color);
          if ( count > 1 ) {
            write_byte(':');
            write_int_hex(count);
          }
          write_byte('\n');
          last_color = color;
          count = 1;
        }
      }
    } // Each column
  } // Each row

  // Don't forget that last one!
  write_int_hex(last_color);
  if ( count > 1 ) {
    write_byte(':');
    write_int_hex(count);
  }
  write_byte('\n');
  write_byte('<');
  write_byte('\n');
}

// Handle operation - dispatch on op code
void handle_op(unsigned char op) {
  // Serial.print("Dispatching on op=");
  // Serial.println(op);
  switch( op ) {
    case OP_RESET:
      handle_RESET();
      break;
    case OP_BLANK_SCREEN:
      handle_BLANK_SCREEN();
      break;
    case OP_DEMO:
      // do nothing
      break;
    case OP_FILL_SCREEN:
      handle_FILL_SCREEN();
      break;
    case OP_SET_TEXT_PARAMS:
      handle_SET_TEXT_PARAMS();
      break;
    case OP_SET_TEXT_COLOR:
      handle_SET_TEXT_COLOR();
      break;
    case OP_WRITE_TEXT:
      handle_WRITE_TEXT();
      break;
    case OP_DRAW_PIXEL:
      handle_DRAW_PIXEL();
      break;
    case OP_DRAW_LINE:
      handle_DRAW_LINE();
      break;
    case OP_DRAW_RECT:
      handle_DRAW_RECT();
      break;
    case OP_XOR_RECT:
      handle_XOR_RECT();
      break;
    case OP_WRITE_BITCOLS:
      handle_WRITE_BITCOLS();
      break;
    case OP_COPY_RECT:
      handle_COPY_RECT();
      break;
    case OP_WIPE:
      handle_WIPE();
      break;
    case OP_DRAW_TRIANGLE:
      handle_DRAW_TRIANGLE();
      break;
    case OP_DRAW_CIRCLE:
      handle_DRAW_CIRCLE();
      break;
    case OP_DRAW_BITMAP:
      handle_DRAW_BITMAP();
      break;
    case OP_DUMP_MATRIX:
      handle_DUMP_MATRIX();
      break;
    case OP_DUMP_MATRIX_RECT:
      handle_DUMP_MATRIX_RECT();
      break;
    case OP_SET_HORIZ_SCROLL:
      handle_SET_HORIZ_SCROLL();
      break;
    case OP_SET_TIMER_DELAYS:
      handle_SET_TIMER_DELAYS();
      break;
    case OP_SLEEP:
      handle_SLEEP();
      break;
    case OP_PING:
      handle_PING();
      break;

    default:
      Serial.print("UNKOWN OPERATION CODE: ");
      Serial.println(op);
      break;
  }
}

// Get the 4-bit R/G/B values at pixel X,Y and store in given addresses
// This is basically the "read" version of drawPixel() which "writes" data

// Copied (!)
#define nPlanes 4

void getPixel(uint8_t x, uint8_t y, uint8_t *rp, uint8_t *gp, uint8_t *bp) {
  uint8_t bit, limit, *ptr;
  uint8_t wid = matrix.width();

  // nRows is half of actual rows
  uint8_t nRows = matrix.height();
  nRows >>= 1; // Dived it by two
  
  uint8_t * matrixbuff_p = matrix.backBuffer();
  // First set all colors to zero, we'll set bits individually below
  uint8_t r = 0, g = 0, b = 0;
  
  if((x < 0) || (x >= wid) || (y < 0) || (y >= matrix.height())) return;

  // Loop counter stuff
  bit   = 2;
  limit = 1 << nPlanes;

  if(y < nRows) {
    // Data for the upper half of the display is stored in the lower
    // bits of each byte.
    ptr = &matrixbuff_p[y * wid * (nPlanes - 1) + x]; // Base addr
    // Plane 0 is a tricky case -- its data is spread about,
    // stored in least two bits not used by the other planes.

    // Don't need to do this as we are just reading
    // ptr[64] &= ~B00000011;            // Plane 0 R,G mask out in one op

    if(ptr[64] & B00000001) r |= 1; // Plane 0 R: 64 bytes ahead, bit 0
    if(ptr[64] & B00000010) g |= 1; // Plane 0 G: 64 bytes ahead, bit 1
    if(ptr[32] & B00000001) b |= 1; // Plane 0 B: 32 bytes ahead, bit 0
    // Don't need to do this as we are just reading
    // else      ptr[32] &= ~B00000001;  // Plane 0 B unset; mask out

    // The remaining three image planes are more normal-ish.
    // Data is stored in the high 6 bits so it can be quickly
    // copied to the DATAPORT register w/6 output lines.
    for(; bit < limit; bit <<= 1) {
      // Don't need to do this as we are just reading
      // *ptr &= ~B00011100;             // Mask out R,G,B in one op
      if(*ptr & B00000100) r |= bit;  // Plane N R: bit 2
      if(*ptr & B00001000) g |= bit;  // Plane N G: bit 3
      if(*ptr & B00010000) b |= bit;  // Plane N B: bit 4
      ptr  += wid;                  // Advance to next bit plane
    }
  } else {
    // Data for the lower half of the display is stored in the upper
    // bits, except for the plane 0 stuff, using 2 least bits.
    ptr = &matrixbuff_p[(y - nRows) * wid * (nPlanes - 1) + x];
    // Don't need to do this as we are just reading
    // *ptr &= ~B00000011;               // Plane 0 G,B mask out in one op
    if(ptr[32] & B00000010) r |= 1; // Plane 0 R: 32 bytes ahead, bit 1
    // Don't need to do this as we are just reading
    // else       ptr[32] &= ~B00000010; // Plane 0 R unset; mask out
    if(*ptr & B00000001)    g |= 1; // Plane 0 G: bit 0
    if(*ptr & B00000010)    b |= 1; // Plane 0 B: bit 0
    for(; bit < limit; bit <<= 1) {
      // Don't need to do this as we are just reading
      // *ptr &= ~B11100000;             // Mask out R,G,B in one op
      if(*ptr & B00100000) r |= bit;  // Plane N R: bit 5
      if(*ptr & B01000000) g |= bit;  // Plane N G: bit 6
      if(*ptr & B10000000) b |= bit;  // Plane N B: bit 7
      ptr  += wid;                  // Advance to next bit plane
    }
  }
  // Set results
  *rp = r;
  *gp = g;
  *bp = b;
}

// Read a color from serial.  Currently read R/G/B bytes and converts.  In future
// can read the 5/6/5 encoded triple with read_int()
uint16_t read_color() {
  uint8_t r = read_byte();
  uint8_t g = read_byte();
  uint8_t b = read_byte();
  return matrix.Color444(r, g, b);
}

// Read a byte, retrying until we get it, or time out, or button pressed
uint8_t read_byte() {
  uint16_t nretry = 0;
  while ( !Serial.available() ) {
    if ( button_pressed() )
      return 0;
    nretry++;
    if ( nretry > 500 ) // == 5 secs at 10ms delay interval
      return 0;
    delay(10);
  }
  return Serial.read();
}

// Read a two byte int, MSB sent first
uint16_t read_int() {
  uint8_t msb = read_byte();
  uint8_t lsb = read_byte();
  return 256 * msb + lsb;
}

// Write a byte, retyring until we can write it, or time out, or button pressed
void write_byte(uint8_t b) {
  uint16_t nretry = 0;
  while ( !Serial.write(b) ) {
    if ( button_pressed() )
      return;
    nretry++;
    if ( nretry > 500 ) // == 5 secs at 10ms delay interval
      return;
    delay(10);
  }
}

// Write a byte as one or two hex digits, maybe truncating leading zeros
void write_byte_hex(uint8_t b, boolean truncate_leading) {
  uint8_t nb;
  if ( !truncate_leading || b > 0XF ) {
    nb = (b >> 4) & 0xF;
    write_nibble(nb);
  }
  nb = b & 0xF;
  write_nibble(nb);
}

// Write a 4-bit nibble as a single hex digit
void write_nibble(uint8_t nb) {
  if ( nb >= 0 && nb <= 9 )
    write_byte('0' + nb);
  else if ( nb >= 10 && nb <= 15 )
    write_byte('a' + (nb - 10));
  else
    write_byte('?');
}

// Write an int
void write_int(uint16_t i) {
  uint8_t b = (i >> 8) & 0xFF;
  write_byte(b);
  b = i & 0xFF;
  write_byte(b);
}

// Write an int, truncating leading zeroes
void write_int_hex(uint16_t i) {
  boolean truncate = true;
  uint8_t b;
  if ( i > 0xFF ) {
    b = (i >> 8) & 0xFF;
    write_byte_hex(b, true);
    // Don't truncate leading zeroes on the lower-order byte
    truncate = false;
  }
  b = i & 0xFF;
  write_byte_hex(b, truncate);
}

// Check if button was pressed
unsigned char button_pressed() {
  int was_pressed = 0;
  int button_val = digitalRead(BUTTON_PIN);
  // Serial.println(button_val);
  if ( button_val == HIGH ) { // Button is up now
    if ( button_down ) { // Button was down before -- release event
      button_down = 0;
    }
  }
  else { // Button is down now
    if ( !button_down ) { // Button was up before -- press event 
      button_down = 1;
      was_pressed = 1;
    }
  }
  return was_pressed;
}

// Set up button (call from setup() ... ).  Note that docs say the button
// is on pin 10 but the board shows it as being on 11.
void button_init() {
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH);
}

// Blank the screen
void blank() {
  matrix.fillScreen(0);
}
void blank2() {
  unsigned char x, y;
    for(y=0; y<matrix.height(); y++) {
      for(x=0; x<matrix.width(); x++) {
        matrix.drawPixel(x, y, matrix.Color333(0, 0, 0));
      }
    }
}

// Drain the serial port
void flush_input() {
  while ( Serial.available() ) {
    while (Serial.available() ) {
      Serial.read();
      delay(250);
    }
  }
}

/*** demo ***/
void demo_loop() {
  static int direction = 1;
  static uint8_t iter = 0;
  static uint8_t r = 0, g = 0, b = 0;
  
  r++;
  if ( r == 4 ) {
    r = 0;
    g++;
    if ( g == 4 ) {
      g = 0;
      b++;
      if ( b == 4 )
        b = 0;
    }
  }
  matrix.drawRect(iter, iter, 32 - 2*iter, 32 - 2 * iter, matrix.Color444(r, g, b));

  if ( iter == 15 )
    direction = -1;
  if ( iter == 0 )
    direction = 1;
  iter += direction;
    
  delay(30);
}

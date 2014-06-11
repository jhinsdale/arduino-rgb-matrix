
Copyright 2014 by John K. Hinsdale <hin@alma.com>
Use under BSD license with above copyright notice(s) retained.

RGBHinsdale -- minimal-memory, horizontally scrollable 32x32 RGB
               matrix driver

This modified version of the AdaFruit display driver is made to work
closely with the "universal_pixel" sketch, which feeds commands into
the display via the serial port.

    VARIABLE WIDTH FONT SUPPORT

Variable width font is supported by providing an operation that paints
an 8-bit high column of pixels across an 8-bit high text row on the
display.  This is the function drawTextBitColumn.  The bits are fed in
from a host via serial.  In this way, large amounts of text can be put
to the display, while, because of the use of variable width font, more
letters can fit on a line of the display.

In order to scroll text horizontally and quickly, we need to modify
the display drive to support horizonta scrolling at a low level (see
below).

    HORIZONTAL SCROLLING SUPPORT (NOT WORKING YET)

This is modified version of Adafruit's RGBmatrixPanel class that supports
fast horizontal scrolling, without using a double-buffer.  It works by
treating the pixel memory as a circularized buffer with a particular
column offset that allows it to shift horizontally without moving
memory around.  In addition, an operation is added to the update logic
that causes a single column of pixels to be "shifted in" as
"atomically" a possible, acting essentially as a one-column backing
buffer.  All this means that it will work with the 32x32 board on the
memory-limited Arduino Uno.

This capability can then be used to create a scroll-off "wipe" as well
as a horizontally rotating "animation" by looping.  The main
application is to support a horiztonally scrolling "text crawl" with
the (potentially voluminous) text being transferred in from a host PC
via the serial port.

HOW IT WORKS:

We introduce a "scroll offset" and re-write the updateDisplay() method
to bump the display data pointer to the correct place, doing two
passes of data copying for the two contiguous regions of memory being
copied.  This requires abandoning the loop-unrolling optimization.

If there is no scrolling offset in place, that is detected, and all
the original code, including the loop-unroll, is used.  Thus there
should be no degredation due to use of this class when there is no
scrolling on.  Also, it should be immunte to any bugs or glitches
introduced by the horizontal scrolling (which is good because it
does not yet work!).

This code should work on both the 16x32 and 32x32 panels,
though 16x32 can be double-buffered on the Uno.

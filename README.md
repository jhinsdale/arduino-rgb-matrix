
arduino-rgb-matrix
==================

Enhancements, libraries and utilities for the Adafruit RGB LED Matrix
Panel.  Created in Spring of 2014 by John K. Hinsdale and first
uploaded to github on June 11, 2014.

This git project contains a number of parts:

libraries/RGBHinsdale/

    A modified version of the AdaFruit RGBmatrixPanel display driver.
    It omits gamma processing (to save memory) and introduces an
    alternate way to control brightness by adjusting the PWM (BCM)
    timer intervals.

    It also adds helpers for painting the display with 8-bit high "bit
    columns" used to create variable width font text letters.

    Finally, it includes unfinished code for horizontally scrolling
    the display contents leaving the memory in place, so that no
    double buffer is required.  This makes scrolling possible on the
    larger 32x32 display on the memory-limited Uno chip.

universal_pixel/

    A "universal" Arduino sketch that listens on the serial port for
    commands (which consist of an "opcode" byte followed by optional,
    relevant arguments) that then invoke the functions of the AdaFruit
    graphics library.  By exposing the entire library in this sketch,
    it should be possible to completely manipulate the display from
    Python by sending the appropriate commands.

py/
font/

    Python code which drives the "universal" sketch by feeding it
    commands.  This allows manipulation from python in a high-level
    way.  A "ping" operation can be used to avoid overflowing the
    serial port, so that very large amounts of text and graphics
    commands can be sent to the display.

    The font subdirectory contains a variable width 8-pixel tall font
    that can be used to put compact text on to the display and improve
    on the five-letter-wide limit imposed by the fixed-width LED font
    built into the stock AdaFruit driver.

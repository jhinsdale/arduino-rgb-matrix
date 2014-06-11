#!/usr/bin/python

import sys
import ledfont
import fileinput
from text_scroll import TextScroll

# Test the TextScroll class that prints out characters
# on the terminal
#
# Copyright 2014 by John K. Hinsdale <hin@alma.com>
# Use under BSD license with above copyright notice(s) retained.

def main():

    scr = TextScroll(["Now is the time for all good men to come to the aid of their country",
                      "This is another line of text",
                      "Four score and seven years ago, our forefathers brought upon this Earth a new nation, conceived in liberty and dedicated to the proposition that all men are created equal",
                      "The quick brown fox jumped over the lazy dogs",
                      ])
                      
    while True:
        rows = scr.fetch_pixels()
        for r in range(len(rows)-1, -1, -1):
            colbits = rows[r]
            for c in range(len(colbits)-1, -1, -1):
                if ( colbits[c] ):
                    sys.stdout.write("*")
                else:
                    sys.stdout.write(" ")
            sys.stdout.write(" | ")
        sys.stdout.write("\n")

main()

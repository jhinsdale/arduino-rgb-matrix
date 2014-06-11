#!/usr/bin/python

# Render lines of stdin as they would appear on the display in the
# variable width font.
#
# Copyright 2014 by John K. Hinsdale <hin@alma.com>
# Use under BSD license with above copyright notice(s) retained.

import sys
import ledfont
import fileinput
from text_scroll import TextScroll

def main():

    for line in fileinput.input():
        line = line.strip()
        print line
        for r in range(8):
            first_letter = True
            for letter in line:
                if not first_letter:
                    sys.stdout.write(" ")
                else:
                    first_letter = False
                rows = ledfont.FONT_MAP[letter]
                ncols = len(rows[0])
                for c in range(ncols):
                    if rows[r][c]:
                        sys.stdout.write("@")
                    else:
                        sys.stdout.write(" ")
            sys.stdout.write("\n")
        # Make the separator 32-wide so can see if it fit
        sys.stdout.write("================================\n")


main()

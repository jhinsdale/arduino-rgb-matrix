#!/usr/bin/python

# Utility to read standard input and print out word widths
# Useful for filtering a word list down to those words that
# will fit 32-pixels wide
#
# Copyright 2014 by John K. Hinsdale <hin@alma.com>
# Use under BSD license with above copyright notice(s) retained.

import fileinput
import font_util

def main():
    for line in fileinput.input():
        word = line.strip()
        valid = True
        for c in word:
            if ord(c) > 127:
                valid = False
        if not valid:
            continue
        print word + " " + str(font_util.get_word_width(word))

main()

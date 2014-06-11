#
# font_util.py -- Utility functions for font map
#
# Copyright 2014 by John K. Hinsdale <hin@alma.com>
# Use under BSD license with above copyright notice(s) retained.

import ledfont

# Cached info.  Map letters to array of bit columns and width
LETTER_TO_BITCOLS = None
LETTER_TO_WIDTH = None
LETTER_HEIGHT = 8

# Get width of a word, in bit columns, including inter-letter space
def get_word_width(word):
    word_width = 0
    for c in word:
        word_width += get_letter_width(c)
    # Add width of inter-letter spaces
    if len(word) >= 2:
        word_width += len(word) - 1
    return word_width

# Return array of bytes corresponding to the concatenated bit columns
# of a word, with a one-column inter-letter space
def get_word_bitcols(word):
    result = []
    first = True
    for c in word:
        if not first:
            # Inter-letter space column
            result.append(0)
        else:
            first = False
        result.extend(get_letter_bitcols(c))
    return result

# Return array of bytes corresponding to the bit columns of a letter
# LSB of the byte is the first, top row (0) ... MSB is the bottom row (7)
def get_letter_bitcols(char):
    global LETTER_TO_BITCOLS
    _init_maps()
    return LETTER_TO_BITCOLS[char]

# Return width, in columns, of a letter
def get_letter_width(char):
    global LETTER_TO_WIDTH
    _init_maps()
    return LETTER_TO_WIDTH[char]

# Pre-compile maps
def _init_maps():
    '''Compile LED font info into useful maps to get letter width and bit columns'''
    global LETTER_TO_BITCOLS
    global LETTER_TO_WIDTH
    if LETTER_TO_BITCOLS is not None:
        return

    # Compile rows of individual 0/1 bits into columns of 8-bit bytes
    LETTER_TO_BITCOLS = {}
    LETTER_TO_WIDTH = {}
    for letter in ledfont.FONT_MAP:
        rows = ledfont.FONT_MAP[letter]
        ncols = len(rows[0])
        LETTER_TO_WIDTH[letter] = ncols
        bitcols = []
        for c in range(ncols):
            cbits = 0
            nrows = len(rows)
            # Sanity check: make sure all letters are LETTER_HEIGHT rows high
            if nrows != LETTER_HEIGHT:
                raise Exception("Do not have " + str(LETTER_HEIGHT) + " rows for letter '" + letter + "'")
            for r in range(nrows):
                # Sanity check: make sure all rows have the same number of column bits
                if len(rows[r]) != ncols:
                    raise Exception("Inconsistent column bit count for letter '" + letter + "'")
                if rows[r][c]:
                    cbits = cbits | (1 << r)
            bitcols.append(cbits)
        LETTER_TO_BITCOLS[letter] = bitcols

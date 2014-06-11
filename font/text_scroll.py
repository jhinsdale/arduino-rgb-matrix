#
# Scroll multiple streams of text, one pixel-column at a time
# on to the terminal.
#
# Copyright 2014 by John K. Hinsdale <hin@alma.com>
# Use under BSD license with above copyright notice(s) retained.

import ledfont
from collections import deque

# Text scroll object, handles multiple lines of text
class TextScroll:

    # Init with message(s)
    def __init__(self, msgs=[]):
        self._height = 8
        self._nlines = 4
        self._messages = msgs
        self._wrapped_message_space = 16
        self._fontmap = ledfont.FONT_MAP
        while len(self._messages) < self._nlines:
            self._messages.append(None)
        # Set up per-line stuff
        self._cursor = []
        self._pixels = []
        for i in range(len(self._messages)):
            # Pointer to next letter to fetch
            self._cursor.append(0)
            # Queue of pixel columns
            self._pixels.append(deque())

        # Blank pixel column for inter-letter separtor
        self._blank_column = []
        for i in range(self._height):
            self._blank_column.append(0)

    # Fetch pixel column for each line
    def fetch_pixels(self):
        result = []
        for i in range(self._nlines):
            if not self._messages[i]:
                self._pixels[i].append(self._blank_column)
            # If need to get more pixel columns, get and append
            elif not self._pixels[i]:
                (letter, wrapped) = self._fetch_letter(i)
                rows = self._fontmap[letter]
                width = len(rows[0])
                for col in range(width):
                    colarr = []
                    for row in range(self._height):
                        colarr.append(rows[row][col])
                    self._pixels[i].append(colarr)
                # Also append inter-letter blank column
                self._pixels[i].append(self._blank_column)
                if wrapped:
                    for wrapcol in range(self._wrapped_message_space):
                        self._pixels[i].append(self._blank_column)
            result.append(self._pixels[i].popleft())
        return result

    # Set the message for line #i
    def set_message(self, i, msg):
        self._message[i] = msg
        self._cursor[i] = 0
        
    # Fetch the next letter from message #i
    def _fetch_letter(self, i):
        result = self._messages[i][self._cursor[i]]
        self._cursor[i] += 1
        # Wrap around to beginning of message
        wrapped = False
        if self._cursor[i] >= len(self._messages[i]):
            self._cursor[i] = 0
            wrapped = True
        return (result, wrapped)
    

#
# Scroll multiple streams of text, one pixel-column at a time
#
# Copyright 2014 by John K. Hinsdale <hin@alma.com>
# Use under BSD license with above copyright notice(s) retained.

from collections import deque
import ledfont
from font_util import *

# Panel array of scrolling lines
class TextPanel:

    # Init with message, color pairs
    def __init__(self, pairs = []):
        self._lines = []
        self._nlines = len(pairs)
        for i in range(self._nlines):
            self._lines.append(HorizText(pairs[i][0], pairs[i][1]))

    # Get line
    def get_line(self, i):
        return self._lines[i]
    
    # Scroll all lines left
    def scroll(self):
        for i in range(self._nlines):
            self._lines[i].scroll()
            
# Horizontal text scroll object.  Streams horizontally, maintaining
# a FIFO queue of pixel bit columns.  Other state: the message text,
# color, replacment (next) message.
class HorizText:

    # Init with message
    def __init__(self, msg = None, color = None):
        self._height = 8
        self._message = msg
        self._color = color
        self._wrapped_message_space = 5
        self._fontmap = ledfont.FONT_MAP
        self._cursor = 0
        self._bitcols = deque()
        self._curline = deque()

        # Init current line
        for i in range(32):
            self._curline.append(self.get_bitcol())

    # Return current line bit columns
    def get_line_bitcols(self):
        return self._curline
        
    # Get the color
    def get_color(self):
        return self._color
    
    # Scroll current line left
    def scroll(self):
        self._curline.popleft()
        self._curline.append(self.get_bitcol())
    
    # Shift pixel column for this scrolling line and return next one
    def get_bitcol(self):
        if not self._message:
            self._bitcols.append(0)
        # If need to get more pixel columns, get and append
        elif not self._bitcols:
            (letter, wrapped) = self._fetch_letter()
            bitcol_arr = get_letter_bitcols(letter)
            for bc in bitcol_arr:
                self._bitcols.append(bc)
            # Also append inter-letter blank column
            self._bitcols.append(0)
            if wrapped:
                for wrapcol in range(self._wrapped_message_space):
                    self._bitcols.append(0)
        return self._bitcols.popleft()

    # Set the message for line #i
    def set_message(self, msg):
        self._message = msg
        self._cursor = 0
        
    # Fetch the next letter from message #i
    def _fetch_letter(self):
        result = self._message[self._cursor]
        self._cursor += 1
        # Wrap around to beginning of message
        wrapped = False
        if self._cursor >= len(self._message):
            self._cursor = 0
            wrapped = True
        return (result, wrapped)
    

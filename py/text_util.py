#
# textutil.py - Functions for writing variable-width font text
#
# Copyright 2014 by John K. Hinsdale <hin@alma.com>
# Use under BSD license with above copyright notice(s) retained.

from font_util import *
from rgb_matrix import *
from rgb_colors import *

DISPLAY_WIDTH = 32
TEXT_HEIGHT = 8

# Put line of text
def write_line(text, line_no, fg_rgb, bg_rgb=None, center=False):
    if bg_rgb is None:
        bg_rgb = COLOR_BLACK
    # Set X offset at left, or maybe center
    x = 0
    if center:
        space = DISPLAY_WIDTH - get_word_width(text)
        x = int(space/2)
    cmd_SET_TEXT_COLOR(fg_rgb, bg_rgb)
    cmd_SET_TEXT_CURSOR(x, TEXT_HEIGHT * line_no)
    cmd_WRITE_BITCOLS(get_word_bitcols(text))
    cmd_PING()

# Clear line of text
def clear_line(line_no, bg_rgb=None):
    if bg_rgb is None:
        bg_rgb = COLOR_BLACK
    cmd_DRAW_RECT(0, TEXT_HEIGHT * line_no, DISPLAY_WIDTH, TEXT_HEIGHT, bg_rgb, True, 0)


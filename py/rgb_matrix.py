#
# rgb_matrix.py
#
# Copyright 2014 by John K. Hinsdale <hin@alma.com>
# Use under BSD license with above copyright notice(s) retained.

import struct
import serial
import time

# Serial constants
SERIAL_FN = "/dev/ttyUSB0"
SERIAL_SPEED = 115200
SERIAL_PORT_OBJECT = None

# Op codes
OP_CODES = {
    "RESET":              0,
    "BLANK_SCREEN":       1,
    "DEMO":               2,
    "FILL_SCREEN":        3,
    "SET_TEXT_PARAMS":    4,
    "SET_TEXT_COLOR":     5,
    "WRITE_TEXT":         6,
    "DRAW_PIXEL":         7,
    "DRAW_LINE":          8,
    "DRAW_RECT":          9,
    "DRAW_TRIANGLE":     10,
    "DRAW_CIRCLE":       11,
    "DRAW_BITMAP":       12,
    "XOR_RECT":          13,
    "WRITE_BITCOLS":     14,
    "COPY_RECT":         15,
    "WIPE":              16,
    "SLEEP":             30,
    "PING":              31,
    "SET_HORIZ_SCROLL":  50,
    "SET_TIMER_DELAYS":  60,
    "DUMP_MATRIX":      250,
    "DUMP_MATRIX_RECT": 251,
}

# Wipe types
WIPE_DOWN          = 1
WIPE_UP            = 2
WIPE_LEFT          = 3
WIPE_RIGHT         = 4
WIPE_RADIAL_OUT    = 5
WIPE_RADIAL_IN     = 6
WIPE_DISSOLVE      = 7

############ Commands

# RESET
def cmd_RESET():
    send_code_only("RESET")
    return None

# BLANK_SCREEN
def cmd_BLANK_SCREEN():
    send_code_only("BLANK_SCREEN")
    return None
    
# DEMO
def cmd_DEMO():
    send_code_only("DEMO")
    return None

# FILL_SCREEN (color)
def cmd_FILL_SCREEN(rgb):
    ser = get_serial()
    write_byte(ser, OP_CODES["FILL_SCREEN"])
    write_bytes(ser, rgb)
    close_serial(ser)
    return None

# SET_TEXT_CURSOR(x, y)
def cmd_SET_TEXT_CURSOR(x, y):
    ser = get_serial()
    write_byte(ser, OP_CODES["SET_TEXT_PARAMS"])
    # Add 1 for size and true for wrapped
    write_bytes(ser, [x, y, 1, 1])
    close_serial(ser)

# SET_TEXT_COLOR(fg_color, bg_color)
def cmd_SET_TEXT_COLOR(fg_rgb, bg_rgb):
    # Set the color
    ser = get_serial()
    write_byte(ser, OP_CODES["SET_TEXT_COLOR"])
    write_bytes(ser, fg_rgb)
    write_bytes(ser, bg_rgb)
    close_serial(ser)

# WRITE_BITCOLS(bc_array)
def cmd_WRITE_BITCOLS(bc_array):
    if not bc_array:
        return
    ser = get_serial()
    write_byte(ser, OP_CODES["WRITE_BITCOLS"])
    write_byte(ser, len(bc_array))
    for b in bc_array:
        write_byte(ser, b)
    close_serial(ser)

# COPY_RECT(x, y, w, h, new_x, new_y)
def cmd_COPY_RECT(x, y, w, h, new_x, new_y):
    ser = get_serial()
    write_byte(ser, OP_CODES["COPY_RECT"])
    write_bytes(ser, [x, y, w, h, new_x, new_y])
    close_serial(ser)

# WIPE(wipe_type, color, wipe_time_ms)
def cmd_WIPE(wipe_type, rgb, wipe_time):
    ser = get_serial()
    write_byte(ser, OP_CODES["WIPE"])
    write_byte(ser, wipe_type)
    write_bytes(ser, rgb)
    write_int(ser, wipe_time)
    close_serial(ser)

# WRITE_TEXT (color, line)
def cmd_WRITE_TEXT(fg_rgb, bg_rgb, x, y, text):

    # Set the color
    ser = get_serial()
    write_byte(ser, OP_CODES["SET_TEXT_COLOR"])
    write_bytes(ser, fg_rgb)
    write_bytes(ser, bg_rgb)

    # Maybe set the location
    if x is not None and y is not None:
        write_byte(ser, OP_CODES["SET_TEXT_PARAMS"])
        # Add 1 for size and true for wrapped
        write_bytes(ser, [x, y, 1, 1])

    # Write the bytes of text
    write_byte(ser, OP_CODES["WRITE_TEXT"])
    write_byte(ser, len(text))
    for c in text:
        write_byte(ser, ord(c))

    close_serial(ser)
    return None

# DRAW_PIXEL (x, y, color)
def cmd_DRAW_PIXEL(x, y, rgb):
    ser = get_serial()
    write_byte(ser, OP_CODES["DRAW_PIXEL"])
    write_bytes(ser, [x, y])
    write_bytes(ser, rgb)
    close_serial(ser)
    return None

# DRAW_LINE (x0, y0, x1, y1, color)
def cmd_DRAW_LINE(x0, y0, x1, y1, rgb):
    ser = get_serial()
    write_byte(ser, OP_CODES["DRAW_LINE"])
    write_bytes(ser, [x0, y0, x1, y1])
    write_bytes(ser, rgb)
    close_serial(ser)
    return None

# DRAW_RECT (x, y, w, h, color, is_filled, round_radius)
def cmd_DRAW_RECT(x, y, w, h, rgb, is_filled, round_radius):
    ser = get_serial()
    write_byte(ser, OP_CODES["DRAW_RECT"])
    write_bytes(ser, [x, y, w, h])
    write_bytes(ser, rgb)
    # Always give round-radius of zero since rounding does not work
    write_bytes(ser, [is_filled, round_radius])
    close_serial(ser)
    return None

# XOR_RECT (x, y, w, h, color)
def cmd_XOR_RECT(x, y, w, h, rgb):
    ser = get_serial()
    write_byte(ser, OP_CODES["XOR_RECT"])
    write_bytes(ser, [x, y, w, h])
    write_bytes(ser, rgb)
    close_serial(ser)
    return None

# DRAW_TRIANGLE (x0, y0, x1, y1, x2, y2, color, is_filled)
def cmd_DRAW_TRIANGLE(x0, y0, x1, y1, x2, y2, rgb, is_filled):
    ser = get_serial()
    write_byte(ser, OP_CODES["DRAW_TRIANGLE"])
    write_bytes(ser, [x0, y0, x1, y1, x2, y2])
    write_bytes(ser, rgb)
    # Triangle filling does not work (has glitches)
    write_byte(ser, is_filled)
    close_serial(ser)
    return None

# DRAW_CIRCLE (x, y, r, color, is_filled)
def cmd_DRAW_CIRCLE(x, y, radius, rgb, is_filled):
    ser = get_serial()
    write_byte(ser, OP_CODES["DRAW_CIRCLE"])
    write_bytes(ser, [x, y, radius])
    write_bytes(ser, rgb)
    write_byte(ser, is_filled)
    close_serial(ser)
    return None

# SLEEP (ms)
def cmd_SLEEP(ms):
    ser = get_serial()
    write_byte(ser, OP_CODES["SLEEP"])
    write_int(ser, ms)
    close_serial(ser)
    return None

# PING - send a ping request and wait until it returns
def cmd_PING():
    ser = get_serial()
    write_byte(ser, OP_CODES["PING"])
    # Now wait for the response to come back
    byte = read_byte(ser)
    if byte == 0:
        print "*** TIMEOUT waiting for response to ping:"
    elif byte != "!":
        print "*** UNEXPECTED RESPONSE from ping: [" + str(byte) + "]"
    close_serial(ser)
    return None

# SET_HORIZ_SCROLL (offset)
def cmd_SET_HORIZ_SCROLL(offset):
    ser = get_serial()
    write_byte(ser, OP_CODES["SET_HORIZ_SCROLL"])
    write_byte(ser, offset)
    close_serial(ser)
    return None

# SET_TIMER_DELAYS (int_arr)
def cmd_SET_TIMER_DELAYS(int_arr):
    if int_arr is None:
        # Reset to 32x32 defaults
        int_arr = [260, 580, 1220, 2500]
    elif len(int_arr) != 4:
        raise Exception("SET_TIMER_DELAYS takes 4 integer arguments")
    ser = get_serial()
    write_byte(ser, OP_CODES["SET_TIMER_DELAYS"])
    for i in int_arr:
        write_int(ser, i)
    close_serial(ser)
    return None

################# 

# Just send the op code byte for commands with no args
def send_code_only(code):
    ser = get_serial()
    write_byte(ser, OP_CODES[code])
    close_serial(ser)

# Get the matrix of pixels as RGB triples
def get_matrix():
    send_code_only("DUMP_MATRIX")
    ser = get_serial()
    s = read_until(ser, "<\n")
    idx = s.find(">")
    if idx > 0:
        s = s[idx + 1:]
    else:
        return None
    idx = s.find("<")
    if idx > 0:
        s = s[:idx]
    else:
        return None
    s = s.strip()
    # Array of pixel color runs in RRRRrGGGGggBBBBb format
    run_strings = s.split("\n")

    # Parse RGB triples and counts
    runs = []
    for range_str in run_strings:
        if ":" in range_str:
            [pixel_str, count_str] = range_str.split(":")
            count = int(count_str, 16)
            pixel = int(pixel_str, 16)
        else:
            pixel = int(range_str, 16)
            count = 1
        # Split 16-bit pixel integer into 5/6/5  R/G/B
        rgb_triple = ((pixel >> 12) & 0xF, (pixel >> 7) & 0XF, (pixel >> 1) & 0xF)
        runs.append((rgb_triple, count))
    # Assemble the linearized pixels
    pixels = []
    for run in runs:
        (trip, count) = run
        for i in range(count):
            pixels.append(trip)
    # Sanity check: should have gotten 1024 (32x32) pixels
    if len(pixels) != 1024:
        return None
    matrix = [[() for x in xrange(32)] for x in xrange(32)] 
    i = 0
    for row in range(32):
        for col in range(32):
            matrix[row][col] = pixels[i]
            i += 1
    return matrix

# Get an integer, or -1 if invalid
def get_int(s):
    result = -1
    try:
        result = int(s)
    except:
        pass
    return result

# Get the serial port - call close() on it when done!
def get_serial():
    global SERIAL_PORT_OBJECT
    if SERIAL_PORT_OBJECT is None:
        SERIAL_PORT_OBJECT = serial.Serial(SERIAL_FN, SERIAL_SPEED)
    return SERIAL_PORT_OBJECT

# Close the serial when done
def close_serial(ser):
    # We cache, so do nothing
    pass

# Write a byte to serial, retrying and timing out.
# The retry may not be needed since pyserial may do
# the blocking for us(?)
def write_byte(ser, b):
    nretry = 0
    while ( 0 == ser.write(struct.pack('B', b)) ):
        nretry += 1
        if nretry > 500: # 5 second timeout at 10ms interval
            return
        time.sleep(0.010)

# Write byte values to the serial port
def write_bytes(ser, byte_arr):
    # print "Writing " + str(args) + " to " + ser.name + " ..."
    for b in byte_arr:
        write_byte(ser, b)

# Write a two-byte int to serial, msb first
def write_int(ser, i):
    write_byte(ser, int(i/256))
    write_byte(ser, i % 256)

# Read byte from serial, retrying until timeout
# The retry may not be needed since pyserial may do the
# blocking for us(?)
def read_byte(ser):
    nretry = 0
    while True:
        byte = ser.read(1)
        if not byte:
            nretry += 1
            if nretry > 500: # 5 second timeout at 10ms interval
                return 0
            time.sleep(0.010)
        else:
            return byte[0]

# Read bytes from serial until terminating string seen, or timed out
def read_until(ser, termstr):
    result = ""
    while not result.endswith(termstr):
        next_char = read_byte(ser)
        if "\0" == next_char: # timeout
            return result
        result += next_char
    return result


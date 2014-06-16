#
# usb_button.py - Take events from a USB button array
#
# Copyright 2014 by John K. Hinsdale <hin@alma.com>
# Use under BSD license with above copyright notice(s) retained.

import select
from evdev import InputDevice, list_devices, categorize, ecodes, KeyEvent

# USB device needs to have this, uniquely, in its name
BUTTON_DEVICE_DISTINGUISHING_NAME = "Delcom"

# The InputDevice object
BUTTON_DEVICE = None

# Currently pressed button
BUTTON_CURRENTLY_DOWN = None

# Map key code to color
CODE_TO_COLOR = {
        'KEY_Z': 'red',
        'KEY_C': 'green',
        'KEY_COMMA': 'blue',
        'KEY_SLASH': 'yellow',
        }

# Button state codes 0=up 1=down 2=hold
BUTTON_STATES = ["Up", "Down", "Hold"]

# Get a button event, timing out, filtering out lots of stuff and only
# returning a strict sequence of button-down, button-up events for a
# specific button at a time.  Overlapping events for multiple buttons
# are discarded.
def get_button_event(timeout):
    global BUTTON_DEVICE
    global BUTTON_CURRENTLY_DOWN
    
    states = ["Up", "Down", "Hold"]
    dev = BUTTON_DEVICE
    if not BUTTON_DEVICE:
        devices = map(InputDevice, list_devices())
        for d in devices:
            if BUTTON_DEVICE_DISTINGUISHING_NAME in d.name:
                if dev:
                    raise Exception("More than one device name containing '" + BUTTON_DEVICE_DISTINGUISHING_NAME + "'")
                dev = d
        # Grab the device so we are the only one using it
        dev.grab()

    fileno = dev.fileno()
    while True:
        r, w, e = select.select([fileno], [], [], timeout)
        if fileno not in r:
            # Timed out
            break

        # Delcom sends EV_KEY, EV_SYN and EV_MSC
        event = dev.read_one()
        if event.type != ecodes.EV_KEY:
            continue
        
        kev = KeyEvent(event)
        state = kev.keystate # 0=up 1=down 2=hold
        # Ignore hold events
        if state == 2:
            continue
        # Ignore a down-press when another is already down
        if BUTTON_CURRENTLY_DOWN and state == 1:
            continue
        # Ignore an up event if nothing is known to be down
        if not BUTTON_CURRENTLY_DOWN and state == 0:
            continue

        color = CODE_TO_COLOR[kev.keycode]

        # Ignore an up event if it is not the one currently down
        if state == 0 and BUTTON_CURRENTLY_DOWN and color != BUTTON_CURRENTLY_DOWN:
            continue
        # Record button currently down
        if state == 1:
            BUTTON_CURRENTLY_DOWN = color
        # Record button released
        if state == 0:
            BUTTON_CURRENTLY_DOWN = None

        return { 'button': color, 'action': states[state] }

    # Timed out
    return None

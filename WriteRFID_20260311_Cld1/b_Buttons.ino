 #include "b_Buttons.h"



byte pollButtonsOnce() {
  // Read each button (active LOW due to INPUT_PULLUP)
  bool b1 = (digitalRead(button1Pin) == LOW);                  // true when BTN1 is pressed
  bool b2 = (digitalRead(button2Pin) == LOW);                  // true when BTN2 is pressed
  bool b3 = (digitalRead(button3Pin) == LOW);                  // true when BTN3 is pressed
  bool b4 = (digitalRead(button4Pin) == LOW);                  // true when BTN4 is pressed

  static bool prev1 = false, prev2 = false, prev3 = false, prev4 = false; // remember last physical states

  byte evt = 0;                                                // event mask to return (0 means "no new press")

  if (b1 && !prev1) evt |= BUTTON1;                            // rising edge: BTN1 newly pressed
  if (b2 && !prev2) evt |= BUTTON2;                            // rising edge: BTN2 newly pressed
  if (b3 && !prev3) evt |= BUTTON3;                            // rising edge: BTN3 newly pressed
  if (b4 && !prev4) evt |= BUTTON4;                            // rising edge: BTN4 newly pressed

  prev1 = b1;                                                  // store current physical state for next call
  prev2 = b2;                                                  // store current physical state for next call
  prev3 = b3;                                                  // store current physical state for next call
  prev4 = b4;                                                  // store current physical state for next call

  return evt;                                                  // return only *new* presses (debounced by edge detect)
}

BtnDebounced b1;
BtnDebounced b2;
BtnDebounced b3;
BtnDebounced b4;


PressAction pollButtonDebounced(uint8_t pin,
                                BtnDebounced &b,
                                uint16_t debounceMs,
                                uint16_t longMs,
                                uint16_t hold4Ms) {
  // Raw read: pressed = LOW (INPUT_PULLUP)
  bool rawDown = (digitalRead(pin) == LOW);

  // 1) Debounce: require raw state to be stable for debounceMs
  if (rawDown != b.lastRead) {
    b.lastRead = rawDown;
    b.lastChange = millis();
  }

  if ((uint32_t)(millis() - b.lastChange) >= debounceMs) {
    b.stable = b.lastRead; // accept new stable state
  }

  // 2) Edge detect on debounced state
  PressAction evt = ACT_NONE;
  bool debouncedDown = b.stable;

  // Pressed edge
  if (debouncedDown && !b.debouncedPrev) {
    b.tDown = millis();
    b.longFired = false;
    b.hold4Fired = false;
  }

// Held logic (fires once per threshold while still held)
if (debouncedDown) {
  uint32_t held = millis() - b.tDown;

  if (!b.hold4Fired && hold4Ms > 0 && held >= hold4Ms) {
    b.hold4Fired = true;
    evt = ACT_HOLD4;
  } 
  else if (longMs > 0 && !b.longFired && held >= longMs) {
    b.longFired = true;
    evt = ACT_LONG;
  }
}

  // Released edge => SHORT only if no long-tier fired
  if (!debouncedDown && b.debouncedPrev) {
    if (!b.longFired && !b.hold4Fired) evt = ACT_SHORT;
  }

  b.debouncedPrev = debouncedDown;
  return evt;
}


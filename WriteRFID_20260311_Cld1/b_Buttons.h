#pragma once

enum Btn1Action { B1_NONE, B1_SHORT, B1_LONG };
enum Btn3Action { B3_NONE, B3_SHORT, B3_LONG };
enum BtnTrack   { tB1, tB2, tB3, tB4};
Btn1Action pollButton1Action();
Btn3Action pollButton3Action();

enum PressAction { ACT_NONE, ACT_SHORT, ACT_LONG, ACT_HOLD4 /* optional */ };
// ---- Button Debounce State ----


struct BtnDebounced {
  bool stable = false;
  bool lastRead = false;
  uint32_t lastChange = 0;
  bool debouncedPrev = false;
  uint32_t tDown = 0;
  bool longFired = false;
  bool hold4Fired = false;
};

extern BtnDebounced b1;
extern BtnDebounced b2;
extern BtnDebounced b3;
extern BtnDebounced b4;

PressAction pollButtonDebounced(uint8_t pin,
                                BtnDebounced &b,
                                uint16_t debounceMs = 25,
                                uint16_t longMs = 2000,
                                uint16_t hold4Ms = 4000);



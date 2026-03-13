#pragma once
#include <Arduino.h>

// Total cursor stops on the main param screen (circular nav 0→7→0)
const uint8_t CURSOR_COUNT = 8;

// Maps cursor stop index → dataStore[] index.
// Stop 0 is the Round row — 255 flags it as a sub-menu launcher,
// not a direct dataStore param.
static const uint8_t cursorToParam[CURSOR_COUNT] = {
    255,   // 0: Round  — opens sub-menu
      0,   // 1: Time   → dataStore[0]
      2,   // 2: Ends   → dataStore[2]
      4,   // 3: Prac   → dataStore[4]
     13,   // 4: Bann   → dataStore[13]
      1,   // 5: Walk   → dataStore[1]
      3,   // 6: Dets   → dataStore[3]
      6    // 7: BrkT   → dataStore[6]
};

// Total items in the Round sub-menu
const uint8_t SUB_COUNT = 6;

// Maps sub-menu row index → dataStore[] index
static const uint8_t subToParam[SUB_COUNT] = {
     5,    // 0: Fnls → dataStore[5]
     7,    // 1: Altr → dataStore[7]
     8,    // 2: Team → dataStore[8]
     9,    // 3: A/B  → dataStore[9]
    10,    // 4: Flnt → dataStore[10]
    12     // 5: IFAA → dataStore[12]
};

// ── RFID / card helpers ──────────────────────────────────
void dump_byte_array(byte *buffer, byte bufferSize);
bool isCardPresentNow();
void endCardSession();

// ── OLED primitives ──────────────────────────────────────
void clearFromLine(uint8_t lineNo);
void wipeOLED(bool homeScr);

// ── New param screen (replaces showAllParams / showOneParam) ──
void drawParamScreen(uint8_t cursor);   // 255 = no highlight
void drawRoundBanner(bool highlighted);
void drawField(uint8_t cursorIdx, bool highlighted);
void drawBannField(bool highlighted);

// ── Sub-menu ─────────────────────────────────────────────
void drawSubMenu(uint8_t subCursor);
void drawSubField(uint8_t subIdx, bool highlighted);

// ── Idle / instruction helpers ────────────────────────────
void enterParamIdle();
void showInstr(void);
void showPick(void);

// ── Param editing ─────────────────────────────────────────
void alterParam(uint8_t ct);
void showParamVal(uint8_t ct);
void showTempVal(uint8_t ct, uint8_t temp);

// ── Debug ─────────────────────────────────────────────────
void printParamVals(void);

// ── Round label ───────────────────────────────────────────
const char* getRoundLabel();


// #pragma once
// #include <Arduino.h>

// void dump_byte_array(byte *buffer, byte bufferSize);
// bool isCardPresentNow();
// void endCardSession();
// void clearFromLine(uint8_t lineNo);
// void showAllParams(byte index);
// void showOneParam(byte ct, byte index);
// void showAllParamsNoWipe(byte index);
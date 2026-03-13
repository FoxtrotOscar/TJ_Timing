// g_Params.ino  — Writer param display & editing
// Complete rewrite for two-column param screen with Round submenu
// dataStore[] indices UNCHANGED — UI-only refactor

// ─────────────────────────────────────────────
//  Cursor map  (main param screen, 8 stops)
//
//  Stop  Label   dataStore idx   Screen pos
//   0    Round   —  (sub entry)  row 0 full width inverse
//   1    Time    0               row 1 left  (x=0)
//   2    Ends    2               row 2 left  (x=0)
//   3    Prac    4               row 3 left  (x=0)
//   4    Bann    13              row 4 left  (x=0)
//   5    Walk    1               row 1 right (x=8)
//   6    Dets    3               row 2 right (x=8)
//   7    BrkT    6               row 3 right (x=8)
//
//  Navigation: circular  0→1→2→3→4→5→6→7→0 ...
// ─────────────────────────────────────────────

//const uint8_t CURSOR_COUNT = 8;

// static const uint8_t cursorToParam[CURSOR_COUNT] = {
//     255,   // 0: Round — opens sub
//       0,   // 1: Time
//       2,   // 2: Ends
//       4,   // 3: Prac
//      13,   // 4: Bann
//       1,   // 5: Walk
//       3,   // 6: Dets
//       6    // 7: BrkT
// };

//static const uint8_t cursorCol[CURSOR_COUNT] = { 0, 0, 0, 0, 0, 8, 8, 8 };
static const uint8_t cursorCol[CURSOR_COUNT] = { 0, 0, 0, 0, 0, 9, 9, 9 };
static const uint8_t cursorRow[CURSOR_COUNT] = { 0, 1, 2, 3, 4, 1, 2, 3 };

static const char* const cursorLabel[CURSOR_COUNT] = {
    "Rnd:", "Time", "Ends", "Prac", "Bann", "Walk", "Dets", "BrkT"
};

// ─────────────────────────────────────────────
//  Sub-menu map  (6 items, rows 1-6)
// ─────────────────────────────────────────────

//const uint8_t SUB_COUNT = 6;

//static const uint8_t subToParam[SUB_COUNT] = { 5, 7, 8, 9, 10, 12 };
static const char* const subLabel[SUB_COUNT] = {
    "Fnls", "Altr", "Team", "A/B ", "Flnt", "IFAA"
};

// ─────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────

// Derive the round type string from current dataStore state
const char* getRoundLabel() {
    if      (dataStore[10]) return "Flint  ";
    else if (dataStore[12]) return "IFAA   ";
    else if (dataStore[8])  return teamParam[dataStore[8]];
    else                    return "WA Std ";
}

// Print val right-justified in 'width' chars
void printValRight(uint16_t val, uint8_t width) {
    char buf[8];
    snprintf(buf, sizeof(buf), "%*u", width, val);
    u8x8.print(buf);
}

// ─────────────────────────────────────────────
//  Main param screen draw functions
// ─────────────────────────────────────────────

// Row 0: "Rnd: WA Std     " — full 16 chars, inverse when cursor==0
void drawRoundBanner(bool highlighted) {
    if (highlighted) u8x8.inverse();
    u8x8.setCursor(0, 0);
    char buf[17];
    snprintf(buf, sizeof(buf), ">Round:  %-9s", getRoundLabel());
    u8x8.print(buf);
    if (highlighted) u8x8.noInverse();
}

// Draw one 8-char field at its screen position, optionally highlighted
// cursorIdx: 1-3 or 5-7 (normal params — NOT Bann, NOT Round)
//  Uses the same digit-count cursor logic as the original
//  showOneParam() — positions value at x+4/x+5/x+6 depending
//  on whether val is 3/2/1 digits, right-justifying it
//  within each column's space without overflow or stray chars.
// ───────────────────────────────────────────────────────────

void drawField(uint8_t cursorIdx, bool highlighted) {
    uint8_t x    = cursorCol[cursorIdx];   // col start: 0 (left) or 9 (right)
    uint8_t y    = cursorRow[cursorIdx];   // OLED row 1-3
    uint8_t pIdx = cursorToParam[cursorIdx]; // which dataStore[] entry
    uint8_t val  = dataStore[pIdx];        // raw stored value

    if (highlighted) u8x8.inverse();      // invert colours if cursor is here

    u8x8.setCursor(x, y);                 // position at column start
    u8x8.print(cursorLabel[cursorIdx]);   // print 4-char label (e.g. "Time")

    // For Time (cursorIdx==1) show actual seconds, not the index
    uint8_t z = (cursorIdx == 1) ? startCounts[val] : val;

    // Right-justify value within column — mirrors original showOneParam() logic:
    // 3-digit number starts at x+4, 2-digit at x+5, 1-digit at x+6
    u8x8.setCursor( (z < 100 ? (z < 10 ? x+6 : x+5) : x+4), y );
    u8x8.print(z);                        // print the value

    if (highlighted) u8x8.noInverse();    // restore normal colours
}



// Row 4: Bann field — three states
void drawBannField(bool highlighted) {
    if (highlighted) u8x8.inverse();
    u8x8.setCursor(0, 4);

    if (!bannerIsReady()) {
        u8x8.print("Bann:--       ");   // no banner ingested
    } else {
        char buf[17];
        snprintf(buf, sizeof(buf), "Bann:   %d         ", dataStore[13]);
        u8x8.print(buf);
    }

    if (highlighted) u8x8.noInverse();
}

// Draw the complete param screen.
// cursor = 255 → no highlight (idle display)
void drawParamScreen(uint8_t cursor) {
    u8x8.setFont(u8x8_font_chroma48medium8_r);

    drawRoundBanner(cursor == 0);

    // Paired field rows  (stops 1-3 left, 5-7 right)
    for (uint8_t i = 1; i <= 7; i++) {
        if (i == 4) continue;              // Bann handled separately
        drawField(i, cursor == i);
    }

    drawBannField(cursor == 4);

    u8x8.clearLine(5);
    u8x8.clearLine(6);
    // Row 7 is the instruction bar — caller's responsibility
}

// ─────────────────────────────────────────────
//  Sub-menu draw functions
// ─────────────────────────────────────────────

// Draw one sub-menu row at y = subIdx + 1  (rows 1–6)
void drawSubField(uint8_t subIdx, bool highlighted) {
    uint8_t pIdx = subToParam[subIdx];
    uint8_t val  = dataStore[pIdx];

    if (highlighted) u8x8.inverse();
    u8x8.setCursor(0, subIdx + 1);

    // "Labl: val       "  — 16 chars total
    if (pIdx == 8) {                       // Team: text value
        char buf[17];
        snprintf(buf, sizeof(buf), "%-4s: %-10s", subLabel[subIdx], teamParam[val]);
        u8x8.print(buf);
    } else {
        char buf[17];
        snprintf(buf, sizeof(buf), "%-4s: %-10u", subLabel[subIdx], val);
        u8x8.print(buf);
    }

    if (highlighted) u8x8.noInverse();
}

// Draw the complete sub-menu screen
void drawSubMenu(uint8_t subCursor) {
    u8x8.setFont(u8x8_font_chroma48medium8_r);

    // Row 0: header
    u8x8.inverse();
    u8x8.setCursor(0, 0);
    u8x8.print("^[Round Submenu]");
    u8x8.noInverse();

    for (uint8_t i = 0; i < SUB_COUNT; i++) {
        drawSubField(i, subCursor == i);
    }

    // Row 7: instruction bar (caller does showPick())
}

// ─────────────────────────────────────────────
//  Idle-screen entry point (replaces showAllParams)
// ─────────────────────────────────────────────

void enterParamIdle() {
    clearFromLine(1);
    drawParamScreen(255);     // 255 = no highlight
    // showInstr() called by enterIdleScreen()
}

// ─────────────────────────────────────────────
//  Value display helpers used by alterParam()
//  (unchanged from original)
// ─────────────────────────────────────────────

void showParamVal(uint8_t ct) {
    wipeOLED(false);
    delay(300);
    clearFromLine(1);
    u8x8.setCursor(3, 2);
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.draw2x2String(2, 2, nameParam[ct]);
    u8x8.draw2x2String(paramShow, 2, ": ");
    u8x8.setFont(u8x8_font_profont29_2x3_n);
    u8x8.setCursor(6, 4);
    byte z = ct ==  0 ? startCounts[dataStore[ct]] :
             ct == 11 ? dataStore[ct] == 177 ? 1 :
               dataStore[ct] : dataStore[ct];
    ct == 8 ? u8x8.print(teamParam[dataStore[ct]]) :
              u8x8.print(z);
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.setCursor(0, 7);
    u8x8.print("[1]Yes < > No[4]");
    return;
}

void showTempVal(uint8_t ct, uint8_t temp) {
    wipeOLED(false);                                    // clear screen, no header

    u8x8.setFont(u8x8_font_chroma48medium8_r);          // small font for label
    u8x8.draw2x2String(2, 1, nameParam[ct]);            // param name double-size, rows 1-2
    u8x8.draw2x2String(10, 1, ":");                     // colon separator

    u8x8.setFont(u8x8_font_profont29_2x3_r);            // big font for value, rows 4-6

    u8x8.setCursor(4, 4);
    u8x8.print("       ");                              // clear previous value first

    // Position value: Team gets more room (left), others centred
    ct == 8 ? u8x8.setCursor(3, 4) : u8x8.setCursor(6, 4);

    // Print value — Team shows text label, Supervisor masks 177→1, others raw
    ct == 8  ? u8x8.print(teamParam[temp])
             : u8x8.print(ct == 11 ? (temp == 177 ? 1 : temp) : temp);

    u8x8.setFont(u8x8_font_chroma48medium8_r);          // reset to small font — critical,
                                                        // prevents font bleed into next draw
    u8x8.setCursor(0, 7);
    u8x8.print(" [2]up [3]dn [1]ok");                  // instruction bar
}

void alterParam(uint8_t ct) {
    uint8_t tempVal;

    // Prime debouncers so the edit screen starts clean
    for (int i = 0; i < 50; i++) {
        pollButtonDebounced(button1Pin, b1, 25, 0, 0);
        pollButtonDebounced(button2Pin, b2, 25, 0, 0);
        pollButtonDebounced(button3Pin, b3, 25, 0, 0);
        pollButtonDebounced(button4Pin, b4, 25, 0, 0);
        delay(1);
    }

    switch (ct) {

        case 0: { // TIME: index 0..8 into startCounts[]
            tempVal = dataStore[ct];
            //showTempVal(startCounts[ct], startCounts[tempVal]);
            showTempVal(ct, startCounts[tempVal]);   // pass ct=0 for label
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT) { tempVal = (tempVal >= 8) ? 0 : tempVal + 1; showTempVal(startCounts[ct], startCounts[tempVal]); }
                if (p3 == ACT_SHORT) { tempVal = (tempVal == 0) ? 8 : tempVal - 1; showTempVal(startCounts[ct], startCounts[tempVal]); }
                if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        case 1: { // WALKUP: 10 / 20
            tempVal = dataStore[ct];
            showTempVal(ct, tempVal);
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT || p3 == ACT_SHORT) { tempVal = (tempVal == 10) ? 20 : 10; showTempVal(ct, tempVal); }
                if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        case 2: { // ENDS: 1..24
            tempVal = dataStore[ct];
            showTempVal(ct, tempVal);
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT) { tempVal = (tempVal >= 24) ? 1  : tempVal + 1; showTempVal(ct, tempVal); }
                if (p3 == ACT_SHORT) { tempVal = (tempVal == 1)  ? 24 : tempVal - 1; showTempVal(ct, tempVal); }
                if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        case 3: { // DETAILS: 1 / 2
            tempVal = dataStore[ct];
            showTempVal(ct, tempVal);
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT || p3 == ACT_SHORT) { tempVal = (tempVal == 2) ? 1 : 2; showTempVal(ct, tempVal); }
                if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        case 4: { // PRACTICE: 0..3
            tempVal = dataStore[ct];
            showTempVal(ct, tempVal);
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT) { tempVal = (tempVal >= 3) ? 0 : tempVal + 1; showTempVal(ct, tempVal); }
                if (p3 == ACT_SHORT) { tempVal = (tempVal == 0) ? 3 : tempVal - 1; showTempVal(ct, tempVal); }
                if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        case 5: { // FINALS: 0/1
            tempVal = dataStore[ct];
            showTempVal(ct, tempVal);
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT || p3 == ACT_SHORT) { tempVal = tempVal ? 0 : 1; showTempVal(ct, tempVal); }
                if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        case 6: { // BREAKTIMER: steps of 10, range 10..240
            tempVal = dataStore[ct];
            showTempVal(ct, tempVal);
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT) {
                    tempVal = (tempVal > 231) ? 10 : tempVal + 1;
                    do { tempVal++; } while (tempVal % 10);
                    showTempVal(ct, tempVal);
                }
                if (p3 == ACT_SHORT) {
                    tempVal = (tempVal <= 10) ? 240 : tempVal - 10;
                    showTempVal(ct, tempVal);
                }
                if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        case 7: { // ALTERNATING: 0/1
            tempVal = dataStore[ct];
            showTempVal(ct, tempVal);
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT || p3 == ACT_SHORT) { tempVal = tempVal ? 0 : 1; showTempVal(ct, tempVal); }
                if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        case 8: { // TEAMPLAY: scroll teamParam[], skipping blanks
            tempVal = dataStore[ct];
            showTempVal(ct, tempVal);
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT) {
                    tempVal = (tempVal == 4) ? 11 : (tempVal == 14 ? 0 : tempVal + 1);
                    showTempVal(ct, tempVal);
                }
                if (p3 == ACT_SHORT) {
                    tempVal = (tempVal == 0) ? 14 : (tempVal == 11 ? 4 : tempVal - 1);
                    showTempVal(ct, tempVal);
                }
                if (p1 == ACT_SHORT) {
                    dataStore[ct] = tempVal;
                    if (tempVal) {                 // Team set
                        dataStore[3]  = 1;         // Dets
                        dataStore[4]  = 0;         // Prac
                        dataStore[6]  = 15;        // BrkT
                        dataStore[10] = 0;         // Flint off
                        dataStore[12] = 0;         // IFAA off
                    }
                    // Side-effects: pre-fill Time and Ends for team modes
                    switch (dataStore[ct]) {
                        case 1: case 3: case 11: case 13: dataStore[0] = 1; dataStore[2] = 5; break;
                        case 2: case 4: case 12: case 14: dataStore[0] = 2; dataStore[2] = 5; break;
                        default: break;
                    }
                    done = true;
                }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        case 9:  // A/B?  — fall through
        case 11: // SUPERVISOR (0 or 177)
        { 
            tempVal = dataStore[ct];
            showTempVal(ct, tempVal);
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT || p3 == ACT_SHORT) {
                    if (ct == 11) tempVal = (tempVal == 0) ? 177 : 0;
                    else          tempVal = tempVal ? 0 : 1;
                    showTempVal(ct, tempVal);
                }
                if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        case 10: { // FLINT: 0/1 with side-effects + IFAA exclusivity
            tempVal = dataStore[ct];
            showTempVal(ct, tempVal);
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT || p3 == ACT_SHORT) { tempVal = tempVal ? 0 : 1; showTempVal(ct, tempVal); }
                if (p1 == ACT_SHORT) {
                    dataStore[ct] = tempVal;
                    if (tempVal) {                 // Flint ON
                        dataStore[0]  = 7;         // Time 180
                        dataStore[2]  = 7;         // Ends
                        dataStore[3]  = 1;         // Dets
                        dataStore[4]  = 1;         // Prac
                        dataStore[5]  = 0;         // Finals off  
                        dataStore[6]  = 15;        // BrkT
                        dataStore[7]  = 0;         // Alternating off 
                        dataStore[8]  = 0;         // Team off
                        dataStore[9]  = 0;         // A/B off
                        dataStore[12] = 0;         // IFAA off
                    }
                    done = true;
                }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        case 12: { // IFAA INDOOR: 0/1 with side-effects + Flint exclusivity
            tempVal = dataStore[ct];
            showTempVal(ct, tempVal);
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT || p3 == ACT_SHORT) { tempVal = tempVal ? 0 : 1; showTempVal(ct, tempVal); }
                if (p1 == ACT_SHORT) {
                    dataStore[ct] = tempVal;
                    if (tempVal) {                 // IFAA ON

                        dataStore[0]  = 0;         // Time 240
                        dataStore[2]  = 12;        // Ends
                        dataStore[3]  = 2;         // Dets
                        dataStore[4]  = 1;         // Prac
                        dataStore[5]  = 0;         // Finals off  
                        dataStore[6]  = 15;        // BrkT
                        dataStore[7]  = 0;         // Alternating off 
                        dataStore[8]  = 0;         // Team off
                        dataStore[9]  = 0;         // A/B off
                        dataStore[10] = 0;         // Flint off
                    }
                    done = true;
                }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        case 13: { // BANN: 0/1 — guarded by bannerIsReady()
            if (!bannerIsReady()) {
                // No banner ingested — flash warning and return
                u8x8.setFont(u8x8_font_chroma48medium8_r);
                u8x8.setCursor(0, 5);
                u8x8.print("No banner yet   ");
                pauseMe(1200);
                u8x8.clearLine(5);
                return;
            }
            tempVal = dataStore[ct];
            showTempVal(ct, tempVal);
            for (bool done = false; !done; ) {
                PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
                PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
                PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
                PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);
                if (p2 == ACT_SHORT || p3 == ACT_SHORT) { tempVal = tempVal ? 0 : 1; showTempVal(ct, tempVal); }
                if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
                if (p4 == ACT_SHORT) { done = true; }
                delay(1);
            }
            break;
        }

        default:
            break;
    }
}

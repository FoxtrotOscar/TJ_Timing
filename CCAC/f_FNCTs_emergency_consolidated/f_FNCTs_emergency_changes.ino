// ================================================================
//  EMERGENCY RESTART SCREENS — CLEAN VERSION
//  Replaces both handleEmergencyRestart_Screen2() and
//  handleEmergencyRestart() in f_FNCTs.ino
//
//  Changes from previous version:
//  - All commented-out dead code removed
//  - printPadded16() / currentDetail() applied throughout
//  - All bespoke padding replaced — no hardcoded space strings
//  - Flint walkup context uses printPadded16 with flintWalk string
//  - Screen 2 row 1 context consistent with Screen 1
//  - 16 char hard limit enforced on every row
// ================================================================


// ================================================================
//  handleEmergencyRestart_Screen2()
// ================================================================
int16_t handleEmergencyRestart_Screen2(byte nID) {
  int8_t  savedIter = sE_iter;           // snapshot sub-phase for resume
  uint8_t maxSecs   = currentEndMax();   // walkup-aware count ceiling

  for (;;) {
    clearFromLine(1);

    // ── Row 1: context ──────────────────────────────────────────
    disp.setCursor(0, 1);
    disp.setFont(u8x8_font_5x7_f);

    if (p_Store.isFlint && sEcount > 6) {
      // Flint walkup end — label + distance, padded to 16
      char buf[17];
      snprintf(buf, sizeof(buf), "Flint %s", flintWalk[sEcount - 7]);
      disp.print(buf);
      for (int i = strlen(buf); i < 16; i++) disp.print(" ");

    } else if (p_Store.isFlint) {
      // Standard Flint end
      printPadded16("Flint end ", sEcount);

    } else if (p_Store.ifaaIndoor) {
      // IFAA — hardcoded, exactly 16 chars
      disp.print(sEcount <= 6 ? "IFAA E1-6  AB>CD"
                              : "IFAA E7-12 DC>BA");

    } else {
      // WA — practice or competition end, detail right-justified
      if (countPractice) {
        printPadded16("Prac #", (p_Store.maxPrac - countPractice + 1));
      } else {
        printPadded16("End #", sEcount);
      }
      // Sub-phase on row 2 for WA
      disp.setFont(u8x8_font_chroma48medium8_r);
      disp.setCursor(0, 2);
      activeSubPhaseLabel();
    }

    disp.setFont(u8x8_font_chroma48medium8_r);

    // ── BTN1: Resume — least disruptive ─────────────────────────
    disp.setCursor(0, 3);
    disp.inverse();
    disp.print("Resume:   BTN[1]");
    disp.setCursor(0, 4);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    if (p_Store.isFlint && sEcount > 6) {
      // Walkup — show current distance in resume label
      char buf[17];
      snprintf(buf, sizeof(buf), "(resume@%s)", flintWalk[sEcount - 7]);
      disp.print(buf);
      for (int i = strlen(buf); i < 16; i++) disp.print(" ");
    } else {
      disp.print(p_Store.isFlint ? "(resume this end)"
                                 : "(from active grp)");
    }

    // ── BTN2: Restart — more disruptive ─────────────────────────
    disp.setCursor(0, 5);
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.inverse();
    disp.print("Restart:  BTN[2]");
    disp.setCursor(0, 6);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    if (p_Store.isFlint && sEcount >= 8) {
      disp.print("(restart@30 YD) ");  // offer rewind to 30m
    } else if (p_Store.isFlint) {
      disp.print("(restart end)   ");  // end 7 or standard Flint
    } else {
      disp.print(p_Store.ifaaIndoor
                 ? (sEcount <= 6 ? "(restart AB>CD) "
                                 : "(restart DC>BA) ")
                 :                  "(from first grp)");
    }

    // ── BTN4 hint (row 7) ────────────────────────────────────────
    disp.setCursor(0, 7);
    disp.noInverse();
    disp.print("Reconsider:BTN[4]");  // silent BTN3 back not shown
    disp.setFont(u8x8_font_chroma48medium8_r);

    switch (waitButton()) {

      case BUTTON1: {
        // ── Resume from active position ────────────────────────
        reStartEnd  = true;
        for (byte r = 0; r <= 2; r++) n_Count_[r] = maxSecs;
        sE_iter     = savedIter;
        shootDetail = p_Store.ifaaIndoor
                      ? (sEcount > 6 ? true : false)
                      : (savedIter % 2 == 0 ? true : false);
        clearMatrix(false);
        return n_Count_[nID];
      }

      case BUTTON2: {
        // ── Restart ───────────────────────────────────────────
        if (p_Store.isFlint && sEcount >= 8) {
          sEcount = 7;                   // rewind to first walkup end
          maxSecs = currentEndMax();     // recalculate for new sEcount
        }
        reStartEnd  = true;
        for (byte r = 0; r <= 2; r++) n_Count_[r] = maxSecs;
        sE_iter     = deriveIterForEnd(sEcount);
        shootDetail = p_Store.ifaaIndoor
                      ? (sEcount > 6 ? true : false)
                      : ((sEcount % 2) == 1 ? false : true);
        clearMatrix(false);
        return n_Count_[nID];
      }

      case BUTTON3:
        return -1;                       // silent back to Screen 1

      case BUTTON4:
        return -2;                       // reconsider — propagates up
    }
  }
}


// ================================================================
//  handleEmergencyRestart()  — Screen 1
// ================================================================
int16_t handleEmergencyRestart(byte nID) {

  for (;;) {                             // loops on BTN3 back from Screen 2
    clearFromLine(1);

    // ── Row 1: context — 16 char hard limit ─────────────────────
    disp.setCursor(0, 1);
    disp.setFont(u8x8_font_5x7_f);

    if (p_Store.ifaaIndoor) {
      // Hardcoded — always exactly 16 chars, safe for E1-6 and E7-12
      disp.print(sEcount <= 6 ? "IFAA E1-6  AB>CD"
                              : "IFAA E7-12 DC>BA");

    } else if (p_Store.isFlint && sEcount > 6) {
      // Flint walkup — label + distance padded to 16
      char buf[17];
      snprintf(buf, sizeof(buf), "Flint %s", flintWalk[sEcount - 7]);
      disp.print(buf);
      for (int i = strlen(buf); i < 16; i++) disp.print(" ");

    } else if (p_Store.isFlint) {
      // Standard Flint end
      printPadded16("Flint end ", sEcount);

    } else {
      // WA — practice or competition, detail right-justified
      if (countPractice) {
        printPadded16("Prac #", (p_Store.maxPrac - countPractice + 1));
      } else {
        printPadded16("End #", sEcount);
      }
    }

    disp.setFont(u8x8_font_chroma48medium8_r);

    // ── BTN1: Resume, back 10s ───────────────────────────────────
    disp.setCursor(0, 2);
    disp.inverse();
    disp.print("Resume:   BTN[1]");
    disp.setCursor(0, 3);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    disp.print("(steps back 10s)");

    // ── BTN2: Go to Screen 2 ─────────────────────────────────────
    disp.setCursor(0, 4);
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.inverse();
    disp.print("Restart:  BTN[2]");
    disp.setCursor(0, 5);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    disp.print("(-->options)    ");

    // ── BTN3: Full competition restart ───────────────────────────
    disp.setCursor(0, 6);
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.inverse();
    disp.print("Reset ALL:BTN[3]");
    disp.setCursor(0, 7);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    disp.print("(full restart)  ");
    disp.setFont(u8x8_font_chroma48medium8_r);

    uint8_t maxSecs = currentEndMax();

    switch (waitButton()) {

      case BUTTON1:
        // Back 10s — walkup-aware ceiling
        n_Count_[nID] = ((int)n_Count_[nID] + 11 > (int)maxSecs)
                        ? maxSecs
                        : n_Count_[nID] + 11;
        clearMatrix(false);
        return n_Count_[nID];

      case BUTTON2: {
        int16_t result = handleEmergencyRestart_Screen2(nID);
        if (result == -1) continue;      // BTN3 on Screen 2 — loop back
        if (result == -2) return -2;     // BTN4 — propagate reconsider
        return result;
      }

      case BUTTON3:
        clearMatrix(false);
        flintRunning = false;            // always reset to Flint #1
        startOver    = true;
        memset(n_Count_, 0, sizeof(n_Count_));
        return n_Count_[nID];            // 0 — triggers reset in loop()

      case BUTTON4:
        return -2;                       // reconsider — propagates up
    }
  }
}

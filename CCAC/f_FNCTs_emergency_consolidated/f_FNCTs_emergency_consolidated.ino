// ================================================================
//  EMERGENCY HANDLING — FULL CONSOLIDATED FIXES
//  Apply to f_FNCTs.ino and r_RFID.ino / f_FNCTs.ino (zeroSettings)
//
//  Summary of all changes:
//
//  f_FNCTs.ino:
//  1. REPLACE  zeroSettings()                    — adds flintRunning=false
//  2. REPLACE  currentEndMax()                   — already added, reproduced clean
//  3. REPLACE  activeSubPhaseLabel()             — Flint guard, confirmed
//  4. REPLACE  handleEmergencyRestart_Screen2()  — full rewrite:
//                BTN1/BTN2 swapped (resume/restart)
//                Flint end 1-6 / end 7 / end 8+ distinct behaviour
//                Screen 2 BTN3 silent back, BTN4 reconsider
//                uses currentEndMax() throughout
//  5. REPLACE  handleEmergencyRestart()          — OLED anti-wrap,
//                currentEndMax() for BTN1 ceiling
//                BTN3 resets flintRunning=false explicitly
//  6. REPLACE  goEmergencyButton()               — OLED refresh on reconsider,
//                currentEndMax() for doBarCount trigger
//
//  Each section marked with apply location and what changed.
// ================================================================


// ================================================================
//  1. REPLACE zeroSettings()
//
//  Find existing zeroSettings() and replace entirely.
//  CHANGE: Added flintRunning = false — card tap or any fresh
//  start must always reset to Flint #1 state.
// ================================================================
void zeroSettings(void) {
  shootDetail  = 0;                // reset detail flag to AB
  sEcount      = 1;                // reset end counter to first end
  continueOn   = false;            // halt the run loop
  sE_iter      = 0;                // reset sub-phase iterator
  countPractice = p_Store.maxPrac; // restore practice count from params
  flintRunning = false;            // always reset to Flint #1 on any fresh start
                                   // natural #1->#2 flip belongs in loop() only
}


// ================================================================
//  2. REPLACE currentEndMax()
//
//  Find existing currentEndMax() and replace.
//  No logic change — reproduced clean for reference.
//  Returns correct maximum seconds for the CURRENT end,
//  walkup-aware.
// ================================================================
uint8_t currentEndMax(void) {
  if (p_Store.isFlint && sEcount > 6) {              // in a Flint walkup end
    return startCounts[p_Store.startCountsIndex + 1]; // walkup duration (30s)
  }
  return startCounts[p_Store.startCountsIndex];       // standard end duration
}


// ================================================================
//  3. REPLACE activeSubPhaseLabel()
//
//  Find existing activeSubPhaseLabel() and replace.
//  CHANGE: Flint guard — Flint has no AB/CD sub-phase concept.
// ================================================================
void activeSubPhaseLabel(void) {

  if (p_Store.isFlint) {                // Flint is single detail — no AB/CD
    disp.print("Single  detail  ");     // neutral, fills row cleanly
    return;
  }

  const char* first;
  const char* second;

  if (p_Store.ifaaIndoor && sEcount > 6) {
    first  = "DC";                      // IFAA second half: DC shoots first
    second = "BA";
  } else {
    first  = "AB";                      // WA and IFAA first half
    second = "CD";
  }

  bool firstActive = (sE_iter % 2 == 1); // odd iter = first group active

  if (firstActive) {
    disp.inverse();   disp.print(first);
    disp.noInverse(); disp.print("  ");
                      disp.print(second);
  } else {
                      disp.print(first);
                      disp.print("  ");
    disp.inverse();   disp.print(second);
    disp.noInverse();
  }
  disp.noInverse();
}


// ================================================================
//  4. REPLACE handleEmergencyRestart_Screen2()
//
//  Full rewrite. Find existing function and replace entirely.
//
//  CHANGES:
//  - BTN1 = Resume (least disruptive) — was BTN2
//  - BTN2 = Restart (more disruptive) — was BTN1
//  - BTN3 = silent back to Screen 1 (unlabelled)
//  - BTN4 = reconsider / oops
//  - Flint ends 1-6: no distance language, no 30m option
//  - Flint end 7: restart = restart this end (already at 30m)
//  - Flint ends 8-10: BTN2 restart offers two sub-options:
//      restart current distance OR restart from 30m (sEcount=7)
//  - All count resets use currentEndMax()
//  - Includes 10s walkup bar on all Flint walkup restarts
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
      // Walkup end — show distance
      disp.print("End ");
      disp.print(sEcount);
      disp.print(" ");
      disp.print(flintWalk[sEcount - 7]);  // e.g. "30 YD"
    } else if (p_Store.isFlint) {
      // Standard Flint end
      disp.print("Flint end ");
      disp.print(sEcount);
      disp.print("      ");
    } else {
      // WA / IFAA
      disp.setCursor(0, 2);
      activeSubPhaseLabel();             // sub-phase indicator (IFAA/WA only)
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
      disp.print("(resume @");
      disp.print(flintWalk[sEcount - 7]);  // current distance
      disp.print(")  ");
    } else {
      disp.print(p_Store.isFlint ? "(resume this end)" :
                                   "(from active grp)");
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
      // Ends 8-10: offer restart from 30m
      disp.print("(restart @30 YD)");
    } else if (p_Store.isFlint) {
      // End 7 or standard Flint ends
      disp.print("(restart end)   ");
    } else {
      disp.print(p_Store.ifaaIndoor ?
                (sEcount <= 6 ? "(restart AB>CD )" :
                                "(restart DC>BA )") :
                                "(from first grp)");
    }

    // BTN4 hint
    disp.setCursor(0, 7);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    disp.print("Reconsider:BTN[4]");
    disp.setFont(u8x8_font_chroma48medium8_r);

    switch (waitButton()) {

      case BUTTON1: {
        // ── Resume from active position ────────────────────────
        reStartEnd = true;
        for (byte r = 0; r <= 2; r++) n_Count_[r] = maxSecs;
        sE_iter     = savedIter;           // restore sub-phase snapshot
        shootDetail = p_Store.ifaaIndoor
                      ? (sEcount > 6 ? true : false)
                      : ((sEcount % 2) == 1 ? false : true);
        clearMatrix(false);
        return n_Count_[nID];              // >= 0, proceed
      }

      case BUTTON2: {
        // ── Restart ───────────────────────────────────────────
        if (p_Store.isFlint && sEcount >= 8) {
          // Flint ends 8-10: restart from 30m — rewind to end 7
          sEcount = 7;                     // rewind to first walkup end
          maxSecs = currentEndMax();       // recalculate for new sEcount
        }
        reStartEnd = true;
        for (byte r = 0; r <= 2; r++) n_Count_[r] = maxSecs;
        sE_iter     = deriveIterForEnd(sEcount); // first sub-phase of this end
        shootDetail = p_Store.ifaaIndoor
                      ? (sEcount > 6 ? true : false)
                      : ((sEcount % 2) == 1 ? false : true);
        clearMatrix(false);
        return n_Count_[nID];              // >= 0, proceed
      }

      case BUTTON3:
        return -1;                         // silent back to Screen 1

      case BUTTON4:
        return -2;                         // reconsider — propagates up
    }
  }
}


// ================================================================
//  5. REPLACE handleEmergencyRestart()  (Screen 1)
//
//  Find existing handleEmergencyRestart() and replace entirely.
//
//  CHANGES:
//  - OLED context line anti-wrap — 16 char hard limit, no spaces
//    in labels, right-justified detail group
//  - BTN1 ceiling uses currentEndMax()
//  - BTN3 explicitly sets flintRunning=false before startOver
//  - IFAA rows hardcoded E1-6 / E7-12 (safe — always 12 ends)
// ================================================================
int16_t handleEmergencyRestart(byte nID) {

  for (;;) {                             // loops on BTN3 back from Screen 2
    clearFromLine(1);

    // ── Row 1: context — 16 char hard limit ─────────────────────
    // Format: left-justified context, right-justified detail group
    // No spaces inside labels, no wrapping
    disp.setCursor(0, 1);
    disp.setFont(u8x8_font_5x7_f);

    if (p_Store.ifaaIndoor) {
      // "IFAA E1-6  AB>CD" = 16 chars exactly
      // "IFAA E7-12 DC>BA" = 16 chars exactly
      if (sEcount <= 6) {
        disp.print("IFAA E1-6  AB>CD");
      } else {
        disp.print("IFAA E7-12 DC>BA");
      }
    } else if (p_Store.isFlint && sEcount > 6) {
      // "Flint -->30 YD  " walkup with distance
      disp.print("Flint ");
      disp.print(flintWalk[sEcount - 7]); // e.g. "30 YD"
      disp.print("       ");              // pad to 16
    } else if (p_Store.isFlint) {
      // "Flint end 3     " = standard Flint end
      disp.print("Flint end ");
      disp.print(sEcount);
      disp.print("      ");              // pad to 16
    } else {
      // "End 3           " = WA
      disp.print("End ");
      disp.print(sEcount);
      disp.print("            ");        // pad to 16
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

    // ── BTN2: Restart options (Screen 2) ────────────────────────
    disp.setCursor(0, 4);
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.inverse();
    disp.print("Restart:  BTN[2]");
    disp.setCursor(0, 5);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    disp.print("(restart options)");

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

    uint8_t maxSecs = currentEndMax();   // walkup-aware ceiling

    switch (waitButton()) {

      case BUTTON1:
        // Back 10s — ceiling is currentEndMax(), not raw startCounts
        // +11 because count already decremented once this second
        n_Count_[nID] = ((int)n_Count_[nID] + 11 > (int)maxSecs)
                        ? maxSecs
                        : n_Count_[nID] + 11;
        clearMatrix(false);
        return n_Count_[nID];

      case BUTTON2: {
        int16_t result = handleEmergencyRestart_Screen2(nID);
        if (result == -1) continue;      // BTN3 on Screen 2 — back here
        if (result == -2) return -2;     // BTN4 on Screen 2 — propagate
        return result;
      }

      case BUTTON3:
        // Full competition restart — wipe everything
        clearMatrix(false);
        flintRunning = false;            // always reset to Flint #1
        startOver    = true;
        memset(n_Count_, 0, sizeof(n_Count_));
        return n_Count_[nID];            // 0 — triggers done/reset in loop()

      case BUTTON4:
        return -2;                       // reconsider — propagates up
    }
  }
}


// ================================================================
//  6. REPLACE goEmergencyButton()
//
//  Find existing goEmergencyButton() and replace entirely.
//
//  CHANGES:
//  - BTN4 reconsider: displayParamsOnOLED() called after message
//    so OLED shows correct state on return to count
//  - doBarCount trigger uses currentEndMax() — walkup-aware
//  - emergencyReconsider behaviour unchanged
// ================================================================
bool goEmergencyButton(uint8_t AIndex, byte nID) {
  bool ret = false;

  if (readButtons() == BUTTON4) {
    ret = true;
    emergencyReconsider = false;         // clear flag at entry

    goWhistle(5);
    stopSign();
    clearFromLine(1);
    disp.setCursor(4, 3);
    disp.print("EMERGENCY");
    disp.setCursor(6, 5);
    disp.print("STOP");
    pauseMe(5 * tick);

    // ── Snapshot globals at moment of emergency ──────────────────
    int8_t snapIter   = sE_iter;
    int8_t snapEcount = sEcount;
    bool   snapDetail = shootDetail;
    // p_Store.isFlint / p_Store.ifaaIndoor untouched by all
    // emergency paths — they live in p_Store (EEPROM-backed)

    for (;;) {
      int16_t result = handleEmergencyRestart(nID);

      if (result == -2) {
        // ── Reconsider — restore globals, exit cleanly ───────────
        sE_iter     = snapIter;
        sEcount     = snapEcount;
        shootDetail = snapDetail;
        emergencyReconsider = true;      // signal caller to restore locals
        clearFromLine(1);
        disp.setCursor(0, 3);
        disp.print("  Reconsidering ");
        disp.setCursor(0, 5);
        disp.print("Resuming count..");
        pauseMe(2 * tick);
        displayParamsOnOLED();           // refresh OLED to correct state
        // Exit — do NOT re-enter menu. Return to the running count.
        break;
      }

      // ── Decision made ────────────────────────────────────────
      n_Count_[nID] = result;
      emergencyReconsider = false;       // clean exit
      break;
    }

    // ── Post-emergency display setup ─────────────────────────────
    // Only runs when a real decision was made (not reconsider).
    if (!emergencyReconsider) {
      if (AIndex < 3) {
        displayParamsOnOLED();
        // Use currentEndMax() — walkup ends return walkup duration
        if (n_Count_[nID] == currentEndMax()) {
          doBarCount(AIndex, nID);       // full restart — run walkup bar
          writeOLED_Data(1, nID);
        } else if (n_Count_[nID] != 0) {
          goWhistle(1);                  // partial resume — one whistle
          writeOLED_Data(1, nID);
        }
      } else if (AIndex == 3) {
        displayParamsOnOLED();
        writeOLED_Data(1, nID);
      } else if (AIndex == 4) {
        // reserved
      }
    }
  }
  return ret;
}

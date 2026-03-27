// ================================================================
//  EMERGENCY HANDLING — CHANGES ONLY
//  Apply to f_FNCTs.ino
//
//  Changes in this file:
//  1. NEW:      currentEndMax()               — walkup-aware ceiling
//  2. REPLACE:  activeSubPhaseLabel()         — Flint guard added
//  3. REPLACE:  handleEmergencyRestart_Screen2() — Flint-aware display,
//                                               uses currentEndMax()
//  4. REPLACE:  handleEmergencyRestart()      — uses currentEndMax()
//  5. REPLACE:  goEmergencyButton()           — BTN4 exits cleanly
//
//  Each section is marked with its apply location.
// ================================================================


// ================================================================
//  1. NEW FUNCTION — add before activeSubPhaseLabel()
//
//  currentEndMax()
//  Returns the correct maximum seconds for the CURRENT end.
//  Walkup ends (Flint, sEcount > 6) use startCountsIndex+1.
//  Everything else uses startCountsIndex.
//  Used for:
//    - back-10 ceiling in BTN1
//    - count reset in BTN1/BTN2 of Screen 2
// ================================================================
uint8_t currentEndMax(void) {
  if (p_Store.isFlint && sEcount > 6) {              // we are in a Flint walkup end
    return startCounts[p_Store.startCountsIndex + 1]; // walkup duration (e.g. 30s)
  }
  return startCounts[p_Store.startCountsIndex];       // standard end duration
}


// ================================================================
//  2. REPLACE activeSubPhaseLabel()
//
//  Find the existing activeSubPhaseLabel() and replace entirely.
//  CHANGE: Added Flint guard at top — Flint has no AB/CD concept.
// ================================================================
void activeSubPhaseLabel(void) {

  // Flint is single detail — no sub-phase split exists
  if (p_Store.isFlint) {
    disp.print("Single  detail  ");   // neutral, fills the row cleanly
    return;
  }

  const char* first;
  const char* second;

  if (p_Store.ifaaIndoor && sEcount > 6) {
    first  = "DC";                    // IFAA second half: DC shoots first
    second = "BA";
  } else {
    first  = "AB";                    // WA and IFAA first half: AB shoots first
    second = "CD";
  }

  // sE_iter odd == first sub-phase active, even == second sub-phase active
  bool firstActive = (sE_iter % 2 == 1);

  if (firstActive) {
    disp.inverse();   disp.print(first);
    disp.noInverse(); disp.print("  ");
                      disp.print(second);
  } else {
                      disp.print(first);
                      disp.print("  ");
    disp.inverse();   disp.print(second);
  }
  disp.noInverse();
}


// ================================================================
//  3. REPLACE handleEmergencyRestart_Screen2()
//
//  Find the existing handleEmergencyRestart_Screen2() and replace.
//  CHANGES:
//    - Uses currentEndMax() instead of raw startCounts[] lookup
//    - Flint: suppresses sub-phase label row, shows "Single detail"
//    - BTN1/BTN2 text adjusted for Flint (no group language)
// ================================================================
int16_t handleEmergencyRestart_Screen2(byte nID) {
  int8_t  savedIter   = sE_iter;      // snapshot active sub-phase for BTN2 resume
  uint8_t maxSecs     = currentEndMax(); // walkup-aware ceiling — the KEY fix

  for (;;) {
    clearFromLine(1);

    // ── Row 1: end number ──────────────────────────────────────
    disp.setCursor(0, 1);
    disp.setFont(u8x8_font_5x7_f);
    disp.print("End ");
    disp.print(sEcount);
    if (p_Store.isFlint && sEcount > 6) {
      disp.print(" (walkup)  ");      // confirm walkup context for operator
    } else {
      disp.print(" active:   ");
    }

    // ── Row 2: sub-phase or single-detail indicator ───────────
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.setCursor(0, 2);
    activeSubPhaseLabel();            // Flint-guarded — shows "Single detail" for Flint

    // ── Divider ───────────────────────────────────────────────
    disp.setCursor(0, 3);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    disp.print("----------------");

    // ── BTN1: full end restart ─────────────────────────────────
    disp.setCursor(0, 4);
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.inverse();
    disp.print("Full end: BTN[1]");
    disp.setCursor(0, 5);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    // Flint has no "first group" concept
    disp.print(p_Store.isFlint ? "(restart from top)" : "(from first grp) ");

    // ── BTN2: resume from active sub-phase ────────────────────
    disp.setCursor(0, 6);
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.inverse();
    disp.print("Resume:   BTN[2]");
    disp.setCursor(0, 7);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    disp.print(p_Store.isFlint ? "(resume this end )" : "(from active grp)");
    disp.setFont(u8x8_font_chroma48medium8_r);

    switch (waitButton()) {

      case BUTTON1:
        // ── Full end restart from beginning ───────────────────
        reStartEnd = true;
        for (byte reset = 0; reset <= 2; reset++) {
          n_Count_[reset] = maxSecs;  // walkup-aware — was startCounts[index], now correct
        }
        // Re-derive sE_iter for first sub-phase of this end
        // (goNormal_Op does sE_iter+=1 at top, so we go one below target)
        sE_iter     = deriveIterForEnd(sEcount);
        // Re-derive shootDetail from sEcount
        shootDetail = p_Store.ifaaIndoor
                      ? (sEcount > 6 ? true : false)
                      : ((sEcount % 2) == 1 ? false : true);
        clearMatrix(false);
        return n_Count_[nID];         // >= 0, decision made

      case BUTTON2:
        // ── Resume from where we stopped ──────────────────────
        reStartEnd = true;
        for (byte reset = 0; reset <= 2; reset++) {
          n_Count_[reset] = maxSecs;  // walkup-aware ceiling
        }
        sE_iter     = savedIter;      // restore sub-phase snapshot
        shootDetail = p_Store.ifaaIndoor
                      ? (sEcount > 6 ? true : false)
                      : ((sEcount % 2) == 1 ? false : true);
        clearMatrix(false);
        return n_Count_[nID];         // >= 0, decision made

      case BUTTON3:
        return -1;                    // back to Screen 1

      case BUTTON4:
        return -2;                    // reconsider — propagates up to goEmergencyButton
    }
  }
}


// ================================================================
//  4. REPLACE handleEmergencyRestart()
//
//  Find the existing handleEmergencyRestart() and replace.
//  CHANGES:
//    - BTN1 back-10 uses currentEndMax() ceiling — walkup-aware
//    - BTN3 full reset: guards flintRunning against mid-round flip
// ================================================================
int16_t handleEmergencyRestart(byte nID) {

  for (;;) {                          // loops if BTN3 on Screen 2 (result == -1)
    clearFromLine(1);

    // ── Row 1: context — operator always knows where they are ─
    disp.setCursor(0, 1);
    disp.setFont(u8x8_font_5x7_f);
    if (p_Store.ifaaIndoor) {
      disp.print(sEcount <= 6 ? "IFAA E1-6:  AB>CD"
                              : "IFAA E7-12: DC>BA");
    } else if (p_Store.isFlint && sEcount > 6) {
      disp.print("Flint walkup end");
    } else if (p_Store.isFlint) {
      disp.print("Flint end: ");
      disp.print(sEcount);
      disp.print("       ");
    } else {
      disp.print("End: ");
      disp.print(sEcount);
      disp.print("            ");
    }
    disp.setFont(u8x8_font_chroma48medium8_r);

    // ── BTN1: step back 10s ───────────────────────────────────
    disp.setCursor(0, 2);
    disp.inverse();
    disp.print("Resume:   BTN[1]");
    disp.setCursor(0, 3);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    disp.print("(steps back 10s)");

    // ── BTN2: restart options (Screen 2) ─────────────────────
    disp.setCursor(0, 4);
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.inverse();
    disp.print("Restart:  BTN[2]");
    disp.setCursor(0, 5);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    if (p_Store.ifaaIndoor) {
      disp.print(sEcount <= 6 ? "(restart: AB>CD )"
                              : "(restart: DC>BA )");
    } else if (p_Store.isFlint) {
      disp.print("(recommence end )");
    } else {
      disp.print("(recommence end )");
    }

    // ── BTN3: full reinitialise ───────────────────────────────
    disp.setCursor(0, 6);
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.inverse();
    disp.print("Reset ALL:BTN[3]");
    disp.setCursor(0, 7);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    disp.print("(reinitialize)  ");
    disp.setFont(u8x8_font_chroma48medium8_r);

    // Ceiling for this end — walkup-aware
    uint8_t maxSecs = currentEndMax();

    switch (waitButton()) {

      case BUTTON1:
        // Step back 10s — ceiling is currentEndMax(), not raw startCounts
        // +11 because the count has already decremented once since the
        // second started; net effect is +10 displayed seconds
        n_Count_[nID] = ((int)n_Count_[nID] + 11 > (int)maxSecs)
                        ? maxSecs
                        : n_Count_[nID] + 11;
        clearMatrix(false);
        return n_Count_[nID];         // >= 0, decision made

      case BUTTON2: {
        // Go to Screen 2 for restart options
        int16_t result = handleEmergencyRestart_Screen2(nID);
        if (result == -1) continue;   // BTN3 on Screen 2 — loop back to Screen 1
        if (result == -2) return -2;  // BTN4 on Screen 2 — propagate reconsider
        return result;                // valid count
      }

      case BUTTON3:
        // Full reinitialise — wipe everything and go back to start
        clearMatrix(false);
        startOver = true;
        memset(n_Count_, 0, sizeof(n_Count_));
        // Guard: do NOT flip flintRunning here — that belongs in loop()
        // when the natural end-of-round condition is met, not on emergency reset.
        // flintRunning is left unchanged; loop() will handle it correctly
        // when startOver causes the done/reset sequence to run.
        return n_Count_[nID];         // 0, triggers done sequence in loop()

      case BUTTON4:
        // Reconsider — signal goEmergencyButton to restore snapshot and EXIT
        return -2;
    }
  }
}


// ================================================================
//  5. REPLACE goEmergencyButton()
//
//  Find the existing goEmergencyButton() and replace.
//  CHANGES:
//    - BTN4 reconsider now EXITS cleanly (ret = false, break)
//      rather than looping back into the emergency menu.
//      This restores the "oops/carry on" behaviour the operator expects.
//    - Snapshot includes all three globals as before.
//    - AIndex bottom section unchanged.
// ================================================================
bool goEmergencyButton(uint8_t AIndex, byte nID) {
  bool ret = false;

  if (readButtons() == BUTTON4) {
    ret = true;
    emergencyReconsider = false;      // clear flag at entry

    goWhistle(5);
    stopSign();
    clearFromLine(1);
    disp.setCursor(4, 3);
    disp.print("EMERGENCY");
    disp.setCursor(6, 5);
    disp.print("STOP");
    pauseMe(5 * tick);

    // ── Snapshot global state at moment of emergency ─────────
    // All three are restored if operator chooses BTN4 reconsider.
    int8_t  snapIter   = sE_iter;
    int8_t  snapEcount = sEcount;
    bool    snapDetail = shootDetail;

    for (;;) {
      int16_t result = handleEmergencyRestart(nID);

      if (result == -2) {
        // ── Operator reconsiders ─────────────────────────────
        // Restore globals to pre-emergency state.
        sE_iter     = snapIter;
        sEcount     = snapEcount;
        shootDetail = snapDetail;
        // p_Store.isFlint / p_Store.ifaaIndoor are never touched by
        // emergency code — they come from EEPROM and stay intact.

        // Signal to calling mode that we are reconsidering,
        // then EXIT as if the emergency button was never pressed.
        // ret = false would be ideal but the calling code in c_Norm.ino
        // checks (goEmergencyButton(...)) for true, so we must return true
        // but set emergencyReconsider so the caller restores its locals too.
        emergencyReconsider = true;   // caller (goNormal_Op etc.) restores locals
        clearFromLine(1);
        disp.setCursor(0, 3);
        disp.print("  Reconsidering ");
        disp.setCursor(0, 5);
        disp.print("Resuming count..");
        pauseMe(2 * tick);
        // Do NOT re-enter the menu — break out and return to the count
        break;
      }

      // ── Decision made — proceed ───────────────────────────
      n_Count_[nID] = result;
      emergencyReconsider = false;    // clean exit, no reconsider
      break;
    }

    // ── Post-emergency display setup ─────────────────────────
    // Only runs when a real decision was made (not reconsider).
    // On reconsider, emergencyReconsider == true and the caller
    // handles display restore itself.
    if (!emergencyReconsider) {
      if (AIndex < 3) {
        displayParamsOnOLED();
        // Use currentEndMax() to decide whether to run doBarCount —
        // a full restart returns the full end duration
        if (n_Count_[nID] == currentEndMax()) {
          doBarCount(AIndex, nID);
          writeOLED_Data(1, nID);
        } else if (n_Count_[nID] != 0) {
          goWhistle(1);
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

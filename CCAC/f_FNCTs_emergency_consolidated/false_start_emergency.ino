// ================================================================
//  FALSE START EMERGENCY HANDLING — CHANGES ONLY
//  Apply to k_Clock.ino (doCountdownBar, doBarCount)
//  and c_Norm.ino (goNormal_Op)
//
//  Summary:
//  1. doCountdownBar() — detect BTN4, set reStartEnd, whistle,
//     show FALSE START screen, wait for BTN1 or BTN3, return
//  2. doBarCount()     — no signature change needed; reStartEnd
//     propagates naturally via global
//  3. goNormal_Op()    — check reStartEnd after doBarCount(),
//     call writeReady(), continue to top of loop
// ================================================================


// ================================================================
//  1. REPLACE doCountdownBar()  — k_Clock.ino
//
//  Find existing doCountdownBar() and replace entirely.
//  CHANGE: BTN4 detection inside the do{}while loop.
//  On BTN4: whistle, wipe, show FALSE START screen,
//  wait for BTN1 (writeReady path) or BTN3 (full reset).
//  Sets reStartEnd or startOver accordingly then returns.
// ================================================================
void doCountdownBar(void) {
  byte loc_Count = (p_Store.isFlint) && (sEcount > 7) ?   // Flint walkup: 20s
                    20 : p_Store.walkUp;                   // else configured walkup
  byte n_Loc     = loc_Count;                              // working count copy
  byte lnNumber  = 30;
  int  clockPulse;
  byte rectWide  = 49;

  writeRectangle(red, 0, lnNumber, rectWide, 13);          // initial red bar
  HC12.flush();
  writeStopwatch(n_Loc);
  sendNumber(red, 50, lnNumber, n_Loc--);                  // first number, decrement
  HC12.flush();

  bool flag = false;
  unsigned long secCount = millis();
  do {} while ((millis() - secCount) < 100);               // settle

  do {
    // ── BTN4: false start emergency ───────────────────────
    if (readButtonNoDelay(button4Pin) == 1) {
      pauseMe(5);                                          // debounce confirm
      if (readButtonNoDelay(button4Pin) == 1) {            // still held

        goWhistle(5);                                      // emergency whistle
        clearMatrix(false);                                // wipe bargraph
        clearFromLine(1);

        // FALSE START screen
        disp.setCursor(0, 2);
        disp.draw2x2String(0, 2, "FALSE");
        disp.draw2x2String(0, 4, "START");
        disp.setCursor(0, 6);
        disp.setFont(u8x8_font_chroma48medium8_r);
        disp.inverse();
        disp.print("Ready:    BTN[1]");                    // back to READY state
        disp.noInverse();
        disp.setCursor(0, 7);
        disp.print("Reset ALL:BTN[3]");                    // full competition reset
        disp.setFont(u8x8_font_chroma48medium8_r);

        // Wait for BTN1 or BTN3
        for (;;) {
          byte btn = readButtons();

          if (btn == BUTTON1) {
            // Return to READY — goNormal_Op handles writeReady + continue
            reStartEnd = true;                             // signal to goNormal_Op
            return;                                        // exit doCountdownBar
          }

          if (btn == BUTTON3) {
            // Full competition reset
            clearMatrix(false);
            flintRunning = false;                          // always reset to Flint #1
            startOver    = true;
            memset(n_Count_, 0, sizeof(n_Count_));
            return;                                        // exit doCountdownBar
          }
        }
      }
    }

    // ── BTN1: operator proceeds early (existing behaviour) ─
    if (readButtonNoDelay(button1Pin) == 1) {
      goGreenZero();
      return;
    }

    // ── Normal bargraph tick ───────────────────────────────
    clockPulse = (((millis() - secCount) % 1000) / 100);
    switch (abs(clockPulse)) {

      case 2:
      case 4:
      case 6:
        if (n_Loc <= 9) {
          writeLine(3, rectWide, 18, rectWide, lnNumber);
          HC12.flush();
          rectWide--;
        }
        break;

      case 8:
        writeRectangle(0, 50, lnNumber, 15, 13);
        HC12.flush();
        if (n_Loc <= 9) {
          writeLine(3, rectWide, 18, rectWide, lnNumber);
          rectWide--;
          HC12.flush();
        }
        break;

      case 0:
        sendNumber(n_Loc ? red : green,
                   (n_Loc >= 10) ? 50 : 54,
                   lnNumber, n_Loc);
        writeStopwatch(n_Loc);
        n_Loc--;
        HC12.flush();
        if (n_Loc >= 10) break;
        writeLine(orange, rectWide, 18, rectWide, lnNumber);
        HC12.flush();
        rectWide--;
        break;
    }

    if (n_Loc == 16 && !flag) {                            // Flint distance label
      sendSerialS(green, 2, 15, "==>> ", flintWalk[sEcount - 7]);
      HC12.flush();
      flag = true;
    }

  } while ((millis() - secCount) < (1000UL * loc_Count));

  goGreenZero();
}


// ================================================================
//  2. doBarCount() — k_Clock.ino
//
//  NO CHANGE to doBarCount() itself.
//  reStartEnd is set inside doCountdownBar() and propagates
//  naturally as a global. doBarCount() calls doCountdownBar()
//  and returns — goNormal_Op() checks reStartEnd on return.
//
//  Leave doBarCount() exactly as it is.
// ================================================================


// ================================================================
//  3. CHANGE goNormal_Op() — c_Norm.ino
//
//  Find this block inside goNormal_Op():
//
//    if (!next) {
//      sendDetail(true);
//      doBarCount(archerIndex, nID);
//    }
//    sendDetail(false);
//
//  REPLACE WITH:
// ================================================================

    if (!next) {
      sendDetail(true);                                    // detail display before barcount
      doBarCount(archerIndex, nID);                        // walkup bargraph countdown

      if (reStartEnd && !startOver) {
        // ── False start — operator pressed BTN4 during barcount
        // reStartEnd set inside doCountdownBar() via BTN1 choice.
        // Go back to READY screen, wait for green, re-run barcount.
        reStartEnd = false;                                // clear flag
        sE_iter--;                                        // undo the increment at top
                                                          // of this loop iteration so
                                                          // re-entry lands on same detail
        writeReady();                                     // show READY, wait for green BTN1
        continue;                                         // back to top of while(continueOn)
                                                          // re-runs n_Count_, sE_iter++,
                                                          // sendDetail, doBarCount cleanly
      }
    }
    sendDetail(false);                                     // detail display at start of shooting

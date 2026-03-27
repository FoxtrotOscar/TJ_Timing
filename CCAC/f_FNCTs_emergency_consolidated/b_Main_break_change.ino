// ================================================================
//  b_Main.ino — BREAK SEQUENCE CHANGE
//  Changes only — apply to the loop() function
//
//  CHANGE: At end of Flint #1, show BREAK screen instead of DONE.
//          DONE only fires at end of Flint #2 (or non-Flint rounds).
//
//  FIND this block in loop() — the flintRunning flip line and the
//  DONE flash sequence that follows it:
//
//    flintRunning = (!flintRunning && p_Store.isFlint) ? true : false;
//    countPractice = flintRunning ? 0 : ...
//    ...
//    for (int p = 0; p < 5; p ++ ) {   // flash the good news
//      clearMatrix(false);
//      ...
//      sendSerialS(green, 2, lnNumber, "D O N E");
//      ...
//    }
//
//  REPLACE the entire if (p_Store.maxEnds != 1) block with the
//  version below.
// ================================================================


// ================================================================
//  FIND AND REPLACE — inside loop(), the block:
//
//    if (p_Store.maxEnds != 1) {
//      ...DONE flash sequence...
//    }
//
//  REPLACE WITH:
// ================================================================

  if (p_Store.maxEnds != 1) {                       // skip for single-end rounds

    if (flintRunning && !startOver) {
      // ── End of Flint #1 — show BREAK, not DONE ───────────────
      // flintRunning just flipped to true — this is the inter-round break.
      // Do NOT trigger the interval timer — just inform the operator.

      // Matrix: large green BREAK
      clearMatrix(false);
      lnNumber = 25;
      HC12.print(F("font 11\r"));
      HC12.flush();
      sendSerialS(green, /*col*/ 0, /*ln*/ lnNumber, "BREAK");
      pauseMe(tick);

      // OLED break screen
      clearFromLine(1);
      disp.setCursor(0, 2);
      disp.draw2x2String(0, 2, "BREAK");            // large BREAK on OLED rows 2-3

      disp.setCursor(0, 4);
      disp.setFont(u8x8_font_5x7_f);
      disp.print("(");
      disp.print(p_Store.breakPeriod);              // configured break duration
      disp.print("min set)        ");

      disp.setCursor(0, 5);
      disp.print("                ");               // blank divider row

      disp.setCursor(0, 6);
      disp.setFont(u8x8_font_chroma48medium8_r);
      disp.inverse();
      disp.print("Timer  : BTN[3]");                // 15 chars — enters break timer
      disp.noInverse();

      disp.setCursor(0, 7);
      disp.print("Bypass : BTN[4]");                // 15 chars — skip timer, go to wait

      disp.setFont(u8x8_font_chroma48medium8_r);

      // Wait for operator decision
      for (;;) {
        byte btn = readButtons();

        if (btn == BUTTON3) {
          // ── Enter break timer menu ──────────────────────────
          // Set intervalOn — checkIntervalTimer() handles the rest.
          // Operator proceeds with BTN1 when ready.
          intervalOn = true;
          started    = false;                       // reset so timer initialises fresh
          lapsed     = p_Store.breakPeriod * 60;   // pre-load for display
          clearFromLine(1);
          break;                                    // exit to normal end-of-round flow
        }

        if (btn == BUTTON4) {
          // ── Bypass timer — go straight to wait state ────────
          // Simulate post-break condition: waiting, ready for #2
          intervalOn = false;
          showWaiting(true);                        // scroll underline, standing by
          break;                                    // exit to normal end-of-round flow
        }
      }

    } else {
      // ── Normal DONE — end of Flint #2 or any non-Flint round ─
      if (!p_Store.isFinals) {
        clearFromLine(1);
        disp.draw2x2String(0, 6, "~~DONE~~");
      }
      lnNumber = 25;
      HC12.print(F("font 11\r"));
      HC12.flush();
      if (p_Store.isFlint) goWhistle(3);            // Flint #2 end whistle

      for (int p = 0; p < 5; p++) {                // flash the good news
        clearMatrix(false);
        pauseMe(tick / 4);
        sendSerialS(green, /*col*/ 2, /*ln*/ lnNumber, "D O N E");
        pauseMe(tick);
      }
    }

  } // end if (p_Store.maxEnds != 1)

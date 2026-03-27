// ================================================================
//  b_Main.ino — BREAK / DONE / RESET three-path change
//  Changes only — apply to loop()
//
//  Replaces the previous b_Main_break_change.ino version.
//  Use this file, not the earlier one.
//
//  Three distinct paths inside if (p_Store.maxEnds != 1):
//
//    startOver == true          → RESET (amber, splash, wait)
//    flintRunning && !startOver → BREAK (green, timer/bypass)
//    everything else            → DONE  (green, existing sequence)
//
//  FIND in loop():
//
//    if (p_Store.maxEnds != 1) {
//      ...existing DONE flash sequence...
//    }
//
//  REPLACE THE ENTIRE BLOCK WITH THE CODE BELOW.
// ================================================================


  if (p_Store.maxEnds != 1) {                       // skip for single-end rounds

    if (startOver) {
      // ── RESET path — emergency BTN3 full restart ─────────────
      // Operator has called a full competition restart mid-round.
      // Show RESET in amber, devolve to splash, wait for input.

      // Matrix: large amber RESET
      clearMatrix(false);
      lnNumber = 25;
      HC12.print(F("font 11\r"));
      HC12.flush();
      sendSerialS(orange, /*col*/ 0, /*ln*/ lnNumber, "RESET");
      pauseMe(3 * tick);                            // hold long enough to read
      clearMatrix(false);

      // OLED: minimal — proceed prompt
      clearFromLine(1);
      disp.setCursor(1, 3);
      disp.print("Proceed: BTN[1]");

      // Devolve to splash — operator taps card or BTN1 to restart
      writeSplash(true);

    } else if (flintRunning) {
      // ── BREAK path — end of Flint #1 ─────────────────────────
      // flintRunning just flipped to true — inter-round break.
      // Inform operator, let them choose timer or bypass.

      // Matrix: large green BREAK
      clearMatrix(false);
      lnNumber = 25;
      HC12.print(F("font 11\r"));
      HC12.flush();
      sendSerialS(green, /*col*/ 0, /*ln*/ lnNumber, "BREAK");
      pauseMe(tick);

      // OLED break screen
      clearFromLine(1);
      disp.draw2x2String(0, 2, "BREAK");            // large on rows 2-3

      disp.setCursor(0, 4);
      disp.setFont(u8x8_font_5x7_f);
      disp.print("(");
      disp.print(p_Store.breakPeriod);              // configured break minutes
      disp.print("min set)        ");

      disp.setCursor(0, 5);
      disp.print("                ");               // blank divider

      disp.setCursor(0, 6);
      disp.setFont(u8x8_font_chroma48medium8_r);
      disp.inverse();
      disp.print("Timer  : BTN[3]");                // enters break countdown
      disp.noInverse();

      disp.setCursor(0, 7);
      disp.print("Bypass : BTN[4]");                // skip timer, go to wait
      disp.setFont(u8x8_font_chroma48medium8_r);

      // Wait for operator decision
      for (;;) {
        byte btn = readButtons();

        if (btn == BUTTON3) {
          // Enter break timer — checkIntervalTimer() handles countdown
          intervalOn = true;
          started    = false;                       // reset so timer initialises fresh
          lapsed     = p_Store.breakPeriod * 60;   // pre-load lapsed for display
          clearFromLine(1);
          break;                                    // exit to end-of-round flow
        }

        if (btn == BUTTON4) {
          // Bypass timer — simulate post-break, go straight to wait
          intervalOn = false;
          showWaiting(true);                        // scroll underline, standing by
          break;                                    // exit to end-of-round flow
        }
      }

    } else {
      // ── DONE path — natural end, non-Flint or Flint #2 ───────
      if (!p_Store.isFinals) {
        clearFromLine(1);
        disp.draw2x2String(0, 6, "~~DONE~~");
      }
      lnNumber = 25;
      HC12.print(F("font 11\r"));
      HC12.flush();
      if (p_Store.isFlint) goWhistle(3);            // Flint #2 completion whistle

      for (int p = 0; p < 5; p++) {                // flash DONE five times
        clearMatrix(false);
        pauseMe(tick / 4);
        sendSerialS(green, /*col*/ 2, /*ln*/ lnNumber, "D O N E");
        pauseMe(tick);
      }
    }

  } // end if (p_Store.maxEnds != 1)

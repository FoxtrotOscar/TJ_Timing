// ================================================================
//  SCREEN 2 DISPLAY STRING FIXES — apply to
//  handleEmergencyRestart_Screen2() in f_FNCTs.ino
//
//  Replace only the display string sections shown below.
//  All logic unchanged.
// ================================================================


// ── BTN1 subtitle — find and replace ────────────────────────────
//
// FIND:
//    disp.print(p_Store.isFlint ? "(resume this end)"
//                               : "(from active grp)");
// REPLACE WITH:
    disp.print(p_Store.isFlint ? "(resume end)    "
                               : "(from activegrp)");
//  16 chars exactly in both cases


// ── BTN2 subtitle IFAA — find and replace ───────────────────────
//
// FIND:
//    disp.print(p_Store.ifaaIndoor
//               ? (sEcount <= 6 ? "(restart AB>CD) "
//                               : "(restart DC>BA) ")
//               :                  "(from first grp)");
// REPLACE WITH:
    disp.print(p_Store.ifaaIndoor
               ? (sEcount <= 6 ? "(restart AB>CD) "
                               : "(restart DC>BA) ")
               :                  "(from firstgrp) ");
//  16 chars exactly in all cases


// ── BTN4 hint — find and replace ────────────────────────────────
//
// FIND:
//    disp.print("Reconsider:BTN[4]");
// REPLACE WITH:
    disp.print("Reconsider:  [4]");
//  16 chars exactly

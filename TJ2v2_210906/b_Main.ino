
/*
  uint8_t startCountsIndex = 1;         // (1)Number from 0 to 4 indentifying which of startCounts is used in this round, default 120
  uint8_t walkUp = 10;                  // (2)
  uint8_t maxEnds = 4;                  // (3)Total number of Ends for competition
  uint8_t Details = 2;                  // (4)Single (1) or Double detail (2)
  uint8_t maxPrac = 2;                  // (5)Initially set as 2x practice ends
  uint8_t isFinals = 0;                 // (6)For alternating A & B session
  uint8_t isAlternating = 0;            // (8)1 == Archer A, Archer B; 0 == Simultaneous
  uint8_t teamPlay = 0;                 // (9)
  uint8_t whichArcher = 1;              //(10)
*/

void loop() {

  if ((sEcount > paramStore.maxEnds) || startOver) {          // test here for DONE or reset
    if (paramStore.maxEnds != 1) {                            // But not for single ends
      clearFromLine(1);
      if (!paramStore.isFinals) {
        u8x8.draw2x2String(0, 6, "~~DONE~~");
      }
      lnNumber = 25;
      HC12.print(F("font 11\r"));
      HC12.flush();
      if (paramStore.notFlint == 0) goWhistle(3);
      for (int p = 0; p < 5; p ++ ) {                         // flash the good news
        clearMatrix();
      pauseMe(tick / 4);
        sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 2, /*line=*/ lnNumber, "D O N E");
      pauseMe(tick);
      }
    }
    clearMatrix();
    displayParamsOnOLED();
    writeSplash(true);                                        // Show Logo
    writeMenuCommands(); 
    continueOn = 0;
    startOver = false;
    while (continueOn == 0)  continueOn = waitButton();       // Alter params or re-start as is
  pauseMe(tick);
    showWaiting(false);
    clearMatrix();
    sEcount = 1;
    sE_iter = 0;
    countPractice = paramStore.maxPrac;
  }

  continueOn = goMenu(true);
  if (sEcount == 1 && countPractice == 0) {                   // If  after Practice and before first END
    if (paramStore.isFinals == 1) {
      showWaiting(false);
      clearFromLine(5);
      u8x8.draw2x2String(0, 6, " FINALS ");
      clearMatrix();
      sendScrollW( /*speed=*/  4,                              // 3 - 15
                               /*loop=*/   0,
                               /*wiggle=*/ 2,
                               /*colour=*/ 3,
                               /*column=*/ 1,
                               /*line=*/   22 ,
                               /*window=*/ 63,
                                          "FINAL!" );          // animation of finals
    pauseMe(tick * 2);                                    
      if (paramStore.isAlternating) {
        clearFromLine(5);
        goChooseArcher();
      } else  paramStore.whichArcher = 0;
    }
    clearFromLine(5);
    u8x8.draw2x2String(0, 6, "SCORING!");
    sEcount = 1;                                                //cope with flint start error after timetap
    clearMatrix();

    HC12.print(F("font 9\r"));  HC12.flush();                   // Then write SCORING
    sendSerialS( /*colour=*/ 1, /*column=*/ 6, /*line=*/22, "SCORING ");
  pauseMe(3 * tick);
    clearMatrix();
  } else {

    if (sEcount == 1 && countPractice == paramStore.maxPrac) {
      showWaiting(false);
      clearFromLine(5);
      u8x8.draw2x2String(0, 6, "PRACTICE");
      clearMatrix();

      HC12.print(F("font 10\r"));  HC12.flush();
      sendSerialS( /*colour=*/ 3, /*column=*/ 0, /*line=*/ 15, "Practice");

    pauseMe(tick);
      HC12.print(F("font 9\r"));   HC12.flush();
      sendSerialS( /*colour=*/ 2, /*column=*/ 2, /*line=*/ 29, "ENDS -- ");
      txtColour = orange;
      colNumber = 54;
      lnNumber = 30;
      sendNumber(paramStore.maxPrac);
    pauseMe(3 * tick);
    }
  }


  if (!paramStore.isFinals) {
    goNormal_Op();
  } else {
    clearFromLine(1);
    goFinals_Op();
  }
}

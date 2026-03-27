  
/*
  uint8_t startCountsIndex = 1;         // (1)Number from 0 to 8 indentifying which of startCounts is used in this round, default 120
  uint8_t walkUp = 10;                  // (2)
  uint8_t maxEnds = 4;                  // (3)Total number of Ends for competition
  uint8_t Details = 2;                  // (4)Single (1) or Double detail (2)
  uint8_t maxPrac = 2;                  // (5)Initially set as 2x practice ends
  uint8_t isFinals = 0;                 // (6)For alternating A & B session
  uint8_t isAlternating = 0;            // (8)1 == Archer A, Archer B; 0 == Simultaneous
  uint8_t teamPlay = 0;                 // (9)
  uint8_t whichArcher = 1;              //(10)

  Functions:
  18    Loop
  
*/

void loop() {
  if ((sEcount > p_Store.maxEnds) || startOver) {               // test here for DONE or reset
    if (p_Store.maxEnds != 1) {                                 // But not for single ends
      clearFromLine(1);
      if (!p_Store.isFinals) {
        disp.draw2x2String(0, 6, "~~DONE~~");
      }
      lnNumber = 25;
      HC12.print(F("font 11\r"));
      HC12.flush();
      if (p_Store.isFlint) goWhistle(3);
      
      for (int p = 0; p < 5; p ++ ) {                          // flash the good news
        clearMatrix(false);
        pauseMe(tick / 4);
        sendSerialS(green,/*col*/ 2,/*ln*/ lnNumber, "D O N E");
        pauseMe(tick);      
      } 
    }
    clearMatrix(false);
    wipeOLED();
    flintRunning = (!flintRunning && p_Store.isFlint)?  true : false;
    countPractice = flintRunning ? 0 :                          // if Flint is running > 1st round then no prac 
                    p_Store.maxPrac !=0 && p_Store.isFlint ? 1 : p_Store.maxPrac;     //and 1 prac only
    displayParamsOnOLED();
    disp.draw2x2String(0, 6, "..WAIT..");
    writeSplash(true);                                          // Show Logo
    writeMenuCommands(); 
    continueOn = false;
    startOver = false;
    showWaiting(false);                                         // Turn off scrolling wait indic - if running
    sEcount = 1;
    sE_iter = 0;   //countPractice = p_Store.maxPrac;  deprecated because of line 39
  } 

  continueOn = goMenu(true);
  showWaiting(false);                                           // Turn off scrolling wait indic - if running
  if (sEcount == 1 && countPractice == 0) {                     // If  after Practice and before first END
    if (p_Store.isFinals) {
      #ifndef DEBUG
      clearFromLine(5);
      disp.draw2x2String(0, 6, " FINALS ");
      clearMatrix(false);
      sendScrollW(/*speed=*/    4,                              // 3 - 15
                  /*loop=*/     0,
                  /*wiggle=*/   2,
                  /*colour=*/   3,
                  /*column=*/   1,
                  /*line=*/    22,
                  /*window=*/  63,
                          "FINAL!" );                           // animation of finals
      #endif
      HC12.flush();                                    
      if (p_Store.isAlternating && p_Store.teamPlay < 11) {
        clearFromLine(5);
        goChooseArcher();
      } else  p_Store.whichArcher = 0;
    }
    clearFromLine(5);
    disp.draw2x2String(0, 6, "SCORING!");
    sEcount = 1;                                                //cope with flint start error after timetap
    clearMatrix(false);
    HC12.print(F("font 9\r"));  HC12.flush();                   // Then write SCORING
    sendSerialS( red, /*column=*/ 6, /*line=*/22, p_Store.teamPlay < 11 ? "SCORING " : " ");
    if (p_Store.teamPlay < 11){
      pauseMe(3 * tick);
      clearMatrix(false);
    }
  } else {
    //if (p_Store.isFlint || p_Store.ifaaIndoor) p_Store.maxPrac = 1;
    if (p_Store.isFlint ) p_Store.maxPrac = 1;
    if (sEcount == 1 && countPractice == p_Store.maxPrac) {
      clearFromLine(5);
      disp.draw2x2String(0, 6, "PRACTICE");
      clearMatrix(true);

      HC12.print(F("font 10\r"));  HC12.flush();
      sendSerialS( orange, /*column=*/ 0, /*line=*/ 15, "Practice");

      pauseMe(tick);
      HC12.print(F("font 9\r"));   HC12.flush();
      sendSerialS( green, /*column=*/ 2, /*line=*/ 29, "ENDS -- ");
      txtColour = orange;
      colNumber = 54;
      lnNumber  = 29;
      sendNumber(txtColour, colNumber, lnNumber, p_Store.maxPrac);
      pauseMe(3 * tick);
      clearMatrix(false);
    }
  }
  if      (p_Store.teamPlay >= 11) goTeamPlay(p_Store.teamPlay);
  else if (p_Store.isFinals) goFinals_Op(0);
  else    goNormal_Op();
}

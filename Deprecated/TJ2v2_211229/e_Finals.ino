


void goFinals_Op(void){

  
  uint8_t offSet        = 13;
  uint8_t archerIndex   = paramStore.whichArcher;
  uint8_t tempCount     = 0;
  uint8_t arrowCount    = 3;
  bool    wFlag         = false;
  bool    shootOff      = false;
  /*
  * check for button1 press to start
  */
  while (continueOn && !startOver) {  
    if (shootOff) {
      arrowCount = 1;
      tempCount  = 0;
      
    }
    while (arrowCount > 0 ) {
    clearMatrix();
    //if ((countPractice && sEcount == 1) || !paramStore.isAlternating ){
    if ((countPractice && sEcount == 1) ){
      displayParamsOnOLED();
      writeOLED_Data(0);
      doBarCount(0);                                                          // no archer A or B here
      startCounts[paramStore.startCountsIndex] == 20 ? 
            n_Count = 3*startCounts[paramStore.startCountsIndex] :
            n_Count = startCounts[paramStore.startCountsIndex];
    }else{
      writeOLED_Data(paramStore.isAlternating ? archerIndex : 0); 
      if (sEcount == 1 && tempCount == 0 && !startOver) doBarCount(archerIndex);
      if (shootOff){
        doBarCount(archerIndex);
        shootOff = false;
      }
      n_Count = startCounts[paramStore.startCountsIndex];
      if (paramStore.isAlternating) offSet = (archerIndex == 1 ? 26 : 0);     // Set offset to lhs or rhs
    }
    unsigned long secCount = millis();
    
    do {} while ((millis() - secCount) % tick > 2);                           // initialise timer to a start-point
    
    clearMatrix();
    if (!countPractice ) {
      writeArcher(archerIndex, 2);
    
      HC12.print(F("font 13\r"));
      u8x8.setCursor(0, 6);
      u8x8.print(paramStore.isAlternating ? "Flip Detail: [1]" : "Halt End   : [1]");
      u8x8.setCursor(0, 7);
      u8x8.print(" !!!STOP!!!: [4]");
    }
    /*  Now that barCount has been completed and first whistle sounded we 
     *  start the first set (three arrows each), with NO whistle in between each arrow
     *  except where time is over-run for an archer - then whistle x1.  
     *  As usual, whistle x3 at the end of each set. 
     *  Judge turns over the play with the green button, and if he does not then there is a whistle. 
     *  We decide the first archer at the head of each three arrows, and put up a 
     *  signal for "Scoring", followed by "Ready" on green button
     *  Green Button re-commences.
     */
    writeOLED_Data((countPractice || !paramStore.isAlternating) ? 0 : archerIndex );
    while (n_Count >= 0){
      if (!goEmergencyButton(archerIndex)) {
        //writeOLED_Data((paramStore.isAlternating || !countPractice) ? archerIndex : 0);
        writeStopwatch(n_Count );
        HC12.print(F("font 13\r"));
        pauseMe(2);
        goClock(offSet);                                                      // Handles formatting of the display
        sendNumber(n_Count > 0 ? txtColour : 1, colNumber, lnNumber, n_Count );
        if (wFlag && n_Count == 0) {
          goWhistle(1);                                                       // sound whistle for time-out and continue 
          writeArcher(archerIndex, 1);
        }
        wFlag = handleCount(secCount);                                        // if wFlag is false then the count didn't run out
        n_Count -- ;
      }
    }
    if (!countPractice) {
      tempCount ++;
      if (paramStore.isAlternating){
        archerIndex = archerIndex == 1 ? 2 : 1;
        if (!(tempCount % 2)) arrowCount--;
        n_Count = startCounts[paramStore.startCountsIndex];
        
      } else {                                                                // Matchplay round non-alternating
        arrowCount = 0;
       // arrowCount --;
      }
    } else {                                                                  // for a practice round 
    countPractice --; 
    continueOn = false; 
    writeHalt();
    return; 
    }
  }                                                                           // where arrowCount runs out
  sEcount ++;
  
  if (sEcount > paramStore.maxEnds){
    continueOn = false;
    clearFromLine(1);
    writeHalt();
    
    if (checkForShootoff()) {                                                 // select whether a shoot-off is needed
      shootOff = true;
      continueOn = true;
      //sEcount = 1;
    }else{
      clearFromLine(1);
      return;
    }
  }
  shootOff ? writeShootOff() : writeScoring();
  clearFromLine(shootOff ? 6 : 1);
  u8x8.setCursor(0, 6);
  u8x8.inverse();
  u8x8.print("Proceed:  BTN[1]");
  u8x8.noInverse();
  for (;;) {
    bool flag = false;
    switch (readButtons()) {
      case BUTTON1: 
        n_Count = startCounts[paramStore.startCountsIndex];
        flag = true;
        break;
      
      case BUTTON4:
        continueOn = false;
        sEcount = paramStore.maxEnds +1; 
        writeHalt();
        flag = false;
        
        return;
    }
    if (flag) break;
  }
  arrowCount = shootOff ? 1 : 3;
  if (paramStore.isAlternating) {
    clearFromLine(5);
    goChooseArcher();
    archerIndex = paramStore.whichArcher;
  } else  archerIndex = 0;
  displayParamsOnOLED();
  writeOLED_Data(0);
  clearMatrix();
  if (!shootOff) doBarCount(archerIndex);
  }
}

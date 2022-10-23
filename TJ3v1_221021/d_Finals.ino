

void goFinals_Op(byte nID){
  uint8_t offSet        = 13;
  uint8_t archerIndex   = p_Store.whichArcher;
  uint8_t tempCount     = 0;
  byte    flipFlag      = 0;
  bool    shootOff      = false;
  bool    wFlag         = false;
  /*
  * check for button1 press to start
  */
  while (continueOn && !startOver) { 
    printDebugLine(false, __LINE__, __NAME__); 
    see(offSet);
    see(menu0[3]);
    arrowCount = shootOff ? 1 : 3;                      // if a shootoff then 1 arrow per player unless teamplay
    tempCount  = 0;
    while (arrowCount > 0 ) {
      printDebugLine(false, __LINE__, __NAME__);
      clearMatrix(false);
      if ((countPractice && sEcount == 1) ){                                  // our end count == 1 and Prac is ON
        displayParamsOnOLED();
        writeOLED_Data(0, nID);
        //doBarCount(0, nID);                                                   // no archer A or B here
        doBarCount( archerIndex, nID);
        startCounts[p_Store.startCountsIndex] == 20 ? 
              n_Count_[nID] = 3*startCounts[p_Store.startCountsIndex] :       // allow 3 arrows @20s for prac
              n_Count_[nID] = startCounts[p_Store.startCountsIndex];
              //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      }else{
        printDebugLine(false, __LINE__, __NAME__);
        writeOLED_Data(p_Store.isAlternating ? archerIndex : 0, nID); 
        if (sEcount == 1 && !tempCount && !startOver){
          doBarCount(archerIndex, nID);  
        }
        if (shootOff){
          doBarCount(archerIndex, nID);
          shootOff = false;
        }
        n_Count_[nID] = startCounts[p_Store.startCountsIndex];
        if (p_Store.isAlternating && !nID) offSet = (archerIndex == 1 ? 26 : 0);     // Set offset to lhs or rhs, else centre (13)
      }
      unsigned long secCount = millis();
      
      do {} while ((millis() - secCount) % tick > 2);                        // initialise timer to a start-point
      
      clearMatrix(false);
      if (!countPractice ) {
        if (!nID) writeArcher(archerIndex, 2);                              // if dual scr in play then no archer index
        HC12.print(F("font 13\r"));
        u8x8.setCursor(0, 6);
        u8x8.inverse();
        u8x8.print(p_Store.isAlternating ? "Flip Detail: [1]" : "Halt End   : [1]");
        u8x8.noInverse();
        u8x8.setCursor(0, 7);
        u8x8.print(" !!!STOP!!!: [4]");
      }
      /*  Now that barCount has been completed and first whistle sounded we 
      *  start the first set (three arrows each), with NO whistle in between each arrow
      *  except where time is over-run for an archer - then whistle x1.  
      *  As usual, whistle x3 at the end of each set. 
      *  Judge progresses the play with the green button, and if he does not and zero reached then there is a whistle. 
      *  We decide the first archer at the head of each three arrows, and put up a 
      *  signal for "Scoring", followed by "Ready" on green button
      *  Green Button re-commences.
      */
      printDebugLine(false, __LINE__, __NAME__);
      writeOLED_Data( ((countPractice || !p_Store.isAlternating) ? 0 : archerIndex), nID );
      uint8_t tempArrowCount[] = {arrowCount, arrowCount, arrowCount};
      while (n_Count_[nID] >= 0){                                               // repeat until n_Count[current] < 0
        if (!goEmergencyButton(archerIndex, nID)) {
          writeStopwatch(n_Count_[nID]);
          HC12.print(F("font 13\r"));
          pauseMe(2);
          goClock(offSet, nID);                                                 // Handles formatting of the display
          sendNumber(n_Count_[nID] > 0 ? txtColour : 1, colNumber, lnNumber, n_Count_[nID] );
          if (!flipFlag && n_Count_[nID] == 0 &&
                !(arrowCount == 1 && tempCount % 2)) {
            goWhistle(1);                                                       // sound whistle for time-out and continue 
            wFlag = true;
            writeArcher(archerIndex, 1);                                        // RED
          }else if(flipFlag == 2 ){                                             // if alternative screens and detail change is invoked
            tempArrowCount[nID] -= set_size;
            tempArrowCount[nID] == 0 ? n_Count_[nID] = 0 : n_Count_[nID] += 1;  // keeping the screen count right for holding
            byte temp = (nID == 1? set_B(1) : set_A(2));                        // Hold the curr count in red and change screen
            n_Count_[nID] -= 1;                                                 // set it back 1 after holding value sent
            nID = temp;
          }
          flipFlag = handleCount(secCount, nID);                                // if flipFlag is false then the count didn't run out
          if (n_Count_[nID] >= 0) n_Count_[nID]--;
        }
      }
      if (!countPractice) {
        tempCount ++;
        if (p_Store.isAlternating || !nID){ //<<<<<<  query "!""
          if (!nID){
            printDebugLine(false, __LINE__, __NAME__);
            archerIndex = archerIndex == 1 ? 2 : 1;
            if (wFlag) wFlag = false;
            else goWhistle(1);
          }else{
            printDebugLine(false, __LINE__, __NAME__);
            nID = (nID == 1 ? set_B(nID) : set_A(nID));                         // if nID is 1 or 2 then flip 
          }
          if (!nID && !(tempCount % 2)) {                                       // when all arrows shot by both archers
            arrowCount--;
            n_Count_[nID] = startCounts[p_Store.startCountsIndex];
          }
        } else {                                                                // Matchplay round non-alternating
          arrowCount = 0; printDebugLine(false, __LINE__, __NAME__);
        // arrowCount --;
        }
      } else {                                                                  // for a practice round 
      countPractice --; 
      continueOn = false;
      printDebugLine(false, __LINE__, __NAME__); 
      writeHalt();
      return; 
      }
    }                                                                           // where arrowCount runs out, change ends?
    sEcount ++;
    if (sEcount > p_Store.maxEnds){
      goWhistle(3);
      clearFromLine(1);
      score_Collect();
      //clearMatrix(false);
      shootOff = checkForShootoff();
      if (shootOff) {
        writeShootOff();
        continueOn = true;
        sEcount = p_Store.maxEnds +1;
      }else{                                                                    // Accept a winner and exit to DONE
        clearFromLine(1);
        return;
      }
    } 
    
    if (sEcount >= 1 && sEcount <= p_Store.maxEnds ) {
      goWhistle(3);
      score_Collect();
    }
    clearFromLine(shootOff ? 6 : 1);
    arrowCount = shootOff ? 1 : 3;
    if (p_Store.isAlternating && continueOrEnd(nID)) {
      printDebugLine(false, __LINE__, __NAME__);
      clearFromLine(5);
      shootOff ? clearMatrix(false) : writeReadySet();
      archerIndex = goChooseArcher();
      //archerIndex = p_Store.whichArcher;
    // } else if (!shootOff && continueOrEnd(nID)) {
    //   archerIndex = 0;
    //   shootOff ? writeShootOff() : writeReadySet();
    } else {
      if (!shootOff) //writeHalt();//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< if (shootOff???)
      return;
    }
    displayParamsOnOLED();
    writeOLED_Data(0, nID);
    clearMatrix(false);
    printDebugLine(false, __LINE__, __NAME__);
    if (!shootOff) doBarCount(archerIndex, nID);
  }
}

bool continueOrEnd(uint8_t nID){
  u8x8.setCursor(0, 6);
  u8x8.inverse();
  u8x8.print("Proceed:  BTN[1]");
  u8x8.noInverse();
  if (sEcount > 3){
    u8x8.setCursor(0, 7);
    u8x8.print(" WINNER?  BTN[3]");
  }
  for (;;) {
    bool flag = false;
    switch (readButtons()) {
      case BUTTON1:
        continueOn = true; 
        n_Count_[nID] = startCounts[p_Store.startCountsIndex];
        flag = true;
        break;
      
      case BUTTON3:
        continueOn = false;
        sEcount = p_Store.maxEnds +1;
        flag = true;
        break;
    }
    if (flag) break;
  } 
  return continueOn;
}

/*
 * STANDARD OPERATION (NOT FINALS) INCLUDING FLINT SETUP
 *
 * Flint operation requires 6 x 3min rounds (4 arrows)
 * //1 x walkup round of 45sec per 1 arrow with 15 sec from line to line//
 * 1 x walkup round of 30sec per 1 arrow with 20 sec from line to line
 * 2 x practice on first, 15 min break after first round
 * Rinse, repeat.
 */

/*
  uint8_t startCountsIndex = 1;         // ( 1)Number from 0 to 8 indentifying which of startCounts is used in this round, default 120 
  uint8_t walkUp = 10;                  // ( 2)
  uint8_t maxEnds = 4;                  // ( 3)Total number of Ends for competition
  uint8_t Details = 2;                  // ( 4)Single (1) or Double detail (2)
  uint8_t maxPrac = 2;                  // ( 5)Initially set as 2x practice ends
  uint8_t isFinals = 0;                 // ( 6)For alternating A & B session
  uint8_t isAlternating = 0;            // ( 8)1 == Archer A, Archer B; 0 == Simultaneous
  uint8_t teamPlay = 0;                 // ( 9)
  uint8_t whichArcher = 1;              // (10)

  FUNCTIONS:
   goNormal_Op
   
  
*/

void goNormal_Op(void){
  uint8_t offSet      = 13;
  uint8_t archerIndex = 1;
  byte    nID         = 0;                                          // ID of the clock in use
  next = false;
  //printDebugLine(false, __LINE__, __NAME__); 
  while (continueOn && !startOver) {
    if (countPractice) {                                            // Check, if Practice, then 
      n_Count_[nID] = p_Store.ifaaIndoor? 180 : startCounts[p_Store.startCountsIndex]; //only 180s practice in IFAA Indoor round
    } else {
      n_Count_[nID] = ((p_Store.isFlint) && (sEcount > 6)) ?        // make n_Count_[nID] (the count duration index), if Flint and 6 ends passed, 
                  startCounts[p_Store.startCountsIndex +1]:         // advance pointer to 30s for the closing Walkup shooting section  
                  startCounts[p_Store.startCountsIndex];            // otherwise remains as set.
    }                            

    sE_iter += 1 ;                                                  // iterate each time the clock cycles  
    
    if  (n_Count_[nID] == startCounts[p_Store.startCountsIndex] ||    // Play round start
        (countPractice && n_Count_[nID] == (p_Store.ifaaIndoor ? 180 : startCounts[p_Store.startCountsIndex])) || // Practice start
        (n_Count_[nID] == startCounts[p_Store.startCountsIndex+1] && sEcount > 6)) { // Flint walkup start    
      writeOLED_Data(1, nID);
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
          //writeReady();                                   // show READY, wait for green BTN1
          continue;                                         // back to top of while(continueOn)
                                                            // re-runs n_Count_, sE_iter++,
                                                            // sendDetail, doBarCount cleanly
        }
        if (!startOver) sendDetail(false);  // ← guard added
      }
      sendDetail(false);                                     // detail display at start of shooting
                                          // true/false = CD in upper/lower position
    } 
    unsigned long secCount = millis(); 
    do {} while ((millis() - secCount) % tick > 0);                 // initialise timer to a 000 start-point (slightly offset?)
    
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
    //printDebugLine(false, __LINE__, __NAME__); 
    see(n_Count_[nID]) ;
    see(startOver) ;       
    while(n_Count_[nID] >= 0 && !startOver){                        // Handle the count-down
      if (goEmergencyButton(archerIndex, nID)) {
        sendDetail(false);                                          // returning from an emergency halt
        do {} while ((millis() - secCount) % tick > 0);             // re-// initialise timer to a 000 start-point
      }
      HC12.print(F("font 13\r"));                                   // large numbers font
      HC12.flush(); 

      if ((p_Store.isFlint) &&                                      // or it is a Flint
         (n_Count_[nID] == startCounts[p_Store.startCountsIndex+1] 
                  && sEcount >  6)) {                               // and n_Count_[nID] has been advanced to Flint walkup
      }
      
      goClock(offSet, nID);                                         // Handles formatting of the display
      writeStopwatch(n_Count_[nID]  );                              // Write count large on OLED
      sendNumber(txtColour, colNumber, lnNumber, n_Count_[nID] );
      handleCount(secCount, nID);                                   // parks there until a second has elapsed
      n_Count_[nID] -- ;
      disp.setCursor(0, 6);                           
      
      (p_Store.Details == 2 && sE_iter%2 == 1 ) ?  disp.print("Flip Detail: [1]") : disp.print(" Finish END: [1]") ;
      disp.setCursor(0, 7);
      disp.print(" !!!STOP!!!: [4]");
    }
    if (!startOver) {
      //shootDetail  = !shootDetail;                                 // invert bool
      printDebugLine(false, __LINE__, __NAME__);
      shootDetail = p_Store.ifaaIndoor ? (sEcount > 6 ? true : false ) : !shootDetail;
      if(p_Store.Details == 2 && sE_iter%2 == 1){                   // now test for NEXT or HALT
        goWhistle(2);
        next = true;
        clearFromLine(5); 
        disp.draw2x2String(0, 6, ".WALKUP.");
        n_Count_[nID] = p_Store.walkUp;                             // setup count for the bar-graph
        rectWide = 49; //int rectWide = 50;
        barWidth = 5; 
        clearMatrix(false);
        HC12.flush();
        ++ sE_iter;                                                 // |
        writeOLED_Data(1, nID);                                     // | Correct for data on LCD
        -- sE_iter;                                                 // |
        lnNumber = 15;         
        HC12.print(F("font 9\r"));
        sendSerialS(green, /*column=*/ 2, /*line=*/ lnNumber, "NEXT:"); 
        if (p_Store.Details == 2 /* && !p_Store.isFlint */){                                  // Double detail?
          printDebugLine(false, __LINE__, __NAME__);
          sendSerialS (orange, 44, lnNumber,                        // colour(R1G2O3),  column, line 
                      (!p_Store.ifaaIndoor ?                        // Not IFAA
                      ( sE_iter%4 == 1  ? "C D" : "A B" ):          // flip
                      ( sEcount  <= 6 ?                             // IFAA!  First 6 ends
                      ( sE_iter%2 == 1 ? "C D" : "A B" ) :
                      ( sE_iter%2 == 0 ? "C D" : "A B" ))));        // A B  else C D                                 

        }
        doCountdownBar();
      } else if(p_Store.isFlint && sEcount > 6){
        if (sEcount > 9) continueOn = false;
        else writeReady(); //}}}}}}}}}}}}}}}
      } else {
        continueOn = false;
        writeHalt();
      }
        if (p_Store.Details == 2){
          if (sE_iter % 2 == 0) {
              if (!countPractice) {
                  sEcount++;
              } else {
                  countPractice--;
                  if (!countPractice) {          // just hit zero — competition starts next
                      sE_iter     = 0;           // reset so AB leads first competition end
                      shootDetail = 0;           // belt and braces — force AB
                  }
              }
          }
      } else {
          if (!countPractice) {
              sEcount++;
          } else {
              countPractice--;
              if (!countPractice) {              // just hit zero — competition starts next
                  sE_iter     = 0;              // reset so AB leads first competition end
                  shootDetail = 0;              // force AB
              }
          }
      }
      // if (p_Store.Details == 2){                                    // If double detail 
      //   if (sE_iter %2 == 0) !countPractice ? sEcount ++ : countPractice --;     // and an even number of iterations
        
      // } else  !countPractice ? sEcount ++ : countPractice --;       // countPractice approaches 0 
    }
  }
  if (startOver) {
    clearFromLine(1);
    disp.setCursor(1, 3);
    disp.print("Proceed: BTN[1]");
  }
}

/*
IFAA Target rotation rule: In the first six ends archers A and B will shoot first and on
the bottom target faces. In the second six ends archers C and D will shoot first
and on the bottom target faces.*/
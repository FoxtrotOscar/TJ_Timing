/*
 * STANDARD OPERATION (NOT FINALS) INCLUDING FLINT SETUP
 *
 * Flint operation requires 6 x 3min rounds (4 arrows)
 * 1 x walkup round of 30sec per 1 arrow with 15 sec from line to line
 * 2 x practice on first, 15 min break after walkup
 * Rinse, repeat.
 */

/*
  uint8_t startCountsIndex = 1;         // ( 1)Number from 0 to 6 indentifying which of startCounts is used in this round, default 120 
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
  byte    nID         = 0;                                          // ID if the clock in use
  /*
  * check for button1 press to start
  */
  next = false;
  while (continueOn && !startOver) {
    if (countPractice) {                                            // Check, if Practice, then 
      n_Count_[nID] = startCounts[p_Store.startCountsIndex]; 
    } else {
      n_Count_[nID] = ((p_Store.isFlint) && (sEcount > 6)) ?          // make n_Count_[nID] (the count duration index), if Flint and 6 ends passed, 
                  startCounts[p_Store.startCountsIndex +1]:                   // advance to 30s for the closing Walkup section  
                  startCounts[p_Store.startCountsIndex];                      // otherwise remains as set.
    }                            

    sE_iter += 1 ;                                                  // iterate each time the clock cycles  
    
    if (n_Count_[nID] == startCounts[p_Store.startCountsIndex] ||   // Are we at beginning of a Play round?
       (countPractice && n_Count_[nID] == 120) ||                   // Or a Practice round
       (n_Count_[nID] == startCounts[p_Store.startCountsIndex+1]   
        &&  sEcount > 6 )){                                         // Or are we at beginning of Flint walkup section?
      writeOLED_Data(1, nID);
      if (!next) {
        sendDetail(true);                                           // true/false = CD in upper/lower position
        doBarCount(archerIndex, nID);                               // Countdown
      }
      sendDetail(false);                                            // true/false = CD in upper/lower position
    } 
    unsigned long secCount = millis(); 
    do {} while ((millis() - secCount) % tick > 0);                 // initialise timer to a 000 start-point (slightly offset?)
    
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++        
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
      u8x8.setCursor(0, 6);                           
      
      (p_Store.Details == 2 && sE_iter%2 == 1 ) ?  u8x8.print("Flip Detail: [1]") : u8x8.print(" Finish END: [1]") ;
      u8x8.setCursor(0, 7);
      u8x8.print(" !!!STOP!!!: [4]");
    }
    if (!startOver) {
      shootDetail = !shootDetail;                                   // invert bool
      if(p_Store.Details == 2 && sE_iter%2 == 1){                   // now test for NEXT or HALT
        goWhistle(2);
        next = true;
        clearFromLine(5); 
        u8x8.draw2x2String(0, 6, ".WALKUP.");
        n_Count_[nID] = p_Store.walkUp;                             // setup count for the bar-graph
        rectWide = 49; //int rectWide = 50;
        barWidth = 5; 
        clearMatrix(false);
        ++ sE_iter;                                                 // |
        writeOLED_Data(1, nID);                                     // | Correct for data on LCD
        -- sE_iter;                                                 // |
        lnNumber = 15;         
        HC12.print(F("font 9\r"));
        sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 2, /*line=*/ lnNumber, "NEXT:"); 
        //lnNumber = 15; 
        if (p_Store.Details == 2){                                  // Double detail?
          sendSerialS(3,44,lnNumber,                                // colour(R1G2O3),  column, line
                      (sE_iter%4 == 1 ? "C D" : "A B")) ;           // if practice seq. then details do not flip, otherwise flip                      
        }
        doCountdownBar();
      } else if(p_Store.isFlint && sEcount > 6){
        if (sEcount > 9) continueOn = false;
        else writeReady(); //}}}}}}}}}}}}}}}
      } else {
        continueOn = false;
        writeHalt();
      }
      if (p_Store.Details == 2){                                    // If double detail 
        if (sE_iter %2 == 0) !countPractice ? sEcount ++ : countPractice --;                                     // and an even number of iterations
        
      } else  !countPractice ? sEcount ++ : countPractice --;
    }
  }
  if (startOver) {
    clearFromLine(1);
    u8x8.setCursor(1, 3);
    u8x8.print("Proceed: BTN[1]");
  }
}

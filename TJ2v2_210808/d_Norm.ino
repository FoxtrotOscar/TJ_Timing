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
  uint8_t offSet = 13;
  uint8_t archerIndex = 1;
/*
* check for button1 press to start
*/
  next = false;
  while (continueOn == 1 && !startOver) {

    n = ((paramStore.notFlint == 0) && (sEcount > 6)) ?         // make n (the count duration index), if Flint and 6 ends passed, 
        startCounts[paramStore.startCountsIndex +1]:            // advance to 30s for the closing Walkup section  
        startCounts[paramStore.startCountsIndex];               // otherwise remains as set.

    sE_iter += 1 ;                                              // iterate each time the clock cycles  
    
    if (n == startCounts[paramStore.startCountsIndex] ||        // Are we at beginning?
        (n == startCounts[paramStore.startCountsIndex+1] && 
         sEcount > 6 )){                                        // Are we at beginning of Flint walkup?
      writeOLED_Data(1);
      if (!next) {
        sendDetail(true);
        doBarCount(archerIndex);                                // Countdown
        
      }
      sendDetail(false);
    } 
    unsigned long secCount = millis(); 
    do {} while ((millis() - secCount) % tick > 0);             // initialise timer to a 000 start-point (slightly offset?)
    
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++        
    while( n >= 0 && !startOver){                               // Handle the count-down
      goEmergencyButton(archerIndex);
      HC12.print(F("font 13\r"));                               // large numbers font
      HC12.flush(); delay(tock);
      if (n == startCounts[paramStore.startCountsIndex] ||      // ? beginning ?
            ((paramStore.notFlint == 0) &&                      // or it is a Flint
            (n == startCounts[paramStore.startCountsIndex+1] && // and n has been advanced to Flint walkup
             sEcount >  6))) {                                  // and the count has exeeded 6th end (?redundant?)
            
        goWhistle(1);

      }
      goClock(offSet);                                          // Handles formatting of the display
      writeStopwatch(n);                                        // Write count large on OLED
      sendNumber(n);
      handleCount(secCount);                                    // parks there until a second has elapsed
      n -- ;
      u8x8.setCursor(0, 6);                           
      
      (paramStore.Details == 2 && sE_iter%2 == 1 ) ?  u8x8.print("Flip Detail: [1]") : u8x8.print(" Finish END: [1]") ;
      u8x8.setCursor(0, 7);
      u8x8.print(" !!!STOP!!!: [4]");
    }
    if (!startOver) {
  
      shootDetail = !shootDetail;                                 // invert bool
      if(paramStore.Details == 2 && sE_iter%2 == 1){              // now test for NEXT or HALT
        goWhistle(2);
        next = true;
        clearFromLine(5); 
        u8x8.draw2x2String(0, 6, ".WALKUP.");
        n = paramStore.walkUp;                                    // setup count for the bar-graph
        rectWide = 49; //int rectWide = 50;
        barWidth = 5; 
        clearMatrix();  
  
        while (n >= 0){                                           // do bar countdown  ((>-1)?)
          ++ sE_iter;                                             // |
          writeOLED_Data(1);                                      // | Correct for data on LCD <<<<<<<<<<<<<<<<<<<<<
          -- sE_iter;                                             // |
          lnNumber = 15;         
          HC12.print(F("font 9\r"));    HC12.flush();
          sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 2, /*line=*/ lnNumber, "NEXT:");      // Write NEXT:
            lnNumber = 15; 
          if (paramStore.Details == 2){                            // Double detail?
            sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 44, /*line=*/ lnNumber, 
                        (sE_iter%4 == 1 ? "C D" : "A B"));
          }
          n = doCountdownBar(n, rectWide, barWidth );
        }
      } else if(paramStore.notFlint == 0 && sEcount > 6){
        if (sEcount > 9) continueOn = 0;
        else writeReady();
      } else {
        continueOn = 0;
        
        writeHalt();
      }
      if (paramStore.Details == 2){                               // If double detail 
        if (sE_iter %2 == 0){                                     // and an even number of iterations
          if (countPractice == 0){
            sEcount ++;                                           // Add  to the End count
          }else { 
            countPractice --;                                            
          }
        }
      } else {
        if (countPractice == 0){
          sEcount ++;                                             // otherwise just iterate  ***********
        }else{
          countPractice --;
        }
      }
    }
  }
  if (startOver) {
    clearFromLine(1);
    u8x8.setCursor(1, 3);
    u8x8.print("Proceed: BTN[1]");
  }
}

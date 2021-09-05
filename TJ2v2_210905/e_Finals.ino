/*
 * Finals operation, as distinct from Standard.
 */
 
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

void goFinals_Op(void){

  
  uint8_t   offSet = 13;
  uint8_t   archerIndex = paramStore.whichArcher;
  uint8_t   tempEndCount = 0;
  /*
  * check for button1 press to start
  */
  while (continueOn == 1 && !startOver) {
    if ((countPractice && sEcount == 1) || !paramStore.isAlternating ){
      displayParamsOnOLED();
      writeOLED_Data(archerIndex); 
      doBarCount(archerIndex);
      goWhistle(1);
      n_Count = 120;
    }else{

      writeOLED_Data(paramStore.isAlternating ? archerIndex : 0); 
      if (sEcount == 1 && tempEndCount == 0 && !startOver) doBarCount(archerIndex);
      n_Count = startCounts[paramStore.startCountsIndex];
      if (paramStore.isAlternating) offSet = (archerIndex == 1 ? 26 : 0);     // Set offset to lhs or rhs
    }
    unsigned long secCount = millis();

    do {} while ((millis() - secCount) % tick > 2);                           // initialise timer to a start-point
    clearMatrix();
    
    if (!countPractice ) writeArcher(archerIndex);
    HC12.print(F("font 13\r"));
    while (n_Count >= 0){
      if (!goEmergencyButton(archerIndex)) {
      writeOLED_Data(archerIndex);
      writeStopwatch(n_Count );                                               // Write countdown large on OLED
      if (n_Count == startCounts[paramStore.startCountsIndex]) goWhistle(1);  // if we are at the beginning sound the GO!
        HC12.print(F("font 13\r"));
        delay(2); 
        goClock(offSet);                                                      // Handles formatting of the display
        
        sendNumber(n_Count );
        handleCount(secCount);
        n_Count -- ;
      }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 
    }
    if (!countPractice) {
      tempEndCount ++;
      if (paramStore.isAlternating){
        archerIndex = archerIndex == 1 ? 2 : 1;
        if (!(tempEndCount % 2)) {
          sEcount ++ ;                                                        // increase endcount when both archers done
        }
        n_Count = startCounts[paramStore.startCountsIndex];

      } else {
        sEcount ++ ;
        continueOn = 0;
        writeHalt();
      }
    } else { countPractice --; continueOn = 0; writeHalt(); }
    if (sEcount > paramStore.maxEnds){
      continueOn = 0;
      writeHalt();
    }
  }  
}

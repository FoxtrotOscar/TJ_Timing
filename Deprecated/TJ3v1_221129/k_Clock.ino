/*
 * MATRIX CLOCK OPERATION
 * 
  uint8_t startCountsIndex = 1;         // (1)Number from 0 to 4 indentifying which of startCounts is used in this round, default 120 
  uint8_t walkUp = 10;                  // (2)
  uint8_t maxEnds = 4;                  // (3)Total number of Ends for competition
  uint8_t Details = 2;                  // (4)Single (1) or Double detail (2)
  uint8_t maxPrac = 2;                  // (5)Initially set as 2x practice ends
  uint8_t isFinals = 0;                 // (6)For alternating A & B session
  uint8_t isAlternating = 0;            // (8)1 == Archer A, Archer B; 0 == Simultaneous
  uint8_t teamPlay = 0;                 // (9)
  uint8_t whichArcher = 1;              //(10)

 * Clock functions that handles the screen formatting etc:
  33  goClock
  80  goBlanking
  88  sendNumber
  99  handleCount
  114 sendSerialS  x3
  140 writeHalt
  181 writeReady
  198 writeReadySet
  205 score_Collect
  214 doBarCount
  258 doCountdownBar
  321 goGreenZero
  338 sendDetail
  357 writeA_B
  366 writeC_D
  375 writeArcher
 */

void goClock(uint8_t offSet, byte nID){                             // Setup for formatting the main count numbers
  switch(n_Count_[nID] ){
    case 0 ... 5:
      if (n_Count_[nID] == 9) goBlanking(tempOffset, nID);            // tempOffset is used to format the screen for different number widths
      txtColour = orange;
      colNumber = (p_Store.isFinals  ? 11 : 15) + offSet; 
      lnNumber = 30;                                                  //Orange, 1 digit
      break;
    case 6 ... 9:
      if (n_Count_[nID] == 9) goBlanking(tempOffset, nID);            // tempOffset is used to format the screen for different number widths
      txtColour = p_Store.teamPlay > 10 ||
                  p_Store.startCountsIndex == 4 ? green : orange;
      colNumber = (p_Store.isFinals  ? 11 : 15) + offSet; 
      lnNumber = 30;                                                  //Orange, 1 digit
      break;
    case 10 ... 19:
      if (n_Count_[nID] == 19) goBlanking(tempOffset, nID);
      txtColour = p_Store.teamPlay > 10 ||
                  p_Store.startCountsIndex == 4 ? green : orange;
      colNumber = (p_Store.isFinals  ? 2 : 4) + offSet;
      lnNumber = 30;                                                  //Orange, 2 digits
      tempOffset = (p_Store.isFinals  ? 2 : 4) + offSet;
      break;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
    case 20 ... 29:
      txtColour = p_Store.teamPlay > 10 ||
                  p_Store.startCountsIndex == 4 ? green : orange;
      colNumber = (p_Store.isFinals  ? 4 : 6) + offSet;
      lnNumber = 30;                                                  //Green, 2 digits
      tempOffset = (p_Store.isFinals  ? 4 : 6) + offSet;
      break;
    case 30 ... 99:
      if (n_Count_[nID] == 99) goBlanking(tempOffset, nID);
      txtColour = green; colNumber = 6 + offSet; lnNumber = 30;       //Green, 3 digits
      break;
    case 100 ... 199:
      if (n_Count_[nID] == 199) goBlanking(tempOffset, nID);
      txtColour = green; colNumber = -4 + offSet; lnNumber = 30;      //Green, 3 digits
      tempOffset = -4 + offSet;
      break;
    default:
      txtColour = green; colNumber = -2 + offSet; lnNumber = 30;      //Green, 3digits
      tempOffset = -2 + offSet;
    break;     
  }
}


void goBlanking(uint8_t tOffset, byte nID){                         // writes a blanking frame on switch from 
  txtColour = 0; colNumber = tOffset; lnNumber = 30;                // 3 digits to 2 digits etc; No colour (0)         
  sendNumber(txtColour, colNumber, lnNumber, n_Count_[nID] + 1);    // overwrite with last larger digit
}

/*
 * Function to output the actual clock output to the screen
 */
void sendNumber(int tColr, int cNum, int lnNum, int digits) {
  HC12.printf(
    F("text %u %u %u \"%u\"\rpaint\r"),
      tColr, cNum, lnNum, digits);
      HC12.flush();
}

/*
 * Handle the count using millis() for accuracy. We take secCount (set at beginning of each round)
 * and use it to find the point where millis() crosses a second (defined in tick = 1000)
 */
byte handleCount(unsigned long secCount, byte nID){
  uint8_t flipFlag = 0;
  while ((millis() - secCount) % tick > 0){         // apply a 'fuzzy' offset of 2 for Uno
    if (readButtonNoDelay(button1Pin) == 1) {
      pauseMe(175);
      if (nID == 0 ) {
        n_Count_[nID] = 0;                          // zero the count
        flipFlag = 1;
      } else flipFlag = 2;                          // in the case of alternating screens
    }
  }
  return flipFlag;
}


void sendSerialS(uint8_t txtColour1, uint8_t colNumber1, uint8_t lnNumber1, const char* i0, const char* i1, const char* i2){
  HC12.printf(
      F("text %u %u %u \"%s%s%s\"\rpaint\r"), 
      txtColour1, colNumber1, lnNumber1, 
      i0 != nullptr ? i0 : "", 
      i1 != nullptr ? i1 : "", 
      i2 != nullptr ? i2 : ""
      );  
  HC12.flush();
  pauseMe(2);
}

// this one calls the big one above 
void sendSerialS(uint8_t txtColour1, uint8_t colNumber1, uint8_t lnNumber1, const char* i0, const char* i1) {
  return sendSerialS(txtColour1, colNumber1, lnNumber1, i0, i1, nullptr); 
}

// same 
void sendSerialS(uint8_t txtColour1, uint8_t colNumber1, uint8_t lnNumber1, const char* i0) {
  return sendSerialS(txtColour1, colNumber1, lnNumber1, i0, nullptr, nullptr); 
}


/*
 * When the Detail(s) have finished, for arrow collection
 */
void writeHalt(void){
  goWhistle(3);
  clearFromLine(5); 
  u8x8.draw2x2String(0, 6, "..HALT..");
  HC12.print(F("font 11\r"));                                       //Change font for the text msg
  HC12.print(F("rect 1 0 32 64 32\r")); 
  HC12.print(F("rect 0 0 25 64 18\r"));   
  lnNumber = 24;                                                    // Print text Centre and in RED  
  sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 2, /*line=*/ lnNumber, "H A L T");
  //goWhistle(3);
  pauseMe(5*tick);
  clearMatrix(false);  

  HC12.print(F("font 9\r"));    HC12.flush();
  if (p_Store.Details == 2){
    if (sEcount < p_Store.maxEnds) {                                // ???
      writeReady();
      txtColour = orange; lnNumber = 31;                            //if (p_Store.Details == 2){                                // ???  Double detail?
      if (p_Store.isFinals 
          && sE_iter == 1){                                         // First end of FINALS
        writeArcher(p_Store.whichArcher, txtColour);
        delay(tick); //}}}}}}}}}}}}}}}}}}}}}}}
        sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 8, /*line=*/ lnNumber, "FINALS");
        delay(tick);
      }else{
        // sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 24, /*line=*/ lnNumber, 
        //             !countPractice ? (sE_iter%4 == 2 ? "C D" : "A B") : 
        //                              (sE_iter%2 == 0 ? "A B" : "C D"));
        sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 24, /*line=*/ lnNumber, 
                    (sE_iter%4 == 2 ? "C D" : "A B"));                                     
      }
    }
  }else {
    see(continueOn);
    if (continueOn) writeReadySet();
    else if (!(sEcount > p_Store.maxEnds)) writeReady() ;
    pauseMe(2000); //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<kill 
  }
}

/*
 *  Function to write READY on the screen
 */
void writeReady(void){ 
  clearMatrix(false);
  HC12.print(F("font 9\r"));    HC12.flush();
  sendSerialS( /*colour=*/ 2, /*column=*/ 0, /*line=*/ 15, 
              (!p_Store.isFlint || sEcount < 1) ? "   READY" :
                sEcount < 7  ? " COLLECT " : " ");
  const char* i0; const char* i1;
  i0 = (!p_Store.isFlint)? " " : 
       sEcount < 7 && countPractice == 0 ? "--> " :  "";
  if((p_Store.isFlint) && sEcount <= 5  && countPractice == 0 ){
    i1 = flint[sEcount];
  }else if((p_Store.isFlint) && sEcount > 5  && countPractice == 0 ){
    i1 = sEcount > 6 ? "" : flintWalk[sEcount-6];
  } else i1 = "";
  sendSerialS( /*colour=*/ orange, /*column=*/ 1, /*line=*/ 30, i0,i1);
}

void writeReadySet(void){
  clearMatrix(false);
  sendSerialS(2, 0, 15, "   READY");
  sendSerialS(2, 0, 29, "   SET:");
  sendNumber(2, 44 , 29, sEcount);
}

void score_Collect(bool redScreen){                                 // remote located function
  HC12.print(redScreen ? "^3" : "^2");                              // 2 == false 3 == true
  HC12.flush();
  pauseMe(30);
}

/*
 * Handle the bargraph count on the screen, for "walkup" secs countdown
 */
void doBarCount(uint8_t archerIndex, byte nID){                     // set to p_Store.whichArcher
  const char* i0; const char* i1;
  clearFromLine(5); 
  u8x8.draw2x2String(0, 6, ".WALKUP.");
  byte loc_Count = (p_Store.isFlint) && (sEcount > 7) ?             // If a Flint and also a Flint walkup round, 
                    20 :  p_Store.walkUp;                           // then 20s walkup, else 10 sec
  barWidth = 5; 
  rectWide = 49;
  lnNumber = 15;
  goWhistle(2);
  writeOLED_Data(archerIndex, nID);
  if (!p_Store.isFinals && 
      !p_Store.B_ScrCh  && 
      !countPractice    /*&& 
       sEcount >= 1*/)         {                                      // if not final/team and the count is >1
    HC12.print(F("font "));
    HC12.print((!p_Store.isFlint && !p_Store.isAlternating) ? 7 : 9 );   // if not a flint && not alternating
    HC12.print(F("\r"));
    printDebugLine(true, __LINE__, __NAME__);
    i0 = ((p_Store.isFlint) && sEcount > 7) ?                       // if Flint && >7
          loc_Count > 17 ? "ADVANCE ": "==>> "                      // "advance" for 3 sec, then arrows from 17sec
          : "END";                                                  // else END
    i1 = ((p_Store.isFlint) && sEcount > 7 && loc_Count <= 17 ) ? 
            flintWalk[sEcount-7] : ""; 
    sendSerialS(green /*colour(R1G2O3)=*/,
                ((!p_Store.isFlint && !p_Store.isAlternating) ? 17 : 2)/*column=*/, 
                 lnNumber, i0, i1);                                 // Write END:
    colNumber = (!p_Store.isFlint && p_Store.isAlternating) ?  38 : 43 ;
    HC12.flush();
    if (!(p_Store.isFlint  && sEcount > 7)) { 
      sendNumber(orange, colNumber, lnNumber, sEcount);             //If not (flint && count >7)
    }
  } else if (!p_Store.B_ScrCh && !countPractice && p_Store.isAlternating ){
    HC12.print(F("font 9\r")); 
    i0 = "ARCHER: ";
    i1 = (archerIndex == 1 ? "A" : "B");                            // Write Archer:
    sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, i0, i1);
    HC12.flush();
  } // else  run the screen selector to single screen after sending the holding time to the other);
  HC12.print(F("font 9\r"));
  doCountdownBar();
  clearMatrix(false);
}

void doCountdownBar(void){
  byte loc_Count = (p_Store.isFlint) && (sEcount > 7) ?             // If a Flint and also a Flint walkup round, 
      20 :  p_Store.walkUp;                                         // determine the starting count value
  byte n_Loc = loc_Count;                                           // for the actual count
  byte lnNumber = 30;
  int clockPulse;
  byte rectWide = 49;     
  writeRectangle(1, 0, lnNumber, rectWide, 13 );                    // rCol, rX0, rY0, rWid, rHi
  HC12.flush();
  writeStopwatch(n_Loc);
  sendNumber(red, 50, lnNumber, n_Loc--);                           // write the number and decrement n
  HC12.flush();
  bool flag = false;
  unsigned long secCount = millis();
  do{} while ((millis()-secCount) < 100); 
  do{                                                               // : run a smooth countdown bar
    clockPulse = (((millis() - secCount) %1000) / 100);
    switch (abs(clockPulse)) {
      
      case 2:
      case 4: 
      case 6:
          if (n_Loc <=9 ) {                                         // not for 20 to 10 (flint case)
            writeLine(3, rectWide, 18, rectWide, lnNumber);         // draw yel over red 1 column wide starting at rectwide, decrement
            HC12.flush();
            rectWide--;
        }    
        break;
      
      case 8:                                                       // getting to the end
        writeRectangle(0, 50, lnNumber, 15, 13 );                   // rCol, rX0, rY0, rWid, rHi
        HC12.flush();
        if (n_Loc <=9 ) {
          writeLine(3, rectWide, 18, rectWide, lnNumber);
          rectWide--;
          HC12.flush();
        }    
        break;
        
      case 0:
        sendNumber(n_Loc ? red : green, (n_Loc >= 10)? 50 : 54, lnNumber, n_Loc);  // if = 0 then green, otherwise red;  format shift inc for 20:10
        writeStopwatch(n_Loc);
        n_Loc--; 
        HC12.flush();
        if (n_Loc >=10) break;
        writeLine(3, rectWide, 18, rectWide, lnNumber);
        HC12.flush();
        rectWide--;
      break;
    }
    if (n_Loc == 16 && !flag) {                                     // flint inclusion for "proceed to"  instr.
      sendSerialS(green, 2, 15, "==>> ", flintWalk[sEcount-7]);
      HC12.flush();
      flag = true;
    }
    if (readButtonNoDelay(button1Pin) == 1 ) {                      // test for proceed button - a debug function really
      goGreenZero();
      return;
    }
  } while ((millis() - secCount) < (1000UL * (loc_Count)));
  goGreenZero();
}

void goGreenZero(void){
  writeRectangle(0, 0, 30, 64, 13);                                 // blanking
  //HC12.flush();
  sendNumber(green, 54, 30, 0);                                     // send the zero
  //HC12.flush();
  writeRectangle(green, 0, 30, 50, 13);                             // rCol, rX0, rY0, rWid, rHi
  //HC12.flush();
  goWhistle(1);
  HC12.flush();
  delay(800);
}

/*
   These four functions are for writing the
   Detail identifier during the count process
*/

void sendDetail(bool cdHigh) {
  clearMatrix(false);
  if (p_Store.Details == 2) {                                       // |Double detail, alternating?
    if (!shootDetail && (sE_iter % 4 == 1)) {                       // |
      writeA_B(false);                                              // |
    } else {                                                        // |
      if (shootDetail  && (sE_iter % 4 == 2)) {                     // |if so, set the correct
        writeC_D(false);                                            // |
      } else {                                                      // |
        if (!shootDetail && (sE_iter % 4 == 3)) {                   // |detail identifier on the screen
          /*countPractice ? writeA_B(false) :*/ writeC_D(cdHigh);       // |
        } else {                                                    // |
          /*countPractice ? writeC_D(false) :*/ writeA_B(false);        // |
        }
      }
    }
  } HC12.flush();
}

void writeA_B(bool cdHigh) {
  HC12.print(F("font 9\r"));                                        // font 12 high double
  HC12.flush();
  lnNumber = 12;                                                    // Write A & B
  sendSerialS(2, 1,  lnNumber, "A");
  sendSerialS(2, 56, lnNumber, "B");
  HC12.flush();
}

void writeC_D(bool cdHigh) {
  HC12.print(F("font 9\r"));
  HC12.flush();
  lnNumber = (cdHigh ? 12 : 30);
  sendSerialS(2,  1, lnNumber, "C");                                // Write C & D
  sendSerialS(2, 56, lnNumber, "D");
  HC12.flush();
}

void writeArcher(uint8_t which, byte archerColour) {
  HC12.print(F("font 9\r"));    HC12.flush();
  switch (which) {
    case 0:                                                         // nothing to see here
      break;
                                                                    // sendSerialS(/*colour(R1G2O3)=*/, /*column=*/, /*line=*/, i0);
    case 1:     
      sendSerialS(archerColour,  0, 22, "A<<");                     //  A<<
      break;

    case 2:
      sendSerialS(archerColour, 40, 22, ">>B");
      break;
  } 
}


/*
 * Font   ID    Description 
          1     5 pixel high narrow
          2     5 pixel high normal
          3     7 pixel high normal 
          4     8 pixel high fixed pitch 
          5     8 pixel high normal 
          6    10 pixel high narrow 
          7    10 pixel high normal 
          8    10 pixel high double
          9    12 pixel high double 
          10   16 pixel high double
          11   16 pixel high triple 
          12   24 pixel high triple 
          13      Bignum font* 
          14   32 pixel high Tahoma** 
          15      Reserved 
          16   32 pixel high Segoe
          17   10 pixel high Arial
          18      5x7   fixed
 */

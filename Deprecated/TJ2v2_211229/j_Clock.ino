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
*/

/*
 * Clock function that handles the screen formatting etc
 * goClock
 * sendNumber
 * goBlanking
 * sendNumber
 * handleCount
 * sendSerialS
 * writeHalt
 * writeReady
 * writeScoring
 * doBarCount
 * doCountdownBar
 */
void goClock(uint8_t offSet){                                       // Setup for formatting the main count numbers

switch(n_Count ){
  case 0 ... 9:
    if (n_Count == 9) goBlanking(tempOffset);
    txtColour = orange;
    colNumber = (paramStore.isFinals  ? 11 : 15) + offSet; 
    lnNumber = 30;                                                  //Orange, 1 digit
    break;
  case 10 ... 19:
    if (n_Count == 19) goBlanking(tempOffset);
    txtColour = orange;
    colNumber = (paramStore.isFinals  ? 2 : 4) + offSet;
    lnNumber = 30;                                                  //Orange, 2 digits
    tempOffset = (paramStore.isFinals  ? 2 : 4) + offSet;
    break;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
  case 20 ... 29:
    txtColour = orange;
    colNumber = (paramStore.isFinals  ? 4 : 6) + offSet;
    lnNumber = 30;                                                  //Green, 2 digits
    tempOffset = (paramStore.isFinals  ? 4 : 6) + offSet;
    break;
  case 30 ... 99:
    if (n_Count == 99) goBlanking(tempOffset);
    txtColour = green; colNumber = 6 + offSet; lnNumber = 30;       //Green, 3 digits
    //tempOffset = 6 + offSet;
    break;
  case 100 ... 199:
    if (n_Count == 199) goBlanking(tempOffset);
    txtColour = green; colNumber = -4 + offSet; lnNumber = 30;      //Green, 3 digits
    tempOffset = -4 + offSet;
    break;
  default:
    txtColour = green; colNumber = -2 + offSet; lnNumber = 30;      //Green, 3digits
    tempOffset = -2 + offSet;
    break;     
  }
}

 
/*
 * Function to output the actual clock output to the screen
 */
void sendNumber(int tColr, int cNum, int lnNum, int digits) {
  HC12.printf(
    F("text %u %u %u \"%u\"\rpaint\r"),
      tColr, cNum, lnNum, digits);
}

void goBlanking(uint8_t tOffset){                                 // writes a blanking frame on switch from 
  txtColour = 0; colNumber = tOffset; lnNumber = 30;              // 3 digits to 2 digits etc; No colour (0)         
  sendNumber(txtColour, colNumber, lnNumber, n_Count + 1);        // overwrite with last larger digit
}

/*
 * ===========================================================================================
 * ___________________________________________________________________________________________
 * Handle the count using millis() for accuracy.
 * We take secCount (set at beginning of each round)
 * and use it to find the point where millis() crosses a 
 * second (defined in tick = 1000)
 */
bool handleCount(unsigned long secCount){
  while ((millis() - secCount) % tick > 0){  // apply a 'fuzzy' offset of 2 for Uno
    if (readButtonNoDelay(button1Pin) == 1 ) {
      pauseMe(175);
      n_Count = 0;
      return false;
    }
  }
  return true;
}


/*
 * Function to send serial strings with formatting applied
 */
void sendSerialS(uint8_t txtColour1, uint8_t colNumber1, uint8_t lnNumber1, const char* i0, const char* i1, const char* i2){
  
  HC12.printf(
      F("text %u %u %u \"%s%s%s\"\rpaint\r"), 
      txtColour1, colNumber1, lnNumber1, 
      i0 != nullptr ? i0 : "", 
      i1 != nullptr ? i1 : "", 
      i2 != nullptr ? i2 : ""
      );  
  HC12.flush();
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
  clearFromLine(5); 
  u8x8.draw2x2String(0, 6, "..HALT..");
  HC12.print(F("font 11\r"));                         //Change font for the text msg
  
  HC12.print(F("rect 1 0 32 64 32\r")); 
  HC12.print(F("rect 0 0 25 64 18\r"));   
  lnNumber = 24;                                      // Print text Centre and in RED  
  sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 2, /*line=*/ lnNumber, "H A L T");
  goWhistle(3);
  pauseMe(5*tick);
  clearMatrix();  

  HC12.print(F("font 9\r"));    HC12.flush();
                                               
  if (paramStore.Details == 2){
   // printDebugLine(__LINE__, __NAME__);
    
    
    if (sEcount < paramStore.maxEnds) {                   // ???
      writeReady();

      txtColour = orange; lnNumber = 31; 
        //if (paramStore.Details == 2){                           // ???  Double detail?
      if (paramStore.isFinals 
          && sE_iter == 1){                                   // First end of FINALS
        writeArcher(paramStore.whichArcher, txtColour);
        delay(tick); //}}}}}}}}}}}}}}}}}}}}}}}
        sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 8, /*line=*/ lnNumber, "FINALS");
        delay(tick);
      }else{
        sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 24, /*line=*/ lnNumber, 
                    !countPractice ? (sE_iter%4 == 2 ? "C D" : "A B") : 
                                     (sE_iter%2 == 0 ? "A B" : "C D"));
        printDebugLine(__LINE__, __NAME__);           
      }
    }
    
//    if (sEcount < paramStore.maxEnds) writeReady();
//    txtColour = orange; lnNumber = 31; 
//    //if (paramStore.Details == 2){                           // ???  Double detail?
//      if (paramStore.isFinals 
//          && sE_iter == 1){                                   // First end of FINALS
//        writeArcher(paramStore.whichArcher, txtColour);
//        delay(tick); //}}}}}}}}}}}}}}}}}}}}}}}
//        sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 8, /*line=*/ lnNumber, "FINALS");
//        delay(tick);
//      }else{
//        if (sEcount < paramStore.maxEnds) sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 24, /*line=*/ lnNumber, 
//                  //sE_iter%4 == 2 ?  (countPractice ? "A B" : "C D") : (countPractice  ? "C D" : "A B"));
//                  !countPractice ? (sE_iter%4 == 2 ? "C D" : "A B") : (sE_iter%2 == 0 ? "A B" : "C D"));
//        printDebugLine(__LINE__, __NAME__);           
//      }               
    //} 
  }else {
    printDebugLine(__LINE__, __NAME__);
    if (continueOn) writeScoring();
    else if (!(sEcount > paramStore.maxEnds)) writeReady() ; 
  }
}

/*
 *  Function to write READY on the screen
 */
void writeReady(void){ 
  clearMatrix();
  //if (!continueOn) {
    HC12.print(F("font 9\r"));    HC12.flush();
    sendSerialS( /*colour=*/ 2, /*column=*/ 0, /*line=*/ 15, 
                (!paramStore.isFlint || sEcount < 1) ? "   READY" :
                  sEcount < 7  ? " COLLECT " : " ");
    const char* i0; const char* i1;
    i0 = (!paramStore.isFlint)? " " : 
         sEcount < 7 && countPractice == 0 ? "--> " :  "";
  
  
  
    if((paramStore.isFlint) && sEcount <= 5  && countPractice == 0 ){
      i1 = flint[sEcount];
      
    }else if((paramStore.isFlint) && sEcount > 5  && countPractice == 0 ){
      i1 = sEcount > 6 ? "" : flintWalk[sEcount-6];
    }
    else i1 = "";
    sendSerialS( /*colour=*/ orange, /*column=*/ 1, /*line=*/ 30, i0,i1);
  //}          
}

void writeScoring(void){
  writeRectangle( 1, 0, 31, 64, 32);
  goWhistle(3);
  pauseMe(3*tick);
  clearMatrix();
  HC12.print(F("font 9\r"));   
  HC12.flush();
  sendSerialS(2, 10, 15, "SCORE +");
  
  sendSerialS(2, 5, 29, "COLLECT");
#ifdef DEBUG
  pauseMe(3000); 
#else
  pauseMe(10000);
#endif
  clearMatrix();
  sendSerialS(2, 0, 15, "   READY");
  sendSerialS(2, 0, 29, "   SET:");
  sendNumber(2, 44 , 29, sEcount);
  
}




/*
 * Handle the bargraph count on the screen, for "walkup" secs countdown
 */
void doBarCount(uint8_t archerIndex){                                           //set to paramStore.whichArcher
  const char* i0; const char* i1;
  clearFromLine(5); 
  u8x8.draw2x2String(0, 6, ".WALKUP.");
  byte loc_Count = (paramStore.isFlint) && (sEcount > 7) ?                      // If a Flint and also a Flint walkup round, 
                    20 :  paramStore.walkUp;                                    // then 20s walkup, else 10 sec
  barWidth = 5;
  rectWide = 49;
  lnNumber = 15;
  goWhistle(2);
  writeOLED_Data(archerIndex);

  if (!paramStore.isFinals && countPractice == 0 && sEcount > 1){  // if not a final and the count is >1
    HC12.print(F("font "));
    HC12.print((!paramStore.isFlint && !paramStore.isAlternating) ? 7 : 9 );  // if not a flint && not alternating
    HC12.print(F("\r"));
    i0 = ((paramStore.isFlint) && sEcount > 7) ?                              // if Flint && >7
          loc_Count > 17 ? "ADVANCE ": "==>> "                                  // "advance" for 3 sec, then arrows from 17sec
          : "END";                                                            // else END
    i1 = ((paramStore.isFlint) && sEcount > 7 && loc_Count <= 17 ) ? 
            flintWalk[sEcount-7] : ""; 
    sendSerialS(green /*colour(R1G2O3)=*/,
                ((!paramStore.isFlint && !paramStore.isAlternating) ? 17 : 2)/*column=*/, 
                 lnNumber, i0, i1);                                           // Write END:
                 
    colNumber = (!paramStore.isFlint && paramStore.isAlternating) ?  38 : 43 ;
    HC12.flush();
    if (!(paramStore.isFlint  && sEcount > 7)) { 
      sendNumber(orange, colNumber, lnNumber, sEcount);     //If not (flint && count >7)
    }
  } else if (countPractice == 0 && paramStore.isAlternating ){
    HC12.print(F("font 9\r")); 
    i0 = "ARCHER: ";
    i1 = (archerIndex == 1 ? "A" : "B");                                      // Write Archer:
    sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, i0, i1);
    HC12.flush();
  }
  HC12.print(F("font 9\r"));
  doCountdownBar1();

  clearMatrix();
}


void doCountdownBar1(void){
  
  byte loc_Count = (paramStore.isFlint) && (sEcount > 7) ?                             // If a Flint and also a Flint walkup round, 
      20 :  paramStore.walkUp;
  byte n_Loc = loc_Count;
  byte lnNumber = 30;
  int clockPulse;
  byte rectWide = 49;     
  writeRectangle(1, 0, lnNumber, rectWide, 13 );                  // rCol, rX0, rY0, rWid, rHi
  writeStopwatch(n_Loc);
  sendNumber(red, 50, lnNumber, n_Loc--);          // write the number and decrement n
  HC12.flush();
  bool flag = false;
  unsigned long secCount = millis();
  do{} while ((millis()-secCount) < 100); 
  do{
    clockPulse = (((millis() - secCount) %1000) / 100);
    switch (abs(clockPulse)) {
      
      case 2:
      case 4: 
      case 6:
          if (n_Loc <=9 ) {
            writeLine(3, rectWide, 18, rectWide, lnNumber);         //  draw yel over red 1 column wide starting at rectwide, decrement
            HC12.flush();
            rectWide--;
        }    
        break;
      
      case 8:
        writeRectangle(0, 50, lnNumber, 15, 13 );                     // rCol, rX0, rY0, rWid, rHi
        HC12.flush();
        if (n_Loc <=9 ) {
          writeLine(3, rectWide, 18, rectWide, lnNumber);
          rectWide--;
          HC12.flush();
        }    
        break;
        
      case 0:
      sendNumber(n_Loc ? red : green, (n_Loc >= 10)? 50 : 54, lnNumber, n_Loc);
      writeStopwatch(n_Loc);
      n_Loc--; 
      HC12.flush();
      if (n_Loc >=10) break;
      writeLine(3, rectWide, 18, rectWide, lnNumber);
      rectWide--;
      break;
    }
    if (n_Loc == 16 && !flag) {
      sendSerialS(green , 2, 15, "==>> ", flintWalk[sEcount-7]);
      HC12.flush();
      flag = true;
    }
    if (readButtonNoDelay(button1Pin) == 1 ) {                // test for proceed button - a debug function really
      goGreenZero();
      return;
    }
  } while ((millis() - secCount) < (1000UL * (loc_Count)));
  goGreenZero();
}

void goGreenZero(void){
  writeRectangle(0, 0, 30, 64, 13);                         // blanking
  //HC12.flush();
  sendNumber(green, 54, 30, 0);                             // send the zero
  //HC12.flush();
  writeRectangle(green, 0, 30, 50, 13);                     // rCol, rX0, rY0, rWid, rHi
  //HC12.flush();
  goWhistle(1);
  HC12.flush();
  delay(800);
}



//int doCountdownBar(int n_Loc, int rectWide, unsigned long secCount){      // use int& to write to the variable location
//  
//  do{
//    txtColour = n_Loc  ? red : green;                               // Numbers are red until zero reached
//    lnNumber = 30;                                                  // position count from 10
//    colNumber = (n_Loc  >= 10)? 50 : 54;                            // allocate space for double (else) single 'End' digit
//    int barStep = 0;                                                // Set 1/2 sec toggle for smoothing bar animation
//    int stripReducer = 5;
//    byte rWidth;   
//    bool wFlag = true; 
//    
//    goEmergencyButton(3);
//    if (!startOver){
//      if (reStartEnd) {
//        n_Loc  = ((paramStore.isFlint) && (sEcount > 7)) ?            // If a Flint and also a Flint walkup round, 
//        20 :  paramStore.walkUp;                                    // then 20s walkup, else 10 sec
//        rectWide = 49;
//        reStartEnd = false;
//        return n_Loc  ;
//      }
//      rWidth = ((n_Loc  < 11) ? (n_Loc * 5) : rectWide);          // only decrement barwidth if count reaches below 11  ** 49
//      writeRectangle(1, 0, lnNumber, rWidth, 13 );                  // rCol, rX0, rY0, rWid, rHi 
//      
//                                                                    // blank the number field, red bar 50 wide
//      //do {} while ((millis() - secCount) % 1000UL > 0);
//      writeRectangle(0, 50, lnNumber, 15, 13 );                     // rCol, rX0, rY0, rWid, rHi                                                              
//      sendNumber(txtColour, colNumber, lnNumber, n_Loc--);          // write the number and decrement n
//      if (n_Loc == 17) {
//        sendSerialS(green , 2, 15, "==>> ", flintWalk[sEcount-7]);
//      }
//      do {} while ((millis() - secCount) % 1000UL > 0);
//      
//      while (barStep < stripReducer){
//        if (n_Loc  < 0) {
//          if (wFlag) {
//            goWhistle(1);                                           // sound the whistle as last sec commences
//            wFlag = !wFlag;
//          }
//          writeRectangle(2, 0, lnNumber, 50, 13 );                  // rCol, rX0, rY0, rWid, rHi; draw the bar, in green
//          clearFromLine(6);
//        } else {
//          if (n_Loc  <= 9) {
//            writeLine(3, rectWide, 18, rectWide, lnNumber);         //  draw yel over red 1 column wide starting at rectwide, decrement
//            rectWide--;
//            barStep++;
//          }
//          do{
//            if (readButtonNoDelay(button1Pin) == 1 ) {                // test for proceed button - a debug function really
//              rectWide = 0;
//              clearFromLine(6);
//              pauseMe(175);
//              n_Loc = 0;
//            }           
//          } while ((millis() - secCount) % 245UL > 2 );
//        }
//        
//      } 
//    } else {                                                        // else kill the countdown
//      clearMatrix();
//      zeroSettings();
//      sEcount = (paramStore.maxEnds);
//      n_Loc -- ;
//    }
//  }  while (n_Loc > 0 && !startOver);
//  return (n_Loc);
//}

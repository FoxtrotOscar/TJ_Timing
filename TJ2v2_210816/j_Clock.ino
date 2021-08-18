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
 * goBlanking
 * sendNumber
 * handleCount
 * sendSerialS
 * writeHalt
 * writeReady
 * doBarCount
 * doCountdownBar
 */
void goClock(uint8_t offSet){

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
void sendNumber(int digits) {
  HC12.print(F("text "));
  HC12.print(txtColour);
  HC12.print(F(" "));
  HC12.print(colNumber);          // x pos for text
  HC12.print(F(" "));
  HC12.print(lnNumber);           // y pos
  HC12.print(F(" "));
  HC12.print('"');
  HC12.print(digits);
  HC12.print('"');
  HC12.print(F("\r"));
  HC12.print(F("paint\r"));    HC12.flush();
}

void goBlanking(uint8_t tOffset){                                 // writes a blanking frame on switch from 
  txtColour = 0; colNumber = tOffset; lnNumber = 30;              // 3 digits to 2 digits etc; No colour (0)         
  sendNumber(n_Count + 1);                                        // overwrite with last larger digit
}

/*
 * ===========================================================================================
 * ___________________________________________________________________________________________
 * Handle the count using millis() for accuracy.
 * We take secCount (set at beginning of each round)
 * and use it to find the point where millis() crosses a 
 * second (defined in tick = 1000)
 */
void handleCount(unsigned long secCount){
  
  while ((millis() - secCount) % tick > 0){  // apply a 'fuzzy' offset of 2 for Uno
    if (readButtonNoDelay(button1Pin) == 1 ) {
      clearFromLine(6);
      delay(175);
      n_Count = 0;
    }
  }
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

void sendSerialS(uint8_t txtColour1, uint8_t colNumber1, uint8_t lnNumber1, const char* i0, const char* i1) {
  return sendSerialS(txtColour1, colNumber1, lnNumber1, i0, i1, nullptr); // this one calls the big one above 
}

void sendSerialS(uint8_t txtColour1, uint8_t colNumber1, uint8_t lnNumber1, const char* i0) {
  return sendSerialS(txtColour1, colNumber1, lnNumber1, i0, nullptr, nullptr); // same 
}

/*
 * When the Detail(s) have finished, for arrow collection
 */
void writeHalt(){
  clearFromLine(5); 
  u8x8.draw2x2String(0, 6, "..HALT..");
  HC12.print(F("font 11\r"));                         //Change font for the text msg
  lnNumber = 24;  // Print text Centre and in RED  
  HC12.print(F("rect 1 0 32 64 32\r"));    HC12.flush();
  HC12.print(F("rect 0 0 25 64 18\r"));    HC12.flush();
  HC12.print(F("paint\r"));    HC12.flush();
  sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 2, /*line=*/ lnNumber, "H A L T");
  goWhistle(3);
  int delayNo = 5;

  delay(delayNo*tick);
  clearMatrix();  

  HC12.print(F("font 9\r"));    HC12.flush();
                                                
  if (paramStore.Details == 2){
    writeReady();
      txtColour = orange; lnNumber = 31; 
      if (paramStore.Details == 2){                           // Double detail?
        if (paramStore.isFinals 
          && sE_iter == 1){                                   // First end of FINALS
          writeArcher(paramStore.whichArcher);
          sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 8, /*line=*/ lnNumber, "FINALS");
        }else{
          sendSerialS( /*colour(R1G2O3)=*/ 3, /*column=*/ 24, /*line=*/ lnNumber, 
              (sE_iter%4 == 2 ? "C D" : "A B"));  
      }
    }
  } else {
    writeReady();
  }
}

/*
 *  Function to write READY on the screen
 */
void writeReady(void){ 
  clearMatrix();
  HC12.print(F("font 9\r"));    HC12.flush();
  sendSerialS( /*colour=*/ 2, /*column=*/ 0, /*line=*/ 15, 
              (paramStore.notFlint || sEcount < 1) ? "   READY" :
                sEcount < 7  ? " COLLECT " : " ");
  const char* i0; const char* i1;
  i0 = (paramStore.notFlint)? " " : 
       sEcount < 7 && countPractice == 0 ? "--> " :  "";



  if((!paramStore.notFlint) && sEcount <= 5  && countPractice == 0 ){
    i1 = flint[sEcount];
    
  }else if((!paramStore.notFlint) && sEcount > 5  && countPractice == 0 ){
    i1 = sEcount > 6 ? "" : flintWalk[sEcount-6];
  }
  else i1 = "";
  sendSerialS( /*colour=*/ orange, /*column=*/ 1, /*line=*/ 30, i0,i1);          
  
}


/*
 * Handle the bargraph count on the screen, for "walkup" secs countdown
 */
void doBarCount(uint8_t archerIndex){                                           //set to paramStore.whichArcher
  const char* i0; const char* i1;
  clearFromLine(5); 
  u8x8.draw2x2String(0, 6, ".WALKUP.");
  uint8_t temp = n_Count ;
  n_Count = (paramStore.notFlint == 0) && (sEcount > 7) ?                       // If a Flint and also a Flint walkup round, 
      20 :  paramStore.walkUp;                                                  // then 20s walkup, else 10 sec
  barWidth = 5;
  rectWide = 49;
  goWhistle(2);

  while (n_Count >= 0 && !startOver){                                           // do bar countdown  
    writeOLED_Data(archerIndex);

    if (!paramStore.isFinals && countPractice == 0 && sEcount > 1){             // if not a final and the count is >1
      lnNumber = 15;
      HC12.print(F("font "));
      HC12.print((paramStore.notFlint && !paramStore.isAlternating) ? 7 : 9 );  // if not a flint && not alternating
      HC12.print(F("\r"));    HC12.flush();
      i0 = ((!paramStore.notFlint) && sEcount > 7) ?                            // if Flint && >7
            n_Count > 17 ? "ADVANCE ": "==>> "                                  // "advance" for 3 sec, then arrows from 17sec
            : "END";                                                            // else END
      i1 = ((!paramStore.notFlint) && sEcount > 7 && n_Count <= 17 ) ? 
              flintWalk[sEcount-7] : "";                                        // if flint && 3 secs of walkup passed
      sendSerialS(2 /*colour(R1G2O3)=*/,
                  ((paramStore.notFlint && !paramStore.isAlternating) ? 17 : 2)/*column=*/, 
                   lnNumber, i0, i1);                                           // Write END:
      colNumber = (paramStore.notFlint && paramStore.isAlternating) ?  38 : 43 ;
      txtColour = orange;

      if (!(paramStore.notFlint == 0 && sEcount > 7))  sendNumber(sEcount);     //If not (normal && count >7)
    } else if (countPractice == 0 && paramStore.isAlternating ){
      lnNumber = 15;
      HC12.print(F("font 9\r"));    HC12.flush();
      i0 = "ARCHER: ";
      i1 = (archerIndex == 1 ? "A" : "B");                                      // Write Archer:
      sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, i0, i1); 
    }
    n_Count = doCountdownBar(n_Count , rectWide, barWidth);                     // returns with decremented n_Count 
  }
  
  clearMatrix();
  n_Count = temp;
}


int doCountdownBar(int n_Loc, int& rectWide, int& barWidth){    // use int& to write to the variable location
  txtColour = n_Loc  ? red : green;                             // Numbers are red until zero reached
  lnNumber = 30;                                                // position count from 10
  colNumber = (n_Loc  >= 10)? 50 : 54;                          // allocate space for double (else) single 'End' digit
  int barStep = 0;                                              // Set 1/2 sec toggle for smoothing bar animation
  int stripReducer = 5;
  unsigned long secCount = millis(); 
  do {} while ((millis() - secCount) % (tick) > 2);             // initialise timer to a start-point++++++++++++++++++++++++++++++++++
  HC12.print(F("font 9\r"));    HC12.flush();                   // set the font
  goEmergencyButton(3);
  if (!startOver){
    if (reStartEnd) {
      n_Loc  = (paramStore.notFlint == 0) && (sEcount > 7) ?    // If a Flint and also a Flint walkup round, 
      20 :  paramStore.walkUp;                                  // then 20s walkup, else 10 sec
      rectWide = 49;
      reStartEnd = false;
      return n_Loc  ;
    }
    HC12.print(F("rect 1 0 "));                                 // colour = red
    HC12.print(lnNumber);
    HC12.print(F(" "));
    HC12.print((n_Loc  < 11) ? (n_Loc  *5-1) : rectWide);       // only decrement barwidth if count reaches below 11  ** 49
    HC12.print(F(" 13 \r"));                                    // draw the red bar 50 wide ++++++++++++++++++++++++
    HC12.print(F("paint\r"));    HC12.flush();                  // rect colour x(0) y(30) width(50) height(13)
    HC12.print(F("rect 0 50 "));                                // colour = 0
    HC12.print(lnNumber);
    HC12.print(F(" 15 13\r"));                                  // blank the number field
    HC12.print(F("paint\r"));    HC12.flush();
    sendNumber(n_Loc  --);                                      // write the number and decrement n
    do{
      if (n_Loc  < 0) {
      HC12.print(F("rect 2 0 "));
      HC12.print(lnNumber);
      HC12.print(F(" 50 13\r"));                                // draw the bar, in green  (2)
      HC12.print(F("paint\r"));    HC12.flush();
      clearFromLine(6);
      } else {
        if (n_Loc  <= 9) {
          HC12.print(F("line 3 "));                             //  line colour x1() y1() x2() y2()
          HC12.print(rectWide);                                 //  . . . colour(R1G2O3)
          HC12.print(F(" 18 "));                                //  draw the yellow strip over the red one column wide
          HC12.print(rectWide--);                               //  starting at rectwide and decrement that position
          HC12.print(F(" "));
          HC12.print(lnNumber);
          HC12.print(F("\r"));
          HC12.print(F("paint\r"));    HC12.flush();
        } else {
          HC12.print(F("line 1 "));                            //  for n_Loc values above 10
          HC12.print(rectWide);                                
          HC12.print(F(" 18 "));                               //  draw the yellow strip over the red one column wide
          HC12.print(rectWide);                                //  starting at rectwide and decrement that position
          HC12.print(F(" "));
          HC12.print(lnNumber);
          HC12.print(F("\r"));
          HC12.print(F("paint\r"));    HC12.flush();
        }
      }
      do {
        if (readButtonNoDelay(button1Pin) == 1 ) {              // test for proceed button - a debug function really
          rectWide = 0;
          clearFromLine(6);
          delay(175);
          n_Loc = 0;
        }           
      } while ((millis() - secCount) % (tick/stripReducer) > 0);
    } while (++barStep < stripReducer);
  } else {                                                       // else kill the countdown
    clearMatrix();
    zeroSettings();
    sEcount = (paramStore.maxEnds);
    n_Loc -- ;
  }
  return (n_Loc);
}

/* 
 *  Splashscreen of Logo "Time Judge"
*/
void writeSplash(bool scrolling){
  for (uint16_t br = 0; br <= sBright; br += 17){                           // fade increment [10]
    unsigned long timer = millis();
    MATRIXSER.printf(
          F("brightness %u\rpaint\r"),                                     //  | Fade in "Time"
            br);
    MATRIXSER.print(F("font 16\r"));    HC12.flush();
    sendSerialS( green, /*column=*/ 2, /*line=*/ 24, "Time");
    do{} while (millis() - timer < 2UL);                                  // allow serial traffic time (15UL)
    }
  const char scrollChar[] = "JUDGE";
  MATRIXSER.print(F("font 9\r"));    
  pauseMe(2);
  if (scrolling){
    pauseMe(2);
    sendScrollW(  /*speed=*/  5,                                           // 3 - 15
                  /*loop=*/   0,
                  /*wiggle=*/ 3,
                  /*colour=*/ orange,
                  /*column=*/ 4, 
                  /*line=*/   30,
                  /*window=*/ 63, 
                            scrollChar );                                   // animate of JUDGE
  } else {
    for (uint8_t iter = 0; iter < strlen(scrollChar); iter ++) {
      sendChar(orange, (4 + 12*iter), 30, scrollChar[iter]);
      pauseMe(9);
    }
  }
  pauseMe(tock);
}

void sendChar(byte chCol, byte chX, byte chY, byte chChar){
  HC12.printf(
            F("text %u %u %u \"%c\"\rpaint\r"),
              chCol, chX, chY, chChar);
}



/*
 * Sends a scroll command to screen with text running through 
 * a window defined by bottom left corner @ x1,y1 and window1
 * wide and as tall as the current font height. If wiggle >0
 * then letters will move wiggle pixels above and below the 
 * baseline set.
 */

 
void sendScrollW( uint16_t      scrollSpeed, 
                  uint8_t       scrollLoop, 
                  uint8_t       scrollWiggle, 
                  uint8_t       txtColour1, 
                  uint8_t       x1, 
                  uint8_t       y1, 
                  int           window1,
                  const char    scrollChar[]  ){
  #ifdef DEBUG
  return;
  #endif
  //uint16_t delVal = scrollSpeed;

  MATRIXSER.printf(
    F("scrollspeed %u \r scrollloop %u \r scrollwiggle %u \r"),
      scrollSpeed, scrollLoop, scrollWiggle);
  HC12.flush();  pauseMe(tock);    
  for (uint8_t iter = 0; iter < strlen(scrollChar); iter ++){
    unsigned long timer = millis();
    MATRIXSER.print(F("font 17\r"));    
    pauseMe(20);
    MATRIXSER.printf(
          F("scroll %u %u %u %u \"%c\"\rpaint\r"),
              txtColour1, (x1 + 12*iter + 1/*+4*/), y1, window1, scrollChar[iter]);
    do{} while (millis() - timer < (scrollSpeed*140));    // 800UL
    MATRIXSER.print(F("font 9\r"));     //HC12.flush();  pauseMe(tock);              // Font for the static letter post-scroll
    sendChar(orange, (x1 + 12*iter), y1, scrollChar[iter]);
    pauseMe(2);
  }
  pauseMe(tick);
}



///*
// * Handle the bargraph count on the screen, for "walkup" secs countdown
// */
//void doBarCount(uint8_t archerIndex){                                           //set to paramStore.whichArcher
//  const char* i0; const char* i1;
//  clearFromLine(5); 
//  u8x8.draw2x2String(0, 6, ".WALKUP.");
//  uint8_t temp = n_Count ;
//  n_Count = (paramStore.isFlint) && (sEcount > 7) ?                             // If a Flint and also a Flint walkup round, 
//      20 :  paramStore.walkUp;                                                  // then 20s walkup, else 10 sec
//  barWidth = 5;
//  rectWide = 49;
//  goWhistle(2);
//
//  while (n_Count >= 0 && !startOver){                                           // do bar countdown  
//    writeOLED_Data(archerIndex);
//
//    if (!paramStore.isFinals && countPractice == 0 && sEcount > 1){             // if not a final and the count is >1
//      lnNumber = 15;
//      MATRIXSER.print(F("font "));
//      MATRIXSER.print((!paramStore.isFlint && !paramStore.isAlternating) ? 7 : 9 );  // if not a flint && not alternating
//      MATRIXSER.print(F("\r"));    //HC12.flush();// ?????????????????????????????????????????????
//      i0 = ((paramStore.isFlint) && sEcount > 7) ?                              // if Flint && >7
//            n_Count > 17 ? "ADVANCE ": "==>> "                                  // "advance" for 3 sec, then arrows from 17sec
//            : "END";                                                            // else END
//      i1 = ((paramStore.isFlint) && sEcount > 7 && n_Count <= 17 ) ? 
//              flintWalk[sEcount-7] : "";                                        // if flint && 3 secs of walkup passed
//      sendSerialS(2 /*colour(R1G2O3)=*/,
//                  ((!paramStore.isFlint && !paramStore.isAlternating) ? 17 : 2)/*column=*/, 
//                   lnNumber, i0, i1);                                           // Write END:
//      colNumber = (!paramStore.isFlint && paramStore.isAlternating) ?  38 : 43 ;
//      txtColour = orange;
//
//      if (!(paramStore.isFlint  && sEcount > 7))  sendNumber(txtColour, colNumber, lnNumber, sEcount);     //If not (normal && count >7)
//    } else if (countPractice == 0 && paramStore.isAlternating ){
//      lnNumber = 15;
//      MATRIXSER.print(F("font 9\r"));    //HC12.flush();// ?????????????????????????????????????????????
//      i0 = "ARCHER: ";
//      i1 = (archerIndex == 1 ? "A" : "B");                                      // Write Archer:
//      sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, i0, i1); 
//    }
//    n_Count = doCountdownBar(n_Count , rectWide, barWidth);                     // returns with decremented n_Count
//  }
//  clearMatrix();
//  n_Count = temp;
//}


//int doCountdownBar(int n_Loc, int& rectWide, int& barWidth){      // use int& to write to the variable location
//  txtColour = n_Loc  ? red : green;                               // Numbers are red until zero reached
//  lnNumber = 30;                                                  // position count from 10
//  colNumber = (n_Loc  >= 10)? 50 : 54;                            // allocate space for double (else) single 'End' digit
//  int barStep = 0;                                                // Set 1/2 sec toggle for smoothing bar animation
//  int stripReducer = 5;
//  byte rWidth;
//  bool wFlag = true;
//  unsigned long secCount = millis(); 
//  do {} while ((millis() - secCount) % (tick) > 2);               // initialise timer to a start-point
//  MATRIXSER.print(F("font 9\r"));                                      // set the font
//  goEmergencyButton(3);
//  if (!startOver){
//    if (reStartEnd) {
//      n_Loc  = (paramStore.isFlint) && (sEcount > 7) ?            // If a Flint and also a Flint walkup round, 
//      20 :  paramStore.walkUp;                                    // then 20s walkup, else 10 sec
//      rectWide = 49;
//      reStartEnd = false;
//      return n_Loc  ;
//    }
//    rWidth = ((n_Loc  < 11) ? (n_Loc  *5-1) : rectWide);          // only decrement barwidth if count reaches below 11  ** 49
//    
//    writeRectangle(1, 0, lnNumber, rWidth, 13 );                  // rCol, rX0, rY0, rWid, rHi 
//    writeRectangle(0, 50, lnNumber, 15, 13 );                     // rCol, rX0, rY0, rWid, rHi
//                                                                  // blank the number field, red bar 50 wide
//    sendNumber(txtColour, colNumber, lnNumber, n_Loc--);          // write the number and decrement n
//    do{
//      if (n_Loc  < 0) {
//
//        wFlag ? goWhistle(1) : wFlag = !wFlag;
//        writeRectangle(2, 0, lnNumber, 50, 13 );                  // rCol, rX0, rY0, rWid, rHi; draw the bar, in green
//
//        //clearFromLine(6);
//      } else {
//        if (n_Loc  <= 9) {
//          writeLine(3, rectWide, 18, rectWide, lnNumber);         //  draw yel over red 1 column wide starting at rectwide, decrement
//          rectWide--;
//        } else {
//          writeLine(1, rectWide, 18, rectWide, lnNumber);         //  for n_Loc values above 10
//        }
//      }
//      do {
//        if (readButtonNoDelay(button1Pin) == 1 ) {                // test for proceed button - a debug function really
//          rectWide = 0;
//          //clearFromLine(6);
//          pauseMe(175);
//          n_Loc = 0;
//        }           
//      } while (((millis() - secCount) % 20) > 0);// ????????????????????????????????????????????? [tick,500/sr]
//    } while (++barStep < stripReducer);
//  } else {                                                        // else kill the countdown
//    clearMatrix();
//    zeroSettings();
//    sEcount = (paramStore.maxEnds);
//    n_Loc -- ;
//  }
//  return (n_Loc);
//}

void writeLine(byte lCol, byte lX1, byte lY1, byte lX2, byte lY2) {
  HC12.printf(
    F("line %u %u %u %u %u \rpaint\r"),                         //  line colour, x1(), y1(), x2(), y2()
      lCol, lX1, lY1, lX2, lY2);
}

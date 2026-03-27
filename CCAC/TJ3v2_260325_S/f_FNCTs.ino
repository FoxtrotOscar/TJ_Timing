
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

  FUNCTIONS:
  56    writeInfoBigscreen
  129   goChooseArcher
  167   checkForShootoff
  190   writeShootOff
  212   clearMatrix
  221   writeSplash 
  238   sendScrollW
  267   goWhistle
  274   stopSign
  283   writeCircle
  289   writeRectangle
  296   writeLine
  302   redBorder
  316   sendChar
  323   sendScrollChar
  334   zeroSettings
  348   goReboot
  367   readButton
  374   readButtonNoDelay
  384   readButtons
  420   waitButton
  450   goEmergencyButton
  485   handleEmergencyRestart
  536   displayMenuPage
  548   command_ON
  554   pauseMe
  562   goPowerOff
    
 DEBUGGING   
    dispSrcFileDetails
    pgm_lastIndexOf
    debugEEPROM
    printDebugLine
    goDemoLoop
*/


 
 
/*
 * Write info regarding tournament setup
 */
void writeInfoBigscreen(void){
  clearMatrix(true);
  #ifdef DEBUG
  return;
  #endif
  pauseMe(tick);   
  HC12.print(F("font 1\r"));    HC12.flush();
  lnNumber = 5;
  
  if (p_Store.isFinals && !p_Store.isAlternating) {
    sendSerialS( green, /*column=*/ 0, /*line=*/ lnNumber, "MATCHPLAY");
  }

  else if (p_Store.isFinals && p_Store.isAlternating) {
    sendSerialS( red, /*column=*/ 0, /*line=*/ lnNumber, "FINALS");
  }

  else if (p_Store.teamPlay) {
    bool recurve = false;
    
    HC12.print(F("font 8\r"));    HC12.flush();
    if (p_Store.teamPlay == 1 || p_Store.teamPlay == 2) recurve = true;
    sendSerialS( green, 4, lnNumber +6, recurve ? "RECURVE" : "COMPOUND");
    sendSerialS( green, /*column=*/16, /*line=*/ lnNumber +17, "TEAM");
    if (p_Store.teamPlay == 2 || p_Store.teamPlay == 4) {
      sendSerialS( green, 13, lnNumber +28, "MIXED");
    }
    pauseMe(3*tick);
    clearMatrix(false);
    HC12.print(F("font 1\r"));    HC12.flush();  
    
  }
  else {  
    if (p_Store.isFlint){
      sendSerialS( red, /*column=*/ 0, /*line=*/ lnNumber, "FLINT Round"); 
    }else if (p_Store.ifaaIndoor){
      sendSerialS( red, /*column=*/ 0, /*line=*/ lnNumber, "IFAA Indoor");
    }else{
      sendSerialS( red, /*column=*/ 0, /*line=*/ lnNumber, 
      (p_Store.Details == 1 ? "Single " : "Double "), "Detail");               // either 1 OR 2
    }
  }
  pauseMe(tick);
  
  lnNumber += 7;                                                                  // Practice?
  sendSerialS( green, /*column=*/ 0, /*line=*/ lnNumber, "Practice");
  colNumber = 56;
  pauseMe(tick/4);
  sendNumber(orange, colNumber, lnNumber, (p_Store.maxPrac && (p_Store.isFlint || p_Store.ifaaIndoor)) ? 1 : p_Store.maxPrac);
  pauseMe(tick/4);
  lnNumber += 7;                                                                // End duration
  sendSerialS( green, /*column=*/ 0, /*line=*/ lnNumber, "END ","Time: ");
  colNumber = 48 +  ( startCounts[p_Store.startCountsIndex] < 10   ? 8 :     // Positioning the number on the matrix
                      startCounts[p_Store.startCountsIndex] < 20   ? 5 :
                      startCounts[p_Store.startCountsIndex] < 100  ? 3 : 
                      startCounts[p_Store.startCountsIndex] < 200  ? 0 : -2);
                      
  pauseMe(tick/4);
  sendNumber(orange, colNumber, lnNumber, startCounts[p_Store.startCountsIndex]);
  pauseMe(tick/4);  
  
  lnNumber += 7;  // Number of Ends
  sendSerialS( green, /*column=*/ 0, /*line=*/ lnNumber, "END ","Count: ");
  colNumber = 48 + (p_Store.maxEnds < 10 ? 8 : 
                    p_Store.maxEnds < 20 ? 5 : 3);                           // Positioning the number on the matrix
  pauseMe(tick/4);
  sendNumber(orange, colNumber, lnNumber, p_Store.maxEnds);
  pauseMe(3*tick);
  clearMatrix(false);
}

/*
 * Choose which archer is UP
 */

byte goChooseArcher(void){
  uint8_t menuArcher  = 1;
  bool setFlag        = false;
  for(;;) {  
    clearFromLine(1);
    disp.setCursor(0, 2);
    p_Store.teamPlay ? disp.print("Set 1st Team :-") : disp.print("Set 1st Player:-");
    disp.setCursor(5, 4);
    p_Store.teamPlay ? disp.print("Team   ") : disp.print("Archer ");
    disp.print(menuArcher == 1 ? "A" : "B");
    doButtonMenu();
    delay(500);                // debouncing                                           
    switch (waitButton()) {
    
      case BUTTON1: 
        p_Store.whichArcher = menuArcher;
        setFlag = true;
        break;
  
      case BUTTON2: 
      case BUTTON3: 
        menuArcher = menuArcher == 1 ? 2 : 1;
        break;
  
      case BUTTON4:
        menuArcher = p_Store.whichArcher;
        
        setFlag = true;
        break;
    }
    if (setFlag) {
      clearFromLine(1);
      break;
    }
  }
  delay(500);                // debouncing
  return menuArcher; 
}

bool checkForShootoff(void){
  bool flag = false;
  clearFromLine(1);
  disp.draw2x2String(0, 2, "SHOOTOFF");
  disp.draw2x2String(0, 4, "NEEDED?");
  disp.setCursor(0, 6);
  disp.inverse();
  disp.print("NO- END: BTN[1]");
  disp.setCursor(0, 7);
  disp.print("YES:     BTN[2]");
  disp.noInverse();
  for (;;) {
    byte btn = readButtons();
    if (btn == BUTTON1) {                         // no, so
        break;                                    // move on
    } else if (btn == BUTTON2) {
        flag = true;
        break;
    } 
  }
  return flag;                                                            // true if yes to SHOOTOFF
}

void writeShootOff(byte nID, bool AB){
  clearMatrix(false);
  HC12.print(F("font 9\r"));   
  sendSerialS(green, 0, 20, "SHOOTOFF");                                      // Bigscreen
  HC12.flush();
  if (AB) {
    nID == 1 ? setControlChannel(p_Store.B_ScrCh) : setControlChannel(p_Store.curChan);
    clearMatrix(false);
    HC12.print(F("font 9\r"));   
    sendSerialS(green, 0, 20, "SHOOTOFF");
    pauseMe(tick);
    HC12.flush();
    nID == 1 ? setControlChannel(p_Store.curChan) : setControlChannel(p_Store.B_ScrCh);
  }
  clearFromLine(4);
  disp.draw2x2String(6, 4, "OK");                                         // acknowledge on controller                
}

/*
 * Wipes the Matrix clear
 */

void clearMatrix(bool scrollOn){
  scrollOn ? HC12.print(F("$1")) : HC12.print(F("$0"));
  pauseMe(scrollOn ? tick : 2*tock);
} 


/* 
 *  Splashscreen of Logo "Time Judge"
*/
void writeSplash(bool scrolling){ 
  clearMatrix(false);
  #ifdef DEBUG
    HC12.print(F("font 8\r"));    HC12.flush();
    sendSerialS( green, /*column=*/ 2, /*line=*/ 24, "[ SPLASH ]");
  return;
  #endif
  HC12.print(scrolling ? "^1" : "^0");                                                         // Localsed to Matrix; send ^0 (false) or ^1 (true)
  pauseMe(scrolling ? 5*tick : 3*tick);
}

/*
 * Sends a scroll command to screen with text running through a window defined by bottom left corner @ x1,y1 and window1 wide
 * and as tall as the current font height. If wiggle >0 then letters will move wiggle pixels above and below the baseline set.
 */

 
void sendScrollW( uint16_t      scrollSpeed, 
                  uint8_t       scrollLoop, 
                  uint8_t       scrollWiggle, 
                  uint8_t       txtColour1, 
                  uint8_t       x1, 
                  uint8_t       y1, 
                  int           window1,
                  const char    scrollChar[]  ){

  HC12.printf(
    F("scrollspeed %u \r scrollloop %u \r scrollwiggle %u \r"),
      scrollSpeed, scrollLoop, scrollWiggle);
  HC12.flush();  pauseMe(tock);    
  for (uint8_t iter = 0; iter < strlen(scrollChar); iter ++){
    unsigned long timer = millis();
    HC12.print(F("font 17\r"));    
    pauseMe(20);
    HC12.printf(
          F("scroll %u %u %u %u \"%c\"\rpaint\r"),
              txtColour1, (x1 + 12*iter + 1/*+4*/), y1, window1, scrollChar[iter]);
    do{} while (millis() - timer < (scrollSpeed*140));                       // 800UL
    HC12.print(F("font 9\r"));                                               // Font for the static letter post-scroll
    sendChar(orange, (x1 + 12*iter), y1, scrollChar[iter]);
    pauseMe(2);
  }
  pauseMe(tick);
}


void goWhistle(uint8_t whistles){
  #if defined DEBUG || defined NOWHISTLE
    return;
  #endif
  HC12.print("~");                                                            //uncomment for whistles
  HC12.print(whistles);
   
}


void stopSign(void){                                                          // writes a bisected circle "STOP" sign
  /*(circle: colour x_centre y_centre radius) */
  clearMatrix(false);
  for (byte x=0; x<3; x++){
    writeCircle(red, 32, 15, 15-x);
    writeLine(red, 41+x, 6+x, 23+x, 24+x);
  }
}

void writeCircle(byte cCol1, byte cX1, byte cY1, byte cR1){
    HC12.printf(
        F("circle %u %u %u %u\rpaint\r"),
            cCol1, cX1, cY1, cR1);
}

void writeRectangle(byte rCol, byte rX1, byte rY1, byte rWid, byte rHi ){
  HC12.printf(
    F("rect %u %u %u %u %u \rpaint\r"),
      rCol, rX1, rY1, rWid, rHi);
      HC12.flush();
}

void writeLine(byte lCol, byte lX1, byte lY1, byte lX2, byte lY2) {
  HC12.printf(
    F("line %u %u %u %u %u \rpaint\r"),                             //  line colour, x1(), y1(), x2(), y2()
      lCol, lX1, lY1, lX2, lY2);
}

void redBorder(bool borderOn, byte nID){
  byte offSet = 13;
  if (borderOn == true) {
    HC12.print(F("$3"));
    pauseMe(5);
    goClock(offSet, nID);                                           
    sendNumber(red, colNumber, lnNumber, n_Count_[nID]);              // parks the count in red
    pauseMe(50);
  } else {
    HC12.print(F("$2"));
  }
}


void sendChar(byte chCol, byte chX, byte chY, byte chChar){
  HC12.printf(
            F("text %u %u %u \"%c\"\rpaint\r"),
              chCol, chX, chY, chChar);
}


void sendScrollChar(byte sSpeed, byte sLoop, byte sWiggle, byte sCol, byte sX, byte sY, byte sW, byte tXt) {
    HC12.printf(
        F("scrollspeed %u\r scrollloop %u\r scrollwiggle %u\r scroll %u %u %u %u\"%c\"\rpaint\r"),
          sSpeed, sLoop, sWiggle, sCol, sX, sY, sW, char(tXt)); 
    
}

/*
 * Reset the game to initial settings depending on
 * parameters set.
 CHANGE: Added flintRunning = false — card tap or any fresh
 start must always reset to Flint #1 state.
 */
// ================================================================
void zeroSettings(void) {
  shootDetail  = 0;                // reset detail flag to AB
  sEcount      = 1;                // reset end counter to first end
  continueOn   = false;            // halt the run loop
  sE_iter      = 0;                // reset sub-phase iterator
  countPractice = p_Store.maxPrac; // restore practice count from params
  flintRunning = false;            // always reset to Flint #1 on any fresh start
                                   // natural #1->#2 flip belongs in loop() only
}


/*
 * Function to re-start the display units on re-boot of the hand controller
 * This introduces a signal in the display on-board MCU to pull  MCLR on the 
 * PLT2001 to ground momentarily before the HC12.begin command is issued.
 */
void goReboot(void){
  HC12.print("^");
  HC12.print(9);  
  HC12.flush();  
  pauseMe(100); 
}


/*
 * 2 x Functions for reading value of a single button 
 * identified by the pin number given in the argument 
 * returned value: 
 * 0 - button is not pressed 
 * 1 - button is pressed 
 * 
 * De-bouncing is by implementing small sleep when button press is detected 
 * and doing a second read to verify if button is still pressed)
 * A)   
*/  
uint8_t readButton(int pin) {
  if (digitalRead(pin) == HIGH) return 0; 
  pauseMe(5);                                 // only reached if pin goes LOW, de-bounce delay, confirm read
  return (digitalRead(pin) == LOW) ? 1 : 0;
}
/* B)  
*/                                    
uint8_t readButtonNoDelay(int pin) {        // as above, without small sleep if button press detected
  return (digitalRead(pin) == LOW) ? 1 : 0;
}

/*
 * Function to read values of all four known buttons 
 * Returns bitmask of pressed buttons or 
 * just one of the BUTTON1..BUTTON4 values if only 
 * one button is pressed; Also handles de-bounce 
 */ 
uint8_t readButtons() {
  uint8_t ret = 0;
  
  if (   digitalRead(button1Pin)  == HIGH 
      && digitalRead(button2Pin)  == HIGH 
      && digitalRead(button3Pin)  == HIGH 
      && digitalRead(button4Pin)  == HIGH) {
      return 0;                                       // all are high - no buttons pressed 
  }
  pauseMe(2);                                         // delay for level de-bouncing; 2nd read to confirm low
  if (!intervalOn && digitalRead(button1Pin) == LOW)
    ret |= BUTTON1;                                   // set leftmost bit in the 'ret' to 1

  if (!intervalOn && digitalRead(button2Pin) == LOW)
    ret |= BUTTON2;                                   // send 2nd leftmost bit in the 'ret' to 1

  if (!intervalOn && digitalRead(button3Pin) == LOW)
    ret |= BUTTON3;                                   // etc...

  if (digitalRead(button4Pin) == LOW){
    if (intervalOn){
      clearFromLine(1);
      clearMatrix(false);
    }
    ret |= BUTTON4;
  }
  return ret;
}

/*
 * Similar to readButtons, but would wait in the infinite loop until any of 
 * the buttons is pressed, and will return value of the 
 * button pressed (BUTTON1..BUTTON4), or bitmask for multiple buttons pressed at once 
 * Use this function when it is required to wait for 
 * user input, and there is nothing to do until user presses some button
*/ 
uint8_t waitButton() {
  long long timeOut = millis();
  bool flag = false; 
  for (;;) {
    if (!intervalOn && !flag && (millis() - timeOut > (tick*60*15))) {
      clearMatrix(false);
      HC12.print(F("^8L"));                                 // dim the logo after the period above
      Serial.println("SPLASH");
      printDebugLine(false, __LINE__, __NAME__);
      writeSplash(true); 
      flag = true;
    }
    checkIntervalTimer();
    //#ifndef DEBUG
    if (getRFID()) {
      continueOn = false;
      displayParamsOnOLED();
      writeMenuCommands();
      printDebugLine(true, __LINE__, __NAME__);
    }
    //#endif
    uint8_t ret = readButtons();                            // read all button states
    if (ret != 0) {                                          // start of power-off routine
      long long goTurnOff = millis();
      while (readButtons() != 0) {
        delay(1);                                           // and now wait for button release as millis() clocks
        if (millis() - goTurnOff < 5000) {
          disp.clearLine(((millis() - goTurnOff)/500)-1);   // clear down the screen line by line to indicate shutdown intent progress
        }
      }
                                        
      if (millis() - goTurnOff > 4000                       // if release made after 4 secs then power off selected
          && ret == BUTTON4 ) {                             
        goPowerOff();                                       // turn controller Power OFF via the digital power switch
      } else { setHeader();
      }
      HC12.print(F("^8H"));                                 // brightness back to high
      return ret;                                           // finally return value of the button he pressed moments ago
      }
    pauseMe(1);
  }
}

// ── helper — print label + number, pad remainder to exactly 16 ──
void printPadded16(const char* label, int value) {
  const char* det = currentDetail();         // now derives fresh from sE_iter
  char left[17];
  snprintf(left, sizeof(left), "%s%d", label, value);  // e.g. "Prac 1" or "End 3"

  char buf[17];
  int detLen = strlen(det);                  // 0 or 2
  snprintf(buf, sizeof(buf), "%-*s%s", 16 - detLen, left, det);
  disp.print(buf);
}

// ================================================================
//  EMERGENCY HANDLING — f_FNCTs.ino
//  Complete drop-in replacement block
//  Replaces: goEmergencyButton, handleEmergencyRestart
//  Adds:     deriveIterForEnd, activeSubPhaseLabel,
//            handleEmergencyRestart_Screen2
// ================================================================


// ----------------------------------------------------------------
//  deriveIterForEnd()
//  Returns the correct sE_iter value for the FIRST sub-phase of
//  a given end, for use on emergency full-end restart.
//  goNormal_Op does sE_iter += 1 at top of its loop, so we
//  return one less — it will be corrected on re-entry.
// ----------------------------------------------------------------
int8_t deriveIterForEnd(int8_t endNum) {
  return (int8_t)((endNum * 2) - 2);
}

//  currentEndMax()
//  Returns the correct maximum seconds for the CURRENT end.
//  Walkup ends (Flint, sEcount > 6) use startCountsIndex+1.
//  Everything else uses startCountsIndex.
//  walkup-aware.
//  Used for:
//    - back-10 ceiling in BTN1
//    - count reset in BTN1/BTN2 of Screen 2
// ================================================================
uint8_t currentEndMax(void) {
  if (p_Store.isFlint && sEcount > 6) {              // in a Flint walkup end
    return startCounts[p_Store.startCountsIndex + 1]; // walkup duration (30s)
  }
  return startCounts[p_Store.startCountsIndex];       // standard end duration
}

const char* currentDetail(void) {
  if (p_Store.isFlint)      return "";
  if (p_Store.Details == 1) return "";
  if (p_Store.ifaaIndoor) {
    return (sEcount <= 6)
      ? (sE_iter % 2 == 1 ? "AB" : "CD")
      : (sE_iter % 2 == 1 ? "DC" : "BA");
  }
  return (sE_iter % 2 == 1 ? "AB" : "CD");  // WA: derive fresh, don't trust shootDetail
}

// ----------------------------------------------------------------
//  activeSubPhaseLabel()
//  Writes both detail groups to the current OLED cursor position,
//  inverting the active one for immediate operator recognition.
//  Call after disp.setCursor() to position correctly.
//  CHANGE: Flint guard — Flint has no AB/CD sub-phase concept.
// ----------------------------------------------------------------
void activeSubPhaseLabel(void) {

  if (p_Store.isFlint) {                // Flint is single detail — no AB/CD
    disp.print("Single  detail  ");     // neutral, fills row cleanly
    return;
  }

  const char* first;
  const char* second;

  if (p_Store.ifaaIndoor && sEcount > 6) {
    first  = "DC";                      // IFAA second half: DC shoots first
    second = "BA";
  } else {
    first  = "AB";                      // WA and IFAA first half
    second = "CD";
  }

  bool firstActive = (sE_iter % 2 == 1); // odd iter = first group active

  if (firstActive) {
    disp.inverse();   disp.print(first);
    disp.noInverse(); disp.print("  ");
                      disp.print(second);
  } else {
                      disp.print(first);
                      disp.print("  ");
    disp.inverse();   disp.print(second);
    disp.noInverse();
  }
  disp.noInverse();
}




// ================================================================
//  EMERGENCY RESTART SCREENS
//  - All commented-out dead code removed
//  - printPadded16() / currentDetail() applied throughout
//  - All bespoke padding replaced — no hardcoded space strings
//  - Flint walkup context uses printPadded16 with flintWalk string
//  - Screen 2 row 1 context consistent with Screen 1
//  - 16 char hard limit enforced on every row
// ================================================================


// ================================================================
//  handleEmergencyRestart_Screen2()
// ================================================================
int16_t handleEmergencyRestart_Screen2(byte nID) {
  int8_t  savedIter = sE_iter;           // snapshot sub-phase for resume
  uint8_t maxSecs   = currentEndMax();   // walkup-aware count ceiling

  for (;;) {
    clearFromLine(1);

    // ── Row 1: context ──────────────────────────────────────────
    disp.setCursor(0, 1);
    disp.setFont(u8x8_font_5x7_f);

    if (p_Store.isFlint && sEcount > 6) {
      // Flint walkup end — label + distance, padded to 16
      char buf[17];
      snprintf(buf, sizeof(buf), "Flint %s", flintWalk[sEcount - 7]);
      disp.print(buf);
      for (int i = strlen(buf); i < 16; i++) disp.print(" ");

    } else if (p_Store.isFlint) {
      // Standard Flint end
      printPadded16("Flint end ", sEcount);

    } else if (p_Store.ifaaIndoor) {
      // IFAA — hardcoded, exactly 16 chars
      disp.print(sEcount <= 6 ? "IFAA E1-6  AB>CD"
                              : "IFAA E7-12 DC>BA");

    } else {
      // WA — practice or competition end, detail right-justified
      if (countPractice) {
        printPadded16("Prac #", (p_Store.maxPrac - countPractice + 1));
      } else {
        printPadded16("End #", sEcount);
      }
      // Sub-phase on row 2 for WA
      disp.setFont(u8x8_font_chroma48medium8_r);
      disp.setCursor(0, 2);
      activeSubPhaseLabel();
    }

    disp.setFont(u8x8_font_chroma48medium8_r);

    // ── BTN1: Resume — least disruptive ─────────────────────────
    disp.setCursor(0, 3);
    disp.inverse();
    disp.print("Resume:   BTN[1]");
    disp.setCursor(0, 4);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    if (p_Store.isFlint && sEcount > 6) {
      // Walkup — show current distance in resume label
      char buf[17];
      snprintf(buf, sizeof(buf), "(resume@%s)", flintWalk[sEcount - 7]);
      disp.print(buf);
      for (int i = strlen(buf); i < 16; i++) disp.print(" ");
    } else {
      disp.print(p_Store.isFlint ? "(resume end)    "
                                : "(from activegrp)");
    }

    // ── BTN2: Restart — more disruptive ─────────────────────────
    disp.setCursor(0, 5);
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.inverse();
    disp.print("Restart:  BTN[2]");
    disp.setCursor(0, 6);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    if (p_Store.isFlint && sEcount >= 8) {
      disp.print("(restart@30 YD) ");  // offer rewind to 30m
    } else if (p_Store.isFlint) {
      disp.print("(restart end)   ");  // end 7 or standard Flint
    } else {
      disp.print(p_Store.ifaaIndoor
                ? (sEcount <= 6 ? "(restart AB>CD) "
                                : "(restart DC>BA) ")
                :                  "(from firstgrp) ");
    }

    // ── BTN4 hint (row 7) ────────────────────────────────────────
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.setCursor(0, 7);
    disp.inverse();
    disp.print("Reconsider:  [4]");  
    disp.noInverse();
    disp.setFont(u8x8_font_chroma48medium8_r);

    switch (waitButton()) {

      case BUTTON1: {
        // ── Resume from active position ────────────────────────
        reStartEnd  = true;
        for (byte r = 0; r <= 2; r++) n_Count_[r] = maxSecs;
        sE_iter     = savedIter;
        shootDetail = p_Store.ifaaIndoor
                      ? (sEcount > 6 ? true : false)
                      : (savedIter % 2 == 0 ? true : false);
        clearMatrix(false);
        return n_Count_[nID];
      }

      case BUTTON2: {
        // ── Restart ───────────────────────────────────────────
        if (p_Store.isFlint && sEcount >= 8) {
          sEcount = 7;                   // rewind to first walkup end
          maxSecs = currentEndMax();     // recalculate for new sEcount
        }
        reStartEnd  = true;
        for (byte r = 0; r <= 2; r++) n_Count_[r] = maxSecs;
        sE_iter     = deriveIterForEnd(sEcount);
        shootDetail = p_Store.ifaaIndoor
                      ? (sEcount > 6 ? true : false)
                      : ((sEcount % 2) == 1 ? false : true);
        clearMatrix(false);
        return n_Count_[nID];
      }

      case BUTTON3:
        return -1;                       // silent back to Screen 1

      case BUTTON4:
        return -2;                       // reconsider — propagates up
    }
  }
}


// ================================================================
//  handleEmergencyRestart()  — Screen 1
// ================================================================
int16_t handleEmergencyRestart(byte nID) {

  for (;;) {                             // loops on BTN3 back from Screen 2
    clearFromLine(1);

    // ── Row 1: context — 16 char hard limit ─────────────────────
    disp.setCursor(0, 1);
    disp.setFont(u8x8_font_5x7_f);

    if (p_Store.ifaaIndoor) {
      // Hardcoded — always exactly 16 chars, safe for E1-6 and E7-12
      disp.print(sEcount <= 6 ? "IFAA E1-6  AB>CD"
                              : "IFAA E7-12 DC>BA");

    } else if (p_Store.isFlint && sEcount > 6) {
      // Flint walkup — label + distance padded to 16
      char buf[17];
      snprintf(buf, sizeof(buf), "Flint %s", flintWalk[sEcount - 7]);
      disp.print(buf);
      for (int i = strlen(buf); i < 16; i++) disp.print(" ");

    } else if (p_Store.isFlint) {
      // Standard Flint end
      printPadded16("Flint end ", sEcount);

    } else {
      // WA — practice or competition, detail right-justified
      if (countPractice) {
        printPadded16("Prac #", (p_Store.maxPrac - countPractice + 1));
      } else {
        printPadded16("End #", sEcount);
      }
    }

    disp.setFont(u8x8_font_chroma48medium8_r);

    // ── BTN1: Resume, back 10s ───────────────────────────────────
    disp.setCursor(0, 2);
    disp.inverse();
    disp.print("Resume:   BTN[1]");
    disp.setCursor(0, 3);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    disp.print("(steps back 10s)");

    // ── BTN2: Go to Screen 2 ─────────────────────────────────────
    disp.setCursor(0, 4);
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.inverse();
    disp.print("Restart:  BTN[2]");
    disp.setCursor(0, 5);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    disp.print("(-->options)    ");

    // ── BTN3: Full competition restart ───────────────────────────
    disp.setCursor(0, 6);
    disp.setFont(u8x8_font_chroma48medium8_r);
    disp.inverse();
    disp.print("Reset ALL:BTN[3]");
    disp.setCursor(0, 7);
    disp.setFont(u8x8_font_5x7_f);
    disp.noInverse();
    disp.print("(full restart)  ");
    disp.setFont(u8x8_font_chroma48medium8_r);

    uint8_t maxSecs = currentEndMax();

    switch (waitButton()) {

      case BUTTON1:
        // Back 10s — walkup-aware ceiling
        n_Count_[nID] = ((int)n_Count_[nID] + 11 > (int)maxSecs)
                        ? maxSecs
                        : n_Count_[nID] + 11;
        clearMatrix(false);
        return n_Count_[nID];

      case BUTTON2: {
        int16_t result = handleEmergencyRestart_Screen2(nID);
        if (result == -1) continue;      // BTN3 on Screen 2 — loop back
        if (result == -2) return -2;     // BTN4 — propagate reconsider
        return result;
      }

      case BUTTON3:
        clearMatrix(false);
        flintRunning = false;            // always reset to Flint #1
        startOver    = true;
        memset(n_Count_, 0, sizeof(n_Count_));
        return n_Count_[nID];            // 0 — triggers reset in loop()

      case BUTTON4:
        return -2;                       // reconsider — propagates up
    }
  }
}



//  CHANGES:
//    - BTN4 reconsider now EXITS cleanly (ret = false, break)
//      rather than looping back into the emergency menu.
//      This restores the "oops/carry on" behaviour the operator expects.
//    - Snapshot includes all three globals as before.
//    - AIndex bottom section unchanged.
//    - BTN4 reconsider: displayParamsOnOLED() called after message
//      so OLED shows correct state on return to count
//    - doBarCount trigger uses currentEndMax() — walkup-aware
//    - emergencyReconsider behaviour unchanged
// ================================================================
bool goEmergencyButton(uint8_t AIndex, byte nID) {
  bool ret = false;

  if (readButtons() == BUTTON4) {
    ret = true;
    emergencyReconsider = false;         // clear flag at entry

    goWhistle(5);
    HC12.flush();
    stopSign();
    clearFromLine(1);
    disp.setCursor(4, 3);
    disp.print("EMERGENCY");
    disp.setCursor(6, 5);
    disp.print("STOP");
    pauseMe(5 * tick);

    // ── Snapshot globals at moment of emergency ──────────────────
    int8_t snapIter   = sE_iter;
    int8_t snapEcount = sEcount;
    bool snapDetail = p_Store.ifaaIndoor
                  ? (sEcount > 6 ? true : false)
                  : (sE_iter % 2 == 0 ? false : true);
    // p_Store.isFlint / p_Store.ifaaIndoor untouched by all
    // emergency paths — they live in p_Store (EEPROM-backed)

    for (;;) {
      int16_t result = handleEmergencyRestart(nID);

      if (result == -2) {
        // ── Reconsider — restore globals, exit cleanly ───────────
        sE_iter     = snapIter;
        sEcount     = snapEcount;
        shootDetail = snapDetail;
        emergencyReconsider = true;      // signal caller to restore locals
        clearFromLine(1);
        disp.setCursor(0, 3);
        disp.print("  Reconsidering ");
        disp.setCursor(0, 5);
        disp.print("Resuming count..");
        pauseMe(2 * tick);
        displayParamsOnOLED();           // refresh OLED to correct state
        // Exit — do NOT re-enter menu. Return to the running count.
        break;
      }

      // ── Decision made ────────────────────────────────────────
      n_Count_[nID] = result;
      emergencyReconsider = false;       // clean exit
      break;
    }

    // ── Post-emergency display setup ─────────────────────────────
    // Only runs when a real decision was made (not reconsider).
    if (!emergencyReconsider) {
      if (AIndex < 3) {
        displayParamsOnOLED();
        // Use currentEndMax() — walkup ends return walkup duration
        if (n_Count_[nID] == currentEndMax()) {
          doBarCount(AIndex, nID);       // full restart — run walkup bar
          writeOLED_Data(1, nID);
        } else if (n_Count_[nID] != 0) {
          goWhistle(1);                  // partial resume — one whistle
          writeOLED_Data(1, nID);
        }
      } else if (AIndex == 3) {
        displayParamsOnOLED();
        writeOLED_Data(1, nID);
      } else if (AIndex == 4) {
        // reserved
      }
    }
  }
  return ret;
}

void displayMenuPage(uint8_t idx, uint8_t selectionIdx) {
  
  if (idx == 0) {
    for (uint8_t i = 0; i < sizeof(menu0)/sizeof(menu0[0]); ++ i) {
      disp.setCursor(0, i+1);
      if (i == selectionIdx) disp.print("> ");
      else disp.print("  ");
      disp.print(menu0[i]);
    }
  }  
}

void command_ON(bool command){                        // enter and exit HC12 Command mode
  digitalWrite(HC12SetPin, command ? LOW : HIGH);     // COMMAND MODE
  pauseMe(80);                                        // settle
  while(HC12.available()) HC12.read();            // empty  out possible garbage
}

void pauseMe(uint16_t holdOff){
  long long pause = millis();
  do {} while (millis() - pause < holdOff);
}

void goPowerOff(void){
  wipeOLED();
  disp.draw2x2String(0, 2, "POWERING");
  disp.draw2x2String(0, 6, "  DOWN  ");
  HC12.print(F("^8L"));                                 // dim the logo after the period above
  writeSplash(false);                                 // write a dimmed splash - as a reminder to turn off...

  HC12.flush();
  delay(tick);
  digitalWrite(offControlPin, HIGH);  
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Show version and file details on bootup; this MUST be called from the file you wish to ID 
 */
void dispSrcFileDetails(const char* fileName ){

  //const char *the_path = PSTR(fileName);                                    // save RAM, use flash to hold __FILE__ instead

  int slash_loc = pgm_lastIndexOf('/',fileName);                            // index of last '/' 
  if (slash_loc < 0) slash_loc = pgm_lastIndexOf('\\',fileName);            // or last '\' (windows, ugh)

  int dot_loc = pgm_lastIndexOf('.',fileName);                              // index of last '.'
  if (dot_loc < 0) dot_loc = pgm_lastIndexOf(0,fileName);                   // if no dot, return end of string
  uint8_t c = 16 - dot_loc;
  uint8_t r = 1;
  disp.setFont(u8x8_font_5x7_f);
  for (int i = slash_loc+1; i < dot_loc; i++) {
    uint8_t b = pgm_read_byte(&fileName[i]);
    if (b != 0) {
      disp.setCursor(c,r);      
      disp.print((char) b);
      c++;
    }
    else break;
  }
  disp.setFont(u8x8_font_chroma48medium8_r);
}


/* Find limits of program header details
 */
int pgm_lastIndexOf      (uint8_t c, const char * p) {
  int last_index = -1; // -1 indicates no match
  uint8_t b;
  for(int i = 0; true; i++) {
    b = pgm_read_byte(p++);
    if (b == c)
      last_index = i;
    else if (b == 0) break;
  }
  return last_index;
}

// ----------------------------------------------------------------
void timeOut(uint32_t intervalSet) {
  uint32_t t0 = millis();
  for (;;) {
    if (readButtons() == BUTTON4) {
      clearMatrix(false);
      return;
    }
    if ((uint32_t)(millis() - t0) >= intervalSet) return;
  }
}

// ----------------------------------------------------------------
//  pollButtonsOnce()
//  Edge-detecting button reader — returns a bitmask of buttons
//  that have just been NEWLY pressed since the last call.
//  Call in tight loops without re-triggering on held buttons.
// ----------------------------------------------------------------
byte pollButtonsOnce() {
  bool b1 = (digitalRead(button1Pin) == LOW);
  bool b2 = (digitalRead(button2Pin) == LOW);
  bool b3 = (digitalRead(button3Pin) == LOW);
  bool b4 = (digitalRead(button4Pin) == LOW);

  static bool prev1 = false, prev2 = false,
              prev3 = false, prev4 = false;

  byte evt = 0;
  if (b1 && !prev1) evt |= BUTTON1;
  if (b2 && !prev2) evt |= BUTTON2;
  if (b3 && !prev3) evt |= BUTTON3;
  if (b4 && !prev4) evt |= BUTTON4;

  prev1 = b1; prev2 = b2; prev3 = b3; prev4 = b4;
  return evt;
}



void bannerPrompt() {
  if (!bnr_isPresent()) return;

  bool defaultRun = (EEPROM.read(EE_BNR_RUN) == 1);  // only 1 means play

  wipeOLED();
  disp.draw2x2String(2, 1, "BANNER");
  disp.setCursor(0, 3);
  disp.print("Play:    BTN[1] ");
  disp.setCursor(0, 4);
  disp.print("WIPE:    BTN[3L]");
  disp.setCursor(0, 5);
  disp.print("Skip:    BTN[4] ");
  disp.setCursor(0, 6);
  disp.inverse();
  disp.print(defaultRun ? "Auto:      PLAY " : "Auto:      SKIP ");
  disp.noInverse();

  const uint32_t timeout = 7000;
  uint32_t t0 = millis();
  // flush any button state before polling
  while (pollButtonsOnce()) delay(10);
  delay(50);
  while ((uint32_t)(millis() - t0) < timeout) {
    byte btn = pollButtonsOnce();
    if (btn & BUTTON1) {
      bnr_setRun(true);
      disp.clearLine(4);
      disp.clearLine(5);
      bnrPlay(true);

      return;
    }
    if (btn & BUTTON3) {  // Must hold BTN3 for 2 seconds to confirm clear      
      disp.setCursor(0, 7);
      disp.print("Hold to clear...");
      uint32_t holdStart = millis();
      bool cancelled = false;
      while ((uint32_t)(millis() - holdStart) < 2000) {
          if (!(digitalRead(button3Pin) == LOW)) {
              cancelled = true;
              break;
          }
      }
      if (!cancelled) {
          bnr_clear();
          disp.setCursor(0, 7);
          disp.print("CLEARED         ");
          delay(1000);
          return;
      } else {
          disp.setCursor(0, 7);   // clean up if they let go early
          disp.print("                ");
      }
    }
    if (btn & BUTTON4) {
      bnr_setRun(false);
      disp.setCursor(0, 7);
      disp.print("SKIP SAVED      ");
      delay(1000);
      return;
    }
    delay(10);
  }

  // Timeout — honour sticky preference
  if (defaultRun) {
    bnrPlay(false);
  }
  // else do nothing — skip
}

//=========================================================================================
/*  DEBUGGING TOOLS  */



void debugEEPROM(byte lowAddress, byte eeAddress){
  #ifndef DEBUG
  return;
  #endif
  const char*     nameParam[18] = { "Time", "Walk", "Ends", "Dets", 
                                  "Prac", "Fnls", "BrkT", "Altr", 
                                  "Team", "A/B?", "Flnt", "Ch/Supv", 
                                  "B_Scr", "PS13", "PS14", "PS15",
                                  "PS16", "PS17"};
  Serial.println();
  Serial.println(F("EEPROM:  "));
  if (eeAddress) {
    for (byte n = lowAddress; n < eeAddress+5; n++){
     Serial.print(n);
     Serial.print(F(":\t"));
     if ( n <= 15) {
      Serial.print(nameParam[n]);
     }
     Serial.print(F(":\t"));
     Serial.println(EEPROM.read(n));               
    }
  } else {
    Serial.print(lowAddress);
    Serial.print(F(":\t"));
    Serial.println(EEPROM.read(lowAddress));
  }
  pauseMe(200);
  
}


/*
 * Handle printing line numbers for debugging 
 */
void printDebugLine(bool dets, uint16_t lineNo, const char* FileName){
  // #ifndef DEBUG  
  // return;
  // #endif  
    Serial.print(F("We are at LINE:\t"));
    Serial.print(lineNo);
    Serial.print(F(" in TAB: "));
    Serial.println(FileName);
    if (dets){
      Serial.print(F("pS.CtIndex :\t"));
      Serial.println(p_Store.startCountsIndex);
      Serial.print(F("pS.walkUp :\t"));
      Serial.println(p_Store.walkUp);
      Serial.print(F("pS.maxEnds :\t"));
      Serial.println(p_Store.maxEnds);
      Serial.print(F("pS.Details :\t"));
      Serial.println(p_Store.Details);

      Serial.print(F("pS.maxPrac :\t"));
      Serial.println(p_Store.maxPrac);
      Serial.print(F("pS.isFinals :\t"));
      Serial.println(p_Store.isFinals);
      Serial.print(F("pS.breakPeriod :\t"));
      Serial.println(p_Store.breakPeriod);
      Serial.print(F("pS.isAlternating :\t"));
      Serial.println(p_Store.isAlternating);

      Serial.print(F("pS.teamPlay :\t"));
      Serial.println(p_Store.teamPlay);
      Serial.print(F("pS.whichArcher :\t"));
      Serial.println(p_Store.whichArcher);
      Serial.print(F("pS.isFlint :\t"));
      Serial.println(p_Store.isFlint);
      Serial.print(F("pS.curChan :\t"));
      Serial.println(p_Store.curChan);

      Serial.print(F("pS.ifaaIndoor :\t"));
      Serial.println(p_Store.ifaaIndoor);
      Serial.print(F("pS.Banner :\t"));
      Serial.println(p_Store.Banner);
      Serial.print(F("pS.B_ScrCh :\t"));
      Serial.println(p_Store.B_ScrCh);
      Serial.print(F("pS.which_Scr_1st :\t"));
      Serial.println(p_Store.which_Scr_1st);
      // Serial.print(F("pS.PS16 :\t"));
      // Serial.println(p_Store.PS16);
      // Serial.print(F("pS.PS17 :\t"));
      // Serial.println(p_Store.PS17);
      
      
      Serial.print(F("shootDetail :\t"));
      Serial.println(shootDetail);
      Serial.print(F("sE_iter :\t"));
      Serial.println(sE_iter);
      Serial.print(F("sEcount :\t"));
      Serial.println(sEcount);
      Serial.print(F("countPractice :\t"));
      Serial.println(countPractice);
      Serial.print(F("continueOn :\t"));
      Serial.println(continueOn);
      Serial.print(F("startOver :\t"));
      Serial.println(startOver);
      Serial.print(F("emergencyReconsider:\t"));
      Serial.println(emergencyReconsider);
          
       
      Serial.print(F("arrowCount :\t"));
      Serial.println(arrowCount);

      ;
      Serial.print(F("n_Count_[0] :\t"));
      Serial.println(n_Count_[0]);
      Serial.print(F("n_Count_[1] :\t"));
      Serial.println(n_Count_[1]);
      Serial.print(F("n_Count_[2] :\t"));
      Serial.println(n_Count_[2]);             
      
//      Serial.print(F("Key12 :\t"));
//      Serial.println(Key12);
//      Serial.print(F("Key13 :\t"));
//      Serial.println(Key13);
//      Serial.print(F("Key14 :\t"));
//      Serial.println(Key14);
//      Serial.print(F("Key15 :\t"));
//      Serial.println(Key15);
    }
    Serial.flush();
}

// void see(const char* peek,  uint8_t seeMe){
//   #ifndef DEBUG
//   return;
//   #endif  
//   Serial.print(F("peek at var '"));
//   Serial.print((const char*) peek);
//   Serial.print("': ");
//   Serial.println(seeMe);
// }


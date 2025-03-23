
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
  sendNumber(orange, colNumber, lnNumber, p_Store.maxPrac);
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
    u8x8.setCursor(0, 2);
    p_Store.teamPlay ? u8x8.print("Set 1st Team :-") : u8x8.print("Set 1st Player:-");
    u8x8.setCursor(5, 4);
    p_Store.teamPlay ? u8x8.print("Team   ") : u8x8.print("Archer ");
    u8x8.print(menuArcher == 1 ? "A" : "B");
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
  u8x8.draw2x2String(0, 2, "SHOOTOFF");
  u8x8.draw2x2String(0, 4, "NEEDED?");
  u8x8.setCursor(0, 6);
  u8x8.inverse();
  u8x8.print("NO- END: BTN[1]");
  u8x8.setCursor(0, 7);
  u8x8.print("YES:     BTN[2]");
  u8x8.noInverse();
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
  u8x8.draw2x2String(6, 4, "OK");                                         // acknowledge on controller                
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
 */
void zeroSettings(void){
  shootDetail = 0;   
  sEcount = 1;
  continueOn = false;                  
  sE_iter = 0;
  countPractice = p_Store.maxPrac;
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
      writeSplash(true); 
      flag = true;
    }
    checkIntervalTimer();
    //#ifndef DEBUG
    if (getRFID()) { //if (getRFID(&p_Store)) {
      continueOn = false;
      printDebugLine(true, __LINE__, __NAME__);
    }
    //#endif
    uint8_t ret = readButtons();                            // read all button states
    if (ret != 0) {                                          // start of power-off routine
      long long goTurnOff = millis();
      while (readButtons() != 0) {
        delay(1);                                           // and now wait for button release as millis() clocks
        if (millis() - goTurnOff < 5000) {
          u8x8.clearLine(((millis() - goTurnOff)/500)-1);   // clear down the screen line by line to indicate shutdown intent progress
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


bool goEmergencyButton(uint8_t AIndex, byte nID){
  bool ret = false;
  if (readButtons() == BUTTON4) {                           // Handle Red-button push
    ret = true;
    goWhistle(5);
    stopSign();
    clearFromLine(1);
    u8x8.setCursor(4,3);
    u8x8.print("EMERGENCY");
    u8x8.setCursor(6,5);
    u8x8.print("STOP");
    pauseMe(5*tick);
    n_Count_[nID] = handleEmergencyRestart(nID);                     // go fetch the next step decision
    if (AIndex < 3){    
      displayParamsOnOLED();
      if (n_Count_[nID] == startCounts[p_Store.startCountsIndex]) {
        //printDebugLine(false, __LINE__, __NAME__); 
        doBarCount(AIndex, nID);
        writeOLED_Data(1, nID);
      } else if (n_Count_[nID] != 0) {
        goWhistle(1);
        writeOLED_Data(1, nID);
      }
    } else if (AIndex == 3){                                        // for in-barCount !STOP!
      displayParamsOnOLED();
      writeOLED_Data(1, nID);
    } else if (AIndex == 4){
      //saved for future use <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    }
  }
  return ret;
}

/*
 * menu to select 1 of 3 options:
 * 1 go back 10 secs and resume
 * 2 go back to zero secs and resume
 * 3 do a re-start keeping current parameters
 */
int16_t handleEmergencyRestart(byte nID){
  clearFromLine(1);
  u8x8.setCursor(0, 2);
  u8x8.inverse(); 
  u8x8.print("Resume:   BTN[1]");
  u8x8.setCursor(0, 3);
  u8x8.setFont(u8x8_font_5x7_f);
  u8x8.noInverse();
  u8x8.print("(steps back 10s)");
  u8x8.setCursor(0, 4);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.inverse();
  u8x8.print("Restart:  BTN[2]");
  u8x8.setCursor(0, 5);
  u8x8.setFont(u8x8_font_5x7_f);
  u8x8.noInverse();
  u8x8.print("(recommence end)");
  u8x8.setCursor(0, 6);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.inverse();
  u8x8.print("Reset ALL:BTN[3]");
  u8x8.setCursor(0, 7);
  u8x8.setFont(u8x8_font_5x7_f);
  u8x8.noInverse();
  u8x8.print("(reinitialize)");
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  uint8_t maxPossSecs = startCounts[p_Store.startCountsIndex];
  
  switch (waitButton()) {

    case BUTTON1: 
      n_Count_[nID] = (n_Count_[nID] +11 > maxPossSecs) ? maxPossSecs : n_Count_[nID] +11;  // rewind 10 sec and continue
      clearMatrix(false);
    break;
    
    case BUTTON2:
      reStartEnd = true;                                                  // reset all counts, restart and continue with competition
      for (byte reset = 0; reset <= 2 ; reset ++) n_Count_[reset] = startCounts[p_Store.startCountsIndex];
      clearMatrix(false);
    break;

    case BUTTON3:
      clearMatrix(false);         
      startOver = true;                                                   // reset and start it all again
      memset(n_Count_, 0, sizeof(n_Count_));                               //n_Count_[nID] = 0;  ##########################################################<<<<<<<<<
    break;
  }
  return n_Count_[nID] ;
}

void displayMenuPage(uint8_t idx, uint8_t selectionIdx) {
  
  if (idx == 0) {
    for (uint8_t i = 0; i < sizeof(menu0)/sizeof(menu0[0]); ++ i) {
      u8x8.setCursor(0, i+1);
      if (i == selectionIdx) u8x8.print("> ");
      else u8x8.print("  ");
      u8x8.print(menu0[i]);
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
  u8x8.draw2x2String(0, 2, "POWERING");
  u8x8.draw2x2String(0, 6, "  DOWN  ");
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
  u8x8.setFont(u8x8_font_5x7_f);
  for (int i = slash_loc+1; i < dot_loc; i++) {
    uint8_t b = pgm_read_byte(&fileName[i]);
    if (b != 0) {
      u8x8.setCursor(c,r);      
      u8x8.print((char) b);
      c++;
    }
    else break;
  }
  u8x8.setFont(u8x8_font_chroma48medium8_r);
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
  #ifndef DEBUG
  return;
  #endif  
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
      Serial.print(F("pS.PS16 :\t"));
      Serial.println(p_Store.PS16);
      Serial.print(F("pS.PS17 :\t"));
      Serial.println(p_Store.PS17);
      
      
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




/*#############################################################################################################*/
/*#############################################################################################################*/

void goDemoLoop() {
  

//  const char demoChar1[] = "Cuchulainn";
  const char demoChar2[] = "Archers";
  
  const char demoChar3[] = "Welcome to";
  //const char demoChar4[] = "a  W.R.S.";
  const char demoChar4[] = "a Wintery";
  //const char demoChar5[] = "  W.R.S. ";
  const char demoChar6[] = " WA18 X 2";
  const char demoChar7[] = "Aim  for the";
  const char demoChar8[] = "GOLD";
  const char demoChar9[] = "C";                                   // |
  const char demoChar10 = 250;                                    // | CúCú Abú
//  const char demoChar11[] = "Ab";                                 // | 
  clearFromLine(1);
  u8x8.draw2x2String(2, 2, "BANNER");
  u8x8.draw2x2String(4, 4, "MODE");
  u8x8.inverse();
  u8x8.setCursor(0, 7);
  u8x8.print("TO EXIT:  BTN[4]");
  for (;;) {
    clearMatrix(true);    
    writeSplash(true);
    timeOut(2000); if (!demoMode) break;
    clearMatrix(true);
    timeOut(2000); if (!demoMode) break;    
    HC12.print(F("font 11\r"));    HC12.flush();
    sendSerialS( orange, /*column=*/ 2, /*line=*/ 20, demoChar2);
    timeOut(2500); if (!demoMode) break;
    clearMatrix(false);    
    HC12.print(F("font 4\r"));    HC12.flush();
    sendSerialS( green, /*column=*/ 4, /*line=*/ 10, demoChar3);
    timeOut(1200); if (!demoMode) break;
    HC12.print(F("font 10\r"));    HC12.flush();
    sendSerialS( orange, /*column=*/ 0 , /*line=*/ 30, demoChar4);
    timeOut(2000); if (!demoMode) break;
    clearMatrix(false);
    timeOut(500); if (!demoMode) break;
    HC12.print(F("font 14\r"));    HC12.flush();
    for (byte i = 4; i<=45; i+=28){
      sendSerialS( orange, /*column=*/ i, /*line=*/ 25, demoChar9);        // | CúCú
      sendChar( orange, /*column=*/ i + 15, /*line=*/ 25, demoChar10);
    }
//    HC12.print(F("font 4\r"));    HC12.flush();
//    sendSerialS( green, /*column=*/ 25, /*line=*/ 10, demoChar5);    
    timeOut(500); if (!demoMode) break;
    HC12.print(F("font 8\r"));    HC12.flush();
    
    sendSerialS( green, /*column=*/ 2, /*line=*/ 33, demoChar6);
    timeOut(4500); if (!demoMode) break;
    clearMatrix(false);
    HC12.print(F("font 4\r"));    HC12.flush();
    sendSerialS( green, /*column=*/ 3, /*line=*/ 10, demoChar7);    
    timeOut(800); if (!demoMode) break;
    HC12.print(F("font 11\r"));    HC12.flush();
    sendSerialS( orange, /*column=*/ 9, /*line=*/ 28, demoChar8);
    timeOut(3000); if (!demoMode) break;
    clearMatrix(false);
//    HC12.print(F("font 14\r"));    HC12.flush();
//    for (byte n = 0; n <= 2; n++){
//      for (byte i = 4; i<=45; i+=28){
//      sendSerialS( orange, /*column=*/ i, /*line=*/ 30, demoChar9);
//      sendChar( orange, /*column=*/ i + 15, /*line=*/ 30, demoChar10);
//      }
//      timeOut(500);
//      clearMatrix();
//      
//      sendSerialS( green, /*column=*/ 10, /*line=*/ 30, demoChar11);
//      sendChar( green, /*column=*/ 40, /*line=*/ 30, demoChar10);
//      timeOut(500);
//      clearMatrix();
//    }
    timeOut(2000);       
    if (!demoMode) break;
    
  }
  clearMatrix(false); 
  return;
}


void timeOut(long long intervalSet) {
  long long demoTimer = millis();  
 for(;;) {
  if (readButtons() == BUTTON4) {
    clearMatrix(true);
    demoMode = false;
    return;
    }
  if (millis() - demoTimer >=  intervalSet) break;
  }
}

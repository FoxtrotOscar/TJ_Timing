
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
    writeInfoBigscreen
    goChooseArcher
    checkForShootoff
    writeShootOff
    clearMatrix
    writeSplash
    sendScrollW
    goWhistle
    stopSign
    writeCircle
    writeRectangle
    writeLine
    sendChar
    sendScrollChar
    zeroSettings
    goReboot
    doButtonMenu
    writeMenuCommands
    readButton
    readButtonNoDelay
    readButtons
    waitButton
    goEmergencyButton
    handleEmergencyRestart
    displayMenuPage
    commandBaudRate
    pauseMe
    goPowerOff
    dispSrcFileDetails
    pgm_lastIndexOf
    checkEEPROM
    printDebugLine
*/


 
 
/*
 * Write info regarding tournament setup
 */
void writeInfoBigscreen(void){
  clearMatrix();
  #ifdef DEBUG
  return;
  #endif
  pauseMe(tick);   
  HC12.print(F("font 1\r"));    HC12.flush();
  lnNumber = 5;
  
  if (paramStore.isFinals && !paramStore.isAlternating) {
    sendSerialS( green, /*column=*/ 0, /*line=*/ lnNumber, "MATCHPLAY", "");
  }

  else if (paramStore.isFinals && paramStore.isAlternating) {
    sendSerialS( green, /*column=*/ 0, /*line=*/ lnNumber, "FINALS", "");
  }

  else if (paramStore.teamPlay) {
    sendSerialS( green, /*column=*/ 0, /*line=*/ lnNumber, "Team", "");
  }
  else {  
    if (paramStore.isFlint ){
      sendSerialS( red, /*column=*/ 0, /*line=*/ lnNumber, "FLINT Round", ""); 
    }else{
      sendSerialS( red, /*column=*/ 0, /*line=*/ lnNumber, 
      (paramStore.Details == 1 ? "Single " : "Double "), "Detail");               // either 1 OR 2
    }
  }
  pauseMe(tick);
  
  lnNumber += 7;                                                                  // Practice?
  sendSerialS( green, /*column=*/ 0, /*line=*/ lnNumber, "Practice");
  colNumber = 56;
  pauseMe(tick/4);
  sendNumber(orange, colNumber, lnNumber, paramStore.maxPrac);
  pauseMe(tick/4);  
  lnNumber += 7;                                                                  // End duration
  sendSerialS( green, /*column=*/ 0, /*line=*/ lnNumber, "END ","Time: ");
  colNumber = 48 +  ( startCounts[paramStore.startCountsIndex] < 10   ? 8 :       // Positioning the number on the matrix
                      startCounts[paramStore.startCountsIndex] < 20   ? 5 :
                      startCounts[paramStore.startCountsIndex] < 100  ? 3 : 
                      startCounts[paramStore.startCountsIndex] < 200  ? 0 : -2);
                      
  pauseMe(tick/4);
  sendNumber(orange, colNumber, lnNumber, startCounts[paramStore.startCountsIndex]);
  pauseMe(tick/4);  

  lnNumber += 7;  // Number of Ends
  sendSerialS( green, /*column=*/ 0, /*line=*/ lnNumber, "END ","Count: ");
  colNumber = 48 + (paramStore.maxEnds < 10 ? 8 : paramStore.maxEnds < 20 ? 5 : 3);// Positioning the number on the matrix
  pauseMe(tick/4);
  sendNumber(orange, colNumber, lnNumber, paramStore.maxEnds);
  pauseMe(5*tick);
  clearMatrix();


}


/*
 * Choose which archer is UP
 */

void goChooseArcher(void){
uint8_t menuArcher  = 1;  
start_AB_menu:

  clearFromLine(1);
  u8x8.setCursor(0, 2);
  u8x8.print("Set 1st player:-");
  u8x8.setCursor(5, 4);
  u8x8.print("Archer ");
  u8x8.print(menuArcher == 1 ? "A" : "B");
  doButtonMenu();
  
  switch (waitButton())
  {
    case BUTTON1: 
      paramStore.whichArcher = menuArcher;
      return;

    case BUTTON2: 
      menuArcher = menuArcher == 1 ? 2 : 1;
      goto start_AB_menu;

    case BUTTON3: 
      menuArcher = menuArcher == 1 ? 2 : 1;
      goto start_AB_menu;

    case BUTTON4:
      menuArcher = paramStore.whichArcher;
      return;
  }
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
    if (btn == BUTTON1) {
        break;                                    // move on
    } else if (btn == BUTTON2) {
        flag = true;
        break;
    } 
  }
  return flag; 
}

void writeShootOff(void){
  clearMatrix();
  HC12.print(F("font 9\r"));   
  HC12.flush();
  sendSerialS(2, 0, 20, "SHOOTOFF");
  
  clearFromLine(4);
  u8x8.draw2x2String(6, 4, "OK");
//  u8x8.setCursor(0, 6);
//  u8x8.inverse();
//  u8x8.print("Proceed:  BTN[1]");
//  u8x8.noInverse();
}

/*
 * Wipes the Matrix clear
 */
void clearMatrix(void){
  HC12.print(F("scrollloop 0\r"));                                        //  if scroll in progress, kill it
  pauseMe(4*tock);
  HC12.print(F("clear\r"));
  HC12.print(F("paint\r"));    
  HC12.flush();
}
  


/* 
 *  Splashscreen of Logo "Time Judge"
*/
void writeSplash(bool scrolling){
  #ifdef DEBUG
  return;
  #endif
  
  for (uint16_t br = 0; br <= bright; br += 17){                           // fade increment [10]
    unsigned long timer = millis();
    HC12.printf(
          F("brightness %u\rpaint\r"),                                     //  | Fade in "Time"
            br);
    HC12.print(F("font 16\r"));    HC12.flush();
    sendSerialS( green, /*column=*/ 2, /*line=*/ 24, "Time");
    //pauseMe(2*tock);
    do{} while (millis() - timer < 2UL);                                  // allow serial traffic time (15UL)
    }
  const char scrollChar[] = "JUDGE";
  HC12.print(F("font 9\r"));    
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

  HC12.printf(
    F("scrollspeed %u \r scrollloop %u \r scrollwiggle %u \r"),
      scrollSpeed, scrollLoop, scrollWiggle);
  HC12.flush();  pauseMe(tock);    
  //pauseMe(20);
  for (uint8_t iter = 0; iter < strlen(scrollChar); iter ++){
    unsigned long timer = millis();
    HC12.print(F("font 17\r"));    
    //HC12.flush();  pauseMe(tock);
    pauseMe(20);
    HC12.printf(
          F("scroll %u %u %u %u \"%c\"\rpaint\r"),
              txtColour1, (x1 + 12*iter + 1/*+4*/), y1, window1, scrollChar[iter]);
    // do{} while (millis() - timer < (scrollSpeed*105));    // 800UL
    do{} while (millis() - timer < (scrollSpeed*140));    // 800UL
    HC12.print(F("font 9\r"));     //HC12.flush();  pauseMe(tock);              // Font for the static letter post-scroll
    sendChar(orange, (x1 + 12*iter), y1, scrollChar[iter]);
    //pauseMe(15);
    pauseMe(2);
  }
  pauseMe(tick);
}




void goWhistle(uint8_t whistles){
  HC12.print("~"); //HC12.flush();              //uncomment for whistles
  HC12.print(whistles);  
  //HC12.flush();
}


void stopSign(void){                                                          // writes a bisected circle "STOP" sign
  /*
   * (circle: colour x_centre y_centre radius)
   */
  clearMatrix();
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
}

void writeLine(byte lCol, byte lX1, byte lY1, byte lX2, byte lY2) {
  HC12.printf(
    F("line %u %u %u %u %u \rpaint\r"),                         //  line colour, x1(), y1(), x2(), y2()
      lCol, lX1, lY1, lX2, lY2);
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
  countPractice = paramStore.maxPrac;
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
 * handle the in-menu button information layout
 */
void doButtonMenu(void) {
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  u8x8.setCursor(2, 7);
  u8x8.write(157);                                  // (a square) for "stop" or "cancel"
  u8x8.write(32); u8x8.write(32);
  u8x8.write(171);                                  // << "back" or "decrement"
  u8x8.write(32); u8x8.write(32); u8x8.write(32);
  u8x8.write(187);                                  // >> "forward" or "increment"
  u8x8.write(32); u8x8.write(32);
  u8x8.inverse();
  u8x8.print("ok");                                 //  > "accept change"
  u8x8.noInverse();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
}


/*
 * Writes the three top-menu command options on lower OLED lines
 */
void writeMenuCommands(void){
  clearFromLine(5);                                 // clear the way
  u8x8.setCursor(0, 5);
  u8x8.inverse(); 
  u8x8.print((intervalOn ? "COUNTDOWN is ON-" : "Continue: BTN[1]"));
  u8x8.noInverse();
  u8x8.setCursor(0, 6);
  u8x8.print((intervalOn ? "-to QUIT: BTN[4]" : "Change:   BTN[2]"));
  u8x8.setCursor(0, 7);
  u8x8.inverse();
  u8x8.print("or TIME-TAP now ");
  u8x8.noInverse();
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
/*
 *  B)  
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
      clearMatrix();
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
      clearMatrix();
      bright = 25;                                          // dim the logo after the period above
      writeSplash(true); 
      bright = 255;                                         // reset the brightness
      flag = true;
    }
    checkIntervalTimer();
    
    getRFID(&paramStore);
    
    uint8_t ret = readButtons();                            // read all button states
    
    if (ret != 0) {
      long long goTurnOff = millis();
      while (readButtons() != 0) {
        delay(1);                                           // and now wait for him to release the button
      }                                                     // if release made after 4 secs then power off selected
      if (millis() - goTurnOff > 4000
          && ret == BUTTON4 ) {                             // if button 4 held for >4 secs 
        goPowerOff();                                       // turn controller Power OFF via the digital power switch
      }
      HC12.print(F("brightness "));                         // return brightness to nominal 
      HC12.print(bright);                                                      
      HC12.print(F("\r"));
      return ret;                                           // finally return value of the button he pressed moments ago
      }
    pauseMe(1);
  }
}


bool goEmergencyButton(uint8_t AIndex){
  bool ret = false;
  if (readButtons() == BUTTON4) {                           // Handle Red-button push
    goWhistle(5);
    stopSign();
    clearFromLine(1);
    u8x8.setCursor(4,3);
    u8x8.print("EMERGENCY");
    u8x8.setCursor(6,5);
    u8x8.print("STOP");
    pauseMe(5*tick);
    n_Count = handleEmergencyRestart();                     // go fetch the next step decision
    
    if (AIndex < 3){    
      displayParamsOnOLED();
      if (n_Count == startCounts[paramStore.startCountsIndex]) {
        doBarCount(AIndex);
        writeOLED_Data(1);
      } else if (n_Count != 0) writeOLED_Data(1);
    } else if (AIndex == 3){                                // for in-barCount !STOP!
      displayParamsOnOLED();
      writeOLED_Data(1);
      ret = true;
    } else if (AIndex == 4){
      //saved for future use <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    }
  }
  return ret;
}

/*
 * Write a menu to select 1 of two options:
 * 1 go back 10 secs and resume
 * 2 go back to zero secs and resume
 * 3 do a re-start keeping current parameters
 */
int16_t handleEmergencyRestart(void){
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
  
  uint8_t maxPossSecs = startCounts[paramStore.startCountsIndex];
  
  switch (waitButton()) {

    case BUTTON1: 
      n_Count = (n_Count +11 > maxPossSecs) ? maxPossSecs : n_Count +11;  // rewind 10 sec and continue
      clearMatrix();
    break;
    
    case BUTTON2:
      reStartEnd = true;                                    
      n_Count = startCounts[paramStore.startCountsIndex];                 // restart this end, continue with competition
      clearMatrix(); 
    break;

    case BUTTON3:
      clearMatrix();         
      startOver = true;                                                   // reset and start it all again
      n_Count = 0;
    break;
  }
  
  return n_Count ;
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

bool commandBaudRate(bool command){
  HC12.flush();
  HC12.end();
  pauseMe(800);
  command ? HC12.begin(9600) : HC12.begin(2400);
   pauseMe(800);
  while(HC12.available()) HC12.read();        // empty  out possible garbage
  return command;
}


void pauseMe(uint16_t holdOff){
  long long pause = millis();
  do {} while (millis() - pause < holdOff);
}

void goPowerOff(void){
  wipeOLED();
  u8x8.draw2x2String(0, 2, "POWERING");
  u8x8.draw2x2String(0, 6, "  DOWN  ");
  clearMatrix();
  bright = 25;
  writeSplash(false);                                 // write a dimmed splash - as a reminder to turn off...
  bright = 255;
  HC12.flush();
  delay(tick);
  digitalWrite(offControlPin, HIGH);                  // send the power-off signal to the e-switch: Kills all.  
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



void checkEEPROM(byte lowAddress, byte eeAddress){
  Serial.println();
  Serial.println("EEPROM:  ");
  for (byte n = lowAddress; n < eeAddress+5; n++){
   Serial.print(n);
   Serial.print(":\t");
   if ( n <= 11) {
    Serial.print(nameParam[n]);
   }
   Serial.print("\t");
   Serial.println(EEPROM.read(n));               
  }
  unsigned long timer=millis();
  do{} while ((millis() - timer) < 200UL);
}





/*
 * Handle printing line numbers for debugging 
 */
void printDebugLine(uint16_t lineNo, const char* FileName){
  #ifdef DEBUG  
      Serial.print(F("We are at LINE:\t"));
      Serial.print(lineNo);
      Serial.print(F(" in TAB: "));
      Serial.println(FileName);
      Serial.print("continueOn :\t");
      Serial.println(continueOn);
      Serial.print("startOver :\t");
      Serial.println(startOver);
      Serial.print("sEcount :\t");
      Serial.println(sEcount);
      Serial.print("sE_iter :\t");
      Serial.println(sE_iter);    
      Serial.print("countPractice :\t");
      Serial.println(countPractice); 
      Serial.print("arrowCount :\t");
      Serial.println(arrowCount);
//      Serial.print("archerIndex :\t");
//      Serial.println(archerIndex);    
          
  #endif      
}


void goDemoLoop() {
  

  const char demoChar1[] = "Cuchulainn";
  const char demoChar2[] = "Archers";
  
  const char demoChar3[] = "We don't do";
  const char demoChar4[] = "'LUCK' !";
  const char demoChar5[] = "We";
  const char demoChar6[] = "AIM TRUE";
  const char demoChar7[] = "for the";
  const char demoChar8[] = "GOLD";
  const char demoChar9[] = "C";
  const char demoChar10 = 250;
  const char demoChar11[] = "Ab"; 
  
  for (;;) {
    clearMatrix();    
    writeSplash(false);
    timeOut(2000);
    clearMatrix();
    timeOut(2000);    
    HC12.print(F("font 11\r"));    HC12.flush();
    sendSerialS( orange, /*column=*/ 2, /*line=*/ 20, demoChar2);
    timeOut(2500);
    clearMatrix();    
    HC12.print(F("font 4\r"));    HC12.flush();
    sendSerialS( green, /*column=*/ 4, /*line=*/ 10, demoChar3);
    timeOut(800);
    HC12.print(F("font 10\r"));    HC12.flush();
    sendSerialS( red, /*column=*/ 2, /*line=*/ 30, demoChar4);
    timeOut(2000);
    clearMatrix();
    timeOut(500);
    HC12.print(F("font 4\r"));    HC12.flush();
    sendSerialS( green, /*column=*/ 25, /*line=*/ 10, demoChar5);    
    timeOut(500);
    HC12.print(F("font 9\r"));    HC12.flush();
    sendSerialS( green, /*column=*/ 2, /*line=*/ 28, demoChar6);
    timeOut(1500);
    clearMatrix();
    HC12.print(F("font 4\r"));    HC12.flush();
    sendSerialS( green, /*column=*/ 14, /*line=*/ 10, demoChar7);    
    timeOut(800);
    HC12.print(F("font 11\r"));    HC12.flush();
    sendSerialS( orange, /*column=*/ 9, /*line=*/ 28, demoChar8);
    timeOut(3000);
    clearMatrix();
    HC12.print(F("font 14\r"));    HC12.flush();
    for (byte n = 0; n <= 2; n++){
      for (byte i = 4; i<=45; i+=28){
      sendSerialS( orange, /*column=*/ i, /*line=*/ 30, demoChar9);
      sendChar( orange, /*column=*/ i + 15, /*line=*/ 30, demoChar10);
      }
      timeOut(500);
      clearMatrix();
      
      sendSerialS( green, /*column=*/ 10, /*line=*/ 30, demoChar11);
      sendChar( green, /*column=*/ 40, /*line=*/ 30, demoChar10);
      timeOut(500);
      clearMatrix();
    }
    timeOut(2000);       
    if (!demoMode) break;
    
  }
  clearMatrix(); 
  return;
}


void timeOut(long long intervalSet) {
  long long demoTimer = millis();  
 for(;;) {
  if (readButtons() == BUTTON4) {
    clearMatrix();
    demoMode = false;
    return;
    }
  if (millis() - demoTimer >=  intervalSet) break;
  }
}

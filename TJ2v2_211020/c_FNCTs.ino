
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
    clearMatrix
    writeSplash
    sendScrollW
    goWhistle
    stopSign
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

  pauseMe(tick);   
  HC12.print(F("font 1\r"));    HC12.flush();
  lnNumber = 5;
  
  if (paramStore.isFinals && !paramStore.isAlternating) {
    sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, "MATCHPLAY", "");
  }

  else if (paramStore.isFinals && paramStore.isAlternating) {
    sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, "FINALS", "");
  }

  else if (paramStore.teamPlay) {
    sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, "Team", "");
  }
  else {  
    if (paramStore.notFlint == 0){
      sendSerialS( /*colour(R1G2O3)=*/ 1, /*column=*/ 0, /*line=*/ lnNumber, "FLINT Round", ""); 
    }else{
      sendSerialS( /*colour(R1G2O3)=*/ 1, /*column=*/ 0, /*line=*/ lnNumber, 
      (paramStore.Details == 1 ? "Single " : "Double "), "Detail");         // either 1 OR 2
    }
  }
  pauseMe(tick);
  
  lnNumber += 7;                                                            // Practice?
  sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, "Practice");
  colNumber = 56; txtColour = orange;
  pauseMe(tick/4);
  sendNumber(paramStore.maxPrac);
  pauseMe(tick/4);  
  
  txtColour = green;
  lnNumber += 7;  // End duration
  sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, "END ","Time: ");
  colNumber = 48 +  ( startCounts[paramStore.startCountsIndex] < 10   ? 8 :       // centering the number on the matrix
                      startCounts[paramStore.startCountsIndex] < 20   ? 5 :
                      startCounts[paramStore.startCountsIndex] < 100  ? 3 : 
                      startCounts[paramStore.startCountsIndex] < 200  ? 0 : -2);
                      
  txtColour = orange;
  pauseMe(tick/4);
  sendNumber(startCounts[paramStore.startCountsIndex]);
  pauseMe(tick/4);  

  txtColour = green;
  lnNumber += 7;  // Number of Ends
  sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, "END ","Count: ");
  colNumber = 48 + (paramStore.maxEnds < 10 ? 8 : paramStore.maxEnds < 20 ? 5 : 3);
  txtColour = orange;
  pauseMe(tick/4);
  sendNumber(paramStore.maxEnds);
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

/*
 * Wipes the Matrix clear
 */
void clearMatrix(void){
  HC12.print(F("scrollloop 0\r"));                        //  if scroll in progress, kill it
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
  
  for (uint16_t b = 0; b <= bright; b += 10){                              // fade increment
    unsigned long timer = millis();
    HC12.print(F("brightness "));                                          //  | 
    HC12.print(b);                                                         //  | Fade in "Time"
    HC12.print(F("\r"));                                                   //  |
    HC12.print(F("paint\r"));      HC12.flush();  pauseMe(tock);
    HC12.print(F("font 16\r"));    HC12.flush();  pauseMe(tock);
    sendSerialS( /*colour=*/ 2, /*column=*/ 2, /*line=*/ 24, "Time");
    pauseMe(2*tock);
    do{} while (millis() - timer < 15UL);                                  // allow serial traffic time
    }
  pauseMe(10);
  
  HC12.print(F("font 9\r"));    
  HC12.flush();
  if (scrolling){
    pauseMe(tock);
    sendScrollW(  /*speed=*/  4,                                            // 3 - 15
                  /*loop=*/   0,
                  /*wiggle=*/ 2,
                  /*colour=*/ 3,
                  /*column=*/ 4, 
                  /*line=*/   30,
                  /*window=*/ 63, 
                            "JUDGE" );  // animation of JUDGE
  } else {
    const char scrollChar[] = "JUDGE";                                      // no animation
    for (uint8_t iter = 0; iter < strlen(scrollChar); iter ++) {
      HC12.print(F("font 9\r"));     HC12.flush();  pauseMe(tock);             // Font for the static letter post-scroll    
      HC12.print(F("text "));      
      HC12.print(orange);      
      HC12.print(F(" "));      
      HC12.print(4 + 12*iter);               // x pos for text      
      HC12.print(F(" "));      
      HC12.print(30);                        // y pos      
      HC12.print(F(" "));
      HC12.print('"');
      HC12.print(scrollChar[iter]);
      HC12.print('"');
      HC12.print(F("\r"));    HC12.flush();  pauseMe(tock);       
      HC12.print(F("paint\r"));   HC12.flush();   pauseMe(tock); 
      pauseMe(5);
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
  uint8_t delVal = scrollSpeed;

  HC12.printf(
    F("scrollspeed %u \r scrollloop %u \r scrollwiggle %u \r"),
      scrollSpeed, scrollLoop, scrollWiggle);
  HC12.flush();  pauseMe(tock);    
  
  for (uint8_t iter = 0; iter < strlen(scrollChar); iter ++){
    unsigned long timer = millis();
    HC12.print(F("font 17\r"));    HC12.flush();  pauseMe(tock);
    HC12.print(F("scroll "));
    HC12.print(txtColour1);
    HC12.print(F(" "));
    HC12.print(x1 + 12*iter + 4);           // x pos for window lhs corner
    HC12.print(F(" "));
    HC12.print(y1);                         // y pos for window lhs bottom
    HC12.print(F(" "));
    HC12.print(window1);                    // Scroll window width
    HC12.print(F(" "));
    HC12.print('"');
    HC12.print(scrollChar[iter]);           //  Send each letter in turn
    HC12.print('"');
    HC12.print(F("\r"));    HC12.flush(); pauseMe(tock);
    HC12.print(F("paint\r"));    HC12.flush();  pauseMe(tock);
    do{} while (millis() - timer < (delVal*100));    // 800UL
    HC12.print(F("font 9\r"));     HC12.flush();  pauseMe(tock);              // Font for the static letter post-scroll
    
    HC12.print(F("text "));
    
    //HC12.print(iter == 1 ? green : orange);
    HC12.print(orange);
    
    HC12.print(F(" "));
    
    HC12.print(x1 + 12*iter);               // x pos for text
    
    HC12.print(F(" "));
    
    HC12.print(y1);                         // y pos
    
    HC12.print(F(" "));
    HC12.print('"');
    HC12.print(scrollChar[iter]);
    HC12.print('"');
    HC12.print(F("\r"));    HC12.flush();  pauseMe(tock);
    
    HC12.print(F("paint\r"));    HC12.flush();  pauseMe(tock);
    pauseMe(5);
  }
  pauseMe(tick);
}


void goWhistle(uint8_t whistles){
  HC12.print("~"); HC12.flush();              //uncomment for whistles
  HC12.print(whistles);  
  HC12.flush();
  //pauseMe(30 );                                 
}


void stopSign(void){
  /*
   * (circle colour x_centre y_centre radius)
   */
  clearMatrix();
  HC12.print(F("circle "));                // Draw a circle
  HC12.print(red);
  HC12.print(F(" 32 15 15\r"));
  HC12.print(F("paint\r"));    HC12.flush();
  HC12.print(F("circle "));
  HC12.print(red);
  HC12.print(F(" 32 15 14\r"));
  HC12.print(F("paint\r"));    HC12.flush();
  HC12.print(F("circle "));
  HC12.print(red);
  HC12.print(F(" 32 15 13\r"));
  HC12.print(F("paint\r"));    HC12.flush();

  
  /*
   * line colour x1 y1 x2 y2
   * Draws a line between x1, y1 and x2, y2 in the specified colour
   */ 
  
  HC12.print(F("line "));                 //  Now bisect the circle
  HC12.print(red);
  HC12.print(F(" 41 6 23 24\r"));
  HC12.print(F("paint\r"));    HC12.flush();
  HC12.print(F("line "));
  HC12.print(red);
  HC12.print(F(" 42 7 24 25\r"));
  HC12.print(F("paint\r"));    HC12.flush();
  HC12.print(F("line "));
  HC12.print(red);
  HC12.print(F(" 43 8 25 26\r"));
  HC12.print(F("paint\r"));    HC12.flush();
}


/*
 * Reset the game to initial settings depending on
 * parameters set.
 */
void zeroSettings(void){
  shootDetail = 0;   
  sEcount = 1;
  continueOn = 0;                  
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
  //HC12.print("\r");
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
  pauseMe(2);                                           // delay for level de-bouncing; 2nd read to confirm low
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
 * user input, and there is nothing to do until user press some button
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
      while (readButtons() != 0) delay(1);                  // and now wait for him to release the button
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
      } else if (n_Count != 0){
        //goWhistle(1);
        writeOLED_Data(1);
      }
    } else if (AIndex == 3){                                // for in-barCount !STOP!
      displayParamsOnOLED();
      writeOLED_Data(1);
      ret = true;
    } else if (AIndex == 4){
                                                            //saved for future use
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
      clearMatrix(); //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    break;

    case BUTTON3:         
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
      if (i == selectionIdx)
        u8x8.print("> ");
      else 
        u8x8.print("  ");

      u8x8.print(menu0[i]);
    }
  }  
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

void pauseMe(uint16_t holdOff){
  long long pause = millis();
  do {} while (millis() - pause < holdOff);
}
//=========================================================================================
/*  DEBUGGING TOOLS  */

void checkEEPROM(byte lowAddress, byte eeAddress){
  Serial.println();
  Serial.println("EEPROM:  ");
  for (byte n = lowAddress; n < eeAddress+5; n++){
   Serial.print(n);
   Serial.print(":  ");
   Serial.println(EEPROM.read(n));               
  }
  unsigned long timer=millis();
  do{} while ((millis() - timer) < 200UL);
}





/*
 * Handle printing line numbers for debugging 
 */
void printDebugLine(uint16_t lineNo){
  #ifdef DEBUG  
      Serial.print(F("We are at LINE: "));
      Serial.println(lineNo);
  #endif      
}

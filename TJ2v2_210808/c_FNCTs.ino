
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
    checkEEPROM
    printDebugLine
    
*/

 
 
/*
 * Write info regarding tournament setup
 */
void writeInfoBigscreen(void){
  clearMatrix();

  delay(tick);   
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
      (paramStore.Details == 1 ? "Single " : "Double "), "Detail");                   // either 1 OR 2
    }
  }
  delay(tick);
  
  lnNumber += 7;                                                                      // Practice?
  sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, "Practice");
  colNumber = 56, txtColour = orange;
  delay (tick/4);
  sendNumber(paramStore.maxPrac);
  delay(tick/4);  
  
  txtColour = green;
  lnNumber += 7;                                                                      // End duration
  sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, "END ","Time: ");
  colNumber = 48 +  ( startCounts[paramStore.startCountsIndex] < 10   ? 8 :           // centering the number on the matrix
                      startCounts[paramStore.startCountsIndex] < 20   ? 5 :
                      startCounts[paramStore.startCountsIndex] < 100  ? 3 : 
                      startCounts[paramStore.startCountsIndex] < 200  ? 0 : -2);
                      
  txtColour = orange;
  delay (tick/4);
  sendNumber(startCounts[paramStore.startCountsIndex]);
  delay(tick/4);  

  txtColour = green;
  lnNumber += 7;                                                                      // Number of Ends
  sendSerialS( /*colour(R1G2O3)=*/ 2, /*column=*/ 0, /*line=*/ lnNumber, "END ","Count: ");
  colNumber = 48 + (paramStore.maxEnds < 10 ? 8 : paramStore.maxEnds < 20 ? 5 : 3);
  txtColour = orange;
  delay(tick/4);
  sendNumber(paramStore.maxEnds);
    delay(5*tick);
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
  delay(4*tock);
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
    HC12.print(F("paint\r"));      HC12.flush();  delay(tock);
    HC12.print(F("font 16\r"));    HC12.flush();  delay(tock);
    sendSerialS( /*colour=*/ 2, /*column=*/ 2, /*line=*/ 24, "Time");
    delay(2*tock);
    do{} while (millis() - timer < 15UL);                                  // allow serial traffic time
    }
  delay(10);
  
  HC12.print(F("font 9\r"));    
  HC12.flush();
  if (scrolling){
    delay(tock);
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
      HC12.print(F("font 9\r"));     HC12.flush();  delay(tock);             // Font for the static letter post-scroll    
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
      HC12.print(F("\r"));    HC12.flush();  delay(tock);       
      HC12.print(F("paint\r"));   HC12.flush();     delay(tock); 
      delay(5);
    }
  }
  delay(tock);
  
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
//  HC12.print(F("scrollspeed "));
//  HC12.print(scrollSpeed);
//  HC12.print(F("\r"));  HC12.flush();    delay(tock); 
//  HC12.print(F("scrollloop "));
//  HC12.print(scrollLoop);
//  HC12.print(F("\r"));  HC12.flush();    delay(tock);
//  HC12.print(F("scrollwiggle "));
//  HC12.print(scrollWiggle);
//  HC12.print(F("\r"));    HC12.flush();  delay(tock);

  HC12.printf(
    F("scrollspeed %u \r scrollloop %u \r scrollwiggle %u \r"),
      scrollSpeed, scrollLoop, scrollWiggle);
  HC12.flush();  delay(tock);    
  
  for (uint8_t iter = 0; iter < strlen(scrollChar); iter ++){
    unsigned long timer = millis();
    HC12.print(F("font 17\r"));    HC12.flush();  delay(tock);
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
    HC12.print(F("\r"));    HC12.flush();  delay(tock);
    HC12.print(F("paint\r"));    HC12.flush();  delay(tock);
    do{} while (millis() - timer < (delVal*100));    // 800UL
    HC12.print(F("font 9\r"));     HC12.flush();  delay(tock);              // Font for the static letter post-scroll
    
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
    HC12.print(F("\r"));    HC12.flush();  delay(tock);
    
    HC12.print(F("paint\r"));    HC12.flush();  delay(tock);
    delay(5);
  }
  delay(tick);
}


void goWhistle(uint8_t whistles){
  HC12.print("~"); HC12.flush(); 
  HC12.print(whistles);  //uncomment for whistles
  HC12.flush();
  delay(30 );                                 //  You would not believe how needed this is!!!!
}


void stopSign(void){
  /*
   * (circle colour x_centre y_centre radius)
   */
  clearMatrix();
  HC12.print(F("circle2 "));                // Draw a circle
  HC12.print(red);
  HC12.print(F(" 32 15 15\r"));
  HC12.print(F("paint\r"));    HC12.flush();
  HC12.print(F("circle2 "));
  HC12.print(red);
  HC12.print(F(" 32 15 14\r"));
  HC12.print(F("paint\r"));    HC12.flush();
  HC12.print(F("circle2 "));
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
  delay(100);
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

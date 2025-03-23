/*
 16   writeSplash
 54   sendChar
 68   sendScrollW
107   clearMatrix
117   stopSign
126   writeCircle
132   writeRectangle
139   writeLine
145   redBorder
159   score_Collect
173   pauseMe
178   testMe
*/
/* Splashscreen of Logo "Time Judge"*/
void writeSplash(bool scrolling){                                          // to localise to Matrix send $£ (false) or $% (true)
  byte pMe = (4);
  clearMatrix(false);
  #ifdef DEBUG
    MATRIXSER.print(F("font 8\r"));    MATRIXSER.flush();
    sendSerialS( green, /*column=*/ 2, /*line=*/ 24, "[ SPLASH ]");
  return;
  #endif
  MATRIXSER.print(F("font 16\r"));    MATRIXSER.flush();
  for (uint16_t br = 0; br <= screenBright; br += 17){                           // fade increment [10]
    MATRIXSER.printf(
          F("brightness %u\rpaint\r"),                                     //  | Fade in "Time"
            br);
    sendSerialS( green, /*column=*/ 2, /*line=*/ 24, "Time");
    pauseMe(50);
    }
  const char scrollChar[] = "JUDGE";
  MATRIXSER.print(F("font 9\r"));     MATRIXSER.flush();    
  pauseMe(pMe);
  if (scrolling){
    pauseMe(pMe);
    sendScrollW(  /*speed=*/  6, //5                                          // 3 - 15
                  /*loop=*/   0,
                  /*wiggle=*/ 3,
                  /*colour=*/ orange,
                  /*column=*/ 4, 
                  /*line=*/   31, //30
                  /*window=*/ 63, 
                            scrollChar );                                   // animate of JUDGE
  } else {
    for (uint8_t iter = 0; iter < strlen(scrollChar); iter ++) {
      sendChar(orange, (4 + 12*iter), 30, scrollChar[iter]);
      pauseMe(100);
    }
  }
  pauseMe(tock);
}

void sendChar(byte chCol, byte chX, byte chY, byte chChar){
  MATRIXSER.printf(
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
  MATRIXSER.flush();  pauseMe(tock);

  for (uint8_t iter = 0; iter < strlen(scrollChar); iter ++){
    unsigned long timer = millis();
    MATRIXSER.print(F("font 17\r"));    
    pauseMe(20);
    MATRIXSER.printf(
          F("scroll %u %u %u %u \"%c\"\rpaint\r"),
              txtColour1, (x1 + 12*iter + 1/*+4*/), y1, window1, scrollChar[iter]);
    do{} while (millis() - timer < (scrollSpeed*130));    // 800UL  Allow the letter to run, then do the static
    MATRIXSER.print(F("font 9\r"));                       // Font for the static letter post-scroll
    sendChar(orange, (x1 + 12*iter), y1, scrollChar[iter]);
    pauseMe(30);
  }
  pauseMe(tick);
}




/*
 * Wipes the Matrix clear
 */
void clearMatrix(bool scrollOn){
  if (scrollOn) {
    MATRIXSER.print(F("scrollloop 0\r"));                                      //  if scroll in progress, kill it
    pauseMe(4*tock);
  }
  MATRIXSER.print(F("clear\r"));
  MATRIXSER.print(F("paint\r"));
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
    MATRIXSER.printf(
        F("circle %u %u %u %u\rpaint\r"),
            cCol1, cX1, cY1, cR1);
}

void writeRectangle(byte rCol, byte rX1, byte rY1, byte rWid, byte rHi ){
  MATRIXSER.printf(
    F("rect %u %u %u %u %u \rpaint\r"),
      rCol, rX1, rY1, rWid, rHi);
      MATRIXSER.flush();
}

void writeLine(byte lCol, byte lX1, byte lY1, byte lX2, byte lY2) {
  MATRIXSER.printf(
    F("line %u %u %u %u %u \rpaint\r"),                             //  line colour, x1(), y1(), x2(), y2()
      lCol, lX1, lY1, lX2, lY2);
}

void redBorder(bool borderOn){
  if (borderOn == true) {
    clearMatrix(false);
    writeRedscreen();                                 // Fill screen with red
    writeRectangle(0, 1, 30, 62, 30);                 // blank centre
    MATRIXSER.print(F("font 13\r"));                  // large numbers font
    MATRIXSER.flush();
  } else {
    writeRectangle(2, 0, 31, 64, 32);
    pauseMe(100);
    clearMatrix(false);
  }
}

void writeRedscreen(void){
   writeRectangle( 1, 0, 31, 64, 32);                 // fill screen with red
}

void score_Collect(bool redScreen){
  if (redScreen) {
    writeRedscreen();                                  // fill screen with red
    pauseMe(2*tick);
  }
  clearMatrix(false);
  MATRIXSER.print(F("font 9\r"));   
  sendSerialS(2, 10, 15, "SCORE +");
  //MATRIXSER.flush();
  pauseMe(30);
  sendSerialS(2, 5, 29, "COLLECT");
  //MATRIXSER.flush();
}

void pauseMe(uint16_t holdOff){
  long long pause = millis();
  do {} while (millis() - pause < holdOff);
}

void testMe(byte times, byte freq) {
  #ifdef DEBUG 
    byte iter = 0;
    do {
      digitalWrite(whistlePin, HIGH);             // start the whistle
      pauseMe(10); 
      digitalWrite(whistlePin, LOW);
      pauseMe(10*freq); 
      } while ( ++ iter < times );
  #endif    
}
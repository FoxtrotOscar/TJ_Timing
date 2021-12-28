/*
 * Take a string from the HC12 receiver and parse it
 */

bool recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\r';
  char rc;
  byte pMe = 2;
  while (HC12.available() > 0 && newData == false) {
    pauseMe(pMe);
    rc = HC12.read();
    
    switch (rc) {
      case '^':
        while (!HC12.available()) {}            // hang around until a char is received
        pauseMe(pMe);
        if (HC12.read() == '9') reboot();       //digitalWrite(rebootPin, LOW);
        
        configMatrix(sWidth, sHeight, sType, sBPP, sInv, sEnabA, sBright);
        digitalWrite    (setupPin, LOW); 
        pauseMe(800);
        showParam('^');                          // WRITE THE CHANNEL NO. ON THE SCREEN
        rc = 0;
        newData = false;
      break;
      
      case '~':                                 // marker for whistle - count follows
        while (!HC12.available()) {}            // hang around until a char is received
        pauseMe(pMe);
        rc = HC12.read();  
        pauseMe(pMe);
        if (   ( rc >  '0') 
            && ( rc <= '5')) {
          numWhistles = (rc - '0');             // converts symbol '0' .. '9' to number 0..9.
        }
        rc = 0;
        n = numWhistles ;
        whistleClock = millis();
        digitalWrite(whistlePin, HIGH);         // fire the (first) whistle immediately
        whistleFlag = true;
        newData = false;
      break;

      case '*':                                 // marker for setup of the HC11 - confirm follows
        while (!HC12.available()) {}            // hang around until a char is received
        pauseMe(pMe);
        rc = HC12.read();
        pauseMe(pMe);
        if (rc == '^' || rc == '&')  {
          goSetParam(rc);
        }
        newData = false;
      break;

      case '$':                                 // marker for setup of the HC11 - confirm follows
        while (!HC12.available()) {}            // hang around until a char is received
        pauseMe(pMe);
        rc = HC12.read();
        pauseMe(pMe);
        switch (rc){
          case '%':
          writeSplash(true);
          break;
          case '£':
          writeSplash(false);
          break;
        }
        newData = false;
      break;
      

      default:
        if (rc != endMarker) {
          receivedChars[ndx] = rc;              // fill the buffer
          ndx++;
          ndx = (ndx >= numChars ? numChars-1 : ndx);
          newData = false;
        } else {
          receivedChars[ndx] = endMarker;       // terminate the buffer string
          ndx = 0;
          newData = true;
        }
      break;
    }
  }
  return newData;
}


void pauseMe(uint16_t holdOff){
  long long pause = millis();
  do {} while (millis() - pause < holdOff);
}

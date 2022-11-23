/*
 * Take a string from the HC12 receiver and parse it
 */

bool recvWithEndMarker() {
  static byte     ndx       = 0;
  char            endMarker = '\r';
  char            rc;
  byte            pMe       = 2;
  
  while (_RADIO.available() > 0 && newData == false) {
    pauseMe(pMe);
    rc = _RADIO.read();
    
    switch (rc) {
      case '^':                                   // initiator for Splash, Score & Collect, Dim screen, Reboot
        while (!_RADIO.available()) {}            // hang around until a char is received
        pauseMe(pMe);
        rc = _RADIO.read();
        pauseMe(pMe);
        switch (rc) {
          case '0':
            writeSplash(false);
            break;

          case '1':
            writeSplash(true);
            break;

          case '2':
            score_Collect(false);
            break;

          case '3':
            score_Collect(true);
            break;

          case '8':                                 // dim the screen to val Low/Med/Bright given
            while (!_RADIO.available()) {}          // hang around until a char is received
            pauseMe(pMe);
            rc = _RADIO.read();
            screenBright = (rc == 'L'? 25 : rc == 'M'? 100 : sBright);
              MATRIXSER.print(F("brightness "));
              MATRIXSER.print(screenBright);
              MATRIXSER.print(F("\r"));
            break;

          case '9':
            reboot();                              //digitalWrite(rebootPin, LOW);
            configMatrix(sWidth, sHeight, sType, sBPP, sInv, sEnabA, sBright);
            digitalWrite    (setupPin, LOW); 
            pauseMe(800);
            showParam();                          // WRITE THE CHANNEL NO. ON THE SCREEN
            break;
        }
        rc = 0;
        newData = false;
      break;
      
      case '~':                                   // initiator for whistle - count follows
        while (!_RADIO.available()) {}            // hang around until a char is received
        pauseMe(pMe);
        rc = _RADIO.read();  
        pauseMe(pMe);
        if (   ( rc >  '0') 
            && ( rc <= '5')) {
          numWhistles = (rc - '0');               // converts symbol '0' .. '9' to number 0..9.
        }
        rc = 0;
        whistleCt = numWhistles ;
        whistleClock = millis();
        digitalWrite(whistlePin, HIGH);           // fire the (first) whistle immediately
        whistleFlag = true;
        newData = false;
      break;

      case '*':                                   // initiator for setup of the HC12 channel
        while (!_RADIO.available()) {}            // hang around until a char is received
        pauseMe(pMe);
        rc = _RADIO.read();
        pauseMe(pMe);
        if (rc == '^'){
          goSetParam(rc);
        }
        newData = false;
      break;

      case '$':                                   // initiator for Matrix wipe, red border,  
        while (!_RADIO.available()) {}            // hang around until a char is received
        pauseMe(pMe);
        rc = _RADIO.read();
        pauseMe(pMe);
        switch (rc){
          case '0':
            clearMatrix(false);
            break;         
          case '1':
            clearMatrix(true);
            break;
          case '2':
            redBorder(false);
            break;
          case '3':
            redBorder(true);
            break;
          case '4':
          case '5':
          case '6':
          case '7':
          case '8':
          break;
        }

        newData = false;
      break;

      default:
        if (rc != endMarker) {
          receivedChars[ndx] = rc;              // No initiators so fill the buffer until an end marker received
          ndx++;
           if (ndx >= numChars) ndx = numChars - 1;
          newData = false;
        } else {
          receivedChars[ndx] = endMarker;       // terminate the buffer string
          ndx = 0;
          newData = true;
        }
      break;
    }
  }
  return newData;                               // return with the received string and a flag to say you have mail...
}



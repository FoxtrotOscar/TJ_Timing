/*
 * Take a string from the HC12 receiver and parse it
 */

bool recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\r';
  char rc;
 
  while (HC12.available() > 0 && newData == false) {
    rc = HC12.read();
    
    switch (rc) {
      case '^':
        while (!HC12.available()) {}            // hang around until a char is received
        delay(1);
        if (HC12.read() == '9') digitalWrite(rebootPin, LOW);
        
        delay(200);
        
        digitalWrite(rebootPin, HIGH);
        
        configMatrix();
        digitalWrite    (setupPin, LOW); 
        delay(3*tock);

        showChannel();                          // WRITE THE CHANNEL NO. ON THE SCREEN
        delay(tock);
        rc = 0;
        newData = false;
      break;
      
      case '~':                                 // marker for whistle - count follows
        while (!HC12.available()) {}            // hang around until a char is received
        delay(1);
        rc = HC12.read();  

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
        rc = HC12.read();
        delay(2);
        if (rc == '^')  {
          goSetChan();
        }
        newData = false;
      break;

      default:
        if (rc != endMarker) {
          receivedChars[ndx] = rc;              // fill the buffer
          ndx++;
          if (ndx >= numChars) {
            ndx = numChars - 1;
          }
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

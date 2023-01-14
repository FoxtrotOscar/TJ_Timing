 /*
 12   showParam
 59   goSetParam
 99   command_ON
*/

/*
 * Manipulate the HC12 channel based upon data received from HC12
 */


void showParam(void){
  MATRIXSER.print("font 6\r");                      
  pauseMe(25);                                  
  clearMatrix(false);
  sendSerialS(2, 3, 20,"CHANNEL: ");
  pauseMe(50);
  char endMarker = '\n';
  char rc;
  command_ON(true);                                 // enter Command mode
  colNumber = 46;
  byte pMe = 4;
  for (;;) {
    byte ptr = 0;
    bool read_error  = false;    
    bool flag        = false;
    _RADIO.print("AT+RC\n");
    _RADIO.flush();
    do {} while (!_RADIO.available());
    while (_RADIO.available()) {        
      while (!flag && !read_error){                   // typical response: "OK+C021"
        rc = _RADIO.read();                           // keep reading until '+' returns
        pauseMe(pMe);  
        flag        = (rc == '+');                    // means we have a good response, exit loop
        read_error  = (rc == 'E');                    // means we have a bad  response, exit loop
      }
      if (read_error) break;                          // skip the rest and re-enter the loop
      pauseMe(pMe);
      rc = _RADIO.read();                             // get the next char
      pauseMe(pMe);
      if (rc && (rc != endMarker)) {                  // grab max 3 chars from HC12 until end marker
        if ((rc >= '0') && (rc <= '9') && (ptr < 3)){
          rc = (rc - '0');
          sendNumber(green, colNumber, 20, rc);
          colNumber +=5;
          ptr++;
        }
      }
    }
    if (!read_error) break;                           // done
  }
  command_ON(false);                                  // Exit command mode
  pauseMe(20);
}




void goSetParam(char newParam) {
  byte pMe = 20;
  HC12End = false;
  HC12ReadBuffer.reserve(64);                         // Reserve 64 bytes for Serial message input
  do{
    HC12ByteIn = _RADIO.read();
    pauseMe(pMe);
  } while (HC12ByteIn !='A');
  HC12ReadBuffer += char(HC12ByteIn);
  while (_RADIO.available() && !HC12End) {            // While Arduino's HC12 soft serial rx buffer has data
    HC12ByteIn = _RADIO.read();                       // Store each character from rx buffer in byteIn
    pauseMe(pMe);
    HC12ReadBuffer += char(HC12ByteIn);               // Write each character of byteIn to HC12ReadBuffer
    if (HC12ByteIn == '\n') {                         // At the end of the line
      HC12End = true;                                 // Set HC12End flag to true
    }
  }
  MATRIXSER.print("font 6\r");              
  delay(2*tock);
  if (HC12End) {                                      // If HC12End flag is true
    digitalWrite(setupPin, LOW);                      // Enter command mode
    pauseMe(800);
    if (HC12ReadBuffer.startsWith("AT")) {            // Check to see if it was a good command
      clearMatrix(false);
      sendSerialS(3, 2, 15, "GOOD COMMAND");
      pauseMe(tick);
      clearMatrix(false);
      _RADIO.println(HC12ReadBuffer);                 // Write command to local HC12
      _RADIO.flush();
      
      showParam();
    } else sendSerialS(1, 2, 15, "BAD COMMAND");
    HC12ReadBuffer = "";                              // Empty buffer
    HC12End = false;                                  // Reset flag
  }
  digitalWrite(setupPin, HIGH);
  delay(3*tock);
}


void command_ON(bool command){                        // enter and exit HC12 Command mode
  digitalWrite(setupPin, command ? LOW : HIGH);       // COMMAND MODE
  pauseMe(80);                                        // settle
  while(_RADIO.available()) _RADIO.read();            // empty  out possible garbage
}

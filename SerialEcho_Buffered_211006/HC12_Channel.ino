/*
 * Manipulate the HC12 channel based upon data received from HC12
 */


void showChannel(void){    
  MATRIXSER.print("font 6\r");                      
  delay(2*tock);                                  
  clearMatrix();
  static byte ndx       = 0;
  char        endMarker = '\n';
  char        rc        = 0;
  bool flag             = false;
  bool read_error       = false; 
 
  sendSerialS(2, 3, 20, "CHANNEL: ");
  colNumber = 46;

  for (;;) {
    HC12.print("AT+RC\n");                        // query HC12 for channel number
    HC12.flush();
    do {} while (!HC12.available());    
    byte ptr = 0;    
    read_error = false;
    while (HC12.available()) {        
      while (!flag && !read_error){               // typical response: "OK+C021"
        rc    = HC12.read();                      // keep reading until '+' returns
        delay(20);        
        flag  = (rc == '+');                      // means we have a good response, exit loop
        read_error = (rc == 'E');                 // means we have a bad  response, exit loop
      }
      if (read_error) break;                      // exit to next iteration of (;;)
      delay(2);
      rc = HC12.read();                           // get the next char
      delay(2);
      if ((rc != endMarker) ) {                   // grab max 3 chars from HC12 until end marker
        if ((rc >= '0') && (rc <= '9') && (ptr < 3)){
          rc = (rc - '0');
          sendNumber(2, colNumber, 20, rc);
          colNumber +=5;
          ptr++;
        }
      }
    }
    if (!read_error) break;                       // no errors - we are done, else go back to (;;)
  }

  digitalWrite    (setupPin,    HIGH);            // PC15: set inactive
}


void goSetChan() {
  HC12End = false;
  HC12ReadBuffer.reserve(64);                     // Reserve 64 bytes for Serial message input
  do{
    HC12ByteIn = HC12.read();
    delay(2);
  } while (HC12ByteIn !='A');
  HC12ReadBuffer += char(HC12ByteIn);
  while (HC12.available() && !HC12End) {          // While Arduino's HC12 soft serial rx buffer has data
    HC12ByteIn = HC12.read();                     // Store each character from rx buffer in byteIn
    delay(1);
    HC12ReadBuffer += char(HC12ByteIn);           // Write each character of byteIn to HC12ReadBuffer
    if (HC12ByteIn == '\n') {                     // At the end of the line
      HC12End = true;                             // Set HC12End flag to true
    }
  }
  MATRIXSER.print("font 6\r");              
  delay(2*tock);
  if (HC12End) {                                  // If HC12End flag is true
    if (HC12ReadBuffer.startsWith("AT")) {        // Check to see if a command is received from remote
      clearMatrix();
      
      sendSerialS(3, 2, 15, "GOOD COMMAND");
      digitalWrite(setupPin, LOW);                // Enter command mode
      delay(1000);                                // Delay before sending command
      HC12.print(HC12ReadBuffer);                 // Write command to local HC12
      delay(500);                                 // Wait 0.5 s for reply
      showChannel();
    } else sendSerialS(1, 2, 15, "BAD COMMAND");
    HC12ReadBuffer = "";                          // Empty buffer
    HC12End = false;                              // Reset flag
  }
  digitalWrite(setupPin, HIGH);
  delay(3*tock);
}

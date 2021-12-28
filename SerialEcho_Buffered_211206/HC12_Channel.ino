/*
 * Manipulate the HC12 channel based upon data received from HC12
 */


void showParam(char newParam){
  MATRIXSER.print("font 6\r");                      
  delay(2*tock);                                  
  clearMatrix();
  char endMarker = '\n';
  char rc        = 0;
  sendSerialS(2, 3, 20,(newParam == '^' ?  "CHANNEL: " : "BAUD"));
  colNumber = 46;
  byte pMe = 10;
  
  for (;;) {
    bool flag = false;
    bool read_error = false;
    byte ptr = 0;
    while(HC12.available()) HC12.read();          // empty  out possible garbage
    HC12.println(newParam == '^' ? "AT+RC" : "AT+RB");                        // query HC12 for channel number
    do {} while (!HC12.available());    

    while (HC12.available()) {        
      while (!flag && !read_error){               // typical response: "OK+C021"
        rc = HC12.read();                         // keep reading until '+' returns
        pauseMe(pMe);        
        flag  = (rc == '+');                      // means we have a good response, exit loop
        read_error = (rc == 'E');                 // means we have a bad  response, exit loop
      }
      if (read_error) break;                      // exit to next iteration of (;;)
      delay(2);
      rc = HC12.read();                           // get the next char
      //pauseMe(pMe);
      if (rc && (rc != endMarker)) {              // grab max 3 chars from HC12 until end marker
        if ((rc >= '0') && (rc <= '9') && (ptr < 6)){
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
  pauseMe(800);                               
}


void goSetParam(char newParam) {
  byte pMe = 20;
  HC12End = false;
//  digitalWrite(whistlePin, HIGH);
//  pauseMe(1000);
//  digitalWrite(whistlePin, LOW);
  do{
    HC12ByteIn = HC12.read();
    pauseMe(pMe);
  } while (HC12ByteIn !='A');
  HC12ReadBuffer += char(HC12ByteIn);
  while (HC12.available() && !HC12End) {          // While Arduino's HC12 soft serial rx buffer has data
    HC12ByteIn = HC12.read();                     // Store each character from rx buffer in byteIn
    pauseMe(pMe);
    HC12ReadBuffer += char(HC12ByteIn);           // Write each character of byteIn to HC12ReadBuffer
    if (HC12ByteIn == '\n') {                     // At the end of the line
      HC12End = true;                             // Set HC12End flag to true
    }
  }
  MATRIXSER.print("font 6\r");              
  delay(2*tock);
  if (HC12End) {                                  // If HC12End flag is true
    digitalWrite(setupPin, LOW);                  // Enter command mode
    pauseMe(800);
    if (HC12ReadBuffer.startsWith("AT")) {        // Check to see if it was a good command
      clearMatrix();
      sendSerialS(3, 2, 15, "GOOD COMMAND");
      pauseMe(1000);
      
      //commandBaudRate(true);
      HC12.println(HC12ReadBuffer);                 // Write command to local HC12
      HC12.flush();
      //pauseMe(800);
      //commandBaudRate(false);
      
      showParam('^');
    } else sendSerialS(1, 2, 15, "BAD COMMAND");
    HC12ReadBuffer = "";                          // Empty buffer
    HC12End = false;                              // Reset flag
  }
  //commandBaudRate(false);
  digitalWrite(setupPin, HIGH);
  delay(3*tock);
}


bool commandBaudRate(bool command){
  HC12.flush();
  HC12.end();
  pauseMe(800);
  HC12.begin(command ? 9600 : 2400);
  //pauseMe(800);
  while(!HC12) {;}
  while(HC12.available()) HC12.read();        // empty  out possible garbage
  return command;
}

//bool setBaudRate(newBaud){
//  
//}

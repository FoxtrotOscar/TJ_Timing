/*
 * HC12 433MHz RADIO DEVICE 
 * Read the current channel, write to p_Store and display
 *
 * 21   readChannel
 * 69   writeChannel_OLED
 * 89   alterChannelWarning
 * 132  new_Channel
 * 191  change_Channel
 * 201  set_A
 * 212  set_B
 * 222  clearAB
 * 235  setControlChannel
 * 268  setB_Chan
 * 473  writeRemoteChannel
 * 488  makeControlString
 */



void readChannel(void){
  digitalWrite (HC12SetPin, LOW);                           // enable Control Mode locally
  pauseMe(1000);
  p_Store.curChan = 0;
  char endMarker = '\n';                                    // a return (EOL) char at end of data from HC12
  char rc;                                                  // data returned from HC12 interrogation
  bool flag;                                                // to mark a read     
  bool read_error;                                          // to mark bad data
  char temp_str[4];
  byte u8x8ColNumber = 10;
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  temp_str[3] = '\0';
  byte pMe = 20;

  for (;;) {
    byte ptr = 0;
    flag = false;    
    read_error = false;
    while(HC12.available()) HC12.read();                    // empty  out possible garbage
    HC12.println("AT+RC");                                  // query HC12 for channel number
    do {} while (!HC12.available());    

    while (HC12.available()) {
      while (!flag && !read_error){                         // typical response: "OK+C021"
        rc = HC12.read();                                   // keep reading until '+' returns
        delay(pMe);        
        flag  = (rc == '+');                                // means we have a good response, exit loop
        read_error = (rc == 'E');                           // means we have a bad  response, exit loop
      }
      if (read_error) break;                                // exit to next iteration of (;;)
      rc = HC12.read();                                     // get the next char
      if (rc && (rc != endMarker)) {                        // grab max 3 chars from HC12 until end marker
        if ((rc >= '0') && (rc <= '9') && (ptr < 3)){
        p_Store.curChan += (ptr > 0 ? (ptr == 1 ? 
        ((rc - '0') *10) : (rc - '0')) : (rc - '0')*100);
        temp_str[ptr++] = rc;                               // set temp pointer to rc and increment pointer
        }
      }
    }
    if (!read_error) break;                                 // no errors - we are done, else go back to (;;)
  }                                                         // and re-commence query
  
  u8x8.draw2x2String(u8x8ColNumber++, 4, temp_str);
  digitalWrite (HC12SetPin, HIGH);                          // HC12 to TRANSPARENT mode to DISable writing (IF NOT)
  pauseMe(800);                 
  
}

void writeChannel_OLED (int channel){
  u8x8.inverse();
  u8x8.setCursor(1, 5);
  u8x8.print("CH:");
  u8x8.setCursor(5, 5);
  u8x8.print("  ");
  u8x8.setCursor((channel >9 ? 5 : 6), 5) ;
  u8x8.print(channel);
  u8x8.noInverse();
  u8x8.setCursor(8, 5);
  float chFrq = 433.4+(0.4*(channel-1.0));
  u8x8.print(chFrq);
  u8x8.setCursor(13, 5);
  u8x8.print("MHz");
}


/*
 * Check if this is a valid request
 */
bool alterChannelWarning(void){
  bool flag = false;  
  wipeOLED();
  u8x8.setCursor(0, 2);
  u8x8.print("    WARNING");
  
  u8x8.setCursor(0, 3);
  u8x8.print(" Chg. CHANNEL ");
  u8x8.setCursor(0, 4);
  u8x8.print("   GLOBALLY?  ");
  u8x8.setCursor(0, 6);
  u8x8.print(" [1]Yes / No[4]");

  for(;;){   
    switch (waitButton()){
      
      case BUTTON1:
        u8x8.print(" OK. INITIATE... ");
        pauseMe(tick);
        flag = true;
        break;
      
      case BUTTON4:
        u8x8.setCursor(0, 6);
        u8x8.print("   CANCELLED   ");
        pauseMe(tick);
        //break;
        return false;
  
      default:
        break;    
    }
    if (flag) break;
  }
  wipeOLED();
  return flag;
}


/*
 * Call HC12 to change operational channel
 * 2 parts:  Local, and Remote with Local.
 */
void new_Channel(bool alterGlobally) {
  pauseMe(tick);
  uint8_t newChan = p_Store.curChan;
  for (;;){ 
    wipeOLED();
    u8x8.setCursor(0, 1);
    u8x8.inverse();
    u8x8.print("CAUTION:");
    u8x8.noInverse();
    u8x8.setCursor(0, 2);
    u8x8.print("Changing Channel");
    u8x8.setCursor(0, 3);
    u8x8.print(alterGlobally ? "   GLOBALLY" : "   LOCALLY");
    
    bool flag = false;
    pauseMe(20);
    writeChannel_OLED(newChan);
    doButtonMenu();                                         // Show radio buttons on OLED
  
    switch ( waitButton())
    {
      case BUTTON1: 
        p_Store.curChan = newChan;                          // call the change_Channel and implement, or
        if (!change_Channel(newChan, alterGlobally)) break; // failed, so go around again ... or
        
        flag = true;                                        // good to go
        wipeOLED();
        u8x8.setCursor(0, 2);
        u8x8.print("System Channel:");
        u8x8.setCursor(0, 3);
        u8x8.print(alterGlobally ? "   GLOBALLY" : "   LOCALLY");
        writeChannel_OLED(newChan);
        pauseMe(2*tick);
        break;
        
      case BUTTON2:
        newChan <= 91 ? newChan += 5 : newChan = 1;         // alter with MAX bounds
      break;
      
      case BUTTON3: 
        newChan >= 6 ? newChan -= 5 : newChan = 96;         // alter with MIN bounds
      break;
      
      case BUTTON4:
        newChan = p_Store.curChan;
        flag = true;
      break;
    }
    if (flag) break;
  }
}





/*
 * Routine to alter the Tx/Rx channel setup
 */
bool change_Channel(int newChan, bool alterGlobally){
  digitalWrite(HC12SetPin, HIGH);                           // Ensure TRANSPARENT mode locally
  pauseMe(80);  
  if (alterGlobally) {                                      // Write the channel to the remote unit(s) first
    writeRemoteChannel(newChan);
  }
  return setControlChannel(newChan);                        // returns true if successful
}


byte set_A(byte nID){                                       // alter the operational channel and 
  if (nID == 1) setControlChannel(p_Store.B_ScrCh) ;        // if not on B_chan
  redBorder(true, 2);                                       // write red border on B channel and fill with n_Count_[2]
  pauseMe(50);
  setControlChannel(p_Store.curChan);                       // change control to A
  redBorder(false, 0);                                      // flash green screen
  pauseMe(50);
  return 1;
}


byte set_B(byte nID){
  if (nID == 2) setControlChannel(p_Store.curChan);         // if not on A_chan
  redBorder(true, 1);                                       // write red border on A channel and fill with n_Count_[1]
  pauseMe(50);
  setControlChannel(p_Store.B_ScrCh);                       // change control to B
  redBorder(false, 0);                                      // flash green screen
  pauseMe(50);
  return 2;
}

void clearAB(byte nID, bool score){                         // empty both screens
  nID == 1 ? setControlChannel(p_Store.B_ScrCh) : setControlChannel(p_Store.curChan);
  pauseMe(50);
  clearMatrix(false);
  if (score) score_Collect();
  nID == 1 ? setControlChannel(p_Store.curChan) : setControlChannel(p_Store.B_ScrCh) ;
  pauseMe(50);
  clearMatrix(false);
  if (score) score_Collect();
  
}


byte setControlChannel(byte newChan){                       // Write to the controller with a channel change
  char    HC12ByteIn;                                       // Temporary variable
  bool isOK = false;                                        // True when successful
  String  HC12ReadBuffer  = "";                             // Read/Write Buffer 1 for HC12
  String  tmp_str = makeControlString(newChan);             // parse the text needed for the command
  digitalWrite(HC12SetPin, LOW);                            // Now enter COMMAND mode locally
  pauseMe(80);                                              // allow catch-up
  do{     
    bool HC12End = false; 
    HC12.println(tmp_str);                                  // Send Channel command to local HC12
    do {} while (!HC12.available());                        // . . .await the reply
    while (HC12.available() && !HC12End) {                  // While Arduino's HC12 soft serial rx buffer has data
      HC12ByteIn = HC12.read();                             // Store each character from rx buffer in byteIn
      pauseMe(10);
      HC12ReadBuffer += char(HC12ByteIn);                   // Write each character of byteIn to HC12ReadBuffer
      if (HC12ByteIn == '\n') HC12End = true;               // Set HC12End flag to exit WHILE loop
    }
    isOK = HC12ReadBuffer.startsWith("OK") ? true : false;      
    digitalWrite(HC12SetPin, HIGH);                         // Exit command & enter transparent mode
    pauseMe(80);                                            // Delay before proceeding to allow HC12 resetting
    HC12ReadBuffer = "";
  } while (isOK == false);
  u8x8.setCursor(14,0);                                     //  |
  u8x8.inverse();                                           //  |
  p_Store.which_Scr_1st == 2 ?                              //  | 
      u8x8.print(p_Store.B_ScrCh) :                         //  | write the current channel info on the controller
      u8x8.print(p_Store.curChan);                          //  |
  u8x8.noInverse();                                         //  |
  return newChan;
}
  


void setB_Chan(void){                                       // in TEAMPLAY we use 2 x screens showing independently
  byte B_Chan = p_Store.curChan;
  //printDebugLine(true, __LINE__, __NAME__);
  //debugEEPROM(18, 0);
  //for (;;) {

    if (EEPROM.read(18) == 1) {                             // If this screen channel selection is already done:
      clearFromLine(1);  
      u8x8.setCursor(0, 4);
      u8x8.print("A&B screens SET");
      pauseMe(tick);
      u8x8.setCursor(0, 5);
      u8x8.print("Keep     :BTN[2]");
      u8x8.setCursor(0, 6);
      u8x8.inverse();
      u8x8.print("Redo SET: BTN[3]");
      u8x8.noInverse();
      u8x8.setCursor(0, 7);
      u8x8.print("NO TeamPl:BTN[4]");
      
      pauseMe(tick);
      //unsigned long offTimer = millis();
      bool flag = false;
      for (;;) {
        
        switch (readButtons()){
          
          case BUTTON2:                                     // KEEP
              
              flag = true;
              break;
  
          case BUTTON3:                                     // DO OVER
              p_Store.B_ScrCh = 0;
              flag = true;
              break;
    
          case BUTTON4:                                     // EXIT Teamplay setup 
              clearFromLine(1);                        
              u8x8.draw2x2String(5, 2, "Use");
              u8x8.draw2x2String(0, 5, "ONE Chan");
              setControlChannel (p_Store.B_ScrCh);          // change frequency to the B chann 
              writeRemoteChannel(p_Store.curChan);          // set B chan back to main A channel.
              setControlChannel (p_Store.curChan);          // back to square 1
              pauseMe(1000);
              p_Store.teamPlay    -= 10;                    // Step to standard, single-chann Team setup
              p_Store.B_ScrCh     = 0;
              EEPROM.put(18, 0);                            // UnSet B_Screen flag
              goWhistle(1);
              pauseMe(12);
              return;                                       // get out of the function
              
        }
        if (flag) break;                                    // exit 
      }
      if (p_Store.B_ScrCh) return;                         // if Accept as Set (eg true), return 
    }
            
/*      
 *  Write "Turn off all A chann screens and proceed with desired B chann screens only".       
 *  -> OK to proceed
 */ 
    clearFromLine(1);
    for (;;) {
    p_Store.curChan <= 91 ?                                 // get the ++new channel number, separated by 5Ch
                      B_Chan = p_Store.curChan +5 :
                      B_Chan = p_Store.curChan -5;          // if MAX ch then --ch
    //printDebugLine(true, __LINE__, __NAME__);                  
    const long    _interval   = 500;
    unsigned long toggle      = millis();
    bool          toggleFlag  = false;                   
    clearFromLine(0);
    u8x8.setCursor(0, 0);
    u8x8.inverse();
    u8x8.print("  INSTRUCTIONS: ");
    u8x8.noInverse();
    pauseMe(500);
    u8x8.setCursor(0, 2);
    u8x8.print("Sel. A&B screens");
    pauseMe(500);
    for (;;) {    // Setup switch to B channel
      u8x8.setCursor(0, 3);
      if (millis() - toggle >= _interval ) {              // setup a flashing OFF action-word
        toggleFlag = !toggleFlag;
        toggle = millis();
      }
      toggleFlag ? u8x8.print("All A units:    ") : u8x8.print("All A units: OFF");
      u8x8.setCursor(0, 4);
      u8x8.print("All B units: ON");
      u8x8.setCursor(0, 6);
      u8x8.print("READY?  >BTN[2]");
      u8x8.setCursor(0, 7);
      u8x8.print("Go Back >BTN[4]");
      if (readButtons() == BUTTON2) break;                // exit the infinite loop here
      if (readButtons() == BUTTON4) {                     // Leave the process
        clearFromLine(0);
        u8x8.draw2x2String(5, 2, "Use");
        u8x8.draw2x2String(0, 5, "ONE Chan");
        pauseMe(1000);
        p_Store.teamPlay    -= 10;                        // Step to standard, single-chann Team setup
        p_Store.B_ScrCh     = 0;
        EEPROM.put(18, 0);                                // unset B_Screen flag
        goWhistle(1);
        pauseMe(120);
        return;        //
      }
    }
    //printDebugLine(true, __LINE__, __NAME__);
    clearFromLine(1);
    u8x8.draw2x2String(0, 6, "..WAIT..");
    /* 
     *  Name the current chan: p_Store.curChan
     *  here select the channel (auto +5 with option to change)
     */
    writeRemoteChannel(B_Chan);                           // set the new channel freq. on to all active screens (usually +5)
    EEPROM.put(18, 1);                                    // set B_Screen flag
    goWhistle(1);
    pauseMe(12);
    p_Store.B_ScrCh = B_Chan;
  /*              
  * Display B chann on screens and request conf.              
  * -> OK  -> TEST or -> REDO
  * Write "Turn on all screens, A & B" 
  * -> OK for test
  * Test seq: 
  * -> OK  or -> REDO
  * if OK then write READY
  */
    setControlChannel(B_Chan);                            // Now shift the controller freq. to talk to the new channel
    pauseMe(2*tick);
    HC12.print(F("font 9\r"));    HC12.flush();
    clearMatrix(false);
    sendSerialS(green, /*column=*/ 0, /*line=*/ 15, "B CH: SET");
    pauseMe(tick);
    sendSerialS(green, /*column=*/ 0, /*line=*/ 31, "TEST:  OK");
    
    clearFromLine(3);
    u8x8.inverse();
    u8x8.setCursor(0, 4);
    u8x8.print("Change Complete?");
    u8x8.noInverse();
    u8x8.setCursor(0, 5);
    u8x8.print("OK:       BTN[1]");
    u8x8.setCursor(0, 6);
    u8x8.print("Retest:   BTN[3]");
    u8x8.setCursor(0, 7);
    u8x8.print("Redo:     BTN[4]");
    bool flag = false;
    bool colFlag = false;                                   // setup a flag to alter the matrix screen retest response colour
    for (;;) {    // now switch all screens back on and confirm
      switch (waitButton()) {
        case BUTTON1:
          clearFromLine(3);
          u8x8.inverse();
          u8x8.setCursor(0, 4);
          u8x8.print(" Now return ALL ");
          u8x8.setCursor(0, 5);
          u8x8.print(" displays to ON ");
          pauseMe(5*tick);
          u8x8.noInverse();
          u8x8.setCursor(0, 7);
          u8x8.print("Proceed:  BTN[1]");
          for(;;) {
            if (readButtons() == BUTTON1) {
              writeReady();
              sendSerialS(3, 0, 29, "       B");
              HC12.flush();
              pauseMe(50);
              setControlChannel(p_Store.curChan);             // return to the base (A) channel and exit
              writeReady();
              sendSerialS(3, 0, 29, "       A");
              flag = true;
            }
            if (flag) break;
          }
          break;
            
        case BUTTON3:
          sendSerialS(colFlag ? green : orange, /*column=*/ 0, /*line=*/ 31, "TEST:  OK");
          colFlag = !colFlag;
          break;
    
        case BUTTON4:
          writeRemoteChannel(p_Store.curChan);                // send altered (B)chan back to main.
          setControlChannel(p_Store.curChan);                 // back to square 1
          clearFromLine(1);
          p_Store.B_ScrCh = 0;
          EEPROM.put(18, 0);                                  // UnSet B_Screen flag
          goWhistle(1);
          pauseMe(120);
          flag = true;
          break;
             
      }
      if (flag) break;                                        // exit infinite loop if BUTTON1 has been selected
    }   
    if (p_Store.B_ScrCh > 0) break;                           // REDO if BTN4 has been selected above, exit otherwise
  }
  return ;
}


/*
 * Write a given channel number to the remote unit(s) and return the AT+Cxxx char
 */
void writeRemoteChannel(byte newChan) {
  digitalWrite(HC12SetPin, HIGH);                             // Ensure TRANSPARENT mode locally
    
  String newChanTxt = makeControlString(newChan);             // parse the string to send
  HC12.print("*^");                                           // call remote HC12(s) to announce change coming 
  HC12.flush();
  pauseMe(180);                                               // let HC12 get ready
  HC12.println(newChanTxt);                                   // Send command to REMOTE HC12
  HC12.flush();
  pauseMe(180);
}

/*
 * Parse the channel number into a control string "AT+Cxxx"
 */
String makeControlString(byte newChan){
  char tmp_str[8]  = "AT+C001";
  tmp_str[4] = '0';                                           // |
  byte x = newChan / 10;                                      // |
  tmp_str[5] = (char)(x + '0');                               // | parse the new channel number into the string
  x = newChan % 10;                                           // |
  tmp_str[6] = (char)(x + '0');                               // |
  tmp_str[7] = '\n';
  return tmp_str;
}

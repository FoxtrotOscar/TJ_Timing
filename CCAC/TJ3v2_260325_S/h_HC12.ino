/*
 * HC12 433MHz RADIO DEVICE 
 * Read the current channel, write to p_Store and display
 *
 * 22   readChannel
 * 67   writeChannel_OLED
 * 87   alterChannelWarning
 * 132  new_Channel
 * 187  change_Channel
 * 194  set_A
 * 205  set_B
 * 215  clearAB
 * 225  setControlChannel
 * 256  setB_Chan
 * 434  readyAB
 * 454  writeRemoteChannel
 * 467  makeControlString
 */


void readChannel(void){
  command_ON(true);                                         // enable Control Mode locally
  p_Store.curChan = 0;
  char endMarker = '\n';                                    // a return (EOL) char at end of data from HC12
  char rc;                                                  // data returned from HC12 interrogation
  bool flag;                                                // to mark a read     
  bool read_error;                                          // to mark bad data
  char temp_str[4];
  byte u8x8ColNumber = 10;
  disp.setFont(u8x8_font_chroma48medium8_r);
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
  
  disp.draw2x2String(u8x8ColNumber++, 4, temp_str);
  command_ON(false);                                        // disable Control Mode locally
}

void writeChannel_OLED (int channel){
  disp.inverse();
  disp.setCursor(1, 5);
  disp.print("CH:");
  disp.setCursor(5, 5);
  disp.print("  ");
  disp.setCursor((channel >9 ? 5 : 6), 5) ;
  disp.print(channel);
  disp.noInverse();
  disp.setCursor(8, 5);
  float chFrq = 433.4+(0.4*(channel-1.0));
  disp.print(chFrq);
  disp.setCursor(13, 5);
  disp.print("MHz");
}


/*
 * Check if this is a valid request
 */
bool alterChannelWarning(void){
  bool flag = false;  
  wipeOLED();
  disp.setCursor(0, 2);
  disp.print("    WARNING");
  
  disp.setCursor(0, 3);
  disp.print(" Chg. CHANNEL ");
  disp.setCursor(0, 4);
  disp.print("   GLOBALLY?  ");
  disp.setCursor(0, 6);
  disp.print(" [1]Yes / No[4]");

  for(;;){   
    switch (waitButton()){
      
      case BUTTON1:
        disp.print(" OK. INITIATE... ");
        pauseMe(tick);
        flag = true;
        break;
      
      case BUTTON4:
        disp.setCursor(0, 6);
        disp.print("   CANCELLED   ");
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
    disp.setCursor(0, 1);
    disp.inverse();
    disp.print("CAUTION:");
    disp.noInverse();
    disp.setCursor(0, 2);
    disp.print("Changing Channel");
    disp.setCursor(0, 3);
    disp.print(alterGlobally ? "   GLOBALLY" : "   LOCALLY");
    
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
        disp.setCursor(0, 2);
        disp.print("System Channel:");
        disp.setCursor(0, 3);
        disp.print(alterGlobally ? "   GLOBALLY" : "   LOCALLY");
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
  if (alterGlobally) {                                      // Write the channel to the remote unit(s) first
    writeRemoteChannel(newChan);
  }
  return setControlChannel(newChan);                        // returns true if successful
}

byte set_A(byte nID){                                       // alter the operational channel and 
  if (nID == 1) setControlChannel(p_Store.B_ScrCh) ;        // if not on B_chan
  redBorder(true, 2);                                       // write red border on B channel and fill with n_Count_[2]
  pauseMe(70);
  setControlChannel(p_Store.curChan);                       // change control to A
  pauseMe(70);
  clearMatrix(false);
  return 1;
}


byte set_B(byte nID){
  if (nID == 2) setControlChannel(p_Store.curChan);         // if not on A_chan
  redBorder(true, 1);                                       // write red border on A channel and fill with n_Count_[1]
  pauseMe(70);
  setControlChannel(p_Store.B_ScrCh);                       // change control to B
  pauseMe(70);
  clearMatrix(false);
  return 2;
}

void clearAB(byte nID, bool score){                         // empty both screens
  if (score) score_Collect(false);
  pauseMe(10);
  nID <= 1 ? setControlChannel(p_Store.B_ScrCh) : setControlChannel(p_Store.curChan);
  if (score) score_Collect(false);
  pauseMe(10);
  nID <= 1 ? setControlChannel(p_Store.curChan) : setControlChannel(p_Store.B_ScrCh);
}


byte setControlChannel(byte newChan){                       // Write to the controller with a channel change
  char    HC12ByteIn;                                       // Temporary variable
  bool    isOK            = false;                          // True when successful
  String  HC12ReadBuffer  = "";                             // Read/Write Buffer 1 for HC12
  String  tmp_str = makeControlString(newChan);             // parse the text needed for the command
  command_ON(true);                                         // Now enter COMMAND mode locally
  pauseMe(80);                                              // allow catch-up
  do{     
    bool HC12End = false;
    HC12.println(tmp_str);                                  // Send Channel command to local HC12
    do {} while (!HC12.available());                        // . . .await the reply
    while (HC12.available() && !HC12End) {                  // While Arduino's HC12 soft serial rx buffer has data
      HC12ByteIn = HC12.read();                             // Store each character from rx buffer in byteIn
      pauseMe(10);
      HC12ReadBuffer += char(HC12ByteIn);                   // Write each character of byteIn to HC12ReadBuffer
      if (HC12ByteIn == '\n') {
        //printDebugLine(false, __LINE__, __NAME__);
        see(HC12ReadBuffer); 
        HC12End = true;               // Set HC12End flag to exit WHILE loop
      }
    }
    isOK = HC12ReadBuffer.startsWith("OK") ? true : false;      
    HC12ReadBuffer = "";
  } while (isOK == false);
  command_ON(false);                                        // Exit command & enter transparent mode
  op_Chan = newChan;                                        //  |

  return newChan;
}
  

void setB_Chan(void){                                       // in TEAMPLAY we use 2 x screens showing independently
  byte B_Chan = p_Store.curChan;
  if (EEPROM.read(18) == 1) {                               // If this screen channel selection is already done:
    clearFromLine(1);  
    disp.setCursor(0, 4);
    disp.print("A&B screens SET");
    pauseMe(tick);
    disp.setCursor(0, 5);
    disp.print("Keep     :BTN[2]");
    disp.setCursor(0, 6);
    disp.inverse();
    disp.print("Redo SET: BTN[3]");
    disp.noInverse();
    disp.setCursor(0, 7);
    disp.print("NO TeamPl:BTN[4]");
    
    pauseMe(tick);
    bool flag = false;
    for (;;) {
      switch (readButtons()){
        case BUTTON2:                                     // KEEP
            flag = true;
            break;

        case BUTTON3:                                     // DO OVER
            p_Store.B_ScrCh = 0;
            EEPROM.update(18, 0);
            flag = true;
            break;
  
        case BUTTON4:                                     // EXIT Teamplay setup 
            clearFromLine(1);                        
            disp.draw2x2String(5, 2, "Use");
            disp.draw2x2String(0, 5, "ONE Chan");
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
      if (flag) break;                                    // exit infinite loop
    }
    if (EEPROM.read(18) == 1) return;                     // done
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
    const long    _interval   = 500;
    unsigned long toggle      = millis();
    bool          toggleFlag  = false;                   
    clearFromLine(0);
    disp.setCursor(0, 0);
    disp.inverse();
    disp.print("  INSTRUCTIONS: ");
    disp.noInverse();
    pauseMe(500);
    disp.setCursor(0, 2);
    disp.print("Sel. A&B screens");
    pauseMe(500);
    for (;;) {                                            // Setup switch to B channel
      disp.setCursor(0, 3);
      if (millis() - toggle >= _interval ) {              // setup a flashing OFF action-word
        toggleFlag = !toggleFlag;
        toggle = millis();
      }
      toggleFlag ? disp.print("All A units:    ") : disp.print("All A units: OFF");
      disp.setCursor(0, 4);
      disp.print("All B units: ON");
      disp.setCursor(0, 6);
      toggleFlag ? disp.print("READY?  >BTN [2]") :disp.print("READY?  >BTN [ ]") ;
      disp.setCursor(0, 7);
      disp.print("Go Back >BTN [4]");
      if (readButtons() == BUTTON2) break;                // exit the infinite loop here
      if (readButtons() == BUTTON4) {                     // Leave the process
        clearFromLine(0);
        disp.draw2x2String(5, 2, "Use");
        disp.draw2x2String(0, 5, "ONE Chan");
        pauseMe(1000);
        p_Store.teamPlay    -= 10;                        // Step to standard, single-chann Team setup
        p_Store.B_ScrCh     = 0;
        EEPROM.put(18, 0);                                // unset B_Screen flag
        goWhistle(1);
        return; 
      }
    }
    clearFromLine(1);
    disp.draw2x2String(0, 6, "..WAIT..");
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
    pauseMe(2*tick);                                      // Allow STM (matrix) changes to take effect
    clearMatrix(false);
    HC12.print(F("font 9\r"));    HC12.flush();
    sendSerialS(green, /*column=*/ 0, /*line=*/ 15, "B CH: SET");
    pauseMe(tick);
    sendSerialS(green, /*column=*/ 0, /*line=*/ 31, "TEST:  OK");
    clearFromLine(3);
    disp.inverse();
    disp.setCursor(0, 4);
    disp.print("Change Complete?");
    disp.noInverse();
    disp.setCursor(0, 5);
    disp.print("OK:       BTN[1]");
    disp.setCursor(0, 6);
    disp.print("Retest:   BTN[3]");
    disp.setCursor(0, 7);
    disp.print("Redo:     BTN[4]");
    bool flag = false;
    bool colFlag = false;                                   // setup a flag to alter the matrix screen retest response colour
    for (;;) {    // now switch all screens back on and confirm
      switch (waitButton()) {
        case BUTTON1:
          //wipeOLED();
          clearFromLine(3);
          disp.inverse();
          disp.setCursor(0, 4);
          disp.print(" Now return ALL ");
          disp.setCursor(0, 5);
          disp.print(" displays to ON ");
          pauseMe(5*tick);
          disp.noInverse();
          disp.setCursor(0, 7);
          disp.print("Proceed:  BTN[1]");
          readyAB();
          wipeOLED();
          flag = true;
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


void readyAB (void){
  bool flag = false;
  for(;;) {
    if (readButtons() == BUTTON1) {
      writeReady();
      sendSerialS(orange, 0, 29, "       B");
      HC12.flush();
      pauseMe(50);
      setControlChannel(p_Store.curChan);             // return to the base (A) channel and exit
      writeReady();
      sendSerialS(orange, 0, 29, "       A");
      flag = true;
    }
    if (flag) break;
  }
}

/*
 * Write a given channel number to the remote unit(s) and return the AT+Cxxx char
 */
void writeRemoteChannel(byte newChan) {
  String newChanTxt = makeControlString(newChan);             // parse the string to send
  HC12.print("*^");                                           // call remote HC12(s) to announce change coming 
  HC12.flush();
  pauseMe(180);                                               // let HC12 get ready
  HC12.println(newChanTxt);                                   // Send command to REMOTE HC12
  HC12.flush();
  pauseMe(2*tick);
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


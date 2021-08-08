/*
 * HC12 433MHz RADIO DEVICE 
 * 
 * Routine to alter the Tx/Rx channel setup
 * Check proceed? y/n
 * gather channel detail required
 * transmit serial command to enter remote COMMAND mode
 * transmit new channel parameters
 * show new channel num on screen(s)
 * Close remote Command mode
 * enter local COMMAND mode
 * Alter channel to match
 * show new channel number
 * close local Command mode
 * exit routine on OK button
 */

/*
 * Call HC12 to change operational channel
 * 2 parts:  Local, and Remote with Local.
 */
void changeGlob_Chan(bool alterGlobally) {
  delay(tick);
  uint8_t curChan = paramStore.currChannel;
   
  wipeOLED();
  u8x8.setCursor(0, 1);
  u8x8.inverse();
  u8x8.print("CAUTION:");
  u8x8.noInverse();
  u8x8.setCursor(0, 2);
  u8x8.print("Changing Channel");
  u8x8.setCursor(0, 3);
  u8x8.print(alterGlobally ? "   GLOBALLY" : "   LOCALLY");

start_CHANNELS_menu:  
  delay(20);
  writeChannel_OLED(curChan);
  doButtonMenu();

  switch (waitButton())
  {
    case BUTTON1: 
      paramStore.currChannel = curChan;             // call the newChannel and implement, or
      if (!newChannel(curChan, alterGlobally)) {    
        goto start_CHANNELS_menu;                   // re-enter loop if un-successful                     
      }
      wipeOLED();
      u8x8.setCursor(0, 2);
      u8x8.print("System Channel:");
      u8x8.setCursor(0, 3);
      u8x8.print(alterGlobally ? "   GLOBALLY" : "   LOCALLY");
      writeChannel_OLED(curChan);
      delay(2*tick);
      break;
      
    case BUTTON2: 
      if (curChan <= 91) {
        curChan += 5;
      } else curChan = 1;
    goto start_CHANNELS_menu;      
    
    case BUTTON3: 
      if (curChan >= 6) {
        curChan -= 5;
      } else curChan = 96;
      
    goto start_CHANNELS_menu;

    case BUTTON4:
      curChan = paramStore.currChannel;
      break;
  }
  
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
  chFrq = 433.4+(0.4*(channel-1.0));
  u8x8.print(chFrq);
  u8x8.setCursor(13, 5);
  u8x8.print("MHz");
}


/*
 * Check if this is a valid request
 */

bool alterChannelWarning(void){  
  wipeOLED();
  u8x8.setCursor(0, 2);
  u8x8.print("    WARNING");
  
  u8x8.setCursor(0, 3);
  u8x8.print(" Chg. CHANNEL ");
  u8x8.setCursor(0, 4);
  u8x8.print("   GLOBALLY?  ");
  u8x8.setCursor(0, 6);
  u8x8.print(" [1]Yes / No[4]");

getButton:   
  switch (waitButton()){
    
    case BUTTON1:
      u8x8.print(" OK. INITIATE... ");
      delay(tick);
      return true;
    
    case BUTTON4:
      u8x8.setCursor(0, 6);
      u8x8.print("   CANCELLED   ");
      delay(tick);
      return false;

    default:
      goto getButton;    
  }
  wipeOLED();
}




/*
 * Write the new channel to HC12, display "Success" and return true if good 
 */

bool newChannel(int newChan, bool alterGlobally){
  
  char    HC12ByteIn;                           // Temporary variable
  String  HC12ReadBuffer  = "";                 // Read/Write Buffer 1 for HC12
  bool    HC12End         = false;              // Flag to indiacte End of HC12 String
  bool    localSuccess    = false;
  char    tmp_str[8]      = "AT+C001";
  HC12ReadBuffer.reserve(64);                   // Reserve 64 bytes for Serial message input
  digitalWrite(HC12SetPin, HIGH);               // Ensure TRANSPARENT mode locally
  delay(80);                                    // 80 ms delay before operation per datasheet
  tmp_str[4] = '0';                             // |
  byte x = newChan / 10;                        // |
  tmp_str[5] = (char)(x + '0');                 // | parse the new channel number into the string
  x = newChan % 10;                             // |
  tmp_str[6] = (char)(x + '0');                 // |
  tmp_str[7] = '\n';                            // |
  if (alterGlobally){                           //   Do this to both LOCAL & any REMOTE listening?
    HC12.print("*^");                           // call remote HC12(s) to announce change coming 
    HC12.flush();
    delay(100);                                 // let HC12 get ready
    HC12.print(tmp_str);                        // Send command to REMOTE HC12
    delay(500);                                 // Wait 0.5s for a response
  }
  digitalWrite(HC12SetPin, LOW);                // Now enter COMMAND mode locally
  delay(100);                                   // Allow chip time to enter command mode
  HC12.print(tmp_str);                          // Send command to local HC12
  HC12.flush();
  do {} while (!HC12.available());              
  while (HC12.available() && !HC12End) {        // While Arduino's HC12 soft serial rx buffer has data
    HC12ByteIn = HC12.read();                   // Store each character from rx buffer in byteIn
    delay(2);
    HC12ReadBuffer += char(HC12ByteIn);         // Write each character of byteIn to HC12ReadBuffer
    if (HC12ByteIn == '\n') {                   // At the end of the line
      HC12End = true;                           // Set HC12End flag to true
    }
  }
                                                // Have we succeeded???
  localSuccess = HC12ReadBuffer.startsWith("OK") ? true : false; 
  digitalWrite(HC12SetPin, HIGH);               // Exit command & enter transparent mode
  delay(100);                                   // Delay before proceeding to allow HC12 resetting
  HC12ReadBuffer = "";
  return localSuccess;
}

/*
 * Read the current channel, write to paramStore and display
 */
 
void readChannel(void){
  paramStore.currChannel = 0;
  char endMarker = '\n';                        // a return (EOL) char at end of data from HC12
  char rc;                                      // data returned from HC12 interrogation
  bool flag = false;                            // to mark good/bad data
  char temp_str[4];
  byte u8x8ColNumber = 10;
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  temp_str[3] = '\0';
  
START:
  byte ptr = 0;
  HC12.print("AT+RC\n");
  HC12.flush();
  do {} while (!HC12.available());
  while (HC12.available()) {                    
    while (flag == false){
      rc = HC12.read();
      delay(2);
      if (rc == '+') {                          // start of data expected
        flag = true;
      }
      if (rc == 'E'){                           // start of word ERROR - bad data
        goto START;                                           
      }
    }
    
    rc = HC12.read();                           // grab next char
    delay(2);                                   // allow serial to catch up
    if ((rc != endMarker) ) {                   // repeat until eol encountered
      
      if ((rc >= '0') && (rc <= '9') && (ptr < 3))   {
        paramStore.currChannel += (ptr > 0 ? (ptr == 1 ? 
        ((rc - '0') *10) : (rc - '0')) : (rc - '0')*100);
        
        temp_str[ptr++] = rc;                   // set temp pointer to rc and increment pointer
      }
    } 
  }
  u8x8.draw2x2String(u8x8ColNumber++, 4, temp_str);
  
  rc = 0;
  flag    = false;
  digitalWrite    (HC12SetPin, HIGH);           // HC12 to TRANSPARENT mode to DISable writing (IF NOT)                 
  
}

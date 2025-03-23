 


void processRFID(void){
      /*   TO DO
     * 1) write a ready / cancel ready message
     * 2) ***read a card
     * 3) ***if details present show details on screen for period
     * 3a)***overwrite or canc
     * 4) write, show success! and return to 1)
     */
  writeReady();   
  //bool readCard = false;
  do {
    writeRFID();
  } while (!((readButton(button4Pin)) || (readButton(button3Pin))));  // loop until either btn 3 or 4
  // if (btn == BUTTON3) {
  //   readCard = true;

  // }
  
  wipeOLED(true);
  u8x8.draw2x2String(0, 2, "QUITTING");
  u8x8.draw2x2String(3, 4, "WRITE");
  u8x8.draw2x2String(4, 6, "MODE");
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  pauseMe(2*tick); 
  wipeOLED(false);
  showAllParams(paramShow+1);
  showInstr();
}

void writeRFID(void){  
  if ( ! mfrc522.PICC_IsNewCardPresent())                   // Look for new cards
      return;

  if ( ! mfrc522.PICC_ReadCardSerial())                     // Select one of the cards
      return;

  #ifdef DEBUG
    Serial.print(F("Card UID:"));                           // Show some details of the PICC (that is: the tag/card)
  #endif
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);   // Gets the UID into 'mfrc522.uid.uidByte' 
  #ifdef DEBUG
    Serial.println();
    Serial.print(F("PICC type: "));
  #endif  
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  #ifdef DEBUG
    Serial.println(mfrc522.PICC_GetTypeName(piccType));
  #endif
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {       // Check for compatibility    
    wipeOLED(true);
    u8x8.draw2x2String(5, 2, "BAD");
    u8x8.draw2x2String(4, 6, "CARD");
    pauseMe(750); 
    writeReady();
    return;
  }
  //numberOfBlocks    = 2;
  byte sector         = 1;                                  //  we use the second sector: sector #1
  byte blockAddr      = 4;                                  // covering block #4 up to and including block #7
  byte blockAddr1     = 5;
  bool ctFlag         = false;
  byte dataBlock[16];
  byte i = sizeof(dataBlock);
  while ( i-- ) *( dataBlock + i ) = *( dataStore + i );    // pop dataStore into dataBlock

  byte dataBlock1[16]    = {
                        0x00, 0x00, 0x00, 0x00,             //  
                        0x00, 0x00, 0x00, 0x00,             //  
                        0x00, 0x00, 0x00, 0x00,             //  
                        mfrc522.uid.uidByte[0] ^ Key12,      //|   0x7f, 0xd4, 0x2a, 0xc6 :  KEY in HEX12-15
                        mfrc522.uid.uidByte[1] ^ Key13,      //|_  positions 12-15
                        mfrc522.uid.uidByte[2] ^ Key14,      //|   XOR  card UID with this KEY to generate GUARD bytes
                        mfrc522.uid.uidByte[3] ^ Key15       //| 
  };
  
  byte trailerBlock   = 7;
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);
  memset(buffer, 0, size);                                  // zero all of buffer
                                                     
  #ifdef DEBUG
    Serial.println(F("Authenticating using key A..."));       // Authenticate using key A
  #endif
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(
            MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));

  if (status != MFRC522::STATUS_OK) {                         // fail state
    #ifdef DEBUG  
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    #endif  
    wipeOLED(true);
    u8x8.draw2x2String(0, 2, "PCD Auth");
    u8x8.draw2x2String(0, 6, " FAILED ");
    
    pauseMe(500);
    return;
  }
  
                    /* ###################### START OF OPERATION READ ##############################*/
  #ifdef DEBUG
    Serial.println(F("Current data in sector:"));             // Show the whole sector as it currently is
  #endif  
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  #ifdef DEBUG
    Serial.println();
    Serial.print(F("Reading data from block "));              // Read data from the block 
    Serial.print(blockAddr);
    Serial.println(F(" ..."));                  
  #endif
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    #ifdef DEBUG
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    #endif  
  }
  #ifdef DEBUG
    Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
  #endif
  dump_byte_array(buffer, 16); Serial.println();
  #ifdef DEBUG
    Serial.println();
  #endif
  // Authenticate using key B
  #ifdef DEBUG
    Serial.println(F("Authenticating again using key A")); 
  #endif
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  // ****** flip the below comment-out if needed.
  //status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    #ifdef DEBUG
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    #endif  
      return;
  }
  printParamVals();
  
/*###########################################################################*/  
// WRITING BLOCK STARTS HERE
  
  #ifdef DEBUG
    Serial.print(F("Writing data into block "));              
    Serial.print(blockAddr);
    Serial.println(F(" ..."));
  #endif
  dump_byte_array(dataBlock, 16); 
  #ifdef DEBUG
    Serial.println();
  #endif
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);        // writing here
  if (status != MFRC522::STATUS_OK) {
    #ifdef DEBUG
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    #endif  
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  #ifdef DEBUG
    Serial.println();
    Serial.print(F("Writing data into block ")); 
    Serial.print(blockAddr1);
    Serial.println(F(" ..."));
  #endif
  dump_byte_array(dataBlock1, 16); Serial.println();
  
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr1, dataBlock1, 16);      // Write data to the second block 
  if (status != MFRC522::STATUS_OK) {
    #ifdef DEBUG
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    #endif  
  }

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
// READBACK  
  #ifdef DEBUG
    Serial.println();
    Serial.print(F("Reading data from block "));              // Read data from the block 
    Serial.print(blockAddr);                                  // (again, should now be what we have written) 
    Serial.println(F(" ..."));
  #endif
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);  
  if (status != MFRC522::STATUS_OK) {
    #ifdef DEBUG
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    #endif  
  }
  #ifdef DEBUG
    Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
  #endif
  dump_byte_array(buffer, 16);                           
  #ifdef DEBUG
    Serial.println();
  #endif
  
  // Check that data in block is what we have written
  // by counting the number of bytes that are equal
  #ifdef DEBUG
    Serial.println(F("Checking result..."));
  #endif
  byte count = 0;
  for (byte i = 0; i < 16; i++) {
      // Compare buffer (= what we've read) with dataBlock (= what we've written)
      if (buffer[i] == dataBlock[i])
          count++;
  }
  #ifdef DEBUG
    Serial.print(F("Number of bytes that match = ")); Serial.println(count);
  #endif
  if (count == 16) ctFlag = true;
  #ifdef DEBUG
    Serial.println();
    Serial.print(F("Reading data from block "));              // Read data from the block 
    Serial.print(blockAddr1);                                  // (again, should now be what we have written) 
    Serial.println(F(" ..."));
  #endif
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr1, buffer, &size);  
  if (status != MFRC522::STATUS_OK) {
    #ifdef DEBUG
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    #endif  
  }
  #ifdef DEBUG
    Serial.print(F("Data in block ")); Serial.print(blockAddr1); Serial.println(F(":"));
  #endif
  dump_byte_array(buffer, 16); 
  #ifdef DEBUG
    Serial.println();
  #endif
  
  // Check that data in block is what we have written
  // by counting the number of bytes that are equal
  #ifdef DEBUG
    Serial.println(F("Checking result..."));
  #endif
//  byte count = 0;
  count = 0;
  for (byte i = 0; i < 16; i++) {
      // Compare buffer (= what we've read) with dataBlock (= what we've written)
      if (buffer[i] == dataBlock1[i])
          count++;
  }
  #ifdef DEBUG
    Serial.print(F("Number of bytes that match = ")); Serial.println(count);
  #endif
  if (count == 16 && ctFlag) {
    #ifdef DEBUG
      Serial.println(F("Success :-)"));
    #endif

    

    #ifdef DEBUG
    Serial.println();
    // Dump the sector data
    Serial.println(F("Data now in sector:"));
  #endif
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  #ifdef DEBUG
    Serial.println();
  #endif

  } else {
    #ifdef DEBUG
      Serial.println(F("Failure, no match :-("));
      Serial.println(F("  perhaps the write didn't work properly..."));
    #endif  
  }

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
  
  wipeOLED(true);
  u8x8.draw2x2String(0, 3, "SUCCESS ");
  pauseMe(tick);
  writeReady();
}

// void readRFID(){


//   for (byte i = 0; i < 16; i++) {
//       // Compare buffer (= what we've read) with dataBlock (= what we've written)
//       if (buffer[i] == dataBlock[i])
//           count++;
//   }
// }

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
      #ifdef DEBUG
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
      #endif  
    }
}

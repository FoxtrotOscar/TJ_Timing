


void processRFID(void){
      /*
     * 1) write a ready / cancel ready message
     * 2) read a card
     * 3) if details present show details on screen for period
     * 3a)overwrite or canc
     * 4) write, show success! and return to 1)
     */
  writeReady();   

  do {
    writeRFID();
  } while (!readButton(button4Pin));
  wipeOLED();
  u8x8.draw2x2String(0, 2, "QUITTING");
  u8x8.draw2x2String(3, 4, "WRITE");
  u8x8.draw2x2String(4, 6, "MODE");
  long long pause = millis();
  do {} while (millis() - pause < 2 * tick);
  wipeOLED();
  showParams(12);
}

void writeRFID(void){  
  if ( ! mfrc522.PICC_IsNewCardPresent())                   // Look for new cards
      return;

  if ( ! mfrc522.PICC_ReadCardSerial())                     // Select one of the cards
      return;

  Serial.print(F("Card UID:"));                             // Show some details of the PICC (that is: the tag/card)
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {       // Check for compatibility    
    wipeOLED();
    u8x8.draw2x2String(5, 2, "BAD");
    u8x8.draw2x2String(4, 6, "CARD");
    long long pause = millis();
    do {} while (millis() - pause < 750);
    writeReady();
    return;
  }
  byte sector         = 1;                                  //  we use the second sector: sector #1
  byte blockAddr      = 4;                                  // covering block #4 up to and including block #7

  
  byte dataBlock[]    = {    

      dataStore[0] , dataStore[1], dataStore[2], dataStore[3], 
       
      dataStore[4], dataStore[5], dataStore[6], dataStore[7], 
       
      dataStore[8], dataStore[9], dataStore[10], dataStore[11],
       
                                                            //    127, 212, 42, 198  : SECURITY KEY
                                                            //    0x7f, 0xd4, 0x2a, 0xc6 : KEY in HEX
      mfrc522.uid.uidByte[0] ^ dataStore[12],               //|
      mfrc522.uid.uidByte[1] ^ dataStore[13],               //|_  XOR  UID with KEY to generate GUARD bytes
      mfrc522.uid.uidByte[2] ^ dataStore[14],               //|   which will be decoded in target sys
      mfrc522.uid.uidByte[3] ^ dataStore[15]                //|
  };

  
  byte blockAddr1      = 5;
  byte dataBlock1[]    = {
      0x04, 0x03, 0x02, 0x01, //  4,   3,   2,  1,
      0x08, 0x07, 0x06, 0x05, //  8,   7,   6,  5,
      0x09, 0x0a, 0xff, 0x0b, //  9,  10, 255, 11,
      0x0c, 0x0d, 0x0e, 0x0f  // 12,  13,  14, 15
  };
  byte trailerBlock   = 7;
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

                                                     
  Serial.println(F("Authenticating using key A..."));       // Authenticate using key A
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(
            MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    wipeOLED();
    u8x8.draw2x2String(0, 2, "PCD Auth");
    u8x8.draw2x2String(0, 6, " FAILED ");
    
    pause = millis();
    do {} while (millis() - pause < 500);
    return;
  }

  Serial.println(F("Current data in sector:"));             // Show the whole sector as it currently is
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  Serial.print(F("Reading data from block "));              // Read data from the block 
  Serial.print(blockAddr);
  Serial.println(F(" ..."));                  
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
  dump_byte_array(buffer, 16); Serial.println();
  Serial.println();

  // Authenticate using key B
  Serial.println(F("Authenticating again using key A")); 
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  // ****** flip the below comment-out if needed.
  //status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
  }

  Serial.print(F("Writing data into block "));              // Write data to the block 
  Serial.print(blockAddr);
  Serial.println(F(" ..."));
  dump_byte_array(dataBlock, 16); Serial.println();
  //dump_byte_array(dataBlock1, 16); Serial.println();
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.println();
  
  
  Serial.print(F("Writing data into block "));              // Write data to the second block 
  Serial.print(blockAddr1);
  Serial.println(F(" ..."));
  dump_byte_array(dataBlock1, 16); Serial.println();
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr1, dataBlock1, 16);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.println();
  
  Serial.print(F("Reading data from block "));              // Read data from the block 
  Serial.print(blockAddr);                                  // (again, should now be what we have written) 
  Serial.println(F(" ..."));
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
  dump_byte_array(buffer, 16); Serial.println();

  // Check that data in block is what we have written
  // by counting the number of bytes that are equal
  Serial.println(F("Checking result..."));
  byte count = 0;
  for (byte i = 0; i < 16; i++) {
      // Compare buffer (= what we've read) with dataBlock (= what we've written)
      if (buffer[i] == dataBlock[i])
          count++;
  }
  Serial.print(F("Number of bytes that match = ")); Serial.println(count);
  if (count == 16) {
    Serial.println(F("Success :-)"));
    wipeOLED();
    u8x8.draw2x2String(0, 3, " SUCCESS");
    long long pause = millis();
    do {} while (millis() - pause < tick/2);
    writeReady();
  } else {
      Serial.println(F("Failure, no match :-("));
      Serial.println(F("  perhaps the write didn't work properly..."));
  }
  Serial.println();

  // Dump the sector data
  Serial.println(F("Current data in sector:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}


/* RFID OPERATION
 * 
 * 
  uint8_t startCountsIndex = 1;         // (1)Number from 0 to 4 indentifying which of startCounts is used in this round, default 120 
  uint8_t walkUp = 10;                  // (2)
  uint8_t maxEnds = 4;                  // (3)Total number of Ends for competition
  uint8_t Details = 2;                  // (4)Single (1) or Double detail (2)
  uint8_t maxPrac = 2;                  // (5)Initially set as 2x practice ends
  uint8_t isFinals = 0;                 // (6)For alternating A & B session
  uint8_t isAlternating = 0;            // (8)1 == Archer A, Archer B; 0 == Simultaneous
  uint8_t teamPlay = 0;                 // (9)
  uint8_t whichArcher = 1;              //(10)
*/


/* The following is a function to read block 4-7 of a MIFARE card
 * and get/dump the data therein. We get the data, verify, and 
 * apply it to the various parameters as done in the menu setup, 
 * and only while in the menu top page.
 * A successful read generates a validation message on the OLED.
 */
 
void getRFID(struct PARAMSTORE *ps){
  bool guardBytes = false;
  if ( ! mfrc522.PICC_IsNewCardPresent())
      return;
  
  
  if ( ! mfrc522.PICC_ReadCardSerial())               // Select one of the cards
      return;

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
      return;
  }
  byte blockAddr      = 4;
  byte trailerBlock   = 7;
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return;
  }

  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  
  /*  ****** Read last four bytes of block to compare with 127,212,42,198: if fail, no valid read *******/
  
  if (   (buffer[12] ^ mfrc522.uid.uidByte[0]) == 127   // Test against mask to verify
      && (buffer[13] ^ mfrc522.uid.uidByte[1]) == 212   // by XOR operation with UID
      && (buffer[14] ^ mfrc522.uid.uidByte[2]) == 42
      && (buffer[15] ^ mfrc522.uid.uidByte[3]) == 198) {
      guardBytes = true;
    }
    
  if   ( (buffer[11] != 177) &&  guardBytes) {        // 0xB1: This byte is used only on Supervisor card 
                                                      // for re-programming the HC12 channel
    mfrc522.MIFARE_Read(blockAddr, (byte*)ps, &size); // Here the data is copied into paramStore
    EEPROM.put(0, paramStore);                        // Here the paramStore data is copied into EEPROM 
    EEPROM.put(29, 111);                              // set flag for stored parameters
    pauseMe(20);
    sEcount = 1;                                      // reset the count to beginning as new params set
    displayParamsOnOLED();
    clearFromLine(5);
    u8x8.inverse(); 
    u8x8.draw2x2String(0, 6, "TIME-TAP");
    u8x8.noInverse(); 
    zeroSettings();
    writeInfoBigscreen();

  } else if ((buffer[11] == 177) && guardBytes) {     // proceed with re-program of the HC12 channel?
    
    if (alterChannelWarning()) {                      // test for genuine intent
      changeGlob_Chan(true);                          // proceed with GLOBAL channel change
    } else {
      displayParamsOnOLED();                          // discard the command and return to top menu.
      clearFromLine(5);
      u8x8.inverse(); 
      u8x8.draw2x2String(0, 6, "-DISCARD");
      u8x8.noInverse(); 
      zeroSettings();
      pauseMe(tick);
    }
    
  } else {                                
    displayParamsOnOLED();
    clearFromLine(5);
    u8x8.inverse(); 
    u8x8.draw2x2String(0, 6, "NO MATCH");
    u8x8.noInverse(); 
  }
  pauseMe(2*tick);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  displayParamsOnOLED();
  writeMenuCommands();
}

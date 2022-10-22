
/* RFID OPERATION
 * 
 * 
  uint8_t startCountsIndex = 1;         // (0)Number from 0 to 8 indentifying which of startCounts is used in this round, default 120 
  uint8_t walkUp = 10;                  // (1)
  uint8_t maxEnds = 4;                  // (2)Total number of Ends for competition
  uint8_t Details = 2;                  // (3)Single (1) or Double detail (2)
  
  uint8_t maxPrac = 2;                  // (4)Initially set as 2x practice ends
  uint8_t isFinals = 0;                 // (5)For alternating A & B session
  uint8_t breakPeriod                   // (6)
  uint8_t isAlternating = 0;            // (7)1 == Archer A, Archer B; 0 == Simultaneous
  
  uint8_t teamPlay = 0;                 // (8)
  uint8_t whichArcher = 1;              // (9)
  uint8_t Flint                         //(10)
  uint8_t curChan / Suprv.              //(11)
  
  "Time", "Walk", "Ends", "Dets", 
  "Prac", "Fnls", "BrkT", "Altr", 
  "Team", "A/B?", "Flnt", "curChan/Supv"
    Key,    Key.    Key.    Key
  "  
  
*/


/* The following is a function to read block 4-7 of a MIFARE card
 * and get/dump the data therein. We get the data, verify, and 
 * apply it to the various parameters as done in the menu setup, 
 * and only while in the menu top page.
 * A successful read generates a validation message on the OLED.
 */
 
void getRFID(struct PARAMSTORE *p_S){
  bool goodFlag   = true;
  byte currentCh  = p_Store.curChan;
  byte B_Ch       = p_Store.B_ScrCh;
  if ( ! mfrc522.PICC_IsNewCardPresent())
      return;
  if ( ! mfrc522.PICC_ReadCardSerial())                 // Select one of the cards
      return;

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    return;
  }
  byte blockAddr      = 5;                                  // Go to BLOCK 5 to read for KEY
//  byte trailerBlock   = 7;
  MFRC522::StatusCode status;
  byte buffer1[18];
  //byte size = sizeof(buffer1);
  byte len = sizeof(buffer1);
  //status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, /*trailerBlock*/ 5, &key, &(mfrc522.uid));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 5, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    return;
  }

  //status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer1, &size);
  status = mfrc522.MIFARE_Read(blockAddr, buffer1, &len);
//  printDebugLine(false, __LINE__, __NAME__);
//  buffChk(buffer1, 12, 15);
  /*  ****** Read last four bytes of block to compare with Key (127,212,42,198): if fail, no valid read *******/
  if (   (buffer1[12] ^ mfrc522.uid.uidByte[0]) == Key12     // Test against mask to verify
      && (buffer1[13] ^ mfrc522.uid.uidByte[1]) == Key13     // using XOR operation with UID
      && (buffer1[14] ^ mfrc522.uid.uidByte[2]) == Key14
      && (buffer1[15] ^ mfrc522.uid.uidByte[3]) == Key15) {  // then good to go on...

    
    
    //memset(buffer, 0, size);                                // wipe for next op
    byte buffer2[18];
    len = sizeof(buffer2);
    blockAddr      = 4;                                       // Go to BLOCK 4 to read for DATA
//    MFRC522::StatusCode status;
//    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, /*trailerBlock*/ 4, &key, &(mfrc522.uid));
//    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid));
//    if (status != MFRC522::STATUS_OK) {
//      return;
//    }
  
    //status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer2, &size);
    status = mfrc522.MIFARE_Read(blockAddr, buffer2, &len);
    //printDebugLine(false, __LINE__, __NAME__);
    buffChk(buffer2, 0, len);
    if (buffer2[15] == 1) {
      EEPROM.update(27, 180);                                  // Set flag for loaded banner
      pauseMe(160);
      clearFromLine(1);
      u8x8.draw2x2String(2, 2, "BANNER");
      u8x8.draw2x2String(2, 4, "LOADED");
      pauseMe(tick/2);
    } else {
      //EEPROM.put(27, 0);                                  // Set flag for no loaded banner
      EEPROM.update(27, (char)0);                                  // Set flag for no loaded banner
      pauseMe(120);
    }
      
    
    if (buffer2[11] != 177) {                                // 0xB1: Supervisor mode for re-programming the HC12 freq.

      //for (byte i = 0; i<=30; i++) EEPROM.put(i, 0);//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      
      //mfrc522.MIFARE_Read(blockAddr, (byte*)p_S, &size);   // Here the data is copied into p_S
      // printDebugLine(false, __LINE__, __NAME__);
      // see("sEcount", sEcount);
      mfrc522.MIFARE_Read(blockAddr, (byte*)p_S, &len); 
      // printDebugLine(false, __LINE__, __NAME__);
      // see("sEcount", sEcount);
      pauseMe(180);
      EEPROM.put(0, p_Store);                               // Here the p_Store data is copied into EEPROM 
      // printDebugLine(false, __LINE__, __NAME__);
      // see("sEcount", sEcount);
      pauseMe(200);
      if (EEPROM.read(0) > 8) {
        goodFlag = false;
        //printDebugLine(false, __LINE__, __NAME__);
        // debugEEPROM(0, 28);
      }        
      p_Store.curChan = currentCh;                          // write the channel(s) back in to the parameters
      p_Store.B_ScrCh = B_Ch;
      EEPROM.put(20, 111);                                  // set flag for stored parameters
      pauseMe(200);
      // printDebugLine(false, __LINE__, __NAME__);
      // see("sEcount", sEcount);
      // debugEEPROM(20, 0);
      if (goodFlag) displayParamsOnOLED(); //############################################+
      goodFlag ? clearFromLine(5) : clearFromLine(1) ;
      u8x8.inverse(); 
      u8x8.draw2x2String(0, 6, goodFlag ? "TIME-TAP" : "ERR-redo");
      u8x8.noInverse();
      // printDebugLine(false, __LINE__, __NAME__); 
      // see("sEcount", sEcount);      
      sEcount = 1;                                          // reset the count to beginning as new params set
      // see("sEcount", sEcount);
      zeroSettings();
      
      if (p_Store.teamPlay && goodFlag) {
        clearFromLine(1);
        u8x8.draw2x2String(0, 2, "Teamplay");
        pauseMe(1000);
        writeSet_sizeType(p_Store.teamPlay);                   // e_Teams 50
        pauseMe(tock);
        if (11 <= p_Store.teamPlay && p_Store.teamPlay <= 14) setB_Chan();             // setup the B screen; freq separation & channels
        
      } else if (goodFlag) writeInfoBigscreen();
    }
    else if (alterChannelWarning())   new_Channel(true);    // Supervisor: test intent & proceed with GLOBAL channel change
    else {                                                  // otherwise discard the command and return to top menu.
      displayParamsOnOLED(); 
      clearFromLine(5);
      u8x8.inverse(); 
      u8x8.draw2x2String(0, 6, "-DISCARD");
      u8x8.noInverse(); 
      zeroSettings();
      pauseMe(tick);
    }
  } else {                                                  // ...otherwise, bad card proffered                               
    displayParamsOnOLED();
    clearFromLine(5);
    u8x8.inverse(); 
    u8x8.draw2x2String(0, 6, "NO MATCH");
    u8x8.noInverse(); 
  }
  pauseMe(2*tick);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  goodFlag ? displayParamsOnOLED() : u8x8.draw2x2String(0, 6, " -OOPS ");
  writeMenuCommands();
  // printDebugLine(false, __LINE__, __NAME__);
  // see("sEcount", sEcount);
  // debugEEPROM(20, 0);
}

void buffChk( byte buff[18], byte lowBuff, byte hiBuff){                   // serial print the BUFFER
  Serial.println();
  for (byte i=lowBuff; i<=hiBuff; i++){
    Serial.print("Buf.");
    Serial.print(i);
    Serial.print(" ");
    Serial.println(buff[i]);
  }
  Serial.println();
}

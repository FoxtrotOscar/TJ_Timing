
/* RFID OPERATION
 * 
 * PARAMETERS UNDERGOING ALTERATION
  uint8_t   startCountsIndex    =  1;             // (0)  Number from 0 to 8 indentifying startCounts[] used, default 1 (120)
  uint8_t   walkUp              = 10;             // (1)
  uint8_t   maxEnds             = 10;             // (2)  Total number of Ends for competition
  uint8_t   Details             =  2;             // (3)  Single (1) or Double detail (2)

  uint8_t   maxPrac             =  2;             // (4)  Initially set as 2x practice ends
  uint8_t   isFinals            =  0;             // (5)  For alternating A & B session
  uint8_t   breakPeriod         = 10;             // (6)  Between sessions break times, max 240min, default 10
  uint8_t   isAlternating       =  0;             // (7)  1 / 2 == Recurve / Compound A/B; 0 == Simultaneous

  uint8_t   teamPlay            =  0;             // (8)  Teams: 1: mixed Recurve, 2 mixed Comp; 3 = Recurve, 4 Comp; 11 - 14 Teamplay ditto
  uint8_t   whichArcher         =  0;             // (9)  0 = ""; 1 = "A"; 2 = "B"
  uint8_t   isFlint             =  0;             //(10)  if True this is a flint round
  uint8_t   curChan             =  0;             //(11)

  uint8_t   B_ScrCh             =  0;             //(12)  shows chann no. if dual screens are set up
  uint8_t   which_Scr_1st       =  0;             //(13)  false until screen-flip in progress, then 1 or 2 for A or B
  uint8_t   PS14                =  0;             //(14)|___  Spares
  uint8_t   PS15                =  1;             //(15)| Banner loaded ?
  uint8_t   PS16                =  0;             //(16)|___  Spares
  uint8_t   PS17                =  0;             //(17)|

  
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
 
bool getRFID(void){  //bool getRFID(struct PARAMSTORE *p_S){
  //printDebugLine(false, __LINE__, __NAME__);
  bool goodResult = false;
  byte currentCh  = p_Store.curChan;
  byte B_Ch       = p_Store.B_ScrCh;
  if ( ! mfrc522.PICC_IsNewCardPresent())
      return false;
  if ( ! mfrc522.PICC_ReadCardSerial())                 // Select one of the cards
      return false;

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
      &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    return false;
  }
  byte blockAddr      = 5;                                  // Go to BLOCK 5 to read for KEY
  //byte trailerBlock    = 7;       //??????????????????????????????????????????????????????????
  MFRC522::StatusCode status;
  byte buffer1[18];
  byte len = sizeof(buffer1);
  //status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    
    printDebugLine(false, __LINE__, __NAME__);
    see(status);
    
    return false;
  }

  status = mfrc522.MIFARE_Read(blockAddr, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    printDebugLine(false, __LINE__, __NAME__);
    see(status);
    mfrc522.PCD_Init();
    return false;
  }  

  /*  ****** Read last four bytes of block to compare with Key (127,212,42,198): if fail, no valid read *******/
  if (   (buffer1[12] ^ mfrc522.uid.uidByte[0]) == Key12     // Test against mask to verify
      && (buffer1[13] ^ mfrc522.uid.uidByte[1]) == Key13     // using XOR operation with UID
      && (buffer1[14] ^ mfrc522.uid.uidByte[2]) == Key14
      && (buffer1[15] ^ mfrc522.uid.uidByte[3]) == Key15) {  // then good to go on...

    
    
    //memset(buffer1, 0, len);                                // DEBUG only: wipe for next op
    //printDebugLine(false, __LINE__, __NAME__);
    byte buffer2[18];
    len = sizeof(buffer2);
    //byte trailerBlock   = 7;       //??????????????????????????????????????????????????????????
    blockAddr      = 4;                                       // Go to BLOCK 4 to read for DATA
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockAddr, &key, &(mfrc522.uid));
    //status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      printDebugLine(false, __LINE__, __NAME__);
      see(status);
      return false;
    }
  
    status = mfrc522.MIFARE_Read(blockAddr, buffer2, &len);
    if (status != MFRC522::STATUS_OK) {
      printDebugLine(false, __LINE__, __NAME__);
      see(status);
      return false;
    }
    buffChk(buffer2, 0, len);  // debug only7

    p_Store.ifaaIndoor = (buffer2[12] == 1 ? true : false);     // ifaaIndoor??
    pauseMe(120);
    if (buffer2[13] == 1) {                                     // Banner??
      EEPROM.update(27, 180);                                   // Set flag for loaded banner
      pauseMe(160);
      clearFromLine(1);
      u8x8.draw2x2String(2, 2, "BANNER");
      u8x8.draw2x2String(2, 4, "LOADED");
      pauseMe(tick/2);
    } else {
      EEPROM.update(27, (char)0);                               // Set flag for no loaded banner
      pauseMe(120);
    }
        
    if (buffer2[11] != 177) {                                   // 0xB1: ? Supervisor mode for re-programming the HC12 freq.
      
      memcpy( &p_Store, buffer2, sizeof(p_Store) );//memcpy( &p_S, buffer2, sizeof(p_S) );
      pauseMe(180);
      bool goodFlag = (p_Store.startCountsIndex >= (sizeof(startCounts)/4) || p_Store.Details> 2) ? false : true;
      Serial.print("goodFlag  ") ; Serial.println(goodFlag);
      if (goodFlag) {
        EEPROM.put(0, p_Store);                                   // Here the p_Store data is copied into EEPROM 
        pauseMe(200);
        p_Store.curChan = currentCh;                              // write the channel(s) back in to the parameters
        p_Store.B_ScrCh = p_Store.teamPlay ?  B_Ch : 0;
        EEPROM.put(20, 111);                                      // set flag for stored parameters
        pauseMe(200);
        displayParamsOnOLED();
        if (p_Store.ifaaIndoor) {u8x8.setCursor(10, 4); u8x8.print("IFAA");}
        if (p_Store.isFlint)    {u8x8.setCursor(10, 4); u8x8.print("FLINT");}
        clearFromLine(5) ;
        u8x8.inverse(); 
        u8x8.draw2x2String(0, 6, "TIME-TAP");
        zeroSettings();                                       // reset the count to beginning as new params set
        goodResult = true;
      } else {
        clearFromLine(1) ;
        u8x8.inverse();
        u8x8.draw2x2String(0, 6, "ERR-redo");
        goodResult = false;
      }
      u8x8.noInverse();
      if (p_Store.teamPlay) {
        clearFromLine(1);
        u8x8.draw2x2String(0, 2, "Teamplay");
        pauseMe(1000);
        writeSet_sizeType(p_Store.teamPlay);                // e_Teams 50
        pauseMe(tock);
        if (11 <= p_Store.teamPlay && p_Store.teamPlay <= 14) setB_Chan();             // setup the B screen; freq separation & channels
        
      } else writeInfoBigscreen();
    } else if (alterChannelWarning())   {
      new_Channel(true);    // Supervisor: test intent & proceed with GLOBAL channel change
      goodResult = true;
            
    } else {                                                  // otherwise discard the command and return to top menu.
      displayParamsOnOLED(); 
      clearFromLine(5);
      u8x8.inverse(); 
      u8x8.draw2x2String(0, 6, "-DISCARD");
      u8x8.noInverse(); 
      zeroSettings();
      goodResult = false;
      pauseMe(tick);
    }
  } else {                                                  // ...otherwise, bad card proffered                               
  displayParamsOnOLED();
  clearFromLine(5);
  u8x8.inverse(); 
  u8x8.draw2x2String(0, 6, "NO MATCH");
  u8x8.noInverse();
  goodResult = false; 
  }
  pauseMe(2*tick);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
  displayParamsOnOLED();
  writeMenuCommands();
  printDebugLine(false, __LINE__, __NAME__);
  return goodResult;
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


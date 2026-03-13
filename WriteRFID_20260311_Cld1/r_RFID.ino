#include "Utils.h"

bool writeBannerToCard(MFRC522& rfid, MFRC522::MIFARE_Key& key);
bool writePresentedCardOnce();
//void dump_byte_array(byte *buffer, byte bufferSize);
static bool readBlock16(MFRC522& rfid, MFRC522::MIFARE_Key& key,
                        uint8_t blockAddr, uint8_t out[16])
{
  uint8_t trailer = (blockAddr & 0xFC) + 3;  // (block/4)*4 + 3
  MFRC522::StatusCode st =
    rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailer, &key, &(rfid.uid));
  if (st != MFRC522::STATUS_OK) return false;

  byte buf[18];
  byte size = sizeof(buf);
  st = (MFRC522::StatusCode)rfid.MIFARE_Read(blockAddr, buf, &size);
  if (st != MFRC522::STATUS_OK) return false;

  memcpy(out, buf, 16);
  return true;
}
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

// ---------------------------
// WRITE_MODE helpers
// ---------------------------
bool isCardPresentNow() {  

  if (!mfrc522.PICC_IsNewCardPresent()) return false;
  if (!mfrc522.PICC_ReadCardSerial()) return false;
  Serial.println("iCPN: UID OK");
  return true;
}

bool cardIsStillPresent() {
  byte atqa[2];
  byte atqaSize = 2;  // IMPORTANT: reset every call
  MFRC522::StatusCode s = mfrc522.PICC_RequestA(atqa, &atqaSize);
  return (s == MFRC522::STATUS_OK);
}
// bool cardIsStillPresent() {
//   byte bufferATQA[2];
//   byte bufferSize = sizeof(bufferATQA);
//   byte atqaSize = 2;  // IMPORTANT: reset every call
//   MFRC522::StatusCode s = mfrc522.PICC_RequestA(bufferATQA, &atqaSize);
//   return (s == MFRC522::STATUS_OK);
// }


void waitForCardRemoval() {
  endCardSession();
  while (!cardIsGoneStable(5)) {
    delay(50);
  }
}


// Require N consecutive "absent" reads before we believe it's gone
bool cardIsGoneStable(uint8_t needed = 5) {
  static uint8_t goneCount = 0;

  if (cardIsStillPresent()) {  // RequestA
    goneCount = 0;
    return false;
  }
  if (goneCount < needed) goneCount++;
  return goneCount >= needed;
}

void endCardSession() {
  mfrc522.PICC_HaltA();                                        // tell PICC we're done (good practice)
  mfrc522.PCD_StopCrypto1();                                   // stop encryption on the PCD (good practice)
  mfrc522.uid.size = 0;   // mark UID as not loaded
}



static bool isTrailerBlock(uint8_t blockAddr) {
  return (blockAddr % 4) == 3;
}

bool writePresentedCardOnce() {
  Serial.println("WP: start");

  if (mfrc522.uid.size == 0) {
    Serial.println("WP: no UID loaded");
    return false;
  }
Serial.print(F("uid.size: "));      Serial.println(mfrc522.uid.size);
  Serial.print("WP: UID=");
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.print("WP: type=");
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println("WP: not MIFARE Classic");
    endCardSession();
    return false;
  }

  const byte block4 = 4;
  const byte block5 = 5;
  const byte trailer7 = 7;

  MFRC522::StatusCode status =
    mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                             trailer7, &key, &(mfrc522.uid));
  Serial.print("WP: auth=");
  Serial.println(mfrc522.GetStatusCodeName(status));

  if (status != MFRC522::STATUS_OK) {
    endCardSession();
    return false;
  }

  byte dataBlock[16];
  for (byte i=0;i<16;i++) dataBlock[i] = dataStore[i];
  dataBlock[13] = (bannerIsReady() && dataStore[13]) ? 1 : 0;
  status = mfrc522.MIFARE_Write(block4, dataBlock, 16);
  Serial.print("WP: write4=");
  Serial.println(mfrc522.GetStatusCodeName(status));
  if (status != MFRC522::STATUS_OK) { endCardSession(); return false; }

  byte guardBlock[16] = {0};
  guardBlock[12] = (byte)(mfrc522.uid.uidByte[0] ^ Key12);
  guardBlock[13] = (byte)(mfrc522.uid.uidByte[1] ^ Key13);
  guardBlock[14] = (byte)(mfrc522.uid.uidByte[2] ^ Key14);
  guardBlock[15] = (byte)(mfrc522.uid.uidByte[3] ^ Key15);

  status = mfrc522.MIFARE_Write(block5, guardBlock, 16);
  Serial.print("WP: write5=");
  Serial.println(mfrc522.GetStatusCodeName(status));
  if (status != MFRC522::STATUS_OK) { endCardSession(); return false; }

  bool bannerOk = true;
    if (bannerIsReady() && dataStore[13]) {  // only show banner feedback if user chose to write it
        wipeOLED(true);
        u8x8.draw2x2String(0, 2, "BANNER  ");
        u8x8.draw2x2String(0, 5, "  OK    ");
        pauseMe(800);
    }
  //   bannerOk = writeBannerToCard(mfrc522, key);
  //   Serial.println(bannerOk ? F("Banner written OK") : F("Banner write FAILED"));
  //   if (bannerOk) bannerReset();  // optional: make END one-shot
  


  if (bannerIsReady() && dataStore[13]) {  // only write banner if user chose to include it
    bannerOk = writeBannerToCard(mfrc522, key);
    Serial.println(bannerOk ? F("Banner written OK") : F("Banner write FAILED"));

    if (bannerOk && bannerGetOneShot()) {
      bannerReset();
      Serial.println(F("Banner one-shot: reset"));
    }
  }

  Serial.println("WP: done OK");
  endCardSession();
  return bannerOk;
}

// void doCloneRead() {
//   wipeOLED(true);
//   u8x8.draw2x2String(0, 2, "READ");
//   u8x8.draw2x2String(0, 5, "CARD");

//   bool ok = loadCardIntoWorkingSet();

//   wipeOLED(true);
//   if (ok) u8x8.draw2x2String(0, 3, "LOADED");
//   else    u8x8.draw2x2String(0, 3, "READFAIL");

//   pauseMe(800);
//   writeReady();
// }


bool loadCardIntoWorkingSet() {
  bool paramsOk = false;
  bool guardOk  = false;
  bool bannerOk = false;
  Serial.println("LOAD: start");
  static uint8_t tempPayload[BANNER_MAX_PAYLOAD];

  // Must already have a UID loaded by isCardPresentNow()
  if (mfrc522.uid.size == 0) {
    Serial.println("LOAD: no UID loaded");
    return false;
  }

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println("LOAD: not MIFARE Classic");
    endCardSession();
    return false;
  }

  // ---- Read params block 4 into dataStore ----
  uint8_t b4[16];
  if (!readBlock16(mfrc522, key, 4, b4)) {
    Serial.println("LOAD: read block 4 failed");
    endCardSession();
    return false;
  }
  memcpy(dataStore, b4, 16);
  Serial.println("LOAD: params loaded");
  paramsOk = true;

  // ---- Read guard block 5 and verify ----
  uint8_t b5[16];
  if (!readBlock16(mfrc522, key, 5, b5)) {
    Serial.println("LOAD: read block 5 failed");
    endCardSession();
    return false;
  }

  uint8_t g0 = (uint8_t)(mfrc522.uid.uidByte[0] ^ Key12);
  uint8_t g1 = (uint8_t)(mfrc522.uid.uidByte[1] ^ Key13);
  uint8_t g2 = (uint8_t)(mfrc522.uid.uidByte[2] ^ Key14);
  uint8_t g3 = (uint8_t)(mfrc522.uid.uidByte[3] ^ Key15);
  guardOk = (b5[12] == g0 && b5[13] == g1 && b5[14] == g2 && b5[15] == g3);
  Serial.println(guardOk ? "LOAD: guard OK" : "LOAD: guard FAIL");

  // ---- Try to load banner (optional) ----
  bannerReset();    // makes: bannerReady = false; bannerPayloadLen = bannerEntryCount = 0;
  
  uint8_t hdr[16];
  bool hdrOk = readBlock16(mfrc522, key, 8, hdr)
            //&& hdr[0]=='B' && hdr[1]=='N' && hdr[2]=='R' && hdr[3]=='1';
            && hdr[0]=='B' && hdr[1]=='N' && hdr[2]=='R' && hdr[3]=='2';

  if (!hdrOk) {
    Serial.println("LOAD: no banner header");
    dataStore[13] = 0;        // mirror for display
    // report "No Banner"    
    endCardSession();

    return paramsOk;  // params cloned even if no banner
  }

  uint16_t payloadLen = (uint16_t)hdr[8] | ((uint16_t)hdr[9] << 8);
  uint16_t crcExpected = (uint16_t)hdr[10] | ((uint16_t)hdr[11] << 8);
  uint8_t entryCount = hdr[6];    //bannerEntryCount = hdr[6];

  if (payloadLen == 0 || payloadLen > BANNER_MAX_PAYLOAD) {
  dataStore[13] = 0;
  endCardSession();
  return paramsOk;
  }

  uint16_t off = 0;
  uint8_t block = 9;

  while (off < payloadLen) {
    if (block >= 64) { 
      Serial.println("LOAD: banner overruns card"); 
      endCardSession(); 
      return paramsOk; 
    }
    if (isTrailerBlock(block)) { block++; continue; }

    uint8_t buf16[16];
    if (!readBlock16(mfrc522, key, block, buf16)) {
      Serial.print("LOAD: read banner block failed at ");
      Serial.println(block);
      endCardSession();
      return paramsOk;
    }

    for (uint8_t i = 0; i < 16 && off < payloadLen; i++) {
      tempPayload[off++] = buf16[i];
    }
    block++;
  }

  uint16_t crcGot = crc16_ccitt(tempPayload, payloadLen, 0xFFFF);
  if (crcGot != crcExpected) {
    Serial.println("LOAD: banner CRC mismatch");
    dataStore[13] = 0;
    endCardSession();
    return paramsOk;          // <-- don’t fail old cards
  }

  // Hand banner into Banner.cpp (it owns the real storage)
  bannerLoadFromCard(tempPayload, payloadLen, entryCount);
  bannerOk = true;
  Serial.println("LOAD: banner OK");
  dataStore[13] = bannerOk ? 1 : 0;  // Mirror banner presence into param byte for display

  endCardSession();
  return paramsOk;
}

void writeRFID() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial())   return;

  bool ok = writePresentedCardOnce();
  wipeOLED(true);
  u8x8.draw2x2String(0, 3, ok ? "SUCCESS " : "FAILED  ");

  requireRemove = true;     // let your gating handle removal
  pauseMe(tick);
  writeReady();
}


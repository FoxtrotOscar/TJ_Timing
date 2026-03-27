#include "Banner.h"
#include <MFRC522.h>

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// ---------------- Banner Upload + Card Write ----------------

static uint8_t  bannerPayload[BANNER_MAX_PAYLOAD];
static uint16_t bannerPayloadLen  = 0;
static uint8_t  bannerEntryCount  = 0;
static bool     bannerReady       = false;
static bool     bannerOneShot     = false;
bool            bannerJustArmed   = false;

void bannerSetOneShot(bool on) { bannerOneShot = on; }
bool bannerGetOneShot()        { return bannerOneShot; }

void bannerLoadFromCard(const uint8_t* payload,
                        uint16_t       len,
                        uint8_t        entryCount)
{
  if (len > BANNER_MAX_PAYLOAD) return;
  //printDebugLine(__LINE__, __NAME__);
  
  memcpy(bannerPayload, payload, len);
  bannerPayloadLen  = len;
  bannerEntryCount  = entryCount;
  bannerReady       = true;
  

  //printDebugLine(__LINE__, __NAME__);
}

bool bannerIsReady() { return bannerReady; }

// ---------- CRC16-CCITT ----------
uint16_t crc16_ccitt(const uint8_t* data, uint16_t len, uint16_t crc) {
  while (len--) {
    crc ^= (uint16_t)(*data++) << 8;
    for (uint8_t i = 0; i < 8; i++) {
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
  }
  return crc;
}

// ---------- MIFARE 1K helpers ----------
static uint8_t trailerForBlock(uint8_t blockAddr) {
  return (blockAddr & 0xFC) + 3;
}
static bool isTrailerBlock(uint8_t blockAddr) {
  return (blockAddr % 4) == 3;
}
static bool authSectorForBlock(MFRC522& rfid,
                               MFRC522::MIFARE_Key& key,
                               uint8_t blockAddr) {
  uint8_t trailer = trailerForBlock(blockAddr);
  auto status = rfid.PCD_Authenticate(
                  MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                  trailer, &key, &(rfid.uid));
  return status == MFRC522::STATUS_OK;
}
static bool writeBlock16(MFRC522& rfid,
                         MFRC522::MIFARE_Key& key,
                         uint8_t blockAddr,
                         const uint8_t data[16])
{
  if (!authSectorForBlock(rfid, key, blockAddr)) return false;
  auto status = (MFRC522::StatusCode)rfid.MIFARE_Write(blockAddr, (byte*)data, 16);
  return status == MFRC522::STATUS_OK;
}
static bool writeBannerPayloadToBlocks(MFRC522& rfid,
                                       MFRC522::MIFARE_Key& key,
                                       const uint8_t* payload,
                                       uint16_t payloadLen) {
  uint8_t  block = 9;
  uint16_t off   = 0;
  while (off < payloadLen) {
    if (block >= 64) return false;
    if (isTrailerBlock(block)) { block++; continue; }
    uint8_t buf[16];
    for (uint8_t i = 0; i < 16; i++) {
      buf[i] = (off < payloadLen) ? payload[off++] : 0x00;
    }
    if (!writeBlock16(rfid, key, block, buf)) return false;
    block++;
  }
  Serial.print(F("bannerReady: "));   Serial.println(bannerReady);
  Serial.print(F("payloadLen: "));    Serial.println(bannerPayloadLen);

  return true;
}

bool writeBannerToCard(MFRC522& rfid, MFRC522::MIFARE_Key& key) {
  if (rfid.uid.size == 0)                                   return false;
  if (!bannerReady)                                         return false;
  if (bannerPayloadLen == 0 ||
      bannerPayloadLen > BANNER_MAX_PAYLOAD)                return false;

  uint16_t crc = crc16_ccitt(bannerPayload, bannerPayloadLen, 0xFFFF);

  uint8_t header[16] = {0};
  header[0] = 'B'; header[1] = 'N'; header[2] = 'R'; header[3] = '2'; // version 2
  header[4] = 2;                  // payload format version (was 1, now includes X)
  header[5] = 0;                  // flags
  header[6] = bannerEntryCount;
  header[7] = 0;
  header[8]  = (uint8_t)(bannerPayloadLen & 0xFF);
  header[9]  = (uint8_t)(bannerPayloadLen >> 8);
  header[10] = (uint8_t)(crc & 0xFF);
  header[11] = (uint8_t)(crc >> 8);

  if (!writeBannerPayloadToBlocks(rfid, key, bannerPayload, bannerPayloadLen)) return false;
  return writeBlock16(rfid, key, 8, header);
}

// ---------- Payload packing ----------
static bool appendByte(uint8_t b) {
  if (bannerPayloadLen >= BANNER_MAX_PAYLOAD) return false;
  bannerPayload[bannerPayloadLen++] = b;
  return true;
}
static bool appendU16LE(uint16_t v) {
  return appendByte((uint8_t)(v & 0xFF)) && appendByte((uint8_t)(v >> 8));
}
static bool appendBytes(const uint8_t* p, uint8_t n) {
  while (n--) if (!appendByte(*p++)) return false;
  return true;
}

// Record types
static const uint8_t REC_CLEAR = 0x01;
static const uint8_t REC_TEXT  = 0x02;

static uint16_t dur10ms(uint32_t ms) {
  uint32_t d = (ms + 5) / 10;
  if (d > 65535) d = 65535;
  return (uint16_t)d;
}

static bool addClear(uint32_t duration_ms) {
  if (!appendByte(REC_CLEAR))            return false;
  if (!appendU16LE(dur10ms(duration_ms))) return false;
  bannerEntryCount++;
  return true;
}

// ---------- UPDATED: addText now includes x position ----------
// Record layout: type | font | colour | x | yBaseline | duration10ms(2) | len | text[]
static bool addText(uint8_t font, uint8_t colour,
                    uint8_t x, uint8_t yBaseline,
                    uint32_t duration_ms, const char* text)
{
  uint8_t len = (uint8_t)strlen(text);
  if (len > 60) len = 60;

  if (!appendByte(REC_TEXT))              return false;
  if (!appendByte(font))                  return false;
  if (!appendByte(colour))               return false;
  if (!appendByte(x))                    return false;   // NEW: x position
  if (!appendByte(yBaseline))            return false;
  if (!appendU16LE(dur10ms(duration_ms))) return false;
  if (!appendByte(len))                  return false;
  if (!appendBytes((const uint8_t*)text, len)) return false;
  bannerEntryCount++;
  return true;
}

// ---------- Serial line reader ----------
static bool readLineFromSerial(char* out, uint8_t outSize) {
  static uint8_t idx = 0;
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      out[idx] = 0;
      idx = 0;
      return true;
    }
    if (idx < outSize - 1) out[idx++] = c;
  }
  return false;
}

static uint8_t splitCSV(char* line, char* tok[], uint8_t maxTok) {
  uint8_t n = 0;
  char* p = line;
  while (n < maxTok) {
    while (*p == ' ' || *p == '\t') p++;
    if (*p == 0) break;
    tok[n++] = p;
    while (*p && *p != ',') p++;
    if (*p == ',') { *p = 0; p++; }
  }
  return n;
}

void bannerReset() {
  bannerPayloadLen  = 0;
  bannerEntryCount  = 0;
  bannerReady       = false;
}

void bannerShowSummary() {
  Serial.print(F("Banner records: "));
  Serial.print(bannerEntryCount);
  Serial.print(F(", payload bytes: "));
  Serial.println(bannerPayloadLen);
}

// ---------- UPDATED: bannerSerialIngest ----------
// TEXT command now: TEXT,font,col,x,y,duration_ms,text  (7 fields)

bool bannerSerialIngest() {
  char line[128];
  if (!readLineFromSerial(line, sizeof(line))) return false;

  char* s = line;
  while (*s == ' ' || *s == '\t') s++;
  if (*s == 0) return false;

  char* tok[8];
  uint8_t nt = splitCSV(s, tok, 8);

  auto eqi = [](const char* a, const char* b) -> bool {
    while (*a && *b) {
      char ca = *a++, cb = *b++;
      if (ca >= 'a' && ca <= 'z') ca -= 32;
      if (cb >= 'a' && cb <= 'z') cb -= 32;
      if (ca != cb) return false;
    }
    return *a == 0 && *b == 0;
  };

  if (eqi(tok[0], "HELP")) {
    Serial.println(F("Commands:"));
    Serial.println(F("  RESET"));
    Serial.println(F("  CLEAR,duration_ms"));
    Serial.println(F("  ONESHOT,1|0"));
    Serial.println(F("  TEXT,font,colour,x,y,duration_ms,text"));
    Serial.println(F("  SHOW"));
    Serial.println(F("  END"));
    return false;
  }

  if (eqi(tok[0], "RESET")) {
    bannerReset();
    Serial.println(F("OK: banner reset"));
    return false;
  }

  if (eqi(tok[0], "SHOW"))  { bannerShowSummary(); return false; }

  if (eqi(tok[0], "END")) {
    bannerReady = true;
    Serial.println(F("OK: banner ready. Present card to write."));
    bannerShowSummary();
    return true;              // ← only true return
  }

  if (eqi(tok[0], "CLEAR")) {
    if (nt < 2) { Serial.println(F("ERR: CLEAR needs duration_ms")); return false; }
    uint32_t dms = (uint32_t)strtoul(tok[1], nullptr, 10);
    if (!addClear(dms)) { Serial.println(F("ERR: payload full")); return false; }
    Serial.println(F("OK: CLEAR added"));
    return false;
  }

  if (eqi(tok[0], "ONESHOT")) {
    if (nt < 2) { Serial.println(F("ERR: ONESHOT needs 0 or 1")); return false; }
    bannerSetOneShot(strtoul(tok[1], nullptr, 10) != 0);
    Serial.print(F("OK: ONESHOT="));
    Serial.println(bannerGetOneShot() ? 1 : 0);
    return false;
  }

  if (eqi(tok[0], "TEXT")) {
    if (nt < 7) {
      Serial.println(F("ERR: TEXT needs font,colour,x,y,duration_ms,text"));
      return false;
    }
    uint8_t  font   = (uint8_t) strtoul(tok[1], nullptr, 10);
    uint8_t  col    = (uint8_t) strtoul(tok[2], nullptr, 10);
    uint8_t  x      = (uint8_t) strtoul(tok[3], nullptr, 10);
    uint8_t  y      = (uint8_t) strtoul(tok[4], nullptr, 10);
    uint32_t dms    = (uint32_t)strtoul(tok[5], nullptr, 10);
    const char* text = tok[6];
    if (!addText(font, col, x, y, dms, text)) {
      Serial.println(F("ERR: payload full"));
      return false;
    }
    Serial.println(F("OK: TEXT added"));
    return false;
  }

  Serial.println(F("ERR: unknown command (try HELP)"));
  return false;
}
// void bannerSerialIngest() {
//   char line[128];
//   if (!readLineFromSerial(line, sizeof(line))) return;

//   char* s = line;
//   while (*s == ' ' || *s == '\t') s++;
//   if (*s == 0) return;

//   char* tok[8];
//   uint8_t nt = splitCSV(s, tok, 8);

//   auto eqi = [](const char* a, const char* b) -> bool {
//     while (*a && *b) {
//       char ca = *a++, cb = *b++;
//       if (ca >= 'a' && ca <= 'z') ca -= 32;
//       if (cb >= 'a' && cb <= 'z') cb -= 32;
//       if (ca != cb) return false;
//     }
//     return *a == 0 && *b == 0;
//   };

//   if (eqi(tok[0], "HELP")) {
//     Serial.println(F("Commands:"));
//     Serial.println(F("  RESET"));
//     Serial.println(F("  CLEAR,duration_ms"));
//     Serial.println(F("  ONESHOT,1|0"));
//     Serial.println(F("  TEXT,font,colour,x,y,duration_ms,text"));  // updated
//     Serial.println(F("  SHOW"));
//     Serial.println(F("  END"));
//     return;
//   }

//   if (eqi(tok[0], "RESET")) {
//     bannerReset();
//     Serial.println(F("OK: banner reset"));
//     return;
//   }

//   if (eqi(tok[0], "SHOW")) {
//     bannerShowSummary();
//     return;
//   }

//   if (eqi(tok[0], "END")) {
//     bannerReady = true;
//     Serial.println(F("OK: banner ready. Present card to write."));
//     bannerShowSummary();
//     return;
//   }

//   if (eqi(tok[0], "CLEAR")) {
//     if (nt < 2) { Serial.println(F("ERR: CLEAR needs duration_ms")); return; }
//     uint32_t dms = (uint32_t)strtoul(tok[1], nullptr, 10);
//     if (!addClear(dms)) { Serial.println(F("ERR: payload full")); return; }
//     Serial.println(F("OK: CLEAR added"));
//     return;
//   }

//   if (eqi(tok[0], "ONESHOT")) {
//     if (nt < 2) { Serial.println(F("ERR: ONESHOT needs 0 or 1")); return; }
//     bannerSetOneShot(strtoul(tok[1], nullptr, 10) != 0);
//     Serial.print(F("OK: ONESHOT="));
//     Serial.println(bannerGetOneShot() ? 1 : 0);
//     return;
//   }

//   // TEXT,font,colour,x,y,duration_ms,text  — 7 tokens required
//   if (eqi(tok[0], "TEXT")) {
//     if (nt < 7) {
//       Serial.println(F("ERR: TEXT needs font,colour,x,y,duration_ms,text"));
//       return;
//     }
//     uint8_t  font   = (uint8_t) strtoul(tok[1], nullptr, 10);
//     uint8_t  col    = (uint8_t) strtoul(tok[2], nullptr, 10);
//     uint8_t  x      = (uint8_t) strtoul(tok[3], nullptr, 10);  // NEW
//     uint8_t  y      = (uint8_t) strtoul(tok[4], nullptr, 10);
//     uint32_t dms    = (uint32_t)strtoul(tok[5], nullptr, 10);
//     const char* text = tok[6];
//     if (!addText(font, col, x, y, dms, text)) {
//       Serial.println(F("ERR: payload full"));
//       return;
//     }
//     Serial.println(F("OK: TEXT added"));
//     return;
//   }

//   Serial.println(F("ERR: unknown command (try HELP)"));
// }



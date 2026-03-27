/*
 * BannerLoad.ino
 * =============
 * Reads a banner payload from a MIFARE 1K card and stores it in EEPROM.
 *
 * Card layout (set by RFID Writer / Banner Composer):
 *   Block 8       : 16-byte header  (magic, version, entryCount, payloadLen, CRC16)
 *   Blocks 9+     : payload bytes   (trailer blocks 11,15,19... skipped automatically)
 *
 * EEPROM layout: see Banner.h (EE_BNR_* constants, starts at addr 32)
 *
 * Public API:
 *   bool bnrLoadFromCard(MFRC522& rfid, MFRC522::MIFARE_Key& key)
 *     Returns true if a valid BNR2 banner was read, CRC-verified and stored.
 *     Sets EE_BNR_FLAG=1 on success, 0 on any failure.
 *     Preserves existing EE_BNR_RUN preference if already set; defaults to 1.
 *
 *   bool bnrReadBlock16(MFRC522& rfid, MFRC522::MIFARE_Key& key,
 *                       uint8_t blockAddr, uint8_t out[16])
 *     Low-level helper: authenticates sector, reads one 16-byte block.
 *     Also used by r_RFID.ino for blocks 4 and 5.
 */

#include "Banner.h"
#include <MFRC522.h>
#include <EEPROM.h>

// ----------------------------------------------------------------
//  Low-level block reader  (replaces readBlock16 from RfidBlocks)
// ----------------------------------------------------------------
bool bnrReadBlock16(MFRC522& rfid, MFRC522::MIFARE_Key& key,
                    uint8_t blockAddr, uint8_t out[16])
{
  uint8_t trailer = (blockAddr & 0xFC) + 3;   // sector trailer for this block
  auto status = rfid.PCD_Authenticate(
                  MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                  trailer, &key, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) return false;

  byte buf18[18];
  byte len = sizeof(buf18);
  status = rfid.MIFARE_Read(blockAddr, buf18, &len);
  if (status != MFRC522::STATUS_OK) return false;

  memcpy(out, buf18, 16);
  return true;
}

// ----------------------------------------------------------------
//  Main loader
// ----------------------------------------------------------------
bool bnrLoadFromCard(MFRC522& rfid, MFRC522::MIFARE_Key& key)
{
  // --- Read and validate header (block 8) ---
  uint8_t hdr[16];
  if (!bnrReadBlock16(rfid, key, 8, hdr)) {
    EEPROM.update(EE_BNR_FLAG, 0);
    return false;
  }

  // Magic check: must be 'BNR2'
  if (hdr[0] != BNR_MAGIC_0 || hdr[1] != BNR_MAGIC_1 ||
      hdr[2] != BNR_MAGIC_2 || hdr[3] != BNR_MAGIC_3) {
    EEPROM.update(EE_BNR_FLAG, 0);
    return false;
  }

  // Format version check
  if (hdr[4] != BNR_FMT_VER) {
    EEPROM.update(EE_BNR_FLAG, 0);
    return false;
  }

  uint16_t payloadLen  = bnr_rdU16LE(&hdr[8]);
  uint16_t crcExpected = bnr_rdU16LE(&hdr[10]);

  if (payloadLen == 0 || payloadLen > BNR_MAX_PAYLOAD) {
    EEPROM.update(EE_BNR_FLAG, 0);
    return false;
  }

  // --- Read payload blocks starting at block 9, skipping trailers ---
  static uint8_t payload[BNR_MAX_PAYLOAD];   // static: avoids stack pressure
  uint16_t off   = 0;
  uint8_t  block = 9;

  while (off < payloadLen) {
    if (block >= 64) {                        // MIFARE 1K: blocks 0..63
      EEPROM.update(EE_BNR_FLAG, 0);
      return false;
    }
    if ((block % 4) == 3) { block++; continue; }  // skip trailer blocks

    uint8_t b16[16];
    if (!bnrReadBlock16(rfid, key, block, b16)) {
      EEPROM.update(EE_BNR_FLAG, 0);
      return false;
    }
    for (uint8_t i = 0; i < 16 && off < payloadLen; i++)
      payload[off++] = b16[i];
    block++;
  }

  // --- CRC verification ---
  uint16_t crcGot = bnr_crc16(payload, payloadLen);
  if (crcGot != crcExpected) {
    EEPROM.update(EE_BNR_FLAG, 0);
    return false;
  }

  // --- Write to EEPROM ---
  EEPROM.update(EE_BNR_FLAG, 0);             // mark invalid while writing

  // Preserve run preference if already valid, else default to auto-run
  uint8_t run = EEPROM.read(EE_BNR_RUN);
  if (run != 0 && run != 1) EEPROM.update(EE_BNR_RUN, 1);

  bnr_eeWriteU16(EE_BNR_LEN_L, payloadLen);
  bnr_eeWriteU16(EE_BNR_CRC_L, crcExpected);

  for (uint16_t i = 0; i < payloadLen; i++)
    EEPROM.update(EE_BNR_PAYLOAD + i, payload[i]);

  EEPROM.update(EE_BNR_FLAG, 1);             // mark valid
  return true;
}
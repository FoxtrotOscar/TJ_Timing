#pragma once
#include <Arduino.h>

// ============================================================
//  Banner.h  —  shared constants for BannerLoad + BannerPlay
// ============================================================

// ---------- Card format ----------
// Block 8  : header
//   [0..3]  magic 'B','N','R','2'
//   [4]     format version = 2
//   [5]     flags (reserved)
//   [6]     entry count
//   [7]     reserved
//   [8..9]  payload length LE
//   [10..11] CRC16-CCITT LE
// Blocks 9+ : payload (skipping trailer blocks i.e. block % 4 == 3)
//
// Payload record types:
//   REC_CLEAR : 0x01 | dur(2 bytes LE, units 10ms)
//   REC_TEXT  : 0x02 | font | colour | x | y | dur(2 LE) | len | text[len]

static const uint8_t  BNR_MAGIC_0    = 'B';
static const uint8_t  BNR_MAGIC_1    = 'N';
static const uint8_t  BNR_MAGIC_2    = 'R';
static const uint8_t  BNR_MAGIC_3    = '2';
static const uint8_t  BNR_FMT_VER   =   2;   // format version; old cards ('BNR1') rejected

static const uint8_t  REC_CLEAR      = 0x01;
static const uint8_t  REC_TEXT       = 0x02;

// ---------- EEPROM layout ----------
// Teensy 3.2: 2048 bytes EEPROM
// Existing usage:
//   0–19   : p_Store (PARAMSTORE)
//   20     : param-valid flag (111)
//   18     : B-screen flag
//   27     : demoMode flag (180)
// Banner block starts at 32, safe gap above existing.
//
static const int      EE_BNR_FLAG    =  32;   // 1 = valid banner stored, 0 = none
static const int      EE_BNR_RUN     =  33;   // 1 = auto-run on boot,    0 = skip
static const int      EE_BNR_LEN_L   =  34;   // payload length low byte
static const int      EE_BNR_LEN_H   =  35;   // payload length high byte
static const int      EE_BNR_CRC_L   =  36;   // CRC16 low byte
static const int      EE_BNR_CRC_H   =  37;   // CRC16 high byte
static const int      EE_BNR_PAYLOAD =  38;   // payload bytes start here

static const uint16_t BNR_MAX_PAYLOAD = 500;  // 38+500 = 538, well within 2048

// ---------- CRC16-CCITT ----------
static inline uint16_t bnr_crc16(const uint8_t* data, uint16_t len, uint16_t crc = 0xFFFF) {
  while (len--) {
    crc ^= (uint16_t)(*data++) << 8;
    for (uint8_t i = 0; i < 8; i++)
      crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
  }
  return crc;
}

// ---------- EEPROM helpers ----------
static inline uint16_t bnr_eeReadU16(int addr) {
  return (uint16_t)EEPROM.read(addr) | ((uint16_t)EEPROM.read(addr + 1) << 8);
}
static inline void bnr_eeWriteU16(int addr, uint16_t v) {
  EEPROM.update(addr,     (uint8_t)(v & 0xFF));
  EEPROM.update(addr + 1, (uint8_t)(v >> 8));
}

// ---------- Status queries (used by both tabs and f_FNCTs) ----------
static inline bool bnr_isPresent()    { return EEPROM.read(EE_BNR_FLAG) == 1; }
static inline bool bnr_runEnabled()   { return EEPROM.read(EE_BNR_RUN)  == 1; }
static inline void bnr_setRun(bool v) { EEPROM.update(EE_BNR_RUN, v ? 1 : 0); }

// ---------- Little-endian u16 read from byte buffer ----------
static inline uint16_t bnr_rdU16LE(const uint8_t* p) {
  return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}
/*
 * BannerPlay.ino
 * ==============
 * Reads the banner payload from EEPROM and plays it on the LED matrix
 * by sending PLT-2001 serial commands via HC12.
 *
 * Payload record format (written by Banner Composer / RFID Writer):
 *   REC_CLEAR (0x01) : dur_lo | dur_hi          (duration in 10ms units)
 *   REC_TEXT  (0x02) : font | colour | x | y | dur_lo | dur_hi | len | text[len]
 *
 * Public API:
 *   bool bnrPlay(bool ignoreRunFlag)
 *     Reads payload from EEPROM, verifies CRC, walks records.
 *     Returns true if played to completion, false if user aborted (BTN4)
 *     or payload invalid.
 *     If ignoreRunFlag==false, checks EE_BNR_RUN first.
 */

#include "Banner.h"
#include <EEPROM.h>

// ----------------------------------------------------------------
//  Internal helpers
// ----------------------------------------------------------------

// Returns true if BTN4 is currently pressed (exit / abort banner)
static bool bnrExitPressed() {
  return (digitalRead(button4Pin) == LOW);
}

// Waits for duration (in 10ms units), returns false if user aborts
static bool bnrWait(uint16_t dur10ms) {
  uint32_t ms    = (uint32_t)dur10ms * 10UL;
  uint32_t start = millis();
  while ((uint32_t)(millis() - start) < ms) {
    if (bnrExitPressed()) return false;
    delay(2);
  }
  return true;
}

// Show a status bar on OLED while banner runs
static void bnrShowBar() {
  disp.setCursor(0, 7);
  disp.inverse();
  disp.print("Banner   Exit[4]");
  disp.noInverse();
}

// ----------------------------------------------------------------
//  Main player
// ----------------------------------------------------------------
bool bnrPlay(bool ignoreRunFlag)
{
  if (!bnr_isPresent())               return false;
  if (!ignoreRunFlag && !bnr_runEnabled()) return false;

  // Read and validate header from EEPROM
  uint16_t payloadLen = bnr_eeReadU16(EE_BNR_LEN_L);
  uint16_t storedCrc  = bnr_eeReadU16(EE_BNR_CRC_L);

  if (payloadLen == 0 || payloadLen > BNR_MAX_PAYLOAD) return false;

  // Load payload
  static uint8_t buf[BNR_MAX_PAYLOAD];
  for (uint16_t i = 0; i < payloadLen; i++)
    buf[i] = EEPROM.read(EE_BNR_PAYLOAD + i);

  // Verify CRC
  if (bnr_crc16(buf, payloadLen) != storedCrc) return false;

  bnrShowBar();

  // Walk records on loop
  for (;;) {                          // loop until BTN4
    uint16_t i = 0;
    while (i < payloadLen) {
      uint8_t rec = buf[i++];

      if (rec == REC_CLEAR) {
        // REC_CLEAR: dur(2)
        if (i + 2 > payloadLen) return false;
        uint16_t dur = bnr_rdU16LE(&buf[i]); i += 2;

        clearMatrix(false);
        if (!bnrWait(dur)) return false;

      } else if (rec == REC_TEXT) {
        // REC_TEXT: font | colour | x | y | dur(2) | len | text[len]
        if (i + 6 > payloadLen) return false;     // minimum bytes before text
        uint8_t  font   = buf[i++];
        uint8_t  colour = buf[i++];
        uint8_t  x      = buf[i++];               // explicit X position
        uint8_t  y      = buf[i++];
        uint16_t dur    = bnr_rdU16LE(&buf[i]); i += 2;
        uint8_t  tlen   = buf[i++];

        if (i + tlen > payloadLen) return false;

        // Build null-terminated text string
        static char text[64];
        uint8_t copyLen = (tlen < sizeof(text) - 1) ? tlen : sizeof(text) - 1;
        memcpy(text, &buf[i], copyLen);
        text[copyLen] = '\0';
        i += tlen;

        // Send to PLT-2001 via HC12
        HC12.printf(F("font %u\r"), font);
        HC12.printf(F("text %u %u %u \"%s\"\rpaint\r"), colour, x, y, text);
        HC12.flush();

        if (!bnrWait(dur)) return false;

      } else {
        return false;           // unknown record type — corrupt payload
      }
    }                           // end of one pass — loop back to top
  }
  clearMatrix(false);
  return true;
}
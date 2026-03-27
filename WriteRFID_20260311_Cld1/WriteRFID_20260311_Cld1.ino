 /**
 * Arduino Uno: ! modified to Rp2040 or Teensy4 (2040 pref)
 * 
 * Writes Parameters to the RFID cards used in the TimeJudge system
 * By Fiachra Judge; Aug 2021
 * 
 * Write parameters as set by pressing            Button 1
 * A basic Menu structure is enabled by pressing  Button 2
 * Up / Down selection of parameters and values   Button 2/3                                        
 * Exit Write mode and edit Menu                  Button 4
 * 
 * 
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 *
 *
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          3             6         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      2             7         D10        10               10
 * SPI MOSI    MOSI(SDO)    11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO(SDI)    12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 

RC522 → RP2040-Zero (SPI1)
SCK → GP2 (SPI1 SCK)
MOSI → GP3 (SPI1 TX)
MISO → GP0 (SPI1 RX)
SDA/SS → GP1 (SPI1 CSn)
RST → GP15 (any free GPIO is fine; GP15 is handy)
VCC → 3V3, GND → GND

OLED_SCK = GP10
OLED_MOSI = GP11
OLED_DC = GP13
OLED_CS = GP12
OLED_RST = GP14 (or 12/any free if you prefer)

Buttons (4x)
GP26, GP27, GP28, GP29
*/

#include <MFRC522.h>
#include <EEPROM.h>
#include <U8x8lib.h>
#include "Banner.h"
#include <SPI.h>
#include "b_Buttons.h"
#include "Utils.h"

#define RC522_SCK   2
#define RC522_MOSI  3
#define RC522_MISO  0
#define RC522_SS    1
#define RC522_RST   15


#ifdef U8x8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define DEBUG

#define OLED_RES 14
 
#define OLED_CS 12
#define OLED_DC 13

// Strip path details from the FILE information, show only the file NAME
//#define __NAME__ (strrchr(__FILE__,'\\') ? strrchr(__FILE__,'\\')+1 : __FILE__)
#define __NAME__ (strrchr(__FILE__,'/') ? strrchr(__FILE__,'/')+1 : (strrchr(__FILE__,'\\') ? strrchr(__FILE__,'\\')+1 : __FILE__))

//U8X8_SH1106_128X64_NONAME_4W_HW_SPI u8x8(__CS, __DC, RESET_PIN);                              // 1.3" little

//=====================================================================================
U8X8_SH1106_128X64_NONAME_4W_SW_SPI u8x8(/*clock*/10, /*data*/11, OLED_CS, OLED_DC, OLED_RES); // 1.3" little
//======================================================================================
//U8X8_SSD1325_NHD_128X64_4W_HW_SPI u8x8(__CS, __DC, RESET_PIN);                                // 2.5" ADA
//U8X8_SSD1325_NHD_128X64_4W_SW_SPI u8x8(/* clock=*/ 13, /* data=*/ 11, __CS, __DC, RESET_PIN);
//U8X8_SSD1322_NHD_128X64_4W_HW_SPI u8x8(__CS, __DC, RESET_PIN);

#define RST_PIN 15     // Configurable, see typical pin layout above
#define SS_PIN  9     // Configurable, see typical pin layout above
//MFRC522 mfrc522(SS_PIN, RST_PIN);       // Create MFRC522 instance.
MFRC522 mfrc522(RC522_SS, RC522_RST);
MFRC522::MIFARE_Key key;

void initRC522() {
  // Force SPI0 pin mapping
  SPI.setRX(RC522_MISO);
  SPI.setCS(RC522_SS);
  SPI.setSCK(RC522_SCK);
  SPI.setTX(RC522_MOSI);
  SPI.begin();

  pinMode(RC522_SS, OUTPUT);
  digitalWrite(RC522_SS, HIGH);

  pinMode(RC522_RST, OUTPUT);
  digitalWrite(RC522_RST, HIGH);
  delay(50);

  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print("RC522 VersionReg: 0x");
  Serial.println(v, HEX);   // expect 0x91 or 0x92
}

//constexpr uint8_t LED_STATUS = 6;
//#define LED_STATUS 6
const uint16_t  tick          = 1000;

const byte      startCounts[9] = {240, 120, 80, 40, 20, 180, 90, 180, 30};
const char*     flintWalk[5]  = {"30 YD", "25 YD", "20 YD", "15 YD", ""};
const char*     flint[6]      = {"25 YD", "20 FT", "30 YD", "15 YD", "20 YD", "10 YD"};
const char*     nameParam[16] = { "Time", "Walk", "Ends", "Dets",   // 0 - 3
                                  "Prac", "Fnls", "BrkT", "Altr",  // 4 - 7
                                  "Team", "A/B?", "Flnt", "Supv",  // 8 -11
                                  "IFAA", "Bann", ""    , ""    }; // 12-15

uint8_t         dataStore[16]   = { 1, 10, 10, 2,         // |
                                    2, 0, 10, 0,          // |- default parameters; overwritten by card write
                                    0, 0, 0, 0,           // |
                                    0, 0, 0, 0  };        // spares

                                

// const struct {
//     char name[5]; // 4 characters + null terminator
//     byte values[4];
// } customerName[10] = {
//     {"DEMO", {127, 212, 42, 198}},
//     {"IFAA", {127, 212, 42, 198}},
//     {"CCAC", {155, 136, 220, 28}},
//     {"-",    {255, 255, 255, 255}},
//     {"-",    {255, 255, 255, 255}},
//     {"-",    {255, 255, 255, 255}},
//     {"-",    {255, 255, 255, 255}},
//     {"-",    {255, 255, 255, 255}},
//     {"-",    {255, 255, 255, 255}},
//     {"-",    {255, 255, 255, 255}}
// }; 

/*
Keycard validity check mask
*/
/*DEMO */
// const byte Key12 =   127;
// const byte Key13 =   212;
// const byte Key14 =   42; 
// const byte Key15 =   198;


/*IFAA*/
// const uint8_t   Key12 = 127;
// const uint8_t   Key13 = 212;
// const uint8_t   Key14 = 42;
// const uint8_t   Key15 = 198;

/* CCAC */
const uint8_t   Key12 = 155;
const uint8_t   Key13 = 136;
const uint8_t   Key14 = 220;
const uint8_t   Key15 = 28;

const char* customerName[] = {"DEMO", "NIFAA", "CCAC","test"}  ; 

//const uint8_t customerCount = 50;
const uint8_t keys = 4;
const uint8_t customerCount = sizeof(customerName) / sizeof(customerName[0]);

// const int customerCodeTable[customerCount][keys] = {
//   {100,100,100,100},
//   {110,110,110,110},
//   {120,120,120,120},
//   {0,0,0,0}  // for "test"
// };


const char*     teamParam[15] = {"-OFF-",          " T R ", "MT R ",  " T C ", "MT C ",
                                "","","","","","", " Tp R", "MTp R",  " Tp C", "MTp C" };
                                  

enum ButtonValueMask {
  BUTTON1       = 1,                                    // in binary: 00001
  BUTTON2       = 2,                                    // in binary: 00010
  BUTTON3       = 4,                                    // in binary: 00100
  BUTTON4       = 8                                     // in binary: 01000
};

PressAction a1 = ACT_NONE;
PressAction a2 = ACT_NONE;
PressAction a3 = ACT_NONE;
PressAction a4 = ACT_NONE;

//enum PendingOp { OP_NONE, OP_WRITE, OP_READ };
//static PendingOp pendingOp = OP_NONE;
//static uint32_t pendingStartMs = 0;
//static const uint16_t PENDING_TIMEOUT_MS = 5000; // 5 seconds to present card


const int button1Pin  = 29;//26;                               // the number of the pushbutton pin UNO
const int button2Pin  = 28;//27;         
const int button3Pin  = 27;//28;         
const int button4Pin  = 26;//29;

//const int paramShow   = 13;
const int paramShow   =  7;         

int currState1 = HIGH;
int prevState1 = HIGH;
   
bool firstTime = true;
bool requireRemove = false; 
bool bannerAccepted = false;
// volatile byte gBtn = 0;
// volatile Btn1Action gBtn1Action = B1_NONE;
// volatile Btn3Action gBtn3Action = B3_NONE;

// ---------------------------
// Minimal state machine for IDLE / WRITE_MODE / PARAM_MENU
// ---------------------------

//enum AppState { ST_IDLE, ST_WRITE_MODE, ST_READ_MODE, ST_PARAM_MENU, ST_BANNER_INGEST };
enum AppState { ST_IDLE, ST_WRITE_MODE, ST_READ_MODE, ST_PARAM_MENU, ST_BANNER_INGEST, ST_SUB_MENU };
static AppState gState = ST_IDLE;                               // current mode
static byte     paramIndex = 0;                                 // menu cursor / selection
static byte     subCursorIdx = 0;
static uint32_t blinkLastMs  = 0;     // millis() timestamp of last blink toggle
static bool     blinkState   = true;  // true = highlighted (inverse), false = normal

// void enterIdleScreen() {
//   #ifdef DEBUG
//   Serial.println(" IN IDLE ");
//   #endif
//   clearFromLine(1);                                             // clear lower screen area
//   showAllParams(paramShow + 1);                                 // show full parameter list in idle format
//   #ifdef DEBUG
//   printParamVals();                              // show current parameter values
//   #endif
//   showInstr();                                                  // show "BTN1 write / BTN2 menu / ..." instructions
// }
void enterIdleScreen() {
    #ifdef DEBUG
    Serial.println(" IN IDLE ");
    #endif
    wipeOLED(false);                   // clear all rows (no programmer header)
    drawParamScreen(255);              // 255 = no cursor highlight
    #ifdef DEBUG
    printParamVals();
    #endif
    showInstr();
}

void enterWriteModeScreen() {
  requireRemove = false;
  clearFromLine(1);
  writeReady();                                             // clear lower screen area
  #ifdef DEBUG
  Serial.println(" IN WRITE ");
  #endif
  requireRemove = false;                                        // allow first write immediately
}

// void dumpButtons(const char* tag) {
//   Serial.printf("%s 2=%d 3=%d 4=%d 5=%d 6=%d 7=%d\n",
//                 tag,
//                 digitalRead(2), digitalRead(3),
//                 digitalRead(4), digitalRead(5),
//                 digitalRead(6), digitalRead(7));
// }
void dumpButtons(const char* tag) {
  Serial.printf("%s  B1=%d B2=%d B3=%d B4=%d\n",
    tag,
    digitalRead(button1Pin),
    digitalRead(button2Pin),
    digitalRead(button3Pin),
    digitalRead(button4Pin));
}
bool writePresentedCardOnce();
bool ButtonShow = false;
bool cardIsGoneStable(uint8_t needed);

 //****************************************************************************
 
void setup() {
  Serial.begin(115200);
  uint32_t t0 = millis();
  while (!Serial && millis() - t0 < 1500) {
    delay(10);
  }
  delay(tick);

  delay(50);                    // let rails settle
  u8x8.begin();
  delay(100);

  u8x8.setPowerSave(0);
  // u8x8.clear();
  // u8x8.draw2x2String(0, 2, "OLED");
  // u8x8.draw2x2String(0, 4, "OK");
  // delay(2*tick);
  u8x8.setFont(u8x8_font_chroma48medium8_r);        //
  delay(100);
  // pinMode(LED_STATUS, OUTPUT);
  // digitalWrite(LED_STATUS, HIGH);
  pinMode(button1Pin, INPUT_PULLUP);                // Button1 for START / PAUSE / RESUME / menu OK
  pinMode(button2Pin, INPUT_PULLUP);                // Button2 for RESTART / MENU / UP / 
  pinMode(button3Pin, INPUT_PULLUP);                // Button3 for DOWN
  pinMode(button4Pin, INPUT_PULLUP);                // Button4 for EMERGENCY STOP / menu EXIT, no change
  dumpButtons("after pullups");
   
  if (EEPROM.read(29) == 111) {                     // if flag for parameters stored is set?
    EEPROM.get(0, dataStore);                       // Copies most recent parameters back in
  }
  dumpButtons("after EEPROM");
  pauseMe(200);
  dataStore[15] = 0;
  pinMode(SS_PIN, OUTPUT);
  digitalWrite(SS_PIN, HIGH);     // deselect

  pinMode(RST_PIN, OUTPUT);
  digitalWrite(RST_PIN, HIGH);    // keep RC522 out of reset
  delay(100);

  initRC522();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
                                                    // Prepare the key (used both as key A and as key B)
                                                    // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }
  dumpButtons("after PCD_Init");
  // TEMP

 // digitalWrite(SS_PIN, LOW);
  delay(5);
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  // digitalWrite(SS_PIN, HIGH);
  // Serial.printf("RC522 VersionReg forced CS: 0x%02X\n", v);
  Serial.print("RC522 VersionReg: 0x");
  Serial.println(v, HEX);
  #ifdef DEBUG
    Serial.println(F("Scan a MIFARE Classic PICC to demonstrate read and write."));
    Serial.print(F("Using key (for A and B):"));
  #endif
  dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
  #ifdef DEBUG
    Serial.println();
    Serial.println(F("BEWARE: Data will be written to the PICC, in sector #1"));
  #endif
  
  
}

//

// ---------------------------
// loop(): state machine version (complete)
// ---------------------------

void loop() {

  if (firstTime) {
    introScreen();
    firstTime = false;
    enterIdleScreen();
    gState = ST_IDLE;
  }
  a1 = pollButtonDebounced(button1Pin, b1, 25, 2000, 0);
  a2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0); // no 2 or 4s tier  
  a3 = pollButtonDebounced(button3Pin, b3, 25, 2000, 0);
  a4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0); // no 2 or 4s tier
  runAppStateMachine(a1,a2,a3,a4);
}


void showWriteResult(bool ok){
    wipeOLED(true);
    u8x8.draw2x2String(0, 3, ok ? "SUCCESS " : "FAILED  ");
    requireRemove = true;
    pauseMe(700);
    writeReady();                         // back to READY TO WRITE screen
    return;
}


void runAppStateMachine(PressAction a1,
                        PressAction a2,
                        PressAction a3,
                        PressAction a4) {

    switch (gState) {

        // ── IDLE ──────────────────────────────────────────
        case ST_IDLE:

            if (a1 == ACT_SHORT) {
                gState = ST_WRITE_MODE;
                enterWriteModeScreen();
                break;
            }

            if (a1 == ACT_LONG) {
                gState = ST_READ_MODE;
                showOfferCardToRead();
                break;
            }

            if (a2 == ACT_SHORT) {
                gState = ST_PARAM_MENU;
                wipeOLED(false);
                drawParamScreen(paramIndex);
                showPick();
                break;
            }

            if (a3 == ACT_LONG) {
                while (Serial.available()) Serial.read();
                bannerReset();
                Serial.println(F("Banner ingest: READY (type HELP)"));
                gState = ST_BANNER_INGEST;
                enterBannerIngestScreen();
                break;
            }
            break;

        // ── WRITE MODE ────────────────────────────────────
        case ST_WRITE_MODE:

            if (a4 == ACT_SHORT) {
                gState = ST_IDLE;
                enterIdleScreen();
                break;
            }

            if (requireRemove) {
                if (cardIsGoneStable(5)) {
                    requireRemove = false;
                    enterWriteModeScreen();
                }
                break;
            }

            if (mfrc522.PICC_IsNewCardPresent() &&
                mfrc522.PICC_ReadCardSerial()) {
                bool ok = writePresentedCardOnce();
                showWriteResult(ok);
            }
            break;

        // ── PARAM MENU ────────────────────────────────────
        case ST_PARAM_MENU: {

            if (a4 == ACT_SHORT) {
                gState = ST_IDLE;
                enterIdleScreen();
                break;
            }

            byte oldIndex = paramIndex;

            // Circular nav: BTN3 = down (next), BTN2 = up (prev)
            if (a3 == ACT_SHORT)
                paramIndex = (paramIndex + 1) % CURSOR_COUNT;
            if (a2 == ACT_SHORT)
                paramIndex = (paramIndex == 0) ? CURSOR_COUNT - 1 : paramIndex - 1;
            
                // Blink tick — only fires when cursor is sitting on the Round row (stop 0).
            // 0.75Hz = ~1333ms period, so toggle every 667ms.
            if (paramIndex == 0) {
                if ((uint32_t)(millis() - blinkLastMs) >= 667) { // cast prevents rollover glitch
                    blinkLastMs = millis();                       // reset the timer
                    blinkState  = !blinkState;                    // flip highlight on/off
                    drawRoundBanner(blinkState);                  // redraw with current blink state
                }
            }

            if (paramIndex != oldIndex) {
                // Un-highlight old field
                if (oldIndex == 0) {
                    blinkState  = true;           // reset so it starts highlighted next visit
                    blinkLastMs = 0;              // reset timer so blink kicks in immediately
                    drawRoundBanner(false);       // draw un-highlighted as cursor leaves
                }
                else if (oldIndex == 4) drawBannField(false);
                else                    drawField(oldIndex, false);

                // Highlight new field
                if (paramIndex == 0) {
                    blinkState  = true;           // start with banner lit when cursor arrives
                    blinkLastMs = millis();       // start the blink timer from now
                    drawRoundBanner(blinkState);  // draw immediately highlighted
                }
                else if (paramIndex == 4) drawBannField(true);
                else                      drawField(paramIndex, true);
            }

            if (a1 == ACT_SHORT) {
                if (paramIndex == 0) {
                    // Round → open sub-menu
                    gState = ST_SUB_MENU;
                    subCursorIdx = 0;
                    wipeOLED(false);
                    drawSubMenu(subCursorIdx);
                    showPick();

                } else if (paramIndex == 4) {
                    // Bann toggle — handled entirely inline.
                    // Never calls alterParam() — that wipes screen and sets
                    // large font (showTempVal) which then bleeds into menu redraws.
                    if (!bannerIsReady()) {
                        // No banner ingested yet — flash warning on row 5,
                        // leave everything else untouched
                        u8x8.setFont(u8x8_font_chroma48medium8_r); // ensure small font
                        u8x8.setCursor(0, 5);
                        u8x8.print("No banner yet   ");  // 16 chars fills the row cleanly
                        pauseMe(1200);                    // 1.2s visible
                        u8x8.clearLine(5);                // tidy up
                    } else {
                        // Banner present — simple toggle 0↔1
                        dataStore[13] = dataStore[13] ? 0 : 1;
                    }
                    u8x8.setFont(u8x8_font_chroma48medium8_r); // ensure font reset before redraw
                    drawBannField(true);                        // redraw Bann row with cursor highlight
                } else {
                    // Normal param — edit, then redraw whole screen cleanly
                    alterParam(cursorToParam[paramIndex]);
                    wipeOLED(false);
                    drawParamScreen(paramIndex);
                    showPick();
                }
            }
            break;
        }

        // ── SUB MENU ──────────────────────────────────────
        // ── SUB MENU ──────────────────────────────────────
        case ST_SUB_MENU: {

            if (a4 == ACT_SHORT) {                        // BTN4 = exit sub, back to param menu
                gState     = ST_PARAM_MENU;               // return to top menu
                paramIndex = 0;                           // cursor on Round row
                wipeOLED(false);
                drawParamScreen(paramIndex);
                showPick();
                break;
            }

            byte oldSub = subCursorIdx;                   // remember where cursor was

            if (a3 == ACT_SHORT)                          // BTN3 = down
                subCursorIdx = (subCursorIdx + 1) % SUB_COUNT;
            if (a2 == ACT_SHORT)                          // BTN2 = up
                subCursorIdx = (subCursorIdx == 0) ? SUB_COUNT - 1 : subCursorIdx - 1;

            if (subCursorIdx != oldSub) {                 // cursor moved — redraw changed rows only
                drawSubField(oldSub,      false);         // un-highlight old row
                drawSubField(subCursorIdx, true);         // highlight new row
            }

            if (a1 == ACT_SHORT) {                        // BTN1 = select current row

                uint8_t pIdx = subToParam[subCursorIdx];  // which dataStore[] param this row controls

                if (pIdx == 10) {
                    // ── FLINT toggle ──────────────────────────────────
                    if (dataStore[10]) {
                        // Already ON → flip off, no side-effect rollback
                        dataStore[10] = 0;
                    } else {
                        // Was OFF → turn on and apply full side effects
                        dataStore[10] = 1;         // Flint ON
                        dataStore[0]  = 7;         // Time 180
                        dataStore[2]  = 7;         // Ends
                        dataStore[3]  = 1;         // Dets
                        dataStore[4]  = 1;         // Prac
                        dataStore[5]  = 0;         // Finals off
                        dataStore[6]  = 15;        // BrkT
                        dataStore[7]  = 0;         // Alternating off
                        dataStore[8]  = 0;         // Team off
                        dataStore[9]  = 0;         // A/B off
                        dataStore[12] = 0;         // IFAA off
                    }
                    gState     = ST_PARAM_MENU;    // return to top menu immediately
                    paramIndex = 0;                // cursor stays on Round row
                    wipeOLED(false);
                    drawParamScreen(paramIndex);   // Round banner reflects new state
                    showPick();

                } else if (pIdx == 12) {
                    // ── IFAA toggle ───────────────────────────────────
                    if (dataStore[12]) {
                        // Already ON → flip off, no side-effect rollback
                        dataStore[12] = 0;
                    } else {
                        // Was OFF → turn on and apply full side effects
                        dataStore[12] = 1;         // IFAA ON
                        dataStore[0]  = 0;         // Time 240
                        dataStore[2]  = 12;        // Ends
                        dataStore[3]  = 2;         // Dets
                        dataStore[4]  = 1;         // Prac
                        dataStore[5]  = 0;         // Finals off
                        dataStore[6]  = 15;        // BrkT
                        dataStore[7]  = 0;         // Alternating off
                        dataStore[8]  = 0;         // Team off
                        dataStore[9]  = 0;         // A/B off
                        dataStore[10] = 0;         // Flint off
                    }
                    gState     = ST_PARAM_MENU;    // return to top menu immediately
                    paramIndex = 0;                // cursor stays on Round row
                    wipeOLED(false);
                    drawParamScreen(paramIndex);   // Round banner reflects new state
                    showPick();

                } else {
                    // ── Everything else (Fnls, Altr, Team, A/B) ──────
                    // Normal alterParam() confirm screen, stay in sub-menu after
                    alterParam(pIdx);
                    wipeOLED(false);
                    drawSubMenu(subCursorIdx);     // redraw sub after edit
                    showPick();
                }
            }
            break;
        }


        // ── READ MODE ─────────────────────────────────────
        case ST_READ_MODE:

            if (a4 == ACT_SHORT) {
                gState = ST_IDLE;
                enterIdleScreen();
                break;
            }

            if (requireRemove) {
                if (cardIsGoneStable(5)) {
                    requireRemove = false;
                    showOfferCardToRead();
                }
                break;
            }

            if (mfrc522.PICC_IsNewCardPresent() &&
                mfrc522.PICC_ReadCardSerial()) {

                bool ok = loadCardIntoWorkingSet();

                if (ok) {
                    showLoadedBannerStatus(dataStore[13]);
                    delay(1200);
                    gState = ST_IDLE;
                    enterIdleScreen();
                    requireRemove = true;
                    break;
                } else {
                    u8x8.draw2x2String(0, 3, "READFAIL");
                    pauseMe(900);
                    showOfferCardToRead();
                    requireRemove = true;
                }
            }
            break;

        // ── BANNER INGEST ─────────────────────────────────
        case ST_BANNER_INGEST:
            if (a4 == ACT_SHORT) {
                gState = ST_IDLE;
                enterIdleScreen();
                break;
            }
            bannerAccepted = bannerSerialIngest();
            if (bannerIsReady() && bannerAccepted) {
                bannerAccepted  = false;
                dataStore[13]   = 1;          // default to write-with-banner on ingest
                wipeOLED(true);
                u8x8.draw2x2String(2, 2, "BANNER");
                u8x8.draw2x2String(0, 4, "ACCEPTED");
                u8x8.drawString(0, 7, "EXIT       [4]");
            }
            break;

        default:
            gState = ST_IDLE;
            break;
    }
}


void showOfferCardToRead(){
  wipeOLED(true);
  u8x8.draw2x2String(0, 2, " READ A ");
  u8x8.draw2x2String(0, 5, "  CARD ");
  return;
}
        

void showLoadedBannerStatus(bool bannerYes) {
  wipeOLED(true);
  u8x8.draw2x2String(0, 3, "LOADED");
  u8x8.drawString(0, 7, bannerYes ? "BNR:Y" : "BNR:N");
  pauseMe(600);
}


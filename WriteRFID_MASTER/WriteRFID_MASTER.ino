 /**
 * Arduino Mega2650
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
 * RST/Reset   RST          9             6         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            7         D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 */

#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include <U8x8lib.h>

#ifdef U8x8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8x8_HAVE_HW_I2C
#include <Wire.h>
#endif

//#define DEBUG

#define RESET_PIN 8
 
#define __CS 10
#define __DC 9


#define RST_PIN 6     // Configurable, see typical pin layout above
#define SS_PIN  7     // Configurable, see typical pin layout above

// Strip path details from the FILE information, show only the file NAME
#define __NAME__ (strrchr(__FILE__,'\\') ? strrchr(__FILE__,'\\')+1 : __FILE__)

//U8X8_SH1106_128X64_NONAME_4W_HW_SPI u8x8(__CS, __DC, RESET_PIN);                              // 1.3" little
//=====================================================================================
U8X8_SH1106_128X64_NONAME_4W_SW_SPI u8x8(/* clock=*/ 13, /* data=*/ 11, __CS, __DC, RESET_PIN); // 1.3" little
//======================================================================================
//U8X8_SSD1325_NHD_128X64_4W_HW_SPI u8x8(__CS, __DC, RESET_PIN);                                // 2.5" ADA
//U8X8_SSD1325_NHD_128X64_4W_SW_SPI u8x8(/* clock=*/ 13, /* data=*/ 11, __CS, __DC, RESET_PIN);
//U8X8_SSD1322_NHD_128X64_4W_HW_SPI u8x8(__CS, __DC, RESET_PIN);



MFRC522 mfrc522(SS_PIN, RST_PIN);       // Create MFRC522 instance.
MFRC522::MIFARE_Key key;

const uint16_t  tick          = 1000;

const byte      startCounts[9] = {240, 120, 80, 40, 20, 180, 90, 180, 30};
const char*     flintWalk[5]  = {"30 YD", "25 YD", "20 YD", "15 YD", ""};
const char*     flint[6]      = {"25 YD", "20 FT", "30 YD", "15 YD", "20 YD", "10 YD"};


const char*     nameParam[16] = {"Time", "Walk", "Ends", "Dets", 
                                  "Prac", "Fnls", "BrkT", "Altr", 
                                  "Team", "A/B?", "Flnt", "Supv"
                                  "","","","Bann"};

uint8_t         dataStore[16]   = { 1, 10, 10, 2,         // |
                                    2, 0, 10, 0,          // |- default parameters; overwritten by card write
                                    0, 0, 0, 0,           // |
                                    0, 0, 0, 0  };        // spares

                                                                    
const byte Key1 =   127;
const byte Key2 =   212;
const byte Key3 =   42; 
const byte Key4 =   198;                                  

const char*     teamParam[15] = {"-OFF-",          " T R ", "MT R ",  " T C ", "MT C ",
                                "","","","","","", " Tp R", "MTp R",  " Tp C", "MTp C" };
                                  

enum ButtonValueMask {
  BUTTON1       = 1,                                    // in binary: 00001
  BUTTON2       = 2,                                    // in binary: 00010
  BUTTON3       = 4,                                    // in binary: 00100
  BUTTON4       = 8                                     // in binary: 01000
};

const int button1Pin = 3;                               // the number of the pushbutton pin Ard.Mega
const int button2Pin = 2;
const int button3Pin = 5;
const int button4Pin = 4;

int currState1 = HIGH;
int prevState1 = HIGH;

bool firstTime = true;

 //****************************************************************************
 
void setup() {
  Serial.begin(115200);                             // Initialize serial communications
  do {} while (!Serial);                            // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  delay(tick);
  SPI.begin();                                      // Init SPI bus
  delay(tick);
  u8x8.begin();
  delay(100);
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r);        // 
  delay(100);
  pinMode(button1Pin, INPUT_PULLUP);                // Button1 for START / PAUSE / RESUME / menu OK
  pinMode(button2Pin, INPUT_PULLUP);                // Button2 for RESTART / MENU / UP / 
  pinMode(button3Pin, INPUT_PULLUP);                // Button3 for DOWN
  pinMode(button4Pin, INPUT_PULLUP);                // Button4 for EMERGENCY STOP / menu EXIT, no change
  //for (byte i = 0; i <= 16; i++) EEPROM.put(i, 0);// clean the EEPROM  
  
  // 
  if (EEPROM.read(29) == 111) {                     // if flag for parameters stored is set?
    EEPROM.get(0, dataStore);                       // Copies most recent parameters back in
  }
  pauseMe(200);
  dataStore[15] = 0;
  mfrc522.PCD_Init(); // Init MFRC522 card
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
                                                    // Prepare the key (used both as key A and as key B)
                                                    // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }
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

//********************************************************************************

void loop() {
  
  if (firstTime) introScreen();

  byte paramIndex = 0;
  for (;;){
    byte btn = waitButton();
    if (btn == BUTTON1){
      processRFID();
      break;
    } else if (btn == BUTTON2){
      clearFromLine(1);
      byte ct;
      do {
        printDebugLine(__LINE__, __NAME__);
        paramIndex = pickParam(paramIndex);
        if (paramIndex == 12) {                                     // catch BUTTON4 exit
          break;
        } else {
          showParamVal(paramIndex);
          alterParam(paramIndex);
          clearFromLine(1);
          showAllParams(paramIndex);
          delay(50);
          printDebugLine(__LINE__, __NAME__);
          
        }
      } while (!(waitButton() == BUTTON4) || (paramIndex == 12));
     clearFromLine(1); 
     showAllParams(12);
     printParamVals();
    }
  }
}    

  

 

 

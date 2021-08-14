/**
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
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 */

#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <EEPROM.h>
//#include <GLOBAL.h>
//#include <EEPROM.h>
//#include <EEPROM.h>


#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
//#include <Wire.h>
#endif

#include <U8x8lib.h>
//#include <U8g2lib.h>
//#include <U8X8lib.h>
#define RESET_PIN 8
 
#define __CS 10
#define __DC 9


#define RST_PIN 6     // Configurable, see typical pin layout above
#define SS_PIN  7     // Configurable, see typical pin layout above


//U8X8_SH1106_128X64_NONAME_4W_HW_SPI u8x8(/* cs=*/ SS_PIN, /* dc=*/ __DC, RESET_DIO);       // 1.3" little
U8X8_SH1106_128X64_NONAME_4W_SW_SPI u8x8(/* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8); // 1.3" little
//U8X8_SH1106_128X64_NONAME_4W_HW_SPI u8x8(/* cs=*/ 3, /* dc=*/ 5, /* reset=*/ 6);       // 1.3" little
//U8X8_SSD1325_NHD_128X64_4W_HW_SPI u8x8(/* cs=*/ 53, /* dc=*/ 9, RESET_DIO);               // 2.5" ADA
//U8X8_SSD1325_NHD_128X64_4W_HW_SPI u8x8(/* cs=*/ __CS, /* dc=*/ __DC, RESET_PIN);               // 2.5" ADA
//U8X8_SSD1325_NHD_128X64_4W_HW_SPI u8x8(/* cs=*/ SS_PIN, /* dc=*/ __DC, RST_PIN);               // 2.5" ADA
//U8X8_SSD1325_NHD_128X64_4W_SW_SPI u8x8(/* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8X8_SH1106_128X64_NONAME_4W_HW_SPI u8x8(/* cs=*/ 10, /* dc=*/ 9, /* rest=*/ RESET_DIO);       // 1.3" little
//U8X8_SSD1325_NHD_128X64_4W_SW_SPI u8x8(/* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//U8X8_SSD1322_NHD_128X64_4W_HW_SPI u8x8(/* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);



MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
//MFRC522 mfrc522(SS_PIN, RESET_PIN);   // Create MFRC522 instance.


MFRC522::MIFARE_Key key;

uint16_t    tick          = 1000;
/**
 * Initialize.
 */

uint8_t     startCounts[] = {240, 120, 80, 40, 20, 180, 90, 180, 30};
const char* flintWalk[5]  = {"30 YD", "25 YD", "20 YD", "15 YD", ""};
const char* flint[6]      = {"25 YD", "20 FT", "30 YD", "15 YD", "20 YD", "10 YD"};

struct PARAMSTORE {

  uint8_t   Time              = 1;                  // (0)  Number from 0 to 4 indentifying startCounts[] used, default 120
  uint8_t   Walk              = 10;                 // (1)
  uint8_t   Ends              = 10;                 // (2)  Total number of Ends for competition
  uint8_t   Details           = 2;                  // (3)  Single (1) or Double detail (2)

  uint8_t   Prac              = 2;                  // (4)  Initially set as 2x practice ends
  uint8_t   Fnls              = 0;                  // (5)  For alternating A & B session
  uint8_t   BrkT              = 10;                 // (6)  Between sessions break times, max 240min, default 10
  uint8_t   Altr              = 0;                  // (7)  1 == Archer A/Archer B; 0 == Simultaneous

  uint8_t   team              = 0;                  // (8)  Teamplay: 20: mixed Recurve, 21 mixed Comp; 30 = Recurve, 31 Comp
  uint8_t   AorB              = 0;                  // (9)
  uint8_t   Flnt              = 1;                  //(10)  if True this is not a flint round
  uint8_t   Supv              = 0;                  //(11)  if 177 == supervisor card for global chann change

  uint8_t   Rec12 = 127;                            //(12)|
  uint8_t   Rec13 = 212;                            //(13)|___  Mask to test for valid card
  uint8_t   Rec14 =  42;                            //(14)|
  uint8_t   Rec15 = 198;                            //(15)|
  uint8_t   Rec16 = 199;                            //(16)  spare
  uint8_t   Rec17 = 200;                            //(17)  spare
};

struct    PARAMSTORE paramStore;

const char* setParam[12] = {"Time", "Walk", "Ends", "Dets", "Prac", "Fnls", "BrkT", "Altr", "Team", "A/B?", "Flnt", "Supv"};

byte dataBlock[]    = {    
//   time, walk, ends, details
    0x07, 0x0A, 0x0A, 0x01, 
//   prac, Final,Break,Alternating        
    0x00, 0x00, 0xff, 0x00, 
//   Team, AorB, flint,Super                 //  Flint=0x00        
    0x00, 0x00, 0x00, 0xff, 
                                            //    127, 212, 42, 198  : KEY
                                            //    0x7f, 0xd4, 0x2a, 0xc6 : KEY in HEX
    mfrc522.uid.uidByte[0] ^ 0x7f,          //|
    mfrc522.uid.uidByte[1] ^ 0xD4,          //|_  XOR  UID with KEY to generate GUARD bytes
    mfrc522.uid.uidByte[2] ^ 0x2A,          //|   which will be decoded in target sys
    mfrc522.uid.uidByte[3] ^ 0xC6           //|
};


enum ButtonValueMask {
  BUTTON1       = 1,                              // in binary: 00001
  BUTTON2       = 2,                              // in binary: 00010
  BUTTON3       = 4,                              // in binary: 00100
  BUTTON4       = 8                               // in binary: 01000
};

const int button1Pin = 3;                       // the number of the pushbutton pin T3.2
const int button2Pin = 2;
const int button3Pin = 5;
const int button4Pin = 4;

int currState1 = HIGH;
int prevState1 = HIGH;

long long pause = 0;
bool firstTime = true;
 
void setup() {
//  Serial.begin(115200); // Initialize serial communications with the PC
//  do {} while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
//  delay(tick);
  SPI.begin();        // Init SPI bus
  delay(tick);
  u8x8.begin();
  delay(100);
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r);      // u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  delay(100);
  pinMode(button1Pin, INPUT_PULLUP);              // Button1 for START / PAUSE / RESUME / menu OK
  pinMode(button2Pin, INPUT_PULLUP);              // Button2 for RESTART / MENU / UP / 
  pinMode(button3Pin, INPUT_PULLUP);              // Button3 for DOWN
  pinMode(button4Pin, INPUT_PULLUP);              // Button4 for EMERGENCY STOP / menu EXIT, no change
    
  


  if (EEPROM.read(29) == 111) {                   // if flag for parameters stored is set?
    EEPROM.get(0, paramStore);                    // Copies most recent parameters back in
  }
  
  mfrc522.PCD_Init(); // Init MFRC522 card
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
            // Prepare the key (used both as key A and as key B)
            // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }

//  Serial.println(F("Scan a MIFARE Classic PICC to demonstrate read and write."));
//  Serial.print(F("Using key (for A and B):"));
//  dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
//  Serial.println();
//  Serial.println(F("BEWARE: Data will be written to the PICC, in sector #1"));
}


/**
 * Main loop.
 */
void loop() {
  
  if (firstTime) introScreen();

WAITBUTTON:
  switch (waitButton()) {
    
    case BUTTON1: 
      processRFID();
      goto WAITBUTTON;
    
    case BUTTON2:
      showParams(0);
      //setParam();
      return;
      
    case BUTTON3:
      goto WAITBUTTON;


    case BUTTON4:
      goto WAITBUTTON;
    
  }
   
 
}    

  

 

 

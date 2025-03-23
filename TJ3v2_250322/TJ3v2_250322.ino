/*
    Teensy3.2, 3.6 or 4.0 setup
    Client :NIFAA / CCAC / DEMO / 
*/
#define Teensy32  
//#define Teensy36
//#define Teensy40
//#define RP2040

 #define DEMO
//#define NIFAA
//#define CCAC  // 08 06 2024

//#define DEBUG               // Kills splashscreen, whistles etc
//#define DEBUG_t             // makes seconds shorter
//#define NOWHISTLE           // comment out before runtime

/*  THE FOLLOWING IS A SPORT COUNT-DOWN TIMER, OPTIMISED
    FOR ARCHERY, BUT EASILY CONFIGURABLE FOR MANY OTHER SPORTS
    By Fiachra Judge 2022
    A program for sending data to a 64x32 LARGE LED Matrix display
    board, which requires altered colour comands,
    using a PLT2001-based driver and Teensy microcontroller with an
    OLED display on the controller.
    HC12 units are employed to send the serial data
    wirelessly to the PLT2001.
    RFID is implemented as the main method for choosing setup parameters, 
    which is backed up in a limited fashion by the OLED on-board menu.
    Channel programing is via a special engineers card
    and permits programming of the screen HC12 units on the fly
    as well as the command unit from the RFID (TimeTap) menu.
    A "count-down to start" function allows for smooth tournament running.
    Incorporates a SOFT-SWITCH On/Off based on the POLOLU 2808 with 
    power ON via discrete Power Button and OFF via long press RED button
    
  * TODO:  
  * make count-down timer independent of the RFID TimeTap functions - 
    it currently allows for them but holds up the timer - requires work
  

    OLED Pin      Arduino Pin
      GND            GND
      VDD             5v
      SCK (D0)        13
      SDA (D1)        11
      RES              8
       DC              9
       CS             10

   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5 (2)     D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

*/



#include <EEPROM.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <MFRC522.h>

#define   HC12 Serial1        // HC12 transmitter serial port
#define   __CS0 10            // SSD1325 CS   OLED
#define   __DC  9             // SSD1325 DC   OLED
#define   SS_PIN  19          // RFID  SDA/SS/SPI
#define   offControlPin 6     // used to kill power with long-press RED button via Digital Power Switch

#ifdef    Teensy32
  #define RESET_DIO 23        // (23: T3.2/T4.0, 28: T3.6) OLED
  #define HC12SetPin 15       // (15: T3.2/T4.0, 17: T3.6) This pin remains HIGH until setting the HC-12 
#elif defined Teensy36 
  #define RESET_DIO 28        // (23: T3.2/T4.0, 28: T3.6) OLED
  #define HC12SetPin 17       // (15: T3.2/T4.0, 17: T3.6) This pin remains HIGH until setting the HC-12
#elif defined Teensy40
  #define RESET_DIO 23        // (23: T3.2/T4.0, 28: T3.6) OLED
  #define HC12SetPin 15       // (15: T3.2/T4.0, 17: T3.6) This pin remains HIGH until setting the HC-12 
#else
  #error Unsupported board selection. 
#endif

#define   __NAME__ (strrchr(__FILE__,'\\') ? strrchr(__FILE__,'\\')+1 : __FILE__)
                              // setup part of DEBUG string, so TAB can be reported
#define   see(variableName) \
          Serial.print( F( #variableName" = ") ); \
          Serial.println(variableName); 

const uint16_t BAUD         = 2400;

//#ifdef U8X8_HAVE_HW_SPI
//#endif
//#ifdef U8X8_HAVE_HW_I2C
//#include <Wire.h>
//#endif
//#include "PN532.h"
//#include <PN532_I2C.h>

//PN532_I2C pn532i2c(Wire);
//PN532 nfc(pn532i2c);

//U8X8_SSD1306_128X64_NONAME_4W_HW_SPI u8x8(/* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);      // 2.5" Polish
//U8X8_SH1106_128X64_NONAME_4W_HW_SPI u8x8(/* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);       // 1.3" little
//U8X8_SSD1325_NHD_128X64_4W_HW_SPI u8x8(/* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);         // 2.5" ADA

U8X8_SSD1325_NHD_128X64_4W_HW_SPI u8x8(/* cs=*/ 10, /* dc=*/ 9, RESET_DIO);               // 2.5" ADA
U8G2_SSD1325_NHD_128X64_1_4W_SW_SPI u8g2(U8G2_R0, 13, 11,/* cs=*/ 10, /* dc=*/ 9, RESET_DIO);
//U8X8_SH1106_128X64_NONAME_4W_HW_SPI u8x8(/* cs=*/ 10, /* dc=*/ 9, /* rest=*/ RESET_DIO);       // 1.3" little

//U8X8_SSD1325_NHD_128X64_4W_SW_SPI u8x8(/* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);
//constexpr uint8_t RST_PIN = 6;                  // RFID; Configurable, see typical pin layout above (9), Uno/Mega(2)
//constexpr uint8_t SS_PIN  = 19;                 // Configurable, see typical pin layout above (10),Uno(3), Mega53

//MFRC522 mfrc522(SS_PIN, RST_PIN);               // Create MFRC522 instance.
MFRC522 mfrc522(SS_PIN, RESET_DIO);               // Create MFRC522 instance.
MFRC522::MIFARE_Key key;

//SPI.setSCK(13);



/*
   These are the main variables which will be addressable by the user
   via RFID card and, as a backup, via a button menu (OLED)
*/
const int   startCounts[] = {240, 120, 80, 40, 20, 180, 90, 180, 45};
const char* flintWalk[5]  = {"30 YD", "25 YD", "20 YD", "15 YD", ""};
const char* flint[6]      = {"25 YD", "20 FT", "30 YD", "15 YD", "20 YD", "10 YD"};

struct PARAMSTORE {

  uint8_t   startCountsIndex    =  6;             // (0)  Number from 0 to 8 indentifying startCounts[] used, default 1 (90)
  uint8_t   walkUp              = 10;             // (1)
  uint8_t   maxEnds             = 12;             // (2)  Total number of Ends for competition
  uint8_t   Details             =  2;             // (3)  Single (1) or Double detail (2)

  uint8_t   maxPrac             =  0;             // (4)  Initially set as 2x practice ends
  uint8_t   isFinals            =  0;             // (5)  For alternating A & B session
  uint8_t   breakPeriod         = 10;             // (6)  Between sessions break times, max 240min, default 10
  uint8_t   isAlternating       =  0;             // (7)  1 / 2 == Recurve / Compound A/B; 0 == Simultaneous

  uint8_t   teamPlay            =  0;             // (8)  Teams: 1: mixed Recurve, 2 mixed Comp; 3 = Recurve, 4 Comp; 11 - 14 Teamplay ditto
  uint8_t   whichArcher         =  0;             // (9)  0 = ""; 1 = "A"; 2 = "B"
  uint8_t   isFlint             =  0;             //(10)  if True this is a flint round
  uint8_t   curChan             =  1;             //(11)  channel in current use // also for Supervisor mode (naturally overwritten)


  uint8_t   ifaaIndoor          =  0;             //(12)  is this an Int. Field Archery Assoc. tournament?
  uint8_t   Banner              =  0;             //(13)  Banner loaded ?
  uint8_t   B_ScrCh             =  0;             //(14)  shows chann no. if dual screens are set up
  uint8_t   which_Scr_1st       =  0;             //(15)  false until screen-flip in progress, then 1 or 2 for A or B

  uint8_t   PS16                =  0;             //(16)|___  Spares
  uint8_t   PS17                =  0;             //(17)|
}  __attribute__ ((packed));
struct    PARAMSTORE p_Store;

/*
Keycard validity check mask
*/
#ifdef CCAC
  const uint8_t   Key12 = 155;
  const uint8_t   Key13 = 136;
  const uint8_t   Key14 = 220;
  const uint8_t   Key15 = 28;

#elif defined  NIFAA || defined DEMO
  const uint8_t   Key12 = 127;
  const uint8_t   Key13 = 212;
  const uint8_t   Key14 = 42;
  const uint8_t   Key15 = 198;
#endif



/*
   These will be set by switches or similar
*/
int bright = 255;                                 // Set brightness of the screens 0 - 255
//  (maybe altered via sensor?)

/*
   These remain unaltered by the user
*/

#ifdef DEBUG_t
constexpr uint16_t tick   = 200;                  // short "second" interval <<(nominally 1000)>>
#else
constexpr uint16_t tick   = 1000;
#endif

constexpr uint16_t tock   = 5;

                                                  // This is the main count index!!!
int         n_Count_[]    = { startCounts[p_Store.startCountsIndex],
                              startCounts[p_Store.startCountsIndex], 
                              startCounts[p_Store.startCountsIndex] };
bool        shootDetail   = 0;                    // toggled for Details AB or CD: 0=AB, 1=CD
int8_t      sE_iter       = 0;                    // Count of End iterations
int8_t      sEcount       = 1;                    // Count of  actual Ends, from 1 to maxEnds
int8_t      countPractice = 0;                    // marker to decrement for sighters test
int         barWidth      = 5;                    // *
int         rectWide      = 49;                   // *
uint8_t     lnNumber;                             // y pos formatter
uint8_t     colNumber;                            // x pos
uint8_t     txtColour;
uint8_t     t_ShootOff    = 0;
enum        Colours:  
    int     { red = 1, green = 2, orange = 3};
bool        continueOn    = false;
bool        next          = false;
bool        startOver     = false;
bool        intervalOn    = false;
bool        started       = false;                // referencing the countdown timer status
bool        reStartEnd    = false;
bool        scrWait_Enable= false;                // status of waiting graphic scroll
bool        demoMode      = false;
uint16_t    lapsed        = 0;
uint16_t    tempOffset    = 0;                    // used to hold the last format value in Clock
long long   intervalTimer = 0;
long long   secondsTimer  = 0;
byte        arrowCount    = 0;
byte        set_size      = 0;                    // The arrow set-count for Teamplay ENDS
byte        op_Chan       = 0;                    // The control channel in use currently - for Teamplay dual screens           

enum ButtonValueMask {
  BUTTON1       = 1,                              // in binary: 00001
  BUTTON2       = 2,                              // in binary: 00010
  BUTTON3       = 4,                              // in binary: 00100
  BUTTON4       = 8                               // in binary: 01000
};


const char* menu0[] =   {
                        "Duration", 
                        "Ends",
                        "Practice", 
                        "Detail",
                        "Walkup",
                        "Finals",
                        "Breaktime"
                        /* Indiv/Team play
                         * Alternating      // y/n
                         * Carry Time On    // y/n
                         */
};


#if defined Teensy32  || defined Teensy40
  const int button1Pin = 2;                       // the number of the pushbutton pin T3.2
  const int button2Pin = 3;
  const int button3Pin = 4;
  const int button4Pin = 5;
#elif defined Teensy36
  const int button1Pin = 32;                      // the number of the pushbutton pin T3.6
  const int button2Pin = 31;
  const int button3Pin = 30;
  const int button4Pin = 29;
// #elif defined Teensy40
//   const int button1Pin = 2;                       // the number of the pushbutton pin T4.0
//   const int button2Pin = 3;
//   const int button3Pin = 4;
//   const int button4Pin = 5; 
#else
  #error    Unsupported board selection.
#endif
int currState1 = HIGH;
int prevState1 = HIGH;



/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


void setup() {
  u8g2.begin();
  u8x8.begin();
  u8g2.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r);        // u8g2.setFont(u8x8_font_amstrad_cpc_extended_f);
  see(p_Store.teamPlay);
  wipeOLED();                                       // Clear the OLED, write header
  u8g2.setContrast(255);
  u8x8.draw2x2String(0, 2, " SYSTEM ");
  u8x8.draw2x2String(0, 6, "STARTING");
  see(p_Store.B_ScrCh);
  HC12.begin(BAUD);                                 // set SoftwareSerial Serial1 port: 2400
  //for (byte i = 0; i <=29; i++) EEPROM.put(i, 0); // clean the EEPROM  (dev only)
  pinMode(offControlPin,      OUTPUT);              // Output High for Power Off / Keep low for continued operation 
  pinMode(HC12SetPin,         OUTPUT);              // Output High for Transparent / Low for Command
  
  command_ON(false);                                // Enter HC12 TRANSPARENT mode
  digitalWrite(offControlPin, LOW);                 // ensure Power Off not selected  
  pauseMe(tick);
  if (EEPROM.read(18) != 1) {                       // if not second screen freq. selected
    EEPROM.update(18, 0); 
    pauseMe(120);
    goWhistle(1);
    pauseMe(10);
  }
   
  if (EEPROM.read(20) == 111) {                   // is flag for stored parameters set?
    EEPROM.get(0, p_Store);                       // Copies most recent parameters back in
    pauseMe(10);
    setControlChannel(p_Store.curChan == 0? 1 : p_Store.curChan);           // ensure saved base-channel is set on the controller
    //printDebugLine(true, __LINE__, __NAME__);    
    if (!p_Store.teamPlay) p_Store.B_ScrCh = 0;
    zeroSettings();
    //printDebugLine(false, __LINE__, __NAME__); 
  }
  pauseMe(tick);
  if (EEPROM.read(27) == 180) demoMode = true;
  pauseMe(tick);
  pinMode(button1Pin, INPUT_PULLUP);              // Button1 for START / PAUSE / RESUME / menu OK
  pinMode(button2Pin, INPUT_PULLUP);              // Button2 for RESTART / MENU / UP /
  pinMode(button3Pin, INPUT_PULLUP);              // Button3 for DOWN
  pinMode(button4Pin, INPUT_PULLUP);              // Button4 for EMERGENCY STOP / menu EXIT, no change
  see(p_Store.B_ScrCh);
  wipeOLED();
  u8x8.draw2x2String(0, 2, " SYSTEM ");
  u8x8.draw2x2String(0, 4, "CHAN:");
  readChannel();                                  // fetch the current channel and display
  //printDebugLine(false, __LINE__, __NAME__); 
  SPI.begin();                                    // Init SPI bus
  pauseMe(800);

  mfrc522.PCD_Init();                             // Init MFRC522 card
  pauseMe(50);
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  pauseMe(50);
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  pauseMe(20);
  goReboot();                                     // Reboot the screen unit
  pauseMe(2*tick);
  clearFromLine(6);
  u8x8.setCursor(0, 6);
  u8x8.inverse();
  u8x8.print("Proceed:  BTN[1]");
  u8x8.setCursor(0, 7);
  u8x8.print("Change:   BTN[3]");
  u8x8.noInverse();
  pauseMe(tick);
  unsigned long offTimer = millis();              // start time-out clock
  for (;;) {
    byte btn = readButtons();
    if (btn == BUTTON1) {
        break;                                    // move on
    } else if (btn == BUTTON3) {
        new_Channel(false);                       // change the channel (false == just locally)
        break;
    }
    if ((millis() - offTimer) > 60000UL ) {
      goPowerOff();                               // check timer - if 1 mins passes, shutoff
    }
  }
  HC12.print(F("^8H"));
  if (demoMode) goDemoLoop();
  HC12.print(F("font 13\r"));                     //  Bignum font
  HC12.flush();
  wipeOLED();
  dispSrcFileDetails(__NAME__);                   // Show *this* file name on OLED
  pauseMe(tick);
  #ifdef CCAC 
    u8x8.draw2x2String(4, 3, "CORK");
    u8x8.draw2x2String(4, 5, "CITY");
  #else
    u8x8.draw2x2String(0, 6, "..WAIT..");
  #endif
  #ifdef DEBUG 
  HC12.print("title\r"); 
  pauseMe(3000);
  #endif
  writeSplash(true);                              // full splashscreen with animation
  if (p_Store.teamPlay)                           // as multi-screen is via card ONLY these vars are reset on startup  
  p_Store.teamPlay = 0;
  p_Store.isFlint  = 0;
  //pauseMe(2 * tick);
  writeInfoBigscreen();                           // Now info regarding setup on bigscreen
  clearFromLine(0);
  displayParamsOnOLED();                          // show current (default) setting
  u8x8.draw2x2String(0, 6, "..WAIT..");
  writeSplash(false);                             // splash without animation
  shootDetail = 0;                                // bool for Detail odd/even, counters
  sE_iter = 0; 
  sEcount = 1;
  countPractice = p_Store.maxPrac; 
}

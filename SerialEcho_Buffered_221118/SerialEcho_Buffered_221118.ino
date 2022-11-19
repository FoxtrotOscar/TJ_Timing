 

/*  STM32F103C8T6 as Serial translator and HC11 programmer
 *  By F.Judge for the TimeJudge system
 *  - any character received on the HC12 port
 *  at BAUD [selected] is printed forward to  
 *  Serial2 @ 115200 for the Matrix Driver
 *  HC12      Serial1 //TX-PA9/RX-PA10
 *  MATRIXSER Serial2 //TX-PA2/RX-PA3
 *  Serial3           //TX-PB10/RX-PB11
 *  Serial            //USB (PA11/PA12) 
*/


// #include <stdio.h>
// #include <stdlib.h>
// #include <hardwareserial.h>


HardwareSerial  Serial2(PA3, PA2);
//#define         DEBUG
#define         _RADIO      Serial                // RX-PA10 / TX-PA9
#define         MATRIXSER   Serial2               // RX-PA3 (NOT IN USE) / TX-PA2/
#define         BAUD        2400
#define         cBAUD       9600
#define         setupPin    PA15


const byte    numChars      =   64;               // counter for buffer to hold incoming data packages
bool          newData       =   false;
const int     rebootPin     =   PB0;              // the reboot pulse output
const int     whistlePin    =   PB1;              // the number of the LED pin
const int     screenBright  =   255;              // (max) brightness of the matrix screen
uint8_t       numWhistles   =   0;                // how many whistles
uint8_t       channNum      =   0;
uint16_t      tick          =   1000;             // timing aids
uint8_t       tock          =   25;
uint8_t       whistleCt     =    0;               // serve the whistle count-down
String        HC12ReadBuffer=   "";               // Read/Write Buffer 1 for HC12
bool          HC12End       =   false;            // Flag to indiacte End of HC12 String
char          HC12ByteIn;                         // Temporary holding variable
bool          whistleFlag   =   false;
long long     whistleClock  =   0;                // time the whistlePin activations
byte          colNumber ; 
byte          txtColour ; 
byte          lnNumber  ;

char          receivedChars[numChars];            // an array to store the received data
enum          screenConfig:                       // configpanel width height paneltype bitsperpixel
     int    { sWidth        = 64, 
              sHeight       = 32, 
              sType         = 0, 
              sBPP          = 2,
              sInv          = 1,
              sEnabA        = 0, 
              sBright       = 255 }; 
enum        Colours:  
    int     { red = 1, green = 2, orange = 3};              


void setup() {
  _RADIO.begin    (2400);
  MATRIXSER.begin (115200, SERIAL_8N1);          // TX-PA2/RX-PA3  MCU to Matrix
  pinMode         (whistlePin,  OUTPUT);
  pinMode         (rebootPin,   OUTPUT);
  pinMode         (setupPin,    OUTPUT);
  digitalWrite    (rebootPin,   HIGH);            // PB0, active low
  digitalWrite    (whistlePin,  LOW);             // PB1: Active HIGH
  digitalWrite    (setupPin,    HIGH);            // PC11: active low - to read/control channel, high for Serial pass-through
  pauseMe(800);
  // Serial.print("AT+B2400\n");                      // making sure the Serial channel is at the correct baud rate
  // pauseMe(800);

  configMatrix(sWidth, sHeight, sType, sBPP, sInv, sEnabA, sBright);
  pauseMe(100);
  showParam();                                 // Write the parameter details to the screen
}
/*
 * ===============================================================================================
 */
void loop() {
  
  if (whistleFlag) {
    whistleFlag = checkWhistleStatus();           // Is there a Whistle command running?
  }
  if (recvWithEndMarker()) {                      // When whole command string is captured
    printNewData();
  }
    
}

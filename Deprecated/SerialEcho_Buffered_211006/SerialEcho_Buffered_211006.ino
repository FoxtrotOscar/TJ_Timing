

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

HardwareSerial Serial2(PA3, PA2);
#define       HC12      Serial1                // RX-PA10 / TX-PA9
#define       MATRIXSER Serial2                // (RX-PA3 - NOT IN USE) / TX-PA2/
#define       BAUD      9600
const byte    numChars      =   64;             // counter for buffer to hold incoming data packages

boolean       newData       =   false;
const int     rebootPin     =   PB0;            // the reboot pulse output
const int     whistlePin    =   PB1;            // the number of the LED pin
const int     whistlePin1   =   PB10;
const int     setupPin      =   PA11;
uint8_t       numWhistles   =   0;              // how many whistles
uint8_t       channNum      =   0;              // 
uint8_t       tock          =   25;
uint8_t       n;                                // serve the whistle count-down
String        HC12ReadBuffer=   "";             // Read/Write Buffer 1 for HC12
bool          HC12End       =   false;          // Flag to indiacte End of HC12 String
char          HC12ByteIn;                       // Temporary holding variable
bool          whistleFlag   =   false;
unsigned long whistleClock  =   0;              // time the whistlePin activations
byte          colNumber ; 
byte          txtColour ; 
byte          lnNumber  ;
char          receivedChars[numChars];          // an array to store the received data

void setup() {
  HC12.begin      (BAUD,        SERIAL_8N1);    // TX-PA9/RX-PA10
  MATRIXSER.begin (115200,      SERIAL_8N1);    // TX-PA2/RX-PA3
  pinMode         (whistlePin,  OUTPUT);
  pinMode         (rebootPin,   OUTPUT);
  pinMode         (setupPin,    OUTPUT);
  digitalWrite    (rebootPin,   HIGH);          // PB0
  digitalWrite    (setupPin,    LOW);           // PC11: active to read channel
  digitalWrite    (whistlePin,  LOW);           // PB1
  delay(5*tock);

  
  configMatrix();
  delay(tock);

  showChannel();                                // WRITE THE CHANNEL NO. ON THE SCREEN
  delay(tock);
}

void loop() {
  
  if (whistleFlag) {
    whistleFlag = checkWhistleStatus();         // Is there a Whistle command running?
  }
  if (recvWithEndMarker()) {                    // When whole command string is captured
    printNewData();
  }
    
}

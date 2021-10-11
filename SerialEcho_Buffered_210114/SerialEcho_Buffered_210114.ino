/*  STM32F103 Serial translator and HC11 programmer
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
#define       HC12      Serial1                 // TX-PA9/RX-PA10
#define       MATRIXSER Serial2                 // TX-PA2/(RX-PA3 - NOT IN USE)
#define       BAUD      9600
const byte    numChars      =   64;

boolean       newData       =   false;
const int     rebootPin     =   PB0;            // the reboot pulse output
const int     whistlePin    =   PB1;            // the number of the LED pin, active HIGH whistle
const int     whistlePin1    =  PB10;           // the number active LOW whistle
const int     setupPin      =   PA11;
uint8_t       numWhistles   =   0;              // how many whistles
uint8_t       channNum      =   0;              // 
const char*   i0            =   0;
const char*   i1            =   0;
const char*   i2            =   0;
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
  pinMode         (whistlePin1, OUTPUT);
  pinMode         (rebootPin,   OUTPUT);
  pinMode         (setupPin,    OUTPUT);
  digitalWrite    (rebootPin,   HIGH);          // PB0
  digitalWrite    (setupPin,    LOW);           // PC11: active to read channel
  digitalWrite    (whistlePin,  LOW);           // PB1
  digitalWrite    (whistlePin1, HIGH);          // PB10
  
  delay(5*tock);

  
  configMatrix();
  delay(tock);

  showChannel();                                // WRITE THE CHANNEL NO. ON THE SCREEN
  delay(tock);
}

void loop() {
  
  if (whistleFlag) {
    whistleFlag = checkWhistleStatus(); 
  }
  if (recvWithEndMarker()) {
    printNewData();
  }
    
}

bool recvWithEndMarker() {
  static byte ndx = 0;
  char endMarker = '\r';
  char rc;
 
  while (HC12.available() > 0 && newData == false) {
    rc = HC12.read();
    
    switch (rc) {
      case '^':
        while (!HC12.available()) {}            // hang around until a char is received
        delay(1);
        if (HC12.read() == '9') digitalWrite(rebootPin, LOW);
        
        delay(200);
        
        digitalWrite(rebootPin, HIGH);
        
        configMatrix();
        digitalWrite    (setupPin,    LOW); 
        delay(3*tock);

        showChannel();                          // WRITE THE CHANNEL NO. ON THE SCREEN
        delay(tock);
        rc = 0;
        newData = false;
      break;
      
      case '~':                                 // marker for whistle - count follows
        while (!HC12.available()) {}            // hang around until a char is received
        delay(1);
        rc = HC12.read();  

        if (   ( rc >  '0') 
            && ( rc <= '5')) {
          numWhistles = (rc - '0');             // converts symbol '0' .. '9' to number 0..9.
        }
        rc = 0;
        n = numWhistles ;
        whistleClock = millis();
        digitalWrite(whistlePin,  HIGH);         // fire the (first) whistle immediately
        digitalWrite(whistlePin1, LOW);
        whistleFlag = true;
        newData = false;
      break;

      case '*':                                 // marker for setup of the HC11 - confirm follows
        while (!HC12.available()) {}            // hang around until a char is received
        rc = HC12.read();
        delay(2);
        if (rc == '^')  {
          goSetChan();
        }
        newData = false;
      break;

      default:
        if (rc != endMarker) {
          receivedChars[ndx] = rc;
          ndx++;
          if (ndx >= numChars) {
            ndx = numChars - 1;
          }
          newData = false;
        } else {
          receivedChars[ndx] = endMarker;       // terminate the string
          ndx = 0;
          newData = true;
        }
      break;
    }
  }
  return newData;
}

/* Write the data to the LED Matrix using 115200baud */
void printNewData() {
    MATRIXSER.print(receivedChars);
    delay(1);
    memset(receivedChars, 0, sizeof(receivedChars));
    newData = false;
}

/* Non-blocking whistle control */
bool checkWhistleStatus(){                      // are we still counting down whistles?
  if ((digitalRead(whistlePin) == HIGH) &&      // Whistle blowing and not last one
      ((millis() - whistleClock)                 
      > (1000 / numWhistles)))  {               // and is the clock exhausted for this one
    digitalWrite(whistlePin,  LOW);              // Turn it off
    digitalWrite(whistlePin1, HIGH);
    whistleClock = millis();                    // reset the clock
    n -= 1;                                     // Still running? False if n == 0
    
  } else if ((digitalRead(whistlePin) == LOW)   // Whistle off and not last one                   
            && ((millis() - whistleClock)            
            > (1200 / (numWhistles + 1)))) {    // and is the clock exhausted for this one
    digitalWrite(whistlePin,  HIGH);             // start the whistle
    digitalWrite(whistlePin1, LOW);  
    whistleClock = millis();                    // reset the clock
  }
  return n;                                     // give diminishing value of 
}



void goSetChan() {
  HC12End = false;
  HC12ReadBuffer.reserve(64);                   // Reserve 64 bytes for Serial message input
  do{
    HC12ByteIn = HC12.read();
    delay(2);
  } while (HC12ByteIn !='A');
  HC12ReadBuffer += char(HC12ByteIn);
  while (HC12.available() && !HC12End) {        // While Arduino's HC12 soft serial rx buffer has data
    HC12ByteIn = HC12.read();                   // Store each character from rx buffer in byteIn
    delay(1);
    HC12ReadBuffer += char(HC12ByteIn);         // Write each character of byteIn to HC12ReadBuffer
    if (HC12ByteIn == '\n') {                   // At the end of the line
      HC12End = true;                           // Set HC12End flag to true
    }
  }
  MATRIXSER.print("font 6\r");              
  delay(2*tock);
  if (HC12End) {                                // If HC12End flag is true
    if (HC12ReadBuffer.startsWith("AT")) {      // Check to see if a command is received from remote
      clearMatrix();
      
      i0 = "GOOD COMMAND";
      sendSerialS(3, 2, 15, i0, 0, 0);
      digitalWrite(setupPin, LOW);              // Enter command mode
      delay(1000);                              // Delay before sending command
      HC12.print(HC12ReadBuffer);               // Write command to local HC12
      delay(500);                               // Wait 0.5 s for reply
      showChannel();
    } else {
      i0 = "BAD COMMAND";
      sendSerialS(1, 2, 15, i0, 0, 0);
      
    }
    
    HC12ReadBuffer = "";                        // Empty buffer
    HC12End = false;                            // Reset flag
  }
  digitalWrite(setupPin, HIGH);
  delay(3*tock);
}

void configMatrix(void){
  digitalWrite(rebootPin, LOW);
  delay(4*tock);
  digitalWrite(rebootPin, HIGH);
  delay(4*tock);
    
  MATRIXSER.print("configpanel 64 32 0 2\r");   //  setup the panel size for the PLT2001 driver
  delay(2*tock);                                //  |
  MATRIXSER.print("configpanel 64 32 0 2\r");   //  setup the panel size for the PLT2001 driver
  delay(2*tock);                                //  |
  MATRIXSER.print("invertdata 1\r");            //  Setup to invert the colours for the large screens
  delay(2*tock);                                //  |  
  MATRIXSER.print("enableactive 0\r");          //  |   
  delay(2*tock);                                //  |
  MATRIXSER.print("brightness ");               //  | 
  MATRIXSER.print(255);                         //  |
  MATRIXSER.print("\r");                        //  |
  delay(tock);                                  //  |
  MATRIXSER.print("paint\r");                   //  |
}

void clearMatrix(void){
      MATRIXSER.print("clear\r");
      MATRIXSER.print("paint\r");    
      //MATRIXSER.flush();
}

void showChannel(void){
  MATRIXSER.print("font 6\r");                       
  delay(2*tock);                                   
  clearMatrix();
  i0 = "CHANNEL: ";
  static byte ndx       = 0;
  char        endMarker = '\n';
  char        rc;
  bool flag = false;
  colNumber = 3; 
  txtColour = 2; 
  lnNumber  = 20;
  sendSerialS(txtColour, colNumber, lnNumber, i0, 0, 0);
  colNumber = 46;
  
START:
  HC12.print("AT+RC\n");
  HC12.flush();
  do {} while (!HC12.available());
  byte ptr = 0;
  while (HC12.available()) {                    
    while (flag == false){
      rc = HC12.read();
      delay(2);
      if (rc == '+') {
        flag = true;
      }
      if (rc == 'E'){
        goto START;                             //  if ERROR returned, send AT command again.
      }
    }
    delay(2);
    
    rc = HC12.read();
    delay(2);
    if ((rc != endMarker) ) {
      if ((rc >= '0') && (rc <= '9') && (ptr < 3)){
        rc = (rc - '0');
        sendNumber(rc);
        colNumber +=5;
        ptr++;
      }
    } 
  }
  rc = 0;
  flag    = false;
  digitalWrite    (setupPin,    HIGH);          // PC15: set inactive
}

void sendSerialS(uint8_t txtColour1, uint8_t colNumber1, uint8_t lnNumber1, const char* i0, const char* i1, const char* i2){
  MATRIXSER.print("text ");
  MATRIXSER.print(txtColour1);
  MATRIXSER.print(" ");
  MATRIXSER.print(colNumber1);                  // x pos for text
  MATRIXSER.print(" ");
  MATRIXSER.print(lnNumber1);                   // y pos
  MATRIXSER.print(" ");
  MATRIXSER.print('"');
  MATRIXSER.print(i0);
  MATRIXSER.print(i1);
  MATRIXSER.print(i2);
  MATRIXSER.print('"');
  MATRIXSER.print("\r");
  delay(4*tock);
  MATRIXSER.print("paint\r");
  MATRIXSER.flush();
  delay(tock); 
}

void sendNumber(int digits) {
  MATRIXSER.print("text ");
  MATRIXSER.print(txtColour);
  MATRIXSER.print(" ");
  MATRIXSER.print(colNumber);                   // x pos for text
  MATRIXSER.print(" ");
  MATRIXSER.print(lnNumber);                    // y pos
  MATRIXSER.print(" ");
  MATRIXSER.print('"');
  MATRIXSER.print(digits);
  MATRIXSER.print('"');
  MATRIXSER.print("\r");
  delay(tock);
  MATRIXSER.print("paint\r");
  MATRIXSER.flush();
  delay(tock);   
}

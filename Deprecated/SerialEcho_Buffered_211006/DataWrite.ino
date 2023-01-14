/* 
  Write the data to the LED Matrix using 115200baud 
*/
void printNewData() {
    MATRIXSER.print(receivedChars);             // send the buffered data to the Matrix
    delay(1);                                   // Now empty it
    memset(receivedChars, 0, sizeof(receivedChars));
    newData = false;
}


void clearMatrix(void){
      MATRIXSER.print("clear\r");
      MATRIXSER.print("paint\r");    
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


void sendSerialS(uint8_t txtColour1, uint8_t colNumber1, uint8_t lnNumber1, const char* i0, const char* i1) {
  return sendSerialS(txtColour1, colNumber1, lnNumber1, i0, i1, nullptr); // this one calls the big one above 
}
void sendSerialS(uint8_t txtColour1, uint8_t colNumber1, uint8_t lnNumber1, const char* i0) {
  return sendSerialS(txtColour1, colNumber1, lnNumber1, i0, nullptr, nullptr); // same 
}



void sendNumber(uint8_t txtColour1, uint8_t colNumber1, uint8_t lnNumber1, int digits) {
  MATRIXSER.print("text ");
  MATRIXSER.print(txtColour1);
  MATRIXSER.print(" ");
  MATRIXSER.print(colNumber1);                   // x pos for text
  MATRIXSER.print(" ");
  MATRIXSER.print(lnNumber1);                    // y pos
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

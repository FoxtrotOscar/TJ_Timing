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
  
  MATRIXSER.printf(
      F("text %u %u %u \"%s%s%s\"\rpaint\r"), 
      txtColour1, colNumber1, lnNumber1, 
      i0 != nullptr ? i0 : "", 
      i1 != nullptr ? i1 : "", 
      i2 != nullptr ? i2 : ""
      );  
  MATRIXSER.flush();
}

// this one calls the big one above 
void sendSerialS(uint8_t txtColour1, uint8_t colNumber1, uint8_t lnNumber1, const char* i0, const char* i1) {
  return sendSerialS(txtColour1, colNumber1, lnNumber1, i0, i1, nullptr); 
}

// same 
void sendSerialS(uint8_t txtColour1, uint8_t colNumber1, uint8_t lnNumber1, const char* i0) {
  return sendSerialS(txtColour1, colNumber1, lnNumber1, i0, nullptr, nullptr); 
}



/*
 * Function to output the actual clock output to the screen
 */
void sendNumber(int tColr, int cNum, int lnNum, int digits) {
  MATRIXSER.printf(
    F("text %u %u %u \"%u\"\rpaint\r"),
      tColr, cNum, lnNum, digits);
}

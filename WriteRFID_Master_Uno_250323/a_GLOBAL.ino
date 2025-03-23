

void introScreen(void){
    //firstTime = false;
    wipeOLED(firstTime);                                     // Clear the OLED, write header
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.setContrast(127);
    u8x8.draw2x2String(0, 2, " SYSTEM ");
    u8x8.draw2x2String(0, 6, "STARTING");
    pauseMe(tick);
    //firstTime = false;
    wipeOLED(firstTime);
    pauseMe(100);
    customerCode = setCustomer();
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    wipeOLED(firstTime);
    u8x8.draw2x2String(0, 2, "  READ ");
    u8x8.draw2x2String(0, 5, "SETTINGS");
    pauseMe(tick);
    wipeOLED(firstTime);
    //if (firstTime) customerCode = setCustomer();
    //u8x8.setFont(u8x8_font_chroma48medium8_r);
  
    u8x8.draw2x2String(0, 2, "DATA TO");
    u8x8.draw2x2String(0, 6, " WRITE:");
    pauseMe(2*tick);
    firstTime = false;
    wipeOLED(firstTime);
    showAllParams(paramShow+1);
    showInstr();
}
  

void writeReady (void){
  wipeOLED(true);
  u8x8.draw2x2String(0, 2, "READY TO");
  u8x8.draw2x2String(3, 4, "WRITE");
  EEPROM.put(0, dataStore);                        // Here the parameter data is copied into EEPROM 
  EEPROM.put(29, 111); 
  u8x8.setCursor(0, 7);
  u8x8.print("Cancel Write:[4]");
}

void pauseMe(uint16_t holdOff){
  long long pause = millis();
  do {} while (millis() - pause < holdOff);
}

void printParamVals(void){
  for (byte i = 0; i <= 15; i++) {
    Serial.print(nameParam[i]); 
    Serial.print("  \t"); 
    Serial.println(dataStore[i]); 
  }
}

void showInstr(void) {
  u8x8.setCursor(0, 7);
  u8x8.inverse();
  u8x8.print("Write[1] Menu[2]");
  u8x8.noInverse();
}

void showPick(void){
  u8x8.setCursor(0, 7);
  u8x8.inverse();
  u8x8.print("Sel:2/3  Exit[4]");
  u8x8.noInverse();  
}

/*
 * Handle Serial printing of line/tab info for debugging
 */
void printDebugLine(uint16_t lineNo, const char* FileName ){
  #ifdef DEBUG
      Serial.print(F("We are at LINE: "));
      Serial.print(lineNo);
      Serial.print(F(" in TAB: "));
      Serial.println(FileName);
  #endif      
}

uint8_t setCustomer(void){
  
  byte customerCode = 1;
  uint8_t tempVal = 1;
  showCustVal(tempVal);
  for (bool valid = false; !valid; ) {
    switch (waitButton()) {
      case BUTTON1:
        customerCode = tempVal-1;
        valid = true;
        break;  
  
      case BUTTON2: 
        tempVal == 10 ? tempVal = 1 : tempVal ++ ;
        showCustVal(tempVal); //, customerName[tempVal-1].name);
        break;
  
      case BUTTON3: 
        tempVal == 1 ? tempVal = 10 : tempVal -- ;
        showCustVal(tempVal); //, customerName[tempVal-1].name);
        break;
  
      case BUTTON4:
        valid = true;
        break;
      }
    }
    return customerCode;
}

void showCustVal (byte ct){  //, char Customer) {
  u8x8.setFont (u8x8_font_profont29_2x3_r);
    u8x8.setCursor(2, 4);
    u8x8.print("       ");
    u8x8.setCursor(2, 4);
    u8x8.print(ct);
    u8x8.print(":");
    u8x8.print(customerName[ct-1].name);
}
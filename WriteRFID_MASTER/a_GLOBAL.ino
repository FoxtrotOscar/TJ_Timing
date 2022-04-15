

void introScreen(void){
    firstTime = false;
    wipeOLED();                                     // Clear the OLED, write header
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.setContrast(127);
    u8x8.draw2x2String(0, 2, " SYSTEM ");
    u8x8.draw2x2String(0, 6, "STARTING");
    pauseMe(tick);
    firstTime = false;
    wipeOLED();
    pauseMe(100);
    u8x8.draw2x2String(0, 2, "  READ ");
    u8x8.draw2x2String(0, 5, "SETTINGS");
    pauseMe(tick);
    wipeOLED();
  
    u8x8.draw2x2String(0, 2, "DATA TO");
    u8x8.draw2x2String(0, 6, " WRITE:");
    pauseMe(2*tick);
    wipeOLED();
    showAllParams(12);
}



void wipeOLED(void){
  for( uint8_t r = 0; r < 8; r++ ){
    u8x8.clearLine(r);
  }
  u8x8.setCursor(0,0);
  u8x8.inverse();
  u8x8.print(" Time PROGRAMMER ");
  
  u8x8.noInverse();  
}  

void writeReady (void){
  wipeOLED();
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

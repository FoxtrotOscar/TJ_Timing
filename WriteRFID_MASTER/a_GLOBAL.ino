

void introScreen(void){
    firstTime = false;
    wipeOLED();                                     // Clear the OLED, write header
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.setContrast(127);
    u8x8.draw2x2String(0, 2, " SYSTEM ");
    u8x8.draw2x2String(0, 6, "STARTING");
    pause = millis();
    do {} while (millis() - pause < tick);

    firstTime = false;
    wipeOLED();
    pause = millis();
    do {} while (millis() - pause < 100);
    u8x8.draw2x2String(0, 2, "  READ ");
    u8x8.draw2x2String(0, 5, "SETTINGS");
    pause = millis();      
    do {} while (millis() - pause <  tick);
    wipeOLED();
  
    u8x8.draw2x2String(0, 2, "DATA TO");
    u8x8.draw2x2String(0, 6, " WRITE:");
    pause = millis();
    do {} while (millis() - pause < 2 * tick);
    wipeOLED();
    showParams(12);
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

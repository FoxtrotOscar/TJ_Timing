

void introScreen(void){
    //firstTime = false;
    wipeOLED(firstTime);                                     // Clear the OLED, write header
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.setContrast(127);
    u8x8.draw2x2String(0, 2, " SYSTEM ");
    u8x8.draw2x2String(0, 6, "STARTING");
    #ifdef DEBUG
    Serial.println();
    Serial.println(". . .SYSTEM STARTING");
    # endif
    pauseMe(tick);
    //firstTime = false;
    wipeOLED(firstTime);
    pauseMe(100);
    setCustomer();
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    wipeOLED(firstTime);
    u8x8.draw2x2String(0, 2, "READING");
    u8x8.draw2x2String(0, 5, "SETTINGS");
    #ifdef DEBUG
    Serial.println(". . .READ SETTINGS");
    # endif
    pauseMe(tick);
    wipeOLED(firstTime);
    //if (firstTime) customerCode = setCustomer();
    //u8x8.setFont(u8x8_font_chroma48medium8_r);
  
    u8x8.draw2x2String(0, 2, "DATA TO");
    u8x8.draw2x2String(0, 6, " WRITE:");
    #ifdef DEBUG
    Serial.println();
    Serial.println("DATA TO WRITE:");
    # endif
    pauseMe(2*tick);
    firstTime = false;
    wipeOLED(firstTime);
    // showAllParams(paramShow+1);
    // showInstr();
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
  u8x8.print("Wrt1 Mnu2  Bnr3L");        // advertise BTN3 long-press for banner ingest
  u8x8.noInverse();
}

void showPick(void){
  u8x8.setCursor(0, 7);
  u8x8.inverse();
  u8x8.print("Sel:2/3  Exit[4]");
  u8x8.noInverse();  
}

void enterBannerIngestScreen(){
  wipeOLED(true);
  u8x8.draw2x2String(2, 2, "ACCEPT");
  u8x8.draw2x2String(2, 4, "BANNER");
  u8x8.setCursor(0, 7);
  u8x8.inverse();                             // status bar style
  u8x8.print("USB Ser Exit[4]");       // clear instruction: only exit button
  u8x8.noInverse();
  //u8x8.print("Cancel Input:[4]");
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

uint8_t setCustomer(void) {
  for (int i = 0; i < 50; i++) { // prime debounce ~50ms
    pollButtonDebounced(button1Pin, b1, 25, 0, 0);
    pollButtonDebounced(button2Pin, b2, 25, 0, 0);
    pollButtonDebounced(button3Pin, b3, 25, 0, 0);
    pollButtonDebounced(button4Pin, b4, 25, 0, 0);
    delay(1);
  }

  byte customerCode = 0;     // what we return (0 = cancelled if you want)
  uint8_t tempVal = 1;   // 1..customerCount

  showCustVal(tempVal);

  for (bool valid = false; !valid; ) {

    // poll once per loop
    PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);  // select
    PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);  // up
    PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);  // down
    PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);  // cancel/exit

    #ifdef DEBUG
    static uint32_t last = 0;
    // if (millis() - last > 200) {
    //   last = millis();
    //   Serial.printf("RAW B1=%d B2=%d B3=%d B4=%d  p2=%d p3=%d\n",
    //     digitalRead(button1Pin),
    //     digitalRead(button2Pin),
    //     digitalRead(button3Pin),
    //     digitalRead(button4Pin),
    //     (int)p2, (int)p3
    //   );
    // }
    #endif

    if (p2 == ACT_SHORT) {
      tempVal++;
      if (tempVal > customerCount) tempVal = 1;
      showCustVal(tempVal);
    }

    if (p3 == ACT_SHORT) {
      tempVal = (tempVal == 1) ? customerCount : (tempVal - 1);
      showCustVal(tempVal);
    }

    if (p1 == ACT_SHORT) {          // select
      customerCode = tempVal - 1;   // 0-based
      valid = true;
    }

    if (p4 == ACT_SHORT) {          // cancel/exit
      valid = true;
    }

    delay(1);
  }

  return customerCode;
}



void showCustVal(byte ct) {
  u8x8.setFont(u8x8_font_profont29_2x3_r);
  u8x8.setCursor(2, 4);
  u8x8.print("       ");
  u8x8.setCursor(2, 4);
  u8x8.print(ct);
  u8x8.print(":");
  if (ct >= 1 && ct <= customerCount) {
    u8x8.print(customerName[ct-1]);
  }
}

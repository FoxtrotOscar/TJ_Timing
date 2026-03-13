void showOneParam(byte ct, byte index);
void wipeOLED();
static uint8_t menuTop = 0;       // first param shown
static uint8_t lastIndex = 0xFF;  // forces first draw

uint8_t pickParam(byte ct){
  a1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
  a2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0); // no 2 or 4s tier  
  a3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
  a4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0); // no 2 or 4s tier
  showAllParams(ct);
  for(;;){
    if (a1 == ACT_SHORT) {
      u8x8.setFont(u8x8_font_chroma48medium8_r);
      break;                                          // exit with val
    } else if (a2 == ACT_SHORT) {
        ct == 0 ? ct = paramShow : ct -- ;                     // subtract, unless bounds reached
        showAllParams(ct);
        showPick();
        
    } else if (a3 == ACT_SHORT) {         
        ct >= paramShow ? ct = 0 : ct ++ ;                     // add, unless bounds reached
        showAllParams(ct);
        showPick();
        
    } if (a4 == ACT_SHORT) {
      printDebugLine(__LINE__, __NAME__);
      ct = paramShow+1;
      showAllParams(ct);                                // put out of parameter bounds.

      showInstr();

      break;
    }
  }
  printDebugLine(__LINE__, __NAME__);
  return ct;
}

void showAllParams(byte index){
  byte ct = 0;
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  wipeOLED(firstTime);
  u8x8.setCursor(0, 0);
  #ifdef DEBUG
  Serial.println(" we are in the showAllParams fnct");
  #endif
  do{
    index == ct ? u8x8.inverse() : u8x8.noInverse();    // if the index == ct then hightlight
    showOneParam(ct++, index);                          // show and increment the count
    delay(15);                     
  } while (ct <= paramShow);                                    // rinse, repeat
  //showInstr();
}

void showAllParamsNoWipe(byte index){
  byte ct = 0;
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  do{
    index == ct ? u8x8.inverse() : u8x8.noInverse();
    showOneParam(ct++, index);
    delay(5);
  } while (ct <= paramShow);
  u8x8.noInverse();
}

void showOneParam(byte ct, byte index){
    //printDebugLine(__LINE__, __NAME__);
  byte vOffset = ( index == paramShow ? 0: 0);          // unused
  byte y = (ct > 6 ? ct - 7 + vOffset : ct + vOffset);  // parse the row  (5;6)
  byte x = (ct > 6 ? 9 : 0);                            // parse the column
  byte z;                                    
  u8x8.setCursor(x, y);                             
  u8x8.print(nameParam[ct]);                            // display the parameter name
  #ifdef DEBUG
  Serial.print(nameParam[ct]);
  Serial.print('\t');
  #endif
  z = ( ct == 0 ? startCounts[dataStore[ct]] :
      ( ct == 11 ? (dataStore[ct] == 177 ? 1 : 
        dataStore[ct]): dataStore[ct]));                // mask Supervisor code
  u8x8.setCursor((z < 100 ?                             // parse the x position of the number
                  (z < 10 ?  x+6 : x+5)                 // ditto
                   : x+4), y);                          // and set its xy coordinates 
  u8x8.print(z);                                        // send it
  #ifdef DEBUG
  Serial.println(z);
  #endif
}
/*teamParam[15] = {"-OFF-",           " T R ", "MT R ",  " T C ", "MT C ",
                   "","","","","","", " Tp R", "MTp R",  " Tp C", "MTp C" };
*/


void showParamVal(uint8_t ct){                           // cut one from the list and display solo
  printDebugLine(__LINE__, __NAME__);
  wipeOLED(false);
  delay(300);
  clearFromLine(1);
  u8x8.setCursor(3, 2);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.draw2x2String(2, 2, nameParam[ct]);
  u8x8.draw2x2String(paramShow, 2, ": ");
  u8x8.setFont (u8x8_font_profont29_2x3_n);
  u8x8.setCursor(6, 4);
  byte z =  ct ==  0 ? startCounts[dataStore[ct]] :   // the time value  
            ct == 11 ? dataStore[ct] == 177 ? 1 :     // masking Supervisor code
              dataStore[ct] : dataStore[ct] ;        
  ct == 8 ? u8x8.print(teamParam[dataStore[ct]]) :    // show the TEAM setting
            u8x8.print(z);                            // and print as set 
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(0, 7);
  u8x8.print("[1]Yes < > No[4]");
  return;
}



void showTempVal(uint8_t ct, uint8_t temp){
    wipeOLED(false);
    printDebugLine(__LINE__, __NAME__);
    u8x8.setFont (u8x8_font_profont29_2x3_r);
    u8x8.setCursor(4, 4);
    u8x8.print("       ");
    ct == 8 ? u8x8.setCursor(3, 4) : u8x8.setCursor(6, 4);
    ct == 8 ? u8x8.print(teamParam[temp]) : u8x8.print(ct == 11 ? (temp == 177 ? 1: temp) : temp); // mask Supervisor code
    // u8x8.setFont(u8x8_font_chroma48medium8_r);
    // u8x8.setCursor(0, 7);
    // u8x8.print("[1]Yes < > No[4]");
}  
//
void alterParam(uint8_t ct) {
  uint8_t tempVal;

  // Prime debouncers so the menu starts clean
  for (int i = 0; i < 50; i++) {
    pollButtonDebounced(button1Pin, b1, 25, 0, 0);
    pollButtonDebounced(button2Pin, b2, 25, 0, 0);
    pollButtonDebounced(button3Pin, b3, 25, 0, 0);
    pollButtonDebounced(button4Pin, b4, 25, 0, 0);
    delay(1);
  }

  switch (ct) {

    case 0: { // TIME: 0..8 options
      tempVal = dataStore[ct];
      showTempVal(startCounts[ct], startCounts[tempVal]);

      for (bool done = false; !done; ) {
        PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
        PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
        PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
        PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);

        if (p2 == ACT_SHORT) {
          tempVal = (tempVal >= 8) ? 0 : (tempVal + 1);
          showTempVal(startCounts[ct], startCounts[tempVal]);
        }
        if (p3 == ACT_SHORT) {
          tempVal = (tempVal == 0) ? 8 : (tempVal - 1);
          showTempVal(startCounts[ct], startCounts[tempVal]);
        }
        if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
        if (p4 == ACT_SHORT) { done = true; }

        delay(1);
      }
      break;
    }

    case 1: { // WALKUP: 10 / 20
      tempVal = dataStore[ct];
      showTempVal(ct, tempVal);

      for (bool done = false; !done; ) {
        PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
        PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
        PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
        PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);

        if (p2 == ACT_SHORT || p3 == ACT_SHORT) {
          tempVal = (tempVal == 10) ? 20 : 10;
          showTempVal(ct, tempVal);
        }
        if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
        if (p4 == ACT_SHORT) { done = true; }

        delay(1);
      }
      break;
    }

    case 2: { // ENDS: 1..24
      tempVal = dataStore[ct];
      showTempVal(ct, tempVal);

      for (bool done = false; !done; ) {
        PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
        PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
        PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
        PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);

        if (p2 == ACT_SHORT) {
          tempVal = (tempVal == 24) ? 1 : (tempVal + 1);
          showTempVal(ct, tempVal);
        }
        if (p3 == ACT_SHORT) {
          tempVal = (tempVal == 1) ? 24 : (tempVal - 1);
          showTempVal(ct, tempVal);
        }
        if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
        if (p4 == ACT_SHORT) { done = true; }

        delay(1);
      }
      break;
    }

    case 3: { // DETAILS: 1 / 2
      tempVal = dataStore[ct];
      showTempVal(ct, tempVal);

      for (bool done = false; !done; ) {
        PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
        PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
        PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
        PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);

        if (p2 == ACT_SHORT || p3 == ACT_SHORT) {
          tempVal = (tempVal == 2) ? 1 : 2;
          showTempVal(ct, tempVal);
        }
        if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
        if (p4 == ACT_SHORT) { done = true; }

        delay(1);
      }
      break;
    }

    case 4: { // PRACTICE: 0..3
      tempVal = dataStore[ct];
      showTempVal(ct, tempVal);

      for (bool done = false; !done; ) {
        PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
        PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
        PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
        PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);

        if (p2 == ACT_SHORT) {
          tempVal = (tempVal >= 3) ? 0 : (tempVal + 1);
          showTempVal(ct, tempVal);
        }
        if (p3 == ACT_SHORT) {
          tempVal = (tempVal == 0) ? 3 : (tempVal - 1);
          showTempVal(ct, tempVal);
        }
        if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
        if (p4 == ACT_SHORT) { done = true; }

        delay(1);
      }
      break;
    }

    case 5: { // FINALS: bool
      tempVal = dataStore[ct];
      showTempVal(ct, tempVal);

      for (bool done = false; !done; ) {
        PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
        PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
        PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
        PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);

        if (p2 == ACT_SHORT || p3 == ACT_SHORT) {
          tempVal = tempVal ? 0 : 1;
          showTempVal(ct, tempVal);
        }
        if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
        if (p4 == ACT_SHORT) { done = true; }

        delay(1);
      }
      break;
    }

    case 6: { // BREAKTIMER: your stepping logic preserved
      tempVal = dataStore[ct];
      showTempVal(ct, tempVal);

      for (bool done = false; !done; ) {
        PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
        PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
        PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
        PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);

        if (p2 == ACT_SHORT) {
          tempVal = (tempVal > 231) ? 5 : tempVal + 1;
          do { tempVal++; } while (tempVal % 10);
          showTempVal(ct, tempVal);
        }
        if (p3 == ACT_SHORT) {
          tempVal = (tempVal <= 2) ? 240 : (tempVal - 1);
          showTempVal(ct, tempVal);
        }
        if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
        if (p4 == ACT_SHORT) { done = true; }

        delay(1);
      }
      break;
    }

    case 7: { // ALTERNATING: bool
      tempVal = dataStore[ct];
      showTempVal(ct, tempVal);

      for (bool done = false; !done; ) {
        PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
        PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
        PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
        PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);

        if (p2 == ACT_SHORT || p3 == ACT_SHORT) {
          tempVal = tempVal ? 0 : 1;
          showTempVal(ct, tempVal);
        }
        if (p1 == ACT_SHORT) { dataStore[ct] = tempVal; done = true; }
        if (p4 == ACT_SHORT) { done = true; }

        delay(1);
      }
      break;
    }

    case 8: { // TEAMPLAY
      tempVal = dataStore[ct];
      showTempVal(ct, tempVal);

      for (bool done = false; !done; ) {
        PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
        PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
        PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
        PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);

        if (p2 == ACT_SHORT) {
          tempVal = (tempVal == 4) ? 11 : (tempVal == 14 ? 0 : (tempVal + 1));
          showTempVal(ct, tempVal);
        }
        if (p3 == ACT_SHORT) {
          tempVal = (tempVal == 0) ? 14 : (tempVal == 11 ? 4 : (tempVal - 1));
          showTempVal(ct, tempVal);
        }

        if (p1 == ACT_SHORT) {
          dataStore[ct] = tempVal;

          // your side-effects preserved
          switch (dataStore[ct]) {
            case 1: case 3: case 11: case 13:
              dataStore[0] = 1;
              dataStore[2] = 5;
              break;
            case 2: case 4: case 12: case 14:
              dataStore[0] = 2;
              dataStore[2] = 5;
              break;
            default:
              break;
          }

          done = true;
        }

        if (p4 == ACT_SHORT) { done = true; }

        delay(1);
      }
      break;
    }

    case 9:  // A/B? bool
    case 10: // FLINT bool (has side effects on accept)
    case 11: // SUPERVISOR (0 or 177)
    case 12: // IFAA bool (side effect)
    case 13: { // BANN bool
      tempVal = dataStore[ct];
      showTempVal(ct, tempVal);

      for (bool done = false; !done; ) {
        PressAction p1 = pollButtonDebounced(button1Pin, b1, 25, 0, 0);
        PressAction p2 = pollButtonDebounced(button2Pin, b2, 25, 0, 0);
        PressAction p3 = pollButtonDebounced(button3Pin, b3, 25, 0, 0);
        PressAction p4 = pollButtonDebounced(button4Pin, b4, 25, 0, 0);

        if (p2 == ACT_SHORT || p3 == ACT_SHORT) {
          // special-case supervisor (ct==11): 0 <-> 177
          if (ct == 11) tempVal = (tempVal == 0) ? 177 : 0;
          else          tempVal = tempVal ? 0 : 1;

          showTempVal(ct, tempVal);
        }

        if (p1 == ACT_SHORT) {
          // preserve special side-effects
          dataStore[ct] = tempVal;

          if (ct == 10 && tempVal) {  // Flint side effects
            dataStore[0] = 7;         //Time 180
            dataStore[2] = 7;         //ends
            dataStore[3] = 1;         //dets
            dataStore[4] = 1;         //Prac
            dataStore[6] = 15;        //brktime
            dataStore[12]= 0;         //IFAA no
          }
          if (ct == 12 && tempVal) {  // IFAA side effects
            dataStore[0] = 0;         //Time 240
            dataStore[2] = 12;        //ends
            dataStore[3] = 2;         //dets
            dataStore[4] = 1;         //Prac
            dataStore[6] = 15;        //brktime            
            dataStore[10]= 0;         //Flint off
          }

          done = true;
        }

        if (p4 == ACT_SHORT) { done = true; }

        delay(1);
      }
      break;
    }

    default:
      printDebugLine(__LINE__, __NAME__);
      break;
  }
}

void drawParamRow(uint8_t row, uint8_t paramIdx, bool selected) {
  // row is OLED text row (e.g. 1..6), paramIdx is 0..15
  #ifdef DEBUG
  Serial.println(" we are in the drawParamRow fnct");
  #endif

  u8x8.setCursor(0, row);
  if (selected) u8x8.inverse();
  u8x8.print(nameParam[paramIdx]);
  u8x8.noInverse();

  // print value at fixed column (example col 6)
  u8x8.setCursor(7, row);
  u8x8.print("   ");
  u8x8.setCursor(7, row);
  u8x8.print(dataStore[paramIdx]);
}


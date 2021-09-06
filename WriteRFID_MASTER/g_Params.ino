


uint8_t pickParam(byte ct){
  showParams(ct);
  for(;;){
    byte btn = readButtons();
    if (btn == BUTTON1) {
        break;                                          // exit with val
    } else if (btn == BUTTON2) {
        ct == 0 ? ct = 11 : ct -- ;                     // subtract, unless bounds reached
        showParams(ct);
        
    } else if (btn == BUTTON3) {         
        ct >= 11 ? ct = 0 : ct ++ ;                     // add, unless bounds reached
        showParams(ct);
        
    } else if (btn == BUTTON4) {
      printDebugLine(__LINE__, __NAME__);
      ct = 12;
      showParams(ct);                                   // put out of parameter bounds.
      break;
    }
  }
  printDebugLine(__LINE__, __NAME__);
  return ct;
}

void showParams(byte index){
  byte ct = 0;
  do{
    index == ct ? u8x8.inverse() : u8x8.noInverse();    // if the index == ct then hightlight
    showOneParam(ct++, index);                          // show and increment the count
    delay(15);                     
  } while (ct < 12);                                    // rinse, repeat
  if (index == 12) {
    u8x8.setCursor(0, 7);
    u8x8.inverse();
    u8x8.print("Write[1] Menu[2]");
    u8x8.noInverse();
  }
}

void showOneParam(byte ct, byte index){
  byte vOffset = ( index == 12 ? 1: 2);
  byte y = (ct > 5 ? ct - 6 + vOffset : ct + vOffset);  // parse the row
  byte x = (ct > 5 ? 9 : 0);                            // parse the column
  byte z;                                    
  u8x8.setCursor(x, y);                             
  u8x8.print(nameParam[ct]);                            // display the parameter name
  ct == 0 ? z = startCounts[dataStore[ct]] :    
               z = dataStore[ct]; 
  u8x8.setCursor((z < 100 ?                             // parse the x position of the number
                  (z < 10 ?  x+6 : x+5)                 // ditto
                   : x+4), y);                          // and set its xy coordinates 
  u8x8.print(z);                                        // send it
}



byte showParamVal(uint8_t ct){                           // cut one from the list and display solo
    wipeOLED();
    delay(300);
    clearFromLine(1);
    u8x8.setCursor(3, 2);
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.draw2x2String(2, 2, nameParam[ct]);
    u8x8.draw2x2String(12, 2, ": ");
    u8x8.setFont (u8x8_font_profont29_2x3_n);
    u8x8.setCursor(6, 4);
    ct == 0 ? u8x8.print(startCounts[dataStore[ct]])  :  u8x8.print(dataStore[ct]);
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.setCursor(0, 7);
    u8x8.print("[1]Yes < > No[4]");
    return ct;
}

void showTempVal(uint8_t ct, uint8_t temp){
    
    u8x8.setFont (u8x8_font_profont29_2x3_n);
    u8x8.setCursor(6, 4);
    u8x8.print("    ");
    u8x8.setCursor(6, 4);
    u8x8.print(temp);
    u8x8.setFont(u8x8_font_chroma48medium8_r);
    u8x8.setCursor(0, 7);
    u8x8.print("[1]Yes < > No[4]");

  
}  

void setParams(void){
  uint8_t ct = pickParam(0);
  if (ct > 11) {
    ct = 0;
    return;
  }
  showParamVal(ct);
  alterParam(ct);
}



byte alterParam(uint8_t ct){
/* nameParam[12] = {"Time" 0,8, "Walk" 1,2, "Ends" 2,byte<20 "Dets" 3,2, "Prac" 4,4, "Fnls" 5,bool, 
                    "BrkT" 6,255, "Altr" 7,bool, "Team" 8,[0, 20, 21, 30, 31] , "A/B?" 9,bool, "Flnt" 10,bool, "Supv" 11, bool}*/
  uint8_t tempVal;
  
  switch (ct) {
    
    case 0:                                       //  TIME: 8 options
      tempVal = dataStore[ct];

      for (bool valid = false; !valid; ) {
        switch (waitButton()) {
          case BUTTON1:
            dataStore[ct] = tempVal;
            valid = true;
            break; 
      
          case BUTTON2: 
            tempVal >= 8 ? tempVal = 0 :  tempVal ++ ;
            showTempVal(startCounts[ct], startCounts[tempVal]);
            break;
      
          case BUTTON3: 
            tempVal <= 0 ? tempVal = 8 :  tempVal -- ;
            showTempVal(startCounts[ct], startCounts[tempVal]);
            break;
      
          case BUTTON4:
            valid = true;
            break;
        }
      }
      break;

    case 1:                                       // WALKUP: 10 / 20
      tempVal = dataStore[ct];
      
      for (bool valid = false; !valid; ) {
        switch (waitButton()) {
          case BUTTON1:
            dataStore[ct] = tempVal;
            valid = true;
            break; 
      
          case BUTTON2: 
            tempVal == 10 ? tempVal = 20 :  tempVal = 10;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON3: 
            tempVal == 10 ? tempVal = 20 :  tempVal = 10;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON4:
            valid = true;
            break;
          }
      }
      break;
    

    case 2:                                     // ENDS: 1 to 24
      tempVal = dataStore[ct];

      for (bool valid = false; !valid; ) {
        switch (waitButton()) {
          case BUTTON1:
            dataStore[ct] = tempVal;
            valid = true;
            break;  
      
          case BUTTON2: 
            tempVal == 24 ? tempVal = 1 : tempVal ++ ;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON3: 
            tempVal == 1 ? tempVal = 24 : tempVal -- ;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON4:
            valid = true;
            break;
          }
      }
      break;


    case 3:                                       //  DETAILS: 1 / 2
      tempVal = dataStore[ct];

      for (bool valid = false; !valid; ) {
        switch (waitButton()) {
          case BUTTON1:
            dataStore[ct] = tempVal;
            valid = true;
            break;  
      
          case BUTTON2: 
            tempVal == 2 ? tempVal = 1 : tempVal = 2;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON3: 
            tempVal == 2 ? tempVal = 1 : tempVal = 2;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON4:
            valid = true;
            break;
          }
      }
      break;

    case  4:                                      //  PRACTICE: 1, 2 or 3 (time)
      tempVal = dataStore[ct];

      for (bool valid = false; !valid; ) {
        switch (waitButton()) {
          case BUTTON1:
            dataStore[ct] = tempVal;
            valid = true;
            break;  
      
          case BUTTON2: 
            tempVal >= 3 ? tempVal = 0 : tempVal ++ ;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON3: 
            tempVal == 0 ? tempVal = 3 : tempVal -- ;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON4:
            valid = true;
            break;
          }
      }
      break;
    
    case  5:                                      //  FINALS: bool
      tempVal = dataStore[ct];

      for (bool valid = false; !valid; ) {
        switch (waitButton()) {
          case BUTTON1:
            dataStore[ct] = tempVal;
            valid = true;
            break;  
      
          case BUTTON2: 
            tempVal == 0 ? tempVal = 1 : tempVal = 0;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON3: 
            tempVal == 0 ? tempVal = 1 : tempVal = 0;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON4:
            valid = true;
            break;
          }
      }
      break;
    
    case  6:                                      //  BREAKTIMER: 3 to 254
      tempVal = dataStore[ct];

      for (bool valid = false; !valid; ) {
        switch (waitButton()) {
          case BUTTON1:
            dataStore[ct] = tempVal;
            valid = true;
            break;  
      
          case BUTTON2: 
            tempVal > 231 ? tempVal = 5 : tempVal ++ ;
            do { tempVal ++; } while (tempVal % 10);
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON3: 
            tempVal <= 2 ? tempVal = 240 : tempVal -- ;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON4:
            valid = true;
            break;
          }
      }
      break;
    
    case  7:                                      //  ALTERNATING: bool
      tempVal = dataStore[ct];

      for (bool valid = false; !valid; ) {
        switch (waitButton()) {
          case BUTTON1:
            dataStore[ct] = tempVal;
            valid = true;
            break;  
      
          case BUTTON2: 
            tempVal == 0 ? tempVal = 1: tempVal = 0;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON3: 
            tempVal == 0 ? tempVal = 1 : tempVal = 0;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON4:
            valid = true;
            break;
          }
      }
      break;
      
    case  8:                                      //  TEAMPLAY: 4
      tempVal = dataStore[ct];
  
      for (bool valid = false; !valid; ) {
        switch (waitButton()) {
          case BUTTON1:
            dataStore[ct] = tempVal;
            valid = true;
            break;  
      
          case BUTTON2: 
            tempVal == 4 ? tempVal = 0 : tempVal ++ ;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON3: 
            tempVal == 0 ? tempVal = 4 : tempVal -- ;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON4:
            valid = true;
            break;
          }
      }
      break;;
      
    case  9:                                      //  A or B: bool
      tempVal = dataStore[ct];
  
      for (bool valid = false; !valid; ) {
        switch (waitButton()) {
          case BUTTON1:
            dataStore[ct] = tempVal;
            valid = true;
            break;  
      
          case BUTTON2: 
            tempVal == 0 ? tempVal = 1 : tempVal = 0;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON3: 
            tempVal == 0 ? tempVal = 1 : tempVal = 0;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON4:
            valid = true;
            break;
          }
      }
      break;
    
    case 10:                                      //  FLINT: bool
      tempVal = dataStore[ct];
  
      for (bool valid = false; !valid; ) {
        switch (waitButton()) {
          case BUTTON1:
            dataStore[ct] = tempVal;
            valid = true;
            break;  
      
          case BUTTON2: 
            tempVal == 0 ? tempVal = 1 : tempVal = 0;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON3: 
            tempVal == 0 ? tempVal = 1 : tempVal = 0;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON4:
            valid = true;
            break;
          }
      }
      break; 
      
    case 11:                                      //  SUPERVISOR MODE:
      tempVal = dataStore[ct];

      for (bool valid = false; !valid; ) {
        switch (waitButton()) {
          case BUTTON1:
            dataStore[ct] = tempVal;
            valid = true;
            break;  
      
          case BUTTON2: 
            tempVal == 0 ? tempVal = 1 : tempVal = 0;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON3: 
            tempVal == 0 ? tempVal = 1 : tempVal = 0;
            showTempVal(ct, tempVal);
            break;
      
          case BUTTON4:
            valid = true;
            break;
        }
      }

    default :
      printDebugLine(__LINE__, __NAME__);
      break;
  }
  printDebugLine(__LINE__, __NAME__);
  return ct;
}

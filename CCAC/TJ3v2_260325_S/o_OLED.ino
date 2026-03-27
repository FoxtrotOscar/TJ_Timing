/*
  uint8_t startCountsIndex = 1;         // (1)Number from 0 to 4 indentifying which of startCounts is used in this round, default 120 
  uint8_t walkUp = 10;                  // (2)
  uint8_t maxEnds = 4;                  // (3)Total number of Ends for competition
  uint8_t Details = 2;                  // (4)Single (1) or Double detail (2)
  uint8_t maxPrac = 2;                  // (5)Initially set as 2x practice ends
  uint8_t isFinals = 0;                 // (6)For alternating A & B session
  uint8_t isAlternating = 0;            // (8)1 == Archer A, Archer B; 0 == Simultaneous
  uint8_t teamPlay = 0;                 // (9)
  uint8_t whichArcher = 1;              //(10)

FUNCTIONS:
27  displayParamsOnOLED
56  writeStopwatch
72  writeOLED_Data
118 wipeOLED
133 clearFromLine
143 doButtonMenu
161 writeMenuCommands
*/



/* 
 *  Function to list the current setup on the OLED
 */
void displayParamsOnOLED() {
  //wipeOLED();
  clearFromLine(1);               
  disp.setCursor(0, 1);                        
  disp.print("Dur.:  ");
  disp.setCursor(0, 2);
  disp.print("Ends: ");
  disp.setCursor(0, 3);
  disp.print("Prac: ");
  disp.setCursor(0, 4);
  disp.print("Det.:  ");                                                  // detailing parameters set
  disp.setCursor(6, 1);                         
  disp.print(startCounts[p_Store.startCountsIndex]);  
  disp.setCursor(6, 2);
  disp.print(p_Store.maxEnds);
  disp.setCursor(9, 2);
  disp.print(": ");
  disp.print(countPractice ? 0 : (sEcount > p_Store.maxEnds ? 0 : sEcount ));  // if still in practice show end zero
  disp.setCursor(6, 3);
  disp.print(flintRunning ? 0 : (p_Store.maxPrac !=0 && p_Store.isFlint) ? 1 : p_Store.maxPrac);   // if flint is already Running, then no practice, else only one.
  disp.setCursor(6, 4);
  if      (p_Store.isFlint)    disp.print("Flint");
  else    disp.print(p_Store.Details);
  disp.setCursor(12, 4);
  if (p_Store.isFlint)    disp.print(flintRunning ? " #2" : " #1");
}


/*
 * Stopwatch digit function on OLED
 * Writes the count down large on rhs
 */

void writeStopwatch(int n ){  
  disp.drawString(10, 1, "      ");                               // Blanking the stopwatch field
  disp.setCursor(11,1);
  disp.inverse();
  disp.setFont(u8x8_font_pressstart2p_n);
  disp.print(" ");
  disp.print(n < 0 ? 0 : n );                                     // Running Time
  disp.print(" ");
  disp.noInverse();
  disp.setFont(u8x8_font_chroma48medium8_r);
  
}

/*
 * Writes the current live data to OLED when not in Stopwatch
 */
void writeOLED_Data(uint8_t archerIndex, byte nID){
  //clearFromLine(1);
  disp.setFont(u8x8_font_chroma48medium8_r);
  if ((p_Store.isFinals && !countPractice && p_Store.isAlternating) || p_Store.teamPlay > 10){
  //clearFromLine(1);
  disp.setCursor(1,2);//0,4
  disp.print(p_Store.teamPlay > 10 ? " Team:" : "Archer:");
  disp.draw2x2String(3,3,archerIndex == 1 ? "A": "B");
  }
  //if (p_Store.B_ScrCh) wipeOLED(); 
  disp.setCursor(11,1);
  disp.print("     ");
  disp.setCursor(11,1);
  disp.print(n_Count_[nID] < 0 ? 0 : n_Count_[nID]);                // Running Time
  disp.setCursor(11,2);
  if (!countPractice){   
    disp.print(sEcount) ;                                           // Running End Count
    disp.print("/");
    disp.print(p_Store.maxEnds - sEcount < 0 ? 0 : p_Store.maxEnds - sEcount) ;  
  }
  disp.setCursor(11,3);
  countPractice ? disp.print(countPractice -1) : disp.print("  ");  // Running Practice-round number 
  if (p_Store.Details == 2){                                        // Double detail?
    disp.setCursor(11,4);
    if (!p_Store.isFinals && !p_Store.ifaaIndoor){
      switch (sE_iter%4){
        case 0:
        case 1:
          disp.print("AB");
          break;
        case 2:
        case 3:
          disp.print("CD");
          break;
        default:
          disp.print("");
          break;
      }
    } if (p_Store.ifaaIndoor){
      disp.setCursor(8,4);
      disp.print("IFAA");
      disp.setCursor(13,4);
      disp.print(sEcount <= 6 ?   (sE_iter%2 == 1 ? "AB" : "CD"):     // if 1st first half of IFAA round
                                  (sE_iter%2 == 0 ? "BA" : "DC"));    // 2nd half of IFAA round, reverses
    }
  }
}



void wipeOLED(void){
  clearFromLine(0);
  setHeader();
   
}


/*
 * Clear the OLED from lineNo to line 8
 */
void clearFromLine(uint8_t lineNo){
  for( uint8_t r = lineNo; r < 8; r++ ) {
    disp.clearLine(r);
    pauseMe(20);
  }
}

/*
 * handle the in-menu button information layout
 */
void doButtonMenu(void) {
  disp.setFont(u8x8_font_amstrad_cpc_extended_f);
  disp.setCursor(2, 7);
  disp.write(157);                                  // (a square) for "stop" or "cancel"
  disp.write(32); disp.write(32);
  disp.write(171);                                  // << "back" or "decrement"
  disp.write(32); disp.write(32); disp.write(32);
  disp.write(187);                                  // >> "forward" or "increment"
  disp.write(32); disp.write(32);
  disp.inverse();
  disp.print("ok");                                 //  > "accept change"
  disp.noInverse();
  disp.setFont(u8x8_font_chroma48medium8_r);
}

/*
 * Writes the three top-menu command options on lower OLED lines
 */
void writeMenuCommands(void){
  clearFromLine(5);                                 // clear the way
  disp.setCursor(0, 5);
  disp.inverse(); 
  disp.print((intervalOn ? "COUNTDOWN is ON-" : "Continue: BTN[1]"));
  disp.noInverse();
  disp.setCursor(0, 6);
  disp.print((intervalOn ? "-to QUIT: BTN[4]" : "Change:   BTN[2]"));
  disp.setCursor(0, 7);
  disp.inverse();
  disp.print("or TIME-TAP now ");
  disp.noInverse();
}

void setHeader(void){
  disp.setCursor(0,0);
  disp.inverse();
  if (!p_Store.teamPlay || !op_Chan) disp.print(" Time Controller"); // op_Chan refers to the operational channel in 2ch team play
  else {  
    char buf[4];
    disp.print("TimeControl Ch"); 
    sprintf (buf, "%2d" , op_Chan);
    disp.drawString (14, 0, buf);
  }
  disp.noInverse(); 
}

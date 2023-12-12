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
  u8x8.setCursor(0, 1);                        
  u8x8.print("Dur.:  ");
  u8x8.setCursor(0, 2);
  u8x8.print("Ends: ");
  u8x8.setCursor(0, 3);
  u8x8.print("Prac: ");
  u8x8.setCursor(0, 4);
  u8x8.print("Det.:  ");                                                  // detailing parameters set
  u8x8.setCursor(6, 1);                         
  u8x8.print(startCounts[p_Store.startCountsIndex]);
  u8x8.setCursor(6, 2);
  u8x8.print(p_Store.maxEnds);
  u8x8.setCursor(9, 2);
  u8x8.print(": ");
  u8x8.print(countPractice ? 0 : (sEcount > p_Store.maxEnds ? 0 : sEcount ));  // if still in practice show end zero
  u8x8.setCursor(6, 3);
  u8x8.print(p_Store.maxPrac);
  u8x8.setCursor(6, 4);
  u8x8.print(p_Store.Details);
}


/*
 * Stopwatch digit function on OLED
 * Writes the count down large on rhs
 */

void writeStopwatch(int n ){  
  u8x8.drawString(10, 1, "      ");                               // Blanking the stopwatch field
  u8x8.setCursor(11,1);
  u8x8.inverse();
  u8x8.setFont(u8x8_font_pressstart2p_n);
  u8x8.print(" ");
  u8x8.print(n < 0 ? 0 : n );                                     // Running Time
  u8x8.print(" ");
  u8x8.noInverse();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
}

/*
 * Writes the current live data to OLED when not in Stopwatch
 */
void writeOLED_Data(uint8_t archerIndex, byte nID){
  //clearFromLine(1);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  if ((p_Store.isFinals && !countPractice && p_Store.isAlternating) || p_Store.teamPlay > 10){
  //clearFromLine(1);
  u8x8.setCursor(1,2);//0,4
  u8x8.print(p_Store.teamPlay > 10 ? " Team:" : "Archer:");
  u8x8.draw2x2String(3,3,archerIndex == 1 ? "A": "B");
  }
  //if (p_Store.B_ScrCh) wipeOLED(); 
  u8x8.setCursor(11,1);
  u8x8.print("     ");
  u8x8.setCursor(11,1);
  u8x8.print(n_Count_[nID] < 0 ? 0 : n_Count_[nID]);                // Running Time
  u8x8.setCursor(11,2);
  if (!countPractice){   
    u8x8.print(sEcount) ;                                           // Running End Count
    u8x8.print("/");
    u8x8.print(p_Store.maxEnds - sEcount < 0 ? 0 : p_Store.maxEnds - sEcount) ;  
  }
  u8x8.setCursor(11,3);
  countPractice ? u8x8.print(countPractice -1) : u8x8.print("  ");  // Running Practice-round number 
  if (p_Store.Details == 2){                                        // Double detail?
    u8x8.setCursor(11,4);
    if (!p_Store.isFinals  ){
      switch (sE_iter%4){
        case 0:
        case 1:
          u8x8.print("AB");
          break;
        case 2:
        case 3:
          u8x8.print("CD");
          break;
        default:
          u8x8.print("");
          break;
      }
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
    u8x8.clearLine(r);
    pauseMe(20);
  }
}

/*
 * handle the in-menu button information layout
 */
void doButtonMenu(void) {
  u8x8.setFont(u8x8_font_amstrad_cpc_extended_f);
  u8x8.setCursor(2, 7);
  u8x8.write(157);                                  // (a square) for "stop" or "cancel"
  u8x8.write(32); u8x8.write(32);
  u8x8.write(171);                                  // << "back" or "decrement"
  u8x8.write(32); u8x8.write(32); u8x8.write(32);
  u8x8.write(187);                                  // >> "forward" or "increment"
  u8x8.write(32); u8x8.write(32);
  u8x8.inverse();
  u8x8.print("ok");                                 //  > "accept change"
  u8x8.noInverse();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
}

/*
 * Writes the three top-menu command options on lower OLED lines
 */
void writeMenuCommands(void){
  clearFromLine(5);                                 // clear the way
  u8x8.setCursor(0, 5);
  u8x8.inverse(); 
  u8x8.print((intervalOn ? "COUNTDOWN is ON-" : "Continue: BTN[1]"));
  u8x8.noInverse();
  u8x8.setCursor(0, 6);
  u8x8.print((intervalOn ? "-to QUIT: BTN[4]" : "Change:   BTN[2]"));
  u8x8.setCursor(0, 7);
  u8x8.inverse();
  u8x8.print("or TIME-TAP now ");
  u8x8.noInverse();
}

void setHeader(void){
  u8x8.setCursor(0,0);
  u8x8.inverse();
  if (!p_Store.teamPlay || !op_Chan) u8x8.print(" Time Controller"); 
  else {  
    char buf[4];
    u8x8.print("TimeControl Ch"); 
    sprintf (buf, "%2d" , op_Chan);
    u8x8.drawString (14, 0, buf);
  }
  u8x8.noInverse(); 
}

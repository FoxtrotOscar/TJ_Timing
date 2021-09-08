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
*/

/* 
 *  Function to list the current setup on the OLED
 */
void displayParamsOnOLED() {
  wipeOLED();               
  u8x8.setCursor(0, 1);                        
  u8x8.print("Dur.:  ");
  u8x8.setCursor(0, 2);
  u8x8.print("Ends: ");
  u8x8.setCursor(0, 3);
  u8x8.print("Prac: ");
  u8x8.setCursor(0, 4);
  u8x8.print("Det.:  ");                    // detailing parameters set
  u8x8.setCursor(6, 1);                         
  u8x8.print(startCounts[paramStore.startCountsIndex]);
  u8x8.setCursor(6, 2);
  u8x8.print(paramStore.maxEnds);
  u8x8.setCursor(9, 2);
  u8x8.print(": ");
  u8x8.print(countPractice ? 0 : sEcount);  // if still in practice show ens zero
  u8x8.setCursor(6, 3);
  u8x8.print(paramStore.maxPrac);
  u8x8.setCursor(6, 4);
  u8x8.print(paramStore.Details);
}


/*
 * Stopwatch digit function on OLED
 * Writes the count down large on rhs
 */

void writeStopwatch(uint16_t n ){
  u8x8.drawString(10, 1, "      ");                       // Blanking
  u8x8.setCursor(11,1);
  u8x8.inverse();
  u8x8.setFont(u8x8_font_pressstart2p_n);
  u8x8.print(" ");
  u8x8.print(n );                                          // Running Time
  u8x8.print(" ");
  u8x8.noInverse();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
}

/*
 * Writes the current live data to OLED when not in Stopwatch
 */
void writeOLED_Data(uint8_t archerIndex){
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(11,1);
  u8x8.print("     ");
  u8x8.setCursor(11,1);
  u8x8.print(n_Count );                                 // Running Time
  u8x8.setCursor(11,2);   
  u8x8.print(sEcount);                                  // Running End Count
  u8x8.setCursor(11,3);
  u8x8.print(countPractice);                            // Running Practice round number 
  if (paramStore.Details == 2){                         // Double detail?
    u8x8.setCursor(11,4);
    if (!paramStore.isFinals  ){
      switch (sE_iter%4){
        case 0:
          u8x8.print("AB");                             // Running AB/CD condition
          break;
        case 1:
          u8x8.print("AB");
            break;
          case 2:
            u8x8.print("CD");
            break;
          case 3:
            u8x8.print("CD");
            break;
          default:
            u8x8.print("");
            break;
      }
    }
  }
  if (paramStore.isFinals && !countPractice && paramStore.isAlternating){
    u8x8.setCursor(0,4);
    u8x8.print("Archer:    ");
    u8x8.print(archerIndex == 1 ? "A": "B");              // Print the archer letter
  }
  u8x8.print(" ");         
}



void wipeOLED(void){
  for( uint8_t r = 0; r < 8; r++ ){
    u8x8.clearLine(r);
  }
  u8x8.setCursor(0,0);
  u8x8.inverse();
  u8x8.print(" Time Controller ");
  u8x8.noInverse();  
}


/*
 * Clear the OLED from lineNo to line 8
 */
void clearFromLine(uint8_t lineNo){
  for( uint8_t r = lineNo; r < 8; r++ ) {
    u8x8.clearLine(r);
    delay(20);
  }
}

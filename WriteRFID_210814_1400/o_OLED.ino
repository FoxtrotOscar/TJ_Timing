//#include <U8x8lib.h>



/*
    OLED Pin      Arduino Pin
      GND            GND
      VDD             5v
      SCK (D0)        13
      SDA (D1)        11
      RES              8
       DC              9
       CS             10
*/
       





//void writeOLED(byte ln, str( msg
//
///*
// * Clear the OLED from lineNo to line 8
// */
//void clearFromLine(uint8_t lineNo){
//  for( uint8_t r = lineNo; r < 8; r++ ) {
//    u8x8.clearLine(r);
//    delay(20);
//  }
//}
//
//
//
///* 
// *  Function to list the current setup on the OLED
// */
//void displayParamsOnOLED() {
//  wipeOLED();               
//  u8x8.setCursor(0, 1);                        
//  u8x8.print("Dur.:  ");
//  u8x8.setCursor(0, 2);
//  u8x8.print("Ends: ");
//  u8x8.setCursor(0, 3);
//  u8x8.print("Prac: ");
//  u8x8.setCursor(0, 4);
//  u8x8.print("Det.:  ");          // detailing parameters set
//  u8x8.setCursor(6, 1);                         
//  u8x8.print(startCounts[paramStore.startCountsIndex]);
//  u8x8.setCursor(6, 2);
//  u8x8.print(paramStore.maxEnds);
//  u8x8.setCursor(6, 3);
//  u8x8.print(paramStore.maxPrac);
//  u8x8.setCursor(6, 4);
//  u8x8.print(paramStore.Details);
//}
//
///*
// * Writes the current live data to OLED when not in Stopwatch
// */
//void writeOLED_Data(uint8_t archerIndex){
//  u8x8.setFont(u8x8_font_chroma48medium8_r);
//  u8x8.setCursor(11,1);
//  u8x8.print("     ");
//  u8x8.setCursor(11,1);
//  u8x8.print(n);                    // Running Time
//  u8x8.setCursor(11,2);   
//  u8x8.print(sEcount);              // Running End Count
//  u8x8.setCursor(11,3);
//  u8x8.print(countPractice);        // Running Practice round number 
//  if (paramStore.Details == 2){     // Double detail?
//    u8x8.setCursor(11,4);
//    if (!paramStore.isFinals  ){
//      switch (sE_iter%4){
//        case 0:
//          u8x8.print("AB");    // Running AB/CD condition
//          break;
//        case 1:
//          u8x8.print("AB");
//            break;
//          case 2:
//            u8x8.print("CD");
//            break;
//          case 3:
//            u8x8.print("CD");
//            break;
//          default:
//            u8x8.print("");
//            break;
//      }
//    }
//  }
//  if (paramStore.isFinals && !countPractice && paramStore.isAlternating){
//    u8x8.setCursor(0,4);
//    u8x8.print("Archer:    ");
//    u8x8.print(archerIndex == 1 ? "A": "B");  // Print the archer letter
//  }
//  u8x8.print(" ");         
//}

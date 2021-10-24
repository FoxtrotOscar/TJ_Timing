
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
       
// * Clear the OLED from lineNo to line 8
// */
void clearFromLine(uint8_t lineNo){
  for( uint8_t r = lineNo; r < 8; r++ ) {
    u8x8.clearLine(r);
    delay(20);
  }
}

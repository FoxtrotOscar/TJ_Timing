// void clearMatrix(bool scrollOn){
//   if (scrollOn) {
//     HC12.print(F("scrollloop 0\r"));                                      //  if scroll in progress, kill it
//     pauseMe(4*tock);
//   }
//   HC12.print(F("clear\r"));
//   pauseMe(10);
//   HC12.print(F("paint\r"));
//   HC12.flush();
//   pauseMe(110);
// }

/* 
 *  Splashscreen of Logo "Time Judge"
*/
// void writeSplash(bool scrolling){ 
//   clearMatrix(false);
//   #ifdef DEBUG
//     HC12.print(F("font 8\r"));    HC12.flush();
//     sendSerialS( green, /*column=*/ 2, /*line=*/ 24, "[ SPLASH ]");
//   return;
//   #endif
//  // HC12.print("^1");                                                         // to localise to Matrix send ^0 (false) or ^1 (true)
// // pauseMe(scrolling ? 5*tick : 3*tick);
                                         

  // HC12.print(F("font 16\r"));    HC12.flush();
  // for (uint16_t br = 0; br <= bright; br += 17){                           // fade increment [10]
  //   //unsigned long timer = millis();
  //   HC12.printf(
  //         F("brightness %u\rpaint\r"),                                     //  | Fade in "Time"
  //           br);
  //   //HC12.print(F("font 16\r"));    HC12.flush();
  //   sendSerialS( green, /*column=*/ 2, /*line=*/ 24, "Time");
  //   //pauseMe(2*tock);
  //   //do{} while (millis() - timer < 2UL);                                  // allow serial traffic time (15UL)
  //   pauseMe(2);
  //   }
  // const char scrollChar[] = "JUDGE";
  // HC12.print(F("font 9\r"));    
  // pauseMe(2);
  // if (scrolling){
  //   pauseMe(2);
  //   sendScrollW(  /*speed=*/  5,                                           // 3 - 15
  //                 /*loop=*/   0,
  //                 /*wiggle=*/ 3,
  //                 /*colour=*/ orange,
  //                 /*column=*/ 4, 
  //                 /*line=*/   30,
  //                 /*window=*/ 63, 
  //                           scrollChar );                                   // animate of JUDGE
  // } else {
  //   for (uint8_t iter = 0; iter < strlen(scrollChar); iter ++) {
  //     sendChar(orange, (4 + 12*iter), 30, scrollChar[iter]);
  //     pauseMe(9);
  //   }
  // }
  // pauseMe(tock);
//}

// void score_Collect(bool redScreen){
//   // HC12.print(redScreen ? "^3" : "^2");
//   // HC12.flush();

//   if (redScreen) {
//     writeRectangle( 1, 0, 31, 64, 32);                                  // fill screen with red
//     pauseMe(2*tick);
//   }
//   HC12.print(F("font 9\r"));   
//   sendSerialS(2, 10, 15, "SCORE +");
//   HC12.flush();
//   pauseMe(30);
//   sendSerialS(2, 5, 29, "COLLECT");
//   HC12.flush();
// }
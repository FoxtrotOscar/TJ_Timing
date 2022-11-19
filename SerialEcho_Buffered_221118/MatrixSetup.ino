/*
 * Configure the MATRIX (here 64x32, byt other config possible
 * like 128 x 64 etc
 */


void configMatrix(byte pWidth, byte pHeight, byte pType, byte pBPP, byte pInv, byte pEnabA, byte pBright){
  reboot();
  MATRIXSER.printf(
    F("configpanel %u %u %u %u\r"),
        pWidth, pHeight, pType, pBPP);
  pauseMe(200);

  // MATRIXSER.printf(
  //   F("configpanel %u %u %u %u\rpaint\r"),
  //       pWidth, pHeight, pType, pBPP);
  // pauseMe(200);
  
  MATRIXSER.printf(
    F("invertdata %u\r enableactive %u\r brightness %u\rpaint\r"),
        pInv, pEnabA, pBright);
  delay(2*tock);      
}


void reboot(void){
  digitalWrite(rebootPin, LOW);                 //  Send a pulse to the Matrix Controller to 
  delay(4*tock);                                //  reboot it - clearing any spurious commands
  digitalWrite(rebootPin, HIGH);
  delay(4*tock);
}

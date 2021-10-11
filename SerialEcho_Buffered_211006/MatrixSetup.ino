/*
 * Configure the MATRIX (here 64x32, byt other config possible
 * like 128 x 64 etc
 */



void configMatrix(void){
  digitalWrite(rebootPin, LOW);                 //  Send a pulse to the Matrix Controller to 
  delay(4*tock);                                //  reboot it - clearing any spurious commands
  digitalWrite(rebootPin, HIGH);
  delay(4*tock);
    
  MATRIXSER.print("configpanel 64 32 0 2\r");   //  setup the panel size for the PLT2001 driver
  delay(2*tock);                                //  |
  MATRIXSER.print("configpanel 64 32 0 2\r");   //  setup the panel size for the PLT2001 driver
  delay(2*tock);                                //  |
  MATRIXSER.print("invertdata 1\r");            //  Setup to invert the colours for the large screens
  delay(2*tock);                                //  |  
  MATRIXSER.print("enableactive 0\r");          //  |   
  delay(2*tock);                                //  |
  MATRIXSER.print("brightness ");               //  | 
  MATRIXSER.print(255);                         //  |
  MATRIXSER.print("\r");                        //  |
  delay(tock);                                  //  |
  MATRIXSER.print("paint\r");                   //  Write the above to the controller
}

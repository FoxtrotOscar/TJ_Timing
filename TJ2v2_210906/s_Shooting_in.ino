/*
 * Countdown to start of play
 */


void checkIntervalTimer(void){

  if (intervalOn){
    if (!started ){
      lapsed = paramStore.breakPeriod *60;
      started = true;
      showWaiting(false);                         // for event where restarting timer during waiting
      goCountdownTimer();
      intervalTimer = millis();
      secondsTimer  = millis(); 
      n_Count = 60;
    pauseMe(109);
    }
    if (lapsed < 60) {                            // when count reaches 1 min blank and post end data
      intervalOn = false;                         // one min = 60000 milliseconds
      lapsed = paramStore.breakPeriod;
      writeInfoBigscreen();
      pauseMe(2*tick);
      clearMatrix();  
      HC12.print(F("font 9\r"));    HC12.flush();
      for (byte i = 0; i < 4; i++){               // flash the prepare message
        sendSerialS( /*colour=*/ 2, /*column=*/ 0, /*line=*/ 18, "PREPARE...");
        pauseMe(2*tick);
        clearMatrix();
        pauseMe(tick);
      }
      showWaiting(true);
      return;
    }
    if ((millis() - intervalTimer) > tick) {      
      intervalTimer = millis();
      secondsTimer  = millis();
      if (lapsed >=60) lapsed = goCountdownTimer();
    }
  }
}

/* 
 *
 * Run an on-screen count-down time to start of play
 * Will not show on OLED - only big screens.
*/

uint16_t goCountdownTimer(){
  if ((lapsed == paramStore.breakPeriod *60) || (lapsed %60 == 0)) { 
    clearMatrix();
    
    HC12.print(F("font 1\r"));     HC12.flush();
    sendSerialS(green, /*column=*/ 2, /*line=*/ 5, "Shooting in:");
  
    HC12.print(F("font 9\r"));    HC12.flush();
    --lapsed;
    sendSerialS((lapsed <= 120 ? 3 : 2), 13, 25,
                  TimeToString(lapsed));                          // choose digit colour and decrement
  }
    
  HC12.print(F("font 5\r"));     HC12.flush();
  sendSerialS(3, 42, 26, SecToString(lapsed));                    // choose digit colour(R1G2O3) and decrement
  lapsed --;
  
  return lapsed;
}

                                     

char* TimeToString(unsigned long t) {                           // t is time in seconds = millis()/1000;
 static char str[9];
 long h = t / 3600;
 t = t % 3600;
 int m = t / 60;
 sprintf(str, "%01ld:%02d", h, m);
 return str;
}

char* SecToString(unsigned long t) {
 static char str[5];
 t = t % 3600;
 int s = t % 60;
 sprintf(str, ":%02d",s);
 return str;
}



void showWaiting(bool enAble){
  if (enAble){
    clearMatrix();
    sendScroll();
    u8x8.setCursor(0, 5); 
    u8x8.print      ("STANDING BY:-   ");
    u8x8.setCursor(0, 6);
    u8x8.print      ("PROCEED:  BTN[1]");
    u8x8.setCursor(0, 7);
    u8x8.inverse();
    u8x8.print("or TIME-TAP now ");
    u8x8.noInverse();

  } else {
    HC12.print("scrollloop 0\r") ;
    pauseMe(2*tick);                              // allow the scroll to finish - it is not immediate
    clearMatrix();                           
  }
}

//void sendScroll(byte sSpeed, byte sLoop, byte sCol, byte sX, byte sY, byte sW, char tXt ){
void sendScroll(void){
    HC12.print("scrollspeed 20\r");
  pauseMe(tock);
    HC12.print("scrollloop 1\r");
  pauseMe(tock);
    HC12.print("scrollwiggle 0\r");
  pauseMe(tock);
    HC12.print("scroll 2 0 35 64 ");
    HC12.print('"');
    HC12.print(char(45));
    HC12.print('"');
    HC12.print("\r");
  pauseMe(tock);
}

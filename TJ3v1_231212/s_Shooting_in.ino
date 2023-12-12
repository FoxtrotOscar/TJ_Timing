/*
 * Countdown to start of play
 * 
 * Functs:
 * checkIntervalTimer
 * goCountdownTimer
 * TimeToString
 * SecToString
 * showWaiting
 */


void checkIntervalTimer(void){
  if (intervalOn){
    if (!started ){
      lapsed = p_Store.breakPeriod *60;           // convert mins to secs; one min = 60000 milliseconds
      started = true;                             // flag to show countdown clock running
      showWaiting(false);                         // for event where restarting timer during waiting
      goCountdownTimer();
      intervalTimer = secondsTimer  = millis();
      pauseMe(109);                               // fucked if I know. . .
    }
    if (lapsed < 60) {                            // when count reaches 1 min blank and post End data
      intervalOn = false;                         // kill flag to show countdown clock running        
      //lapsed = p_Store.breakPeriod;             <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
      pauseMe(tick);                              // allow the time to show for remainder of sec
      writeInfoBigscreen();                       // show what's coming
      pauseMe(2*tick);
      clearMatrix(false);  
      HC12.print(F("font 9\r"));    HC12.flush();
      goWhistle(4);                               // alert all
      for (byte i = 0; i < 4; i++){               // flash the prepare message four times
        sendSerialS( green, /*column=*/ 0, /*line=*/ 18, "PREPARE...");
        pauseMe(2*tick);
        clearMatrix(false);
        pauseMe(tick);
      }
      showWaiting(true);                          // enable the "waiting" _underline_ scroll
      return;
    }
    if ((millis() - intervalTimer) > tick) {      
      intervalTimer = secondsTimer  = millis();
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
  if ((lapsed == p_Store.breakPeriod *60) || (lapsed %60 == 0)) { // if time to change a minute field
    clearMatrix(false);
    HC12.print(F("font 1\r"));     HC12.flush();
    sendSerialS(green, /*column=*/ 2, /*line=*/ 5, "Starting in:");
    HC12.print(F("font 9\r"));    HC12.flush();
    --lapsed;
    sendSerialS((lapsed <= 120 ? 3 : 2), 13, 25,                // choose digit colour
                  TimeToString(lapsed));                          // and decrement          
  }
  HC12.print(F("font 5\r"));     HC12.flush();                    // set font for small secs
  sendSerialS(3, 42, 26, SecToString(lapsed));                    // choose digit colour(R1G2O3) and decrement
  return --lapsed;
}

char* TimeToString(unsigned long t) {                           // t is time in seconds = millis()/1000;
 static char str[11];
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

bool showWaiting(bool enAble){
  if (enAble){                                    // if True
    clearMatrix(false);
    scrWait_Enable = true;
    sendScrollChar(20, 1, 0, 2, 0, 35, 64, 45) ;  // scroll a '=' to indicate waiting
    u8x8.setCursor(0, 5); 
    u8x8.print      ("STANDING BY:-   ");
    u8x8.setCursor(0, 6);
    u8x8.print      ("PROCEED:  BTN[1]");
    u8x8.setCursor(0, 7);
    u8x8.inverse();
    u8x8.print("or TIME-TAP now ");
    u8x8.noInverse();

  } else {                                        // Otherwise
    if (scrWait_Enable){
      HC12.print("scrollloop 0\r") ;
      pauseMe(2*tick);                             // allow the scroll to finish - it is not immediate
      scrWait_Enable = false;
      clearMatrix(true);
    }                           
  }
  return scrWait_Enable;
}

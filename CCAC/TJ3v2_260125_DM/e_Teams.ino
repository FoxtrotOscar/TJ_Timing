/*
 * DUAL SCREEN TEAM FINALS: 
 * 5 ends (R), 6 arrows per end, three per set
 * 120s (atm) for the six arrows per team.
 * When a three arrow set is done, the clock for that team holds, 
 * Whistle blows and the clock for team (2) starts.
 * Then show "Scoring" before a button push for READY  
 * A further button-push takes you to restart
 * 
 14   goTeamPlay   
 179  writeSet_sizeType
*/

void goTeamPlay(byte teamType){                                                     // teamType is p_Store.teamPlay val 0 - 4  Zero is Off
  
  /*
   * Choose A or B and write max time to 2nd in red  and READY to first team up
   * Set frequency of controller to suit and when ready do a countdown bar to first team
   * Write max time to first and commence countdown
   * 
   * run first team and flip freq. to complete set.
   * On flip: Turn count to red and hold - switch freq to alternate and turn to GRN; commence count
   * 
   * Rinse and repeat according to val of <set_size> 
   * At end of play, choose shootoff or end
   * 
   */
 
  uint8_t offSet        = 13;
  uint8_t flipFlag      = 0;
  uint8_t shootOff_ct   = 0;
  uint8_t nID           = 0;
  bool    shootOff      = false;
  uint8_t firstToShoot  = 0;
  uint8_t whistleCount  = 0;
  sEcount               = 0;
  startOver             = false;
  continueOn            = true;
  

  /*
  * check for button1 press to start
  */
  while (continueOn && !startOver ) {
    while (sEcount < p_Store.maxEnds || shootOff) {
      n_Count_[1] = n_Count_[2] = (shootOff ? 20*set_size  :  startCounts[p_Store.startCountsIndex]);
      if (!sEcount) clearAB(nID, false);                                            // clear both screens
      nID = !shootOff ? goChooseArcher() : firstToShoot ;                           // The team that shot first in the match shall start shooting
      if (sEcount) clearAB(nID, false); 
      nID == 1 ? set_A(1) : set_B(2) ;                                              // set nID to match first team up, and send info to screens
      pauseMe(20);
      if (!sEcount) firstToShoot = nID;                                             // save the first team up, for tie-breaks  
      sEcount ++;
      if (shootOff)  shootOff_ct ++;
      
      writeOLED_Data(nID, nID);
      HC12.print(F("font 9\r"));
      sendSerialS(green, 0, 15, "   READY");
      sendSerialS(green,  0,  29, shootOff ? "TIE BREAK" : "   SET:");
      if (!shootOff) sendNumber (2, 44 , 29, sEcount );
      continueOn = false;
      goMenu(true); 
                                                                      // wait for proceed
      clearMatrix(false);
      clearFromLine(1);
      printDebugLine(false, __LINE__, __NAME__);
      writeOLED_Data(nID, nID );
      printDebugLine(false, __LINE__, __NAME__);
      doBarCount(nID, nID); 
      unsigned long secCount = millis();
      
      do {} while ((millis() - secCount) % tick > 2);                               // initialise timer to a start-point
      
      clearMatrix(false);
      
      HC12.print(F("font 13\r"));
      //clearFromLine(1);
      disp.setCursor(0, 6);
      disp.inverse();
      disp.print("Flip Detail: [1]");
      disp.noInverse();
      disp.setCursor(0, 7);
      disp.print(" !!!STOP!!!: [4]");
    
      /*  Now that barCount has been completed and first whistle sounded we 
      *  start the first set (three arrows each), [with NO whistle in between each arrow]
      *  except where time is over-run for an archer - then whistle x1.  
      *  As usual, whistle x3 at the end of each set. 
      *  Judge progresses the play with the green button, and if he does not and zero reached then there is a whistle. 
      *  We decide the first archer at the head of each three arrows, and put up a 
      *  signal for "Scoring", followed by "Ready" on green button
      *  Green Button re-commences.
      */
      
      writeOLED_Data(nID, nID );                                                    // set up for 5 ends of 4 or 6 arrows in 
      uint8_t tempArrowCount[] = {0, arrowCount, arrowCount};
      if (shootOff) tempArrowCount[1] = tempArrowCount[2] = set_size;
      while (n_Count_[nID] > 0){                                                    // repeat until n_Count[current] < 0
        if (!goEmergencyButton(nID, nID)) {                                         // are we halting???
          writeStopwatch(n_Count_[nID]);                                            // run a set
          HC12.print(F("font 13\r"));
          goClock(offSet, nID);                                                     // Handles formatting of the display
          sendNumber((n_Count_[nID] > 0 ? txtColour : 1), colNumber, lnNumber, n_Count_[nID] );
          n_Count_[nID] > 0 ? n_Count_[nID]-- : 
                              nID = (nID == 1? set_B(1) : set_A(2));                // catch run-out
          if(flipFlag){                                                             // if alternative screens and detail change is invoked
            if (whistleCount++ <= set_size) goWhistle(1);                           // skip 1 x whistle when about to hit 3 x whistle.
            shootOff ?
                tempArrowCount[nID] -- :                                            // either (as shootoff) reduce arrowcount in flip-flop 
                tempArrowCount[nID] -= set_size;                                    // or reduce by the qty of arrows per set of 2 or 3  
            tempArrowCount[nID] == 0 ? n_Count_[nID] = 0 : n_Count_[nID] += 1;      // Unless at zero, keep the screen count right for holding
            nID = (nID == 1? set_B(1) : set_A(2));                                  // Hold the curr count in red and change screen
            pauseMe(50);
            //wipeOLED(); /////////////////////////////////////////////////////
            setHeader();                                                            // write TimeController or Ch no.                                                                                                                        
            writeOLED_Data(nID, nID );
          }
          flipFlag = handleCount(secCount, nID);                                    // if flipFlag is false then the count didn't run out
        } else if (reStartEnd || startOver) {  //<<<<<<<<<<<<<<<<<<<  check for either restart or startover                             
          

          for (;;) {
            byte btn = readButtons();
            if (btn == BUTTON1)  break;                                             // move on
          }
        }
      }
      goWhistle(3);
      whistleCount = 0;                                                             // reset to restart
      clearAB(nID, true);                                                           // 3x whistle, clear matrix with Score and Collect message
      shootOff = false;
    }                                                                               

    continueOn = false;
    clearFromLine(1);
     
     
    if (checkForShootoff()) {                                                       // select whether a shoot-off is needed
      shootOff = true;
      continueOn = true;
    }else{
      clearFromLine(1);
      sEcount = 0;
      clearAB(nID, false);
      writeSplash(true);
            
      return;
    }
    writeShootOff(nID, true);
    
    clearFromLine(6);
    disp.setCursor(0, 6);
    disp.inverse();
    disp.print("Proceed:  BTN[1]");
    disp.noInverse();
    for (;;) {
      bool flag = false;
      switch (readButtons()) {
        case BUTTON1: 
          flag = true;
          break;
        
        case BUTTON4:
          continueOn = false;
          sEcount = p_Store.maxEnds +1; 
          writeHalt();
          flag = false;
          
          return;
      }
      if (flag) break;
    }
    clearMatrix(false);
  }
}



byte writeSet_sizeType(byte teamType) {                                             // called from r_RFID

  clearFromLine(1);
  clearMatrix(false);
  HC12.print(F("font 8\r"));  HC12.flush();
  switch (teamType) {                                                               // Team Final 1 = Recurve;  2= mixed Recurve;  3 Comp;   4 Mixed Comp                                               
                                                                                    // TeamPlay  11 = Recurve; 12= mixed Recurve; 13 Comp;  14 Mixed Comp
    case 1:     // Recurve Team
      disp.draw2x2String(4, 2, "TEAM");                                             // 120 SEC/END, set_size: 2x 3 ARROW SETS, 3-5 ENDS / 6 ARROWS
      disp.draw2x2String(1, 4, "RECURVE");
      clearMatrix(false);
      sendSerialS(green, 18, 12, "TEAM");
      sendSerialS(green, 8, 24, "RECURVE");
      p_Store.startCountsIndex = 5;
      arrowCount = 6;
      set_size = 3;
      t_ShootOff = 40;
      break;
      
    case 2:     // Recurve Team MIXED
      disp.draw2x2String(0, 2, "TEAM-MIX");                                         // 80 SEC/END, set_size: 2x 2 ARROW SETS, 3-5 ENDS / 4 ARROWS
      disp.draw2x2String(1, 4, "RECURVE");
      sendSerialS(orange, 15, 11, "MIXED");
      sendSerialS(green, 18, 22, "TEAM");
      sendSerialS(green, 8, 33, "RECURVE");
      p_Store.startCountsIndex = 1;
      arrowCount  = 4;
      set_size    = 2;
      t_ShootOff  = 40;
      break;

    case 3:     // Compound Team
      disp.draw2x2String(4, 2, "TEAM");                                             // 120 SEC/END OF 6, set_size: 2x 3 ARROW SETS, 5 ENDS / 6 ARROWS
      disp.draw2x2String(0, 4, "COMPOUND");
      sendSerialS(green, 18, 12, "TEAM");
      sendSerialS(green, 2, 24, "COMPOUND");
      p_Store.startCountsIndex = 5;
      arrowCount  = 6;
      set_size    = 3;
      t_ShootOff  = 40;
      break;
      
    case 4:     // Compound Team MIXED
      disp.draw2x2String(0, 2, "TEAM-MIX");                                         // 80 SEC/END, set_size: 2x 2 ARROW SETS, 5 ENDS / 4 ARROWS
      disp.draw2x2String(0, 4, "COMPOUND");
      sendSerialS(orange, 15, 11, "MIXED");
      sendSerialS(green, 18, 22, "TEAM");
      sendSerialS(green, 4, 33, "COMPOUND");      
      p_Store.startCountsIndex = 1;
      arrowCount  = 4;
      set_size    = 2;
      t_ShootOff  = 40;
      break;


    case 11:    // Recurve TeamPlay
      disp.draw2x2String(0, 2, "TEAMPLAY");                                         // 120 SEC/END OF 6, set_size: 2x 3 ARROW SETS, 3-5 ENDS / 6 ARROWS
      disp.draw2x2String(1, 4, "RECURVE");
      sendSerialS(green, 4, 15, "TEAMPLAY");
      sendSerialS(green, 8, 27, "RECURVE");
      p_Store.startCountsIndex = 1;
      arrowCount  = 6;
      set_size    = 3;
      t_ShootOff  = 20;
      break;
          
    case 12:    // Recurve TeamPlay MIXED
      disp.draw2x2String(1, 2, "T/P-MIX");                                          // 80 SEC/END of 4, set_size: 2x 2 ARROW SETS, 3-5 ENDS / 4 ARROWS
      disp.draw2x2String(1, 4, "RECURVE");
      sendSerialS(orange, 15, 11, "MIXED");
      sendSerialS(green, 4, 22, "TEAMPLAY");
      sendSerialS(green, 8, 33, "RECURVE");      
      p_Store.startCountsIndex = 2;
      arrowCount  = 4;
      set_size    = 2;
      t_ShootOff  = 20;
      break;

    case 13:    // Compound TeamPlay
      disp.draw2x2String(0, 2, "TEAMPLAY");                                         // 120 SEC/END OF 6, set_size: 2x 3 ARROW SETS, 5 ENDS / 6 ARROWS
      disp.draw2x2String(0, 4, "COMPOUND");
      sendSerialS(green, 4, 15, "TEAMPLAY");
      sendSerialS(green, 5, 27, "COMPOUND");      
      p_Store.startCountsIndex = 1;
      arrowCount = 6;
      set_size = 3;
      t_ShootOff = 20;
      break;
      
    case 14:    // Compound TeamPlay MIXED
      disp.draw2x2String(1, 2, "T/P-MIX");                                          // 80 SEC/END of 4, set_size: 2x 2 ARROW SETS, 5 ENDS / 4 ARROWS
      disp.draw2x2String(0, 4, "COMPOUND");
      sendSerialS(orange, 15, 11, "MIXED");
      sendSerialS(green, 4, 22, "TEAMPLAY");
      sendSerialS(green, 5, 33, "COMPOUND");       
      p_Store.startCountsIndex = 2;
      arrowCount = 4;
      set_size = 2;
      t_ShootOff = 20; 
      break;

    default:
      disp.draw2x2String(0, 2, "UNKNOWN");
      disp.draw2x2String(0, 4, "SETTING");
      pauseMe(3000);
      p_Store.teamPlay = 0;                                                         // as unknown state exists, reset it to 0 
      clearFromLine(1);
      break;
  }
  
  clearFromLine(6);
  disp.setCursor(0, 6);
  disp.inverse();
  disp.print("Proceed:  BTN[1]");
  disp.setCursor(0, 7);
  //set_size ?  disp.print("T.Play:NO BTN[4]") : disp.print("                ");
  set_size ?  disp.print("One Chann BTN[4]") : disp.print("                ");
  disp.noInverse();
  unsigned long offTimer = millis();                                                // set start of Timeout period
  for (;;) {
    byte btn = readButtons();
    if (btn == BUTTON1) {
        break;                                                                      // move on with play
    } else if (btn == BUTTON4) {
      clearFromLine(1);
      set_size = 0;
      p_Store.teamPlay = 0;
      break;                                                                        // exit
    }
    if ((millis() - offTimer) > 60000UL*89 ) {                                      // check timeout - if 1.5hrs passes with no actions, 
      set_size = 0;
      p_Store.teamPlay = 0;
      goPowerOff();                                                                 // reset & start 1 min countdown to shutoff
    }
  }
  return set_size;
}


/*  WA RULES:
    In the Team Event (both teams shooting at the same time):
    The left/right position for matches shall follow the match play chart. 
    The team on the top line of each pair (of the chart) shall shoot on the left-hand side of that match. 
    The target allocation for each round of the competition is the choice of the organisers;
    Both teams shall start each end of their match with the athletes behind the 1m line. 
    The first athlete may only cross the 1m line when the Director of Shooting has given the signal to start the match;
    The athletes in the team shall shoot two arrows each in an order of their own choosing;
    One athlete shall occupy the shooting line, while the other athletes remain behind the 1m line. 
    No more than one athlete at a time shall be in front of the 1m line;
    Wheelchair athletes may stay on the shooting line throughout the match. 
    They indicate that they have finished shooting by raising a hand above the head (see Chapter 21 - Para-Archery);
    When moving forward to shoot, athletes shall not remove their arrows from their quivers until they are on the shooting line;
    In compound team matches, athletes shall not hook their release aids on the string until they are on the shooting line 
    and the signal for shooting has been given. 
    Once the athlete is on the shooting line and the signal for shooting has been given, the release aid may be hooked 
    either before or after the arrow nocked on the string. 
    This restriction shall not apply to Para Athletes whose classification permits a mouth tab which is permanently attached to the string;
    Violations of the Team Event rules shall be handled according to Chapter 15 - Consequences of Breaking Rules.
    
    13.2.3.
    In the Team Event Finals Rounds (alternating shooting):
    Both teams shall start each end of their match with their athletes behind the 1m line;
    The higher placed team in the Qualification Round shall decide the order of shooting of the first end. 
    The team with the lower set points score for recurve and barebow, or lower cumulative score for compound 
    shall shoot first the next end/set. 
    If teams are tied, the team which started the match shall shoot first;
    Each team has to alternate between their members after each shot so that each member has shot 
    one arrow in each phase of the rotation;
    When the first team has shot three arrows (two for Mixed Team) and the athlete has returned behind the 1m line 
    the clock of that team is stopped, displaying the time remaining;
    When the clock of the second team is started, the first athlete of that team may cross the 1m line and start shooting;
    This is repeated until both teams have shot six arrows (four for Mixed Team) or their time has expired;
    The team that shot first in the match shall start shooting the shoot-off and 
    the alternation between the teams shall take place after every shot arrow.
    
    Shootoff
      Elimination: In the event of a shoot-off the teams will have 60 sec. / 40 sec. to shoot three / two arrows, 
        one archer at a time shooting their arrow.  The teams shoot simultaneously.
      Finals round: the teams shoot alternately, each team shoots one arrow in alternation.
        Team A one arrow, team B one arrow etc: A-B-A-B (Mixed) / A-B-A-B-A-B

    13.4.1.2.
Transitional rule for all other events until 1 March 2023:

    20 seconds per arrow is the time allowed for individual alternate shooting (30 seconds for para-archery), 
    and for all team and mixed team rounds including shoot-offs;
    40 seconds per arrow is the time allowed for individual shooting during qualifications, Olympic round and 
    Compound rounds where alternate shooting does not apply, including shoot-offs.

13.2.3.
In the Team Event Finals Rounds (alternating shooting):

    Both teams shall start each end of their match with their athletes behind the 1m line;
    The higher placed team in the Qualification Round shall decide the order of shooting of the first end. 
    The team with the lower set points score for recurve and barebow, or lower cumulative score for compound 
    shall shoot first the next end/set. If teams are tied, the team which started the match shall shoot first;

    Each team has to alternate between their members after each shot so that each member has shot one arrow in 
    each phase of the rotation;
    When the first team has shot three arrows (two for Mixed Team) and the athlete has returned behind the 1m line 
    the clock of that team is stopped, displaying the time remaining;
    When the clock of the second team is started, the first athlete of that team may cross the 1m line and start shooting;
    This is repeated until both teams have shot six arrows (four for Mixed Team) or their time has expired;
    The team that shot first in the match shall start shooting the shoot-off and the alternation between the teams 
    shall take place after every shot arrow.

14.5.2.3.
Teams:

    A three-arrow (two-arrow for Mixed Team) shoot-off for score, a single arrow by each team member;
    If the score is tied, the team with the arrow closest to the centre shall win;
    If still tied the second arrow (or third) closest to the centre shall determine the winner.

*/

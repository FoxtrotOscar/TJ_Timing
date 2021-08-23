//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//          Sergey Parshin Menu1
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
int goMenu(bool displayParams)
{
  // defines the type of the menu state
  // note: difference between "enum" and "enum class" is that the latter one is a more strict in conversion to ingeters. 
  // In particular if you do the following: 
  //
  //  enum Something { A = 1, B = 2, C = 3};
  //
  // then you can do:
  //
  //  int value = A; 
  //
  // or equally: 
  //
  //  int value = Something::A; 
  //
  // This will compile and work as expected, as well as: 
  // 
  //  Something value = A; 
  //
  // but if you define "Something" as: 
  // 
  //  enum class Something { A = 1, B = 2, C = 3};
  // 
  // then the following is the compiler error: 
  // 
  //  int value = Something::A;  // an error  
  //
  // And only the following is a valid syntax: 
  // 
  //  Something value = Something::A; 
  // 
  // or to get an int value, you have to do an explicit cast:
  //
  //  int value = (int)Something::A;  
  //
  // or better in C++ way:
  //  
  //  int value = static_cast<int>(Something::A);
  // 
  // So in short - if you need your enum to hold some integer values - use "enum", 
  // if you need enum to just hold some named distinguished values (like states of the state machine)
  //  - better way is to use "enum class" 
  //


  enum class MenuState {
    start, 
    awaiting_confirmation_1, 
    awaiting_confirmation_2, 
    menu_root,
    start_INTERVALS_menu, 
    start_duration_menu,
    start_ends_menu,
    start_practice_menu,
    start_details_menu,
    start_walkup_menu,
    start_finals_menu,
    confirmed,
  };
    
  enum {
    MENU_INDEX_DURATION   = 0, 
    MENU_INDEX_ENDS       = 1, 
    MENU_INDEX_PRACTICE   = 2, 
    MENU_INDEX_DETAILS    = 3,
    MENU_INDEX_WALKUP     = 4,
    MENU_INDEX_FINALS     = 5,
    MENU_INDEX_INTERVAL   = 6,                  // redundant really - change to another function? Teamplay perhaps,
                                                // or keep teamplay for RFID and just clear the option
  };

  
  uint8_t   rootMenuIndex         = 0;        // currently selected menu item at root level menu
  uint8_t   menuStartCountsIndex  = 0;        // currently selected item in the "duration" submenu
  uint8_t   menuWalkup            = 0;        // same as above
  uint8_t   menuMaxEnds           = 0;        // currently selected value for 'maxEnds' - unconfirmed one
  uint8_t   menuPractice          = 0;        // same as above
  uint8_t   menuDetails           = 0;        // same as above
  uint8_t   menuFinals            = 0;
  uint8_t   menuAlternating       = 0;
  uint16_t  menuInterval          = 0;

  
  for (
    MenuState state = MenuState::start;  // Creates a particular instance of the state, default state is "started"
    state != MenuState::confirmed;  // keep looping until state reaches "confirmed" 
  )
  {
    switch (state)
    {
    case MenuState::start:

      u8x8.setFont(u8x8_font_chroma48medium8_r);
      wipeOLED(); 
      if (displayParams){
        EEPROM.put(0, paramStore);              // Here the paramStore data is copied into EEPROM 
        EEPROM.put(29, 111);                    // set flag for stored parameters
        displayParamsOnOLED();
      } else 
        displayParams = true;                   // we skip displayParamsOnOLED only for the first time, 
                          // so if some code below jumps back to start, 
                          // we would display the whole current params again
      u8x8.clearLine(5);

      // Initial state - display confirmation request
      writeMenuCommands();
      state = MenuState::awaiting_confirmation_1; 
      break; // jumps into the next iteratino of the main for loop, and from there - directly into the switch statement that brings us into awaiting_confirmation_1

    case MenuState::awaiting_confirmation_1:      
    
        // Actually wait for user confirmation
        // Pressing BTN1 would proceed with requesting it to be pressed 2nd time
        // Pressing any other button will open the menu
      
      switch (waitButton()) 
      {
      case BUTTON1:
        u8x8.setCursor(0, 5); 
        u8x8.print("B1 Pressed      ");
        u8x8.setCursor(0, 6);
        u8x8.print("Confirm:  BTN[1]");
        u8x8.setCursor(0, 7);
        u8x8.print("Change:   BTN[2]");
        state = (continueOn == 0) ? MenuState::confirmed : MenuState::awaiting_confirmation_2;
        break;
        
      case BUTTON2: 
        u8x8.fillDisplay();
        delay(30);
        clearFromLine(1);
      
        u8x8.draw2x2String(6, 2, "OK");
        u8x8.draw2x2String(4, 5, "MENU:");
        zeroSettings();
        delay(700);
        wipeOLED();
        // go to menu_root label to display the root level of the menu
        state = MenuState::menu_root;
        break;

      case BUTTON3:
          
        for (bool done = false; !done; )
        {
          wipeOLED();
          delay(30);
          clearFromLine(1);
          if (paramStore.breakPeriod > 240) paramStore.breakPeriod = 240;  // catch spurious high numbers
          u8x8.setCursor(3, 2);
          u8x8.setFont (u8x8_font_amstrad_cpc_extended_f);
          u8x8.print("RUN ");
          u8x8.print(paramStore.breakPeriod);
          u8x8.print(" min");
          u8x8.draw2x2String(2, 4, "TIMER?");
          u8x8.setCursor(1, 7);
          u8x8.print("[1]Yes / No[4]");
                       
          switch (waitButton()) {
          case BUTTON1:
            intervalOn = true;
            state = MenuState::start;
            done = true;
            break;
        
          case BUTTON2:
            paramStore.breakPeriod > 231 ?  paramStore.breakPeriod = 5 : paramStore.breakPeriod++;
            do { paramStore.breakPeriod ++; } while (paramStore.breakPeriod % 10);    
            break;
             
            
          case BUTTON3:
            paramStore.breakPeriod <= 3 ?  paramStore.breakPeriod = 240 : paramStore.breakPeriod --;
            break;

              
          case BUTTON4:
            intervalOn = false;
            state = MenuState::start;
            done = true;
            break;
          }
        }       
        break;
        
      case BUTTON4:
        displayParamsOnOLED(); //+++++++++++++++++++++++++++++++++++++++++++++++++
        writeSplash(false);
        delay(2 * tick);
        intervalOn = false;
        started = false;
        writeInfoBigscreen();
        writeMenuCommands();
        // state - unchanged  -- any other button - go and read button state again
        break;

      }
      break;

      


    case MenuState::awaiting_confirmation_2: 
      // Now - awaiting 2nd confirmation
      switch (waitButton()) {
      case BUTTON1:
        //HCuOLED.clear();
        u8x8.setCursor(0, 7); 
        u8x8.print("     Confirmed");
        state = MenuState::confirmed;
        break;
      case BUTTON2:
        u8x8.fillDisplay();
        for( uint8_t r = 1; r < 8; r++ )
        {
          u8x8.clearLine(r);
          delay(20);
        } 
        u8x8.draw2x2String(6, 2, "OK");
        u8x8.draw2x2String(4, 5, "MENU:");
        delay(700);
        state = MenuState::menu_root;
        break;
      }
      
      break;

      
    ///////////////////////////////////////////
    ////////////// Main Menu //////////////////
    ///////////////////////////////////////////
    case MenuState::menu_root: 
      // Display the root page of the menu, 0 - page index, rootSelectionIdx - current selection 
      wipeOLED();
      displayMenuPage(0, rootMenuIndex);
  
      switch (waitButton()) {      
      case BUTTON1:
        if (rootMenuIndex == MENU_INDEX_DURATION) {
          // selected time duration 
          menuStartCountsIndex = paramStore.startCountsIndex;
          state = MenuState::start_duration_menu;
        }
        else if (rootMenuIndex == MENU_INDEX_ENDS) {
          menuMaxEnds = paramStore.maxEnds;
          state = MenuState::start_ends_menu;
        }
        else if (rootMenuIndex == MENU_INDEX_PRACTICE) {
          menuPractice = paramStore.maxPrac ;
          state = MenuState::start_practice_menu;
        }
        else if (rootMenuIndex == MENU_INDEX_DETAILS) {
          menuDetails = paramStore.Details;
          state = MenuState::start_details_menu;
        }
        else if (rootMenuIndex == MENU_INDEX_WALKUP) {
          menuWalkup = paramStore.walkUp;
          state = MenuState::start_walkup_menu;
        }
        else if (rootMenuIndex == MENU_INDEX_FINALS) {
          menuFinals = paramStore.isFinals;
          menuAlternating = paramStore.isAlternating;
          state = MenuState::start_finals_menu;
        }
        else if (rootMenuIndex == MENU_INDEX_INTERVAL) {
          menuInterval = (paramStore.breakPeriod > 240 ? 10 : paramStore.breakPeriod);  // catch spurious numbers
          state = MenuState::start_INTERVALS_menu;
        }
        break;

      case BUTTON3: 
        // state unchanged - menu_root 
        rootMenuIndex = (rootMenuIndex + 1) % 7; // this will increment rootMenuIndex, but if it was 6 before - it would jump back to 0
        break;

      case BUTTON2: 
        // state unchanged - menu_root 
        rootMenuIndex = (rootMenuIndex -1 + 7) % 7; // same as above but for decrement, and if it was 0 - it would jump to 6
        break;

      case BUTTON4:
        state = MenuState::start;
        break;
      }
      
      break;


    ////////////////////////////////////////////
    ////////////// Intervals Menu //////////////
    ////////////////////////////////////////////

    case MenuState::start_INTERVALS_menu: 
      wipeOLED();
      u8x8.setCursor(0, 2);
      u8x8.print("Timer Dur: ");
      u8x8.print(menuInterval);
      doButtonMenu();
      
      switch (waitButton())
      {
      case BUTTON1: 
        paramStore.breakPeriod = menuInterval;                          // save the new val
        state = MenuState::menu_root;
        break;
        
      case BUTTON2: 
        menuInterval > 231 ? menuInterval = 5 : menuInterval +=1;       // UP in 10s and not past 240
        do { menuInterval ++; } while (menuInterval % 10);        
        break;
        
      case BUTTON3: 
        (menuInterval <= 2) ? menuInterval = 240 : menuInterval --;     // DOWN in single digits, not lower than 2
        break;
  
      case BUTTON4:
        menuInterval = paramStore.breakPeriod;                          // no change, exit
        state = MenuState::menu_root;
        break;
      }
      
      break;
  
    ///////////////////////////////////////////
    ////////////// Duration Menu //////////////
    ///////////////////////////////////////////
    case MenuState::start_duration_menu: 
      wipeOLED();
      u8x8.setCursor(0, 2);
      u8x8.print("Start Dur: "); 
      u8x8.print(startCounts[menuStartCountsIndex]);
      doButtonMenu();
      switch (waitButton())
      {
      case BUTTON1: 
        paramStore.startCountsIndex = menuStartCountsIndex;
        paramStore.notFlint = true;
        state = MenuState::menu_root;
        break;
  
      case BUTTON2: 
        menuStartCountsIndex = (menuStartCountsIndex -1 + 7) % 7;   // cycle through 6 options
        break; // state unchanged 
  
      case BUTTON3: 
        menuStartCountsIndex = (menuStartCountsIndex +1) % 7;       // ditto
        break;
  
      case BUTTON4:
        menuStartCountsIndex = paramStore.startCountsIndex; 
        state = MenuState::menu_root;
        break;
      }

      break;

    ///////////////////////////////////////////
    ////////////// Ends Menu //////////////////
    ///////////////////////////////////////////
    case MenuState::start_ends_menu: 
      wipeOLED();
      u8x8.setCursor(0, 2);
      u8x8.print("Ends: "); u8x8.print(menuMaxEnds);
      doButtonMenu();
  
      switch (waitButton())
      {
      case BUTTON1: 
        paramStore.maxEnds = menuMaxEnds;
        paramStore.notFlint = true;
        sEcount = 1;
        state = MenuState::menu_root;
        break;
  
      case BUTTON3: 
        if (menuMaxEnds > 1)
          -- menuMaxEnds;
        break;
  
      case BUTTON2: 
        if (menuMaxEnds < 100)
          ++ menuMaxEnds;
        break;
  
      case BUTTON4:
        menuMaxEnds = paramStore.maxEnds;
        state = MenuState::menu_root;
        break;
      }
  
      break;

    ///////////////////////////////////////////
    ////////////// Practice Menu //////////////////
    ///////////////////////////////////////////
    case MenuState::start_practice_menu:
      wipeOLED();
      u8x8.setCursor(0, 2);
      u8x8.print("Practice: "); u8x8.print(menuPractice);
      doButtonMenu();
  
      switch (waitButton())
      {
      case BUTTON1: 
        paramStore.maxPrac = countPractice = menuPractice;
        state = MenuState::menu_root;
        break;
  
      case BUTTON3: 
        if (menuPractice > 0)
        -- menuPractice;
        break;
  
      case BUTTON2: 
        if (menuPractice < 4)
        ++ menuPractice;
        break;
  
      case BUTTON4:
        menuPractice = countPractice = paramStore.maxPrac;
        state = MenuState::menu_root;
        break;
      }
  
      break;

    ///////////////////////////////////////////
    ////////////// Details Menu ///////////////
    ///////////////////////////////////////////

    case MenuState::start_details_menu: 
      wipeOLED();
      u8x8.setCursor(0, 2);
      u8x8.print("Details: "); 
      u8x8.print(menuDetails == 2 ? "Double " : "Single ");
      doButtonMenu();
  
      switch (waitButton())
      {
      case BUTTON1: 
        paramStore.Details = menuDetails;
        paramStore.notFlint = true;
        state = MenuState::menu_root;
        break;
  
      case BUTTON2: 
        menuDetails = menuDetails == 2 ? 1 : 2; 
        break;
  
      case BUTTON3: 
        menuDetails = menuDetails == 2 ? 1 : 2; 
        break;
  
      case BUTTON4:
        menuDetails = paramStore.Details;
        state = MenuState::menu_root;
        break;
      }
      break;


    ///////////////////////////////////////////
    ////////////// Walkup Menu ///////////////
    ///////////////////////////////////////////

    case MenuState::start_walkup_menu: 
      wipeOLED();
      u8x8.setCursor(0, 2);
      u8x8.print("Walkup Dur: "); 
      u8x8.print(menuWalkup == 10 ? 10 : 20);  
      doButtonMenu();
      
      switch (waitButton())
      {
      case BUTTON1: 
        paramStore.walkUp = menuWalkup;
        state = MenuState::menu_root;
        break;
  
      case BUTTON2: 
        menuWalkup = menuWalkup == 10 ? 20 : 10;
        break;
  
      case BUTTON3: 
        menuWalkup = menuWalkup == 10 ? 20 : 10;
        break;
  
      case BUTTON4:
        menuWalkup = paramStore.walkUp;
        state = MenuState::menu_root;
        break;
  
      }
      break;


    ///////////////////////////////////////////
    ////////////// Finals Menu ////////////////
    ///////////////////////////////////////////

    case MenuState::start_finals_menu:
      wipeOLED();

      // loop on "Finals" selection 
      for (bool selected = false; !selected; )
      {
        u8x8.setCursor(0, 2);
        u8x8.print("Finals: "); 
        u8x8.print(menuFinals == 0 ? "No " : "Yes");   

        switch (waitButton())
        {
        case BUTTON1: 
          paramStore.isFinals = menuFinals;
          paramStore.notFlint = true;
          selected = true;
          break;        
        
        case BUTTON2: 
        case BUTTON3: 
          menuFinals = menuFinals == 0 ? 1 : 0;
          break;
        
        case BUTTON4:
          menuFinals = paramStore.isFinals;
          if (menuFinals == 0)
          {         
            state = MenuState::menu_root;
          }
          selected = true;
          break;
        }
      }

      if (state != MenuState::start_finals_menu)
        break;       
      
      u8x8.setCursor(0, 2);
      u8x8.print("Finals: "); 
      u8x8.print(menuFinals == 0 ? "No " : "Yes");
      
      if (menuFinals)
      {
        paramStore.notFlint = true;
        paramStore.Details = 1;                           //  Single detail by default
        paramStore.startCountsIndex = 4;                  //  set to 20sec
        paramStore.maxPrac = 0;                           //  no practice
        paramStore.maxEnds = 5;                           //  5 ends
      
        for (bool done = false; !done; )
        {
          u8x8.setCursor(0, 4);
          u8x8.print("Alternating: ");
          u8x8.print(menuAlternating == 0 ? "No " : "Yes");
          doButtonMenu();
        
          switch (waitButton())
          {
          case BUTTON1: 
            paramStore.isAlternating = menuAlternating;
            state = MenuState::start;
            done = true;
            break;
        
          case BUTTON2: 
            menuAlternating = menuAlternating == 0 ? 1 : 0;
            break;
        
          case BUTTON3: 
            menuAlternating = menuAlternating == 0 ? 1 : 0;
            break;
        
          case BUTTON4:
            menuAlternating = paramStore.isAlternating;
            state = MenuState::menu_root;     
            done = true;
            break;
          }
        }
      }
      
      doButtonMenu();
      
      switch (waitButton())
      {
      case BUTTON1: 
        paramStore.isFinals = menuFinals;
        paramStore.notFlint = true;
        state = MenuState::start;
        break;

      case BUTTON2: 
        menuFinals = menuFinals == 0 ? 1 : 0;
        break;

      case BUTTON3: 
        menuFinals = menuFinals == 0 ? 1 : 0;
        break;

      case BUTTON4:
        menuFinals = paramStore.isFinals;
        state = MenuState::menu_root;
        break;

      }
      
      break;

      
    case MenuState::confirmed:
      break;
    }
  }


  // we are out of the loop as we've reached 'confirmed' state 
  clearFromLine(5); 
  u8x8.draw2x2String(0, 6, "..WAIT..");
  intervalOn = false;
  started = false;
  delay(250);
  
  return 1;
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
  u8x8.setCursor(0, 5); 
  intervalOn ? u8x8.print      ("COUNTDOWN is ON-") : u8x8.print("Continue: BTN[1]");
  u8x8.setCursor(0, 6);
  intervalOn ? u8x8.print      ("-to QUIT: BTN[4]") : u8x8.print("Change:   BTN[2]");
  u8x8.setCursor(0, 7);
  u8x8.inverse();
  u8x8.print("or TIME-TAP now ");
  u8x8.noInverse();
}





/*
 * 2 x Functions for reading value of a single button 
 * identified by the pin number given in the argument 
 * returned value: 
 * 0 - button is not pressed 
 * 1 - button is pressed 
 * 
 * De-bouncing is by implementing small sleep when button press is detected 
 * and doing a second read to verify if button is still pressed)
 * A)   
*/  
uint8_t readButton(int pin) {
  if (digitalRead(pin) == HIGH) return 0; 
  delay(5);                                 // only reached if pin goes LOW, de-bounce delay, confirm read
  return (digitalRead(pin) == LOW) ? 1 : 0;
}
/*
 *  B)  
*/                                    
uint8_t readButtonNoDelay(int pin) {        // as above, without small sleep if button press detected
  return (digitalRead(pin) == LOW) ? 1 : 0;
}

/*
 * Function to read values of all four known buttons 
 * Returns bitmask of pressed buttons or 
 * just one of the BUTTON1..BUTTON4 values if only 
 * one button is pressed; Also handles de-bounce 
 */ 
uint8_t readButtons() {
  uint8_t ret = 0;

  if (   digitalRead(button1Pin)  == HIGH 
      && digitalRead(button2Pin)  == HIGH 
      && digitalRead(button3Pin)  == HIGH 
      && digitalRead(button4Pin)  == HIGH) {
      return 0;                                       // all are high - no buttons pressed 
  }
  delay(2);                                           // delay for level de-bouncing; 2nd read to confirm low
  if (!intervalOn && digitalRead(button1Pin) == LOW)
    ret |= BUTTON1;                                   // set leftmost bit in the 'ret' to 1

  if (!intervalOn && digitalRead(button2Pin) == LOW)
    ret |= BUTTON2;                                   // send 2nd leftmost bit in the 'ret' to 1

  if (!intervalOn && digitalRead(button3Pin) == LOW)
    ret |= BUTTON3;                                   // etc...

  if (digitalRead(button4Pin) == LOW){
    if (intervalOn){
      clearFromLine(1);
    }
    ret |= BUTTON4;
  }
  return ret;
}

/*
 * Similar to readButtons, but would wait in the infinite loop until any of 
 * the buttons is pressed, and will return value of the 
 * button pressed (BUTTON1..BUTTON4), or bitmask for multiple buttons pressed at once 
 * Use this function when it is required to wait for 
 * user input, and there is nothing to do until user press some button
*/ 
uint8_t waitButton() {
  long long timeOut = millis();
  bool flag = false; 
  for (;;) {
    if (!intervalOn && !flag && (millis() - timeOut > (tick*60*15))) {
      clearMatrix();
      bright = 25;                                          // dim the logo after the period above
      writeSplash(true); 
      bright = 255;                                         // reset the brightness
      flag = true;
    }
    checkIntervalTimer();
    
    getRFID(&paramStore);
    
    uint8_t ret = readButtons();                            // read all button states
    if (ret != 0) {
      while (readButtons() != 0) delay(1);                  // and now wait for him to release the button
      HC12.print(F("brightness "));                         // return brightness to nominal 
      HC12.print(bright);                                                      
      HC12.print(F("\r"));
      return ret;                                           // finally return value of the button he pressed moments ago
      }
    delay(1);
  }
}


bool goEmergencyButton(uint8_t AIndex){
  bool ret = false;
  if (readButtons() == BUTTON4) {                           // Handle Red-button push
    goWhistle(5);
    stopSign();
    clearFromLine(1);
    u8x8.setCursor(4,3);
    u8x8.print("EMERGENCY");
    u8x8.setCursor(6,5);
    u8x8.print("STOP");
    delay(5*tick);
    n_Count = handleEmergencyRestart();                     // go fetch the next step decision
    if (AIndex < 3){    
      displayParamsOnOLED();
      if (n_Count == startCounts[paramStore.startCountsIndex]) {
        doBarCount(AIndex);
        writeOLED_Data(1);
      } else if (n_Count != 0){
        goWhistle(1);
        writeOLED_Data(1);
      }
    } else if (AIndex == 3){                                // for in-barCount !STOP!
      displayParamsOnOLED();
      writeOLED_Data(1);
      ret = true;
    } else if (AIndex == 4){
                                                            //saved for future use
    }
  }
  return ret;
}

/*
 * Write a menu to select 1 of two options:
 * 1 go back 10 secs and resume
 * 2 go back to zero secs and resume
 * 3 do a re-start keeping current parameters
 */
int16_t handleEmergencyRestart(void){
  clearFromLine(1);
  u8x8.setCursor(0, 2);
  u8x8.inverse(); 
  u8x8.print("Resume:   BTN[1]");
  u8x8.setCursor(0, 3);
  u8x8.setFont(u8x8_font_5x7_f);
  u8x8.noInverse();
  u8x8.print("(steps back 10s)");
  u8x8.setCursor(0, 4);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.inverse();
  u8x8.print("Restart:  BTN[2]");
  u8x8.setCursor(0, 5);
  u8x8.setFont(u8x8_font_5x7_f);
  u8x8.noInverse();
  u8x8.print("(recommence end)");
  u8x8.setCursor(0, 6);
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.inverse();
  u8x8.print("Reset ALL:BTN[3]");
  u8x8.setCursor(0, 7);
  u8x8.setFont(u8x8_font_5x7_f);
  u8x8.noInverse();
  u8x8.print("(reinitialize)");
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  
  uint8_t maxPossSecs = startCounts[paramStore.startCountsIndex];
  
  switch (waitButton()) {

    case BUTTON1: 
      n_Count = (n_Count +11 > maxPossSecs) ? maxPossSecs : n_Count +11;  // rewind 10 sec and continue
      clearMatrix();
    break;
    
    case BUTTON2:
      reStartEnd = true;                                    
      n_Count = startCounts[paramStore.startCountsIndex];                 // restart this end, continue with competition
      clearMatrix(); //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    break;

    case BUTTON3:         
      startOver = true;                                                   // reset and start it all again
      n_Count = 0;
    break;
  }
  
  return n_Count ;
}

void displayMenuPage(uint8_t idx, uint8_t selectionIdx) {
  
  if (idx == 0) {
    for (uint8_t i = 0; i < sizeof(menu0)/sizeof(menu0[0]); ++ i) {
      u8x8.setCursor(0, i+1);
      if (i == selectionIdx)
        u8x8.print("> ");
      else 
        u8x8.print("  ");

      u8x8.print(menu0[i]);
    }
  }  
}

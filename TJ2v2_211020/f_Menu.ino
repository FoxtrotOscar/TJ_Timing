///*
//
//  uint8_t startCountsIndex = 1;         // (1)Number from 0 to 4 indentifying which of startCounts is used in this round, default 120 
//  uint8_t walkUp = 10;                  // (2)
//  uint8_t maxEnds = 4;                  // (3)Total number of Ends for competition
//  uint8_t Details = 2;                  // (4)Single (1) or Double detail (2)
//  uint8_t maxPrac = 2;                  // (5)Initially set as 2x practice ends
//  uint8_t isFinals = 0;                 // (6)For alternating A & B session
//  uint8_t isAlternating = 0;            // (8)1 == Archer A, Archer B; 0 == Simultaneous
//  uint8_t teamPlay = 0;                 // (9)
//  uint8_t whichArcher = 1;              //(10)
//  FUNCTIONS:
//    displayMenuPage
//    goMenu
//      awaiting_confirmation_1
//      awaiting_confirmation_2
//      menu_root
//      start_INTERVALS_menu
//      start_duration_menu
//      start_ends_menu
//      start_practice_menu
//      start_details_menu
//      start_walkup_menu
//      start_finals_menu
//      start_alternating_menu
//      confirmed
//    doButtonMenu
//    writeMenuCommands
//    handleEmergencyRestart
//    readButton
//    readButtonNoDelay
//    readButtons
//    waitButton
//    goEmergencyButton
//*/
// **************************************************************************************
//                    Sergey Parshin          Menu2
// **************************************************************************************

// defines the type of the menu state

struct MenuContext
{
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

  // Context variables 
  bool      displayParams     = false;
  uint8_t   rootMenuIndex         = 0;        // currently selected menu item at root level menu
  uint8_t   menuStartCountsIndex  = 0;        // currently selected item in the "duration" submenu
  uint8_t   menuWalkup            = 0;        // same as above
  uint8_t   menuMaxEnds           = 0;        // currently selected value for 'maxEnds' - unconfirmed one
  uint8_t   menuPractice          = 0;        // same as above
  uint8_t   menuDetails           = 0;        // same as above
  uint8_t   menuFinals            = 0;
  uint8_t   menuAlternating       = 0;
  uint16_t  menuInterval          = 0;  


  // each of the following functions handles particular state, and returns what must be the next state 
  
  MenuState handle_start()
  {
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
  
      
      writeMenuCommands();                        // Initial state - display confirmation request
      
      
      return MenuState::awaiting_confirmation_1;  // return what must be the next state of the State Machine 
  }
  
  MenuState handle_awaiting_confirmation_1()      // Actually wait for user confirmation
                                                  // Pressing BTN1 would proceed with requesting it to be pressed 2nd time
                                                  // Pressing any other button will open the menu
  {
    switch (waitButton()) 
      {
      case BUTTON1:
        u8x8.setCursor(0, 5); 
        u8x8.print("B1 Pressed      ");
        u8x8.setCursor(0, 6);
        u8x8.print("Confirm:  BTN[1]");
        u8x8.setCursor(0, 7);
        u8x8.print("Change:   BTN[2]");
        return (continueOn == 0) ? MenuState::confirmed : MenuState::awaiting_confirmation_2;
        
      case BUTTON2: 
        u8x8.fillDisplay();
        pauseMe(30);
        clearFromLine(1);
        
        u8x8.draw2x2String(6, 2, "OK");
        u8x8.draw2x2String(4, 5, "MENU:");
        zeroSettings();
        pauseMe(700);
        wipeOLED();
        
        return MenuState::menu_root;                // go to menu_root label to display the root level of the menu
  
      case BUTTON3:
          
        for (;;)                                    // the only exits from this loop are two return statements 
                                                    //for cases where user did press BTN1 or BTN4 
        {
          wipeOLED();
          pauseMe(30);
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
            intervalOn  = true;
            started     = false;
            return MenuState::start;
          
          case BUTTON2:
            paramStore.breakPeriod > 231 ?  paramStore.breakPeriod = 5 : paramStore.breakPeriod++;
            do { paramStore.breakPeriod ++; } while (paramStore.breakPeriod % 10);    
            break;
            
            
          case BUTTON3:
            paramStore.breakPeriod <= 3 ?  paramStore.breakPeriod = 240 : paramStore.breakPeriod --;
            break;
            
          case BUTTON4:
            intervalOn = false;
            return MenuState::start;
          }
        }       
        break;
        
      case BUTTON4:
        displayParamsOnOLED(); //+++++++++++++++++++++++++++++++++++++++++++++++++
        u8x8.draw2x2String(0, 6, "..WAIT..");
        writeSplash(false);
        pauseMe(2 * tick);
        intervalOn  = false;
        started     = false;
        writeInfoBigscreen();
        writeMenuCommands();
        // state - unchanged  -- any other button - go and read button state again
        break;
    }
    
    // unless we have returned another state somewhere above - return the current state again, as we want to iterate here more 
    return MenuState::awaiting_confirmation_1;
  }
  
  MenuState handle_awaiting_confirmation_2()
  {
      
      switch (waitButton()) {                           // Now - awaiting 2nd confirmation
      case BUTTON1:
        //HCuOLED.clear();
        u8x8.setCursor(0, 7); 
        u8x8.print("     Confirmed");
        return MenuState::confirmed;
      
      case BUTTON2:
          u8x8.fillDisplay();
          for( uint8_t r = 1; r < 8; r++ )
          {
              u8x8.clearLine(r);
              pauseMe(20);
          } 
          u8x8.draw2x2String(6, 2, "OK");
          u8x8.draw2x2String(4, 5, "MENU:");
          pauseMe(700);
          return MenuState::menu_root;
      }
      
    return MenuState::awaiting_confirmation_2; // by default always return the current state 
  }
  
  MenuState handle_menu_root()
  {
      ///////////////////////////////////////////
      ////////////// Main Menu //////////////////
      ///////////////////////////////////////////
      
      // Display the root page of the menu, 0 - page index, rootSelectionIdx - current selection 
      wipeOLED();
      displayMenuPage(0, rootMenuIndex);
    
      switch (waitButton()) {      
      case BUTTON1:
          if (rootMenuIndex == MENU_INDEX_DURATION) {
              // selected time duration 
            menuStartCountsIndex = paramStore.startCountsIndex;
            return MenuState::start_duration_menu;
          }
          else if (rootMenuIndex == MENU_INDEX_ENDS) {
            menuMaxEnds = paramStore.maxEnds;
            return MenuState::start_ends_menu;
          }
          else if (rootMenuIndex == MENU_INDEX_PRACTICE) {
            menuPractice = paramStore.maxPrac ;
            return MenuState::start_practice_menu;
          }
          else if (rootMenuIndex == MENU_INDEX_DETAILS) {
            menuDetails = paramStore.Details;
            return MenuState::start_details_menu;
          }
          else if (rootMenuIndex == MENU_INDEX_WALKUP) {
            menuWalkup = paramStore.walkUp;
            return MenuState::start_walkup_menu;
          }
          else if (rootMenuIndex == MENU_INDEX_FINALS) {
            menuFinals = paramStore.isFinals;
            menuAlternating = paramStore.isAlternating;
            return MenuState::start_finals_menu;
          }
          else if (rootMenuIndex == MENU_INDEX_INTERVAL) {
            menuInterval = (paramStore.breakPeriod > 240 ? 10 : paramStore.breakPeriod);  // catch spurious numbers
            return MenuState::start_INTERVALS_menu;
          }
          break;
    
      case BUTTON3: 
          // state unchanged - menu_root 
          rootMenuIndex = (rootMenuIndex + 1) % 7; // this will increment rootMenuIndex, but if it was 6 before - it would jump back to 0
          return MenuState::menu_root;
    
      case BUTTON2: 
        // state unchanged - menu_root 
        rootMenuIndex = (rootMenuIndex -1 + 7) % 7; // same as above but for decrement, and if it was 0 - it would jump to 6
        return MenuState::menu_root;
    
      case BUTTON4:
        return MenuState::start;
      }
      
      return MenuState::menu_root; // by default always return the current state 
  }
  
  MenuState handle_start_INTERVALS_menu()
  {
      ////////////////////////////////////////////
      ////////////// Intervals Menu //////////////
      ////////////////////////////////////////////
      wipeOLED();
      u8x8.setCursor(0, 2);
      u8x8.print("Timer Dur: ");
      u8x8.print(menuInterval);
      doButtonMenu();
      
      switch (waitButton())
      {
      case BUTTON1: 
        paramStore.breakPeriod = menuInterval;                          // save the new val
        return MenuState::menu_root;
        
      case BUTTON2: 
        menuInterval > 231 ? menuInterval = 5 : menuInterval +=1;       // UP in 10s and not past 240
        do { menuInterval ++; } while (menuInterval % 10);        
        break;
        
      case BUTTON3: 
        (menuInterval <= 2) ? menuInterval = 240 : menuInterval --;     // DOWN in single digits, not lower than 2
        break;
    
      case BUTTON4:
        menuInterval = paramStore.breakPeriod;                          // no change, exit
        return MenuState::menu_root;
      }
      
      return MenuState::start_INTERVALS_menu; // by default always return the current state 
  }
  
  MenuState handle_start_duration_menu()
  {
      ///////////////////////////////////////////
      ////////////// Duration Menu //////////////
      ///////////////////////////////////////////
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
        return MenuState::menu_root;
    
      case BUTTON2: 
        menuStartCountsIndex = (menuStartCountsIndex -1 + 9) % 9;   // cycle through 8 options <<------------------------
        break; // state unchanged 
    
      case BUTTON3: 
        menuStartCountsIndex = (menuStartCountsIndex +1) % 9;       // ditto <<------------------------
        break;
    
      case BUTTON4:
        menuStartCountsIndex = paramStore.startCountsIndex; 
        return MenuState::menu_root;
      }
  
      return MenuState::start_duration_menu; // by default always return the current state 
  }
  
  MenuState handle_start_ends_menu()
  {
      ///////////////////////////////////////////
      ////////////// Ends Menu //////////////////
      ///////////////////////////////////////////
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
        return MenuState::menu_root;
    
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
        return MenuState::menu_root;
      }
    
      return MenuState::start_ends_menu; // by default always return the current state 
  
  }
  
  MenuState handle_start_practice_menu()
  {
      ///////////////////////////////////////////
      ////////////// Practice Menu //////////////////
      ///////////////////////////////////////////
      wipeOLED();
      u8x8.setCursor(0, 2);
      u8x8.print("Practice: "); u8x8.print(menuPractice);
      doButtonMenu();
    
      switch (waitButton())
      {
      case BUTTON1: 
        paramStore.maxPrac = countPractice = menuPractice;
        return MenuState::menu_root;
    
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
        return MenuState::menu_root;
      }
    
      return MenuState::start_practice_menu; // by default always return the current state 
  }
  
  MenuState handle_start_details_menu()
  {
      ///////////////////////////////////////////
      ////////////// Details Menu ///////////////
      ///////////////////////////////////////////
  
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
        return MenuState::menu_root;
    
      case BUTTON2: 
        menuDetails = menuDetails == 2 ? 1 : 2; 
        break;
    
      case BUTTON3: 
        menuDetails = menuDetails == 2 ? 1 : 2; 
        break;
    
      case BUTTON4:
        menuDetails = paramStore.Details;
        return MenuState::menu_root;
      }
    
      return MenuState::start_details_menu; // by default always return the current state     
  }
  
  MenuState handle_start_walkup_menu()
  {
      ///////////////////////////////////////////
      ////////////// Walkup Menu ///////////////
      ///////////////////////////////////////////
      wipeOLED();
      u8x8.setCursor(0, 2);
      u8x8.print("Walkup Dur: "); 
      u8x8.print(menuWalkup == 10 ? 10 : 20);  
      doButtonMenu();
      
      switch (waitButton())
      {
        case BUTTON1: 
        paramStore.walkUp = menuWalkup;
        return MenuState::menu_root;
    
      case BUTTON2: 
        menuWalkup = menuWalkup == 10 ? 20 : 10;
        break;
    
      case BUTTON3: 
        menuWalkup = menuWalkup == 10 ? 20 : 10;
        break;
    
      case BUTTON4:
        menuWalkup = paramStore.walkUp;
        return MenuState::menu_root;
  
      }
    
      return MenuState::start_walkup_menu; // by default always return the current state    
  }
  
  MenuState handle_start_finals_menu()
  {
    ///////////////////////////////////////////
      ////////////// Finals Menu ////////////////
      ///////////////////////////////////////////
  
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
            return MenuState::menu_root;
          }
          selected = true;
          break;
        }
      }
      
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
        
        for (;;)
        {
          u8x8.setCursor(0, 4);
          u8x8.print("Alternating: ");
          u8x8.print(menuAlternating == 0 ? "No " : "Yes");
          doButtonMenu();
          
          switch (waitButton())
          {
          case BUTTON1: 
            paramStore.isAlternating = menuAlternating;
            return MenuState::start;
          
          case BUTTON2: 
            menuAlternating = menuAlternating == 0 ? 1 : 0;
            break;
          
          case BUTTON3: 
            menuAlternating = menuAlternating == 0 ? 1 : 0;
            break;
          
          case BUTTON4:
            menuAlternating = paramStore.isAlternating;
            return MenuState::menu_root;     
          }
        }
      } else {
        paramStore.isAlternating = 0;
        u8x8.setCursor(0, 4);
        u8x8.print("Alternating: ");
        u8x8.print("Off");
      }
      
      doButtonMenu();
      
      switch (waitButton())
      {
      case BUTTON1: 
        paramStore.isFinals = menuFinals;
        paramStore.notFlint = true;
        return MenuState::start;
  
      case BUTTON2: 
      case BUTTON3: 
        menuFinals = menuFinals == 0 ? 1 : 0;
        break;
  
      case BUTTON4:
        menuFinals = paramStore.isFinals;
        return MenuState::menu_root;
      }
      
      return MenuState::start_finals_menu; // by default always return the current state    
  }
  
  int go(bool displayParamsArg)
  {
    // handler functions will only have the context, so copy the argument value into it
    displayParams = displayParamsArg; 
    
    for (
      MenuState state = MenuState::start;  // Creates a particular instance of the state, default state is "started"
      state != MenuState::confirmed;  // keep looping until state reaches "confirmed" 
    )
    {
      switch (state)
      {
      case MenuState::start:                    state = handle_start(); break;
      case MenuState::awaiting_confirmation_1:  state = handle_awaiting_confirmation_1(); break;
      case MenuState::awaiting_confirmation_2:  state = handle_awaiting_confirmation_2(); break;
      case MenuState::menu_root:                state = handle_menu_root(); break;
      case MenuState::start_INTERVALS_menu:     state = handle_start_INTERVALS_menu(); break;
      case MenuState::start_duration_menu:      state = handle_start_duration_menu(); break;
      case MenuState::start_ends_menu:          state = handle_start_ends_menu(); break;
      case MenuState::start_practice_menu:      state = handle_start_practice_menu(); break;
      case MenuState::start_details_menu:       state = handle_start_details_menu(); break;
      case MenuState::start_walkup_menu:        state = handle_start_walkup_menu(); break;
      case MenuState::start_finals_menu:        state = handle_start_finals_menu(); break;
      
      case MenuState::confirmed:          break; // only handling to avoid the compiler warning (that one of the enum values is not handled)
      }
    }
    
    
    // we are out of the loop as we've reached 'confirmed' state 
    clearFromLine(5); 
    u8x8.draw2x2String(0, 6, "..WAIT..");
    intervalOn = false;
    started = false;
    pauseMe(250);
    
    return 1;
  }
};

int goMenu(bool displayParams)
{
  MenuContext menu;
  return menu.go(displayParams);
}

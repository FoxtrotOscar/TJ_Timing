



//const char* menu0[] = {
//  "Time",
//  "Walk", 
//  "Ends",
//  "Dets", 
//  "Prac",
//  "Fnls",
//  "BrkT",
//  "Altr",
//  "Team",
//  "A/B?",
//  "Flnt",
//  "Supv",
//};


//void displayMenuPage(uint8_t selectionIdx) {
//  
//  
//  for (uint8_t i = 0; i < sizeof(setParam)/sizeof(setParam[0]); ++ i) {
//    u8x8.setCursor((i < 6 ? 0 : 7) , (i < 6 ? i + 1 : i - 6));
//    if (i == selectionIdx)
//      u8x8.inverse();
//    else 
//      u8x8.noInverse();
//
//    u8x8.print(setParam[i]);
//  }  
//}
//
//
//enum {
//  MENU_INDEX_TIME       = 0,
//  MENU_INDEX_WALK       = 1, 
//  MENU_INDEX_ENDS       = 2,
//  MENU_INDEX_DETS       = 3,   
//  MENU_INDEX_PRAC       = 4, 
//  MENU_INDEX_FNLS       = 5,
//  MENU_INDEX_BRKT       = 6,
//  MENU_INDEX_ALTR       = 7,
//  MENU_INDEX_TEAM       = 8,
//  MENU_INDEX_AORB       = 9,
//  MENU_INDEX_FLNT       =10,
//  MENU_INDEX_SUPV       =11,
//
//};

//
//
//
//int goMenu(bool displayParams){
//
//  uint8_t   rootMenuIndex         = 0;        // currently selected menu item at root level menu
//  uint8_t   menuStartCountsIndex  = 0;        // currently selected item in the "duration" submenu
//  uint8_t   menuWalkup            = 0;        // same as above
//  uint8_t   menuMaxEnds           = 0;        // currently selected value for 'maxEnds' - unconfirmed one
//  uint8_t   menuDetails           = 0;        // same as above
//  uint8_t   menuPractice          = 0;        // same as above
//  uint8_t   menuFinals            = 0;
//  uint16_t  menuInterval          = 0;  
//  uint8_t   menuAlternating       = 0;
//  uint8_t   menuTeamplay          = 0;
//  uint8_t   menuAorBselect        = 0;
//  uint8_t   menuFlintRules        = 0;
//  uint8_t   menuSupervisorMode    = 0;
//
//
//
//
//start:
//
//  u8x8.setFont(u8x8_font_chroma48medium8_r);
//  wipeOLED(); 
//
//  goto awaiting_confirmation_1;
//
//awaiting_confirmation_1:
//} 

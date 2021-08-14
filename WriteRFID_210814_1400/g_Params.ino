

byte showParams(byte index){
  byte x  = 0; 
  byte y  = 2;
  byte ct = 0;
  do{
    index == ct ? u8x8.inverse() : u8x8.noInverse();  // if the index == ct then hightlight
    showOneParam(ct++);                               // show and increment the count
    delay(tick/10);                     
  } while (ct < 12);                                  // rinse, repeat
}

byte showOneParam(byte index){
  byte y = (index > 5 ? index - 6 + 2 : index + 2);
  byte x = (index > 5 ? 9 : 0);
  u8x8.setCursor(x, y);
  u8x8.print(setParam[index]);                        // display the parameter name
  u8x8.setCursor((dataBlock[index] < 100 ?            // parse the x position of the number
                    (dataBlock[index] < 10 ?          // ditto
                      x+6 : x+5) : x+4), y);          // and set its xy coordinates
  u8x8.print(dataBlock[index]);                        // send it
                   
}

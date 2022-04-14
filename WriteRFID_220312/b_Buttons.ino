

/*
 * Similar to readButtons, but would wait in the infinite loop until any of 
 * the buttons is pressed, and will return value of the 
 * button pressed (BUTTON1..BUTTON4), or bitmask for multiple buttons pressed at once 
 * Use this function when it is required to wait for 
 * user input, and there is nothing to do until user press some button
*/ 
uint8_t waitButton() {
  for (;;) { 
    uint8_t ret = readButtons();                            // read all button states
    if (ret != 0) {
      delay(50);
      while (readButtons() != 0) delay(1);                  // and now wait for him to release the button
      return ret;                                           // finally return value of the button he pressed moments ago
      }
    delay(1);
  }
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
    return 0;                                         // all are high - no buttons pressed 
  }
  delay(2);                                           // delay for level de-bouncing; 2nd read to confirm low
  if (digitalRead(button1Pin) == LOW)
    ret |= BUTTON1;                                   // set leftmost bit in the 'ret' to 1

  if (digitalRead(button2Pin) == LOW)
    ret |= BUTTON2;                                   // send 2nd leftmost bit in the 'ret' to 1

  if (digitalRead(button3Pin) == LOW)
    ret |= BUTTON3;                                   // etc...

  if (digitalRead(button4Pin) == LOW){
    ret |= BUTTON4;
  }
  return ret;
}

uint8_t readButton(int pin) {
  if (digitalRead(pin) == HIGH) return 0; 
  delay(5);                                 // only reached if pin goes LOW, de-bounce delay, confirm read
  return (digitalRead(pin) == LOW) ? 1 : 0;
}

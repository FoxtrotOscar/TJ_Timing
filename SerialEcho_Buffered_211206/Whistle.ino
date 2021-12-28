/* 
Non-blocking whistle control 
*/



bool checkWhistleStatus(){                      // are we still counting down whistles?
  if ((digitalRead(whistlePin) == HIGH) &&      // Whistle blowing and not last one
      ((millis() - whistleClock)                 
      > (1000 / numWhistles)))  {               // and is the clock exhausted for this one
    digitalWrite(whistlePin, LOW);              // Turn it off
    whistleClock = millis();                    // reset the clock
    n -= 1;                                     // give diminishing value of n
    
  } else if ((digitalRead(whistlePin) == LOW)   // Whistle off and not last one                   
            && ((millis() - whistleClock)            
            > (1200 / (numWhistles + 1)))) {    // and is the clock exhausted for this one
    digitalWrite(whistlePin, HIGH);             // start the whistle  
    whistleClock = millis();                    // reset the clock
  }
  return n;                                     // Still running? False if n == 0
}

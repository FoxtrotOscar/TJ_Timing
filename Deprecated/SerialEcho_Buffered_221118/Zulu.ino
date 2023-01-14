// void showParam(bool newParam){
//   char endMarker = '\n';
//   char rc        = 0;

//   MATRIXSER.print("font 6\r");                      
//   delay(2*tock);                                  
//   clearMatrix(false);
//   sendSerialS(2, 3, 20,(newParam ?  "CHANNEL: " : "BAUD"));
//   command_ON(true);
//   //while(Serial.available()) Serial.read();        // empty  out possible garbage
//   colNumber = 46;
//   byte pMe = 2;
//   for (;;) {
//     byte ptr = 0;
//     bool read_error  = false;    
//     bool flag        = false;
//     newParam ? Serial.print("AT+RC\n") : Serial.print("AT+RB\n") ;
//     Serial.flush();
//     do {} while (!Serial.available());
//     while (Serial.available()) {        
//       while (!flag && !read_error){               // typical response: "OK+C021"
//         rc = Serial.read();                         // keep reading until '+' returns
//         pauseMe(pMe);  
//         //testMe(1);      
//         flag  = (rc == '+');                      // means we have a good response, exit loop
//         read_error = (rc == 'E');                 // means we have a bad  response, exit loop
//       }
//       if (read_error) {
//         testMe(6, 10); 
//         break;
//       }                      // exit to next iteration of (;;)
//       delay(2);
//       rc = Serial.read();                           // get the next char
//       pauseMe(pMe);
//       if (rc && (rc != endMarker)) {              // grab max 3 chars from HC12 until end marker
//         if ((rc >= '0') && (rc <= '9') && (ptr < 3)){
//           rc = (rc - '0');
//           sendNumber(green, colNumber, 20, rc);
//           colNumber +=5;
//           ptr++;
//         }
//       }
//     }
//     if (!read_error) break;                       // no errors - we are done, else go back to (;;)
//   }
  
//   digitalWrite (setupPin, HIGH);                  // PC15: set inactive
//   //testMe(2,10);                                    // start the whistle                              
//}
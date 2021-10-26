/*
   These four functions are for writing the
   Detail identifier during the count process
*/

void sendDetail(bool cdHigh) {

  clearMatrix();

  if (paramStore.Details == 2) {                          // |Double detail, alternating?
    if (!shootDetail && (sE_iter % 4 == 1)) {             // |
      writeA_B(false);                                    // |
    } else {                                              // |
      if (shootDetail  && (sE_iter % 4 == 2)) {           // |if so, set the correct
        writeC_D(false);                                  // |
      } else {                                            // |
        if (!shootDetail && (sE_iter % 4 == 3)) {         // |detail identifier on the screen
          writeC_D(cdHigh);                               // |
        } else {                                          // |
          writeA_B(false);                                // |
        }
      }
    }
  }
}

void writeA_B(bool cdHigh) {
  HC12.print(F("font 9\r"));                              // font 12 high double
  HC12.flush();
  lnNumber = 12;                                          // Write A & B
  sendSerialS(2, 1,  lnNumber, "A");
  sendSerialS(2, 56, lnNumber, "B");
}

void writeC_D(bool cdHigh) {
  HC12.print(F("font 9\r"));
  HC12.flush();
  lnNumber = (cdHigh ? 12 : 30);
  sendSerialS(2,  1, lnNumber, "C");                       // Write C & D
  sendSerialS(2, 56, lnNumber, "D");
}

void writeArcher(uint8_t which) {
  HC12.print(F("font 9\r"));    HC12.flush();
  switch (which) {
    case 0:
      break;
            // sendSerialS(/*colour(R1G2O3)=*/, /*column=*/, /*line=*/, i0);
    case 1:
      sendSerialS(2, 0, 22, "A");                    //  A <<
      sendSerialS(2, 8, 22, "<<");
                                 
      break;

    case 2:
      sendSerialS(2, 57, 22, "B");                    //  B >>
      sendSerialS(2, 40, 22, ">>");
      break;
  }
}

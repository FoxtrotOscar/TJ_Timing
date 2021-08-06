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
  HC12.print(F("font 9\r"));                                // font 12 high double
  HC12.flush();
  lnNumber = 12;                                            // Write A & B

  i0 = "A";
  sendSerialS(/*colour(R1G2O3)=*/ 2,
                                  /*column=*/ 1,
                                  /*line=*/ lnNumber,
                                  i0, 0, 0);
  i0 = "B";
  sendSerialS(/*colour(R1G2O3)=*/ 2,
                                  /*column=*/ 56,
                                  /*line=*/ lnNumber,
                                  i0, 0, 0);
}

void writeC_D(bool cdHigh) {
  HC12.print(F("font 9\r"));
  HC12.flush();
  lnNumber = (cdHigh ? 12 : 30);
  // Write C & D
  i0 = "C";
  sendSerialS(/*colour(R1G2O3)=*/ 2,
                                  /*column=*/ 1,
                                  /*line=*/ lnNumber,
                                  i0, 0, 0);
  i0 = "D";
  sendSerialS(/*colour(R1G2O3)=*/ 2,
                                  /*column=*/ 56,
                                  /*line=*/ lnNumber,
                                  i0, 0, 0);
}

void writeArcher(uint8_t which) {
  HC12.print(F("font 9\r"));    HC12.flush();
  switch (which) {
    case 0:
      break;

    case 1:
      i0 = "A";
      sendSerialS(/*colour(R1G2O3)=*/ 2,
                                      /*column=*/ 0,
                                      /*line=*/ 22,
                                      i0, 0, 0);                    //  A <<
      i0 = "<<";
      sendSerialS(/*colour(R1G2O3)=*/ 2,
                                      /*column=*/ 8,
                                      /*line=*/ 22,
                                      i0, 0, 0);
      break;

    case 2:
      i0 = "B";
      sendSerialS(/*colour(R1G2O3)=*/ 2,
                                      /*column=*/ 57,
                                      /*line=*/ 22,
                                      i0, 0, 0);                    //  B >>
      i0 = ">>";
      sendSerialS(/*colour(R1G2O3)=*/ 2,
                                      /*column=*/ 40,
                                      /*line=*/ 22,
                                      i0, 0, 0);
      break;
  }
}

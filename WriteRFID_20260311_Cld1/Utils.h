#pragma once
#include <Arduino.h>

void dump_byte_array(byte *buffer, byte bufferSize);
bool isCardPresentNow();
void endCardSession();
void clearFromLine(uint8_t lineNo);
void showAllParams(byte index);
void showOneParam(byte ct, byte index);
void showAllParamsNoWipe(byte index);
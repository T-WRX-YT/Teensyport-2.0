// include/DataUtils.h
#ifndef DATAUTILS_H
#define DATAUTILS_H

#include <Arduino.h>

// Declare all calc functions
void processOil (char *t);
float calcAfr(unsigned char data);
float calcFloatFull(unsigned char data[4], float multiplier);
int calcIntFull(unsigned char data[2], float multiplier);
int calcTemp(unsigned char data);
float calcByteToFloat(unsigned char data, float multiplier);
float calcTargetBoost(unsigned char data[2]);
float calcThrottle(unsigned char data);
float calcAfCorrection(unsigned char data);
int calcByteToInt(unsigned char data);
int calcAvcs(unsigned char data);
float calcTiming(unsigned char data);
float calcInjDutyCycle(unsigned char data);

// Optional: shared union for reuse
union floatUnion {
  byte byteArray[4];
  float floatValue;
  int intValue;
};

#endif
#include <Arduino.h>
#include "ILI9341_t3n.h"

#ifndef CANSENDER_H
#define CANSENDER_H



void sendLargeRequest();
void sendSmallRequest();
void sendNewRequest();
void sendFlow();
void sendMessage(const unsigned char data[8]);




#endif
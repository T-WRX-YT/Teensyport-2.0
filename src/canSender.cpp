#include "canSender.h"
#include "globalVars.h"
#include <FlexCAN_T4.h>


// yes this should just be a loop - sue me


// sends the new standlone request
void sendNewRequest() {
  sendMessage(newReq1);
  delay(10);  // assumed to be getting the 0x30 flow control here, give a little delay to receive
  sendMessage(newReq2);
  sendMessage(newReq3);
  sendMessage(newReq4);
  sendMessage(newReq5);
  sendMessage(newReq6);
  sendMessage(newReq7);
  sendMessage(newReq8);
  sendMessage(newReq9);
  delay(2);  // the ECU likes a little delay after youve sent 9 packets before the next one
  sendMessage(newReq10);
  delay(5);  // give a little delay after the last packet, the ecu should be sending a 0x10 back, then send the 0x30 flow to tell it i want more
  sendFlow();
}

// sends the large request that mimics my AP datalogging mode, not really used for anything currently but i typed it so im not deleting it
void sendLargeRequest() {
  
  sendMessage(longReq1);
  delay(10);
  sendMessage(longReq2);
  sendMessage(longReq3);
  sendMessage(longReq4);
  sendMessage(longReq5);
  sendMessage(longReq6);
  sendMessage(longReq7);
  sendMessage(longReq8);
  sendMessage(longReq9);
  sendMessage(longReq10);
  sendMessage(longReq11);
  sendMessage(longReq12);
  sendMessage(longReq13);
  sendMessage(longReq14);
  sendMessage(longReq15);
  sendMessage(longReq16);
  sendMessage(longReq17);
  sendMessage(longReq18);
  sendMessage(longReq19);
  sendMessage(longReq20);
  sendMessage(longReq21);
  sendMessage(longReq22);
  sendMessage(longReq23);
  sendMessage(longReq24);
  sendMessage(longReq25);
  sendMessage(longReq26);
  sendMessage(longReq27);
  delay(10);
  sendFlow();
}

// sends the regular 6 gauge display request for normal operation, not used anymore
void sendSmallRequest() {
  sendMessage(req1);
  delay(10);
  sendMessage(req2);
  sendMessage(req3);
  sendMessage(req4);
  sendMessage(req5);
  sendMessage(req6);
  sendMessage(req7);
  sendMessage(req8);
  delay(10);
  sendFlow();
}




void sendFlow() {
    CAN_message_t msg;
    msg.id = 0x7E0;

    for (int i = 0; i < 8; i++) {
      msg.buf[i] = req0[i];
    }

    if (verbose) { Serial.println("[VERBOSE] Sending flow message"); }
    Can0.write(msg);
}

void sendMessage(const unsigned char data[8]) {
    CAN_message_t msg;
    msg.id = 0x7E0;


      if (verbose) { Serial.print("Sending message: "); }
      for (int i = 0; i < 8; i++) {
        msg.buf[i] = data[i];
        if (verbose) { Serial.print(msg.buf[i], HEX); }
        if (verbose) { Serial.print(" "); }
      }
      //if (verbose) { Serial.println(); }
      Can0.write(msg);
      if (verbose) { Serial.println("  Sent"); }
}

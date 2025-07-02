#include <FlexCAN_T4.h>
#include "canParser.h"
#include "globalVars.h"
#include "dataParser.h"

// new hotness, closs enough to isotp to count
void canSniffIso(const CAN_message_t &msg) {
    Serial.println("******************* ENTERING CANSNIFISO");
    if (verbose) {
    Serial.print("[VERBOSE] MB "); Serial.print(msg.mb);
    Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
    Serial.print("  LEN: "); Serial.print(msg.len);
    Serial.print(" EXT: "); Serial.print(msg.flags.extended);
    Serial.print(" TS: "); Serial.print(msg.timestamp);
    Serial.print(" ID: "); Serial.print(msg.id, HEX);
    Serial.print(" IDD: "); Serial.print(msg.id);
    Serial.print(" Buffer: ");
    for ( uint8_t i = 0; i < msg.len; i++ ) {
      Serial.print(msg.buf[i], HEX); Serial.print(" ");
    } Serial.println();
  }

  // detection for something else requesting data, this will turn off sending to prevent collisions with the ecu
  if ((msg.id == 0x7E0) && (ssmActive == 1)) {
    ssmActive = 0;
    Serial.println("Switching ssm active to 0");
  }

  // only parse 7E8, this will keep the 7E0 monitor from interferring with the data
  if (msg.id == 0x7E8) {
    if (msg.buf[0] == 0x10) {  // 0x10 is the first packet in a new set of response data
        //zero out the data for 0x10 new response
        for (uint8_t i = 0; i < 61; i++) {
          responseData[i] = 0x00;
        }

        responseBytes = msg.buf[1] - 1; // read the 2nd byte of the response - how much data to expect.  subtract 1 to not count the response code
        if (responseBytes == 0x11) {
          displayMode = displayModeNormal;  // switch to 6 guage mode.  this is an ssm passive mode so no extra logic needed
          responseType = 0;
        }
        else if (responseBytes == 0x3D) {
          displayMode = displayModeLogging;  // switch to logging mode.  this is an ssm passive mode so no extra logic needed
          responseType = 1;
        }
        else if (responseBytes == 0x15) {  // handle the standalone mode.  this will handle standalone active mode
          responseType = 2;
          if (logger) {
            displayMode = displayModeLogging;
          }
          else {
            displayMode = displayModeNormal;
          }
        }
        else {
          displayMode = 0;
          responseType = 99;
          Serial.println(responseBytes, HEX);
        }

        // write the last 5 bytes of the response to the beginning of the array
        for (uint8_t i = 0; i < 5; i++) {
          responseData[byteCount] = msg.buf[i+3];  // byte count starts at 0 here for first run, tracking the array index
          byteCount++;
        }
        packetCount++;
    } // finished with 0x10

    // 0x30 received a continue from the first request byte, that means the last message received was the end of the data 
    else if (msg.buf[0] == 0x30) {
        
        if (printStats) {
            // responseBytes are the returned value (array might have more), this will be the entire response pure data
            if (verbose) {
              for (int z = 0; z < responseBytes; z++) {
                  Serial.print(responseData[z], HEX);
                  Serial.print(" ");
              }
              Serial.println();
            }
            

            timer = millis();
            char header[64];
            sprintf(header, "[PRINTSTATS] %d.%03d 30 | FB: ", (int)(timer/1000), (int)(timer % 1000)); Serial.print(header); Serial.print(feedbackKnockFinal);
            //Serial.print("[PRINTSTATS] 30 | FB: "); Serial.print(feedbackKnockFinal);
            Serial.print(" FN: "); Serial.print(fineKnockFinal);
            Serial.print(" BST: "); Serial.print(boostFinal);
            Serial.print(" COOL: "); Serial.print(coolantFinal);
            Serial.print(" DAM: "); Serial.print(damFinal);
            Serial.print(" INTAKE: "); Serial.print(intakeTempFinal);
            Serial.print(" OIL T: "); Serial.print(oilTemperature);
            Serial.print(" OIL P: "); Serial.println(oilPressure);
        }

        flowCont = 1;
        for (int i = 1; i < 8; i++) {
            if (msg.buf[i] != 0x00) { flowCont = 0; }   // the response wasnt all zeroes, set flow continue to 0 to stop trying to send more messages
        }
        if (verbose) {
            if (flowCont) { Serial.println("[VERBOSE] **************** FLOW CONTINUE RECEIVED"); }
            else { Serial.println("[VERBOSE] !!!!!!!!!!!!!!!!!!!!!!!!!!! FLOW ERROR RECEIVED"); }
        }



        if (verbose) {
          Serial.print("Parsed ");
          Serial.print(packetCount);  // each 0x## message parsed
          Serial.print(" packets in this response and ");
          Serial.print(byteCount);    // each data byte added to the responseData array, should all be in order pure data
          Serial.print(" bytes.  ECU says I should have gotten ");
          Serial.print(responseBytes);    // the number of bytes the 0x10 response said to have, -1 for the response code
          Serial.print(" bytes to process.  Response type: ");
          Serial.println(responseType);


          for (int i = 0; i < responseBytes; i++) {
            Serial.print(responseData[i], HEX);
          }
          Serial.println();

        }

        // do work on the final data here.  responseData now has the entire response, array indexes depend on the order of your request
        // ap 6 gauge mode
        if (responseType == 0) {
          unsigned char feedbackKnockData[4] = {responseData[3], responseData[2], responseData[1], responseData[0]};
          feedbackKnockFinal = calcFloatFull(feedbackKnockData, 1);

          unsigned char fineKnockData[4] = {responseData[7], responseData[6], responseData[5], responseData[4]};
          fineKnockFinal = calcFloatFull(fineKnockData, 1);

          unsigned char boostData[4] = {responseData[11], responseData[10], responseData[9], responseData[8]};
          boostFinal = calcFloatFull(boostData, 0.01933677);

          unsigned char rpmData[2] = {responseData[13], responseData[12]};
          rpmFinal = calcIntFull(rpmData, .25);

          coolantFinal = calcTemp(responseData[14]);
          damFinal = calcByteToFloat(responseData[15], 0.0625);
          intakeTempFinal = calcTemp(responseData[16]);
        }
        // ap logging logging mode
        else if (responseType == 1) {
          unsigned char feedbackKnockData[4] = {responseData[31], responseData[30], responseData[29], responseData[28]};
          feedbackKnockFinal = calcFloatFull(feedbackKnockData, 1);

          unsigned char fineKnockData[4] = {responseData[27], responseData[26], responseData[25], responseData[24]};
          fineKnockFinal = calcFloatFull(fineKnockData, 1);

          unsigned char boostData[4] = {responseData[7], responseData[6], responseData[5], responseData[4]};
          boostFinal = calcFloatFull(boostData, 0.01933677);

          unsigned char rpmData[2] = {responseData[33], responseData[32]};
          rpmFinal = calcIntFull(rpmData, .25);

          coolantFinal = calcTemp(responseData[42]);
          damFinal = calcByteToFloat(responseData[53], 0.0625);
          intakeTempFinal = calcTemp(responseData[43]);
        }
        else if (responseType == 2) {
          if (verbose) { Serial.println("[VERBOSE] Sending feedbackKnock"); }
          unsigned char feedbackKnockData[4] = {responseData[3], responseData[2], responseData[1], responseData[0]};
          feedbackKnockFinal = calcFloatFull(feedbackKnockData, 1);

          if (verbose) { Serial.println("[VERBOSE] Sending fineKnock"); }
          unsigned char fineKnockData[4] = {responseData[7], responseData[6], responseData[5], responseData[4]};
          fineKnockFinal = calcFloatFull(fineKnockData, 1);

          if (verbose) { Serial.println("[VERBOSE] Sending boost"); }
          unsigned char boostData[4] = {responseData[11], responseData[10], responseData[9], responseData[8]};
          boostFinal = calcFloatFull(boostData, 0.01933677);

          if (verbose) { Serial.println("[VERBOSE] Sending rpm"); }
          unsigned char rpmData[2] = {responseData[13], responseData[12]};
          rpmFinal = calcIntFull(rpmData, .25);

          if (verbose) { Serial.println("[VERBOSE] Sending coolant"); }
          coolantFinal = calcTemp(responseData[14]);
          if (verbose) { Serial.println("[VERBOSE] Sending dam"); }
          damFinal = calcByteToFloat(responseData[15], 0.0625);
          if (verbose) { Serial.println("[VERBOSE] Sending intakeTemp"); }
          intakeTempFinal = calcTemp(responseData[16]);
          if (verbose) { Serial.println("[VERBOSE] Sending gear"); }
          gearFinal = calcByteToInt(responseData[17]);
          if (verbose) { Serial.println("[VERBOSE] Sending speed"); }
          speedFinal = calcByteToFloat(responseData[18], 0.621371192);
          if (verbose) { Serial.println("[VERBOSE] Sending afr"); }
          afrFinal = calcAfr(responseData[19]);
          if (verbose) { Serial.println("[VERBOSE] Sending throttle"); }
          throttleFinal = calcThrottle(responseData[20]);
        }
        else {
          // something went wrong here :(
          Serial.println("****************************** HIT THE ELSE IN RESPONSETYPE");
        }


        packetCount = 0;    // next message is going to be the 0x10 starting the next response
        byteCount = 0;
    } // finished with 0x30

    // else it is a continuous byte IE: not 10 or 30, assumed to be in order
    else {   
        for (uint8_t i = 1; i < 8; i++) {
            responseData[byteCount] = msg.buf[i];  // write the values to the array, bytecount global tracks the position 
            byteCount++;
        }
      packetCount++;
    } // finished with continuous message
  } // finished with the 7E8 ID message
}
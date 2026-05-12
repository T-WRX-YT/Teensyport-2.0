// teensy 2.1

#include <Arduino.h>
#include <FlexCAN_T4.h>
#include "SPI.h"
#include "ILI9341_t3n.h"
#include <Adafruit_GPS.h>
#include "config.h"

/* CAN BUS STATE */
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;

unsigned char responseData[RESPONSE_DATA_MAX];
uint8_t packetCount = 0;
uint8_t byteCount = 0;
uint8_t responseBytes;
uint8_t responseType;
bool flowCont = 1;

unsigned long lastTime = 0;
unsigned long count = 0;
unsigned long lastCanTime = 0;

/* GPS */
#define GPSSerial Serial5
Adafruit_GPS GPS(&GPSSerial);

/* GAUGE DATA */
float feedbackKnockFinal;
float feedbackMax = 0;
float fineKnockFinal;
float fineMax = 0;
uint16_t fineRpmMin = 9999;
uint16_t fineRpmMax = 0;
float boostFinal;
int16_t coolantFinal;
float damFinal;
int16_t intakeTempFinal;
uint16_t rpmFinal;
uint8_t gearFinal;
uint8_t speedFinal;
float afrFinal;
uint8_t throttleFinal;
unsigned long timer;
unsigned int logger;

/* RPM BAR VARIABLES */
uint16_t yellowMin, yellowMinPx, yellowMax, yellowMaxPx, yellowFill, redMin, redMinPx, revLimitPx, redMax, redMaxPx, redFill;

/* PERIPHERALS */
char buf[10];
int16_t oilTemperature, oilPressure, diffTemperature;
uint8_t dccdPercent;

void updateAllBufferAsync();

ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST);

union floatUnion {
  byte byteArray[4];
  float floatValue;
  int intValue;
};

/* RUNTIME STATE */
bool ssmActive = 1;
unsigned int updateHz;
unsigned int displayMode = 3;



/* FUNCTION DECLARATION FOR PLATFORMIO */
void sendMessage(const unsigned char data[8]);
void sendFlow();
float calcAfr(unsigned char data);
void canSniffIso(const CAN_message_t &msg);
float calcFloatFull(unsigned char data[4], float multiplier);
int calcIntFull(unsigned char data[2], float multiplier);
int calcTemp(unsigned char data);
float calcByteToFloat(unsigned char data, float multiplier);
void processOil (char *t);
void sendSmallRequest();
void sendNbp();
void sendNewRequest();
int calcByteToInt(unsigned char data);
float calcThrottle(unsigned char data);
void sendEsp();
void readGps();
/* FUNCTION DECLARATION FOR PLATFORMIO */


void setup(void) {
  
  // init hardware serial output for physical usb
  Serial.begin(SERIAL_BAUD);
  delay(100);
  if (!(testData)) { HWSERIAL.begin(OIL_SERIAL_BAUD); } // this inits the serial to arduino if not in test mode
  if (sendToEsp) { delay(100); HWSERIAL3.begin(ESP_SERIAL_BAUD); } // this inits the serial3 connection to the esp
  if (gpsConnected) {
    delay(100);
    GPS.begin(9600); //  init the serial5 connection for the gps module
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
    GPS.sendCommand(PGCMD_ANTENNA);   // request antenna status reports  
  }
  delay(100);
  tft.begin();
  delay(100);
  //tft.setRotation(2);

  tft.fillScreen(ILI9341_BLACK);
  //if (!(testData)) { delay(5000); }
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(2);
  Serial.println(CORE_PIN10_CONFIG, HEX);
  tft.println("INIT");
  tft.println("SERIAL STARTED");

  tft.println();
  tft.println();
  tft.println();
  tft.setTextSize(2);

  tft.println("INIT CAN");
  Serial.println("Starting can");
  Can0.begin();
  Can0.setClock(CLK_60MHz); // MOAR POWAHHH
  Can0.setBaudRate(CAN_BAUD_RATE);
  Can0.setMaxMB(16);
  Can0.enableFIFO();
  Can0.enableFIFOInterrupt();
  Can0.onReceive(canSniffIso);

  Can0.setFIFOFilter(REJECT_ALL);
  Can0.setFIFOFilter(0, CAN_RESPONSE_ID, STD);
  Can0.setFIFOFilter(1, CAN_REQUEST_ID, STD);

  Can0.mailboxStatus();
  tft.println("WAITING FOR CAN MSG");
  //if (!(testData)) { delay(10000); }
  if (testData) { 
    ssmActive = 0;
    flowCont = 0; 
  }  // turn off SSM active is test data is on, no need for this

  pinMode(BUTTON1, INPUT_PULLUP);
  //pinMode(21, OUTPUT);
  //digitalWrite(21, HIGH);
  //delay(5000);


  //Serial3.println("init");

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);

  lastTime = millis();

}




// new hotness, closs enough to isotp to count
void canSniffIso(const CAN_message_t &msg) {

    /*
    Serial.print("Received: ");
    Serial.print(msg.id, HEX);
    Serial.print(" ");
    for ( uint8_t i = 0; i < msg.len; i++ ) {
      Serial.print(msg.buf[i], HEX); Serial.print(" ");
    } 
    Serial.println();
    */




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
  if ((msg.id == CAN_REQUEST_ID) && (ssmActive == 1)) {
    ssmActive = 0;
    Serial.println("Switching ssm active to 0");
  }

  // only parse 7E8, this will keep the 7E0 monitor from interferring with the data
  if (msg.id == CAN_RESPONSE_ID) {
    if (msg.buf[0] == 0x10) {
        //zero out the data for 0x10 new response
        for (uint8_t i = 0; i < sizeof(responseData); i++) {
          responseData[i] = 0x00;
        }

        responseBytes = msg.buf[1] - 1; // read the 2nd byte of the response - how much data to expect.  subtract 1 to not count the response code
        if (responseBytes == RESP_BYTES_AP6) {
          displayMode = displayModeNormal;  // switch to 6 guage mode.  this is an ssm passive mode so no extra logic needed
          responseType = RESP_TYPE_AP6;
        }
        else if (responseBytes == RESP_BYTES_AP_LOG) {
          displayMode = displayModeLogging;  // switch to logging mode.  this is an ssm passive mode so no extra logic needed
          responseType = RESP_TYPE_AP_LOG;
        }
        else if (responseBytes == RESP_BYTES_STANDALONE) {  // handle the standalone mode.  this will handle standalone active mode
          responseType = RESP_TYPE_STANDALONE;
          if (!(logger)) {
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
        //Serial.println("*************************** RECEIVED FLOW, SETTING FLOWRCV TO 1");
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



        //if (verbose) {
          //  send data after a successful update
          if (packetCount == 4 && byteCount == 26) {
          lastCanTime = millis();
          /*
          Serial.print("Parsed ");
          Serial.print(packetCount);  // each 0x## message parsed
          Serial.print(" packets in this response and ");
          Serial.print(byteCount);    // each data byte added to the responseData array, should all be in order pure data
          Serial.print(" bytes.  ECU says I should have gotten ");
          Serial.print(responseBytes);    // the number of bytes the 0x10 response said to have, -1 for the response code
          Serial.print(" bytes to process.  Response type: ");
          Serial.println(responseType);
          */
           if (!(testData)) { 
            sendEsp();
            updateAllBufferAsync();
           }
          }
          else {
          Serial.print("--------------------------Parsed ");
          Serial.print(packetCount);  // each 0x## message parsed
          Serial.print(" packets in this response and ");
          Serial.print(byteCount);    // each data byte added to the responseData array, should all be in order pure data
          Serial.print(" bytes.  ECU says I should have gotten ");
          Serial.print(responseBytes);    // the number of bytes the 0x10 response said to have, -1 for the response code
          Serial.print(" bytes to process.  Response type: ");
          Serial.println(responseType);
          }

              //for (int z = 0; z < 8; z++) {
              //    Serial.print(msg.buf[z], HEX);
              //    Serial.print(" ");
              //}
              //Serial.println();

          //for (int i = 0; i < responseBytes; i++) {
          //  Serial.print(responseData[i], HEX);
          //}
          //Serial.println();

        //}

        // do work on the final data here.  responseData now has the entire response, array indexes depend on the order of your request
        // ap 6 gauge mode
        if (responseType == RESP_TYPE_AP6) {
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
        else if (responseType == RESP_TYPE_AP_LOG) {
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
        else if (responseType == RESP_TYPE_STANDALONE) {
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
          //if (verbose) { Serial.println("[VERBOSE] Sending brake"); }
          //unsigned char brakeData[2] = {responseData[22], responseData[21]};
          //brakeFinal = ((calcIntFull(brakeData, 37)) / 255);
        }
        else {
          // something went wrong here :(
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




void loop() {  
  
  //Serial.println(digitalRead(BUTTON1));
  
  
  // some crazy stuff i found on the internet.  how i receive and parse two integers at once via serial from an arduino
  if (!(testData)) {
    if (HWSERIAL.available ()) {
      char buf [80];
      int n = HWSERIAL.readBytesUntil ('\n', buf, sizeof(buf));
      // check for a real value.  weird things happen if the logic converter is connected but nothing is sending
      if ((n > 1) && (n < 30)) {
        buf [n] = '\0';     // terminate with null
        if (verbose) { Serial.println("[VERBOSE] Serial Received"); }

        char *t = strtok (buf, ",");
        processOil (t);
        while ((t = strtok (NULL, ",")))
            processOil (t);
      }
    }  // finished parsing arduino oil data
  }  // testData condition end
  
  

  // if active is still set, send the entire small request.  since flexcan runs on interrupts, this can run in the loop and still hit the cansniffiso parsing
  // if this is not set, cansniffiso will still work in case an AP is plugged in
  if (ssmActive) {
    if (flowCont) { sendNewRequest(); }
  }

  // bunch of stuff to just make up data to test how the screen looks
  if (testData) {
    for (int i = -40; i < 280; i++) {
      //unsigned long start = micros();
      coolantFinal = i;
      oilTemperature = i;
      intakeTempFinal = i;
      feedbackKnockFinal = (random(0,5) * 1.4) * -1;
      fineKnockFinal = (random(0,2) * 1.4) * -1;
      damFinal = 1;
      boostFinal = (random(-20,20) / 1.1 );
      oilPressure = (random(0,99));
      rpmFinal = random(0,6800);
      if (feedbackKnockFinal < feedbackMax) { feedbackMax = feedbackKnockFinal; }
      if (fineKnockFinal < fineMax) { fineMax = fineKnockFinal; }
      if (fineKnockFinal < 0) {
        if (rpmFinal > fineRpmMax) { fineRpmMax = rpmFinal; }
        if (rpmFinal < fineRpmMin) { fineRpmMin = rpmFinal; } 
      }
      gearFinal = random(1,5);
      speedFinal = random(0,150);
      afrFinal = (random(10,25) * 1.1);
      throttleFinal = random(0,100);

      if (printLoopStats) {
        timer = millis();
        char header[64];
        sprintf(header, "[PRINTLOOPSTATS] %d.%03d FB: ", (int)(timer/1000), (int)(timer % 1000)); Serial.print(header); Serial.print(feedbackKnockFinal); 
        Serial.print(" FN: "); Serial.print(fineKnockFinal);
        Serial.print(" BST: "); Serial.print(boostFinal);
        Serial.print(" COOL: "); Serial.print(coolantFinal);
        Serial.print(" DAM: "); Serial.print(damFinal);
        Serial.print(" INTAKE: "); Serial.print(intakeTempFinal);
        Serial.print(" OIL T: "); Serial.print(oilTemperature);
        Serial.print(" OIL P: "); Serial.print(oilPressure);
        Serial.print(" DIFF T: "); Serial.print(diffTemperature);
        Serial.print(" DCCD: "); Serial.print((int)dccdPercent);
        Serial.print(" RPM: "); Serial.print(rpmFinal);
        Serial.print(" GEAR: "); Serial.print(gearFinal);
        Serial.print(" SPEED: "); Serial.print(speedFinal);
        Serial.print(" AFR: "); Serial.print(afrFinal);
        Serial.print(" THROTTLE: "); Serial.print(throttleFinal);
        Serial.print(" GPS FIX: "); Serial.print((int)GPS.fix);
        Serial.print(" SATS: "); Serial.print((int)GPS.satellites);
        Serial.print(" LAT: "); Serial.print(GPS.latitudeDegrees, 6);
        Serial.print(" LON: "); Serial.println(GPS.longitudeDegrees, 6);
      }


      if (!(logger)) {
        displayMode = displayModeLogging;
      }
      else {
        displayMode = displayModeNormal;
      }

      updateAllBufferAsync();
      logger = digitalRead(BUTTON1);  // reads the logging button value
      //if (logger == HIGH) { sendNbp(); }
      sendEsp();
      delay(updateInt);
      //updateHz = 1.0 / ((micros() - start) / 1000000.0);
    }
  }
  // else - you are getting real data from flexcan
  else {
    //unsigned long start = micros();
    if (feedbackKnockFinal < feedbackMax) { feedbackMax = feedbackKnockFinal; }
    if (fineKnockFinal < fineMax) { fineMax = fineKnockFinal; }
      if (fineKnockFinal < 0) {
        if (rpmFinal > fineRpmMax) { fineRpmMax = rpmFinal; }
        if (rpmFinal < fineRpmMin) { fineRpmMin = rpmFinal; } 
      }


    if (printLoopStats) {
      timer = millis();
      char header[64];
      sprintf(header, "[PRINTLOOPSTATS] %d.%03d FB: ", (int)(timer/1000), (int)(timer % 1000)); Serial.print(header); Serial.print(feedbackKnockFinal); 
      Serial.print(" FN: "); Serial.print(fineKnockFinal);
      Serial.print(" BST: "); Serial.print(boostFinal);
      Serial.print(" COOL: "); Serial.print(coolantFinal);
      Serial.print(" DAM: "); Serial.print(damFinal);
      Serial.print(" INTAKE: "); Serial.print(intakeTempFinal);
      Serial.print(" OIL T: "); Serial.print(oilTemperature);
      Serial.print(" OIL P: "); Serial.print(oilPressure);
      Serial.print(" DIFF T: "); Serial.print(diffTemperature);
      Serial.print(" DCCD: "); Serial.print((int)dccdPercent);
      Serial.print(" RPM: "); Serial.print(rpmFinal);
      Serial.print(" GEAR: "); Serial.print(gearFinal);
      Serial.print(" SPEED: "); Serial.print(speedFinal);
      Serial.print(" AFR: "); Serial.print(afrFinal);
      Serial.print(" THROTTLE: "); Serial.print(throttleFinal);
      Serial.print(" GPS FIX: "); Serial.print((int)GPS.fix);
      Serial.print(" SATS: "); Serial.print((int)GPS.satellites);
      Serial.print(" LAT: "); Serial.print(GPS.latitudeDegrees, 6);
      Serial.print(" LON: "); Serial.println(GPS.longitudeDegrees, 6);
    }

    //  no longer printing the screen after every main loop(), instead only printing and sending data after a successful 0x30 full message
    updateAllBufferAsync();  // also update here so display shows arduino data when no CAN connection
    logger = digitalRead(BUTTON1);  // reads the logging button value
    //if (logger == HIGH) { sendNbp(); }
    //sendEsp();
    delay(updateInt);
    //updateHz = 1.0 / ((micros() - start) / 1000000.0);
  }

    //  reports on how many times per second the data is being updated
    if (millis() - lastTime >= 1000) {
      Serial.print("Function called ");
      Serial.print(count);
      Serial.println(" times in the last second.");
      updateHz = count;

      count = 0;
      lastTime = millis();
    }

    readGps();
}

void readGps() {
  // drain whatever bytes have arrived since the last loop pass
  while (GPS.read()) {}

  // if a full NMEA sentence is buffered, parse it — this updates
  // GPS.latitude, GPS.longitude, GPS.fix, GPS.speed, GPS.antenna, etc.
  if (GPS.newNMEAreceived()) {
    GPS.parse(GPS.lastNMEA());
  }

  // current values are now live in the GPS object, e.g.:
  //   (int)GPS.antenna     -> 1=no antenna, 2=internal, 3=external
  //   GPS.fix              -> 0/1
  //   GPS.latitude/.longitude, GPS.speed (knots), GPS.satellites
}



void sendEsp() {
  if (sendToEsp) {
    count++;
    //unsigned long start = micros();
    int nums[8] = {coolantFinal, intakeTempFinal, rpmFinal, gearFinal, speedFinal, throttleFinal, oilTemperature, oilPressure};
    float floats[5] = {feedbackKnockFinal, fineKnockFinal, boostFinal, damFinal, afrFinal};

    for (int z = 0; z < 8; z++) {
      Serial3.print(nums[z]);
      //Serial.print(nums[z]);
      Serial3.print(",");
      //Serial.print(",");
    }
    for (int z = 0; z < 5; z++) {
      Serial3.print(floats[z]);
      //Serial.print(floats[z]);
      if (z < 4) {
        Serial3.print(",");
        //Serial.print(",");
      }
    }

    Serial3.print("\n");
    //Serial.print("\n");
    //updateHz = 1.0 / ((micros() - start) / 1000000.0);
    //Serial.println(updateHz);
  }
}


// found on stack, wanky way of converting a string containing two integers received via serial into two different ints.  this sucks
void processOil (char *t) {
  char c;
  int  val;
  sscanf (t, "%c%d", &c, &val);

  switch (c) {
  case 'a':
      if (verbose) {
        Serial.print ("[VERBOSE] cmd a ");
        Serial.println (val);
      }
      oilTemperature = (val);
      break;

  case 'b':
      if (verbose) {
        Serial.print ("[VERBOSE] cmd b ");
        Serial.println (val);
      }
      // write the second value as oil pressure
      if (val < 0) { val = 0; }
      oilPressure = (val);
      break;

  case 'c':
      if (verbose) {
        Serial.print ("[VERBOSE] cmd c ");
        Serial.println (val);
      }
      diffTemperature = (val);
      break;

  case 'd': {
      float fval;
      sscanf (t, "%c%f", &c, &fval);
      if (verbose) {
        Serial.print ("[VERBOSE] cmd d ");
        Serial.println (fval);
      }
      int pct = (int)((fval / 5.0f) * 100.0f);
      if (pct < 0)   { pct = 0; }
      if (pct > 100) { pct = 100; }
      dccdPercent = (uint8_t)pct;
      break;
  }

  default:
      //if (verbose) {
        Serial.print ("[VERBOSE] unknown cmd ");
        Serial.println (val);
      //}
      break;
  }
}

float calcFloatFull(unsigned char data[4], float multiplier) {
  	if (verbose) {
      Serial.print("[VERBOSE] Input data: ");
      for(int z = 0; z < 4; z++) {
        Serial.print(data[z], HEX);
      }
      Serial.println(); 
    }
  
  
	floatUnion converter; 
  
    for (int i = 0; i < 4; i++) {
      converter.byteArray[i] = data[i];
    }
  
  	float calc = converter.floatValue * multiplier;
  	return calc;  	
}

int calcTemp(unsigned char data) {
  	if (verbose) {
      Serial.print("[VERBOSE] Input data: ");
      Serial.println(data, HEX);
    }

	int calc = 32+((9*((data)-40))/5);
	return calc;		
}

float calcByteToFloat(unsigned char data, float multiplier) {
  	if (verbose) {
      Serial.print("[VERBOSE] Input data: ");
      Serial.println(data, HEX);
    }

	float calc = data * multiplier;
	return calc;
}

int calcIntFull(unsigned char data[2], float multiplier) { 	
  	if (verbose) {
      Serial.print("[VERBOSE] Input data: ");
      for(int z = 0; z < 2; z++) {
        Serial.print(data[z], HEX);
      }
      Serial.println();
    }

	floatUnion converter; 
  
    for (int i = 0; i < 2; i++) {
      converter.byteArray[i] = data[i];
    }
  
  	int calc = converter.intValue * multiplier;
  	return calc;
}


float calcTargetBoost(unsigned char data[2]) {
  if (verbose) {	
    Serial.print("[VERBOSE] Input data: ");
  	  for(int z = 0; z < 2; z++) {
  		  Serial.print(data[z], HEX);
      }
  	  Serial.println();
  }
  
	floatUnion converter; 

  converter.byteArray[0] = data[0];
  converter.byteArray[1] = data[1];
  converter.byteArray[2] = 0x00;
  converter.byteArray[3] = 0x00;

  float calc = converter.intValue;
  calc = (calc - 760) * 0.01933677;
	return calc;	
}


int calcAvcs(unsigned char data) {
  if (verbose) {
    Serial.print("[VERBOSE] Input data: ");
  	Serial.println(data, HEX);
  	Serial.println();
  }

	int calc = data - 50;
	return calc;
}

float calcTiming(unsigned char data) {
  if (verbose) {
    Serial.print("[VERBOSE] Input data: ");
  	Serial.println(data, HEX);
  	Serial.println();
  }
	
	float calc = (data - 128) / 2;
	return calc;
}

int calcByteToInt(unsigned char data) {
  	if (verbose) {
      Serial.print("[VERBOSE] Input data: ");
      Serial.println(data, HEX);
    }

	int calc = data;
	return calc;
}

float calcAfCorrection(unsigned char (data)) {
  if (verbose) {
    Serial.print("[VERBOSE] Input data: ");
  	Serial.println(data, HEX);
  	Serial.println();
  }

	float calc = ((data - 128) * 100) / 128;
	return calc;
}

float calcThrottle(unsigned char data) {
  if (verbose) {
    Serial.print("[VERBOSE] Input data: ");
  	Serial.println(data, HEX);
  }
	
	float calc = (data * 100) / 255;
  //if (calc <= 14) { calc = 0; } // account for idle throttle position
	return calc;
}	

float calcInjDutyCycle(unsigned char data) {
  if (verbose) {
    Serial.print("[VERBOSE] Input data: ");
  	Serial.println(data, HEX);
  	Serial.println();
  }

	float calc = (data * 256) / 1000;
	return calc;
}

float calcAfr(unsigned char data) {
  if (verbose) {
    Serial.print("[VERBOSE] Input data: ");
  	Serial.println(data, HEX);
  }

	float calc = (data / 128) * 14.7;
	return calc;
}


void setFrameBuffer() {
  //Serial.println("buffer)");
  
  tft.useFrameBuffer(1);
  tft.fillScreen(ILI9341_BLACK);
  

  ////////////////////////////////////////* Top RPM bar *////////////////////////////////////////
  int barMap;
  int rpmPx = map(rpmFinal, 0, RPM_MAX, 0, RPM_BAR_WIDTH);

  // defines the yellow and redline blocks based on oil temperature
  if (oilTemperature <= OIL_COLD_THRESHOLD) {
    yellowMin = RPM_YELLOW_COLD;
    yellowMax = RPM_RED_COLD - 1;
    redMin = RPM_RED_COLD;
    redMax = RPM_MAX;
  }
  else if ((oilTemperature > OIL_COLD_THRESHOLD) && (oilTemperature < OIL_WARMUP_THRESHOLD)) {
    yellowMin = RPM_YELLOW_WARMUP;
    yellowMax = RPM_RED_WARMUP - 1;
    redMin = RPM_RED_WARMUP;
    redMax = RPM_MAX;
  }
  else {
    yellowMin = RPM_YELLOW_NORMAL;
    yellowMax = RPM_RED_NORMAL - 1;
    redMin = RPM_RED_NORMAL;
    redMax = RPM_MAX;
  }

  yellowMinPx = map(yellowMin, 0, RPM_MAX, 0, RPM_BAR_WIDTH);
  yellowMaxPx = map(yellowMax, 0, RPM_MAX, 0, RPM_BAR_WIDTH);
  yellowFill = yellowMaxPx - yellowMinPx;
  redMinPx = map(redMin, 0, RPM_MAX, 0, RPM_BAR_WIDTH);
  revLimitPx = REV_LIMIT_PX;
  redMaxPx = map(redMax, 0, RPM_MAX, 0, RPM_BAR_WIDTH);
  redFill = redMaxPx - redMinPx;

  // draws the actual line for rpm based on 8000 max and 240px width
  if (rpmFinal < yellowMin) {
    tft.fillRect(0, 0, rpmPx, RPM_BAR_HEIGHT, ILI9341_WHITE);
  }
  else if ((rpmFinal >= yellowMin) && (rpmFinal <= yellowMax)) {
    tft.fillRect(0, 0, rpmPx, RPM_BAR_HEIGHT, ILI9341_YELLOW);
  }
  else if ((rpmFinal >= redMin) && (rpmFinal <= redMax)) {
    tft.fillRect(0, 0, rpmPx, RPM_BAR_HEIGHT, ILI9341_RED);
  }

  // this draws the yellow and red top block sections
  tft.fillRect(yellowMinPx, 0, yellowFill, 10, ILI9341_YELLOW);
  tft.fillRect(redMinPx, 0, redFill, 10, ILI9341_RED);

  // this draws the vertical lines that seperate the colors
  tft.drawLine(yellowMinPx, 0, yellowMinPx, RPM_BAR_HEIGHT, ILI9341_WHITE);
  tft.drawLine(redMinPx, 0, redMinPx, RPM_BAR_HEIGHT, ILI9341_WHITE);
  tft.drawLine(revLimitPx, 0, revLimitPx, RPM_BAR_HEIGHT, ILI9341_WHITE);
  ////////////////////////////////////////* Top RPM bar *////////////////////////////////////////

    


  ////////////////////////////////////////* displaymode 2 aka race mode *////////////////////////////////////////
  bool noCanData = (!testData && (millis() - lastCanTime > 1000));

  if (noCanData) {
    /* OIL TEMP */
    tft.setCursor(0, oilTempRow);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println(" OIL\n TEMP");

    tft.drawRect(40, oilTempRow, 130, 15, ILI9341_WHITE);
    tft.fillRect(117, oilTempRow + 1, 25, 4, ILI9341_GREEN);
    tft.fillRect(142, oilTempRow + 1, 5, 4, ILI9341_YELLOW);
    tft.fillRect(147, oilTempRow + 1, 22, 4, ILI9341_RED);

    barMap = map(oilTemperature, -40, 300, 0, 130);
    tft.setCursor(180, oilTempRow - 4);
    tft.setTextSize(3);
    if ((oilTemperature >= 40) && (oilTemperature <= OIL_TEMP_COLD_MAX)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      tft.fillRect(41, oilTempRow + 1, barMap, 13, ILI9341_BLUE);
    }
    else if (((oilTemperature >= OIL_TEMP_WARM_MIN) && (oilTemperature <= OIL_TEMP_WARM_MAX)) || ((oilTemperature >= OIL_TEMP_HOT_MIN) && (oilTemperature <= OIL_TEMP_HOT_MAX))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
      tft.fillRect(41, oilTempRow + 1, barMap, 13, ILI9341_YELLOW);
    }
    else if ((oilTemperature >= OIL_TEMP_DANGER) || (oilTemperature < 40)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
      if (barMap > 128) { barMap = 128; }
      tft.fillRect(41, oilTempRow + 1, barMap, 13, ILI9341_RED);
    }
    else {
      tft.setTextColor(ILI9341_WHITE);
      tft.fillRect(41, oilTempRow + 1, barMap, 13, ILI9341_WHITE);
    }
    tft.print(oilTemperature);
    /* OIL TEMP */

    /* OIL PRESSURE */
    tft.setCursor(10, oilPressRow);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("OIL\nPRESS");

    tft.setCursor(180, oilPressRow - 4);
    tft.setTextSize(3);
    if (oilPressure <= 14 || oilPressure >= 90) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
    }
    else {
      tft.setTextColor(ILI9341_WHITE);
    }
    tft.print(oilPressure);

    tft.drawRect(40, oilPressRow, 130, 15, ILI9341_WHITE);
    tft.fillRect(41, oilPressRow + 1, 20, 4, ILI9341_RED);
    tft.fillRect(156, oilPressRow + 1, 13, 4, ILI9341_RED);

    barMap = map(oilPressure, 0, 100, 0, 130);
    tft.fillRect(41, oilPressRow + 1, barMap, 13, ILI9341_WHITE);
    /* OIL PRESSURE */

    tft.setCursor(10, 220);
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(3);
    tft.println("NO CAN DATA");
  }
  else if (displayMode == 2) {
  
    // row 1 left
    tft.setCursor(10, row1Lrg);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("OIL TEMP");

    //oil temp
    tft.setCursor(10, row1Lrg + 10);
    tft.setTextSize(6);

    // row 1 right
      // 40 - 129: blue
    if ((oilTemperature >= 40) && (oilTemperature <= OIL_TEMP_COLD_MAX)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    }
    // 130-159 and 225-240: yellow
    else if (((oilTemperature >= OIL_TEMP_WARM_MIN) && (oilTemperature <= OIL_TEMP_WARM_MAX)) || ((oilTemperature >= OIL_TEMP_HOT_MIN) && (oilTemperature <= OIL_TEMP_HOT_MAX))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((oilTemperature >= OIL_TEMP_DANGER) || (oilTemperature < 40)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
    } 
    // 160 to 225: normal
    else {
      tft.setTextColor(ILI9341_WHITE);
    }  
    tft.print(oilTemperature);


    tft.setCursor(150, row1Lrg);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("COOLANT"); 

    //coolant
    // 40 - 129: blue
    if ((coolantFinal >= 40) && (coolantFinal <= COOL_TEMP_COLD_MAX)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    }
    // 130-159 and 207-209: yellow
    else if (((coolantFinal >= COOL_TEMP_WARM_MIN) && (coolantFinal <= COOL_TEMP_WARM_MAX)) || ((coolantFinal >= COOL_TEMP_HOT_MIN) && (coolantFinal <= COOL_TEMP_HOT_MAX))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((coolantFinal >= COOL_TEMP_DANGER) || (coolantFinal < 40)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
    } 
    // 160 to 206: normal
    else {
      tft.setTextColor(ILI9341_WHITE);
    }  
    
    tft.setCursor(130, row1Lrg + 10);
    tft.setTextSize(6);
    tft.print(coolantFinal);





    // row 2 left
    tft.setCursor(10, row2Lrg + 30);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("OIL PRESS");

    //oil pressure
    tft.setCursor(10, row2Lrg + 40);
    tft.setTextSize(4);
    tft.print(oilPressure);


    // row 2 right
    tft.setCursor(150, row2Lrg + 30);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("INTAKE");

    // intake temp
    tft.setCursor(130, row2Lrg + 40);
    tft.setTextSize(4);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(intakeTempFinal);


    // row 3 left
    tft.setCursor(10, row3Lrg + 40);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("DAM");

    //dam
    tft.setCursor(10, row3Lrg + 50);
    tft.setTextSize(3);
    if (damFinal != 1.00) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
    }
    else {
      tft.setTextColor(ILI9341_WHITE);
    }
    tft.print(damFinal);


    // row 3 right
    tft.setCursor(150, row3Lrg + 40);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("BOOST");

    //boost
    tft.setCursor(130, row3Lrg + 50);
    tft.setTextSize(3);
    tft.print(boostFinal);


    // row 4 left
    tft.setCursor(10, row4 + 50);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("FEEDBACK KNOCK");

    // feedback knock
    tft.setCursor(10, row4 + 60);
    tft.setTextSize(3);
    if (feedbackKnockFinal < 0) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    else {
      tft.setTextColor(ILI9341_WHITE);
    }
    tft.print(feedbackMax);


    // row 4 right
    tft.setCursor(150, row4 + 50);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("FINE KNOCK");

    //fine knock
    tft.setCursor(130, row4 + 60);
    tft.setTextSize(3);
    if (fineKnockFinal < 0) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    else {
      tft.setTextColor(ILI9341_WHITE);
    }
    tft.print(fineMax);
    tft.setTextColor(ILI9341_WHITE);


    // prints the RPM range of the fine knock events its seen
    tft.setTextSize(2);
    tft.setCursor(130, 280);
    if (fineRpmMin == 9999) { tft.print(0); }
    else { tft.print(fineRpmMin); }
    tft.setCursor(190, 280);
    tft.print(fineRpmMax);    
  }

  ////////////////////////////////////////* displaymodd 3 - new normal mode with bars *////////////////////////////////////////
  else if (displayMode == 3) {
    
    /* OIL TEMP */
    // print oil temp label
    tft.setCursor(0, oilTempRow);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println(" OIL\n TEMP");

    // draws the first row empty bar
    tft.drawRect(40, oilTempRow, 130, 15, ILI9341_WHITE);
    tft.fillRect(117, oilTempRow + 1, 25, 4, ILI9341_GREEN); // green range
    tft.fillRect(142, oilTempRow + 1, 5, 4, ILI9341_YELLOW); // yellow range
    tft.fillRect(147, oilTempRow + 1, 22, 4, ILI9341_RED); // red range

    // maps the oil temp value to pixels for the bar printing
    //barMap = map(oilTemperature, 147, 277, 0, 130);
    barMap = map(oilTemperature, -40, 300, 0, 130);
    //Serial.println(barMap);

    // prints oil temp bar fill and number
    tft.setCursor(180, oilTempRow - 4);
    tft.setTextSize(3);
      // 40 - 129: blue
    if ((oilTemperature >= 40) && (oilTemperature <= OIL_TEMP_COLD_MAX)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      tft.fillRect(41, oilTempRow + 1, barMap, 13, ILI9341_BLUE);
    }
    // 130-159 and 225-240: yellow
    else if (((oilTemperature >= OIL_TEMP_WARM_MIN) && (oilTemperature <= OIL_TEMP_WARM_MAX)) || ((oilTemperature >= OIL_TEMP_HOT_MIN) && (oilTemperature <= OIL_TEMP_HOT_MAX))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
      tft.fillRect(41, oilTempRow + 1, barMap, 13, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((oilTemperature >= OIL_TEMP_DANGER) || (oilTemperature < 40)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
      if (barMap > 128) { barMap = 128; }
      tft.fillRect(41, oilTempRow + 1, barMap, 13, ILI9341_RED);
    } 
    // 160 to 225: normal
    else {
      tft.setTextColor(ILI9341_WHITE);
      tft.fillRect(41, oilTempRow + 1, barMap, 13, ILI9341_WHITE);
    }  
    tft.print(oilTemperature);
    /* OIL TEMP */



    /* COOLANT TEMP */
    tft.setCursor(0, coolantRow);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println(" COOL\n TEMP");

    // draws the first row empty bar
    //tft.drawRect(40, 90, 130, 24, ILI9341_WHITE);
    tft.drawRect(40, coolantRow, 130, 15, ILI9341_WHITE);
    tft.fillRect(125, coolantRow + 1, 19, 4, ILI9341_GREEN); // green range
    tft.fillRect(144, coolantRow + 1, 2, 4, ILI9341_YELLOW); // yellow range
    tft.fillRect(146, coolantRow + 1, 23, 4, ILI9341_RED); // red range

    // maps the coolant temp value to pixels for the bar printing
    //barMap = map(coolantFinal, 140, 270, 0, 130);
    barMap = map(coolantFinal, -40, 270, 0, 130);
    //Serial.println(barMap);

    //coolant
    // 40 - 129: blue
    if ((coolantFinal >= 40) && (coolantFinal <= COOL_TEMP_COLD_MAX)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      tft.fillRect(41, coolantRow + 1, barMap, 13, ILI9341_BLUE);
    }
    // 130-159 and 207-209: yellow
    else if (((coolantFinal >= COOL_TEMP_WARM_MIN) && (coolantFinal <= COOL_TEMP_WARM_MAX)) || ((coolantFinal >= COOL_TEMP_HOT_MIN) && (coolantFinal <= COOL_TEMP_HOT_MAX))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
      tft.fillRect(41, coolantRow + 1, barMap, 13, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((coolantFinal >= COOL_TEMP_DANGER) || (coolantFinal < 40)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
      if (barMap > 128) { barMap = 128; }
      tft.fillRect(41, coolantRow + 1, barMap, 13, ILI9341_RED);
    } 
    // 160 to 206: normal
    else {
      tft.setTextColor(ILI9341_WHITE);
      tft.fillRect(41, coolantRow + 1, barMap, 13, ILI9341_WHITE);
    }  
    
    tft.setCursor(180, coolantRow - 4);
    tft.setTextSize(3);
    tft.print(coolantFinal);
    /* COOLANT TEMP */



    /* OIL PRESSURE */
    tft.setCursor(10, oilPressRow);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("OIL\nPRESS");

    //oil pressure
    tft.setCursor(180, oilPressRow - 4);
    tft.setTextSize(3);
    if (oilPressure <= 14 || oilPressure >= 90) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
    }
    else {
      tft.setTextColor(ILI9341_WHITE);
    }
    tft.print(oilPressure);

    // draws the first row empty bar
    tft.drawRect(40, oilPressRow, 130, 15, ILI9341_WHITE);
    tft.fillRect(41, oilPressRow + 1, 20, 4, ILI9341_RED); // green range
    tft.fillRect(156, oilPressRow + 1, 13, 4, ILI9341_RED); // yellow range

    // maps the coolant temp value to pixels for the bar printing
    barMap = map(oilPressure, 0, 100, 0, 130);
    tft.fillRect(41, oilPressRow + 1, barMap, 13, ILI9341_WHITE);
    /* OIL PRESSURE */



    /* BOOST */
    tft.setCursor(0, boostRow);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("BOOST");

    //boost
    tft.setCursor(160, boostRow - 4);
    tft.setTextSize(2);
    tft.print(boostFinal);

    // draws the first row empty bar
    tft.drawRect(40, boostRow - 4, 110, 15, ILI9341_WHITE);
    tft.fillRect(144, boostRow - 3, 5, 4, ILI9341_RED); // yellow range

    // maps the boost value to pixels for the bar printing
    barMap = map(boostFinal, 0, 20, 0, 110);
    tft.fillRect(41, boostRow -4, barMap, 14, ILI9341_WHITE);
    /* BOOST */
    
    


    /* DIFF AND DCCD */
    tft.setCursor(0, diffDccdRow);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("DIFF");

    // diff temp
    tft.setCursor(50, diffDccdRow - 4);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(diffTemperature);


    // diff/dccd row right
    tft.setCursor(130, diffDccdRow);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("DCCD");

    // dccd percent
    tft.setCursor(170, diffDccdRow - 4);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE);
    tft.print((int)dccdPercent);
    tft.print("%");
    /* DIFF AND DCCD */


    /* INTAKE AND DAM */
    tft.setCursor(0, intakeDamRow);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("INTAKE");

    // intake temp
    tft.setCursor(50, intakeDamRow - 4);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_WHITE);
    tft.print(intakeTempFinal);


    // row 5 right
    tft.setCursor(130, intakeDamRow);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("DAM");

    //dam
    tft.setCursor(170, intakeDamRow - 4);
    tft.setTextSize(2);
    if (damFinal != 1.00) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
    }
    else {
      tft.setTextColor(ILI9341_WHITE);
    }
    tft.print(damFinal);
    /* INTAKE AND DAM */


    // row 6 left
    tft.setCursor(0, knockRow);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("FEED");

    // feedback knock
    tft.setCursor(40, knockRow);
    tft.setTextSize(2);
    if (feedbackKnockFinal < 0) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    else {
      tft.setTextColor(ILI9341_WHITE);
    }
    tft.print(feedbackMax);


    // row 6 right
    tft.setCursor(130, knockRow);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("FINE");

    //fine knock
    tft.setCursor(160, knockRow);
    tft.setTextSize(2);
    if (fineKnockFinal < 0) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    else {
      tft.setTextColor(ILI9341_WHITE);
    }
    tft.print(fineMax);
    tft.setTextColor(ILI9341_WHITE);


    tft.setTextSize(1);
    tft.setCursor(130, knockRow + 20);
    if (fineRpmMin == 9999) { tft.print(0); }
    else { tft.print(fineRpmMin); }
    tft.setCursor(190, knockRow + 20);
    tft.print(fineRpmMax);
  }

  ////////////////////////////////////////* anything else, mostly to handle a mode 0 if the canbus data is not parsed right *////////////////////////////////////////
  else {


    tft.setCursor(10, row1);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("OIL TEMP");

    //oil temp
    tft.setCursor(10, row1 + 10);
    tft.setTextSize(4);

    // row 1 right
      // 40 - 129: blue
    if ((oilTemperature >= 40) && (oilTemperature <= OIL_TEMP_COLD_MAX)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    }
    // 130-159 and 225-240: yellow
    else if (((oilTemperature >= OIL_TEMP_WARM_MIN) && (oilTemperature <= OIL_TEMP_WARM_MAX)) || ((oilTemperature >= OIL_TEMP_HOT_MIN) && (oilTemperature <= OIL_TEMP_HOT_MAX))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((oilTemperature >= OIL_TEMP_DANGER) || (oilTemperature < 40)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
    } 
    // 160 to 225: normal
    else {
      tft.setTextColor(ILI9341_WHITE);
    }  
    tft.print(oilTemperature);

    tft.setCursor(180, row1);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(1);
    tft.println("OIL PRESS");

    //oil pressure
    tft.setCursor(180, row1 + 10);
    tft.setTextSize(4);
    tft.print(oilPressure);


    tft.setCursor(0, 150);
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(3);
    tft.println("UNKN CAN DATA");
  }

 


  ////////////////////////////////////////* BOTTOM BAR *////////////////////////////////////////
  // sets the first block: active/passive/testing mode
  if (ssmActive) {
    tft.setCursor(5,statusRow);
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
    tft.print("ACTIVE");
  }
  else if ((!(ssmActive)) && (testData)) {
    tft.setCursor(5,statusRow);
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK, ILI9341_RED);
    tft.print("TESTING");    
  }
  else {
    tft.setCursor(5,statusRow);
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    tft.print("PASSIVE");
  }

  // sets the second block: refresh rate, now actually shows the real value based on the global var
  tft.setCursor(50,statusRow);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
  tft.print(updateHz);
  tft.print("Hz");
  
  // sets the third block: display mode normal/race/unknown
  tft.setCursor(85,statusRow);
  tft.setTextSize(1);
  if (displayMode == 1) {
    tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
    tft.print("MODE: NORMAL");   
  }
  else if (displayMode == 2) {
    tft.setTextColor(ILI9341_BLACK, ILI9341_BLUE);
    tft.print("MODE: RACE");   
  }
  else if (displayMode == 3) {
    tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
    tft.print("MODE: NORM");   
  }
  else {
    tft.setTextColor(ILI9341_BLACK, ILI9341_RED);
    tft.print("MODE: UNKNOWN");   
  }

  tft.setCursor(160,statusRow);
  if (flowCont) {
    tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
  }
  else {
    tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
  }
  tft.print("FLOW");

  tft.setCursor(190,statusRow);
  if ((GPS.HDOP > 0.0) && (GPS.HDOP < 2.0)) {
    tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
  }
  else {
    tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
  }
  tft.print("HDOP ");
  tft.print(GPS.HDOP, 1);
  ////////////////////////////////////////* BOTTOM BAR *////////////////////////////////////////

}



void updateAllBufferAsync() {
  if (!tft.asyncUpdateActive()) {
    setFrameBuffer();
    tft.updateScreenAsync();
  }
}




// sends the new standlone request
void sendNewRequest() {
  int sendDelay = 2;
  int initialDelay = 10;
  int flowDelay = 5;

  sendMessage(newReq1);
  delay(initialDelay);
  sendMessage(newReq2);
  delay(sendDelay);
  sendMessage(newReq3);
  delay(sendDelay);
  sendMessage(newReq4);
  delay(sendDelay);
  sendMessage(newReq5);
  delay(sendDelay);
  sendMessage(newReq6);
  delay(sendDelay);
  sendMessage(newReq7);
  delay(sendDelay);
  sendMessage(newReq8);
  delay(sendDelay);
  sendMessage(newReq9);
  delay(sendDelay);
  sendMessage(newReq10);
  delay(flowDelay);
  sendFlow();
}

void sendFlow() {
    CAN_message_t msg;
    msg.id = 0x7E0;

    for (int i = 0; i < 8; i++) {
      msg.buf[i] = newReq0[i];
    }

      /*
      Serial.print("Sending message: "); 
      for (int i = 0; i < 8; i++) {
        msg.buf[i] = req0[i];
        Serial.print(msg.buf[i], HEX); 
        Serial.print(" "); 
      }
      Serial.println();
      */

    Can0.write(msg);
}

void sendMessage(const unsigned char data[8]) {
    CAN_message_t msg;
    msg.id = 0x7E0;

      
      //Serial.print("Sending message: "); 
      for (int i = 0; i < 8; i++) {
        msg.buf[i] = data[i];
        //Serial.print(msg.buf[i], HEX); 
        //Serial.print(" "); 
      }
      //Serial.println(); 
      
      Can0.write(msg);
}
#include <Arduino.h>
#include <FlexCAN_T4.h>
#include "SPI.h"
#include "ILI9341_t3n.h"
#include "globalVars.h"
#include "canParser.h"
#include "dataParser.h"
#include "serialSender.h"
#include "printDisplay.h"
#include "canSender.h"





void setup(void) {
  Serial.begin(115200);
  delay(10000);
  if (!(testData)) { HWSERIAL.begin(9600); }
  if (sendToEsp) { delay(400); HWSERIAL2.begin(38400); }
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
  Can0.setBaudRate(500000);
  Can0.setMaxMB(16);
  Can0.enableFIFO();
  Can0.enableFIFOInterrupt();
  Can0.onReceive(canSniffIso);

  Can0.setFIFOFilter(REJECT_ALL);
  Can0.setFIFOFilter(0, 0x7E8, STD);
  Can0.setFIFOFilter(1, 0x7E0, STD);

  Can0.mailboxStatus();
  tft.println("WAITING FOR CAN MSG");
  //if (!(testData)) { delay(10000); }
  if (testData) { 
    ssmActive = 0;
    flowCont = 0; 
  }  // turn off SSM active is test data is on, no need for this

  pinMode(5, INPUT);  // pin 5 is the check for whether the mode button is pushed
  pinMode(21, OUTPUT);  // pin 21 is the high side of the button
  digitalWrite(21, HIGH);
  //delay(5000);


}


void loop() {  
  // some crazy stuff i found on the internet.  how i receive and parse two integers at once via serial from an arduino
  if (!(testData)) {
    if (HWSERIAL.available ()) {

      //Serial.println("HWSERIAL Received");
      char buf [80];
      int n = HWSERIAL.readBytesUntil ('\n', buf, sizeof(buf));
      //Serial.println(n);
      // check for a real value.  weird things happen if the logic converter is connected but nothing is sending
      if ((n > 1) && (n < 15)) {
        buf [n] = '\0';     // terminate with null
        if (verbose) { Serial.println("[VERBOSE] Serial Received"); }

        char *t = strtok (buf, ",");
        //Serial.println(t);
        processOil (t);
        while ((t = strtok (NULL, ",")))
            processOil (t);
      }
    }
  }
  
  

  // if active is still set, send the entire small request.  since flexcan runs on interrupts, this can run in the loop and still hit the cansniffiso parsing
  // if this is not set, cansniffiso will still work in case an AP is plugged in
  if (ssmActive) {
    //if (flowCont) { sendSmallRequest(); }
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
        Serial.print(" RPM: "); Serial.print(rpmFinal);
        Serial.print(" GEAR: "); Serial.print(gearFinal);
        Serial.print(" SPEED: "); Serial.print(speedFinal);
        Serial.print(" AFR: "); Serial.print(afrFinal);
        Serial.print(" THROTTLE: "); Serial.println(throttleFinal);
      }


      if (logger) {
        displayMode = displayModeLogging;
      }
      else {
        displayMode = displayModeNormal;
      }

      //if (print) { updateAllBuffer(); }
      updateAllBufferAsync();
      print = !print;
      logger = digitalRead(BUTTONSIGNAL);  // reads the logging button value
      sendEsp();
      delay(updateInt);
      //updateHz = 1.0 / ((micros() - start) / 1000000.0);
      //Serial.println(updateHz);
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
      Serial.print(" RPM: "); Serial.print(rpmFinal);
      Serial.print(" GEAR: "); Serial.print(gearFinal);
      Serial.print(" SPEED: "); Serial.print(speedFinal);
      Serial.print(" AFR: "); Serial.print(afrFinal);
      Serial.print(" THROTTLE: "); Serial.println(throttleFinal);
    }

    //if (print) { updateAllBuffer(); }
    updateAllBufferAsync();
    print = !print;
    logger = digitalRead(BUTTONSIGNAL);  // reads the logging button value
    sendEsp();
    delay(updateInt);
    //updateHz = 1.0 / ((micros() - start) / 1000000.0);
    //Serial.println(updateHz);
  }
}







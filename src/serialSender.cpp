#include "serialSender.h"
#include "globalVars.h"




void sendEsp() {
  if (sendToEsp) {
    int nums[8] = {coolantFinal, intakeTempFinal, rpmFinal, gearFinal, speedFinal, throttleFinal, oilTemperature, oilPressure};
    float floats[5] = {feedbackKnockFinal, fineKnockFinal, boostFinal, damFinal, afrFinal};

    for (int z = 0; z < 8; z++) {
      Serial2.print(nums[z]);
      //Serial.print(nums[z]);
      Serial2.print(",");
      //Serial.print(",");
    }
    for (int z = 0; z < 5; z++) {
      Serial2.print(floats[z]);
      //Serial.print(floats[z]);
      if (z < 4) {
        Serial2.print(",");
        //Serial.print(",");
      }
    }

    Serial2.print("\n");
    //Serial.print("\n");
  }
}


void sendNbp() {
  timer = millis();
  char header[64];
  sprintf(header, "*NBP1,UPDATEALL,%d.%03d", (int)(timer/1000), (int)(timer % 1000));  
  Serial.println(header);
  Serial.print("\"Feedback Knock\",\"Deg\":");
  Serial.println(feedbackKnockFinal);
  Serial.print("\"Fine Knock\",\"Deg\":");
  Serial.println(fineKnockFinal);
  Serial.print("\"Boost\",\"PSI\":");
  Serial.println(boostFinal);
  Serial.print("\"Coolant Temperature\",\"F\":");
  Serial.println(coolantFinal);
  Serial.print("\"DAM\",\"Deg\":");
  Serial.println(damFinal);
  Serial.print("\"Intake Air Temperature\",\"F\":");
  Serial.println(intakeTempFinal);
  Serial.print("\"Oil Temperature\",\"F\":");
  Serial.println(oilTemperature);
  Serial.print("\"Oil Pressure\",\"PSI\":");
  Serial.println(oilPressure);
  Serial.print("\"Engine Speed\",\"RPM\":");
  Serial.println(rpmFinal);
  Serial.print("\"Gear Position\":");
  Serial.println(gearFinal);
  Serial.print("\"Vehicle Speed\",\"MPH\":");
  Serial.println(speedFinal);
  Serial.print("\"AFR\",\"AFR\":");
  Serial.println(afrFinal);
  Serial.print("\"Throttle Position\",\"%\":");
  Serial.println(throttleFinal);
  Serial.println("#");
}


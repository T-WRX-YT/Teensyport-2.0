#include "printDisplay.h"
#include "globalVars.h"
#include "ILI9341_t3n.h"



void updateAllBuffer() {
  //unsigned long start = micros();
  tft.useFrameBuffer(1);
  tft.fillScreen(ILI9341_BLACK);
  

  ////////////////////////////////////////* Top RPM bar *////////////////////////////////////////
  int barMap;
  int rpmPx = map(rpmFinal, 0, 8000, 0, 240);

  // defines the yellow and redline blocks based on oil temperature
  if (oilTemperature <= 120) {
    yellowMin = 2500;
    yellowMax = 2999;
    redMin = 3000;
    redMax = 8000;
  }
  else if ((oilTemperature > 120) && (oilTemperature < 160)) {
    yellowMin = 2500;
    yellowMax = 3499;
    redMin = 3500;
    redMax = 8000;
  }
  else {
    yellowMin = 5000;
    yellowMax = 5999;
    redMin = 6000;
    redMax = 8000;
  }

  yellowMinPx = map(yellowMin, 0, 8000, 0, 240);
  yellowMaxPx = map(yellowMax, 0, 8000, 0, 240);
  yellowFill = yellowMaxPx - yellowMinPx;
  redMinPx = map(redMin, 0, 8000, 0, 240);
  revLimitPx = 204;
  redMaxPx = map(redMax, 0, 8000, 0, 240);
  redFill = redMaxPx - redMinPx;

  // draws the actual line for rpm based on 8000 max and 240px width
  if (rpmFinal < yellowMin) {
    tft.fillRect(0, 0, rpmPx, 40, ILI9341_WHITE);
  }
  else if ((rpmFinal >= yellowMin) && (rpmFinal <= yellowMax)) {
    tft.fillRect(0, 0, rpmPx, 40, ILI9341_YELLOW);
  }
  else if ((rpmFinal >= redMin) && (rpmFinal <= redMax)) {
    tft.fillRect(0, 0, rpmPx, 40, ILI9341_RED);
  }

  // this draws the yellow and red top block sections
  tft.fillRect(yellowMinPx, 0, yellowFill, 10, ILI9341_YELLOW);
  tft.fillRect(redMinPx, 0, redFill, 10, ILI9341_RED);

  // this draws the vertical lines that seperate the colors
  tft.drawLine(yellowMinPx, 0, yellowMinPx, 40, ILI9341_WHITE);
  tft.drawLine(redMinPx, 0, redMinPx, 40, ILI9341_WHITE);
  tft.drawLine(revLimitPx, 0, revLimitPx, 40, ILI9341_WHITE);
  ////////////////////////////////////////* Top RPM bar *////////////////////////////////////////

    
 

  // displaymode 2 aka race mode
  if (displayMode == 2) {
  
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
    if ((oilTemperature >= 40) && (oilTemperature < 130)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    }
    // 130-159 and 225-240: yellow
    else if (((oilTemperature >= 130) && (oilTemperature < 160)) || ((oilTemperature >= 225) && (oilTemperature < 241))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((oilTemperature >= 241) || (oilTemperature < 40)) {
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
    if ((coolantFinal >= 40) && (coolantFinal < 130)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    }
    // 130-159 and 207-209: yellow
    else if (((coolantFinal >= 130) && (coolantFinal < 160)) || ((coolantFinal >= 207) && (coolantFinal < 210))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((coolantFinal >= 210) || (coolantFinal < 40)) {
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

  // displaymodd 3 - new normal mode with bars
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
    if ((oilTemperature >= 40) && (oilTemperature < 130)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      tft.fillRect(41, oilTempRow + 1, barMap, 13, ILI9341_BLUE);
    }
    // 130-159 and 225-240: yellow
    else if (((oilTemperature >= 130) && (oilTemperature < 160)) || ((oilTemperature >= 225) && (oilTemperature < 241))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
      tft.fillRect(41, oilTempRow + 1, barMap, 13, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((oilTemperature >= 241) || (oilTemperature < 40)) {
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
    if ((coolantFinal >= 40) && (coolantFinal < 130)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      tft.fillRect(41, coolantRow + 1, barMap, 13, ILI9341_BLUE);
    }
    // 130-159 and 207-209: yellow
    else if (((coolantFinal >= 130) && (coolantFinal < 160)) || ((coolantFinal >= 207) && (coolantFinal < 210))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
      tft.fillRect(41, coolantRow + 1, barMap, 13, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((coolantFinal >= 210) || (coolantFinal < 40)) {
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

  // anything else, mostly to handle a mode 0 if the canbus data is not parsed right
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
    if ((oilTemperature >= 40) && (oilTemperature < 130)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    }
    // 130-159 and 225-240: yellow
    else if (((oilTemperature >= 130) && (oilTemperature < 160)) || ((oilTemperature >= 225) && (oilTemperature < 241))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((oilTemperature >= 241) || (oilTemperature < 40)) {
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

 


  ////////////////////////////////////////////////////////////////////// bottom
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
  //tft.setCursor(50,statusRow);
  //tft.setTextSize(1);
  //tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
  //tft.print(updateHz);
  //tft.print("Hz");
  
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
    tft.print("MODE: NORMAL v2");   
  }
  else {
    tft.setTextColor(ILI9341_BLACK, ILI9341_RED);
    tft.print("MODE: UNKNOWN");   
  }

  tft.setCursor(180,statusRow);
  if (flowCont) {
    tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
  }
  else {
    tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
  }
  tft.print("FLOW");   
  ////////////////////////////////////////////////////////////////////// bottom end

  
  //tft.updateScreen(); 
  
  if (!tft.asyncUpdateActive()) {
    tft.updateScreenAsync();
  }  
  
  
  //fps = 1.0 / ((micros() - start) / 1000000.0);
  //Serial.println(fps);
}



void setFrameBuffer() {
  tft.useFrameBuffer(1);
  tft.fillScreen(ILI9341_BLACK);
  

  ////////////////////////////////////////* Top RPM bar *////////////////////////////////////////
  int barMap;
  int rpmPx = map(rpmFinal, 0, 8000, 0, 240);

  // defines the yellow and redline blocks based on oil temperature
  if (oilTemperature <= 120) {
    yellowMin = 2500;
    yellowMax = 2999;
    redMin = 3000;
    redMax = 8000;
  }
  else if ((oilTemperature > 120) && (oilTemperature < 160)) {
    yellowMin = 2500;
    yellowMax = 3499;
    redMin = 3500;
    redMax = 8000;
  }
  else {
    yellowMin = 5000;
    yellowMax = 5999;
    redMin = 6000;
    redMax = 8000;
  }

  yellowMinPx = map(yellowMin, 0, 8000, 0, 240);
  yellowMaxPx = map(yellowMax, 0, 8000, 0, 240);
  yellowFill = yellowMaxPx - yellowMinPx;
  redMinPx = map(redMin, 0, 8000, 0, 240);
  revLimitPx = 204;
  redMaxPx = map(redMax, 0, 8000, 0, 240);
  redFill = redMaxPx - redMinPx;

  // draws the actual line for rpm based on 8000 max and 240px width
  if (rpmFinal < yellowMin) {
    tft.fillRect(0, 0, rpmPx, 40, ILI9341_WHITE);
  }
  else if ((rpmFinal >= yellowMin) && (rpmFinal <= yellowMax)) {
    tft.fillRect(0, 0, rpmPx, 40, ILI9341_YELLOW);
  }
  else if ((rpmFinal >= redMin) && (rpmFinal <= redMax)) {
    tft.fillRect(0, 0, rpmPx, 40, ILI9341_RED);
  }

  // this draws the yellow and red top block sections
  tft.fillRect(yellowMinPx, 0, yellowFill, 10, ILI9341_YELLOW);
  tft.fillRect(redMinPx, 0, redFill, 10, ILI9341_RED);

  // this draws the vertical lines that seperate the colors
  tft.drawLine(yellowMinPx, 0, yellowMinPx, 40, ILI9341_WHITE);
  tft.drawLine(redMinPx, 0, redMinPx, 40, ILI9341_WHITE);
  tft.drawLine(revLimitPx, 0, revLimitPx, 40, ILI9341_WHITE);
  ////////////////////////////////////////* Top RPM bar *////////////////////////////////////////

    


  ////////////////////////////////////////* displaymode 2 aka race mode *////////////////////////////////////////
  if (displayMode == 2) {
  
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
    if ((oilTemperature >= 40) && (oilTemperature < 130)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    }
    // 130-159 and 225-240: yellow
    else if (((oilTemperature >= 130) && (oilTemperature < 160)) || ((oilTemperature >= 225) && (oilTemperature < 241))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((oilTemperature >= 241) || (oilTemperature < 40)) {
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
    if ((coolantFinal >= 40) && (coolantFinal < 130)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    }
    // 130-159 and 207-209: yellow
    else if (((coolantFinal >= 130) && (coolantFinal < 160)) || ((coolantFinal >= 207) && (coolantFinal < 210))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((coolantFinal >= 210) || (coolantFinal < 40)) {
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
    if ((oilTemperature >= 40) && (oilTemperature < 130)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      tft.fillRect(41, oilTempRow + 1, barMap, 13, ILI9341_BLUE);
    }
    // 130-159 and 225-240: yellow
    else if (((oilTemperature >= 130) && (oilTemperature < 160)) || ((oilTemperature >= 225) && (oilTemperature < 241))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
      tft.fillRect(41, oilTempRow + 1, barMap, 13, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((oilTemperature >= 241) || (oilTemperature < 40)) {
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
    if ((coolantFinal >= 40) && (coolantFinal < 130)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
      tft.fillRect(41, coolantRow + 1, barMap, 13, ILI9341_BLUE);
    }
    // 130-159 and 207-209: yellow
    else if (((coolantFinal >= 130) && (coolantFinal < 160)) || ((coolantFinal >= 207) && (coolantFinal < 210))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
      tft.fillRect(41, coolantRow + 1, barMap, 13, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((coolantFinal >= 210) || (coolantFinal < 40)) {
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
    if ((oilTemperature >= 40) && (oilTemperature < 130)) {
      tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    }
    // 130-159 and 225-240: yellow
    else if (((oilTemperature >= 130) && (oilTemperature < 160)) || ((oilTemperature >= 225) && (oilTemperature < 241))) {
      tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
    }
    // 210+ or below 40: red
    else if ((oilTemperature >= 241) || (oilTemperature < 40)) {
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
  //tft.setCursor(50,statusRow);
  //tft.setTextSize(1);
  //tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
  //tft.print(updateHz);
  //tft.print("Hz");
  
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
    tft.print("MODE: NORMAL v2");   
  }
  else {
    tft.setTextColor(ILI9341_BLACK, ILI9341_RED);
    tft.print("MODE: UNKNOWN");   
  }

  tft.setCursor(180,statusRow);
  if (flowCont) {
    tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
  }
  else {
    tft.setTextColor(ILI9341_BLACK, ILI9341_YELLOW);
  }
  tft.print("FLOW"); 
  ////////////////////////////////////////* BOTTOM BAR *////////////////////////////////////////

}



void updateAllBufferAsync() {
  if (!tft.asyncUpdateActive()) {
    setFrameBuffer();
    tft.updateScreenAsync();
  }
}


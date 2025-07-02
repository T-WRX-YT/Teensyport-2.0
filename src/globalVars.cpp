#include <Arduino.h>
#include "globalVars.h"



/* GLOBAL and SETUP VARS */
const bool verbose = 1; // prints the raw packet data for each canbus received message.  this generates a LOT of text!
const bool printStats = 1;  // prints the current gauge data values after each 0x30 packet.  mostly deprecated with printloopstats in place
const bool printLoopStats = 0;  // prints the current gauge data values when pushing to the display
const bool testData = 0;  // generate fake data and loop it to the display
const bool sendToEsp = 1;
bool ssmActive = 1; // set to 1 for active sending, 0 for passive listening.  will always turn off passive if it sees other traffic
const unsigned int updateInt = 100; // how fast to do an update in the loop, 20 should be ~40-50 times a second
unsigned int updateHz;  // the hz update speed
// 0 - unknown, 1 - normal, 2 - data logging, 3 - normal with bars.  it will auto detect 2 or 3 when active, if using test data set it manually
unsigned int displayMode = 3; // set this manually for test mode, otherwise it will use the below two values
const unsigned int displayModeNormal = 3; // set which display mode to use when the device is not logging.
const unsigned int displayModeLogging = 2; // set which display mode to use when the accessport is logging
/* GLOBAL and SETUP VARS */


unsigned char responseData[61]; // holds the entire response, 61 bytes is the max i am working with
uint8_t packetCount = 0;  // how many packets parsed in this block
uint8_t byteCount = 0;  // how many bytes written, this will include any empty bytes filling the last packet, counts as the index of the responseData array
uint8_t responseBytes;  // the value in the 0x10 packet that the ECU says is how many bytes its sending
uint8_t responseType;  // holds the type of data.  0 = AP 6 gauge, 1 = AP full logging, 2 = standalone
bool flowCont = 1;  // this starts at 1, but if the 0x30 packet isn't all zeroes it will go to 0 then ssm active will turn off

float feedbackKnockFinal;  // holds the calculated value for current feedback knock
float feedbackMax = 0;  // holds the max negative value seen for feedback knock
float fineKnockFinal;  // holds the calculated value for the current fine knock correction
float fineMax = 0;  // holds the max negative value seen for fine knock
uint16_t fineRpmMin = 9999;  // this is the smallest RPM value seen when there is fine knock correction, starts at 9999
uint16_t fineRpmMax = 0;  // this is the higest RPM value seen when there is fine knock, combine both to see what range you've got fine knock (its good enough)
float boostFinal;  // holds the calculated boost value
int16_t coolantFinal;  // holds the calculated coolant temperature
float damFinal; // holds the calculated DAM value, if this isn't 1... youve got problems
int16_t intakeTempFinal;  // holds the calculated value of the intake air temp
uint16_t rpmFinal;  // holds the calculated value of engine speed aka RPM
uint8_t gearFinal; // holds the assumed gear position, not always accurate
uint8_t speedFinal; // holds obd2 vehicle speed
float afrFinal; // holds calulated AFR
uint8_t throttleFinal; // holds throttle plate angle
unsigned long timer;  // will hold the seconds count used for the nbp send
unsigned int logger; // holds the value of the button for turning on nbp logging
bool print = 1; // toggles whether to update the display or not this loop

// this is the standard 6 gauge non logging setup request.  this is only valid for my ECU ID yours might be different
// data collected (not in same order): feedback knock, fine knock, rpm, boost, coolant temp, dam, intake temp
const unsigned char req0[8] = {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const unsigned char req1[8] = {0x10, 0x35, 0xA8, 0x00, 0xFF, 0x7D, 0xA0, 0xFF};
const unsigned char req2[8] = {0x21, 0x7D, 0xA1, 0xFF, 0x7D, 0xA2, 0xFF, 0x7D};
const unsigned char req3[8] = {0x22, 0xA3, 0xFF, 0x7E, 0x3C, 0xFF, 0x7E, 0x3D};
const unsigned char req4[8] = {0x23, 0xFF, 0x7E, 0x3E, 0xFF, 0x7E, 0x3F, 0xFF};
const unsigned char req5[8] = {0x24, 0x62, 0x00, 0xFF, 0x62, 0x01, 0xFF, 0x62};
const unsigned char req6[8] = {0x25, 0x02, 0xFF, 0x62, 0x03, 0x00, 0x00, 0x0E};
const unsigned char req7[8] = {0x26, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x08, 0xFF};
const unsigned char req8[8] = {0x27, 0x68, 0x5E, 0x00, 0x00, 0x12, 0x00, 0x00};

// this is the new standalone request, will handle the normal non logging mode and the newer smaller logging mode
// data collected (not in same order): feedback knock, fine knock, rpm, boost, coolant temp, dam, intake temp, gear, speed, afr, throttle, brake pressure(?)
const unsigned char newReq0[8] = {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const unsigned char newReq1[8] = {0x10, 0x41, 0xA8, 0x00, 0xFF, 0x7D, 0xA0, 0xFF};
const unsigned char newReq2[8] = {0x21, 0x7D, 0xA1, 0xFF, 0x7D, 0xA2, 0xFF, 0x7D};
const unsigned char newReq3[8] = {0x22, 0xA3, 0xFF, 0x7E, 0x3C, 0xFF, 0x7E, 0x3D};
const unsigned char newReq4[8] = {0x23, 0xFF, 0x7E, 0x3E, 0xFF, 0x7E, 0x3F, 0xFF};
const unsigned char newReq5[8] = {0x24, 0x62, 0x00, 0xFF, 0x62, 0x01, 0xFF, 0x62};
const unsigned char newReq6[8] = {0x25, 0x02, 0xFF, 0x62, 0x03, 0x00, 0x00, 0x0E};
const unsigned char newReq7[8] = {0x26, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x08, 0xFF};
const unsigned char newReq8[8] = {0x27, 0x68, 0x5E, 0x00, 0x00, 0x12, 0xFF, 0x67};
const unsigned char newReq9[8] = {0x28, 0xF4, 0x00, 0x00, 0x10, 0x00, 0x00, 0x46};
const unsigned char newReq10[8] = {0x29, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x00};



// this is the big request used when im data logging, based on my ECU ID yours might be different
unsigned char longReq1[8] = {0x10, 0xB9, 0xA8, 0x00, 0xFF, 0x63, 0xE4, 0xFF};
unsigned char longReq2[8] = {0x21, 0x63, 0xE5, 0xFF, 0x63, 0xE6, 0xFF, 0x63};
unsigned char longReq3[8] = {0x22, 0xE7, 0xFF, 0x62, 0x00, 0xFF, 0x62, 0x01};
unsigned char longReq4[8] = {0x23, 0xFF, 0x62, 0x02, 0xFF, 0x62, 0x03, 0xFF};
unsigned char longReq5[8] = {0x24, 0x8D, 0x30, 0xFF, 0x8D, 0x31, 0xFF, 0x8D};
unsigned char longReq6[8] = {0x25, 0x32, 0xFF, 0x8D, 0x33, 0xFF, 0x1F, 0x00};
unsigned char longReq7[8] = {0x26, 0xFF, 0x1F, 0x01, 0xFF, 0x1F, 0x02, 0xFF};
unsigned char longReq8[8] = {0x27, 0x1F, 0x03, 0xFF, 0x79, 0x90, 0xFF, 0x79};
unsigned char longReq9[8] = {0x28, 0x91, 0xFF, 0x79, 0x92, 0xFF, 0x79, 0x93};
unsigned char longReq10[8] = {0x29, 0xFF, 0x81, 0x04, 0xFF, 0x81, 0x05, 0xFF};
unsigned char longReq11[8] = {0x2A, 0x81, 0x06, 0xFF, 0x81, 0x07, 0xFF, 0x7E};
unsigned char longReq12[8] = {0x2B, 0x3C, 0xFF, 0x7E, 0x3D, 0xFF, 0x7E, 0x3E};
unsigned char longReq13[8] = {0x2C, 0xFF, 0x7E, 0x3F, 0xFF, 0x7D, 0xA0, 0xFF};
unsigned char longReq14[8] = {0x2D, 0x7D, 0xA1, 0xFF, 0x7D, 0xA2, 0xFF, 0x7D};
unsigned char longReq15[8] = {0x2E, 0xA3, 0x00, 0x00, 0x0E, 0x00, 0x00, 0x0F};
unsigned char longReq16[8] = {0x2F, 0xFF, 0x1E, 0xE4, 0xFF, 0x1E, 0xE5, 0xFF};
unsigned char longReq17[8] = {0x20, 0x1E, 0xFA, 0xFF, 0x1E, 0xFB, 0xFF, 0x68};
unsigned char longReq18[8] = {0x21, 0x32, 0xFF, 0x68, 0x33, 0xFF, 0x68, 0x4C};
unsigned char longReq19[8] = {0x22, 0xFF, 0x68, 0x4D, 0x00, 0x00, 0x08, 0x00};
unsigned char longReq20[8] = {0x23, 0x00, 0x12, 0x00, 0x00, 0x3D, 0x00, 0x00};
unsigned char longReq21[8] = {0x24, 0x11, 0xFF, 0x67, 0xF4, 0x00, 0x00, 0x09};
unsigned char longReq22[8] = {0x25, 0x00, 0x00, 0xD8, 0x00, 0x00, 0x15, 0x00};
unsigned char longReq23[8] = {0x26, 0x00, 0x20, 0x00, 0x00, 0x10, 0x00, 0x00};
unsigned char longReq24[8] = {0x27, 0x0A, 0xFF, 0x68, 0x5E, 0x00, 0x00, 0x46};
unsigned char longReq25[8] = {0x28, 0x00, 0x00, 0xCE, 0x00, 0x00, 0xCF, 0x00};
unsigned char longReq26[8] = {0x29, 0x00, 0x1D, 0x00, 0x00, 0x30, 0x00, 0x00};
unsigned char longReq27[8] = {0x2A, 0x3C, 0x00, 0x00, 0xD9, 0x00, 0x00, 0x00};


/* RPM BAR VARIABLES */
uint16_t yellowMin, yellowMinPx, yellowMax, yellowMaxPx, yellowFill, redMin, redMinPx, revLimitPx, redMax, redMaxPx, redFill;  // defines all the variables for making the RPM bar
/* RPM BAR VARIABLES */


/* SERIAL CONNECTION TO ARDUINO */
char buf[10];
int16_t oilTemperature, oilPressure;
/* SERIAL CONNECTION TO ARDUINO */



ILI9341_t3n tft = ILI9341_t3n(TFT_CS, TFT_DC, TFT_RST);  // screen init
const uint8_t row1 = 100;
const uint8_t row2 = 130;
const uint8_t row3 = 160;
const uint8_t row4 = 190;
const uint8_t row5 = 240;
const uint8_t oilTempRow = row1;
const uint8_t coolantRow = row2;
const uint8_t oilPressRow = row3;
const uint8_t boostRow = row4;
const uint8_t intakeDamRow = row5;
const uint16_t knockRow = 270;
const uint16_t statusRow = 310;

const uint8_t row1Lrg = row1 - 40;
const uint8_t row2Lrg = row2 - 30;
const uint8_t row3Lrg = row3 - 10;

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;  // canbus init
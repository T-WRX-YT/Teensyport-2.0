#include <Arduino.h>
#include "ILI9341_t3n.h"
#include <FlexCAN_T4.h>

#ifndef GLOBALS_H
#define GLOBALS_H



// yo dawg i heard you like global variables

#define BUTTONSIGNAL 4 // which pin on the teensy should be checked for the screen mode button
#define HWSERIAL Serial1  //rx pin 0, data from arduino
#define HWSERIAL2 Serial2  //rx pin pin 7, tx pin 8, data to esp32

/* SCREEN SETUP for ILI9341 2.8inch screen */
/*
#define TFT_SCK 13  // default port, mostly for reference
#define TFT_MISO 12  // default port, mostly for reference
#define TFT_MOSI 11  // default port, mostly for reference
*/
/* USE THIS FOR TEENSY 4.0 */
//#define TFT_RST 8
//#define TFT_DC 9
//#define TFT_CS 10
/* USE THIS FOR TEENSY 4.0 */
/* USE THIS FOR TEENSY 4.1 */
#define TFT_CS 10
#define TFT_DC 14
#define TFT_RST 15
/* USE THIS FOR TEENSY 4.1 */


extern FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can0;  // canbus init

/* GLOBAL and SETUP VARS */
extern const bool verbose; // prints the raw packet data for each canbus received message.  this generates a LOT of text!
extern const bool printStats;  // prints the current gauge data values after each 0x30 packet.  mostly deprecated with printloopstats in place
extern const bool printLoopStats;  // prints the current gauge data values when pushing to the display
extern const bool testData;  // generate fake data and loop it to the display
extern const bool sendToEsp;
extern bool ssmActive; // set to 1 for active sending, 0 for passive listening.  will always turn off passive if it sees other traffic
extern const unsigned int updateInt; // how fast to do an update in the loop, 20 should be ~40-50 times a second
extern unsigned int updateHz;  // the hz update speed
// 0 - unknown, 1 - normal, 2 - data logging, 3 - normal with bars.  it will auto detect 2 or 3 when active, if using test data set it manually
extern unsigned int displayMode; // set this manually for test mode, otherwise it will use the below two values
extern const unsigned int displayModeNormal; // set which display mode to use when the device is not logging.
extern const unsigned int displayModeLogging; // set which display mode to use when the accessport is logging
/* GLOBAL and SETUP VARS */



extern unsigned char responseData[61];
extern uint8_t packetCount;  // how many packets parsed in this block
extern uint8_t byteCount;  // how many bytes written, this will include any empty bytes filling the last packet, counts as the index of the responseData array
extern uint8_t responseBytes;  // the value in the 0x10 packet that the ECU says is how many bytes its sending
extern uint8_t responseType;  // holds the type of data.  0 = AP 6 gauge, 1 = AP full logging, 2 = standalone
extern bool flowCont;  // this starts at 1, but if the 0x30 packet isn't all zeroes it will go to 0 then ssm active will turn off


extern float feedbackKnockFinal;  // holds the calculated value for current feedback knock
extern float feedbackMax;  // holds the max negative value seen for feedback knock
extern float fineKnockFinal;  // holds the calculated value for the current fine knock correction
extern float fineMax;  // holds the max negative value seen for fine knock
extern uint16_t fineRpmMin;  // this is the smallest RPM value seen when there is fine knock correction, starts at 9999
extern uint16_t fineRpmMax;  // this is the higest RPM value seen when there is fine knock, combine both to see what range you've got fine knock (its good enough)
extern float boostFinal;  // holds the calculated boost value
extern int16_t coolantFinal;  // holds the calculated coolant temperature
extern float damFinal; // holds the calculated DAM value, if this isn't 1... youve got problems
extern int16_t intakeTempFinal;  // holds the calculated value of the intake air temp
extern uint16_t rpmFinal;  // holds the calculated value of engine speed aka RPM
extern uint8_t gearFinal; // holds the assumed gear position, not always accurate
extern uint8_t speedFinal; // holds obd2 vehicle speed
extern float afrFinal; // holds calulated AFR
extern uint8_t throttleFinal; // holds throttle plate angle
extern unsigned long timer;  // will hold the seconds count used for the nbp send
extern unsigned int logger; // holds the value of the button for turning on nbp logging
extern bool print; // toggles whether to update the display or not this loop


// this is the standard 6 gauge non logging setup request.  this is only valid for my ECU ID yours might be different
// data collected (not in same order): feedback knock, fine knock, rpm, boost, coolant temp, dam, intake temp
extern const unsigned char req0[8];
extern const unsigned char req1[8];
extern const unsigned char req2[8];
extern const unsigned char req3[8];
extern const unsigned char req4[8];
extern const unsigned char req5[8];
extern const unsigned char req6[8];
extern const unsigned char req7[8];
extern const unsigned char req8[8];



// this is the new standalone request, will handle the normal non logging mode and the newer smaller logging mode
// data collected (not in same order): feedback knock, fine knock, rpm, boost, coolant temp, dam, intake temp, gear, speed, afr, throttle, brake pressure(?)
extern const unsigned char newReq0[8];
extern const unsigned char newReq1[8];
extern const unsigned char newReq2[8];
extern const unsigned char newReq3[8];
extern const unsigned char newReq4[8];
extern const unsigned char newReq5[8];
extern const unsigned char newReq6[8];
extern const unsigned char newReq7[8];
extern const unsigned char newReq8[8];
extern const unsigned char newReq9[8];
extern const unsigned char newReq10[8];



// this is the big request used when im data logging, based on my ECU ID yours might be different
extern unsigned char longReq1[8];
extern unsigned char longReq2[8];
extern unsigned char longReq3[8];
extern unsigned char longReq4[8];
extern unsigned char longReq5[8];
extern unsigned char longReq6[8];
extern unsigned char longReq7[8];
extern unsigned char longReq8[8];
extern unsigned char longReq9[8];
extern unsigned char longReq10[8];
extern unsigned char longReq11[8];
extern unsigned char longReq12[8];
extern unsigned char longReq13[8];
extern unsigned char longReq14[8];
extern unsigned char longReq15[8];
extern unsigned char longReq16[8];
extern unsigned char longReq17[8];
extern unsigned char longReq18[8];
extern unsigned char longReq19[8];
extern unsigned char longReq20[8];
extern unsigned char longReq21[8];
extern unsigned char longReq22[8];
extern unsigned char longReq23[8];
extern unsigned char longReq24[8];
extern unsigned char longReq25[8];
extern unsigned char longReq26[8];
extern unsigned char longReq27[8];


/* RPM BAR VARIABLES */
extern uint16_t yellowMin, yellowMinPx, yellowMax, yellowMaxPx, yellowFill, redMin, redMinPx, revLimitPx, redMax, redMaxPx, redFill;  // defines all the variables for making the RPM bar
/* RPM BAR VARIABLES */

/* SERIAL CONNECTION TO ARDUINO */
extern char buf[10];
extern int16_t oilTemperature, oilPressure;
/* SERIAL CONNECTION TO ARDUINO */




extern ILI9341_t3n tft ;
extern const uint8_t row1;
extern const uint8_t row2;
extern const uint8_t row3;
extern const uint8_t row4;
extern const uint8_t row5;
extern const uint8_t oilTempRow;
extern const uint8_t coolantRow;
extern const uint8_t oilPressRow;
extern const uint8_t boostRow;
extern const uint8_t intakeDamRow;
extern const uint16_t knockRow;
extern const uint16_t statusRow;

extern const uint8_t row1Lrg;
extern const uint8_t row2Lrg;
extern const uint8_t row3Lrg;





#endif // GLOBALS_H
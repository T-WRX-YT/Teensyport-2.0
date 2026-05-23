#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/* ---- PIN ASSIGNMENTS ---- */

// TFT display (ILI9341) - Teensy 4.0
#define TFT_RST 8  // short orange
#define TFT_DC  9  // brown
#define TFT_CS  10 // blue
//      MOSI    11 // tall orange
//      MISO    12 
//      SCK     13 // red

// Teensy 4.1 alternative (uncomment and comment out 4.0 above)
// #define TFT_CS  10
// #define TFT_DC  14
// #define TFT_RST 15

// Buttons
#define BUTTON1 2 // pin 2 signal -- button for changing display modes
#define BUTTON2 3 // pin 3 signal -- button for enabling diag display mode
#define BUTTON3 4 // pin 4 signal

/* ---- SERIAL CONNECTIONS ---- */

#define HWSERIAL  Serial1  // Arduino oil sensors (RX pin 0, TX pin 1)
#define HWSERIAL3 Serial3  // ESP32
#define HWSERIAL5 Serial5  // GPS

#define SERIAL_BAUD       115200
#define OIL_SERIAL_BAUD   9600
#define ESP_SERIAL_BAUD   500000

/* ---- CAN BUS ---- */

#define CAN_BAUD_RATE     500000
#define CAN_RESPONSE_ID   0x7E8
#define CAN_REQUEST_ID    0x7E0
#define RESPONSE_DATA_MAX 61

// Standalone SSM request packets
// data collected: feedback knock, fine knock, rpm, boost, coolant temp, dam, intake temp, gear, speed, afr, throttle
const unsigned char newReq0[8]  = {0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const unsigned char newReq1[8]  = {0x10, 0x47, 0xA8, 0x00, 0xFF, 0x7D, 0xA0, 0xFF}; //4 bytes of request + 2 bytes before it
const unsigned char newReq2[8]  = {0x21, 0x7D, 0xA1, 0xFF, 0x7D, 0xA2, 0xFF, 0x7D}; //7
const unsigned char newReq3[8]  = {0x22, 0xA3, 0xFF, 0x7E, 0x3C, 0xFF, 0x7E, 0x3D}; //7
const unsigned char newReq4[8]  = {0x23, 0xFF, 0x7E, 0x3E, 0xFF, 0x7E, 0x3F, 0xFF}; //7
const unsigned char newReq5[8]  = {0x24, 0x62, 0x00, 0xFF, 0x62, 0x01, 0xFF, 0x62}; //7
const unsigned char newReq6[8]  = {0x25, 0x02, 0xFF, 0x62, 0x03, 0x00, 0x00, 0x0E}; //7
const unsigned char newReq7[8]  = {0x26, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x08, 0xFF}; //7
const unsigned char newReq8[8]  = {0x27, 0x68, 0x5E, 0x00, 0x00, 0x12, 0xFF, 0x67}; //7
const unsigned char newReq9[8]  = {0x28, 0xF4, 0x00, 0x00, 0x10, 0x00, 0x00, 0x46}; //7
const unsigned char newReq10[8] = {0x29, 0x00, 0x00, 0x29, 0xFF, 0x1E, 0xE4, 0xFF}; //7
const unsigned char newReq11[8] = {0x2A, 0x1E, 0xE5, 0x00, 0x00, 0x00, 0x00, 0x00}; //2 bytes of request = 71 bytes aka 0x47

/* ---- RESPONSE TYPE CODES ---- */

#define RESP_TYPE_AP6       0   // AccessPort 6-gauge mode
#define RESP_TYPE_AP_LOG    1   // AccessPort full logging mode
#define RESP_TYPE_STANDALONE 2  // Standalone mode

#define RESP_BYTES_AP6       0x11
#define RESP_BYTES_AP_LOG    0x3D
#define RESP_BYTES_STANDALONE 0x17

/* ---- DISPLAY LAYOUT ---- */

// Row Y positions for normal/bar modes
const uint8_t row1 = 100;
const uint8_t row2 = 130;
const uint8_t row3 = 160;
const uint8_t row4 = 190;
const uint8_t row5 = 240;

// Named row aliases for display mode 3 (bar mode)
const uint8_t oilTempRow   = row1 - 50;
const uint8_t coolantRow   = row2 - 50;
const uint8_t oilPressRow  = row3 - 50;
const uint8_t ethRow       = row4 - 50;
const uint8_t boostRow     = row4 - 25;
const uint8_t diffDccdRow  = row5 - 30;
const uint8_t intakeDamRow = row5;
const uint16_t knockRow    = 270;
const uint16_t statusRow   = 310;

// Row Y positions for race mode (display mode 2)
const uint8_t row1Lrg = row1 - 40;
const uint8_t row2Lrg = row2 - 30;
const uint8_t row3Lrg = row3 - 10;

// RPM bar
#define RPM_MAX       8000
#define RPM_BAR_WIDTH 240
#define RPM_BAR_HEIGHT 40
#define REV_LIMIT_PX  204

/* ---- TEMPERATURE THRESHOLDS (Fahrenheit) ---- */

// Oil temperature ranges
#define OIL_TEMP_COLD_MAX     129   // below this = blue
#define OIL_TEMP_WARM_MIN     130   // warm-up zone = yellow
#define OIL_TEMP_WARM_MAX     159
#define OIL_TEMP_NORMAL_MIN   160   // normal operating = white
#define OIL_TEMP_NORMAL_MAX   224
#define OIL_TEMP_HOT_MIN      225   // getting hot = yellow
#define OIL_TEMP_HOT_MAX      240
#define OIL_TEMP_DANGER       241   // danger = red

// Coolant temperature ranges
#define COOL_TEMP_COLD_MAX    129
#define COOL_TEMP_WARM_MIN    130
#define COOL_TEMP_WARM_MAX    159
#define COOL_TEMP_NORMAL_MIN  160
#define COOL_TEMP_NORMAL_MAX  206
#define COOL_TEMP_HOT_MIN     207
#define COOL_TEMP_HOT_MAX     209
#define COOL_TEMP_DANGER      210

// RPM limits based on oil temperature
#define OIL_COLD_THRESHOLD    120   // oil temp <= this = cold RPM limits
#define OIL_WARMUP_THRESHOLD  160   // oil temp < this = warmup RPM limits

#define RPM_YELLOW_COLD       2500
#define RPM_RED_COLD          3000
#define RPM_YELLOW_WARMUP     2500
#define RPM_RED_WARMUP        3500
#define RPM_YELLOW_NORMAL     5000
#define RPM_RED_NORMAL        6000

/* ---- BEHAVIOR FLAGS ---- */

const bool verbose        = 0;    // prints raw CAN packet data (generates a LOT of text)
const bool printStats     = 0;    // prints gauge values after each 0x30 packet (mostly deprecated)
const bool printLoopStats = 0;    // prints gauge values when pushing to the display
const bool testData       = 0;    // generate fake data and loop it to the display
const bool sendToEsp      = 1;    // send data to ESP32 over Serial3
const bool gpsConnected   = 1;    // the gps module is installed and should be read on Serial5

const unsigned int updateInt         = 10;  // loop update interval in ms (40 = ~10hz, 10 = ~25hz)
const unsigned int displayModeNormal  = 3;    // display mode when not logging (1 = normal, 3 = normal with bars)
const unsigned int displayModeLogging = 2;    // display mode when AP is logging (2 = race)
const unsigned int displayModeDiag    = 4;    // diag mode for printing raw stats

#endif

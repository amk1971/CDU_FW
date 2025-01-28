#ifndef CONSTANT_DEFINES_H
#define CONSTANT_DEFINES_H
#include <stdint.h>

// Configuration and thresholds
static const uint8_t UPLOAD_DEFAULT = 1;  // 0 Disable and 1 for Enable
static const uint8_t CHANGE_VALUE = 1;    // Used for encoder Increment/Decrement
static const uint8_t CHANGE_KHZ = 5;
// Frequency limits
static const uint8_t STANDBY_MHZ_MAX = 117;
static const uint8_t STANDBY_MHZ_MIN = 108;
static const uint8_t STANDBY_KHZ_MAX = 95;
static const uint8_t STANDBY_KHZ_MIN = 0;
static const uint8_t VOLUME_MAX = 20;
static const uint8_t VOLUME_MIN = 0;

// Default values
static const uint8_t DEFAULT_VOLUME = 5;
static const uint8_t DEFAULT_KHZ = 90;
static const uint8_t DEFAULT_MHZ = 1;

// Combined frequency limits
static const uint16_t STANDBY_FREQ_MAX = 1799;
static const uint8_t STANDBY_FREQ_MIN = 190;

// Thresholds
static const uint8_t POLL_INTERVAL_MS = 50;            // Polling interval
static const uint16_t LONG_PRESS_THRESHOLD_MS = 3000;  // Long press threshold (3 seconds)
static const uint16_t DEBOUNCE_DELAY = 300;

// Conversion constants
static const uint8_t CONVERSION_VAL_HEX = 30;
static const uint8_t CONVERSION_VAL_DEC = 48;

// Message constants
static const char NAV_ID = 'V';  // N for NAV

static const uint8_t MAX_PAGES = 4;

static const uint8_t PRESETS_PER_PAGE =3;
static const uint8_t TOTAL_PRESETS =14;



static const float EMPTY_FREQ= 0.0 ;// Define an "empty" frequency value

// Buffer sizes and other macros
#define RX_BUFFER_SIZE 25
#define TX_BUFFER_SIZE 25
#define NAV_TX_BUFFER_SIZE 3
#define NUM_MESSAGE_ID 4


#endif  // CONSTANT_DEFINES_H

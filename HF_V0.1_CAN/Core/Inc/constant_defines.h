#ifndef CONSTANT_DEFINES_H
#define CONSTANT_DEFINES_H
#include <stdint.h>

// Configuration and thresholds

static const uint8_t CHANGE_VALUE = 1;  // Used for encoder Increment/Decrement
static const uint8_t CHANGE_KHZ = 25;
// Frequency limits
static const uint16_t STANDBY_MHZ_MAX = 399;
static const uint8_t STANDBY_MHZ_MIN = 225;
static const uint16_t STANDBY_KHZ_MAX = 975;
static const uint8_t STANDBY_KHZ_MIN = 0;
static const uint8_t VOLUME_MAX = 20;
static const uint8_t VOLUME_MIN = 0;

static const uint8_t VolumeMap[][3] = {"00", "01", "04", "07", "12", "17", "22", "27", "32", "37", "42", "47", "52", "57", "62", "67", "72", "77", "82", "87", "99"};


// Default values
static const uint8_t DEFAULT_VOLUME = 5;
static const uint8_t DEFAULT_KHZ = 90;
static const uint8_t DEFAULT_MHZ = 1;


static const uint8_t START_CURSOR_LOCATION = 0;
static const uint8_t END_CURSOR_LOCATION = 50;

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
static const char HF_ID = 'V';  // N for HF

static const uint8_t MAX_PAGES = 6;

static const uint8_t PRESETS_PER_PAGE = 3;
static const uint8_t TOTAL_PRESETS = 20;

static const float EMPTY_FREQ = 0.0;  // Define an "empty" frequency value

// Buffer sizes and other macros
#define RX_BUFFER_SIZE 25
#define TX_BUFFER_SIZE 25
#define TX_BUFFER_SIZE_INTERFACE 20
#define HF_TX_BUFFER_SIZE 3
#define NUM_MESSAGE_ID 3
#define saved_channel_SIZE 20
#define UPLOAD_DEFAULT 0
#define DECIMAL_CURSOR_POSITION 32

// 0 Disable and 1 for Enable
typedef enum
{
    FRQ,
    CH,
} FRQ_CH;

static const char *message_id_HF[NUM_MESSAGE_ID] = {
    "28",  // Standby Frequency Message ID
    "27",  // active
    "73",  // vol

};

#endif  // CONSTANT_DEFINES_H

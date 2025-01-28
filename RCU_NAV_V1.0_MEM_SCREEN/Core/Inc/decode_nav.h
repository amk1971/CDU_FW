#ifndef INC_DECODE_AFD_H_
#define INC_DECODE_AFD_H_

#include "serial_handler.h"
#include "stdbool.h"
#include "stdint.h"

#define LOCK_BIT_SHIFT 20
#define FREQ_BIT_MASK 0x7ff
#define FREQ_SHIFT 10
#define KEYS_SHIFT_MASK 0X3
#define KEY_SHIFT 8
#define VOLUME_BIT_MASK 0XFF
#define VOLUME_SHIT 0
#define ISOLATE_LOWER_21_BITS_MASK 0X1FFFFF

// Define the struct for the message
typedef struct
{
    uint8_t lock_bit : 1;
    uint16_t frequency : 11;  // 11 bits for Frequency
    uint8_t soft_keys : 2;    // 2 bits for Soft Keys
    uint8_t volume : 8;       // 8 bits for Volume
} SerialMessage;

/**
 * @brief decodes the 21 bits message received
 * return struct of serial message which has bit fields used
 * @return SerialMessage
 */
bool decode_message_NAV(uint8_t *data, SerialMessage *obj);

void fill_tx_buf_send(uint32_t t_val);
uint32_t get_23_value_to_transmit(uint16_t freq, B_Status NAV_ant, B_Status bfo, uint8_t vol);

void encode_message(uint8_t *tx_buffer, Message_ID m_index, uint8_t mhz, uint16_t khz, bool f_flag, uint8_t vol);
void decode_receive_data(uint8_t *buffer);
bool decode_message(uint8_t *rx_buffer, Identifier *ident, uint8_t *mhz, uint16_t *khz, uint8_t *f_flag, uint8_t *vol);
Message_ID get_message_ID(const char *message);

#endif /* INC_DECODE_AFD_H_ */

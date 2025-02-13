#ifndef ENCODE_HF_H
#define ENCODE_HF_H

#include <stdint.h>
#include "..\Inc\data_handler.h"

void encode_message(uint8_t *tx_buffer, Message_ID m_index, uint8_t mhz, uint16_t khz, bool f_flag, uint8_t vol);

void encode_message_for_interface(s_HF_Parameters * obj, char *tx_buffer);

#endif // ENCODE_HF_H



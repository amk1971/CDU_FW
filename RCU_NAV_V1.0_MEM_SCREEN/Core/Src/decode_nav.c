#include "decode_nav.h"

#include "constant_defines.h"
#include "nav_funcs.h"
#include "serial_handler.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "stm32f1xx.h"
#include "string.h"

uint8_t comstart[10] = "START\r\n";

const char *message_id_nav[NUM_MESSAGE_ID] =
{ "28",  // Standby Frequency Message ID
		"27",  // active
		"73",  // vol
		"100"	//Health

		};

uint8_t split_frequency_khz(uint16_t f_khz, uint8_t *khz_buf);
void calculate_checksum(const char *msg, uint8_t *checksum_high,
		uint8_t *checksum_low);
uint16_t combine_frequency_khz(uint8_t *khz_buf);

bool decode_message_NAV(uint8_t *data, SerialMessage *obj)
{
	uint8_t rx_buffer[25] =
	{ 0 };
	uint32_t raw_data = 0;
	uint8_t len = sizeof(data);
	__disable_irq();
	memcpy((char*) rx_buffer, (char*) data, len);

	raw_data |= (uint32_t) (rx_buffer[0] << 16);
	raw_data |= (uint32_t) (rx_buffer[1] << 8);
	raw_data |= (uint32_t) (rx_buffer[2] << 0);

	if (obj != NULL)
	{
		obj->frequency = (uint16_t) ((raw_data >> FREQ_SHIFT) & FREQ_BIT_MASK);
		obj->soft_keys = (uint8_t) ((raw_data >> KEY_SHIFT) & KEYS_SHIFT_MASK);
		obj->volume = (uint8_t) ((raw_data >> VOLUME_SHIT) & VOLUME_BIT_MASK);
		obj->lock_bit = (uint8_t) ((raw_data >> LOCK_BIT_SHIFT) & 0x01);
		__enable_irq();
		return true;
	}
	__enable_irq();
	return false;
}

void encode_message(uint8_t *tx_buffer, Message_ID m_index, uint8_t mhz,
		uint16_t khz, bool f_flag, uint8_t volume)
{
	uint8_t msg[20]; // Buffer for constructing the message (excluding start, checksum, and end)

	// Construct the proprietary message identifier ("$PATCcNNdd")
	if (m_index == A_FREQ || m_index == S_FREQ)
	{
		mhz -= CONVERSION_VAL_DEC;
		khz = (khz / 25);
		khz += CONVERSION_VAL_DEC;
		char khz_ascii = (char) khz;
		sprintf((char*)msg, "%s%c%cN", message_id_nav[m_index],
				(unsigned char) mhz, khz_ascii);
	}
	// Calculate checksum from the message (from 'P' to last data character)
	if (m_index == VOLUME)
	{
		volume += CONVERSION_VAL_DEC;
		sprintf((char*) msg, "%s%c", message_id_nav[m_index],
				(unsigned char) volume);
	}
	if(m_index == Health) {
		sprintf((char*) msg, "%s", message_id_nav[m_index]);
	}
	uint8_t checksum_h = 0;
	uint8_t checksum_l = 0;

	calculate_checksum((char*) msg, &checksum_h, &checksum_l);

	// Encode the checksum into two hex characters (0-9, A-F) and store the final message in TX buffer
	sprintf((char*) tx_buffer, "$PATN%c%s%c%c\r\n", (char) NAV_ID, msg,
			(checksum_h + 0x30), (checksum_l + 0x30));
}

/*
 * Function: decode_receive_data
 * Arguments: uint8_t* buffer
 *
 * Description:
 * This function decodes a received NMEA message. It creates an Identifier
 * structure and variables for MHz, kHz, and a frequency flag. The function
 * then calls decode_message to interpret the buffer contents.
 *
 * If decoding is successful, the function updates values based on the
 * class of the identifier and processes the incoming message accordingly
 *
 */

void decode_receive_data(uint8_t *buffer)
{
	// Decode the NMEA Message
	Identifier ident;
	uint8_t mhz = 0;
	uint16_t khz = 0;
	uint8_t vol = 0;
	uint8_t freq_flag = '\0';

	// Call the decode function with the message
	bool decode_success = decode_message(buffer, &ident, &mhz, &khz, &freq_flag,
			&vol);
	if ((decode_success) && (xMainNotifyQueue != NULL))
	{
		// added to the queue
		if (ident.ID == A_FREQ || ident.ID == S_FREQ)
		{
			ident.mhz = mhz;

			ident.khz = khz;
		}
		if (ident.ID == VOLUME)
		{
			ident.volume = vol;
		}

		if (xQueueSend(xMainNotifyQueue, &ident, (TickType_t)10) != pdPASS)
		{
#if DEBUG_CONSOLE
            debug_print("----Error: Failed to send Channel to queue. xMainNotifyQueue \r\n");
#endif
		}
	}
}

bool decode_message(uint8_t *rx_buffer, Identifier *ident, uint8_t *mhz,
		uint16_t *khz, uint8_t *f_flag, uint8_t *vol)
{
	char received_msg[25] =
	{ 0 };  // Buffer to extract the message data
	char message[3] =
	{ 0 };
	uint8_t received_checksum = 0, calculated_checksum = 0;
	uint8_t calculated_checksum_l;
	uint8_t calculated_checksum_h;
	size_t len = strlen((char*) rx_buffer);

	// Validate message length
	if (len < 10 || len > 24)
	{
		return false;  // Invalid length
	}

	// Copy the message into a local buffer
	memcpy(received_msg, rx_buffer, len);

	// Ensure the message starts with "$PATC" and ends with "\r\n"
	if (strncmp(received_msg, "$PATN", 5) != 0 || received_msg[len - 2] != '\r'
			|| received_msg[len - 1] != '\n')
	{
		return false;  // Invalid format
	}

	// Extract and validate the message identifier
	char cID = received_msg[5];
	if (cID != NAV_ID)
	{
		return false;  // Invalid identifier
	}

	// Extract and validate the message type
	strncpy(message, &received_msg[6], 2);
	ident->ID = get_message_ID(message);

	if (ident->ID == INVALID)
	{
		return false;  // Invalid message ID
	}

	// Extract frequency if applicable
	if (ident->ID == S_FREQ || ident->ID == A_FREQ)
	{
		*f_flag = true;
		char mhz_c, khz_c;
		sscanf(&received_msg[8], "%c%c", &mhz_c, &khz_c);
		*mhz = mhz_c + CONVERSION_VAL_DEC; // Reverse the conversion done during encoding
		int k = (khz_c - CONVERSION_VAL_DEC);
		k *= 25;
		*khz = k;  // Reverse the conversion done during encoding
	}
	if (ident->ID == VOLUME)
	{
		char volume;
		if (sscanf(&received_msg[8], "%c", &volume) == 1) // Check for successful parsing
		{
			*vol = volume - CONVERSION_VAL_DEC;  // Adjust the volume as needed
		}
		else
		{
			// Handle parsing error
			*vol = 0;  // Or set to a default value
		}
	}

	else
	{
		*f_flag = false;
	}

	// Extract the received checksum
	uint8_t received_checksum_h = received_msg[len - 4];
	uint8_t received_checksum_l = received_msg[len - 3];
	received_checksum = (
			(received_checksum_h >= 'A') ?
					(received_checksum_h - 'A' + 10) :
					(received_checksum_h - '0')) << 4
			| ((received_checksum_l >= 'A') ?
					(received_checksum_l - 'A' + 10) :
					(received_checksum_l - '0'));

	// Calculate the checksum from the message
	char checksum_data[15] =
	{ 0 };
	strncpy(checksum_data, &received_msg[6], len - 10);
	calculate_checksum(checksum_data, &calculated_checksum_h,
			&calculated_checksum_l);
	calculated_checksum = (calculated_checksum_h << 4) | calculated_checksum_l;

	// Compare checksums
	if (calculated_checksum != received_checksum)
	{
		return false;  // Checksum mismatch
	}

	return true;  // Successfully decoded
}

uint32_t get_23_value_to_transmit(uint16_t freq, B_Status NAV_ant, B_Status bfo,
		uint8_t vol)
{
	uint32_t msg = 0;
	uint16_t freq_KHz = freq;
	msg |= 1 << 21;
	msg |= ((uint32_t) freq_KHz << 10);
	msg |= ((uint32_t) NAV_ant << 9);
	msg |= ((uint32_t) bfo << 8);
	msg |= (uint32_t) inverse_volume_maping(vol);

#if DEBUG_CONSOLE
    debug_print(" message 23 Bit = %d\n", msg);
#endif

	return msg;
}

/*
 * Function: UART_StartTransmittion_NAV_Receiver
 * Arguments: None
 *
 * Description:
 * This function initiates the transmission of data over UART using interrupt mode.
 * It sets up the UART handle to transmit data from the provided buffer (`txBuffer`)
 * with the size of the buffer being transmitted. The function leverages the
 * HAL_UART_Transmit_IT function to enable interrupt-based data transmission.
 */
// void UART_StartTransmittion_NAV_Receiver(void)
//{
//	HAL_UART_Transmit_IT(&NAV_RECEIVER_COM, txBuffer_NAV_REC, sizeof(txBuffer_NAV_REC));
// }
/* ------------------------------- Receive Interrupt Service Routine --------------------------- */

/* ----------------------------------- Helper Functions ------------------------------- */

void fill_tx_buf_send(uint32_t t_val)
{
	static uint8_t tx_buffer[NAV_TX_BUFFER_SIZE];
	memset(tx_buffer, 0, NAV_TX_BUFFER_SIZE);

	tx_buffer[0] = (uint8_t) ((t_val >> 16) & 0xFF);  // Most significant 8 bits
	tx_buffer[1] = (uint8_t) ((t_val >> 8) & 0xFF);   // Middle 8 bits
	tx_buffer[2] = (uint8_t) (t_val & 0xFF);         // Least significant 8 bits

	// Transmit the bytes via UART
	if (HAL_UART_Transmit_IT(&NAV_RECEIVER_COM, tx_buffer, sizeof(tx_buffer))
			!= HAL_OK)
	{
		// Handle transmission error
#if DEBUG_CONSOLE
        debug_print("Transmission to NAV Failed\n");
        debug_print("Data = %02X %02X %02X\n", tx_buffer[0], tx_buffer[1], tx_buffer[2]);
#endif
	}
}

Message_ID get_message_ID(const char *message)
{
	Message_ID id = INVALID;

	for (int i = 0; i < NUM_MESSAGE_ID; i++)
	{
		if (strcmp(message, message_id_nav[i]) == 0)
		{
			id = i;
			break;
		}
	}

	return id;
}

/* ----------------------------------- Encoder/Decoder ------------------------------- */

/*
 * Function: calculate_checksum
 * Arguments: const char *msg, uint8_t *checksum_high, uint8_t *checksum_low
 *
 * Description:
 * This function calculates a checksum for a given message by summing all characters and
 * applying a mask to ignore carry bits. It then separates the high and low 4-bit nibbles
 * and converts them to ASCII.
 *
 * - msg: The message for which the checksum is being calculated.
 * - checksum_high: Pointer to store the high nibble of the checksum.
 * - checksum_low: Pointer to store the low nibble of the checksum.
 *
 * Process:
 * 1. Sum all characters in the message.
 * 2. Apply a mask to ignore carry bits, keeping the result within 6 bits.
 * 3. Separate the high and low 4-bit nibbles.
 */
void calculate_checksum(const char *msg, uint8_t *checksum_high,
		uint8_t *checksum_low)
{
	uint16_t checksum = 0;

	// Calculate the raw checksum by summing all characters
	while (*msg)
	{
		checksum += *msg++;
	}

	// Apply the mask to ignore the carry bits, keeping the result within 6 bits
	checksum &= 0xFF;

	// Separate the high and low 4-bit nibbles
	*checksum_high = (checksum >> 4) & 0x0F;
	*checksum_low = checksum & 0x0F;
}

uint16_t combine_frequency_khz(uint8_t *khz_buf)
{
	uint16_t res = 0;
	for (uint8_t i = 0; i < strlen((char*) khz_buf); i++)
	{
		res = res * 10 + (khz_buf[i] - '0');
	}
	return res;
}

uint8_t split_frequency_khz(uint16_t f_khz, uint8_t *khz_buf)
{
	uint16_t res = 0, size = 0;
	uint8_t tmp[5];
	if (f_khz == 0)
	{
		khz_buf[0] = '0';
		return 1;
	}

	while (f_khz != 0)
	{
		res = f_khz % 10;
		f_khz /= 10;
		tmp[size] = res;
		size++;
	}
	for (uint8_t i = 0; i < size; i++)
	{
		khz_buf[size - i - 1] = tmp[i] + '0';
	}
	return size;
}

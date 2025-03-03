#include "encode_HF.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "constant_defines.h"

static Identifier CDU_ident;

void compute_checksum(const char *message, char *checksum)
{
    uint8_t sum = 0;
    for (int i = 1; i <= 12; i++)  // Skip '$' while computing
    {
        sum += message[i];
    }

    //snprintf(checksum, 3, "%02X", sum % 256);  // Convert sum to 2-char ASCII
    checksum[0] = (sum >> 4) + '0';
    checksum[1] = (sum & 0x0F) + '0';
    checksum[2] = 0;
}
void encode_message(uint8_t *tx_buffer, Message_ID m_index, uint8_t mhz, uint16_t khz, bool f_flag, uint8_t volume)
{
    uint8_t msg[20];  // Buffer for constructing the message (excluding start, checksum, and end)

    // Construct the proprietary message identifier ("$PATCcNNdd")
    if (m_index == S_FREQ)
    {
        mhz -= CONVERSION_VAL_DEC;
        khz = (khz / 25);
        khz += CONVERSION_VAL_DEC;
        char khz_ascii = (char)khz;
        sprintf((char *)msg, "%s%c%cN", message_id_HF[m_index], (unsigned char)mhz, khz_ascii);
    }
    // Calculate checksum from the message (from 'P' to last data character)
    if (m_index == VOLUME)
    {
        volume += CONVERSION_VAL_DEC;
        sprintf((char *)msg, "%s%c", message_id_HF[m_index], (unsigned char)volume);
    }
    uint8_t checksum_h = 0;
    uint8_t checksum_l = 0;

    calculate_checksum((char *)msg, &checksum_h, &checksum_l);

    // Encode the checksum into two hex characters (0-9, A-F) and store the final message in TX buffer
    sprintf((char *)tx_buffer, "$PATN%c%s%c%c\r\n", (char)HF_ID, msg, (checksum_h + 0x30), (checksum_l + 0x30));
}
// DONE encode message for interface
void encode_message_for_interface(s_HF_Parameters *obj, char *tx_buffer)
{
	char CrLf[3]  = "\r\n";
	char checksum[3];
	//char test_tone=0;
    char volume_as[3];
    char mhz_str[4];
    char khz_str[4];

    uint16_t mhz = (uint16_t)obj->tuned_freq;
    uint16_t khz = (uint16_t)((obj->tuned_freq - mhz) * 1000);
    B_Status on_off = obj->ON_OFF;
    B_Status STOuelch = obj->STO;

    khz = ((khz+12)/25)*25;

    snprintf(volume_as, 3, "%s", VolumeMap[obj->volume]);


    mhz_str[0] = (mhz / 100) + '0';
    mhz_str[1] = ((mhz / 10) % 10) + '0';
    mhz_str[2] = (mhz % 10) + '0';
    mhz_str[3] = '\0';  // Null-terminate the string for safety

    khz_str[0] = (khz / 100) + '0';
    khz_str[1] = ((khz / 10) % 10) + '0';
    khz_str[2] = (khz % 10) + '0';
    khz_str[3] = '\0';  // Null-terminate the string for safety

    snprintf(tx_buffer, 15, "$%s%s%s%c%c%c%c", mhz_str, khz_str, volume_as, on_off + '0', STOuelch + '0',
             obj->Test + '0', obj->Mode + '0');



    // Compute checksum and append it
    compute_checksum(tx_buffer, checksum);
    strcat(tx_buffer, checksum);
    strcat(tx_buffer, CrLf);
}

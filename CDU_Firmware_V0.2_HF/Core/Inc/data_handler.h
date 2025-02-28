/*
 * data_handler.h
 *
 *  Created on: Oct 8, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_DATA_HANDLER_H_
#define INC_DATA_HANDLER_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

#define TACAN_S	5

typedef enum {
	NAV = 0,
	ADF,
	TACAN,
	HF,
	VHF,
	UHF,
	class_ID_Count
}Class_Id;

typedef enum {
	S_FREQ,
	A_FREQ,
	VOLUME,
	Health,
	MODE,
	V_T_S,
	RESET_STATUS,
	COM_ERROR,
    INVALID
}Message_Id;

typedef enum{
	MODE_CHANNEL,
	MODE_FREQUENCY
}tacan_mode;

typedef struct {
	// NAV Parameters
	float standby_nav_freq;					// NAV Standby Frequency
	float active_nav_freq;					// NAV Active Frequency
	float p1_nav_freq;						// NAV Pre-Programmed Frequency P1
	float p2_nav_freq;						// NAV Pre-Programmed Frequency P2
	float p3_nav_freq;						// NAV Pre-Programmed Frequency P3
	float p4_nav_freq;						// NAV Pre-Programmed Frequency P4
	float p5_nav_freq;						// NAV Pre-Programmed Frequency P5
	float p6_nav_freq;						// NAV Pre-Programmed Frequency P6
	float p7_nav_freq;						// NAV Pre-Programmed Frequency P7
	float p8_nav_freq;						// NAV Pre-Programmed Frequency P8
	// ADF Parameters
	uint16_t standby_adf_freq;					// ADF Standby Frequency
	uint16_t active_adf_freq;					// ADF Active Frequency
	uint16_t p1_adf_freq;						// ADF Pre-Programmed Frequency P1
	uint16_t p2_adf_freq;						// ADF Pre-Programmed Frequency P2
	uint16_t p3_adf_freq;						// ADF Pre-Programmed Frequency P3
	uint16_t p4_adf_freq;						// ADF Pre-Programmed Frequency P4
	uint16_t p5_adf_freq;						// ADF Pre-Programmed Frequency P5
	uint16_t p6_adf_freq;						// ADF Pre-Programmed Frequency P6
	uint16_t p7_adf_freq;						// ADF Pre-Programmed Frequency P7
	uint16_t p8_adf_freq;						// ADF Pre-Programmed Frequency P8
	// TACAN Frequency Parameters
	float standby_tacan_freq;				// TACAN Standby Frequency
	float active_tacan_freq;				// TACAN Active Frequency
	float p1_tacan_freq;					// TACAN Pre-Programmed Frequency P1
	float p2_tacan_freq;					// TACAN Pre-Programmed Frequency P2
	float p3_tacan_freq;					// TACAN Pre-Programmed Frequency P3
	float p4_tacan_freq;					// TACAN Pre-Programmed Frequency P4
	float p5_tacan_freq;					// TACAN Pre-Programmed Frequency P5
	float p6_tacan_freq;					// TACAN Pre-Programmed Frequency P6
	float p7_tacan_freq;					// TACAN Pre-Programmed Frequency P7
	float p8_tacan_freq;					// TACAN Pre-Programmed Frequency P8
	// HF Parameters
	float standby_hf_freq;					// HF Standby Frequency
	float active_hf_freq;					// HF Active Frequency
	float p1_hf_freq;						// HF Pre-Programmed Frequency P1
	float p2_hf_freq;						// HF Pre-Programmed Frequency P2
	float p3_hf_freq;						// HF Pre-Programmed Frequency P3
	float p4_hf_freq;						// HF Pre-Programmed Frequency P4
	float p5_hf_freq;						// HF Pre-Programmed Frequency P5
	float p6_hf_freq;						// HF Pre-Programmed Frequency P6
	float p7_hf_freq;						// HF Pre-Programmed Frequency P7
	float p8_hf_freq;						// HF Pre-Programmed Frequency P8
	float p9_hf_freq;						// HF Pre-Programmed Frequency P9
	float p10_hf_freq;						// HF Pre-Programmed Frequency P10
	float p11_hf_freq;						// HF Pre-Programmed Frequency P11
	float p12_hf_freq;						// HF Pre-Programmed Frequency P12
	float p13_hf_freq;						// HF Pre-Programmed Frequency P13
	float p14_hf_freq;						// HF Pre-Programmed Frequency P14
	float p15_hf_freq;						// HF Pre-Programmed Frequency P15
	float p16_hf_freq;						// HF Pre-Programmed Frequency P16
	float p17_hf_freq;						// HF Pre-Programmed Frequency P17
	float p18_hf_freq;						// HF Pre-Programmed Frequency P18
	float p19_hf_freq;						// HF Pre-Programmed Frequency P19


	// VHF Parameters
	float standby_vhf_freq;					// VHF Standby Frequency
	float active_vhf_freq;			 		// VHF Active Frequency
	float p1_vhf_freq;						// VHF Pre-Programmed Frequency P1
	float p2_vhf_freq;						// VHF Pre-Programmed Frequency P2
	float p3_vhf_freq;						// VHF Pre-Programmed Frequency P3
	float p4_vhf_freq;						// VHF Pre-Programmed Frequency P4
	float p5_vhf_freq;						// VHF Pre-Programmed Frequency P5
	float p6_vhf_freq;						// VHF Pre-Programmed Frequency P6
	float p7_vhf_freq;						// VHF Pre-Programmed Frequency P7
	float p8_vhf_freq;						// VHF Pre-Programmed Frequency P8
	// UHF Parameters
	float standby_uhf_freq;					// UHF Standby Frequency
	float active_uhf_freq;					// UHF Active Frequency
	float p1_uhf_freq;						// UHF Pre-Programmed Frequency P1
	float p2_uhf_freq;						// UHF Pre-Programmed Frequency P2
	float p3_uhf_freq;						// UHF Pre-Programmed Frequency P3
	float p4_uhf_freq;						// UHF Pre-Programmed Frequency P4
	float p5_uhf_freq;						// UHF Pre-Programmed Frequency P5
	float p6_uhf_freq;						// UHF Pre-Programmed Frequency P6
	float p7_uhf_freq;						// UHF Pre-Programmed Frequency P7
	float p8_uhf_freq;						// UHF Pre-Programmed Frequency P8

	// Volumes
	uint8_t volume_nav;
	uint8_t volume_adf;
	uint8_t volume_tacan;

	// TACAN Channel Parameters
	char standby_channel[TACAN_S];			// TACAN Standby Channel
	char active_channel[TACAN_S];			// TACAN Active Channel
	char	p1_channel[TACAN_S];			// TACAN Pre-Programmed Channel p1
	char	p2_channel[TACAN_S];			// TACAN Pre-Programmed Channel p2
	char	p3_channel[TACAN_S];			// TACAN Pre-Programmed Channel p3
	char	p4_channel[TACAN_S];			// TACAN Pre-Programmed Channel p4
	char	p5_channel[TACAN_S];			// TACAN Pre-Programmed Channel p5
	char	p6_channel[TACAN_S];			// TACAN Pre-Programmed Channel p6
	char	p7_channel[TACAN_S];			// TACAN Pre-Programmed Channel p7
	char	p8_channel[TACAN_S];			// TACAN Pre-Programmed Channel p8

	int     NAV_message_counter;			// Counts the time after the last nav health message
	int		TACAN_message_counter;			// Counts the time after the last tacan health message
	int		HF_message_counter;			// Counts the time after the last hf health message
	int		UHF_message_counter;			// Counts the time after the last uhf health message
	int		VHF_message_counter;			// Counts the time after the last vhf health message
	int		ADF_message_counter;			// Counts the time after the last adf health message


}CDU_Parameters;

typedef struct {
	char freq_flag; //'Y' for yes any thing else false
	uint8_t mhz;
	uint16_t khz;
	Class_Id class;
	Message_Id msg_id;
	uint8_t vol;
	tacan_mode mode;
}Identifier;

extern SemaphoreHandle_t xFlashMutex;

extern CDU_Parameters cdu_parameters;

extern Class_Id get_class_Id(void);
Message_Id get_message_Id(void);
extern void swap_active_standby_freq(Class_Id unit);
extern void swap_active_standby_channel(void);
void copy_preset_to_standby_freq(char index, Class_Id unit);
extern void copy_preset_to_standby_channel(char index);


extern void decode_receive_data(uint8_t *buffer);
extern void encode_message(uint8_t *tx_buffer, Class_Id c_index, Message_Id m_index, uint8_t mhz , uint16_t khz, uint8_t vol , uint8_t mode, char f_flag);
// Data read and write from and to flash
extern void Load_CDU_Parameters(CDU_Parameters* params);
extern void update_flash(void);

#endif /* INC_DATA_HANDLER_H_ */

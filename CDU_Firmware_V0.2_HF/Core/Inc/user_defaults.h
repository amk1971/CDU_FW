/*
 * user_defaults.h
 *
 *  Created on: Oct 21, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_USER_DEFAULTS_H_
#define INC_USER_DEFAULTS_H_


#include "data_handler.h"  // Include the structure definition

// Define default values in a constant structure
const CDU_Parameters CDU_Default_Values = {
			// NAV
			.standby_nav_freq = 108.00,
			.active_nav_freq = 113.10,
			.p1_nav_freq = 110.10,
			.p2_nav_freq = 115.32,
		  	.p3_nav_freq = 109.65,
			.p4_nav_freq = 114.00,
			.p5_nav_freq = 133.45,
			.p6_nav_freq = 110.25,
			.p7_nav_freq = 112.95,
			.p8_nav_freq = 109.90,
			//ADF
			.standby_adf_freq = 190,
			.active_adf_freq = 1799,
			.p1_adf_freq = 195,
			.p2_adf_freq = 579,
		  	.p3_adf_freq = 1678,
			.p4_adf_freq = 1290,
			.p5_adf_freq = 1345,
			.p6_adf_freq = 1755,
			.p7_adf_freq = 1449,
			.p8_adf_freq = 1195,
			// TACAN Freq
			.standby_tacan_freq = 108.55,
			.active_tacan_freq = 113.65,
			.p1_tacan_freq = 134.45,
			.p2_tacan_freq = 110.75,
			.p3_tacan_freq = 111.35,
			.p4_tacan_freq = 112.80,
			.p5_tacan_freq = 135.35,
			.p6_tacan_freq = 112.15,
			.p7_tacan_freq = 113.05,
			.p8_tacan_freq = 135.15,
			// HF
			.standby_hf_freq = 2.0000,
			.active_hf_freq  = 2.9999,
			.p1_hf_freq = 2.0009,
			.p2_hf_freq = 2.1005,
			.p3_hf_freq = 2.3757,
			.p4_hf_freq = 2.0099,
			.p5_hf_freq = 2.0195,
			.p6_hf_freq = 2.0196,
			.p7_hf_freq = 2.9550,
			.p8_hf_freq = 2.8999,
			// VHF
			.standby_vhf_freq = 120.250,
			.active_vhf_freq  = 128.275,
			.p1_vhf_freq = 131.975,
			.p2_vhf_freq = 132.975,
			.p3_vhf_freq = 133.975,
			.p4_vhf_freq = 122.450,
			.p5_vhf_freq = 122.925,
			.p6_vhf_freq = 133.675,
			.p7_vhf_freq = 131.925,
			.p8_vhf_freq = 122.850,
			// VHF
			.standby_uhf_freq = 108.500,
			.active_uhf_freq  = 117.200,
			.p1_uhf_freq = 116.150,
			.p2_uhf_freq = 109.200,
			.p3_uhf_freq = 110.350,
			.p4_uhf_freq = 115.450,
			.p5_uhf_freq = 115.525,
			.p6_uhf_freq = 109.650,
			.p7_uhf_freq = 112.775,
			.p8_uhf_freq = 114.875,


			.volume_nav  = 05,
			.volume_adf  = 05,
			.volume_tacan = 05,

			//TACAN
			.standby_channel = "22Y",
			.active_channel = "83Y",
			.p1_channel = "1Y",
			.p2_channel = "44Y",
			.p3_channel = "50Y",
			.p4_channel = "75Y",
			.p5_channel = "10Y",
			.p6_channel = "58Y",
			.p7_channel = "77Y",
			.p8_channel = "8Y"
};


#endif /* INC_USER_DEFAULTS_H_ */

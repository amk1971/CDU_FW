/*
 * tacan_map.h
 *
 *  Created on: Oct 22, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_TACAN_MAP_H_
#define INC_TACAN_MAP_H_

#include <stdint.h>


extern float find_value(const char *key);
extern int8_t find_channel(float freq, char *key);
extern void update_corresponding_frequency(const char * identifier, const char *key, CDU_Parameters *param);
extern void update_corresponding_channel(const char *identifier, float f_val, CDU_Parameters *param);


#endif /* INC_TACAN_MAP_H_ */

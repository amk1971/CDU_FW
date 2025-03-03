/*
 * data_validation.h
 *
 *  Created on: Oct 30, 2024
 *      Author: Hamza Javed
 */

#ifndef INC_DATA_VALIDATION_H_
#define INC_DATA_VALIDATION_H_

#include <stdbool.h>
#include "data_handler.h"

extern bool input_data_validation_channel(Class_Id input_class, const char* channel);
extern bool input_data_validation_freq(Class_Id input_class, float input_val);
extern bool input_volume_validation(uint8_t inp_vol);

#endif /* INC_DATA_VALIDATION_H_ */

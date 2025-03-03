/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include<stdio.h>
#include<string.h>
#include<stdbool.h>
/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */
typedef union
{
    struct {
        uint8_t Request_Reply : 1;
        uint8_t power_status  : 1;
        uint8_t Squelch       : 1;
        uint8_t TestTone      : 1;
        uint8_t Receiver_Mode : 1;
        uint8_t Volume;
        uint16_t Checksum;         // 2 bytes
        float   Frequency;         // 4 bytes
    } fields;
    uint64_t RX_TX_FRAME;  // Full 8 bytes
} FRAME;

typedef enum
{
	Request,
	Reply
}CAN_REQUEST_REPLY;

/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */
void CAN_Filter_Config(void);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void CAN_SEND_FRAME(FRAME *frame);
void CAN_RECEIVE_FRAME(FRAME *frame);
float ConvertToFrequency(int mhz, int khz);
void Can_TX_SF(bool request_reply, bool power_status, bool squelch, bool test_tone,
               bool receiver_mode, uint8_t volume, int mhz,int khz, uint16_t checksum);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */


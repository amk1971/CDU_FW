/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    can.c
 * @brief   This file provides code for the configuration
 *          of the CAN instances.
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
/* Includes ------------------------------------------------------------------*/
#include "can.h"

/* USER CODE BEGIN 0 */
uint8_t CAN_RX_Buffer[20];
/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_15TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
//void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* canHandle)
//{
//	CAN_RxHeaderTypeDef rx_header;
//	uint8_t data[20];
//
//	if (HAL_CAN_GetRxMessage(canHandle, CAN_RX_FIFO0, &rx_header, data) == HAL_OK) {
//
//		for (int i = 0; i < rx_header.DLC; i++){
//			CAN_RX_Buffer[i] = data[i];
//		}
//	}
//}
// Fucntions
void CAN_RECEIVE_FRAME(FRAME *frame)
{
	//Reply With Health Message

	if(frame->data.eID == UHF) {
		if(frame->data.mID == Health){
			CAN_SEND_FRAME(frame);		//For health message Reply with same frame
		}
	}

//    if(frame->fields.Request_Reply == Reply)
//    {
//    	//Slave Reply
//    	Can_TX_SF(Reply,0,1,0,1,99,290,725,955);
//    }
//    else
//    {
//    }

}

void CAN_SEND_FRAME(FRAME *frame) {

    uint8_t buffer[8];
    memcpy(buffer, &frame->RX_TX_FRAME, 8); // Copy frame data to buffer

    CAN_TxHeaderTypeDef TxHeader;

	uint32_t TxMailbox;

	TxHeader.DLC=8;
	TxHeader.StdId = 0x460;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.RTR = CAN_RTR_DATA;

	if(HAL_CAN_AddTxMessage(&hcan, &TxHeader, buffer, &TxMailbox)!= HAL_OK)
	{
		Error_Handler();
	}
}

void Can_TX_SF(CAN_REQUEST_REPLY request_reply, bool power_status, bool squelch, bool test_tone,
               bool receiver_mode, uint8_t volume, int mhz,int khz, uint16_t checksum)
{
	float tFreq = ConvertToFrequency(mhz,khz);
    FRAME frame = {0};
    frame.fields.Request_Reply = request_reply;
    frame.fields.power_status  = power_status;
    frame.fields.Squelch       = squelch;
    frame.fields.TestTone      = test_tone;
    frame.fields.Receiver_Mode = receiver_mode;
    frame.fields.Volume        = volume;
    frame.fields.Frequency     = tFreq;
    frame.fields.Checksum      = checksum;

    CAN_SEND_FRAME(&frame);
}

void CAN_Filter_Config(void)
{
	CAN_FilterTypeDef can1_filter_init;

	can1_filter_init.FilterActivation = ENABLE;
	can1_filter_init.FilterBank  = 0;
	can1_filter_init.FilterFIFOAssignment = CAN_RX_FIFO0;
	can1_filter_init.FilterIdHigh = 0x0000;
	can1_filter_init.FilterIdLow = 0x0000;
	can1_filter_init.FilterMaskIdHigh = 0x0000;
	can1_filter_init.FilterMaskIdLow = 0x0000;
	can1_filter_init.FilterMode = CAN_FILTERMODE_IDMASK;
	can1_filter_init.FilterScale = CAN_FILTERSCALE_32BIT;

	if( HAL_CAN_ConfigFilter(&hcan,&can1_filter_init) != HAL_OK)
	{
		Error_Handler();
	}
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t buffer[8];

    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, buffer) == HAL_OK)
    {
        FRAME RxData;
        memcpy(&RxData.RX_TX_FRAME, buffer, 8);
        CAN_RECEIVE_FRAME(&RxData);
    }
}

float ConvertToFrequency(int mhz, int khz)
{
    return (mhz + (khz / 1000.0f))+0.0005f;
}

/* USER CODE END 1 */

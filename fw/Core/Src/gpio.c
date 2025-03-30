/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LedCh_Pin|LedCg_Pin|LedCf_Pin|LedCe_Pin
                          |LedCd_Pin|LedCc_Pin|LedCb_Pin|LedCa_Pin
                          |LedA0_Pin|LedA1_Pin|LedA2_Pin|LedA3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : IMU_INT1_Pin */
  GPIO_InitStruct.Pin = IMU_INT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IMU_INT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IrRx_Pin */
  GPIO_InitStruct.Pin = IrRx_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IrRx_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BtnB_Pin BtnC_Pin BtnD_Pin BtnE_Pin
                           BtnF_Pin BtnG_Pin BtnH_Pin BtnA_Pin */
  GPIO_InitStruct.Pin = BtnB_Pin|BtnC_Pin|BtnD_Pin|BtnE_Pin
                          |BtnF_Pin|BtnG_Pin|BtnH_Pin|BtnA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LedCh_Pin LedCg_Pin LedCf_Pin LedCe_Pin
                           LedCd_Pin LedCc_Pin LedCb_Pin LedCa_Pin
                           LedA0_Pin LedA1_Pin LedA2_Pin LedA3_Pin */
  GPIO_InitStruct.Pin = LedCh_Pin|LedCg_Pin|LedCf_Pin|LedCe_Pin
                          |LedCd_Pin|LedCc_Pin|LedCb_Pin|LedCa_Pin
                          |LedA0_Pin|LedA1_Pin|LedA2_Pin|LedA3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

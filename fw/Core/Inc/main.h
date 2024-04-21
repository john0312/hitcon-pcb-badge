/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define IrIn_Pin GPIO_PIN_0
#define IrIn_GPIO_Port GPIOA
#define IrOut_Pin GPIO_PIN_0
#define IrOut_GPIO_Port GPIOB
#define LedCa_Pin GPIO_PIN_1
#define LedCa_GPIO_Port GPIOB
#define LedCb_Pin GPIO_PIN_2
#define LedCb_GPIO_Port GPIOB
#define LedCc_Pin GPIO_PIN_10
#define LedCc_GPIO_Port GPIOB
#define LedCd_Pin GPIO_PIN_11
#define LedCd_GPIO_Port GPIOB
#define LedCe_Pin GPIO_PIN_12
#define LedCe_GPIO_Port GPIOB
#define LedCf_Pin GPIO_PIN_13
#define LedCf_GPIO_Port GPIOB
#define LedCg_Pin GPIO_PIN_14
#define LedCg_GPIO_Port GPIOB
#define LedCh_Pin GPIO_PIN_15
#define LedCh_GPIO_Port GPIOB
#define UsbDm_Pin GPIO_PIN_11
#define UsbDm_GPIO_Port GPIOA
#define UsbDp_Pin GPIO_PIN_12
#define UsbDp_GPIO_Port GPIOA
#define LedA0_Pin GPIO_PIN_6
#define LedA0_GPIO_Port GPIOB
#define LedA1_Pin GPIO_PIN_7
#define LedA1_GPIO_Port GPIOB
#define LedA2_Pin GPIO_PIN_8
#define LedA2_GPIO_Port GPIOB
#define LedA3_Pin GPIO_PIN_9
#define LedA3_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

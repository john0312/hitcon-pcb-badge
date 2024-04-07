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
#define IrOut_Pin GPIO_PIN_1
#define IrOut_GPIO_Port GPIOA
#define LedCh_Pin GPIO_PIN_4
#define LedCh_GPIO_Port GPIOA
#define LedCg_Pin GPIO_PIN_5
#define LedCg_GPIO_Port GPIOA
#define LedCf_Pin GPIO_PIN_6
#define LedCf_GPIO_Port GPIOA
#define LedCe_Pin GPIO_PIN_7
#define LedCe_GPIO_Port GPIOA
#define LedCd_Pin GPIO_PIN_0
#define LedCd_GPIO_Port GPIOB
#define LedCc_Pin GPIO_PIN_1
#define LedCc_GPIO_Port GPIOB
#define LedCb_Pin GPIO_PIN_2
#define LedCb_GPIO_Port GPIOB
#define LedCa_Pin GPIO_PIN_10
#define LedCa_GPIO_Port GPIOB
#define LedRh_Pin GPIO_PIN_11
#define LedRh_GPIO_Port GPIOB
#define LedRg_Pin GPIO_PIN_12
#define LedRg_GPIO_Port GPIOB
#define LedRf_Pin GPIO_PIN_13
#define LedRf_GPIO_Port GPIOB
#define LedRe_Pin GPIO_PIN_14
#define LedRe_GPIO_Port GPIOB
#define LedRd_Pin GPIO_PIN_15
#define LedRd_GPIO_Port GPIOB
#define LedRc_Pin GPIO_PIN_8
#define LedRc_GPIO_Port GPIOA
#define LedRb_Pin GPIO_PIN_9
#define LedRb_GPIO_Port GPIOA
#define LedRa_Pin GPIO_PIN_10
#define LedRa_GPIO_Port GPIOA
#define UsbDm_Pin GPIO_PIN_11
#define UsbDm_GPIO_Port GPIOA
#define UsbDp_Pin GPIO_PIN_12
#define UsbDp_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

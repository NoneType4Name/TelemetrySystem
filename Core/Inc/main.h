/* USER CODE BEGIN Header */
/**
******************************************************************************
* @file           : main.h
* @brief          : Header for main.c file.
*                   This file contains the common defines of the application.
******************************************************************************
* @attention
*
* Copyright (c) 2026 STMicroelectronics.
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
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

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

    void HAL_TIM_MspPostInit( TIM_HandleTypeDef *htim );

    /* Exported functions prototypes ---------------------------------------------*/
    void Error_Handler( void );

    /* USER CODE BEGIN EFP */
    void my_printf( const char *fmt, ... );

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin             GPIO_PIN_3
#define LED_GPIO_Port       GPIOE
#define BUTTON_Pin          GPIO_PIN_13
#define BUTTON_GPIO_Port    GPIOC
#define BUTTON_EXTI_IRQn    EXTI15_10_IRQn
#define DCMI_PWDN_Pin       GPIO_PIN_7
#define DCMI_PWDN_GPIO_Port GPIOA
#define ESP_PW_Pin          GPIO_PIN_12
#define ESP_PW_GPIO_Port    GPIOB
#define DCMI_XCLK_Pin       GPIO_PIN_8
#define DCMI_XCLK_GPIO_Port GPIOA
#define SDMMC1_SW_Pin       GPIO_PIN_4
#define SDMMC1_SW_GPIO_Port GPIOD
#define SDMMC1_SW_EXTI_IRQn EXTI4_IRQn

    /* USER CODE BEGIN Private defines */

#define WIDTH  200 // 0-255 - TxData_T
#define HEIGHT 80  // 0-127 - TxData_T

    /* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

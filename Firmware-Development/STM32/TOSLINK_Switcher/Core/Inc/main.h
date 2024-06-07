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
#include "stm32f4xx_hal.h"

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
//void route_Control(GPIO_TypeDef* GPIOx, uint8_t config);
//void Timer_Control(STATES_T state);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SW2_S_Pin GPIO_PIN_13
#define SW2_S_GPIO_Port GPIOC
#define SW2_S_EXTI_IRQn EXTI15_10_IRQn
#define SW2_V__Pin GPIO_PIN_14
#define SW2_V__GPIO_Port GPIOC
#define SW2_V__EXTI_IRQn EXTI15_10_IRQn
#define SW2_V_C15_Pin GPIO_PIN_15
#define SW2_V_C15_GPIO_Port GPIOC
#define SW2_V_C15_EXTI_IRQn EXTI15_10_IRQn
#define LED4_Pin GPIO_PIN_0
#define LED4_GPIO_Port GPIOC
#define LED5_Pin GPIO_PIN_1
#define LED5_GPIO_Port GPIOC
#define LED6_Pin GPIO_PIN_2
#define LED6_GPIO_Port GPIOC
#define LED7_Pin GPIO_PIN_3
#define LED7_GPIO_Port GPIOC
#define SW3_S_Pin GPIO_PIN_0
#define SW3_S_GPIO_Port GPIOA
#define SW3_S_EXTI_IRQn EXTI0_IRQn
#define SW3_V__Pin GPIO_PIN_1
#define SW3_V__GPIO_Port GPIOA
#define SW3_V__EXTI_IRQn EXTI1_IRQn
#define SW3_V_A2_Pin GPIO_PIN_2
#define SW3_V_A2_GPIO_Port GPIOA
#define SW3_V_A2_EXTI_IRQn EXTI2_IRQn
#define LED9_Pin GPIO_PIN_3
#define LED9_GPIO_Port GPIOA
#define SS_T_Pin GPIO_PIN_4
#define SS_T_GPIO_Port GPIOA
#define SCLK_Pin GPIO_PIN_5
#define SCLK_GPIO_Port GPIOA
#define MISO_Pin GPIO_PIN_6
#define MISO_GPIO_Port GPIOA
#define MOSI_Pin GPIO_PIN_7
#define MOSI_GPIO_Port GPIOA
#define LED8_Pin GPIO_PIN_4
#define LED8_GPIO_Port GPIOC
#define SS_R_Pin GPIO_PIN_5
#define SS_R_GPIO_Port GPIOC
#define NPCM_INT1_T_Pin GPIO_PIN_0
#define NPCM_INT1_T_GPIO_Port GPIOB
#define ERROR_INT0_T_Pin GPIO_PIN_1
#define ERROR_INT0_T_GPIO_Port GPIOB
#define NPCM_INT1_R_Pin GPIO_PIN_2
#define NPCM_INT1_R_GPIO_Port GPIOB
#define ERROR_INT0_R_Pin GPIO_PIN_10
#define ERROR_INT0_R_GPIO_Port GPIOB
#define I2S2_LRCK_Pin GPIO_PIN_12
#define I2S2_LRCK_GPIO_Port GPIOB
#define I2S2_BCK_Pin GPIO_PIN_13
#define I2S2_BCK_GPIO_Port GPIOB
#define I2S2_DOUT_Pin GPIO_PIN_15
#define I2S2_DOUT_GPIO_Port GPIOB
#define RESET_Pin GPIO_PIN_6
#define RESET_GPIO_Port GPIOC
#define LED10_Pin GPIO_PIN_7
#define LED10_GPIO_Port GPIOC
#define LED11_Pin GPIO_PIN_8
#define LED11_GPIO_Port GPIOC
#define LED12_Pin GPIO_PIN_9
#define LED12_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_8
#define LED2_GPIO_Port GPIOA
#define SW_E_Pin GPIO_PIN_9
#define SW_E_GPIO_Port GPIOA
#define SW_S0_Pin GPIO_PIN_10
#define SW_S0_GPIO_Port GPIOA
#define SW_S1_Pin GPIO_PIN_11
#define SW_S1_GPIO_Port GPIOA
#define SW_S2_Pin GPIO_PIN_12
#define SW_S2_GPIO_Port GPIOA
#define I2S3_LRCK_Pin GPIO_PIN_15
#define I2S3_LRCK_GPIO_Port GPIOA
#define SW4_S_Pin GPIO_PIN_10
#define SW4_S_GPIO_Port GPIOC
#define SW4_S_EXTI_IRQn EXTI15_10_IRQn
#define SW4_V__Pin GPIO_PIN_11
#define SW4_V__GPIO_Port GPIOC
#define SW4_V__EXTI_IRQn EXTI15_10_IRQn
#define SW4_V_C12_Pin GPIO_PIN_12
#define SW4_V_C12_GPIO_Port GPIOC
#define SW4_V_C12_EXTI_IRQn EXTI15_10_IRQn
#define LED1_Pin GPIO_PIN_2
#define LED1_GPIO_Port GPIOD
#define I2S3_BCK_Pin GPIO_PIN_3
#define I2S3_BCK_GPIO_Port GPIOB
#define SW1_S_Pin GPIO_PIN_4
#define SW1_S_GPIO_Port GPIOB
#define SW1_S_EXTI_IRQn EXTI4_IRQn
#define I2S3_DIN_Pin GPIO_PIN_5
#define I2S3_DIN_GPIO_Port GPIOB
#define SW1_V__Pin GPIO_PIN_6
#define SW1_V__GPIO_Port GPIOB
#define SW1_V__EXTI_IRQn EXTI9_5_IRQn
#define SW1_V_B7_Pin GPIO_PIN_7
#define SW1_V_B7_GPIO_Port GPIOB
#define SW1_V_B7_EXTI_IRQn EXTI9_5_IRQn
#define LED3_Pin GPIO_PIN_8
#define LED3_GPIO_Port GPIOB
#define COP_Pin GPIO_PIN_9
#define COP_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

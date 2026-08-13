/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_hal_conf.h
  * @brief   HAL configuration file for STM32F1xx (balance car project)
  ******************************************************************************
  * @attention
  * 本文件为手动配置版本：仅启用了本工程使用的外设模块（RCC/GPIO/FLASH/PWR/CORTEX/TIM/UART）。
  * 如需新增外设，请同时启用对应模块并添加对应的 stm32f1xx_hal_xxx.c 源文件。
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __STM32F1xx_HAL_CONF_H
#define __STM32F1xx_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ########################## Module Selection ############################## */
#define HAL_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED

/* ########################## Oscillator Values adaptation ################### */
#if !defined(HSE_VALUE)
#define HSE_VALUE               8000000U /*!< Value of the External oscillator in Hz */
#endif
#if !defined(HSE_STARTUP_TIMEOUT)
#define HSE_STARTUP_TIMEOUT     100U     /*!< Time out for HSE start up */
#endif
#if !defined(HSI_VALUE)
#define HSI_VALUE               8000000U /*!< Value of the Internal oscillator in Hz */
#endif
#if !defined(HSI_STARTUP_TIMEOUT)
#define HSI_STARTUP_TIMEOUT     5000U    /*!< Time out for HSI start up */
#endif
#if !defined(LSE_VALUE)
#define LSE_VALUE               32768U   /*!< Value of the External Low speed oscillator in Hz */
#endif
#if !defined(LSE_STARTUP_TIMEOUT)
#define LSE_STARTUP_TIMEOUT     5000U    /*!< Time out for LSE start up */
#endif
#if !defined(LSI_VALUE)
#define LSI_VALUE               40000U   /*!< Value of the Internal Low speed oscillator in Hz */
#endif
#if !defined(LSI_STARTUP_TIMEOUT)
#define LSI_STARTUP_TIMEOUT     5000U    /*!< Time out for LSI start up */
#endif

/* ########################## System Configuration ########################### */
#define VDD_VALUE               3300U    /*!< Value of VDD in mv */
#define TICK_INT_PRIORITY       0U       /*!< tick interrupt priority */
#define USE_RTOS                0U
#define PREFETCH_ENABLE         1U

/* ########################## Assert Selection ############################## */
#define USE_FULL_ASSERT         0U

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_cortex.h"
#include "stm32f1xx_hal_flash.h"
#include "stm32f1xx_hal_pwr.h"
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_hal_tim.h"
#include "stm32f1xx_hal_uart.h"

#ifdef USE_FULL_ASSERT
#define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
void assert_failed(uint8_t *file, uint32_t line);
#else
#define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1xx_HAL_CONF_H */

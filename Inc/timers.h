/**
  ******************************************************************************
  * @file    timers.h
  * @brief   Файл, содержащий настройки и функции таймеров
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIMERS_H
#define __TIMERS_H

#ifdef __cplusplus
 extern "C" {
#endif
	 
#include "stm32f1xx_hal.h"

void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_TIM5_Init(void);

uint32_t ReadHighPrecisionCounter (void);
void ClearHighPrecisionCounter(void);
void WaitTimeMCS(uint32_t lTimeMS);
	 
uint32_t ReadCMX7262TimerCounter(void);
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIMERS_H */


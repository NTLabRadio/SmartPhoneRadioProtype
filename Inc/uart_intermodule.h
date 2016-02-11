/**
  ******************************************************************************
  * @file    uart_intermodule.h
  * @brief   Файл, содержащий заголовки функций, реализующих межмодульный обмен
	*						по интерфейсу UART
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_INTERMODULE_H
#define __UART_INTERMODULE_H

#include "slipinterface.h"

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"
#include <string.h>
#include "DebugLog.h"
	 
	 
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
	 
void UART_InitInterface(UART_HandleTypeDef *huart);
	 
#ifdef DEBUG_UART_TEST_TXRX	 
void UART_Receive_Pck_Wait(UART_HandleTypeDef *huart, uint8_t *pBufForRecData);	 
void UART_Send_TestData(UART_HandleTypeDef *huart);
#endif
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UART_INTERMODULE_H */

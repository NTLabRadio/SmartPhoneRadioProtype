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

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include	"stm32f1xx_hal_uart.h"
	 
	 
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void UART_Receive_Pck_Wait(UART_HandleTypeDef *huart, uint8_t *pBufForRecData);	 

void UART_Send_TestData(UART_HandleTypeDef *huart);
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UART_INTERMODULE_H */

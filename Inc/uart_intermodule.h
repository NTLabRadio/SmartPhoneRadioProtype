/**
  ******************************************************************************
  * @file    uart_intermodule.h
  * @brief   ����, ���������� ��������� �������, ����������� ������������ �����
	*						�� ���������� UART
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
	 
	 
//��������� ��������� ��������� ��������� UART-���������
typedef enum 
{
	UART_IDLE,
	UART_DATA_RX_NEED_TO_PROCESS
} en_UARTstates;
	 

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
	 
void UART_InitInterface(UART_HandleTypeDef *huart);
void UART_DeInitInterface(UART_HandleTypeDef *huart);

void InitSerialProtocol(void);
void DeinitSerialProtocol(void);

void WaitNextByteFromUART(UART_HandleTypeDef *huart);

uint8_t CheckForSerialProtocolData(uint8_t* pPayloadPackData, uint16_t& nSizePackData);

void SendDataToExtDev(uint8_t* pData, uint16_t nSizeData);
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UART_INTERMODULE_H */

#include "uart_intermodule.h"


#define SIZE_OF_UART_REC_BUFFER (32)
uint8_t receiveUARTBuffer[SIZE_OF_UART_REC_BUFFER];

#define SIZE_OF_PCK (8)

int16_t cntRecPcks = -1;


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
	
	cntRecPcks++;
			
	if(cntRecPcks>=SIZE_OF_UART_REC_BUFFER/SIZE_OF_PCK)
		cntRecPcks=0;
	
	UART_Receive_Pck_Wait(huart, receiveUARTBuffer+cntRecPcks*SIZE_OF_PCK);	
	
	UART_Send_TestData(huart);
}


void UART_Receive_Pck_Wait(UART_HandleTypeDef *huart, uint8_t *pBufForRecData)
{
	if(pBufForRecData)
		HAL_UART_Receive_DMA(huart,pBufForRecData,SIZE_OF_PCK);
	else
		HAL_UART_Receive_DMA(huart,receiveUARTBuffer,SIZE_OF_PCK);
}


void UART_Send_TestData(UART_HandleTypeDef *huart)
{
	//uint8_t pBufTestData[8] = {0x00, 0x06, 0x48, 0x00, 0x00, 0xBE, 0x00, 0x00};
	uint8_t pBufTestData[8] = {0x00, 0x06, 0x00, 0x00, 0x48, 0x00, 0x00, 0xBE};
		
	HAL_UART_Transmit_DMA(huart, pBufTestData, 8);
}

#include "uart_intermodule.h"


#define SIZE_OF_UART_RX_BUFFER (32)
uint8_t pUARTRxBuf[SIZE_OF_UART_RX_BUFFER];

#define MAX_SIZE_OF_SLIP_PACK_PAYLOAD	(128+4)		// В соответствие с межмодульным протоколом SPIM (Smart Phone InterModule) 
																								// - максимальная длина полезных данных - 128 байт;
																								// - размер служебных данных в пакете (заголовок, CRC) - 4 байта
uint8_t pDataSLIPPack[MAX_SIZE_OF_SLIP_PACK_PAYLOAD];


#define SIZE_OF_PCK (8)

int16_t cntRecPcks = -1;

SLIPInterface* objSLIPinterface;


void UART_InitInterface(UART_HandleTypeDef *huart)
{
	HAL_UART_Receive_IT(huart,pUARTRxBuf,1);

	objSLIPinterface = new SLIPInterface;

	//Переходим в режим ожидадания пакета
	memset(pDataSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
	objSLIPinterface->WaitForPack();
	//устанавливаем состояние SLIP-интерфейса - ожидание начала SLIP-пакета
}

void UART_DeInitInterface(UART_HandleTypeDef *huart)
{
	delete objSLIPinterface;
}






void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint16_t nSizeSLIPPack = 0;	
	uint8_t nCheckState = 0;
	
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
	
	//Обрабатываем принятый символ
	if(objSLIPinterface->CheckForSLIPData(pUARTRxBuf[0], &pDataSLIPPack[0], nSizeSLIPPack, nCheckState))
	{
		#ifdef DEBUG_PRINTF_SLIP_DATA
		printf("SLIP Pack is Rcvd\n * Payload of Pack:");
		
		ConvertHexIntToUTF8(pDataSLIPPack,nSizeSLIPPack);
		printf((const char*)pDataSLIPPack);
		
		printf("\n");
		#endif
		
		//Ожидаем следующий пакет
		memset(pDataSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
		objSLIPinterface->WaitForPack();
	}
	
	//Ожидаем следующий символ
	HAL_UART_Receive_IT(huart,pUARTRxBuf,1);
	
	#ifdef DEBUG_UART_TEST_TXRX
	cntRecPcks++;
			
	if(cntRecPcks>=SIZE_OF_UART_RX_BUFFER/SIZE_OF_PCK)
		cntRecPcks=0;
	
	UART_Receive_Pck_Wait(huart, receiveUARTBuffer+cntRecPcks*SIZE_OF_PCK);	
	
	UART_Send_TestData(huart);
	#endif
}

#ifdef DEBUG_UART_TEST_TXRX
void UART_Receive_Pck_Wait(UART_HandleTypeDef *huart, uint8_t *pBufForRecData)
{
	if(pBufForRecData)
		HAL_UART_Receive_DMA(huart,pBufForRecData,SIZE_OF_PCK);
	else
		HAL_UART_Receive_DMA(huart,receiveUARTBuffer,SIZE_OF_PCK);
}

void UART_Send_TestData(UART_HandleTypeDef *huart)
{
	uint8_t pBufTestData[8] = {0x00, 0x06, 0x00, 0x00, 0x48, 0x00, 0x00, 0xBE};
		
	HAL_UART_Transmit_DMA(huart, pBufTestData, 8);
}
#endif

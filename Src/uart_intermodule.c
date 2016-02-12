#include "uart_intermodule.h"


#define SIZE_OF_UART_RX_BUFFER (32)
uint8_t pUARTRxBuf[SIZE_OF_UART_RX_BUFFER];

#define MAX_SIZE_OF_SLIP_PACK_PAYLOAD	(128+4)		// В соответствие с межмодульным протоколом SPIM (Smart Phone InterModule) 
																								// - максимальная длина полезных данных - 128 байт;
																								// - размер служебных данных в пакете (заголовок, CRC) - 4 байта
uint8_t pDataSLIPPack[MAX_SIZE_OF_SLIP_PACK_PAYLOAD];

uint8_t UTF8DataSLIPPack[2*MAX_SIZE_OF_SLIP_PACK_PAYLOAD];

uint8_t pUARTTxBuf[2*MAX_SIZE_OF_SLIP_PACK_PAYLOAD];
uint16_t nSizeTxBuf;

#define SIZE_OF_PCK (8)

int16_t cntRecPcks = -1;

SLIPInterface* objSLIPinterface;


/**
  * @brief  Функция инициализации приема данных по интерфейсу UART
	* @param  hspi - handle UART-интерфейса, для которого выполняется функция
	*
  * @note   Функция выполняет настройку интерфейса на прием данных в
	*					соответствии с протоколом SLIP. Функции SLIP-интерфейса
	*					реализованы в классе SLIPInterface, объект которого создается
	*					в UART_InitInterface(). Функция настраивает UART на посимвольный
	*					прием данных для дальнейшей их передачи в SLIP-интерфейс
	*
	* @retval void
  */
void UART_InitInterface(UART_HandleTypeDef *huart)
{
	//Настраиваем прерывание на событие приема одного байта из UART
	HAL_UART_Receive_IT(huart,pUARTRxBuf,1);

	//Создаем объект для SLIP-интерфейса
	objSLIPinterface = new SLIPInterface;

	//Переводим SLIP-интерфейс в режим ожидания SLIP-пакета
	memset(pDataSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
	objSLIPinterface->WaitForPack();
	
	//устанавливаем состояние SLIP-интерфейса - ожидание SLIP-пакета
}

/**
  * @brief  Функция деинициализации приема данных по интерфейсу UART
	* @param  hspi - handle UART-интерфейса, для которого выполняется функция
	*
  * @note   Функция выполняет удаление объекта SLIP-интерфейса, "привязанного"
	*					к интерфейсу UART
	* @retval void
  */
void UART_DeInitInterface(UART_HandleTypeDef *huart)
{
	delete objSLIPinterface;
}


/**
  * @brief  Callback-функция, вызываемая по прерыванию, извещаюшего о том, 
	*					что приняты данные от интерфейса UART
	* @param  hspi - handle UART-интерфейса
	*
  * @note   Функция переадресует принятые данные SLIP-интерфейсу, который в
	*					случае нахождения полезных данных, извлекает их из протокола SLIP
	*
	* @retval void
  */
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
		
		memcpy(UTF8DataSLIPPack,pDataSLIPPack,nSizeSLIPPack);
		
		ConvertHexIntToUTF8(UTF8DataSLIPPack,nSizeSLIPPack);
		printf((const char*)UTF8DataSLIPPack);
		
		printf("\n");
		#endif

		#ifdef DEBUG_SEND_RCVD_SLIP_PACK
		objSLIPinterface->FormPack(pDataSLIPPack, nSizeSLIPPack, pUARTTxBuf, nSizeTxBuf);

			#ifdef DEBUG_PRINTF_SLIP_DATA
			printf("SLIP Pack Ready for Send\n * Pack Data:");

			memcpy(UTF8DataSLIPPack,pUARTTxBuf,nSizeTxBuf);
			
			ConvertHexIntToUTF8(UTF8DataSLIPPack,nSizeTxBuf);
			printf((const char*)UTF8DataSLIPPack);
			
			printf("\n");
			#endif

		HAL_UART_Transmit_DMA(huart, pUARTTxBuf, nSizeTxBuf);
		#endif
		
		//Ожидаем следующий пакет
		memset(pDataSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
		objSLIPinterface->WaitForPack();
		
	}
	
	//Ожидаем следующий символ
	HAL_UART_Receive_IT(huart,pUARTRxBuf,1);

}

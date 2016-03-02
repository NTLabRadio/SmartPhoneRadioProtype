#include "uart_intermodule.h"

//Параметры UART по умолчанию
UART_InitTypeDef DefaultUARTParams = {115200,
																			UART_WORDLENGTH_8B,
																			UART_STOPBITS_1,
																			UART_PARITY_NONE,
																			UART_MODE_TX_RX,
																			UART_HWCONTROL_NONE,
																			UART_OVERSAMPLING_16
																			};


//Размер буфера для приема данных от UART
#define SIZE_OF_UART_RX_BUFFER (32)
//Буфер для приема данных от UART
uint8_t pUARTRxBuf[SIZE_OF_UART_RX_BUFFER];

//Буфер, в который копируются полезные данные SLIP-пакета, принятого из UART 
uint8_t pUARTRxSLIPPack[MAX_SIZE_OF_SLIP_PACK_PAYLOAD];
//Размер полезных данных SLIP-пакета, принятого из UART
uint16_t nSizeSLIPPack;	

#ifdef DEBUG_PRINTF_SLIP_DATA
uint8_t UTF8DataSLIPPack[2*MAX_SIZE_OF_SLIP_PACK_PAYLOAD];
#endif


//Буфер полезных данных для передачи в UART
uint8_t pUARTTxPayload[MAX_SIZE_OF_SLIP_PACK_PAYLOAD];
//Размер буфера полезных данных на передачу в UART
uint16_t nSizeTxPayload;
																			
//Буфер для передачи данных в UART
uint8_t pUARTTxBuf[(MAX_SIZE_OF_SLIP_PACK_PAYLOAD*3)/2];
//Размер пакета на передачу в UART
uint16_t nSizeTxBuf;

//Объект для работы с SLIP-интерфейсом
SLIPInterface* objSLIPinterface;

//Текущее состояние механизма обработки UART-сообщений
en_UARTstates UARTstate;

//Handle UART-интерфейса внешнего управляющего модуля
UART_HandleTypeDef *huartExtDev;



/**
  * @brief  Функция инициализации приема данных по интерфейсу UART
	*
	* @param  hspi - handle UART-интерфейса, для которого выполняется функция
	*
  * @note   Функция выполняет настройку интерфейса на прием данных:
	*					инициализирует объекты и переменные логического протокола;
	* 				настраивает UART на прием данных и устанавливает начальное 
	*					состояние механизма обработки UART-сообщений
	*
	* @retval void
  */
void UART_InitInterface(UART_HandleTypeDef *huart)
{
	huartExtDev = huart;
	
	//Инициализируем объекты и переменные логического протокола 
	//последовательного порта
	InitSerialProtocol();
	
	//Ожидаем данных из UART
	WaitNextByteFromUART(huart);
	
	//Устанавливаем начальное состояние механизма обработки UART-сообщений
	UARTstate = UART_IDLE;
	
	//устанавливаем состояние SLIP-интерфейса - ожидание SLIP-пакета
}

/**
  * @brief  Функция деинициализации приема данных по интерфейсу UART
	*
	* @param  hspi - handle UART-интерфейса, для которого выполняется функция
	*
  * @note   Функция выполняет удаление объекта SLIP-интерфейса, "привязанного"
	*					к интерфейсу UART
	* @retval void
  */
void UART_DeInitInterface(UART_HandleTypeDef *huart)
{
	huartExtDev = NULL;
	
	delete objSLIPinterface;
}


/**
  * @brief  Функция инициализации объектов и переменных логического протокола
	*					последовательного порта
	* @param  нет
  * @note   Функция выполняет настройку интерфейса на прием данных в
	*					соответствии с протоколом SLIP. Функции SLIP-интерфейса
	*					реализованы в классе SLIPInterface, объект которого создается
	*					в InitSerialProtocol()
	* @retval void
  */
void InitSerialProtocol()
{
	//Создаем объект для SLIP-интерфейса
	objSLIPinterface = new SLIPInterface;

	//Переводим SLIP-интерфейс в режим ожидания SLIP-пакета
	memset(pUARTRxSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
	nSizeSLIPPack = 0;
	objSLIPinterface->WaitForPack();
}

/**
  * @brief  Функция деинициализации объектов и переменных логического протокола
	*					последовательного порта
	* @retval void
  */
void DeinitSerialProtocol()
{
	delete objSLIPinterface;
}


/**
  * @brief  Callback-функция, вызываемая по прерыванию, извещаюшего о том, 
	*					что приняты данные от интерфейса UART
	*
	* @param  huart - handle UART-интерфейса
	*
  * @note   Функция переадресует принятые данные обработчику внутреннего
	*					логического интерфейса (SLIP-интерфейс), который в случае 
	*					нахождения полезных данных, извещает об этом
	*
	* @retval void
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart!=huartExtDev)
		return;
	
	//Передаем принятые данные обработчику внутреннего логического интерфейса
	if(CheckForSerialProtocolData(pUARTRxSLIPPack,nSizeSLIPPack))
	{//Если найдены полезные данные
		#ifdef DEBUG_PRINTF_SLIP_DATA
		printf("SLIP Pack is Rcvd\n * Payload of Pack:");
		
		memcpy(UTF8DataSLIPPack,pUARTRxSLIPPack,nSizeSLIPPack);
		
		ConvertHexIntToUTF8(UTF8DataSLIPPack,nSizeSLIPPack);
		printf((const char*)UTF8DataSLIPPack);
		
		printf("\n");
		#endif

		#ifdef DEBUG_SEND_RCVD_SLIP_PACK
		objSLIPinterface->FormPack(pUARTRxSLIPPack, nSizeSLIPPack, pUARTTxBuf, nSizeTxBuf);

			#ifdef DEBUG_PRINTF_SLIP_DATA
			printf("SLIP Pack Ready for Send\n * Pack Data:");

			memcpy(UTF8DataSLIPPack,pUARTTxBuf,nSizeTxBuf);
			
			ConvertHexIntToUTF8(UTF8DataSLIPPack,nSizeTxBuf);
			printf((const char*)UTF8DataSLIPPack);
			
			printf("\n");
			#endif

		HAL_UART_Transmit_DMA(huart, pUARTTxBuf, nSizeTxBuf);
		#endif
		
		//Указываем, что принятые данные необходимо обработать
		UARTstate = UART_DATA_RX_NEED_TO_PROCESS;
		
		//Ожидаем следующий пакет
		//memset(pUARTRxSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
		objSLIPinterface->WaitForPack();
	}
	
	//Ожидаем следующий символ
	WaitNextByteFromUART(huart);

}


/**
  * @brief  Настройка UART-интерфейса на ожидание следующей порции данных
	*/
void WaitNextByteFromUART(UART_HandleTypeDef *huart)
{
	//Ожидаем следующий 
	HAL_UART_Receive_IT(huart,pUARTRxBuf,1);
}


/**
  * @brief  Поиск в потоке данных, принятых из UART, полезных данных,
	*					в формате используемого протокола для последовательного порта
	*
  * @note   Функция побайтно анализирует потоковые данные, которые 
	*					забирает по указателю pUARTRxBuf. В случае нахождения 
	*					начала и конца пакета, функция возвращает код успешного
	*					выполнения процедуры поиска и передает принятые полезные данные
	*					по указателю pUARTRxSLIPPack
	*
	* @retval Результат поиска:
	*					0 - SLIP-пакет не принят;
	*					1 - SLIP-пакет принят полностью;
	*/
uint8_t CheckForSerialProtocolData(uint8_t* pPayloadPackData, uint16_t& nSizePackData)
{
	uint8_t nCheckState = 0;
	nSizePackData = 0;
	
	//TODO Полезные данные копируются в один и тот же буфер, но обработка этих данных может занимать прилично
	// времени. Поэтому крайне желательно их складировать в очередь
	return(objSLIPinterface->CheckForSLIPData(*pUARTRxBuf, pPayloadPackData, nSizePackData, nCheckState));
}

/**
  * @brief  Функция передачи полезных данных внешнему устройству управления
	*
	* @param  pData - указатель на полезные данные, которые должны быть переданы в UART
	* @param  nSizeData - размер полезных данных, байт
	*
  * @note   Функция из полезных данных формирует SLIP-пакет и передает его в UART
	*
	* @retval нет
	*/
void SendDataToExtDev(uint8_t* pData, uint16_t nSizeData)
{
		memcpy(pUARTTxPayload,pData,nSizeData);
		nSizeTxPayload = nSizeData;
		objSLIPinterface->FormPack(pUARTTxPayload, nSizeTxPayload, pUARTTxBuf, nSizeTxBuf);

			#ifdef DEBUG_PRINTF_SLIP_DATA
			printf("SLIP Pack Ready for Send\n * Pack Data:");

			memcpy(UTF8DataSLIPPack,pUARTTxBuf,nSizeTxBuf);
			
			ConvertHexIntToUTF8(UTF8DataSLIPPack,nSizeTxBuf);
			printf((const char*)UTF8DataSLIPPack);
			
			printf("\n");
			#endif

		HAL_UART_Transmit_DMA(huartExtDev, pUARTTxBuf, nSizeTxBuf);		
}


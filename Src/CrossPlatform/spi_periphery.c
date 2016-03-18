#include "spi_periphery.h"

extern SPI_HandleTypeDef hspi1;


uint8_t Cplt_SPI_TransmitReceive[NUM_OF_SPI_INTERFACES];



void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance==SPI1)
	{
		Cplt_SPI_TransmitReceive[INTERFACE_SPI1] = TRUE;
	}
	else if(hspi->Instance==SPI2)
	{
		Cplt_SPI_TransmitReceive[INTERFACE_SPI2] = TRUE;
	}
}


//Cброс состояния успешного окончания транзакции SPI
void ResetCpltState_SPI_TransmitReceive(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance==SPI1)
	{
		Cplt_SPI_TransmitReceive[INTERFACE_SPI1] = FALSE;
	}
	else if(hspi->Instance==SPI2)
	{
		Cplt_SPI_TransmitReceive[INTERFACE_SPI2] = FALSE;
	}
}	
	
	
/* Проверка завершения транзакции SPI */
uint8_t isCplt_SPI_TransmitReceive(SPI_HandleTypeDef *hspi)
{
	if( ((hspi->Instance==SPI1)&&Cplt_SPI_TransmitReceive[INTERFACE_SPI1]) ||
			((hspi->Instance==SPI2)&&Cplt_SPI_TransmitReceive[INTERFACE_SPI2]) )
		return(1);
	else
		return(0);
}


/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received
	* @param  hspi : Handle of SPI Interface
	* 				nByteForTX : Byte To Send.
	*					nByteForRX : pointer to Received Byte Value
  * @retval Result of Transmission
  */
HAL_StatusTypeDef SPI_TransmitRecieveByte(SPI_HandleTypeDef *hspi, uint8_t nByteForTX, uint8_t *nByteForRX)
{
	HAL_StatusTypeDef nRes = HAL_OK; // начальное состояние HAL. 
	
	#ifndef WAIT_END_OF_SPI_TRNSACTION_BY_TIM_COUNTER
	uint32_t cntWaitEndOfTransaction;
	#endif
	
	ResetCpltState_SPI_TransmitReceive(hspi);
	
	//Передаем 1 байт и в это же время принимаем
	//Если что-то не так (например, занята шина), возвращаем 0	
	//nRes=HAL_SPI_TransmitReceive_DMA(hspi, &nByteForTX, nByteForRX, 1);
	nRes=HAL_SPI_TransmitReceive_IT(hspi, &nByteForTX, nByteForRX, 1);
	
	if(nRes)
	{
		#ifdef DEBUG_PRINTF_SPI_EXCHANGE
		printf("ERROR In SPI Exchange: HAL_Status=%d\n",nRes);
		#endif
		return(nRes);
	}

	/* Ожидание окончания транзакции */
	
	//Wait end of transmission for 100 mcs
	#ifdef WAIT_END_OF_CBUS_TRANSACTION_BY_TIM_COUNTER
	ClearHighPrecisionCounter();
	while(ReadHighPrecisionCounter() < 1e2)
	#else
	cntWaitEndOfTransaction = 1e6;
	while(cntWaitEndOfTransaction)
	#endif
	{
		cntWaitEndOfTransaction--;
		if(isCplt_SPI_TransmitReceive(hspi))
		{
			return nRes;
		}
	}
	
	SPI_TIMEOUT_UserCallback(hspi);
		
	return HAL_TIMEOUT;
}


void SPI_TIMEOUT_UserCallback(SPI_HandleTypeDef *hspi)
{	
	#ifdef DEBUG_PRINTF_SPI_EXCHANGE
	printf("ERROR In SPI Exchange: No Answer From Slave\n");
	#endif
}


/**
  * @brief  Sends a number of Bytes through the SPI interface and return the number of Bytes received
	* @param  hspi : Handle of SPI Interface
	* 				pTxData: pointer to transmission data buffer
	*					pRxData: pointer to reception data buffer to be
  *					Size: amount of data to be sent

  * @retval Result of Transmission (HAL Status)
  */
HAL_StatusTypeDef SPI_TransmitRecieve(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size)
{
	HAL_StatusTypeDef nRes = HAL_OK; // начальное состояние HAL. 
	
	#ifndef WAIT_END_OF_SPI_TRNSACTION_BY_TIM_COUNTER // если меряем таймаут прецезионным таймером
	uint32_t cntWaitEndOfTransaction;
	#endif
	
	ResetCpltState_SPI_TransmitReceive(hspi); //Cброс состояния успешного окончания транзакции SPI 
	
	//Передаем Size байтов и в это же время принимаем
	//Если что-то не так (например, занята шина), возвращаем 0	
	nRes=HAL_SPI_TransmitReceive_IT(hspi, pTxData, pRxData, Size);
	
	if(nRes) // если nRes отличен от нуля (HAL не OK)
	{
		#ifdef DEBUG_PRINTF_SPI_EXCHANGE
		printf("ERROR In SPI Exchange: HAL_Status=%d\n",nRes);
		#endif
		return(nRes);
	}

	/* Ожидание окончания транзакции */
	
	//Wait end of transmission for 100 mcs
	#ifdef WAIT_END_OF_CBUS_TRANSACTION_BY_TIM_COUNTER // если используем прецезионный таймер
	ClearHighPrecisionCounter();
	while(ReadHighPrecisionCounter() < 1e2)
	#else
	cntWaitEndOfTransaction = 1e6; // или просто вычитаем до 0 
	while(cntWaitEndOfTransaction)
	#endif
	{
		cntWaitEndOfTransaction--;
		if(isCplt_SPI_TransmitReceive(hspi)) // если SPI передача/прием завершены, возвращаем HAL_OK в nRes
		{
			return nRes;
		}
	}
	
	SPI_TIMEOUT_UserCallback(hspi); // если до сих пор функция не закнчилась (return не сработал)
		
	return HAL_TIMEOUT; // возвращаем ошибку таймаут по SPI
}





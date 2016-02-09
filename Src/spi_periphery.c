#include "spi_periphery.h"

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;

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
	HAL_StatusTypeDef nRes = HAL_OK;
	
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
		printf("ERROR In SPI Exchange: HAL_Status=%d\n",nRes);
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
	printf("ERROR In SPI Exchange: No Answer From Slave\n");
}

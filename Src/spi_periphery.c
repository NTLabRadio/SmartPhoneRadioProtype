#include "spi_periphery.h"

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

uint8_t Cplt_SPI_TransmitReceive[NUM_OF_SPI_INTERFACES];

#define SPI_TIME_OUT         ((uint32_t)0x1000000)
__IO uint32_t  SPITimeout = SPI_TIME_OUT;


/* SPI1 init function */
void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;		// режим работы: двухпроводный full duplex
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;				// размер данных - 8 бит
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;					// синхронизация по заднему фронту
  hspi1.Init.NSS = SPI_NSS_SOFT;									// программный CS (аппаратный (SPI_NSS_HARD_OUTPUT) не понятно, как задействовать)
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;	//предделитель частоты SPI
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;					// старший бит - первый
  hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  hspi1.Init.CRCPolynomial = 7;
  HAL_SPI_Init(&hspi1);
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
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







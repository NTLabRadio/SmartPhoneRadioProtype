#include "cc1120.h"

SPI_HandleTypeDef *hspiCC1120 = NULL;

uint8_t pCC1120TxData[255];
uint8_t pCC1120RxData[255];


uint8_t CC1120_CheckModule(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	pCC1120TxData[0] = 0xAF;	pCC1120TxData[1] = 0x8F; pCC1120TxData[2] = 0x00;
	pCC1120TxData[3] = 0xAF;	pCC1120TxData[4] = 0x90; pCC1120TxData[5] = 0x00;	
	
	//Опускаем CS	
	CC1120_CSN_LOW();
	
	//Передаем данные и одновременно принимаем ответ
	HAL_SPI_TransmitReceive_DMA(hspiCC1120, pCC1120TxData, pCC1120RxData, 6);
	//HAL_SPI_TransmitReceive_IT(hspiCC1120, pCC1120TxData, pCC1120RxData, 6);
	
	// Wait for a 0.1 second.
	WaitTimeMCS(1e5);
	
	//Поднимаем CS	
	CC1120_CSN_HIGH();
	
	return(1);
}

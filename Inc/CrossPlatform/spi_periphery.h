/**
  ******************************************************************************
  * @file    spi_periphery.h
  * @brief   Файл, содержащий все необходимое для работы с периферийными
	*					 устройствами макета радиомодуля для смартфона на базе
	*					 SDR-демонстратора DE9943
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_PERIPHERY_H
#define __SPI_PERIPHERY_H

#ifdef __cplusplus
 extern "C" {
#endif

#ifdef STM32F071xB
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_spi.h"
#endif
	 
#ifdef STM32F103xE	 
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"	 
#endif
	 
#include "globals.h"
#include "timers.h"
	 
typedef enum
{
  DEVICE_CMX7262      = 0x00,
  DEVICE_CC1120      	= 0x01,
	NUM_OF_PERIPH_DEVICES
} HandlePeriphDeviceTypeDef;

typedef enum
{
	INTERFACE_SPI1,
	INTERFACE_SPI2,
	NUM_OF_SPI_INTERFACES
} InterfaceSPITypeDef;


void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi);

void ResetCpltState_SPI_TransmitReceive(SPI_HandleTypeDef *hspi);
uint8_t isCplt_SPI_TransmitReceive(SPI_HandleTypeDef *hspi);

HAL_StatusTypeDef SPI_TransmitRecieveByte(SPI_HandleTypeDef *hspi, uint8_t nByteForTX, uint8_t *nByteForRX);
HAL_StatusTypeDef SPI_TransmitRecieve(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size);
void SPI_TIMEOUT_UserCallback(SPI_HandleTypeDef *hspi);
	 	 
 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SPI_PERIPHERY_H */

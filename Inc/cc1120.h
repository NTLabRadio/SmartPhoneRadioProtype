/**
  ******************************************************************************
  * @file    spi_cc1120.h
  * @brief   Файл, содержащий заголовки функций, реализующих обмен котроллера
	*						с микросхемой трансивера по интерфейсу SPI
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_CC1120_H
#define __SPI_CC1120_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"
#include "spi_periphery.h"
#include "timers.h"

uint8_t CC1120_CheckModule(SPI_HandleTypeDef *hspi);
	 

uint8_t CC1120_Write (uint8_t uGenAddress, uint8_t uExtAddress, uint8_t bBurst, uint16_t *data_ptr, uint16_t uAccesses);
uint8_t CC1120_Read (uint8_t uGenAddress, uint8_t uExtAddress, uint8_t bBurst, uint16_t *data_ptr, uint16_t uAccesses);

	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SPI_CC1120_H */

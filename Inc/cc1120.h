/**
  ******************************************************************************
  * @file    spi_cc1120.h
  * @brief   ����, ���������� ��������� �������, ����������� ����� ����������
	*						� ����������� ���������� �� ���������� SPI
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


typedef enum
{
	ReadWriteOk   		= 0x00, // �������� ��������� �������
	SPIBusy       		= 0x01, // ���� SPI ������
	DataInMismatch		= 0x02  // �������������� ������� ������� ������
} ReadWriteRegTypeDef;	 
	 
	 
	 
	 
	 
#define EXT_ADDRESS						0x2F  			/* EXTENDED ADDRESS fields */			 
#define REG_ADDRESS						0x00				/* Register space */
#define	REG_DMA								0x3E				/* Direct FIFO Access */	 
	 
#define READ_CC1120						0x01				/* Read register*/
#define WRITE_CC1120					0x00				/* Write register */	 

#define BURST									0x01				/* Burst mode */
#define NO_BURST							0x00				/* Single mode */
	 
	 
	 
#define EXT_PARTNUMBER				0x8F  			/* PARTNUMBER */			 
	 
	 
	 
	 
uint8_t CC1120_CheckModule(SPI_HandleTypeDef *hspi);
	 

ReadWriteRegTypeDef CC1120_Write (uint8_t uGenAddress, uint8_t uExtAddress, uint8_t bBurst, uint8_t *data_ptr, uint16_t uAccesses);
ReadWriteRegTypeDef CC1120_Read (uint8_t uGenAddress, uint8_t uExtAddress, uint8_t bBurst, uint8_t *data_ptr, uint8_t uAccesses);

	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SPI_CC1120_H */

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


typedef enum
{
	ReadWriteOk   		= 0x00, // операция завершена успешно
	SPIBusy       		= 0x01, // шина SPI занята
	DataInMismatch		= 0x02  // несоответствие формата входных данных
} ReadWriteRegTypeDef;	 

typedef enum
{
	STATUS_IDLE   							= 0x00, 
	STATUS_RX       						= 0x01,
	STATUS_TX										= 0x02,
	STATUS_FSTXON 							= 0x03,
	STATUS_CALIBRATE 						= 0x04,
	STATUS_SETTLING 						= 0x05,
	STATUS_RX_FIFO_ERROR				= 0x06,
	STATUS_TX_FIFO_ERROR 				= 0x07,
	STATUS_SPI_ERROR 						= 0x08
} CC1120STATUSTypeDef;	


	 
#define EXT_ADDRESS						0x2F  			/* EXTENDED ADDRESS fields */			 
#define REG_ADDRESS						0x00				/* Register space */
#define	REG_DMA								0x3E				/* Direct FIFO Access */	 
	 
#define READ_CC1120						0x01				/* Read register*/
#define WRITE_CC1120					0x00				/* Write register */	 

#define BURST									0x01				/* Burst mode */
#define NO_BURST							0x00				/* Single mode */

#define TX_FIFO_FAIL					0xFF				/* Ошибка чтения данных FIFO_TX */
	 
	 
	 
#define EXT_PARTNUMBER				0x8F  			/* PARTNUMBER */
#define EXT_PARTVERSION				0x90  			/* PARTVERSION */
#define EXT_NUM_TXBYTES				0xD6  			/* Количество байтов в FIFO TX */
#define S_STATUS							0x3D				/* No operation. May be used to get access to the chip status byte */
#define S_TX									0x35				/* Enable Tx */	 
#define S_RX									0x34				/* Enable Rx */
#define S_IDLE								0x36				/* IDLE */
#define S_RESET								0x30				/* Сброс трансивера */
#define	S_TX_FIFO_FLUSH				0x3B				/* Очистка FIFO TX */
#define R_TX_FIFO_WRITE				0x7F				/* Стандартная запись данных в Tx FIFO */


#define CC1120_ID							0x48				/* Chip ID CC1120 */ 


	 
uint8_t CC1120_CheckModule(SPI_HandleTypeDef *hspi);
	 

ReadWriteRegTypeDef CC1120_Write (uint8_t uGenAddress, uint8_t uExtAddress, uint8_t bBurst, uint8_t *data_ptr, uint16_t uAccesses);
ReadWriteRegTypeDef CC1120_Read (uint8_t uGenAddress, uint8_t uExtAddress, uint8_t bBurst, uint8_t *data_ptr, uint8_t uAccesses);

uint8_t CC1120_Status (SPI_HandleTypeDef *hspi);
uint8_t CC1120_CheckVersion(SPI_HandleTypeDef *hspi);
uint8_t CC1120_Status(SPI_HandleTypeDef *hspi);
uint8_t CC1120_Tx(SPI_HandleTypeDef *hspi);
uint8_t CC1120_IDLE_set(SPI_HandleTypeDef *hspi);
uint8_t CC1120_Rx(SPI_HandleTypeDef *hspi);
uint8_t CC1120_Reset(SPI_HandleTypeDef *hspi);
uint8_t CC1120_TxFIFONumBytes(SPI_HandleTypeDef *hspi);
uint8_t CC1120_TxFIFOFlush(SPI_HandleTypeDef *hspi);
uint8_t CC1120_TxFIFOWrite(SPI_HandleTypeDef *hspi, uint8_t *fifo_write_data_ptr, uint8_t tx_num);

	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SPI_CC1120_H */

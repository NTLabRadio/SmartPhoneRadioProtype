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

typedef enum
{
	MARCSTATE_SLEEP								= 0x00,
	MARCSTATE_IDLE								= 0x01,
	MARCSTATE_XOFF								= 0x02,
	MARCSTATE_BIAS_SETTLE_MC			= 0x03, 
	MARCSTATE_REG_SETTLE_MC				= 0x04, 
	MARCSTATE_MANCAL							= 0x05, 
	MARCSTATE_BIAS_SETTLE					= 0x06, 
	MARCSTATE_REG_SETTLE					= 0x07, 
	MARCSTATE_STARTCAL						= 0x08, 
	MARCSTATE_BWBOOST							= 0x09, 
	MARCSTATE_FS_LOCK							= 0x0A, 
	MARCSTATE_IFADCON							= 0x0B, 
	MARCSTATE_ENDCAL							= 0x0C,
	MARCSTATE_RX									= 0x0D,
	MARCSTATE_RX_END							= 0x0E, 
	MARCSTATE_Reserved						= 0x0F, 
	MARCSTATE_TXRX_SWITCH					= 0x10, 
	MARCSTATE_RX_FIFO_ERR					= 0x11, 
	MARCSTATE_FSTXON							= 0x12, 
	MARCSTATE_TX									= 0x13, 
	MARCSTATE_TX_END							= 0x14, 
	MARCSTATE_RXTX_SWITCH					= 0x15, 
	MARCSTATE_TX_FIFO_ERR					= 0x16, 
	MARCSTATE_IFADCON_TXRX				= 0x17,
	MARCSTATE_SPI_ERROR 					= 0x18
} CC1120MARCSTATETypeDef;	


typedef struct
{
  uint16_t  addr; // поле адреса
  uint8_t   data; // поле данных
}registerSetting_t;	// шаблон структуры конфигурации






	 
#define EXT_ADDRESS						0x2F  			/* EXTENDED ADDRESS fields */			 
#define REG_ADDRESS						0x00				/* Register space */
#define	REG_DMA								0x3E				/* Direct FIFO Access */	 
	 
#define READ_CC1120						0x01				/* Read register*/
#define WRITE_CC1120					0x00				/* Write register */	 

#define BURST									0x01				/* Burst mode */
#define NO_BURST							0x00				/* Single mode */

#define TX_FIFO_FAIL					0xFF				/* Ошибка чтения данных FIFO_TX */
#define RX_FIFO_FAIL					0xFF				/* Ошибка чтения данных FIFO_RX */
	 
	 
	 
#define EXT_PARTNUMBER				0x8F  			/* PARTNUMBER */
#define EXT_PARTVERSION				0x90  			/* PARTVERSION */
#define EXT_NUM_TXBYTES				0xD6  			/* Количество байтов в FIFO TX */
#define EXT_NUM_RXBYTES				0xD7				/* Количество байтов в FIFO RX */
#define EXT_FS_VCO2						0x25				/* FS_VCO2 */
#define EXT_FS_CAL2						0x15				/* FS_CAL2 */
#define EXT_FS_VCO4						0x23				/* FS_VCO_4 */
#define EXT_FS_CHP						0x18				/* FS_CHP */
#define EXT_MARCSTATE					0x73				/* Опрос состояния трансивера */
#define S_STATUS							0x3D				/* No operation. May be used to get access to the chip status byte */
#define S_TX									0x35				/* Enable Tx */	 
#define S_RX									0x34				/* Enable Rx */
#define S_IDLE								0x36				/* IDLE */
#define S_RESET								0x30				/* Сброс трансивера */
#define	S_TX_FIFO_FLUSH				0x3B				/* Очистка FIFO TX */
#define	S_RX_FIFO_FLUSH				0x3A				/* Очистка FIFO TX */
#define S_CAL									0x33				/* Запуск калибровки синтезатора */
#define S_SFSTXON							0x31				/* запуск автоматической калибровки синтезатора */
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
uint8_t CC1120_ManualCalibration(SPI_HandleTypeDef *hspi);
CC1120MARCSTATETypeDef CC1120_MARCState(SPI_HandleTypeDef *hspi);
uint8_t CC1120_SFSTXON_set (SPI_HandleTypeDef *hspi);
uint8_t CC1120_RxFIFONumBytes(SPI_HandleTypeDef *hspi);
uint8_t CC1120_RxFIFOFlush(SPI_HandleTypeDef *hspi);

	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SPI_CC1120_H */

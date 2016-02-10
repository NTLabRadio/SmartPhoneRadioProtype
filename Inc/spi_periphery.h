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

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"
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


// Definitions for IO port mapped as chip select (output).

#define CMX7262_SPI_CS_PIN              	GPIO_PIN_2                 	/* PA.02 */
#define CMX7262_SPI_CS_GPIO_PORT        	GPIOA                      	/* GPIOA */

// GPIO lines for SPI interfaces

#define CMX7262_SPI_SCK_PIN               GPIO_PIN_5                  /* PA.05 */
#define CMX7262_SPI_SCK_GPIO_PORT         GPIOA                       /* GPIOA */

#define CMX7262_SPI_MISO_PIN              GPIO_PIN_6                  /* PA.06 */
#define CMX7262_SPI_MISO_GPIO_PORT        GPIOA                       /* GPIOA */

#define CMX7262_SPI_MOSI_PIN              GPIO_PIN_7                  /* PA.07 */
#define CMX7262_SPI_MOSI_GPIO_PORT        GPIOA                       /* GPIOA */

//IRQ from CMX7262
#define CMX7262_IRQN_PIN              		GPIO_PIN_1                  /* PA.01 */
#define CMX7262_IRQN_PORT        					GPIOA                       /* GPIOA */



// Definitions for IO port mapped as chip select (output).

#define CC1120_SPI_CS_PIN            	   	GPIO_PIN_4              		/* PA.04 */
#define CC1120_SPI_CS_GPIO_PORT      	   	GPIOA                       /* GPIOA */

// GPIO lines for SPI interfaces

#define CC1120_SPI_SCK_PIN               	GPIO_PIN_5                  /* PA.05 */
#define CC1120_SPI_SCK_GPIO_PORT         	GPIOA                       /* GPIOA */

#define CC1120_SPI_MISO_PIN              	GPIO_PIN_6                  /* PA.06 */
#define CC1120_SPI_MISO_GPIO_PORT        	GPIOA                       /* GPIOA */

#define CC1120_SPI_MOSI_PIN              	GPIO_PIN_7                  /* PA.07 */
#define CC1120_SPI_MOSI_GPIO_PORT        	GPIOA                       /* GPIOA */

//IRQ from CC1120
#define CC1120_IRQN_PIN              			GPIO_PIN_0                  /* PA.00 */
#define CC1120_IRQN_PORT        					GPIOA                       /* GPIOA */

#define CMX7262_CSN_LOW()     HAL_GPIO_WritePin(CMX7262_SPI_CS_GPIO_PORT, CMX7262_SPI_CS_PIN, GPIO_PIN_RESET)
#define CMX7262_CSN_HIGH()    HAL_GPIO_WritePin(CMX7262_SPI_CS_GPIO_PORT, CMX7262_SPI_CS_PIN, GPIO_PIN_SET)
#define CC1120_CSN_LOW()     	HAL_GPIO_WritePin(CC1120_SPI_CS_GPIO_PORT, CC1120_SPI_CS_PIN, GPIO_PIN_RESET)
#define CC1120_CSN_HIGH()			HAL_GPIO_WritePin(CC1120_SPI_CS_GPIO_PORT, CC1120_SPI_CS_PIN, GPIO_PIN_SET)

	 
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

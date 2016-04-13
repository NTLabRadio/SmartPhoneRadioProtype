/**
  ******************************************************************************
  * @file    flash.h
  * @brief   ‘айл, содержащий все необходимое дл€ работы с загрузочной флеш
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_H
#define __FLASH_H

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
	 
#define MAIN_PROGRAM_MAX_SIZE				(0xE000)	//56 кЅ
#define MAIN_PROGRAM_NUM_OF_PAGES		(MAIN_PROGRAM_MAX_SIZE / FLASH_PAGE_SIZE)
#define MAIN_PROGRAM_START_ADDRESS 	(uint32_t)0x08000000

	 
uint8_t StartFirmLoadToFlash(void);
uint8_t FirmLoadToFlash(uint8_t* pDataFrame, uint16_t nSizeDataFrame);
uint8_t FinishFirmLoadToFlash(uint16_t nFirmDumpCRC, uint16_t nFirmCmd);
	 	 
 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FLASH_H */

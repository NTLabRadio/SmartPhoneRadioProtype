/**
  ******************************************************************************
  * @file    DebugLog.h
  * @brief   Файл, содержащий все необходимое для вывода отладочной информации
	*						в лог
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DEBUGLOG_H
#define __DEBUGLOG_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include "stm32f1xx_hal.h"

#define ECHO_FGETC

int fputc(int ch, FILE *f);
uint8_t ConvertHexIntToUTF8(uint8_t* pBufData, uint16_t nSizeData);
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DEBUGLOG_H */

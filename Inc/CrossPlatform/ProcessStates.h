/**
  ******************************************************************************
  * @file    ProcessStates.h
  * @brief   Файл, содержащий прототипы функций обработки состояний различных
	*					 компонентов радиомодуля
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PROCESSSTATES_H
#define __PROCESSSTATES_H

#include <string.h>
#include "cc1120.h"
#include "cmx7262.h"
#include "FIFOBuffers.h"
#include "RadioLogic.h"
#include "RadioModule.h"


#define MAX_SIZE_OF_DATA_FROM_CMX7262 (2048)
#define MAX_SIZE_OF_DATA_TO_CMX7262 	(4096)

void RadioModuleInit(SPI_HandleTypeDef *hspiCMX7262, SPI_HandleTypeDef *hspiCC1120);
void RadioModuleDeInit(void);

void ProcessPTTState(void);

void ProcessRadioState(void);

void CMX7262_TestMode(void);
void ProcessCMX7262State(void);

void TransceiverStartRx(void);
void VocoderStartDecode(void);
void VocoderStartEncode(void);

#endif /* __PROCESSSTATES_H */	 

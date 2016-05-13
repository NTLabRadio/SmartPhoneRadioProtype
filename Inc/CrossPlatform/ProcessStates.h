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
#include "mathfuncs.h"
#include "QueDataFrames.h"
#include "RadioLogic.h"
#include "RadioModule.h"


#define MAX_SIZE_OF_DATA_FROM_CMX7262 (2048)
#define MAX_SIZE_OF_DATA_TO_CMX7262 	(4096)

void RadioModuleInit(SPI_HandleTypeDef *hspiCMX7262, SPI_HandleTypeDef *hspiCC1120);
void RadioModuleDeInit(void);

void RadioImitator_TxData(uint8_t* pPackData, uint16_t packSize);

void ProcessPTTState(void);

void ProcessRadioState(void);
void ProcessTxWaiting(void);
void ProcessTxRunning(void);
void ProcessRxWaiting(void);
void ProcessRxRunning(void);

void CMX7262_TestMode(void);
void ProcessCMX7262State(void);

void TransceiverStartRx(void);
void VocoderStartDecode(void);
void VocoderStartEncode(void);

#ifndef SMART_PROTOTYPE
void FrontEndSetToTx();
void FrontEndSetToRx();
#endif

#define CHECK_RSSI_PERIOD 	(100000)
uint8_t isNeedCheckRSSI();

#endif /* __PROCESSSTATES_H */	 

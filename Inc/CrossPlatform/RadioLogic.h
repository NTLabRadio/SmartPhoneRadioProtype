/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** Этот файл описывает функции реализации логики радиоинтерфейса
**
****************************************************************************/

#ifndef RADIOLOGIC_H
#define RADIOLOGIC_H

#include <stdint.h>
#include <string.h>
#include "cc1120.h"
#include "ProcessStates.h"
#include "RadioMessage.h"
#include "RadioModule.h"
#include "trellisCoder.h"

#define RADIO_BROADCAST_ADDR 	(0)

#define MAX_NUM_RECEIVE_STATS_IN_QUE_TO_EXT_DEV 	(15)
#define SIZE_OF_RECEIVER_STATS 										(3)

extern RadioModule* pobjRadioModule;
extern CC1120_TypeDef  g_CC1120Struct;

void FormAndSendRadioPack(uint8_t* pPayloadData, uint16_t nPayloadSize, uint8_t nPayloadDataType);
void FormRadioPack(RadioMessage* RadioPack, uint8_t* pPayloadData, uint8_t nPayloadSize, uint8_t nPayloadDataType);
uint8_t SendRadioPackToTansceiver(uint8_t* pData, uint16_t nSizeData);
void ProcessRadioPack(uint8_t* pPayloadData, uint16_t& nPayloadSize, uint8_t& nDataType, uint8_t* pRadioStatusData);

int8_t ApplyRSSIOffset(int8_t nRSSIRegValue);
uint8_t FormAndPushToQueRecStatsMsg(int8_t nRSSI, uint8_t nLQIAndCRCFlag, int8_t nBER);
uint8_t BERInPack(uint8_t* pPackData, uint8_t nSizePackData, uint8_t noPattern);

void EncodeRadioMsg(RadioMessage* RadioPack);
void DecodeRadioMsg(RadioMessage* RadioPack);

uint8_t EncodeData(uint8_t* pDataIn, uint8_t nSizeDataIn, uint8_t* pDataOut, uint8_t& nSizeDataOut);
uint8_t DecodeData(uint8_t* pDataIn, uint8_t nSizeDataIn, uint8_t* pDataOut, uint8_t& nSizeDataOut, int16_t& nDetectedErrors);
uint8_t HexToBinArray(const uint8_t * const pHexDataIn, const uint16_t sizeHexDataIn, uint8_t* pBinDataOut);
uint8_t BinToHexArray(const uint8_t * const pBinDataIn, const uint16_t sizeHexDataOut, uint8_t* pHexDataOut);

#endif // RADIOLOGIC_H

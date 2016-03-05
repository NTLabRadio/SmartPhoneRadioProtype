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
#include "RadioMessage.h"
#include "RadioModule.h"

extern RadioModule* pobjRadioModule;
extern CC1120_TypeDef  g_CC1120Struct;

void FormAndSendRadioPack(uint8_t* pPayloadData, uint16_t nPayloadSize);
void FormRadioPack(RadioMessage* RadioPack, uint8_t* pPayloadData, uint16_t nPayloadSize);
void SendRadioPackToTansceiver(uint8_t* pData, uint16_t nSizeData);
void ProcessRadioPack(uint8_t* pPayloadData, uint16_t& nPayloadSize);


#endif // RADIOLOGIC_H

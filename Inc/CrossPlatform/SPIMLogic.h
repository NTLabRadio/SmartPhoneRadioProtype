/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** Этот файл описывает функции реализации логического интерфейса SPIM (Smart Phone 
** InterModule)
**
****************************************************************************/

#ifndef SPIMLOGIC_H
#define SPIMLOGIC_H

#include <stdint.h>
#include <string.h>
#include "FirmwareFrame.h"
#include "flash.h"
#include "QueDataFrames.h"
#include "RadioModule.h"
#include "SPIMMessage.h"
#include "uart_intermodule.h"

extern uint8_t pUARTRxSLIPPack[];
extern uint16_t nSizeSLIPPack;
extern RadioModule* pobjRadioModule;

enum en_SPIMaddrs
{
	SPIM_ADDR_STM32				=0x1,		//контроллер STM32 целевого устройства (радимодуля)
	SPIM_ADDR_EXTDEV			=0x2		//внешнее управляющее устройство (процессор NT1004, ПК или др.)
};

enum en_SPIMReqTypes
{
	SPIM_REQTYPE_SINGLE		=0,			//одиночный синхронный запрос - запрос, ответ на который должен быть выслан ведомым 
																//устройством однократно в момент получения запроса
	SPIM_REQTYPE_ASYNC		=1			//асинхронный запрос - запрос, в ответ на который параметр (например, RSSI) высылается 
																//ведомым устройством в произвольный момент времени, при изменении его значения
};

enum en_SPIMOperationResults
{
	SPIM_OP_RESULT_FAIL 			= 0x00,
	SPIM_OP_RESULT_SUCCESS 		= 0x01
};

void SPIMInit(void);
void SPIMDeInit(void);

void ProcessDataFromExtDev(void);
void FormAndSendAnswerToExtDev(SPIMMessage* SPIMmsgRcvd);
void FormAnswerToExtDev(SPIMMessage* SPIMCmdRcvd, SPIMMessage* SPIMBackCmdToSend);
void FormBodyOfAnswerToExtDev(SPIMMessage* SPIMCmdRcvd, uint8_t* pBodyData, uint8_t& bodySize);
void FormCurrentParamAnswer(SPIMMessage* SPIMCmdRcvd, uint8_t* pBodyData, uint8_t& bodySize);
void ProcessCmdSetMode(SPIMMessage* SPIMCmdRcvd);

void FormAsyncReqParamBack(uint8_t maskReqParam, SPIMMessage* SPIMmsgToSend);
void FormBodyOfAsyncReqParamBack(uint8_t maskReqParam, uint8_t* pBodyMsgToSend, uint8_t& bodySizeMsgToSend);

void ProcessAsyncReq(void);

void ProcessDataToExtDev(void);
void FormAndSendDataMsgToExtDev(void);
void FormDataMsgToExtDev(SPIMMessage* SPIMCmdToSend);

void FormAndSendRecStatusToExtDev(void);
void FormRecStatusMsgToExtDev(SPIMMessage* SPIMCmdToSend);

uint8_t ProcessFirmwareFrame(uint8_t* pBodyFrame, uint8_t nSizeFrame);
//uint8_t ProcessFirmwareFrame(SPIMMessage* SPIMCmd);

#endif // SPIMLOGIC_H

/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** ���� ���� ��������� ������� ���������� ����������� ���������� SPIM (Smart Phone 
** InterModule)
**
****************************************************************************/

#ifndef SPIMLOGIC_H
#define SPIMLOGIC_H

#include <stdint.h>
#include <string.h>
#include "QueDataFrames.h"
#include "RadioModule.h"
#include "SPIMMessage.h"
#include "uart_intermodule.h"

extern uint8_t pUARTRxSLIPPack[];
extern uint16_t nSizeSLIPPack;
extern RadioModule* pobjRadioModule;

enum en_SPIMaddrs
{
	SPIM_ADDR_STM32				=0x1,		//���������� STM32 �������� ���������� (����������)
	SPIM_ADDR_EXTDEV			=0x2		//������� ����������� ���������� (��������� NT1004, �� ��� ��.)
};

enum en_SPIMReqTypes
{
	SPIM_REQTYPE_SINGLE		=0,			//��������� ���������� ������ - ������, ����� �� ������� ������ ���� ������ ������� 
																//����������� ���������� � ������ ��������� �������
	SPIM_REQTYPE_ASYNC		=1			//����������� ������ - ������, � ����� �� ������� �������� (��������, RSSI) ���������� 
																//������� ����������� � ������������ ������ �������, ��� ��������� ��� ��������
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


#endif // SPIMLOGIC_H

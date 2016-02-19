/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** ���� ���� ��������� ����� SPIMMessage, ���������� � ���� ����� ��������
** ������� � ������� ��� ���������� ����������� ���������� SPIM (Smart Phone 
** InterModule). ����� ��������� ����������� � ������������ ��������� �������, 
** ������������� ����������� SPIM
**
****************************************************************************/

#ifndef SPIMMESSAGE_H
#define SPIMMESSAGE_H

#include <stdint.h>
#include <string.h>

enum SPIMcmd
{
	SPIM_NOP										=0x00,
	SPIM_NOP_BACK								=0x01,
	SPIM_SET_MODE								=0x02,
	SPIM_SET_MODE_BACK					=0x03,
	SPIM_SEND_DATA_FRAME				=0x04,
	SPIM_SEND_DATA_FRAME_BACK		=0x05,
	SPIM_TAKE_DATA_FRAME				=0x06,
	SPIM_TAKE_DATA_FRAME_BACK		=0x07,
	SPIM_REQ_CURRENT_PARAM 			=0x08,
	SPIM_REQ_CURRENT_PARAM_BACK =0x09,
	SPIM_SOFT_VER								=0x0A,
	SPIM_SOFT_VER_BACK					=0x0B
};

enum SPIMadr
{
	SPIM_ADR_STM32							=0x1,		//���������� STM32 �������� ���������� (����������)
	SPIM_ADR_EXTDEV							=0x2		//������� ����������� ���������� (��������� NT1004, �� ��� ��.)
};

class SPIMMessage
{
public:
	SPIMMessage();
	SPIMMessage(const uint8_t* pMsgData, uint16_t msgSize);
	~SPIMMessage();

	uint8_t* Data;
	uint8_t Size;

	uint8_t* Body;

	uint8_t setHeader(uint8_t bodySize, uint8_t address, uint8_t noMsg, uint8_t IDcmd);
	uint8_t setBody(uint8_t* pBodyData, uint8_t bodySize);
	uint8_t setCRC();
	uint8_t setMsg(uint8_t* pMsgData, uint8_t msgSize);

	uint8_t getHeader(uint8_t* pHeaderData);
	uint8_t getBody(uint8_t* pBodyData);
	uint8_t getCRC();

	uint8_t checkCRC();

	uint8_t getAddress();
	uint8_t getNoMsg();
	uint8_t getSizeBody();
	uint8_t getIDCmd();
	
	uint8_t IDBackCmd(uint8_t IDCmd);

private:

	//������������ ������ ���� ���������, ����
	#define MAX_SIZE_OF_BODY     (128)
	//������ ���������, ����
	#define SIZE_OF_HEADER       (3)
	//������ ���� CRC, ����
	#define SIZE_OF_CRC          (1)

	//������������ ������ ����� ���������, ����
	#define MAX_SIZE_OF_MSG     (SIZE_OF_HEADER + MAX_SIZE_OF_BODY + SIZE_OF_CRC)
	//����������� ������ ����� ���������, ����
	#define MIN_SIZE_OF_MSG     (SIZE_OF_HEADER + SIZE_OF_CRC)

	struct structSPIMMsgHeader {
		uint8_t bodySize;               // ������ ����, ����
		uint8_t adress          :4;     // ������� ���������
		uint8_t noMsg          	:4;     // ������� ��������� (�� ������ 4)
		uint8_t IDCmd;                  // ������������� �������
	};

	uint8_t SPIMmsgData[MAX_SIZE_OF_MSG];
	uint8_t* SPIMHeaderData;
	uint8_t* SPIMbodyData;
	uint8_t SPIMbodySize;
	uint8_t* SPIMCRC;

	uint8_t CRC_Calc(uint8_t* pData, uint8_t sizeData);
};

#endif // SPIMMESSAGE_H

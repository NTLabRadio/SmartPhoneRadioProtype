/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** ���� ���� ��������� ����� RadioMessage, ���������� � ���� ����� ��������
** ������� � ������� ��� ������������ � ��������� ��������� ���������������
**
****************************************************************************/

#ifndef RADIOMESSAGE_H
#define RADIOMESSAGE_H

#include <stdint.h>
#include <string.h>

#ifdef DEBUG_CC1120_VARIABLE_PACKET_LENGTH
#include "globals.h"
#endif


class RadioMessage
{
public:
	
	RadioMessage();
	RadioMessage(const uint8_t* pMsgData, uint16_t msgSize);
	~RadioMessage();

	uint8_t* Data;
	uint8_t Size;

	uint8_t* Body;

	uint8_t setHeader(uint8_t nDstAddress, uint8_t nSrcAddress, uint8_t nDataType, uint8_t nDataSize);
	uint8_t setBody(uint8_t* pBodyData, uint8_t bodySize);
	uint8_t setMsg(uint8_t* pMsgData, uint8_t msgSize);

	uint8_t getHeader(uint8_t* pHeaderData);
	uint8_t getBody(uint8_t* pBodyData);
	uint8_t getMsg(uint8_t* pMsgData);

	uint8_t getDstAddress();
	uint8_t getSrcAddress();
	uint8_t getDataType();
	uint8_t getDataSize();
	
	uint8_t applyFEC();

	typedef enum radioDataTypes_t
	{
		RADIO_DATATYPE_VOICE					=0x01,
		RADIO_DATATYPE_CONF_DATA			=0x02,
		RADIO_DATATYPE_UNCONF_DATA		=0x03
	} radioDataTypes_t;

	//������������ ������ ����� ���������, ����
	static const uint8_t MAX_SIZE_OF_MSG = 255;

private:

	typedef struct{
		#ifdef DEBUG_CC1120_VARIABLE_PACKET_LENGTH
		uint8_t packLength;   	// ����� ������, ����
		#endif		
		uint8_t dstAddress;     // ����� ����������
		uint8_t srcAddress;     // ����� ���������
		uint8_t dataType;     	// ��� ������ (���� / ������. ������ / ��������. ������)
		uint8_t dataSize;     	// ������ �������� ������ � ���������, ����
		#ifndef DEBUG_CC1120_VARIABLE_PACKET_LENGTH
		uint8_t reserve;
		#endif
	} structRadioMsgHeader;	

	//������ ���������, ����
	static const uint8_t SIZE_OF_HEADER = sizeof(structRadioMsgHeader);//5;

	//������������ ������ ���� ���������, ����
	static const uint8_t MAX_SIZE_OF_BODY = (MAX_SIZE_OF_MSG - SIZE_OF_HEADER);

	//����������� ������ ����� ���������, ����
	static const uint8_t MIN_SIZE_OF_MSG = SIZE_OF_HEADER;
	
	uint8_t RadioMsgData[MAX_SIZE_OF_MSG];
	uint8_t* RadioHeaderData;
	uint8_t* RadioBodyData;
	uint8_t RadioBodySize;

};

#endif // RADIOMESSAGE_H

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


#define RADIO_BROADCAST_ADDR 	(0)

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

	uint8_t getDstAddress();
	uint8_t getSrcAddress();
	uint8_t getDataType();
	uint8_t getDataSize();

	typedef enum radioDataTypes_t
	{
		RADIO_DATATYPE_VOICE					=0x01,
		RADIO_DATATYPE_CONF_DATA			=0x02,
		RADIO_DATATYPE_UNCONF_DATA		=0x03
	} radioDataTypes_t;

private:

	//������������ ������ ����� ���������, ����
	static const uint8_t MAX_SIZE_OF_MSG = 128;

	//������ ���������, ����
	static const uint8_t SIZE_OF_HEADER = 5;

	//������������ ������ ���� ���������, ����
	static const uint8_t MAX_SIZE_OF_BODY = (MAX_SIZE_OF_MSG - SIZE_OF_HEADER);

	//����������� ������ ����� ���������, ����
	static const uint8_t MIN_SIZE_OF_MSG = SIZE_OF_HEADER;

	typedef struct {
		uint8_t dstAddress;     // ����� ����������
		uint8_t srcAddress;     // ����� ���������
		uint8_t dataType;     	// ��� ������ (���� / ������. ������ / ��������. ������)
		uint8_t dataSize;     	// ������ �������� ������ � ���������, ����
		uint8_t reserve;
	} structRadioMsgHeader;

	uint8_t RadioMsgData[MAX_SIZE_OF_MSG];
	uint8_t* RadioHeaderData;
	uint8_t* RadioBodyData;
	uint8_t RadioBodySize;

};

#endif // RADIOMESSAGE_H

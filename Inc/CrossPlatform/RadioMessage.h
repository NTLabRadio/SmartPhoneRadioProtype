/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** Ётот файл описывает класс RadioMessage, включающий в себ€ набор основных
** методов и свойств дл€ формировани€ и обработки сообщений радиоинтерфейса
**
****************************************************************************/

#ifndef RADIOMESSAGE_H
#define RADIOMESSAGE_H

#include <stdint.h>
#include <string.h>


typedef struct {
	uint8_t dstAddress;     // адрес получател€
	uint8_t srcAddress;     // адрес источника
	uint8_t dataType;     	// тип данных
	uint8_t reserve[2];
} structRadioMsgHeader;


#define RADIO_BROADCAST_ADDR 	(0)

typedef enum
{
	RADIO_DATATYPE_VOICE					=0x01,
	RADIO_DATATYPE_CONF_DATA			=0x02,
	RADIO_DATATYPE_UNCONF_DATA		=0x03
} radioDataTypes_t;


class RadioMessage
{
public:
	
	RadioMessage();
	RadioMessage(const uint8_t* pMsgData, uint16_t msgSize);
	~RadioMessage();

	uint8_t* Data;
	uint8_t Size;

	uint8_t* Body;

	uint8_t setHeader(uint8_t nDstAddress, uint8_t nSrcAddress, uint8_t nDataType);
	uint8_t setBody(uint8_t* pBodyData, uint8_t bodySize);
	uint8_t setMsg(uint8_t* pMsgData, uint8_t msgSize);

	uint8_t getHeader(uint8_t* pHeaderData);
	uint8_t getBody(uint8_t* pBodyData);


private:

	//ћаксимальный размер всего сообщени€, байт
	static const uint8_t MAX_SIZE_OF_MSG = 128;

	//–азмер заголовка, байт
	static const uint8_t SIZE_OF_HEADER = 5;

	//ћаксимальный размер тела сообщени€, байт
	static const uint8_t MAX_SIZE_OF_BODY = (MAX_SIZE_OF_MSG - SIZE_OF_HEADER);

	//ћинимальный размер всего сообщени€, байт
	static const uint8_t MIN_SIZE_OF_MSG = SIZE_OF_HEADER;

	uint8_t RadioMsgData[MAX_SIZE_OF_MSG];
	uint8_t* RadioHeaderData;
	uint8_t* RadioBodyData;
	uint8_t RadioBodySize;

};

#endif // RADIOMESSAGE_H

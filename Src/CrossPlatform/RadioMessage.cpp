#include "RadioMessage.h"


RadioMessage::RadioMessage()
{
	RadioHeaderData = RadioMsgData;
	RadioBodyData = RadioMsgData + SIZE_OF_HEADER;

	//�������������� ������ ��� ������, ���������� ��� ���������, � �.�. ���������
	memset(RadioMsgData,0,MAX_SIZE_OF_MSG);

	//�� ��������� ������� ��������� � ����� ������� �����
	Size = MIN_SIZE_OF_MSG;
	RadioBodySize = 0;

	Data = RadioMsgData;
	Body = RadioBodyData;	
}


RadioMessage::RadioMessage(const uint8_t* pMsgData, uint16_t msgSize)
{
	if(!pMsgData)
		return;

	if((msgSize < MIN_SIZE_OF_MSG) || (msgSize > MAX_SIZE_OF_MSG))
		return;

	RadioHeaderData = RadioMsgData;
	RadioBodyData = RadioMsgData + SIZE_OF_HEADER;

	//�������������� ������ ��� ������, ���������� ��� ���������
	memset(RadioMsgData,0,MAX_SIZE_OF_MSG);

	//�������� ������ � ���� ���������
	memcpy((void*)RadioMsgData,(void*)pMsgData,msgSize);

	Size = msgSize;
	RadioBodySize = Size - SIZE_OF_HEADER;

	Data = RadioMsgData;
	Body = RadioBodyData;

	return;
}


RadioMessage::~RadioMessage()
{
}


uint8_t RadioMessage::setHeader(uint8_t nDstAddress, uint8_t nSrcAddress, uint8_t nDataType)
{
	structRadioMsgHeader strMsgHeader;

	strMsgHeader.dstAddress = nDstAddress;

	strMsgHeader.srcAddress = nSrcAddress;

	strMsgHeader.dataType = nDataType;

	memcpy((void*)RadioHeaderData,(void*)&strMsgHeader,SIZE_OF_HEADER);

	return 0;
}


uint8_t RadioMessage::setBody(uint8_t* pBodyData, uint8_t bodySize)
{
	if((!pBodyData) && bodySize)
		return(1);

	if(bodySize > MAX_SIZE_OF_BODY)
		return(1);

	memcpy((void*)RadioBodyData,(void*)pBodyData,bodySize);

	RadioBodySize = bodySize;
	Size = RadioBodySize + SIZE_OF_HEADER;

	return(0);
}


uint8_t RadioMessage::setMsg(uint8_t* pMsgData, uint8_t msgSize)
{
	if(!pMsgData)
		return 1;

	if((msgSize < MIN_SIZE_OF_MSG) || (msgSize > MAX_SIZE_OF_MSG))
		return 1;

	memcpy((void*)RadioMsgData,(void*)pMsgData,msgSize);

	Size = msgSize;
	RadioBodySize = Size - SIZE_OF_HEADER;

	return 0;
}


uint8_t RadioMessage::getHeader(uint8_t* pHeaderData)
{
	if(!pHeaderData)
		return(0xFF);

	memcpy(pHeaderData,RadioHeaderData,SIZE_OF_HEADER);

	return(SIZE_OF_HEADER);
}


uint8_t RadioMessage::getDstAddress()
{
	structRadioMsgHeader *RadioMsgHeader;

	RadioMsgHeader = (structRadioMsgHeader*)RadioHeaderData;

	return(RadioMsgHeader->dstAddress);
}
	
uint8_t RadioMessage::getSrcAddress()
{
	structRadioMsgHeader *RadioMsgHeader;

	RadioMsgHeader = (structRadioMsgHeader*)RadioHeaderData;

	return(RadioMsgHeader->srcAddress);
	
}

uint8_t RadioMessage::getDataType()
{
	structRadioMsgHeader *RadioMsgHeader;

	RadioMsgHeader = (structRadioMsgHeader*)RadioHeaderData;

	return(RadioMsgHeader->dataType);
}

uint8_t RadioMessage::getBody(uint8_t* pBodyData)
{
	if(!pBodyData)
		return(0xFF);

	if(!RadioBodySize)
		return(0);

	memcpy(pBodyData,RadioBodyData,RadioBodySize);

	return(RadioBodySize);
}

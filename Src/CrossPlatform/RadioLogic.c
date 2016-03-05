#include "RadioLogic.h"


//Данные радиопакета для передачи
uint8_t RadioPackForSend[MAX_RADIOPACK_SIZE];

//Данные принятого радиопакета
uint8_t RadioPackRcvd[MAX_RADIOPACK_SIZE];


void FormAndSendRadioPack(uint8_t* pPayloadData, uint16_t nPayloadSize)
{	
	RadioMessage RadioMsgToSend;
	
	//Формируем радиопакет
	FormRadioPack(&RadioMsgToSend,pPayloadData,nPayloadSize);
	
	//Отправляем сформированный пакет в эфир
	#ifndef TEST_RADIO_IMITATE
	SendRadioPackToTansceiver(RadioMsgToSend.Data, RadioMsgToSend.Size);
	#else
	//В режиме радиоимитатора не отправляем в эфир, а накапливаем в буфер для воспроизведения по отпусканию тангенты
	//Накапливаем только полезную нагрузку
	RadioImitator_TxData(pPayloadData, nPayloadSize);
	#endif
	
	return;
}


void SendRadioPackToTansceiver(uint8_t* pData, uint16_t nSizeData)
{
	CC1120_TxData(&g_CC1120Struct, pData, nSizeData);
}


void FormRadioPack(RadioMessage* RadioPack, uint8_t* pPayloadData, uint16_t nPayloadSize)
{
	//Устанавливаем широковещательный адрес
	uint8_t dstAddress = RADIO_BROADCAST_ADDR;
	//Собственный адрес берем из настроек радиомодуля
	uint8_t srcAddress = pobjRadioModule->GetRadioAddress();
	//Тип передаваемых данных
	uint8_t dataType = RADIO_DATATYPE_VOICE;
	
	//Формируем ответ для управляющего устройства
	RadioPack->setHeader(dstAddress,srcAddress,dataType);
	RadioPack->setBody(pPayloadData,nPayloadSize);

	return;
}


void ProcessRadioPack(uint8_t* pPayloadData, uint16_t& nPayloadSize)
{
	nPayloadSize = 0;
	
	uint16_t nSizeOfRecData = CC1120_RxFIFONumBytes(g_CC1120Struct.hSPI);
			
	if(nSizeOfRecData<=MAX_RADIOPACK_SIZE)
		CC1120_RxData(&g_CC1120Struct,RadioPackRcvd,&nSizeOfRecData);
	else
		return;
	
	RadioMessage RadioMsgRcvd(RadioPackRcvd,nSizeOfRecData);
	
	nPayloadSize = RadioMsgRcvd.getBody(pPayloadData);

	return;
}


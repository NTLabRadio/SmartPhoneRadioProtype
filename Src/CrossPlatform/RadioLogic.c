#include "RadioLogic.h"


//Данные радиопакета для передачи
uint8_t RadioPackForSend[MAX_RADIOPACK_SIZE];

//Данные принятого радиопакета
uint8_t RadioPackRcvd[MAX_RADIOPACK_SIZE+SIZE_OF_RADIO_STATUS];

#ifdef DEBUG_CHECK_ERRORS_IN_SEND_RADIO_PACKS				
uint16_t g_cntCC1120_TxDataErrors = 0;
#endif

#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
uint16_t g_cntRcvdPacksWithErrSize = 0;
#endif


void FormAndSendRadioPack(uint8_t* pPayloadData, uint16_t nPayloadSize, uint8_t nPayloadDataType)
{	
	RadioMessage RadioMsgToSend;
	
	//Формируем радиопакет
	FormRadioPack(&RadioMsgToSend,pPayloadData,nPayloadSize,nPayloadDataType);
	
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


uint8_t SendRadioPackToTansceiver(uint8_t* pData, uint16_t nSizeData)
{
	uint8_t nRes;
	
	#ifdef DEBUG_USE_TL_LINES
	TL1_HIGH();
	#endif
	
	if(!CC1120_TxData(&g_CC1120Struct, pData, nSizeData))
	{
		#ifdef DEBUG_CHECK_ERRORS_IN_SEND_RADIO_PACKS				
		g_cntCC1120_TxDataErrors++;
		#endif
		
		nRes = 0;
	}
	else
		nRes = 1;

	#ifdef DEBUG_USE_TL_LINES
	TL1_LOW();
	#endif

	return(nRes);
}


void FormRadioPack(RadioMessage* RadioPack, uint8_t* pPayloadData, uint16_t nPayloadSize, uint8_t nPayloadDataType)
{
	//Устанавливаем широковещательный адрес
	uint8_t dstAddress = RADIO_BROADCAST_ADDR;
	//Собственный адрес берем из настроек радиомодуля
	uint8_t srcAddress = pobjRadioModule->GetRadioAddress();
	//Тип передаваемых данных
	uint8_t dataType = nPayloadDataType;
	//Размер полезных данных в пакете
	uint8_t dataSize = nPayloadSize;
	
	//Данные радиопакета
	uint8_t pBodyData[MAX_RADIOPACK_SIZE];
	memset(pBodyData,RADIOPACK_DATAMODE_SIZE,0);	
	//Размер пакета
	uint8_t nBodySize;
	switch(nPayloadDataType)
	{
		case RadioMessage::RADIO_DATATYPE_VOICE:
			nBodySize	= RADIOPACK_VOICEMODE_SIZE;
			break;
		case RadioMessage::RADIO_DATATYPE_CONF_DATA:
			nBodySize	= RADIOPACK_DATAMODE_SIZE;
			break;
		case RadioMessage::RADIO_DATATYPE_UNCONF_DATA:
			nBodySize	= RADIOPACK_DATAMODE_SIZE;
			break;		
		default:
			nBodySize	= RADIOPACK_DATAMODE_SIZE;
	}
	
	//Копируем полезные данные в начало тела пакета, остальное - нули
	memcpy(pBodyData,pPayloadData,nPayloadSize);
	
	//Формируем ответ для управляющего устройства
	RadioPack->setHeader(dstAddress,srcAddress,dataType,dataSize);
	RadioPack->setBody(pBodyData,nBodySize);

	return;
}


void ProcessRadioPack(uint8_t* pPayloadData, uint16_t& nPayloadSize, uint8_t& nDataType)
{
	nPayloadSize = 0;
	
	uint16_t nSizeOfRecData = CC1120_RxFIFONumBytes(g_CC1120Struct.hSPI);
	
	//Флаг, указыюващий на то, что данные из буфера RxFIFO прочитаны
	uint8_t flDataRcvdFromCC1120 = false;
	
	//Читаем данные из буфера RxFIFO только если размер данных адекватный (ненулевой и не слишком большой)
	if((nSizeOfRecData<=MAX_RADIOPACK_SIZE) && (nSizeOfRecData))
	{
		flDataRcvdFromCC1120 = true;
		CC1120_RxData(&g_CC1120Struct,RadioPackRcvd,&nSizeOfRecData);
	}

	//Переводим трансивер повторно в режим приема
	CC1120_Rx(g_CC1120Struct.hSPI);
	
	//Если из буфера RxFIFO ничего не прочитали, то и обрабатывать нечего
	if(!flDataRcvdFromCC1120)
	{
		#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
		g_cntRcvdPacksWithErrSize++;
		#endif
		return;
	}

	//В конце принятого буфера располагаются статус-байты, удалим их
	//TODO Сделать функцию обработки этих данных
	nSizeOfRecData-=SIZE_OF_RADIO_STATUS;
	
	RadioMessage RadioMsgRcvd(RadioPackRcvd,nSizeOfRecData);

	//Данные заголовка радиосообщения
	uint8_t dstAddress = RadioMsgRcvd.getDstAddress();
	uint8_t srcAddress = RadioMsgRcvd.getSrcAddress();
	nDataType = RadioMsgRcvd.getDataType();
	nPayloadSize = RadioMsgRcvd.getDataSize();
	
	//Данные тела радиосообщения
	RadioMsgRcvd.getBody(pPayloadData);

	return;
}


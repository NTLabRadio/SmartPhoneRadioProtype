#include "RadioLogic.h"

//Данные радиопакета для передачи
uint8_t RadioPackForSend[MAX_RADIOPACK_SIZE];

//Данные принятого радиопакета
uint8_t RadioPackRcvd[MAX_RADIOPACK_SIZE+SIZE_OF_RADIO_STATUS];

//Очередь данных статистики приема (RSSI+BER), предназначенных внешнему управляющему устройству
QueDataFrames QueReceiverStatsToExtDev(MAX_NUM_RECEIVE_STATS_IN_QUE_TO_EXT_DEV, SIZE_OF_RECEIVER_STATS);


#ifdef DEBUG_CHECK_ERRORS_IN_SEND_RADIO_PACKS				
uint16_t g_cntCC1120_TxDataErrors = 0;
#endif

#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
uint16_t g_cntRcvdPacksWithErrSize = 0;
#endif

#ifndef SEND_RECEIVER_STATS_WO_REQUEST
extern uint8_t g_flNeedSendReceiverStats;
#endif


//uint8_t SymbolPatterns[NUM_OF_SYMBOL_PATTERNS][MAX_RADIOPACK_SIZE] =
uint8_t SymbolPatterns[NUM_OF_SYMBOL_PATTERNS][RADIOPACK_MODE4800_EXTSIZE] =
{
//SYMBOL_PATTERN_ZEROES
{	
	#ifndef DEBUG_CC1120_VARIABLE_PACKET_LENGTH
	0x00, 0x01, 0x01, 0x51, 0x00,
	#else
	0x55, 0x00, 0x01, 0x01, 0x51,
	#endif
					 0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0},

//SYMBOL_PATTERN_TONE
{	
	#ifndef DEBUG_CC1120_VARIABLE_PACKET_LENGTH
	0x00, 0x01, 0x01, 0x51, 0x00,
	#else
	0x55, 0x00, 0x01, 0x01, 0x51,
	#endif	
 0x98, 0x0F, 0x0E, 0x85, 0xAC, 0x72, 0x66, 0xBF, 0x55, 0xEA, 0x51, 0x77, 0x01, 0x78, 0x14, 0x71,
 0x2D, 0x5D, 0xF8, 0x53, 0x34, 0x34, 0x48, 0x1F, 0x03, 0x6B, 0x79, 
 0xB9, 0x8F, 0x07, 0x8D, 0xAE, 0x7B, 0x71, 0x97, 0x74, 0x3E, 0x54, 0xBD, 0x61, 0x38, 0x34, 0x51, 
 0xAD, 0x57, 0xFA, 0x5B, 0x34, 0x34, 0x49, 0x1F, 0x03, 0xEB, 0x7B, 
 0xAA, 0x8B, 0x05, 0x8D, 0x6F, 0x17, 0x33, 0xB7, 0x35, 0x0F, 0x34, 0xFD, 0x01, 0x90, 0x95, 0x39, 
 0x28, 0x5D, 0xFA, 0xE3, 0xF2, 0x16, 0x21, 0x0F, 0xC2, 0xC1, 0xE9},
 
 //SYMBOL_PATTERN_ANALOG
{	
	#ifndef DEBUG_CC1120_VARIABLE_PACKET_LENGTH
	0x00, 0x01, 0x01, 0x51, 0x00,
	#else
	0x55, 0x00, 0x01, 0x01, 0x51,
	#endif
					 1,0,1, 0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,
 0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1,
 0,1,0,1,0,1,0,1, 0,1,0,1,0,1,0,1, 0,1,0,1,0,1}
};
	


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
	
	if(pobjRadioModule->IsTestMode())
	{
		uint8_t noTestPattern = pobjRadioModule->GetTestPattern();
		RadioPack->setMsg(SymbolPatterns[noTestPattern], RADIOPACK_MODE4800_EXTSIZE);
	}
	
	//TODO:
	//Для неречевых данных вызываем помехоустойчивый кодер для RadioPack
	//Кодер оставляет нетронутыми первые 2 байта (длина и адрес), делит пакет на 12-байтные кадры
	//и каждый кадр преобразует в 25 байтный. Результат снова укладывается в RadioPack
	//Для стандартных 81-байтных пакетов результат применения кодера - 175-байтный пакет

	return;
}


void ProcessRadioPack(uint8_t* pPayloadData, uint16_t& nPayloadSize, uint8_t& nDataType, uint8_t* pRadioStatusData)
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
	
	//Радиосообщение составляем из данных трансивера без статус-байтов
	uint16_t nSizeOfRadioMessage = nSizeOfRecData-SIZE_OF_RADIO_STATUS;
	RadioMessage RadioMsgRcvd(RadioPackRcvd,nSizeOfRadioMessage);

	//Данные заголовка радиосообщения
	uint8_t dstAddress = RadioMsgRcvd.getDstAddress();
	uint8_t srcAddress = RadioMsgRcvd.getSrcAddress();
	nDataType = RadioMsgRcvd.getDataType();
	nPayloadSize = RadioMsgRcvd.getDataSize();
	
	//Данные тела радиосообщения
	RadioMsgRcvd.getBody(pPayloadData);

	//Копируем отдельно статус-байты приемника. Они могут понадобиться внешнему устройству
	if(pRadioStatusData)
		memcpy(pRadioStatusData, RadioPackRcvd+nSizeOfRadioMessage, SIZE_OF_RADIO_STATUS);

	#ifdef SEND_RECEIVER_STATS
	if(nDataType==RadioMessage::RADIO_DATATYPE_VOICE)
	{
		//RSSI и LQI читаем из статус-байтов приемника
		int8_t nRSSIval = ApplyRSSIOffset(pRadioStatusData[0]);
		uint8_t nLQIAndCRCFlag = pRadioStatusData[1];
		
		//BER определяем, сравнивая принятый сигнал с шаблонным
		int8_t nBERval = BERInPack(RadioPackRcvd, nSizeOfRadioMessage, pobjRadioModule->GetTestPattern());

		#ifndef SEND_RECEIVER_STATS_WO_REQUEST
		if(g_flNeedSendReceiverStats)
		#endif
			FormAndPushToQueRecStatsMsg(nRSSIval, nLQIAndCRCFlag, nBERval);
	}
	#endif
	
	return;
}

uint8_t FormAndPushToQueRecStatsMsg(int8_t nRSSI, uint8_t nLQIAndCRCFlag, int8_t nBER)
{
	//Данные статистики приемника
	int8_t arReceiverStats[SIZE_OF_RECEIVER_STATS];
					
	arReceiverStats[0] = nRSSI;
	arReceiverStats[1] = (int8_t)nLQIAndCRCFlag;
	arReceiverStats[2] = nBER;

	QueReceiverStatsToExtDev.PushFrame((uint8_t*)arReceiverStats, SIZE_OF_RECEIVER_STATS);
	
	return(1);
}


uint8_t BERInPack(uint8_t* pPackData, uint8_t nSizePackData, uint8_t typePattern)
{
	uint8_t resBER = 100;
	uint8_t numErrors = 0;
	uint8_t cntSymbols = nSizePackData;

	if((!pPackData) || (!nSizePackData) || (typePattern>=NUM_OF_SYMBOL_PATTERNS))
		return(resBER);
	
	uint8_t *pPattern = SymbolPatterns[typePattern];
	
	while(cntSymbols--)
	{
		if( (*pPattern++) != (*pPackData++))
			numErrors++;
	}
	
	resBER = ( (uint16_t)numErrors*100 ) / nSizePackData;
	
	return(resBER);
}

int8_t ApplyRSSIOffset(int8_t nRSSIRegValue)
{
	int16_t nRSSIRealValue = (int16_t)nRSSIRegValue - (CC1120_RSSI_OFFSET-CC1120_AGC_GAIN_ADJUST);
	
	nRSSIRealValue = (nRSSIRealValue<SCHAR_MIN) ? SCHAR_MIN : nRSSIRealValue;
		
	return((int8_t)nRSSIRealValue);		
}

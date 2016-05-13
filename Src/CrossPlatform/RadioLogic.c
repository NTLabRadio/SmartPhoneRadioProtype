#include "RadioLogic.h"

//������ ����������� ��� ��������
uint8_t RadioPackForSend[RADIOPACK_MAX_SIZE];

//������ ��������� �����������
uint8_t RadioPackRcvd[RADIOPACK_MAX_SIZE+SIZE_OF_RADIO_STATUS];

//������� ������ ���������� ������ (RSSI+BER), ��������������� �������� ������������ ����������
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


//uint8_t SymbolPatterns[NUM_OF_SYMBOL_PATTERNS][RADIOPACK_MAX_SIZE] =
uint8_t SymbolPatterns[NUM_OF_SYMBOL_PATTERNS][RADIOPACK_DEFAULT_SIZE] =
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
	
	//��������� ����������
	FormRadioPack(&RadioMsgToSend, pPayloadData, nPayloadSize, nPayloadDataType);
	
	//���������� �������������� ����� � ����
	#ifndef TEST_RADIO_IMITATE
	SendRadioPackToTansceiver(RadioMsgToSend.Data, RadioMsgToSend.Size);
	//SendRadioPackToTansceiver(RadioMsgToSend.Data, 129);	//���� ������������� ������� �����������
	#else
	//� ������ �������������� �� ���������� � ����, � ����������� � ����� ��� ��������������� �� ���������� ��������
	//����������� ������ �������� ��������
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


void FormRadioPack(RadioMessage* RadioPack, uint8_t* pPayloadData, uint8_t nPayloadSize, uint8_t nPayloadDataType)
{
	//���������� ������ ������
	uint8_t nPackLength = RADIOPACK_VOICEMODE_PAYLOAD_SIZE + RadioMessage::SIZE_OF_HEADER - 1;
	//������������� ����������������� �����
	uint8_t dstAddress = RADIO_BROADCAST_ADDR;
	//����������� ����� ����� �� �������� �����������
	uint8_t srcAddress = pobjRadioModule->GetRadioAddress();
	
	//������ ������
	uint8_t nBodySize;
	switch(nPayloadDataType)
	{
		case RadioMessage::RADIO_DATATYPE_VOICE:
			nBodySize	= RADIOPACK_VOICEMODE_PAYLOAD_SIZE;
			break;
		case RadioMessage::RADIO_DATATYPE_CONF_DATA:
			nBodySize	= RADIOPACK_DATAMODE_PAYLOAD_SIZE;
			break;
		case RadioMessage::RADIO_DATATYPE_UNCONF_DATA:
			nBodySize	= RADIOPACK_DATAMODE_PAYLOAD_SIZE;
			break;		
		default:
			nBodySize	= RADIOPACK_VOICEMODE_PAYLOAD_SIZE;
	}
	
	//������ �����������
	uint8_t pBodyData[RADIOPACK_MAX_SIZE];
	//�������� �������� ������ � ������ ���� ������, ��������� - ����
	memset(pBodyData,RADIOPACK_VOICEMODE_PAYLOAD_SIZE,0);
	memcpy(pBodyData,pPayloadData,nPayloadSize);
	
	//��������� ���������������
	RadioPack->setHeader(nPackLength, dstAddress, srcAddress, nPayloadDataType, nPayloadSize);
	RadioPack->setBody(pBodyData, nBodySize);
	
	if(pobjRadioModule->IsTestMode())
	{
		//� �������� ������ ��������� ��������� ���������
		uint8_t noTestPattern = pobjRadioModule->GetTestPattern();
		RadioPack->setMsg(SymbolPatterns[noTestPattern], RADIOPACK_DEFAULT_SIZE);
	}
	
	#ifdef DEBUG_APPLY_FEC_ON_RADIODATA
	//��� ��������� ������ �������� ���������������� �����, ������� ������ ���������� ���������,
	//������� ����� � ������� ��� ������ ��������� ����� ������ 2� ���� ���������
	EncodeRadioMsg(RadioPack);
	#endif

	return;
}


void ProcessRadioPack(uint8_t* pPayloadData, uint16_t& nPayloadSize, uint8_t& nPayloadType, uint8_t* pRadioStatusData)
{
	nPayloadSize = 0;
	
	uint16_t nSizeOfRecData = CC1120_RxFIFONumBytes(g_CC1120Struct.hSPI);
	
	//����, ����������� �� ��, ��� ������ �� ������ RxFIFO ���������
	uint8_t flDataRcvdFromCC1120 = false;
	
	//������ ������ �� ������ RxFIFO ������ ���� ������ ������ ���������� (��������� � �� ������� �������)
	if((nSizeOfRecData<=RADIOPACK_MAX_SIZE) && (nSizeOfRecData))
	{
		flDataRcvdFromCC1120 = true;
		CC1120_RxData(&g_CC1120Struct,RadioPackRcvd,&nSizeOfRecData);
	}

	//��������� ��������� �������� � ����� ������
	CC1120_Rx(g_CC1120Struct.hSPI);
	
	//���� �� ������ RxFIFO ������ �� ���������, �� � ������������ ������
	if(!flDataRcvdFromCC1120)
	{
		#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
		g_cntRcvdPacksWithErrSize++;
		#endif
		return;
	}
	
	//�������������� ���������� �� ������ ���������� ��� ������-������
	uint16_t nSizeOfRadioMessage = nSizeOfRecData-SIZE_OF_RADIO_STATUS;
	RadioMessage RadioMsgRcvd(RadioPackRcvd,nSizeOfRadioMessage);

	#ifdef DEBUG_APPLY_FEC_ON_RADIODATA
	DecodeRadioMsg(&RadioMsgRcvd);
	#endif	
	
	//������ ��������� ��������������
	#ifdef DEBUG_CC1120_VARIABLE_PACKET_LENGTH
	uint8_t packLength = RadioMsgRcvd.getPackLength();
	#endif
	uint8_t dstAddress = RadioMsgRcvd.getDstAddress();
	uint8_t srcAddress = RadioMsgRcvd.getSrcAddress();
	nPayloadType = RadioMsgRcvd.getPayloadType();
	nPayloadSize = RadioMsgRcvd.getPayloadSize();
	
	//������ ���� ��������������
	RadioMsgRcvd.getBody(pPayloadData);

	//�������� �������� ������-����� ���������. ��� ����� ������������ �������� ����������
	if(pRadioStatusData)
		memcpy(pRadioStatusData, RadioPackRcvd+nSizeOfRadioMessage, SIZE_OF_RADIO_STATUS);

	#ifdef SEND_RECEIVER_STATS
	if(nPayloadType==RadioMessage::RADIO_DATATYPE_VOICE)
	{
		//RSSI � LQI ������ �� ������-������ ���������
		int8_t nRSSIval = ApplyRSSIOffset(pRadioStatusData[0]);
		uint8_t nLQIAndCRCFlag = pRadioStatusData[1];
		
		//BER ����������, ��������� �������� ������ � ���������
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
	//������ ���������� ���������
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


//����� ��������� ����������� ������ 2 ����� (����� � �����), ����� ����� �� 12-������� �����
//� ������ ���� ����������� � 25 �������. ��������� ����� ������������ � RadioPack
//��� ������������ 86-�������� ������ (c 5-������� ����������) ��������� ���������� ������ - 
//177-������� ����� (25*7+2)
void EncodeRadioMsg(RadioMessage* RadioPack)
{
	#ifdef DEBUG_USE_TL_LINES
	TL3_HIGH();
	#endif
	
	uint8_t arEncodedRadioPack[RADIOPACK_MAX_SIZE];
	memset(arEncodedRadioPack,0,RADIOPACK_MAX_SIZE);
	uint8_t sizeOfEncodedRadioPack;
	const uint8_t sizeNoFECData = 2;
	
	memcpy(arEncodedRadioPack,RadioPack->Data,sizeNoFECData);
	
	EncodeData(RadioPack->Data+sizeNoFECData, RadioPack->Size-sizeNoFECData, arEncodedRadioPack+sizeNoFECData, sizeOfEncodedRadioPack);
	
	#ifdef DEBUG_TEST_ENCDEC_RADIODATA_TRELLIS
	uint8_t arDecodedRadioPack[RADIOPACK_MAX_SIZE];
	memset(arDecodedRadioPack,0,RADIOPACK_MAX_SIZE);
	uint8_t sizeOfDecodedRadioPack;
	int16_t numDetectedErrors = 0;
	
	DecodeData(arEncodedRadioPack+sizeNoFECData, sizeOfEncodedRadioPack, 
							arDecodedRadioPack, sizeOfDecodedRadioPack, numDetectedErrors);
	#endif
	
	sizeOfEncodedRadioPack+=sizeNoFECData;
	
	#ifdef DEBUG_CC1120_VARIABLE_PACKET_LENGTH
	//������������ ����� ������, ������� �������� � ������ (����������������!) ����� ��������������
	arEncodedRadioPack[0] = sizeOfEncodedRadioPack - 1;
	#endif
	
	RadioPack->setMsg(arEncodedRadioPack,sizeOfEncodedRadioPack);
	
	#ifdef DEBUG_USE_TL_LINES
	TL3_LOW();	
	//� ������� TL-����� ���������� ����� ����������� �����������
	//��� ���������� 2-3 �� ��� ���������� �����������
	#endif
}


void DecodeRadioMsg(RadioMessage* RadioPack)
{
	#ifdef DEBUG_USE_TL_LINES
	TL3_HIGH();
	#endif
	
	int16_t numDetectedErrors = 0;

	uint8_t arDecodedRadioPack[RADIOPACK_MAX_SIZE];
	memset(arDecodedRadioPack,0,RADIOPACK_MAX_SIZE);
	uint8_t sizeOfDecodedRadioPack;	
	const uint8_t sizeNoFECData = 2;
	
	memcpy(arDecodedRadioPack,RadioPack->Data,sizeNoFECData);
	
	DecodeData(RadioPack->Data+sizeNoFECData, RadioPack->Size-sizeNoFECData, 
							arDecodedRadioPack+sizeNoFECData, sizeOfDecodedRadioPack, numDetectedErrors);
	
	arDecodedRadioPack[0] = sizeOfDecodedRadioPack;
	
	RadioPack->setMsg(arDecodedRadioPack, sizeOfDecodedRadioPack+sizeNoFECData);
	
	#ifdef DEBUG_USE_TL_LINES
	TL3_LOW();	
	#endif
}


uint8_t EncodeData(uint8_t* pDataIn, uint8_t nSizeDataIn, uint8_t* pDataOut, uint8_t& nSizeDataOut)
{
	if((!pDataIn) || (!nSizeDataIn) || (!pDataOut))
		return(0);
	
	nSizeDataOut = 0;
	
	const uint16_t numBytesInUncodedFrame = ceil(double(SIZE_OF_UNCODED_FRAME_TRELLIS_3_4)/8);
	const uint16_t numBytesInCodedFrame = floor(double(SIZE_OF_CODED_FRAME_TRELLIS)/8);
	const uint16_t numAddBitsInCodedFrame = SIZE_OF_CODED_FRAME_TRELLIS - 8*numBytesInCodedFrame;
	uint8_t arRestBits[8];
	uint8_t numRestBits = 0;
	
	//������ ���������� ���������. ����� ����� ������:
	const uint16_t numFrames = ceil(double(nSizeDataIn)/numBytesInUncodedFrame);

	//����� ��/����� �����������
	uint8_t pBinDataIn[SIZE_OF_UNCODED_FRAME_TRELLIS_3_4];
	uint8_t pBinDataOut[SIZE_OF_CODED_FRAME_TRELLIS + 8];
	
	uint16_t cntFrames;
	for(cntFrames=0; cntFrames<numFrames; cntFrames++)
	{
		//������� ����������� ��������� � �������� ��������� (������ ��� - � ��������� �����)
		//������������� � ������� ������
		HexToBinArray(pDataIn+cntFrames*numBytesInUncodedFrame, numBytesInUncodedFrame, pBinDataIn);
		
		trellisEnc3_4((const int8_t*)pBinDataIn, (int8_t*)pBinDataOut);

		//������������ ������ �� �������� ������� � �������� ������
		//��������! ��� ������� �������������, ��� numAddBitsInCodedFrame==4 
		//(��� ����� ��� ������� � �������� ������� 196 ���, �� ����� ���� ������� ��� ������ �������)
		if(!numRestBits)
		{
			BinToHexArray(pBinDataOut, numBytesInCodedFrame, pDataOut+nSizeDataOut);
			nSizeDataOut+=numBytesInCodedFrame;
			
			//��������� ����, ���������� ���������������
			memcpy(arRestBits, pBinDataOut+8*numBytesInCodedFrame, numAddBitsInCodedFrame);
			numRestBits += numAddBitsInCodedFrame;
		}
		else
		{
			//��������� ���������� � ����������� ����� ����
			memmove(pBinDataOut+numRestBits, pBinDataOut, SIZE_OF_CODED_FRAME_TRELLIS);
			memcpy(pBinDataOut,arRestBits,numRestBits);
			numRestBits = 0;
			
			BinToHexArray(pBinDataOut, numBytesInCodedFrame+1, pDataOut+nSizeDataOut);
			nSizeDataOut += (numBytesInCodedFrame+1);
		}		
	}
	
	if(numRestBits)
	{
		BinToHexArray(arRestBits, 1, pDataOut+nSizeDataOut);
		nSizeDataOut++;
	}
	
	return(1);
}


uint8_t DecodeData(uint8_t* pDataIn, uint8_t nSizeDataIn, uint8_t* pDataOut, uint8_t& nSizeDataOut, int16_t& nDetectedErrors)
{
	if((!pDataIn) || (!nSizeDataIn) || (!pDataOut))
		return(0);
	
	nSizeDataOut = 0;
	nDetectedErrors = 0;

	uint16_t nSizeProcessedDataIn = 0;
	uint16_t nSizeBinDataIn;
	const uint16_t numBytesInUncodedFrame = ceil(double(SIZE_OF_UNCODED_FRAME_TRELLIS_3_4)/8);
	const uint16_t numBytesInCodedFrame = floor(double(SIZE_OF_CODED_FRAME_TRELLIS)/8);
	const uint16_t numAddBitsInCodedFrame = SIZE_OF_CODED_FRAME_TRELLIS - 8*numBytesInCodedFrame;
	uint8_t arRestBits[8];
	uint8_t numRestBits = 0;

	//������ ������������ ���������. ����� ����� ������:
	const uint16_t numFrames = floor(double(8*nSizeDataIn)/SIZE_OF_CODED_FRAME_TRELLIS);

	//����� ��/����� �������������
	uint8_t pBinDataIn[SIZE_OF_CODED_FRAME_TRELLIS + 8];
	uint8_t pBinDataOut[SIZE_OF_UNCODED_FRAME_TRELLIS_3_4];
	
	uint16_t cntFrames;
	for(cntFrames=0; cntFrames<numFrames; cntFrames++)
	{
		nSizeBinDataIn = 0;
		
		//������� ������������� ��������� � �������� ��������� (������ ��� - � ��������� �����)
		//������������� � ������� ������
		if(!numRestBits)
		{
			//������������� ����� ����� ����
			HexToBinArray(pDataIn+nSizeProcessedDataIn, numBytesInCodedFrame, pBinDataIn);
			nSizeProcessedDataIn += numBytesInCodedFrame;
			nSizeBinDataIn+=8*numBytesInCodedFrame;
			
			//���� �������� ��������� ��� (<8) ����������������
			if(numAddBitsInCodedFrame)	
			{
				//������������� ��� ���� ����. ���� ���� ����� � ������ pBinDataIn, ���� ��� ������������� �������� �����
				//����������� ������ numAddBitsInCodedFrame ���
				HexToBinArray(pDataIn+nSizeProcessedDataIn, 1, pBinDataIn+nSizeBinDataIn);
				nSizeProcessedDataIn++;
				//��������� ��, ��� �� ���������� � ������� ����
				numRestBits = 8-numAddBitsInCodedFrame;
				memcpy(arRestBits, pBinDataIn+8*numBytesInCodedFrame+numAddBitsInCodedFrame, numRestBits);
			}
		}
		else
		{
			//�������� � ������������ ������ ��, ��� �� ����������� � ���������� ����
			memcpy(pBinDataIn,arRestBits,numRestBits);
			nSizeBinDataIn += numRestBits;
			numRestBits = 0;
			
			//������������� ����� ����� ����
			HexToBinArray(pDataIn+nSizeProcessedDataIn, numBytesInCodedFrame, pBinDataIn+nSizeBinDataIn);
			nSizeProcessedDataIn += numBytesInCodedFrame;
		}	
		
		nDetectedErrors += trellisDec3_4((const int8_t*)pBinDataIn, (int8_t*)pBinDataOut);

		//������������ ������ �� �������� �������
		BinToHexArray(pBinDataOut, numBytesInUncodedFrame, pDataOut+cntFrames*numBytesInUncodedFrame);
		
		
		nSizeDataOut += numBytesInUncodedFrame;
	}
	
	
	return(1);
}



uint8_t HexToBinArray(const uint8_t * const pHexDataIn, const uint16_t sizeHexDataIn, uint8_t* pBinDataOut)
{
	uint16_t cntBytes;
	uint16_t cntBits = 0;
	uint8_t posBit;
	uint8_t nByte;
	
	for(cntBytes=0; cntBytes<sizeHexDataIn; cntBytes++)
	{
		nByte = pHexDataIn[cntBytes];

		for(posBit=0; posBit<8; posBit++)
		{
			//No: ��� ��������� ������� ���. ������� ������ � ����� ������� � pBinDataOut ������
			pBinDataOut[cntBits++] = nByte&1;
			nByte = nByte>>1;			
		}
	}
	
	return(1);
}

uint8_t BinToHexArray(const uint8_t * const pBinDataIn, const uint16_t sizeHexDataOut, uint8_t* pHexDataOut)
{
	uint16_t cntBytes;
	uint16_t cntBits = 0;
	uint8_t posBit;
	uint8_t nByte;
	
	for(cntBytes=0; cntBytes<sizeHexDataOut; cntBytes++)
	{
		nByte = 0;
		
		for(posBit=0; posBit<8; posBit++)
			nByte = nByte | pBinDataIn[cntBits++]<<posBit;
		
		pHexDataOut[cntBytes] = nByte;
	}
	
	return(1);
}

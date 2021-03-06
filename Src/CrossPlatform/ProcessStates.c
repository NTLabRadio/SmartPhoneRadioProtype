#include "ProcessStates.h"

//����� ���� ������ �������������� ������ �� ���� ���������� � �������� �����������
RadioModule *pobjRadioModule;

CMX7262_TypeDef  g_CMX7262Struct;
CC1120_TypeDef  g_CC1120Struct;

uint8_t g_flCMX7262_IRQ_CHECKED = FALSE;
uint8_t g_flCC1120_IRQ_CHECKED = FALSE;


uint8_t pDataFromCMX7262[MAX_SIZE_OF_DATA_FROM_CMX7262];
uint16_t nLengthDataFromCMX7262 = 0;

uint8_t pDataToCMX7262[MAX_SIZE_OF_DATA_TO_CMX7262];
uint16_t nLengthDataToCMX7262 = 0;


#ifdef DEBUG_CMX7262_CNT_TX_AUDIO_BUF
uint16_t cntCMX7262TxAudioBuf = 0;
#endif


#define MAX_NUM_RADIOPACKS_IN_QUE_FROM_EXT_DEV 	(5)
//������� ������� ������, �������� �� �������� ������������ ���������� ��� �������� � ��������������
QueDataFrames QueDataFromExtDev(MAX_NUM_RADIOPACKS_IN_QUE_FROM_EXT_DEV, RADIOPACK_MAX_SIZE);

#define MAX_NUM_RADIOPACKS_IN_QUE_TO_EXT_DEV 	(5)
//������� ������� ������, �������� �� ���������������, ��������������� �������� ������������ ����������
QueDataFrames QueDataToExtDev(MAX_NUM_RADIOPACKS_IN_QUE_TO_EXT_DEV, RADIOPACK_MAX_SIZE);


#ifdef DEBUG_CHECK_ERRORS_IN_SEND_RADIO_PACKS
uint16_t g_cntSendRadioPacks = 0;
#endif

#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
uint16_t g_cntRcvdRadioPacks = 0;
#endif


extern uint8_t SymbolPatterns[NUM_OF_SYMBOL_PATTERNS][RADIOPACK_DEFAULT_SIZE];


// ------------------------------- �������� ������ �������� �������� ������� -------------------------------------
//
//1. ��� ������� �� �������� � ����������� ProcessPTTState():
//	1.1 ������������� ��������� ����������� � RADIOMODULE_STATE_TX_WAITING;
//	1.2 ��������� ������� CMX7262 � ��������� ����������� ������ � ����������;
//
//2. ��������� ������� ��������� � ��������� �����������, �� ��� ������ ������������� ����� 60-�� ����, �������
//������ ���������� ODA. � ����������� ProcessCMX7262State() �����������, ��� �� ����������. ���� ����, ��
//ARM �������� ������ � ���������� �� � ������� �� ��������.
//
//3. � ����������� ProcessRadioState(), ���� ����������� ��������� ����������� � RADIOMODULE_STATE_TX_WAITING, 
//���������, ���������� �� ������ ��������� �� ��������. ���� ������ �������� ����������, �������� ��������� 
//���������� �� RADIOMODULE_STATE_TX_RUNNING � ��������� FrontEnd � ��������
//
//4. � ����������� ProcessRadioState(), ���� ����������� ��������� ����������� RADIOMODULE_STATE_TX_RUNNING:
//	4.1 ���������, ���������� �� � ������� �� �������� ������ ��� ������������ ������ ������ � �������� �� � ������
//	������ ���������� CC1120; ���� ��� ������� �����������, �� ��������� ����� �� ������ ������� �������� �
//	��������� ������ � ���������� ��� � CC1120 ��� �������� � ����;
//	4.2 ���� ��������� ������, �� ����������, ��� ���������� ��������� � �������� ��������� ��������; ������, 
//	���������� � CC1120, ������� �� ������� ��� ��������;
//	4.3 ���� �� ��1120 ������� ���������� � ���, ��� ����� �������, �������, ��� ���������� ��������� � ���������
//	��������� � ����� ���������� ��������� ����� ������
//
//
// -------------------------------- �������� ������ ������ �������� ������� --------------------------------------
//
//1. ��� ���������� �������� � ����������� ProcessPTTState():
//	1.1 ������������� ��������� ����������� RADIOCHAN_STATE_WAIT_RECEIVE (��������� ���������� � ������);
//	1.2 ��������� ������� CMX7262 � ��������� ������������� ������ � ��������������� ���������� �� ����������;
//	1.3 ���������� ���������� ���������� �� �������� � ���, ��� �� ����� ��������� ������ (��� �� �� ������ 
//	���������� IDW, ���� � ���� �� �������� ������);
//
//2. � ����������� ProcessRadioState(), ���� ����������� ��������� ����������� � RADIOMODULE_STATE_RX_WAITING:
//	2.1 ��������� ��������� � ����� ������;
//	2.2 �������� ��������� ���������� �� RADIOMODULE_STATE_RX_RUNNING (��������������� �����)
//	2.3 ��������� FrontEnd � �����
//
//3. � ����������� ProcessRadioState(), ���� ����������� ��������� ����������� � RADIOMODULE_STATE_RX_RUNNING:
//	3.1 ���������, ��� �� ����������, ������������, ��� ��������� CC1120 ������ ������;
//	3.2 ���� ���� ���������� �� ���������� � � ��� ������ RxFIFO ���� ������, �������� �� �� ����������;
//	3.3 ���������� ������ � ������� ��� ��������, ���� � ��� ���� �����
//
//4. � ����������� ��������� �������� ProcessCMX7262State(), ��������� �� ���� �� ���������� CMX7262_IDW, 
// ������������ �� ��, ��� ������� ����� ������������ � �������������� ��������� ����� ������. ���� ����������
// ����, ��:
//	4.1 ���� � ������� ��� �������� ���� ���� �� ���� ������ ����� (60 �� �����), ���������� �� � CMX7262;
//	4.2 ������� �� ������� ���������� �������� ������



void RadioModuleInit(SPI_HandleTypeDef *hspiCMX7262, SPI_HandleTypeDef *hspiCC1120)
{
	//������������� CMX7262: �������� ������ � ������, ��������� ���������
	CMX7262_Init(&g_CMX7262Struct, hspiCMX7262);

	//������� CMX7262 � ����� Idle
	CMX7262_Idle(&g_CMX7262Struct);	
	
	#ifndef TEST_CMX7262
	//������������� ��1120
	CC1120_Init(&g_CC1120Struct, hspiCC1120);
	#endif
	
	//����� ����, ��� ��� ������������ ���������� ���������� ���������, ������� ������
	//��� ���������� ������ ����������� �����������
	pobjRadioModule = new RadioModule;	
}


void RadioModuleDeInit()
{
	delete pobjRadioModule;
}


//������� ��������� ��������� ��������
//TODO �� ���������� �����������
void ProcessPTTState()
{
	//���� � ������ ������ �� �������� ������
	if(pobjRadioModule->GetTxTrafficType()!=TRAFFIC_TYPE_DATA)
	{
		//���� ������ �������
		if(PTT_PRESSED())
		{
			//��������� ��������� �����������
			//���� �� ��� ��� �� ��������� � ��������
			if( !pobjRadioModule->isTxMode() )
			{
				//�������� ��������� ����������� �� "��������"
				pobjRadioModule->SetRadioChanState(RADIOCHAN_STATE_TRANSMIT);
				
				//�������� ������� ��������� ����������� �� "���������� � ��������"
				pobjRadioModule->SetRadioModuleState(RADIOMODULE_STATE_TX_WAITING);
				
				VocoderStartEncode();
				
				nLengthDataFromCMX7262 = 0;
				nLengthDataToCMX7262 = 0;
			}
		}
		else
		{
			//��������� ��������� �����������
			//���� �� ��� ��� �� ��������� � ������
			if( !pobjRadioModule->isRxMode() )
			{
				//�������� ��������� ����������� �� "�����"
				pobjRadioModule->SetRadioChanState(RADIOCHAN_STATE_WAIT_RECEIVE);
				
				//�������� ������� ��������� ����������� �� "���������� � ������"
				pobjRadioModule->SetRadioModuleState(RADIOMODULE_STATE_RX_WAITING);
				
				VocoderStartDecode();
				
				#ifndef TEST_RADIO_IMITATE
				nLengthDataToCMX7262 = 0;
				#endif
				nLengthDataFromCMX7262 = 0;
			}
		}	//if(PTT_PRESSED())
	}	//if(pobjRadioModule->GetTxTrafficType()!=TRAFFIC_TYPE_DATA)
}


void RadioImitator_TxData(uint8_t* pPackData, uint16_t packSize)
{
	if(nLengthDataToCMX7262 <= MAX_SIZE_OF_DATA_TO_CMX7262-packSize)
	{
		memcpy(&pDataToCMX7262[nLengthDataToCMX7262],pPackData,packSize);

		nLengthDataToCMX7262+=packSize;
	}

	g_flCC1120_IRQ_CHECKED = TRUE;
}


void ProcessRadioState()
{
	//TODO ���� switch ����� �������� �� 4 ������� � �������� �������-����������, �� ������� ���������� ��������� ������� �������
	switch(pobjRadioModule->GetRadioModuleState())
	{		
		case RADIOMODULE_STATE_TX_WAITING:
			ProcessTxWaiting();
			break;

		case RADIOMODULE_STATE_TX_RUNNING:
			ProcessTxRunning();
			break;

		case RADIOMODULE_STATE_RX_WAITING:
			ProcessRxWaiting();
			break;

		case RADIOMODULE_STATE_RX_RUNNING:
			ProcessRxRunning();
			break;
			
		default:
			break;
	}
}


void ProcessTxWaiting()
{
	//���� �������� ���������� �������� ������ �� ��������, ������������� � ����� RADIOMODULE_STATE_TX_RUNNING
	if( (nLengthDataFromCMX7262 > SIZE_OF_DATA_FROM_CMX7262_INITACCUM_FOR_TX) ||
	//��� ���� ������ ��� �������� �� �������� ����������
	(!QueDataFromExtDev.isEmpty()) )
	{
		pobjRadioModule->SetRadioModuleState(RADIOMODULE_STATE_TX_RUNNING);
		//��������, ��� ���������� ��������� � ��������� ��������� � ����� ���������� ��������� ����� ������
		g_CC1120Struct.TxState = CC1120_TX_STATE_WAIT;
		
		#ifndef SMART_PROTOTYPE
		FrontEndSetToTx();			
		#endif
	}
}


void ProcessTxRunning()
{
	//���� ���� ���������� � ���, ��� ����� �������
	if(g_flCC1120_IRQ_CHECKED)
	{
		//���������� ���� ����������
		g_flCC1120_IRQ_CHECKED = FALSE;

		//����������, ��� ���������� ��������� � ��������� ��������� � ����� ���������� ��������� ����� ������
		g_CC1120Struct.TxState = CC1120_TX_STATE_WAIT;
		
		//���� ���������� ����� ������, �� ��������� � ����� ������
		if(pobjRadioModule->GetTxTrafficType()==TRAFFIC_TYPE_DATA)
		{
			//�������� ��������� ����������� �� "�����"
			pobjRadioModule->SetRadioChanState(RADIOCHAN_STATE_WAIT_RECEIVE);
			
			//�������� ������� ��������� ����������� �� "���������� � ������"
			pobjRadioModule->SetRadioModuleState(RADIOMODULE_STATE_RX_WAITING);
			return;
		}
	}

	//���� ���������� CC1120 ��������, �� ����� ���������� ������
	if(g_CC1120Struct.TxState!=CC1120_TX_STATE_ACTIVE)
	{
		//���� � ������� �� �������� ���������� ������ ��� ������������ ������ �����������, �� �������� ������ � ���������
		if(nLengthDataFromCMX7262 >= RADIOPACK_VOICEMODE_PAYLOAD_SIZE)
		{
			pobjRadioModule->SetTxTrafficType(TRAFFIC_TYPE_VOICE);
			
			//�� ������ �������� ��������� ���������� � ���������� ��� � ���������
			FormAndSendRadioPack(pDataFromCMX7262, RADIOPACK_VOICEMODE_PAYLOAD_SIZE, RadioMessage::RADIO_DATATYPE_VOICE);

			//������� ���������� ������ �� ������� ������ �� ��������
			RemDataFromFIFOBuf(pDataFromCMX7262, nLengthDataFromCMX7262, RADIOPACK_VOICEMODE_PAYLOAD_SIZE);
			
			//����������, ��� ������ ���������� ��������� � �������� ��������� ��������
			g_CC1120Struct.TxState = CC1120_TX_STATE_ACTIVE;
		}
		
		//���� ���� ������ ��� �������� �� �������� ����������
		if(!QueDataFromExtDev.isEmpty())
		{
			pobjRadioModule->SetTxTrafficType(TRAFFIC_TYPE_DATA);
			
			uint8_t pDataPack[RADIOPACK_DATAMODE_PAYLOAD_SIZE];
			//�������� �� ������� ���� ����� ������ ��� ��������
			uint16_t sizePack = QueDataFromExtDev.PopFrame(pDataPack);
			
			//��������� ���������� � ���������� ��� � ���������
			FormAndSendRadioPack(pDataPack, sizePack, RadioMessage::RADIO_DATATYPE_CONF_DATA);
				
			#ifdef DEBUG_CHECK_ERRORS_IN_SEND_RADIO_PACKS				
			g_cntSendRadioPacks++;
			#endif
			
			//����������, ��� ������ ���������� ��������� � �������� ��������� ��������
			g_CC1120Struct.TxState = CC1120_TX_STATE_ACTIVE;
		}
	}
}


void ProcessRxWaiting()
{
	//�������������� ��������� � ������ ������
	TransceiverStartRx();

	pobjRadioModule->SetRadioModuleState(RADIOMODULE_STATE_RX_RUNNING);

	pobjRadioModule->SetTxTrafficType(TRAFFIC_TYPE_IDLE);

	#ifndef SMART_PROTOTYPE
	FrontEndSetToRx();
	#endif
}


void ProcessRxRunning()
{
	//���������, ��� �� ����������, ������������, ��� ��������� ������ ������
	if(g_flCC1120_IRQ_CHECKED)
	{
		uint8_t pRadioPayloadData[RADIOPACK_MAX_SIZE];
		uint16_t nSizeOfRadioPayload = 0;
		uint8_t nRadioPayloadType;
		uint8_t arRadioStatusData[SIZE_OF_RADIO_STATUS];
		
		//�������� ������ �� ������ RxFIFO ����������
		ProcessRadioPack(pRadioPayloadData, nSizeOfRadioPayload, nRadioPayloadType, arRadioStatusData);

		#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
		g_cntRcvdRadioPacks++;
		#endif
		
		//��������, ������� �� ����-��� ������ �� ����������
		if(nSizeOfRadioPayload)
		{
			if(nRadioPayloadType==RadioMessage::RADIO_DATATYPE_VOICE)
			{
				//�������� �������� ������ ��������� ����������� � ������� ��� ��������, ���� � ��� ���� �����
				if( (nLengthDataToCMX7262 <= MAX_SIZE_OF_DATA_TO_CMX7262-nSizeOfRadioPayload)
				 && (nSizeOfRadioPayload == RADIOPACK_VOICEMODE_PAYLOAD_SIZE) )
					AddDataToFIFOBuf(pDataToCMX7262, nLengthDataToCMX7262, pRadioPayloadData, nSizeOfRadioPayload);
			}
			else
			{
				//����� ������, �������� �� ���������������, ������ �� ������-������� �������� � ������� ��� �������� ����������
				memcpy(pRadioPayloadData+nSizeOfRadioPayload, arRadioStatusData, SIZE_OF_RADIO_STATUS);
				QueDataToExtDev.PushFrame(pRadioPayloadData, nSizeOfRadioPayload+SIZE_OF_RADIO_STATUS);
			}
		}
		//���������� ���� ����������
		g_flCC1120_IRQ_CHECKED = FALSE;
	}	//if(g_flCC1120_IRQ_CHECKED)
	else
	{
		//���� ��� �������� ������, �� ��������� ������� RSSI � �������� ��� �� ������� ����������
		#ifdef SEND_RECEIVER_STATS
		//ProcessRadioState() ���������� �����, ��� ����� �������� RSSI ������������ �� ����
		if(isNeedCheckRSSI())
		{
			//������ � ���������� ������� ������� RSSI
			int8_t nRSSIval = ApplyRSSIOffset(CC1120_CheckRSSI(g_CC1120Struct.hSPI));
			//��������� ������� ����, ������� �������, ��� LQI=127 � CRC �������
			uint8_t nLQIAndCRCFlag = 127;
			//��������� ������� ����, ������� �������, ��� BER = 100%
			int8_t nBERval = 100;
			
			#ifdef DEBUG_SHOW_CC1120_AGC_INSTEAD_RSSI
			int8_t nAGCgain = CC1120_AGCGain(g_CC1120Struct.hSPI);
			nRSSIval = nAGCgain;
			#endif

			#ifndef DEBUG_SEND_RECEIVER_STATS_WO_REQUEST
			if(pobjRadioModule->IsAsyncReqReceiverStats())
			#endif
				FormAndPushToQueRecStatsMsg(nRSSIval, nLQIAndCRCFlag, nBERval);
		}
		#endif
	}	//if(g_flCC1120_IRQ_CHECKED) ... else ...
	
	//���� ���� ������ ��� �������� �� �������� ����������, �� ��������� � ����� ��������
	if(!QueDataFromExtDev.isEmpty())
	{
		//�������� ��������� ����������� �� "��������"
		pobjRadioModule->SetRadioChanState(RADIOCHAN_STATE_TRANSMIT);
		
		//�������� ������� ��������� ����������� �� "���������� � ��������"
		pobjRadioModule->SetRadioModuleState(RADIOMODULE_STATE_TX_WAITING);
		
		pobjRadioModule->SetTxTrafficType(TRAFFIC_TYPE_DATA);
	}
}	//ProcessRxRunning()


void CMX7262_TestMode()
{
	#ifdef TEST_CMX7262_ENC_PATTERN
	CMX7262_Encode_CBUS2CBUS(&g_CMX7262Struct);
	
	uint8_t numFrames = 3;
	FillBufByToneSignal((int16_t*)pDataToCMX7262, numFrames*CMX7262_AUDIOFRAME_SIZE_SAMPLES, CMX7262_FREQ_SAMPLING, 1000);
	CMX7262_TxFIFO_Audio(&g_CMX7262Struct, (uint8_t *)&pDataToCMX7262[0], numFrames);
	#endif
	
	#ifdef TEST_CMX7262_DEC_PATTERN
	VocoderStartDecode();
	#endif
	
	#ifdef TEST_CMX7262_ENCDEC_AUDIO2AUDIO_MODE
	CMX7262_EncodeDecode_Audio(&g_CMX7262Struct);	
	#endif

	#ifdef TEST_CMX7262_AUDIO_TESTMODE
	CMX7262_Test_AudioOut(&g_CMX7262Struct);
	#endif
	
	#ifdef TEST_CMX7262_ENCDEC_AUDIO2CBUS_MODE
	CMX7262_EncodeDecode_Audio2CBUS(&g_CMX7262Struct);
	#endif
	
	#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_MODE
		#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_INTERNAL_SIN
		CMX7262_EncodeDecode_CBUS2Audio(&g_CMX7262Struct);
		
		//��������� ����� ��������� �������� 1���
		FillBufByToneSignal((int16_t*)pDataToCMX7262,CMX7262_AUDIOFRAME_SIZE_SAMPLES,CMX7262_FREQ_SAMPLING,1000);
		CMX7262_TxFIFO_Audio(&g_CMX7262Struct,(uint8_t *)&pDataToCMX7262[0],1);
		#endif
	#endif
}


void ProcessCMX7262State()
{
	//���� ���� ���������� �� CMX7262
	if(g_flCMX7262_IRQ_CHECKED)
	{
		//������������ ����������: ���������, ��� ����� CMX7262                                                                                       
		//������ ��������� ������� � ������������� �������� g_CMX7262Struct.uIRQRequest		
		CMX7262_IRQ(&g_CMX7262Struct);
		//���������� ����, ����� ���������� ��������� ����������
		g_flCMX7262_IRQ_CHECKED = FALSE;
	}
	
	//���������, ��� �� ���������� � ������� ����� ������ � �������� 
	if((g_CMX7262Struct.uIRQRequest & CMX7262_ODA) == CMX7262_ODA)
	{
		//���������� ���� CMX7262_ODA
		g_CMX7262Struct.uIRQRequest = g_CMX7262Struct.uIRQRequest & ~CMX7262_ODA;
		
		//------- ������ ������ ----------
		#ifndef TEST_CMX7262	
		//������� �����
			//���� ������ ��� ���� ���� ������������
			if(nLengthDataFromCMX7262 <= MAX_SIZE_OF_DATA_FROM_CMX7262-CMX7262_CODEC_BUFFER_SIZE)
			{
				//�������� �� � CMX7262 � ���������� � �������
				CMX7262_RxFIFO(&g_CMX7262Struct,(uint8_t *)&pDataFromCMX7262[nLengthDataFromCMX7262]);
				nLengthDataFromCMX7262 += CMX7262_CODEC_BUFFER_SIZE;
			}
			#ifdef DEBUG_PRINTF_CMX7262_EXCHANGE
			else
				printf("Bufer pDataFromCMX7262 is Full");
			#endif
		#else
		//�������� ������
			#ifdef TEST_CMX7262_ENCDEC_AUDIO2CBUS_MODE
			CMX7262_RxFIFO_Audio(&g_CMX7262Struct,(uint8_t *)&pDataFromCMX7262[0]);
			#endif
			
			#ifdef TEST_CMX7262_ENC_PATTERN
			//���� ������ ��� ���� ���� ������������
			if(nLengthDataFromCMX7262 <= MAX_SIZE_OF_DATA_FROM_CMX7262-CMX7262_CODEC_BUFFER_SIZE)
			{
				//�������� �� � CMX7262 � ���������� � �������
				CMX7262_RxFIFO(&g_CMX7262Struct,(uint8_t *)&pDataFromCMX7262[nLengthDataFromCMX7262]);
				nLengthDataFromCMX7262 += CMX7262_CODEC_BUFFER_SIZE;
				
				nLengthDataFromCMX7262 %= 3*CMX7262_CODEC_BUFFER_SIZE;
			}
			
			uint8_t numFrames = 3;
			FillBufByToneSignal((int16_t*)pDataToCMX7262, numFrames*CMX7262_AUDIOFRAME_SIZE_SAMPLES, CMX7262_FREQ_SAMPLING, 1000);
			CMX7262_TxFIFO_Audio(&g_CMX7262Struct, (uint8_t *)&pDataToCMX7262[0], numFrames);
			#endif
			
		#endif
	}

	//���������, ��� �� ���������� � ���, ��� ������� ����� ������� ��������� ������ ������
	if((g_CMX7262Struct.uIRQRequest & CMX7262_IDW) == CMX7262_IDW)
	{
		//�������� ������ �� CMX7262
		
	#ifndef TEST_CMX7262
		//������� �����
		#ifndef TEST_RADIO_IMITATE
		//���� � ������� ��� �������� ���� ���� �� ���� ������ ����� ������� ������, ���������� �� � CMX7262
		if(nLengthDataToCMX7262>=CMX7262_CODEC_BUFFER_SIZE)
		#else
		//� �������� ������ ������������� ���������, �� ������ �� ��������
		//��� ������� �������� ������ �� �������������
		if((nLengthDataToCMX7262>=CMX7262_CODEC_BUFFER_SIZE) && (!PTT_PRESSED()))
		#endif
		{
			CMX7262_TxFIFO(&g_CMX7262Struct,(uint8_t *)&pDataToCMX7262[0]);

			//������� �� ������� ���������� �������� ������
			//TODO �������� pDataToCMX7262 �� ��������� �����			
			RemDataFromFIFOBuf(pDataToCMX7262, nLengthDataToCMX7262, CMX7262_CODEC_BUFFER_SIZE);			
			
			//���������� ���� CMX7262_IDW
			g_CMX7262Struct.uIRQRequest = g_CMX7262Struct.uIRQRequest & ~CMX7262_IDW;

		}
		#ifdef DEBUG_PRINTF_CMX7262_EXCHANGE
		else
			printf("Bufer pDataToCMX7262 is Empty");
		#endif
		
	#else
	//�������� ������
		#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_MODE			
			#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_INTERNAL_SIN
			//��������� ����� ��������� �������� 1���
			FillBufByToneSignal((int16_t*)pDataToCMX7262,CMX7262_AUDIOFRAME_SIZE_SAMPLES,CMX7262_FREQ_SAMPLING,1000);
			#endif
			
			//�������� ����� �� CMX7262
			CMX7262_TxFIFO_Audio(&g_CMX7262Struct,(uint8_t *)&pDataToCMX7262[0],1);
		
			#ifdef DEBUG_CMX7262_CNT_TX_AUDIO_BUF
			cntCMX7262TxAudioBuf++;
			#endif
			
			uint16_t nSizeOfTxBuf = sizeof(uint16_t)*CMX7262_AUDIOFRAME_SIZE_SAMPLES;		
			if(nLengthDataToCMX7262>=nSizeOfTxBuf)
			{
				//������� �� ������� ���������� �������� ������
				//TODO �������� pDataToCMX7262 �� ��������� �����
				RemDataFromFIFOBuf(pDataToCMX7262, nLengthDataToCMX7262, nSizeOfTxBuf);
			}
			#ifdef DEBUG_PRINTF_CMX7262_EXCHANGE
			else
				printf("Bufer pDataToCMX7262 is Empty");
			#endif
		#endif	//#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_MODE
				
		#ifdef TEST_CMX7262_DEC_PATTERN
		memcpy(pDataToCMX7262,&SymbolPatterns[SYMBOL_PATTERN_TONE][nLengthDataToCMX7262],CMX7262_CODEC_BUFFER_SIZE);
		CMX7262_TxFIFO(&g_CMX7262Struct,(uint8_t *)&pDataToCMX7262[0]);
		nLengthDataToCMX7262+=CMX7262_CODEC_BUFFER_SIZE;
		nLengthDataToCMX7262 %= 3*CMX7262_CODEC_BUFFER_SIZE;

		//���������� ���� CMX7262_IDW
		g_CMX7262Struct.uIRQRequest = g_CMX7262Struct.uIRQRequest & ~CMX7262_IDW;
		#endif
				
	#endif	//#ifndef TEST_CMX7262 ... #else
	}

}


void TransceiverStartRx()
{
	//������� Rx FIFO ����������
	CC1120_RxFIFOFlush(g_CC1120Struct.hSPI);
	//������� ���������� � ����� ������
	CC1120_Rx(g_CC1120Struct.hSPI);
}

void VocoderStartDecode()
{
	//��������� ������� � ����� �������������
	CMX7262_Decode(&g_CMX7262Struct);	
	
	//���������, ��� ������� ����� ��������� ������ (��� �� �� ������ ���������� IDW, ���� � ���� �� �������� ������)
	g_CMX7262Struct.uIRQRequest |= CMX7262_IDW;
}

void VocoderStartEncode()
{
	CMX7262_Encode(&g_CMX7262Struct);
}

#ifndef SMART_PROTOTYPE
void FrontEndSetToTx()
{
	SKY_TR_HIGH();
}

void FrontEndSetToRx()
{
	SKY_TR_LOW();
}
#endif


uint8_t isNeedCheckRSSI()
{
	static uint32_t cntCheckRSSI = 0;
	cntCheckRSSI++;
	cntCheckRSSI %= CHECK_RSSI_PERIOD;

	return(cntCheckRSSI==0);
}

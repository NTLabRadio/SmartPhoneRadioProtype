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


//������ ����������� ��� ��������
uint8_t RadioPackForSend[MAX_RADIOPACK_SIZE];

//������ ��������� �����������
uint8_t RadioPackRcvd[MAX_RADIOPACK_SIZE];

#ifdef DEBUG_CMX7262_CNT_TX_AUDIO_BUF
uint16_t cntCMX7262TxAudioBuf = 0;
#endif



void RadioModuleInit(SPI_HandleTypeDef *hspiCMX7262, SPI_HandleTypeDef *hspiCC1120)
{
	//������������� CMX7262: �������� ������ � ������, ��������� ���������
	CMX7262_Init(&g_CMX7262Struct, hspiCMX7262);

	//������� CMX7262 � ����� Idle
	CMX7262_Idle(&g_CMX7262Struct);	
	
	//������������� ��1120
	CC1120_Init(&g_CC1120Struct, hspiCC1120);
	
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
			pobjRadioModule->RadioModuleState = RADIOMODULE_STATE_TX_WAITING;
			
			//������� �������� � ����� �����������
			CMX7262_Encode(&g_CMX7262Struct);
			
			nLengthDataFromCMX7262 = 0;
			nLengthDataToCMX7262 = 0;
			
			#ifndef SMART_PROTOTYPE
			//TODO SKY_TR_HIGH - ������� �� ��������������� ������ ���������� ProcessPTTState()
			//��� ������ ���������� ������� ����� FrontEndSetTx()
			SKY_TR_HIGH();
			#endif
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
			pobjRadioModule->RadioModuleState = RADIOMODULE_STATE_RX_WAITING;
			
			//������� �������� � ����� �������������
			CMX7262_Decode(&g_CMX7262Struct);
			
			#ifndef TEST_RADIO_IMITATE
			nLengthDataToCMX7262 = 0;
			#endif
			nLengthDataFromCMX7262 = 0;
			
			#ifndef SMART_PROTOTYPE
			//TODO ��� ������ ���������� ������� ����� FrontEndSetRx()
			SKY_TR_LOW();
			#endif
		}
	}
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


// ------------------------------- �������� ������ �������� �������� ������� -------------------------------------
//
//1. ��� ������� �� �������� � ����������� ProcessPTTState():
//	1.1 ������������� ��������� ����������� � RADIOMODULE_STATE_TX_WAITING;
//	1.2 ��������� ������� CMX7262 � ��������� ����������� ������ � ����������;
//	1.3 ��������� FrontEnd � ��������
//
//2. ��������� ������� ��������� � ��������� �����������, �� ��� ������ ������������� ����� 60-�� ����, �������
//������ ���������� ODA. � ����������� ProcessCMX7262State() �����������, ��� �� ����������. ���� ����, ��
//ARM �������� ������ � ���������� �� � ������� �� ��������.
//
//3. � ����������� ProcessRadioState(), ���� ����������� ��������� ����������� � RADIOMODULE_STATE_TX_WAITING, 
//���������, ���������� �� ������ ��������� �� ��������. ���� ������ �������� ����������, �������� ��������� 
//���������� �� RADIOMODULE_STATE_TX_RUNNING
//
//4. � ����������� ProcessRadioState(), ���� ����������� ��������� ����������� RADIOMODULE_STATE_TX_RUNNING:
//	4.1 ���������, ���������� �� � ������� �� �������� ������ ��� ������������ ������ ������ � �������� �� � ������
//	������ ���������� CC1120; ���� ��� ������� �����������, �� ��������� ����� �� ������ ������� �������� �
//	��������� ������ � ���������� ��� � CC1120 ��� �������� � ����;
//	4.2 ���� ��������� ������, �� ����������, ��� ���������� ��������� � �������� ��������� ��������; ������, 
//	���������� � CC1120, ������� �� ������� ��� ��������;
//	4.3 ���� �� ��1120 ������� ���������� � ���, ��� ����� �������, �������, ��� ���������� ��������� � ���������
//	��������� � ����� ���������� ��������� ����� ������

void ProcessRadioState()
{
	switch(pobjRadioModule->GetRadioChanState())
	{
		case RADIOCHAN_STATE_TRANSMIT:

			switch(pobjRadioModule->RadioModuleState)
			{		
				case RADIOMODULE_STATE_TX_WAITING:
					//���� �������� ���������� �������� ������ �� ��������, ������������� � ����� RADIOMODULE_STATE_TX_RUNNING
					if(nLengthDataFromCMX7262 > NUM_CMX7262_BUFFERS_INITACCUM_FOR_TX * CMX7262_CODEC_BUFFER_SIZE)
					{
						pobjRadioModule->RadioModuleState = RADIOMODULE_STATE_TX_RUNNING;
						g_CC1120Struct.TxState = CC1120_TX_STATE_WAIT;						
					}
						
					break;

				case RADIOMODULE_STATE_TX_RUNNING:
					//���� ���� ���������� � ���, ��� ����� �������
					if(g_flCC1120_IRQ_CHECKED)
					{
						//���������� ���� ����������
						g_flCC1120_IRQ_CHECKED = FALSE;

						//����������, ��� ���������� ��������� � ��������� ��������� � ����� ���������� ��������� ����� ������
						g_CC1120Struct.TxState = CC1120_TX_STATE_WAIT;
					}
				
					//���� � ������� �� �������� ���������� ������ ��� ������������ ������ ������ 
					//� ���������� CC1120 ��������, �� �������� ������ � ��1120
					if((nLengthDataFromCMX7262 >= RADIOPACK_VOICEMODE_SIZE) &&
						 (g_CC1120Struct.TxState!=CC1120_TX_STATE_ACTIVE))
					{
						//TODO ���������� ������ ��� �������� ������� � ��������� �������
						//1� ���� - �����. ������������� �����������������
						RadioPackForSend[0] = 0;
						//�� ������� ��������� ������� ������
						memcpy(RadioPackForSend+1,pDataFromCMX7262,RADIOPACK_VOICEMODE_SIZE);
						
						#ifndef TEST_RADIO_IMITATE
						//���������� ������ �� CC1120
						CC1120_TxData(&g_CC1120Struct, RadioPackForSend, RADIOPACK_MODE4800_EXTSIZE);
						#else
						RadioImitator_TxData(RadioPackForSend+1, RADIOPACK_VOICEMODE_SIZE);
						#endif
						
						g_CC1120Struct.TxState = CC1120_TX_STATE_ACTIVE;
						
						memmove(pDataFromCMX7262,pDataFromCMX7262+RADIOPACK_VOICEMODE_SIZE,nLengthDataFromCMX7262-RADIOPACK_VOICEMODE_SIZE);
						nLengthDataFromCMX7262-=RADIOPACK_VOICEMODE_SIZE;
						memset(pDataFromCMX7262+nLengthDataFromCMX7262,0,RADIOPACK_VOICEMODE_SIZE);
					}

					break;
			}

			break;
		case RADIOCHAN_STATE_RECEIVE:
		case RADIOCHAN_STATE_WAIT_RECEIVE:

			switch(pobjRadioModule->RadioModuleState)
			{
				case RADIOMODULE_STATE_RX_WAITING:
					//���������, ��� ������� ����� ��������� ������ (��� �� �� ������ ���������� IDW, ���� � ���� �� �������� ������)
					g_CMX7262Struct.uIRQRequest |= CMX7262_IDW;
				
					//������� Rx FIFO
					CC1120_RxFIFOFlush(g_CC1120Struct.hSPI);
					//������� ���������� � ����� ������
					CC1120_Rx(g_CC1120Struct.hSPI);
				
					pobjRadioModule->RadioModuleState = RADIOMODULE_STATE_RX_RUNNING;
					break;
		
				case RADIOMODULE_STATE_RX_RUNNING:
					//1. ���������, ��� �� ����������, ������������, ��� CC1120 ������ ������
					if(g_flCC1120_IRQ_CHECKED)
					{
						uint16_t nSizeOfRecData = CC1120_RxFIFONumBytes(g_CC1120Struct.hSPI);
					
					//2. ���� ���� ���������� � � RxFIFO ���� ������, �������� �� �� CC1120 					
						if(nSizeOfRecData)
						{
							CC1120_RxData(&g_CC1120Struct,RadioPackRcvd,&nSizeOfRecData);

							//���������� ������ � ������� ��� ��������, ���� � ��� ���� �����
							if((nLengthDataToCMX7262 <= MAX_SIZE_OF_DATA_TO_CMX7262-nSizeOfRecData) && (nSizeOfRecData>RADIOPACK_VOICEMODE_SIZE))
							{
								#ifndef DEBUG_CMX7262_RECPACK_WO_ADDR
								//�������� ������ ��������� ����������� � ������� ��� �������� � ������ ����, ��� ������ ���� - �����
								//�������� ������ ���������� ������. ��������� ���� ������ �� ����� �������, ������� ������� ����������, �� �������
								memcpy(&pDataToCMX7262[nLengthDataToCMX7262],RadioPackRcvd+1,RADIOPACK_VOICEMODE_SIZE);
								#else
								memcpy(&pDataToCMX7262[nLengthDataToCMX7262],RadioPackRcvd,RADIOPACK_VOICEMODE_SIZE);
								#endif
								
								nLengthDataToCMX7262+=RADIOPACK_VOICEMODE_SIZE;
							}
						}
			
					//3. ���������� ���� ���������� 
						g_flCC1120_IRQ_CHECKED = FALSE;
					}

					//4. ���� � ������� ��� �������� ���� ���������� ������, ���������� �� � CMX7262:	(�������� � ProcessCMX7262State())
					break;
			}
			break;
		default:
			break;
	}
}


void CMX7262_TestMode()
{
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
		CMX7262_TxFIFO_Audio(&g_CMX7262Struct,(uint8_t *)&pDataToCMX7262[0]);
		#endif
	#endif
}


void ProcessCMX7262State()
{
	//���� ���� ���������� �� CMX7262
	if(g_flCMX7262_IRQ_CHECKED)
	{
		//������������ ����������: ���������, ��� ����� CMX7262                                                                                       
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

			//TODO �������� pDataToCMX7262 �� ��������� �����
			memmove(pDataToCMX7262,pDataToCMX7262+CMX7262_CODEC_BUFFER_SIZE,nLengthDataToCMX7262-CMX7262_CODEC_BUFFER_SIZE);
			nLengthDataToCMX7262-=CMX7262_CODEC_BUFFER_SIZE;
			
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
				CMX7262_TxFIFO_Audio(&g_CMX7262Struct,(uint8_t *)&pDataToCMX7262[0]);
				#ifdef DEBUG_CMX7262_CNT_TX_AUDIO_BUF
				cntCMX7262TxAudioBuf++;
				#endif
			
				uint16_t nSizeOfTxBuf = sizeof(uint16_t)*CMX7262_AUDIOFRAME_SIZE_SAMPLES;		
				if(nLengthDataToCMX7262>=nSizeOfTxBuf)
				{
					//TODO �������� pDataToCMX7262 �� ��������� �����
					memmove(pDataToCMX7262,pDataToCMX7262+nSizeOfTxBuf,nLengthDataToCMX7262-nSizeOfTxBuf);
					nLengthDataToCMX7262-=nSizeOfTxBuf;
				}
				#ifdef DEBUG_PRINTF_CMX7262_EXCHANGE
				else
					printf("Bufer pDataToCMX7262 is Empty");
				#endif
			#endif
		#endif
	}

}

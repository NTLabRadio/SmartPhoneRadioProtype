#include "SPIMLogic.h"

//������ ��� ��������� �������� ��������� SPIM-���������
SPIMMessage*	pSPIMmsgRcvd;
//������ ��� ������������ ��������� SPIM-��������� ��� ��������
SPIMMessage*	pSPIMmsgToSend;

void SPIMInit()
{
	//������� ������� ��� ��������� � ������������ ��������� SPIM-���������
	pSPIMmsgRcvd = new SPIMMessage;
	//pSPIMmsgToSend  = new SPIMMessage;
}

void SPIMDeInit()
{
	//������� ������� ��� ��������� � ������������ ��������� SPIM-���������
	delete pSPIMmsgRcvd;
	//delete pSPIMmsgToSend;
}

void ProcessDataFromExtDev()
{
	#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_EXTSIGNAL_FROM_UART
	ProcessAudioDataFromUART();
	#endif
	
	pSPIMmsgRcvd->setMsg(pUARTRxSLIPPack,nSizeSLIPPack);
	
	if(pSPIMmsgRcvd->checkCRC())
	{
		#ifdef DEBUG_PRINTF_SPIM_DATA
		printf("Rcvd SPIM Message\n");
		printf("* Address: %d\n", pObjSPIMmsgRcvd->getAddress());
		printf("* Cmd ID: %x\n", pObjSPIMmsgRcvd->getIDCmd());
		printf("* No Msg: %d\n", pObjSPIMmsgRcvd->getNoMsg());
		#endif

		//TODO ���������, �� ���� �� ������� ����� ��� ������� (�� ����������� ������)
		//���� ����� ���������, �� �������� ����������� ������ ������������� ����� ���������
		
		//��������� � ���������� �����, �������������� �������� ����� �������
		FormAndSendAnswerToExtDev(pSPIMmsgRcvd);
		
		//TODO ������� FormAndSendAnswerToExtDev() ������ ������������ ��������� � ��������, 
		//�� �� �������� ��������� �����������. ������� ��������� ������� ��� ��������� ������
		//���������� �����������		
	}
	
	//������� ����� � ������������� ������� SLIP-������
	memset(pUARTRxSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
}


void FormAndSendAnswerToExtDev(SPIMMessage* SPIMmsgRcvd)
{
	SPIMMessage SPIMmsgToSend;

	//��������� ����� ��� ������������ ����������
	FormAnswerToExtDev(SPIMmsgRcvd,&SPIMmsgToSend);
	
	//���������� �������������� �����
	SendDataToExtDev(SPIMmsgToSend.Data, SPIMmsgToSend.Size);
}


void FormAnswerToExtDev(SPIMMessage* SPIMCmdRcvd, SPIMMessage* SPIMBackCmdToSend)
{
	//���������� ID ������ �� ID �������� �������
	uint8_t IDanswer = SPIMBackCmdToSend->IDBackCmd(SPIMCmdRcvd->getIDCmd());
	//������� - ���������� ����������� ����������
	uint8_t address = SPIM_ADDR_EXTDEV;
	//���������� ����� ������ ��������� � ������� �������� �������
	uint8_t noMsg = SPIMCmdRcvd->getNoMsg();
	
	//������� ��������� �� ���� ���������
	uint8_t* pBodyData = SPIMBackCmdToSend->Body;
	uint8_t bodySize = 0;

	FormBodyOfAnswerToExtDev(SPIMCmdRcvd,pBodyData,bodySize);
	
	SPIMBackCmdToSend->setHeader(bodySize,address,noMsg,IDanswer);
	
	SPIMBackCmdToSend->setBody(pBodyData,bodySize);
	
	SPIMBackCmdToSend->setCRC();
}


void FormBodyOfAnswerToExtDev(SPIMMessage* SPIMCmdRcvd, uint8_t* pBodyData, uint8_t& bodySize)
{
	bodySize = 0;
	
	switch(SPIMCmdRcvd->getIDCmd())
	{
		case SPIM_CMD_NOP:
			//���� ������ ������� ��������� (������ ���� ����� 1)
			if(SPIMCmdRcvd->getSizeBody())
			{
				//��������� ���� ������, ��������� �� ������� (� �������������) ����� �������
				bodySize = 1;
				*pBodyData = SPIMCmdRcvd->Body[0];
			}
			break;
		case SPIM_CMD_SET_MODE:
		/*
			//��������� �� ������������ ������ �������
			//TODO �������� ������������� ���������		
			if(SPIMCmdRcvd->getSizeBody()==4)
			{
				//TODO ��������� CMX7262 ������� � ��������� ������� � ��������������� ������
				uint8_t nAudioVolume = SPIMCmdRcvd->Body[2]&0x07;
				CMX7262_AudioOutputGain(&g_CMX7262Struct,nAudioVolume);
			}			
		*/
			ProcessCmdSetMode(SPIMCmdRcvd);
			
			//��������� ���� ������, ������������, ��� ������� ��������� �������
			bodySize = 1;
			*pBodyData = 1;
		
			break;
		case SPIM_CMD_SEND_DATA_FRAME:
			break;
		case SPIM_CMD_TAKE_DATA_FRAME:
			break;	
		case SPIM_CMD_REQ_CURRENT_PARAM:
		{
			//TODO ���� ������ �����������, �� ���������� ����� �������� �� ���������������� �����������

			//��� ��������� ��������� ���� REQ_CURRENT_PARAM ����� ������������ ����������� ��������� ����� cmdReqParam
			//��������� ��� ��������� �� ���������
			SPIMCmdRcvd->cmdReqParam.SetPointerToMessage(SPIMCmdRcvd);
			//� ������������ � �������������� ����������� ���������� ���� ��������� ������
			FormCurrentParamAnswer(SPIMCmdRcvd, pBodyData, bodySize);
			break;
		}
		case SPIM_CMD_SOFT_VER:
			uint16_t noSoftVersion;
			bodySize = sizeof(noSoftVersion);
			noSoftVersion = pobjRadioModule->GetARMSoftVer();
			memcpy(pBodyData,&noSoftVersion,sizeof(noSoftVersion));
			break;
		default:
			break;
	}
}


void ProcessCmdSetMode(SPIMMessage* SPIMCmdRcvd)
{
	//������ ��� �������� ������
	uint8_t opModeCode = SPIMCmdRcvd->Body[0];
	
	//��������� ��� �� ��������� ���������
	uint8_t radioChanType, signalPower, ARMPowerMode;
	SPIMCmdRcvd->ParseOpModeCode(opModeCode, radioChanType, signalPower, ARMPowerMode);
	
	//��������� �������� �������� ���������� �������� ������
	pobjRadioModule->SetRadioChanType(radioChanType);
	pobjRadioModule->SetRadioSignalPower(signalPower);
	//TODO ProcessCmdSetMode() ���������� ���� � ������� FormBodyOfAnswerToExtDev()
	//�������� ��� ������. ����� ������ �� ������� �������� ���������� � �� �������� ��������� ���������,
	//��������� � �������. ����� ProcessCmdSetMode() ������ ���� ��������� �� �������� � ������������ ������
	//�������� ����������
	#ifdef ENABLE_SET_ARM_STANDBY_MODE
	pobjRadioModule->SetARMPowerMode(ARMPowerMode);
	#endif
	
	
	//������ ��� ���������������
	uint8_t audioCode = SPIMCmdRcvd->Body[1];
	//��������� ��� �� ��������� ���������
	uint8_t audioOutLevel, audioInLevel;
	SPIMCmdRcvd->ParseAudioCode(audioCode, audioOutLevel, audioInLevel);

	//��������� �������� �������� ���������������
	pobjRadioModule->SetAudioOutLevel(audioOutLevel);	
	pobjRadioModule->SetAudioInLevel(audioInLevel);
	
	
	//������ ��� ������� ������� ��������
	uint16_t* pTXFreqCode = (uint16_t*)(SPIMCmdRcvd->Body+2);
	uint16_t TXFreqCode = *pTXFreqCode;
	//��������� ��� ������� ������� ��������
	pobjRadioModule->SetTxFreqChan(TXFreqCode);
	
	//������ ��� ������� ������� ������
	uint16_t* pRXFreqCode = (uint16_t*)(SPIMCmdRcvd->Body+4);
	uint16_t RXFreqCode = *pRXFreqCode;
	//��������� ��� ������� ������� ��������
	pobjRadioModule->SetRxFreqChan(RXFreqCode);
}



void FormCurrentParamAnswer(SPIMMessage* SPIMCmdRcvd, uint8_t* pBodyData, uint8_t& bodySize)
{
	//� ������ ����� ������ ���������� ����� ����������, ����� �� ��� � � �������
	pBodyData[0] = SPIMCmdRcvd->cmdReqParam.MaskReqParam();
	bodySize=1;
	
	//���� ������������� ������� ����� �����������
	if(SPIMCmdRcvd->cmdReqParam.isOpModeReq())
	{
		//���������� ��������� �������� ������ �����������
		uint8_t radioChanType = pobjRadioModule->GetRadioChanType();
		uint8_t radioSignalPower = pobjRadioModule->GetRadioSignalPower();
		uint8_t powerMode = pobjRadioModule->GetARMPowerMode();

		//��������� ��� �������� ������
		uint8_t OpModeCode = SPIMCmdRcvd->cmdReqParam.OpModeCode(radioChanType,radioSignalPower,powerMode);
		
		pBodyData[bodySize] = OpModeCode;
		bodySize++;		
	}
	
	//���� ������������� �������������� �����������
	if(SPIMCmdRcvd->cmdReqParam.isAudioReq())
	{
		//����������� �������������� �����������
		uint8_t audioInLevel = pobjRadioModule->GetAudioInLevel();
		uint8_t audioOutLevel = pobjRadioModule->GetAudioOutLevel();
		
		//��������� ��� �������������
		uint8_t AudioCode = SPIMCmdRcvd->cmdReqParam.AudioCode(audioOutLevel,audioInLevel);
		
		pBodyData[bodySize] = AudioCode;
		bodySize++;
	}
	
	//���� ������������� ������� ������ �����������
	if(SPIMCmdRcvd->cmdReqParam.isRxFreqReq())
	{
		pBodyData[bodySize] = pobjRadioModule->GetRxFreqChan();
		bodySize++;
	}

	//���� ������������� ������� �������� �����������
	if(SPIMCmdRcvd->cmdReqParam.isTxFreqReq())
	{
		pBodyData[bodySize] = pobjRadioModule->GetTxFreqChan();
		bodySize++;
	}	
	
	//���� ������������� ������� ������� ������ �������
	if(SPIMCmdRcvd->cmdReqParam.isRSSIReq())
	{
		pBodyData[bodySize] = pobjRadioModule->GetRSSILevel();
		bodySize++;
	}		
	
	//���� ������������� ������� ��������� �����������
	if(SPIMCmdRcvd->cmdReqParam.isChanStateReq())
	{
		pBodyData[bodySize] = pobjRadioModule->GetRadioChanState();
		bodySize++;
	}		
}

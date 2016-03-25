#include "SPIMLogic.h"

//������ ��� ��������� �������� ��������� SPIM-���������
SPIMMessage*	pSPIMmsgRcvd;
//������ ��� ������������ ��������� SPIM-��������� ��� ��������
SPIMMessage*	pSPIMmsgToSend;

extern QueDataFrames QueDataFromExtDev;
extern QueDataFrames QueDataToExtDev;	

#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
uint16_t g_cntDataPckToExtDev = 0;
#endif

#ifdef DEBUG_CHECK_ERRORS_IN_SEND_RADIO_PACKS				
uint16_t g_cntFramesPushToQue = 0;
#endif


void SPIMInit()
{
	//������� ������� ��� ��������� � ������������ ��������� SPIM-���������
	pSPIMmsgRcvd = new SPIMMessage;
}

void SPIMDeInit()
{
	//������� ������� ��� ��������� � ������������ ��������� SPIM-���������
	delete pSPIMmsgRcvd;
}

void ProcessDataFromExtDev()
{
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
			//���������� ������� SET_MODE
			ProcessCmdSetMode(SPIMCmdRcvd);
			
			//��������� ���� ������, ������������, ��� ������� ��������� �������
			//TODO ������������� ���������
			bodySize = 1;
			*pBodyData = SPIM_OP_RESULT_SUCCESS;
			break;
		case SPIM_CMD_SEND_DATA_FRAME:
			uint8_t nAnswer;
		
			//�������� ������ � ������� ��� ��������, ���� � ��� ���� �����
			if(QueDataFromExtDev.PushFrame(SPIMCmdRcvd->Body,SPIMCmdRcvd->getSizeBody()))
			{
				nAnswer = SPIM_OP_RESULT_SUCCESS;		//� ��������, ��� ��� ��, ������ ����� ��������
				#ifdef DEBUG_CHECK_ERRORS_IN_SEND_RADIO_PACKS				
				g_cntFramesPushToQue++;
				#endif
			}
			else
				nAnswer = SPIM_OP_RESULT_FAIL;			//���� ����� ���, ��������, ��� ��� �����, ����� ����������� �������

			bodySize = 1;
			*pBodyData = nAnswer;
			break;
		case SPIM_CMD_TAKE_DATA_FRAME:
			break;	
		case SPIM_CMD_REQ_CURRENT_PARAM:
		{
			//��� ��������� ��������� ���� REQ_CURRENT_PARAM ����� ������������ ����������� ��������� ����� cmdReqParam
			//��������� ��� ��������� �� ���������
			SPIMCmdRcvd->cmdReqParam.SetPointerToMessage(SPIMCmdRcvd);
			//� ������������ � �������������� ����������� ���������� ���� ��������� ������
			FormCurrentParamAnswer(SPIMCmdRcvd, pBodyData, bodySize);
			
			//���� ������ �����������, �� ������������� ����� �������� �� ���������������� �����������
			if(SPIMCmdRcvd->cmdReqParam.isAsynReqParam())
				pobjRadioModule->SetAsyncReqMaskParam(SPIMCmdRcvd->cmdReqParam.MaskReqParam());
			break;
		}
		case SPIM_CMD_SOFT_VER:
			uint16_t noSoftVersion;
			noSoftVersion = pobjRadioModule->GetARMSoftVer();

			bodySize = sizeof(noSoftVersion);
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
	uint8_t radioChanType, signalPower, ARMPowerMode, baudRate;
	SPIMCmdRcvd->ParseOpModeCode(opModeCode, radioChanType, signalPower, ARMPowerMode, baudRate);
	
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
	pobjRadioModule->SetRadioBaudRate(baudRate);
	
	
	//������ ��� ���������������
	uint8_t audioCode = SPIMCmdRcvd->Body[1];
	//��������� ��� �� ��������� ���������
	uint8_t audioOutLevel, audioInLevel;
	SPIMCmdRcvd->ParseAudioCode(audioCode, audioOutLevel, audioInLevel);

	//��������� �������� �������� ���������������
	pobjRadioModule->SetAudioOutLevel(audioOutLevel);	
	pobjRadioModule->SetAudioInLevel(audioInLevel);
	

	//������ ��� ������� ������� ��������
	uint16_t TXFreqCode;
	//TODO ������������� ���������
	memcpy(&TXFreqCode,SPIMCmdRcvd->Body+2,sizeof(TXFreqCode));
	
	//��������� ��� ������� ������� ��������
	pobjRadioModule->SetTxFreqChan(TXFreqCode);
	
	//������ ��� ������� ������� ������
	uint16_t RXFreqCode;
	//TODO ������������� ���������
	memcpy(&RXFreqCode,SPIMCmdRcvd->Body+4,sizeof(RXFreqCode));	
	
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
		uint8_t baudRate = pobjRadioModule->GetRadioBaudRate();

		//��������� ��� �������� ������
		uint8_t OpModeCode = SPIMMessage::CmdReqParam::OpModeCode(radioChanType,radioSignalPower,powerMode,baudRate);
		
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
		uint8_t AudioCode = SPIMMessage::CmdReqParam::AudioCode(audioOutLevel,audioInLevel);
		
		pBodyData[bodySize] = AudioCode;
		bodySize++;
	}
	
	//���� ������������� ������� ������ �����������
	if(SPIMCmdRcvd->cmdReqParam.isRxFreqReq())
	{
		uint16_t nRxFreq = pobjRadioModule->GetRxFreqChan();
		memcpy(&pBodyData[bodySize],&nRxFreq,sizeof(nRxFreq));
		bodySize+=sizeof(nRxFreq);
	}

	//���� ������������� ������� �������� �����������
	if(SPIMCmdRcvd->cmdReqParam.isTxFreqReq())
	{
		uint16_t nTxFreq = pobjRadioModule->GetTxFreqChan();
		memcpy(&pBodyData[bodySize],&nTxFreq,sizeof(nTxFreq));
		bodySize+=sizeof(nTxFreq);
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


void ProcessDataToExtDev()
{
	//���� ���� ������ ��� �������� ����������
	if(!QueDataToExtDev.isEmpty())
	{
		//��������� � ���������� ��������� � ������� ��� �������� ����������
		FormAndSendDataMsgToExtDev();
	}
}


void ProcessAsyncReq()
{
	ProcessDataToExtDev();
	
	uint8_t maskParam = pobjRadioModule->GetMaskOfChangedParams();
	if(maskParam)
	{
		//������� ��������� ��� ��������
		SPIMMessage SPIMmsgToSend;

		//��������� ����� ��� ������������ ����������
		FormAsyncReqParamBack(maskParam,&SPIMmsgToSend);
		
		//���������� �������������� �����
		SendDataToExtDev(SPIMmsgToSend.Data, SPIMmsgToSend.Size);
	}
}
	

void FormAsyncReqParamBack(uint8_t maskReqParam, SPIMMessage* SPIMmsgToSend)
{
	//ID �������
	uint8_t IDcmd = SPIM_CMD_REQ_CURRENT_PARAM_BACK;
	//������� - ���������� ����������� ����������
	uint8_t address = SPIM_ADDR_EXTDEV;
	//���������� ����� ������������ ������ - ������ �������
	uint8_t noMsg = 0;
	
	//������� ��������� �� ���� ���������
	uint8_t* pBodyData = SPIMmsgToSend->Body;
	uint8_t bodySize = 0;

	FormBodyOfAsyncReqParamBack(maskReqParam,pBodyData,bodySize);
	
	SPIMmsgToSend->setHeader(bodySize,address,noMsg,IDcmd);
	
	SPIMmsgToSend->setBody(pBodyData,bodySize);
	
	SPIMmsgToSend->setCRC();
}


void FormBodyOfAsyncReqParamBack(uint8_t maskReqParam, uint8_t* pBodyMsgToSend, uint8_t& bodySizeMsgToSend)
{
	//��������� ����� ��������� �������
	SPIMMessage SPIMCmdRcvd;
		
	//������� ��������� �� ���� ��������� �������
	uint8_t* pBodyCmdRcvd = SPIMCmdRcvd.Body;
	uint8_t bodySizeCmdRcvd = 0;
		
	//�������� ���� ��������� �������
	pBodyCmdRcvd[0] = maskReqParam;
	pBodyCmdRcvd[0] |= SPIMMessage::CmdReqParam::ASYNC_MASK_IN_REQ;
	bodySizeCmdRcvd++;
		
	//��������� ��������� �� ��������� ������������ ���������� ������ cmdReqParam
	SPIMCmdRcvd.cmdReqParam.SetPointerToMessage(&SPIMCmdRcvd);	
		
	//� ������������ � �������������� ����������� ���������� ���� ��������� ������
	FormCurrentParamAnswer(&SPIMCmdRcvd, pBodyMsgToSend, bodySizeMsgToSend);
}


void FormAndSendDataMsgToExtDev()
{
	//���� ���� ������ ��� �������� ����������
	if(!QueDataToExtDev.isEmpty())
	{
		SPIMMessage SPIMmsgToSend;

		//��������� ��������� � ������� ��� �������� ����������
		FormDataMsgToExtDev(&SPIMmsgToSend);

		#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
		g_cntDataPckToExtDev++;
		#endif
		
		//���������� �������������� ���������
		SendDataToExtDev(SPIMmsgToSend.Data, SPIMmsgToSend.Size);
	}
}


void FormDataMsgToExtDev(SPIMMessage* SPIMCmdToSend)
{
	//���� ���� ������ ��� �������� ����������
	if(!QueDataToExtDev.isEmpty())
	{	
		//ID �������
		uint8_t IDcmd = SPIM_CMD_TAKE_DATA_FRAME_BACK;
		//������� - ���������� ����������� ����������
		uint8_t address = SPIM_ADDR_EXTDEV;
		//���������� ����� ������� - ������� (��� �� ����� �� �������)
		uint8_t noMsg = 0;		
		
		//������� ��������� �� ���� ���������
		uint8_t* pBodyData = SPIMCmdToSend->Body;

		uint16_t bodySize = QueDataToExtDev.PopFrame(pBodyData);
		
		SPIMCmdToSend->setHeader(bodySize,address,noMsg,IDcmd);
		
		SPIMCmdToSend->setBody(pBodyData,bodySize);
		
		SPIMCmdToSend->setCRC();
	}
}

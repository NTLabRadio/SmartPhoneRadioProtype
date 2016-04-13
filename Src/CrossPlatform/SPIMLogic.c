#include "SPIMLogic.h"

//TODO В этом модуле достаточно много функций взаимосвязано с большим набором данных
//Надо вынести это в класс

//Объект для обработки принятых сообщений SPIM-протокола
SPIMMessage*	pSPIMmsgRcvd;
//Объект для формирования сообщений SPIM-протокола для отправки
SPIMMessage*	pSPIMmsgToSend;

#ifndef DEBUG_NO_CNT_SPIM_MSG_CONTROL
uint8_t noLastSPIMMsgRcvd;
uint8_t pLastSPIMMsgData[MAX_SIZE_OF_SLIP_PACK_PAYLOAD];
uint8_t nSizeLastSPIMMsg = 0xFF;
#endif

extern QueDataFrames QueDataFromExtDev;
extern QueDataFrames QueDataToExtDev;	
extern QueDataFrames QueRecStatusToExtDev;


#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
uint16_t g_cntDataPckToExtDev = 0;
#endif

#ifdef DEBUG_CHECK_ERRORS_IN_SEND_RADIO_PACKS				
uint16_t g_cntFramesPushToQue = 0;
#endif


void SPIMInit()
{
	//Создаем объекты для обработки и формирования сообщений SPIM-протокола
	pSPIMmsgRcvd = new SPIMMessage;
}

void SPIMDeInit()
{
	//Удаляем объекты для обработки и формирования сообщений SPIM-протокола
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

		//Порядковый номер принятого сообщения
		uint8_t noSPIMMsgRcvd = pSPIMmsgRcvd->getNoMsg();
		
		#ifndef DEBUG_NO_CNT_SPIM_MSG_CONTROL
		//Проверяем, не была ли принята ранее эта команда (по порядковому номеру)
		if(noSPIMMsgRcvd!=noLastSPIMMsgRcvd)
		{
			noLastSPIMMsgRcvd = noSPIMMsgRcvd;
			#endif
			//Формируем и отправляем ответ, подтверждающий успешный прием команды
			FormAndSendAnswerToExtDev(pSPIMmsgRcvd);
		#ifndef DEBUG_NO_CNT_SPIM_MSG_CONTROL
		}
		else
		{
			//Если ранее принимали, то отвечаем сохраненной копией отправленного ранее сообшения
			SendDataToExtDev(pLastSPIMMsgData, nSizeLastSPIMMsg);
		}
		#endif
		
		//TODO Функция FormAndSendAnswerToExtDev() только обрабатывает сообщение и отвечает, 
		//но не изменяет состояние радиомодуля. Сделать отдельную функцию для обработки команд
		//управления радимодулем		
	}
	
	//Очищаем буфер с обработанными данными SLIP-пакета
	memset(pUARTRxSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
}


void FormAndSendAnswerToExtDev(SPIMMessage* SPIMmsgRcvd)
{
	SPIMMessage SPIMmsgToSend;

	//Формируем ответ для управляющего устройства
	FormAnswerToExtDev(SPIMmsgRcvd,&SPIMmsgToSend);
	
	//Отправляем сформированный ответ
	SendDataToExtDev(SPIMmsgToSend.Data, SPIMmsgToSend.Size);
	
	#ifndef DEBUG_NO_CNT_SPIM_MSG_CONTROL
	nSizeLastSPIMMsg = SPIMmsgToSend.Size;
	memcpy(pLastSPIMMsgData,SPIMmsgToSend.Data,nSizeLastSPIMMsg);
	#endif	
}



void FormAnswerToExtDev(SPIMMessage* SPIMCmdRcvd, SPIMMessage* SPIMBackCmdToSend)
{
	//Определяем ID ответа по ID исходной команды
	uint8_t IDanswer = SPIMBackCmdToSend->IDBackCmd(SPIMCmdRcvd->getIDCmd());
	//Адресат - внешнешнее управляющее устройство
	uint8_t address = SPIM_ADDR_EXTDEV;
	//Порядковый номер ответа совпадает с номером исходной команды
	uint8_t noMsg = SPIMCmdRcvd->getNoMsg();
	
	//Получим указатель на тело сообщения
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
			//Если размер команды ненулевой (должен быть равен 1)
			if(SPIMCmdRcvd->getSizeBody())
			{
				//Формируем тело ответа, состоящее из первого (и единственного) байта команды
				bodySize = 1;
				*pBodyData = SPIMCmdRcvd->Body[0];
			}
			break;
		case SPIM_CMD_SET_MODE:
			//Обработаем команду SET_MODE
			ProcessCmdSetMode(SPIMCmdRcvd);
			
			//Формируем тело ответа, указывающего, что команда выполнена успешно
			//TODO Неименованные константы
			bodySize = 1;
			*pBodyData = SPIM_OP_RESULT_SUCCESS;
			break;
		case SPIM_CMD_SEND_DATA_FRAME:
			uint8_t nAnswer;
		
			//Копируем данные в очередь для отправки, если в ней есть место
			if(QueDataFromExtDev.PushFrame(SPIMCmdRcvd->Body,SPIMCmdRcvd->getSizeBody()))
			{
				nAnswer = SPIM_OP_RESULT_SUCCESS;		//и отвечаем, что все ок, данные будут переданы
				#ifdef DEBUG_CHECK_ERRORS_IN_SEND_RADIO_PACKS				
				g_cntFramesPushToQue++;
				#endif
			}
			else
				nAnswer = SPIM_OP_RESULT_FAIL;			//если места нет, отвечаем, что все плохо, стоит попробовать попозже

			bodySize = 1;
			*pBodyData = nAnswer;
			break;
		case SPIM_CMD_TAKE_DATA_FRAME:
			break;	
		case SPIM_CMD_REQ_CURRENT_PARAM:
		{
			//Для обработки сообщения типа REQ_CURRENT_PARAM будем использовать специальный вложенный класс cmdReqParam
			//Передадим ему указатель на сообщение
			SPIMCmdRcvd->cmdReqParam.SetPointerToMessage(SPIMCmdRcvd);
			//В соответствии с запрашиваемыми параметрами сформируем тело сообщения ответа
			FormCurrentParamAnswer(SPIMCmdRcvd, pBodyData, bodySize);
			
			//Если запрос асинхронный, то устанавливаем режим слежения за соответствующими параметрами
			if(SPIMCmdRcvd->cmdReqParam.isAsynReqParam())
				pobjRadioModule->SetAsyncReqMaskParam(SPIMCmdRcvd->cmdReqParam.MaskReqParam());
			break;
		}
		case SPIM_CMD_SOFT_VER:
		{
			uint16_t noSoftVersion = pobjRadioModule->GetARMSoftVer();

			bodySize = sizeof(noSoftVersion);
			memcpy(pBodyData,&noSoftVersion,sizeof(noSoftVersion));
			break;
		}
		case SPIM_CMD_SEND_FIRM_FRAME:
		{
			uint8_t nAnswer = ProcessFirmwareFrame(SPIMCmdRcvd->Body,SPIMCmdRcvd->getSizeBody());

			bodySize = 1;
			*pBodyData = nAnswer;
			break;
		}
		default:
			break;
	}
}


void ProcessCmdSetMode(SPIMMessage* SPIMCmdRcvd)
{
	//Читаем код рабочего режима
	uint8_t opModeCode = SPIMCmdRcvd->Body[0];
	
	//Разбираем код на отдельные параметры
	uint8_t radioChanType, signalPower, ARMPowerMode, baudRate;
	SPIMCmdRcvd->ParseOpModeCode(opModeCode, radioChanType, signalPower, ARMPowerMode, baudRate);
	
	//Применяем принятые значения параметров рабочего режима
	pobjRadioModule->SetRadioChanType(radioChanType);
	pobjRadioModule->SetRadioSignalPower(signalPower);
	//TODO ProcessCmdSetMode() вызывается пока в функции FormBodyOfAnswerToExtDev()
	//Засыпать тут нельзя. Иначе ничего не ответим ведущему устройству и не применим остальные параметры,
	//указанные в команде. Вызов ProcessCmdSetMode() должен быть перенесен за проверку и формирование ответа
	//ведущего устройства
	#ifdef ENABLE_SET_ARM_STANDBY_MODE
	pobjRadioModule->SetARMPowerMode(ARMPowerMode);
	#endif
	pobjRadioModule->SetRadioBaudRate(baudRate);
	
	
	//Читаем код аудиопараметров
	uint8_t audioCode = SPIMCmdRcvd->Body[1];
	//Разбираем код на отдельные параметры
	uint8_t audioOutLevel, audioInLevel;
	SPIMCmdRcvd->ParseAudioCode(audioCode, audioOutLevel, audioInLevel);

	//Применяем принятые значения аудиопараметров
	pobjRadioModule->SetAudioOutLevel(audioOutLevel);	
	pobjRadioModule->SetAudioInLevel(audioInLevel);
	

	//Читаем код рабочей частоты передачи
	uint16_t TXFreqCode;
	//TODO Неименованная константа
	memcpy(&TXFreqCode,SPIMCmdRcvd->Body+2,sizeof(TXFreqCode));
	
	//Применяем код рабочей частоты передачи
	pobjRadioModule->SetTxFreqChan(TXFreqCode);
	
	//Читаем код рабочей частоты приема
	uint16_t RXFreqCode;
	//TODO Неименованная константа
	memcpy(&RXFreqCode,SPIMCmdRcvd->Body+4,sizeof(RXFreqCode));	
	
	//Применяем код рабочей частоты приемачи
	pobjRadioModule->SetRxFreqChan(RXFreqCode);
}



void FormCurrentParamAnswer(SPIMMessage* SPIMCmdRcvd, uint8_t* pBodyData, uint8_t& bodySize)
{
	//В первом байте ответа содержится маска параметров, такая же как и в запросе
	pBodyData[0] = SPIMCmdRcvd->cmdReqParam.MaskReqParam();
	bodySize=1;
	
	//Если запрашивается рабочий режим радиомодуля
	if(SPIMCmdRcvd->cmdReqParam.isOpModeReq())
	{
		//Определяем параметры рабочего режима радиомодуля
		uint8_t radioChanType = pobjRadioModule->GetRadioChanType();
		uint8_t radioSignalPower = pobjRadioModule->GetRadioSignalPower();
		uint8_t powerMode = pobjRadioModule->GetARMPowerMode();
		uint8_t baudRate = pobjRadioModule->GetRadioBaudRate();

		//Формируем код рабочего режима
		uint8_t OpModeCode = SPIMMessage::CmdReqParam::OpModeCode(radioChanType,radioSignalPower,powerMode,baudRate);
		
		pBodyData[bodySize] = OpModeCode;
		bodySize++;		
	}
	
	//Если запрашиваются аудиопараметры радиомодуля
	if(SPIMCmdRcvd->cmdReqParam.isAudioReq())
	{
		//Определеяем аудионастройки радиомодуля
		uint8_t audioInLevel = pobjRadioModule->GetAudioInLevel();
		uint8_t audioOutLevel = pobjRadioModule->GetAudioOutLevel();
		
		//Формируем код аудионастроек
		uint8_t AudioCode = SPIMMessage::CmdReqParam::AudioCode(audioOutLevel,audioInLevel);
		
		pBodyData[bodySize] = AudioCode;
		bodySize++;
	}
	
	//Если запрашивается частота приема радиомодуля
	if(SPIMCmdRcvd->cmdReqParam.isRxFreqReq())
	{
		uint16_t nRxFreq = pobjRadioModule->GetRxFreqChan();
		memcpy(&pBodyData[bodySize],&nRxFreq,sizeof(nRxFreq));
		bodySize+=sizeof(nRxFreq);
	}

	//Если запрашивается частота передачи радиомодуля
	if(SPIMCmdRcvd->cmdReqParam.isTxFreqReq())
	{
		uint16_t nTxFreq = pobjRadioModule->GetTxFreqChan();
		memcpy(&pBodyData[bodySize],&nTxFreq,sizeof(nTxFreq));
		bodySize+=sizeof(nTxFreq);
	}	
	
	//Если запрашивается текущий уровень приема сигнала
	if(SPIMCmdRcvd->cmdReqParam.isRSSIReq())
	{
		pBodyData[bodySize] = pobjRadioModule->GetRSSILevel();
		bodySize++;
	}		
	
	//Если запрашивается текущее состояние радиоканала
	if(SPIMCmdRcvd->cmdReqParam.isChanStateReq())
	{
		pBodyData[bodySize] = pobjRadioModule->GetRadioChanState();
		bodySize++;
	}		
}


void ProcessDataToExtDev()
{
	//Если есть данные для внешнего устройства
	if(!QueDataToExtDev.isEmpty())
	{
		//Формируем и отправляем сообщение с данными для внешнего устройства
		FormAndSendDataMsgToExtDev();
	}
	
	//Если есть статус-данные приемника (RSSI, Link Quality) для внешнего устройства
	if(!QueRecStatusToExtDev.isEmpty())
	{
		//Формируем и отправляем сообщение со статус-данными
		FormAndSendRecStatusToExtDev();
	}
}


void ProcessAsyncReq()
{
	ProcessDataToExtDev();
	
	uint8_t maskParam = pobjRadioModule->GetMaskOfChangedParams();
	if(maskParam)
	{
		//Создаем сообщение для отправки
		SPIMMessage SPIMmsgToSend;

		//Формируем ответ для управляющего устройства
		FormAsyncReqParamBack(maskParam,&SPIMmsgToSend);
		
		//Отправляем сформированный ответ
		SendDataToExtDev(SPIMmsgToSend.Data, SPIMmsgToSend.Size);
	}
}
	

void FormAsyncReqParamBack(uint8_t maskReqParam, SPIMMessage* SPIMmsgToSend)
{
	//ID конмады
	uint8_t IDcmd = SPIM_CMD_REQ_CURRENT_PARAM_BACK;
	//Адресат - внешнешнее управляющее устройство
	uint8_t address = SPIM_ADDR_EXTDEV;
	//Порядковый номер асинхронного ответа - всегда нулевой
	uint8_t noMsg = 0;
	
	//Получим указатель на тело сообщения
	uint8_t* pBodyData = SPIMmsgToSend->Body;
	uint8_t bodySize = 0;

	FormBodyOfAsyncReqParamBack(maskReqParam,pBodyData,bodySize);
	
	SPIMmsgToSend->setHeader(bodySize,address,noMsg,IDcmd);
	
	SPIMmsgToSend->setBody(pBodyData,bodySize);
	
	SPIMmsgToSend->setCRC();
}


void FormBodyOfAsyncReqParamBack(uint8_t maskReqParam, uint8_t* pBodyMsgToSend, uint8_t& bodySizeMsgToSend)
{
	//Имитируем прием сообщения запроса
	SPIMMessage SPIMCmdRcvd;
		
	//Получим указатель на тело сообщения запроса
	uint8_t* pBodyCmdRcvd = SPIMCmdRcvd.Body;
	uint8_t bodySizeCmdRcvd = 0;
		
	//Составим тело сообщения запроса
	pBodyCmdRcvd[0] = maskReqParam;
	pBodyCmdRcvd[0] |= SPIMMessage::CmdReqParam::ASYNC_MASK_IN_REQ;
	bodySizeCmdRcvd++;
		
	//Передадим указатель на сообщение специальному вложенному классу cmdReqParam
	SPIMCmdRcvd.cmdReqParam.SetPointerToMessage(&SPIMCmdRcvd);	
		
	//В соответствии с запрашиваемыми параметрами сформируем тело сообщения ответа
	FormCurrentParamAnswer(&SPIMCmdRcvd, pBodyMsgToSend, bodySizeMsgToSend);
}


void FormAndSendDataMsgToExtDev()
{
	//Если есть данные для внешнего устройства
	if(!QueDataToExtDev.isEmpty())
	{
		SPIMMessage SPIMmsgToSend;

		//Формируем сообщение с данными для внешнего устройства
		FormDataMsgToExtDev(&SPIMmsgToSend);

		#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
		g_cntDataPckToExtDev++;
		#endif
		
		//Отправляем сформированное сообщение
		SendDataToExtDev(SPIMmsgToSend.Data, SPIMmsgToSend.Size);
	}
}


void FormDataMsgToExtDev(SPIMMessage* SPIMCmdToSend)
{
	//Если есть данные для внешнего устройства
	if(!QueDataToExtDev.isEmpty())
	{	
		//ID команды
		uint8_t IDcmd = SPIM_CMD_TAKE_DATA_FRAME_BACK;
		//Адресат - внешнешнее управляющее устройство
		uint8_t address = SPIM_ADDR_EXTDEV;
		//Порядковый номер команды - нулевой (это не ответ на команду)
		uint8_t noMsg = 0;		
		
		//Получим указатель на тело сообщения
		uint8_t* pBodyData = SPIMCmdToSend->Body;

		uint16_t bodySize = QueDataToExtDev.PopFrame(pBodyData);
		
		SPIMCmdToSend->setHeader(bodySize,address,noMsg,IDcmd);
		
		SPIMCmdToSend->setBody(pBodyData,bodySize);
		
		SPIMCmdToSend->setCRC();
	}
}


void FormAndSendRecStatusToExtDev()
{
	//Если есть статус-данные приемника для внешнего устройства
	if(!QueRecStatusToExtDev.isEmpty())
	{
		SPIMMessage SPIMmsgToSend;

		//Формируем сообщение с данными для внешнего устройства
		FormRecStatusMsgToExtDev(&SPIMmsgToSend);

		//Отправляем сформированное сообщение
		SendDataToExtDev(SPIMmsgToSend.Data, SPIMmsgToSend.Size);
	}
}


void FormRecStatusMsgToExtDev(SPIMMessage* SPIMCmdToSend)
{
	//Если есть статус-данные приемника для внешнего устройства
	if(!QueRecStatusToExtDev.isEmpty())
	{	
		//ID команды
		uint8_t IDcmd = SPIM_CMD_RECEIVER_STATUS_BACK;
		//Адресат - внешнешнее управляющее устройство
		uint8_t address = SPIM_ADDR_EXTDEV;
		//Порядковый номер команды - нулевой (это не ответ на команду)
		uint8_t noMsg = 0;		
		
		//Получим указатель на тело сообщения
		uint8_t* pBodyData = SPIMCmdToSend->Body;

		uint16_t bodySize = QueRecStatusToExtDev.PopFrame(pBodyData);
		
		SPIMCmdToSend->setHeader(bodySize,address,noMsg,IDcmd);
		
		SPIMCmdToSend->setBody(pBodyData,bodySize);
		
		SPIMCmdToSend->setCRC();
	}
}


//TODO Реализовать один из двух способов прошивки:
// 1) копировать принятые от терминала данные в основную загрузочную область, в случае нарушения обмена
//	или несовпадения CRC - скопировать прошивку из резервной области флеш в основную;
// 2) копировать принятые от терминала данные в резервную загрузочную область, в случае если данные прошивки
//	успешно переданы и CRC дампа корректна - скопировать прошивку из резервной области флеш в основную;
//TODO Вынести эту функцию, а также все глобальные данные, связанные с ней, в отдельный класс
uint8_t ProcessFirmwareFrame(uint8_t* pBodyFrame, uint8_t nSizeFrame)
//uint8_t ProcessFirmwareFrame(SPIMMessage* SPIMCmd)
{
	uint8_t nRes;
	
	FirmwareFrame FirmFrame(pBodyFrame,nSizeFrame);
	//FirmwareFrame FirmFrame = (FirmwareFrame)SPIMCmd;
	
	uint8_t nTypeOfFrame = FirmFrame.GetTypeOfFrame();
	
	switch(nTypeOfFrame)
	{
		case FirmwareFrame::TYPE_OF_FRAME_HEADER:
		{
			if(StartFirmLoadToFlash())
				nRes = SPIM_OP_RESULT_SUCCESS;

			break;
		}
		case FirmwareFrame::TYPE_OF_FRAME_FIRMDATA:
		{
			if(FirmLoadToFlash(FirmFrame.GetFirmData(), FirmFrame.GetSizeOfFirmData()))
				nRes = SPIM_OP_RESULT_SUCCESS;		//и отвечаем, что все ок, данные будут записаны
			else
				nRes = SPIM_OP_RESULT_FAIL;				//если произошла ошибка, отвечаем, что все плохо, можно попробовать повторно
				
			break;
		}
		case FirmwareFrame::TYPE_OF_FRAME_END:
		{
			if(FinishFirmLoadToFlash(FirmFrame.GetFirmCRC(), FirmFrame.GetFirmCmd()))
				nRes = SPIM_OP_RESULT_SUCCESS;

			break;
		}
		default:
			nRes = SPIM_OP_RESULT_FAIL;
			break;
	}
	
	return(nRes);
}



#include "ProcessStates.h"

//Через этот объект осуществляется доступ ко всем параметрам и функциям радиомодуля
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

// ------------------------------- Описание режима передачи речевого сигнала -------------------------------------
//
//1. При нажатии на тангенту в обработчике ProcessPTTState():
//	1.1 устанавливаем состояние радиомодуля в RADIOMODULE_STATE_TX_WAITING;
//	1.2 переводим вокодер CMX7262 в состояние кодирования данных с аудиовхода;
//	1.3 переводим FrontEnd в передачу
//
//2. Поскольку вокодер переведен в состояние кодирования, то как только накапливается новый 60-мс кадр, вокодер
//выдает прерывание ODA. В обработчике ProcessCMX7262State() проверяется, нет ли прерывания. Если есть, то
//ARM забирает данные и складирует их в очередь на передачу.
//
//3. В обработчике ProcessRadioState(), если установлено состояние радиомодуля в RADIOMODULE_STATE_TX_WAITING, 
//проверяем, достаточно ли данных накоплено от вокодера. Если данных накопили достаточно, изменяем состояние 
//радимодуля на RADIOMODULE_STATE_TX_RUNNING
//
//4. В обработчике ProcessRadioState(), если установлено состояние радиомодуля RADIOMODULE_STATE_TX_RUNNING:
//	4.1 проверяем, достаточно ли в очереди на передачу данных для формирования одного пакета и свободен ли в данный
//	момент передатчик CC1120; если эти условия выполняются, то формируем пакет из данных очереди вокодера и
//	служебных данных и отправляем его в CC1120 для передачи в эфир;
//	4.2 если отправили данные, то запоминаем, что передатчик находится в активном состоянии передачи; данные, 
//	переданные в CC1120, удаляем из очереди для передачи;
//	4.3 если от СС1120 приняли прерывание о том, что пакет передан, считаем, что передатчик находится в свободном
//	состоянии и может передавать следующий пакет данных
//
//
// -------------------------------- Описание режима приема речевого сигнала --------------------------------------
//
//1. При отпускании тангенты в обработчике ProcessPTTState():
//	1.1 устанавливаем состояние радиомодуля RADIOCHAN_STATE_WAIT_RECEIVE (состояние подготовки к приему);
//	1.2 переводим вокодер CMX7262 в состояние декодирования данных и воспроизведения результата на аудиовыход;
//	1.3 переводим FrontEnd в прием
//
//2. В обработчике ProcessRadioState(), если установлено состояние радиомодуля в RADIOMODULE_STATE_RX_WAITING:
//	2.1 программно генерируем прерывание от вокодера о том, что он готов принимать данные (сам он не выдаст 
//	прерывание IDW, пока в него не переданы данные);
//	2.2 переводим трансивер в режим приема;
//	2.3  изменяем состояние радимодуля на RADIOMODULE_STATE_RX_RUNNING (непосредственно прием)
//
//3. В обработчике ProcessRadioState(), если установлено состояние радиомодуля в RADIOMODULE_STATE_RX_RUNNING:
//	3.1 проверяем, нет ли прерывания, указывающего, что трансивер CC1120 принял данные;
//	3.2 если есть прерывание от трансивера и в его буфере RxFIFO есть данные, забираем их из трансивера;
//	3.3 складируем данные в очередь для вокодера, если в ней есть место;
//
//4. В обработчике состояний вокодера ProcessCMX7262State(), проверяем не было ли прерывания CMX7262_IDW, 
// указывающего на то, что вокодер готов декодировать и воспроизводить следующий пакет данных. Если прерывание
// есть, то:
//	4.1 если в очереди для вокодера есть хотя бы один полный пакет (60 мс звука), отправляем их в CMX7262;
//	4.2 удаляем из очереди переданные вокодеру данные



void RadioModuleInit(SPI_HandleTypeDef *hspiCMX7262, SPI_HandleTypeDef *hspiCC1120)
{
	//Инициализация CMX7262: загрузка образа в память, начальная настройка
	CMX7262_Init(&g_CMX7262Struct, hspiCMX7262);

	//Перевод CMX7262 в режим Idle
	CMX7262_Idle(&g_CMX7262Struct);	
	
	//Инициализация СС1120
	CC1120_Init(&g_CC1120Struct, hspiCC1120);
	
	//После того, как все периферийные микросхемы радимодуля настроены, создаем объект
	//для управления общими параметрами радиомодуля
	pobjRadioModule = new RadioModule;	
}


void RadioModuleDeInit()
{
	delete pobjRadioModule;
}


//Функция обработки состояние тангенты
//TODO Не реализован антидребезг
void ProcessPTTState()
{
	//Если нажата тангета
	if(PTT_PRESSED())
	{
		//Проверяем состояние радиоканала
		//Если до сих пор не находимся в передаче
		if( !pobjRadioModule->isTxMode() )
		{
			//Изменяем состояние радиоканала на "передачу"
			pobjRadioModule->SetRadioChanState(RADIOCHAN_STATE_TRANSMIT);
			
			//Изменяем рабочее состояние радиомодуля на "подготовку к передаче"
			pobjRadioModule->RadioModuleState = RADIOMODULE_STATE_TX_WAITING;
			
			//Перевод вокодера в режим кодирования
			CMX7262_Encode(&g_CMX7262Struct);
			
			nLengthDataFromCMX7262 = 0;
			nLengthDataToCMX7262 = 0;
			
			#ifndef SMART_PROTOTYPE
			//TODO SKY_TR_HIGH - функция не соответствующая уровню абстракции ProcessPTTState()
			//Тут должна вызываться функция вроде FrontEndSetTx()
			SKY_TR_HIGH();
			#endif
		}
	}
	else
	{
		//Проверяем состояние радиоканала
		//Если до сих пор не находимся в приеме
		if( !pobjRadioModule->isRxMode() )
		{
			//Изменяем состояние радиоканала на "прием"
			pobjRadioModule->SetRadioChanState(RADIOCHAN_STATE_WAIT_RECEIVE);
			
			//Изменяем рабочее состояние радиомодуля на "подготовку к приему"
			pobjRadioModule->RadioModuleState = RADIOMODULE_STATE_RX_WAITING;
			
			//Перевод вокодера в режим декодирования
			CMX7262_Decode(&g_CMX7262Struct);
			
			#ifndef TEST_RADIO_IMITATE
			nLengthDataToCMX7262 = 0;
			#endif
			nLengthDataFromCMX7262 = 0;
			
			#ifndef SMART_PROTOTYPE
			//TODO Тут должна вызываться функция вроде FrontEndSetRx()
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



void ProcessRadioState()
{
	switch(pobjRadioModule->RadioModuleState)
	{		
		case RADIOMODULE_STATE_TX_WAITING:
			//Если накопили достаточно звуковых данных от вокодера, переключаемся в режим RADIOMODULE_STATE_TX_RUNNING
			if(nLengthDataFromCMX7262 > SIZE_OF_DATA_FROM_CMX7262_INITACCUM_FOR_TX)
			{
				pobjRadioModule->RadioModuleState = RADIOMODULE_STATE_TX_RUNNING;
				//Отмечаем, что передатчик находится в свободном состоянии и может передавать следующий пакет данных
				g_CC1120Struct.TxState = CC1120_TX_STATE_WAIT;						
			}
		break;

		case RADIOMODULE_STATE_TX_RUNNING:
			//Если было прерывание о том, что пакет передан
			if(g_flCC1120_IRQ_CHECKED)
			{
				//Сбрасываем флаг прерывания
				g_flCC1120_IRQ_CHECKED = FALSE;

				//Запоминаем, что передатчик находится в свободном состоянии и может передавать следующий пакет данных
				g_CC1120Struct.TxState = CC1120_TX_STATE_WAIT;
			}
		
			//Если в очереди на передачу достаточно данных для формирования одного пакета 
			//и передатчик CC1120 свободен, то посылаем данные в СС1120
			if((nLengthDataFromCMX7262 >= RADIOPACK_VOICEMODE_SIZE) &&
				 (g_CC1120Struct.TxState!=CC1120_TX_STATE_ACTIVE))
			{
				//Из данных вокодера формируем радиопакет и отправляем его в трансивер
				FormAndSendRadioPack(pDataFromCMX7262,RADIOPACK_VOICEMODE_SIZE);
				
				//Запоминаем, что теперь передатчик находится в активном состоянии передачи
				g_CC1120Struct.TxState = CC1120_TX_STATE_ACTIVE;
				
				//Удаляем переданные данные из очереди данных от вокодера
				RemDataFromFIFOBuf(pDataFromCMX7262, nLengthDataFromCMX7262, RADIOPACK_VOICEMODE_SIZE);
			}
		break;

		case RADIOMODULE_STATE_RX_WAITING:
			//Запускаем процесс декодирования вокодера
			VocoderStartDecode();
		
			//Подготавливаем трансивер к приему данных
			TransceiverStartRx();
		
			pobjRadioModule->RadioModuleState = RADIOMODULE_STATE_RX_RUNNING;
		break;

		case RADIOMODULE_STATE_RX_RUNNING:
			//Проверяем, нет ли прерывания, указывающего, что трансивер принял данные
			if(g_flCC1120_IRQ_CHECKED)
			{
				uint8_t pRadioPayloadData[MAX_RADIOPACK_SIZE];
				uint16_t nSizeOfRadioPayload = 0;
				
				//Забираем данные из буфера RxFIFO трансивера
				ProcessRadioPack(pRadioPayloadData, nSizeOfRadioPayload);

				//Проверям, приняли ли хоть-что нибудь из трансивера
				if(nSizeOfRadioPayload)
				{
					//Копируем полезные данные принятого радиопакета в очередь для вокодера, если в ней есть место
					if(nLengthDataToCMX7262 <= MAX_SIZE_OF_DATA_TO_CMX7262-nSizeOfRadioPayload)
						AddDataToFIFOBuf(pDataToCMX7262, nLengthDataToCMX7262, pRadioPayloadData, nSizeOfRadioPayload);
				}
				//Сбрасываем флаг прерывания 
				g_flCC1120_IRQ_CHECKED = FALSE;
			}	//if(g_flCC1120_IRQ_CHECKED)
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
		
		//Заполняем буфер тональным сигналом 1кГц
		FillBufByToneSignal((int16_t*)pDataToCMX7262,CMX7262_AUDIOFRAME_SIZE_SAMPLES,CMX7262_FREQ_SAMPLING,1000);
		CMX7262_TxFIFO_Audio(&g_CMX7262Struct,(uint8_t *)&pDataToCMX7262[0]);
		#endif
	#endif
}


void ProcessCMX7262State()
{
	//Если было прерывание от CMX7262
	if(g_flCMX7262_IRQ_CHECKED)
	{
		//Обрабатываем прерывание: проверяем, что хочет CMX7262                                                                                       
		CMX7262_IRQ(&g_CMX7262Struct);
		//Сбрасываем флаг, чтобы обнаружить следующее прерывание
		g_flCMX7262_IRQ_CHECKED = FALSE;
	}
	
	//Проверяем, нет ли прерывания о приходе новых данных с вокодера 
	if((g_CMX7262Struct.uIRQRequest & CMX7262_ODA) == CMX7262_ODA)
	{
		//Сбрасываем флаг CMX7262_ODA
		g_CMX7262Struct.uIRQRequest = g_CMX7262Struct.uIRQRequest & ~CMX7262_ODA;
		
		//------- Читаем данные ----------
		#ifndef TEST_CMX7262	
		//Рабочий режим
			//Если данные еще есть куда складировать
			if(nLengthDataFromCMX7262 <= MAX_SIZE_OF_DATA_FROM_CMX7262-CMX7262_CODEC_BUFFER_SIZE)
			{
				//Забираем их с CMX7262 и записываем в очередь
				CMX7262_RxFIFO(&g_CMX7262Struct,(uint8_t *)&pDataFromCMX7262[nLengthDataFromCMX7262]);
				nLengthDataFromCMX7262 += CMX7262_CODEC_BUFFER_SIZE;
			}
			#ifdef DEBUG_PRINTF_CMX7262_EXCHANGE
			else
				printf("Bufer pDataFromCMX7262 is Full");
			#endif
		#else
		//Тестовые режимы
			#ifdef TEST_CMX7262_ENCDEC_AUDIO2CBUS_MODE
			CMX7262_RxFIFO_Audio(&g_CMX7262Struct,(uint8_t *)&pDataFromCMX7262[0]);			
			#endif
		#endif
	}

	//Проверяем, нет ли прерывания о том, что вокодер готов принять следующую порцию данных
	if((g_CMX7262Struct.uIRQRequest & CMX7262_IDW) == CMX7262_IDW)
	{
		//Передаем данные на CMX7262
		
	#ifndef TEST_CMX7262
		//Рабочий режим
		#ifndef TEST_RADIO_IMITATE
		//Если в очереди для вокодера есть хотя бы один полный пакет речевых данных, отправляем их в CMX7262
		if(nLengthDataToCMX7262>=CMX7262_CODEC_BUFFER_SIZE)
		#else
		//В тестовом режиме дополнительно проверяем, не нажата ли тангента
		//При нажатой тангенте ничего не воспроизводим
		if((nLengthDataToCMX7262>=CMX7262_CODEC_BUFFER_SIZE) && (!PTT_PRESSED()))
		#endif
		{
			CMX7262_TxFIFO(&g_CMX7262Struct,(uint8_t *)&pDataToCMX7262[0]);

			//Удаляем из очереди переданные вокодеру данные
			//TODO Заменить pDataToCMX7262 на кольцевой буфер			
			RemDataFromFIFOBuf(pDataToCMX7262, nLengthDataToCMX7262, CMX7262_CODEC_BUFFER_SIZE);			
			
			//Сбрасываем флаг CMX7262_IDW
			g_CMX7262Struct.uIRQRequest = g_CMX7262Struct.uIRQRequest & ~CMX7262_IDW;

		}
		#ifdef DEBUG_PRINTF_CMX7262_EXCHANGE
		else
			printf("Bufer pDataToCMX7262 is Empty");
		#endif
		
	#else
	//Тестовые режимы
			#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_MODE			
				#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_INTERNAL_SIN
				//Заполняем буфер тональным сигналом 1кГц
				FillBufByToneSignal((int16_t*)pDataToCMX7262,CMX7262_AUDIOFRAME_SIZE_SAMPLES,CMX7262_FREQ_SAMPLING,1000);
				#endif
			
				//Передаем буфер на CMX7262
				CMX7262_TxFIFO_Audio(&g_CMX7262Struct,(uint8_t *)&pDataToCMX7262[0]);
				#ifdef DEBUG_CMX7262_CNT_TX_AUDIO_BUF
				cntCMX7262TxAudioBuf++;
				#endif
			
				uint16_t nSizeOfTxBuf = sizeof(uint16_t)*CMX7262_AUDIOFRAME_SIZE_SAMPLES;		
				if(nLengthDataToCMX7262>=nSizeOfTxBuf)
				{
					//Удаляем из очереди переданные вокодеру данные
					//TODO Заменить pDataToCMX7262 на кольцевой буфер
					RemDataFromFIFOBuf(pDataToCMX7262, nLengthDataToCMX7262, nSizeOfTxBuf);
				}
				#ifdef DEBUG_PRINTF_CMX7262_EXCHANGE
				else
					printf("Bufer pDataToCMX7262 is Empty");
				#endif
			#endif
	#endif	//#ifndef TEST_CMX7262 ... #else
	}

}


void TransceiverStartRx()
{
	//Очистка Rx FIFO трансивера
	CC1120_RxFIFOFlush(g_CC1120Struct.hSPI);
	//Перевод трансивера в режим приема
	CC1120_Rx(g_CC1120Struct.hSPI);
}

void VocoderStartDecode()
{
	//Указываем, что вокодер готов принимать данные (сам он не выдаст прерывание IDW, пока в него не переданы данные)
	g_CMX7262Struct.uIRQRequest |= CMX7262_IDW;
}

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


//Данные радиопакета для передачи
uint8_t RadioPackForSend[MAX_RADIOPACK_SIZE];

//Данные принятого радиопакета
uint8_t RadioPackRcvd[MAX_RADIOPACK_SIZE];

#ifdef DEBUG_CMX7262_CNT_TX_AUDIO_BUF
uint16_t cntCMX7262TxAudioBuf = 0;
#endif



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

void ProcessRadioState()
{
	switch(pobjRadioModule->GetRadioChanState())
	{
		case RADIOCHAN_STATE_TRANSMIT:

			switch(pobjRadioModule->RadioModuleState)
			{		
				case RADIOMODULE_STATE_TX_WAITING:
					//Если накопили достаточно звуковых данных от вокодера, переключаемся в режим RADIOMODULE_STATE_TX_RUNNING
					if(nLengthDataFromCMX7262 > NUM_CMX7262_BUFFERS_INITACCUM_FOR_TX * CMX7262_CODEC_BUFFER_SIZE)
					{
						pobjRadioModule->RadioModuleState = RADIOMODULE_STATE_TX_RUNNING;
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
						//TODO Наполнение пакета для передачи вынести в отдельную функцию
						//1й байт - адрес. Устанавливаем широковещательный
						RadioPackForSend[0] = 0;
						//За адресом размещаем речевые данные
						memcpy(RadioPackForSend+1,pDataFromCMX7262,RADIOPACK_VOICEMODE_SIZE);
						
						#ifndef TEST_RADIO_IMITATE
						//Отправляем данные на CC1120
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
					//Указываем, что вокодер готов принимать данные (сам он не выдаст прерывание IDW, пока в него не переданы данные)
					g_CMX7262Struct.uIRQRequest |= CMX7262_IDW;
				
					//Очистка Rx FIFO
					CC1120_RxFIFOFlush(g_CC1120Struct.hSPI);
					//Перевод трансивера в режим приема
					CC1120_Rx(g_CC1120Struct.hSPI);
				
					pobjRadioModule->RadioModuleState = RADIOMODULE_STATE_RX_RUNNING;
					break;
		
				case RADIOMODULE_STATE_RX_RUNNING:
					//1. Проверяем, нет ли прерывания, указывающего, что CC1120 принял данные
					if(g_flCC1120_IRQ_CHECKED)
					{
						uint16_t nSizeOfRecData = CC1120_RxFIFONumBytes(g_CC1120Struct.hSPI);
					
					//2. Если есть прерывание и в RxFIFO есть данные, забираем их из CC1120 					
						if(nSizeOfRecData)
						{
							CC1120_RxData(&g_CC1120Struct,RadioPackRcvd,&nSizeOfRecData);

							//Складируем данные в очередь для вокодера, если в ней есть место
							if((nLengthDataToCMX7262 <= MAX_SIZE_OF_DATA_TO_CMX7262-nSizeOfRecData) && (nSizeOfRecData>RADIOPACK_VOICEMODE_SIZE))
							{
								#ifndef DEBUG_CMX7262_RECPACK_WO_ADDR
								//Копируем данные принятого радиопакета в очередь для вокодера с учетом того, что первый байт - адрес
								//Копируем только вокодерные данные. Несколько байт мусора за этими данными, которые генерит передатчик, не трогаем
								memcpy(&pDataToCMX7262[nLengthDataToCMX7262],RadioPackRcvd+1,RADIOPACK_VOICEMODE_SIZE);
								#else
								memcpy(&pDataToCMX7262[nLengthDataToCMX7262],RadioPackRcvd,RADIOPACK_VOICEMODE_SIZE);
								#endif
								
								nLengthDataToCMX7262+=RADIOPACK_VOICEMODE_SIZE;
							}
						}
			
					//3. Сбрасываем флаг прерывания 
						g_flCC1120_IRQ_CHECKED = FALSE;
					}

					//4. Если в очереди для вокодера есть достаточно данных, отправляем их в CMX7262:	(делается в ProcessCMX7262State())
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

			//TODO Заменить pDataToCMX7262 на кольцевой буфер
			memmove(pDataToCMX7262,pDataToCMX7262+CMX7262_CODEC_BUFFER_SIZE,nLengthDataToCMX7262-CMX7262_CODEC_BUFFER_SIZE);
			nLengthDataToCMX7262-=CMX7262_CODEC_BUFFER_SIZE;
			
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
					//TODO Заменить pDataToCMX7262 на кольцевой буфер
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

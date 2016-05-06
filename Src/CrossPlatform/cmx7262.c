#include "cmx7262.h"

SPI_HandleTypeDef *hspi_CMX7262 = NULL;

uint8_t nCMX7262TxNumBytes = 0;
uint8_t pCMX7262TxData[256];
uint8_t nCMX7262RxNumBytes = 0;
uint8_t pCMX7262RxData[256];


uint8_t CMX7262_CheckModule(SPI_HandleTypeDef *hspi)
{
	hspi_CMX7262 = hspi;	
	
	uint8_t uInterface = 0;
	uint16_t data;
	
	//Передаем команду General Reset
	data = 0;
	CBUS_Write8(1, (uint8_t *)&data, 0, uInterface);
	
	// Wait for a 0.3 second.
	WaitTimeMCS(3e5);
	

	//Передаем команду запроса FIFO output level
	CBUS_Read8(0x4F,(uint8_t*)&data,1,uInterface);

	//Подождем 100 мкс. Этого хватит для передачи по SPI 2 байт с тактовой выше 200 кГц
	WaitTimeMCS(1e2);
	
	
	//Должны принять 3
	if(data != 3)
		return 0;
	
	return 1;
}


//Bootload of CMX7262
uint16_t SDR_Load_FI (cmxFI_TypeDef *pFI, uint8_t uInterface )
{
	uint16_t	*pData;
	uint16_t 	start_code;
	uint16_t 	length;
	uint16_t	data;
	uint16_t	state;

	uint16_t	uTxFIFOCount;
        
	char tempstr[5];        //used to comvers the version from hex to decimal

	uTxFIFOCount = 0;
	data = 0;
	state = 0;
	// Write a general reset to the pDSP6
	CBUS_Write8(1, (uint8_t *)&data, 0, uInterface);
	// Wait for a 0.3 second.
	WaitTimeMCS(3e5);

	// Read the FIFO output level. It should be 3
	CBUS_Read8(0x4F,(uint8_t*)&data,1,uInterface);

	// Read the 3 device check words.
	if (data == 3)
	{
		CBUS_Read16 (0x4D,&data,1,uInterface);
		CBUS_Read16 (0x4D,&data,1,uInterface);
		CBUS_Read16 (0x4D,&data,1,uInterface);
	}
	else
	{
		// If there are no device check words, we return a failure.
		return 0;
	}
	// Initialise the data pointer to the start of the FI image.
	pData = pFI->db1_start_address;
	while (1)
	{
			switch (state)
			{
				case 0:

					// For each of the data blocks set the start codes and length from the definitions in
					// the FI header file. The address of pData is used to identify the current data block.
					if (pData==pFI->db1_start_address)
					{
						start_code = pFI->db1_ptr;
						length = pFI->db1_len;
					}
					else if (pData==pFI->db2_start_address)
					{
						start_code =  pFI->db2_ptr;
						length = pFI->db2_len;
					}
					else if (pData==LOAD_FI_END)
					{
						start_code = pFI->activate_ptr;
						length = pFI->activate_len;
					}
					else
					{
						// If the data pointer setting is not recognised then we have an error condition.
						// So return with an error code.
						return 0;
					}
					state++;
					break;

				case 1:

					CBUS_Write16(0x49, &length,1,uInterface);
					CBUS_Write16(0x49, &start_code,1,uInterface);

					// At this point, if the length is 0 then we have reached the end of the data blocks and the
					// FI should be loaded, so return 1.
					if (length == 0)
					{
						// Check the FI has programmed by checking the programming bit - protect the loop
						// with a 1 second time out. Return 1 for success or 0 for failure..

						ClearHighPrecisionCounter();
						while (ReadHighPrecisionCounter()<1e6)
						{                                                  
							CBUS_Read16 (0x7E,&data,1,uInterface);
							if (data & 0x4000 && (pFI->type==CMX7262FI))
							{
								//read version code
								CBUS_Read16 (0x4D,&data,1,uInterface);  //device type; ignore
								CBUS_Read16 (0x4D,&data,1,uInterface);  //version (in hex) e.g. version 1.0.0.0 will be 0x1000
								sprintf(tempstr,"%X",data);             //convert to string "1000"
								return 1;
							}
							if (data & 0x4000 && (pFI->type==CMX7161FI))
							{
								//read version code
                CBUS_Read16 (0x4D,&data,1,uInterface);
                CBUS_Read16 (0x4D,&data,1,uInterface);
                sprintf(tempstr,"%X",data);
                return 1;
							}
						}
						// Program Failure.
						return 0;
					}
					else
					{
						// Move to next state to programme the target device with
						// the function image.
						state++;
					}

	 				break;

				case 2:
					// There is a 128 deep FIFO on the CBUS interface which allows us to write
					// 128 words before checking the level register (0x4B). Check the level
					// register if the Tx FIFO count is 128. If the level does not reach zero in 1 second
					// we return an error code.
					if (uTxFIFOCount >= 128)
					{
						if (!CBUS_WaitBitClr8(0x4B,0xFF,uInterface))
							// Input FIFO level failure.
							return 0;
						uTxFIFOCount = 0;
					}
					CBUS_Write16(0x49,pData++,1,uInterface);
					length--;
					uTxFIFOCount++;

					// When the length is 0 we will be at the end of the current data block, so we
					// then work out which data block it is and set the data pointer to the next data
					// block or to END. If length is not 0 then there is still more data block to output.

					if (length == 0)
					{
						// If we are at the end of block 1 set the data pointer to the start of block 2.
						// Check if we are at the end of Block 1. If we are ....
						if (pData==(pFI->db1_start_address+pFI->db1_len))
						{
							// Read the level register but doing nothing with it.
							CBUS_Read8 (0x4F,(uint8_t *)&data,1,uInterface);
							// We are at the end of block 1, so read the checksums for the block and
							// return a failure if they are wrong.
							CBUS_Read16 (0x4D,&data,1,uInterface);
							if (data != pFI->db1_chk_hi)
								return 0;
							CBUS_Read16 (0x4D,&data,1,uInterface);
							if(data != pFI->db1_chk_lo)
								return 0;
							pData = pFI->db2_start_address;
						}
						// If we are at the end of block 2 set the data pointer to END.
						else if (pData==(pFI->db2_start_address+pFI->db2_len))
						{
							// Read the level register but doing nothing with it.
							CBUS_Read8 (0x4F,(uint8_t *)&data,1,uInterface);
							// We are at the end of block 2, so read the checksums for the block and
							// return a failure if they are wrong.
							CBUS_Read16 (0x4D,&data,1,uInterface);
							if (data != pFI->db2_chk_hi)
								return 0;
							CBUS_Read16 (0x4D,&data,1,uInterface);
							if(data != pFI->db2_chk_lo)
								return 0;
							pData = LOAD_FI_END;
						}
						state=0;
					}
					else
						state=2;
					break;
			}
		}

}


// This must be the first routine called so that the CBUS interfaced is mapped correctly and
// the FI loaded for the other CMX7262 routines.
uint16_t  CMX7262_Init(CMX7262_TypeDef *pCmx7262, SPI_HandleTypeDef *hspi)
{
	hspi_CMX7262 = hspi;	
	
	pCmx7262->FI = (cmxFI_TypeDef*)START_7262;								// Initialise to the FI load definitions above
	pCmx7262->pFlash = (DMR_Flash_TypeDef *)ADDR_FLASH_PAGE;
	pCmx7262->uInterface = CBUS_INTERFACE_CMX7262;
	pCmx7262->uMode = CMX7262_INIT_MODE;
	pCmx7262->uPacketSize = 0;							// How many bytes we read and write.
	pCmx7262->uIRQRequest = 0;							// Control bits, set by IRQ, MMI, SysTick
	pCmx7262->pDataBuffer = NULL;
	pCmx7262->uIRQ_STATUS_REG = 0;					// Shadow register.
	pCmx7262->uIRQ_ENABLE_REG = 0;					// Shadow register.
	pCmx7262->uOutputGain = 0;
	pCmx7262->uError = 0;										// Clear error field.

	// Check the flag in flash, 0 indicates programmed. Read from saved defaults if the flash
	// has been programmed. Otherwise load the standard macro defaults. Defaults are modified
	// and saved from the MMI.
	if(pCmx7262->pFlash->sFlag == 0)
	{
		pCmx7262->sInputGain = pCmx7262->pFlash->sCmx7262InputGain;
	}
	else
	{
		pCmx7262->sInputGain = CMX7262_INPUT_GAIN_DEFAULT;
	}

	if (!SDR_Load_FI(pCmx7262->FI,pCmx7262->uInterface))
	{
		#ifdef DEBUG_TEST_LOAD_FI_CMX7262
		GPIO_SetBits(GPIOE, TEST_RED_LED_D8);			// Red LED - Failed of Load_FI
		#endif		
		pCmx7262->uError |= CMX7262_FI_LOAD_ERROR;
		return 0;
	}
	// Initialise clocks, analog blocks, input and output gains and reg done select.
	if (!CMX7262_InitHardware (pCmx7262))
	{
		#ifdef DEBUG_TEST_LOAD_FI_CMX7262
		GPIO_SetBits(GPIOE, TEST_GREEN_LED_D7);		// Green LED - Failed of InitHardware
		#endif		
		pCmx7262->uError |= CMX7262_CONFIG_CLK_ERROR;
		return 0;
	}
	
	#if !defined(TEST_CMX7262_ENCDEC_AUDIO2AUDIO_MODE) 	&& \
			!defined(TEST_CMX7262_ENCDEC_AUDIO2CBUS_MODE)		&& \
			!defined(TEST_CMX7262_ENCDEC_CBUS2AUDIO_MODE)
		// Set up packet length, hard decision decoding, FEC disabled.
		CMX7262_Config(pCmx7262, THREE_FRAME | HDD | FEC);
	#else
		CMX7262_Config(pCmx7262, ONE_FRAME);
	#endif
		
	// Clear any bits set in the status register and align the shadow register.
	CMX7262_FlushStatusReg(pCmx7262);
	
	return 1;
}


void CMX7262_Idle(CMX7262_TypeDef *pCmx7262)
{
	// Setting the codec to IDLE when it is already IDLE sometimes causes a problem with the
	// the CMX7262 FI. This has been verified as a known issue and the problem found.

	if (pCmx7262->uMode != CMX7262_IDLE_MODE)
	{
		// Disable CBUS IRQs before putting the codec into into Idle mode.
		pCmx7262->uIRQ_ENABLE_REG &= ~(IRQ+ODA+IDW+UF);
		CBUS_Write16(IRQ_ENABLE_REG,&pCmx7262->uIRQ_ENABLE_REG,1,pCmx7262->uInterface);

		if (!CMX7262_Transcode(pCmx7262,CMX7262_VCTRL_IDLE))
			pCmx7262->uError |= CMX7262_IDLE_ERROR;
		else
			pCmx7262->uMode = CMX7262_IDLE_MODE;
	}
        
	#ifdef SMART_PROTOTYPE
	CMX7262_AudioPA(pCmx7262,DISABLE);      //Audio PA Off
	#endif

	// The codec could have underflowed or set any of the IRQ flags before the Idle mode change took effect,
	// As we confirm the mode change we read the status register and pick up any other flags such as the
	// underflow flag (which is valid for a starved decoder). Therefore after the idle we clear the flags
	// in the shadow status register. As good measure I have added this to all calls to Idle - rrespective
	// of wether the hardware is already idle or not.
	pCmx7262->uIRQ_STATUS_REG &= ~(IRQ+ODA+IDW+OV+UF);

}


void CMX7262_Encode (CMX7262_TypeDef *pCmx7262)
{
	// PCM samples in through audio port and TWELP out through CBUS - in relation to the CMX7262
	CMX7262_Routing(pCmx7262, SRC_AUDIO | DEST_CBUS);
	
	#ifdef DEBUG_CMX7262_NOISE_REDUCTION
	uint16_t uData = 3;	//Noise Supression -14dB
	//uint16_t uData = 1;	//Noise Supression -20dB (most aggressive noise suppression)
	CBUS_Write16(NOISE_REDUCTION,&uData,1,pCmx7262->uInterface);
	#endif
	
	// The encoder is started, there will be a packet delay before  we are requested to service it..
	if (!CMX7262_Transcode (pCmx7262,CMX7262_VCTRL_ENCODE))
		pCmx7262->uError |= CMX7262_ENCODE_ERROR;
	else
	{
		// Set the soft copy of the mode before we enable the IRQ because this is used by the IRQ
		// to set the appropriate request flags.
		pCmx7262->uMode = CMX7262_ENCODE_MODE;
		CMX7262_EnableIRQ(pCmx7262, IRQ+ODA);
	}
}


// We no longer enable the IDW interrupt in this routine because the decoder (Radio Rx) is fed samples
// when they become available from the modem. These are triggered by modem IRQs. A general
// note - To start the decoder we manually write the first TWELP packet without checking the IDW
// flag to start decoding. At the start, the decoder generates interrupts at a higher rate to fill its
// pipeline. Normal is 60mS for three frames per packet. If we do not service the interrupts at the
// higher rate then we could see multiple occurrences of IDW before we service.

void CMX7262_Decode (CMX7262_TypeDef *pCmx7262)
{
	// PCM samples out through audio port and TWELP in through CBUS - in relation to the CMX7262.
	CMX7262_Routing(pCmx7262, SRC_CBUS | DEST_AUDIO);
	#ifdef SMART_PROTOTYPE
	//Audio PA On.
	CMX7262_AudioPA(pCmx7262,ENABLE);
	#endif
	// So this routine is taking a long time to execute. About 10mS.
	if(!CMX7262_Transcode(pCmx7262,CMX7262_VCTRL_DECODE))
		pCmx7262->uError |= CMX7262_DECODE_ERROR;
	else
	{
		pCmx7262->uMode = CMX7262_DECODE_MODE;
		CMX7262_EnableIRQ(pCmx7262, IRQ+UF+IDW);        //enable underflow irq so we know when the call is over.
	}
}


void CMX7262_EncodeDecode_Audio (CMX7262_TypeDef *pCmx7262)
{
	#if defined(TEST_CMX7262_FADE_IN_ENCDEC_MODE) || defined(TEST_CMX7262_NOISE_GATE_IN_ENCDEC_MODE)
	uint16_t uData;
	#endif
	
	// PCM samples in through audio port, encode, decode and out through audio port - in relation to the CMX7262
	CMX7262_Routing(pCmx7262, SRC_AUDIO | DEST_AUDIO);

	#ifdef TEST_CMX7262_FADE_IN_ENCDEC_MODE
	//Fade In: the audio signal will ramp up from zero to the selected full-scale amplitude
	uData = 1;
	CBUS_Write16(SIGNAL_CONTROL,&uData,1,pCmx7262->uInterface);
	#endif
	
	#ifdef SMART_PROTOTYPE
	//Включение звукового усилителя
	CMX7262_AudioPA(pCmx7262,ENABLE);
	#endif
	
	#ifdef TEST_CMX7262_NOISE_GATE_IN_ENCDEC_MODE
	uData = (CMX7262_NOISEGATE_FRAMEDELAY_DEFAULT<<12) | CMX7262_NOISEGATE_THRESHOLD_DEFAULT;
	CBUS_Write16(NOISE_GATE_REG,&uData,1,pCmx7262->uInterface);
	#endif
	
	// The encoder+decoder is started, there will be a delay before we are requested to service it..
	if (!CMX7262_Transcode (pCmx7262,CMX7262_VCTRL_ENCDEC))
		pCmx7262->uError |= CMX7262_ENCODE_ERROR;
	else
	{
		// Set the soft copy of the mode before we enable the IRQ because this is used by the IRQ
		// to set the appropriate request flags.
		pCmx7262->uMode = CMX7262_ENCDEC_MODE;
	}
}

void CMX7262_EncodeDecode_Audio2CBUS (CMX7262_TypeDef *pCmx7262)
{
	// PCM samples in through audio port, encode, decode and out through cbus port - in relation to the CMX7262
	CMX7262_Routing(pCmx7262, SRC_AUDIO | DEST_CBUS);

	#ifdef TEST_CMX7262_NOISE_GATE_IN_ENCDEC_MODE
	uData = (CMX7262_NOISEGATE_FRAMEDELAY_DEFAULT<<12) | CMX7262_NOISEGATE_THRESHOLD_DEFAULT;
	CBUS_Write16(NOISE_GATE_REG,&uData,1,pCmx7262->uInterface);
	#endif
	
	// The encoder+decoder is started, there will be a delay before we are requested to service it..
	if (!CMX7262_Transcode (pCmx7262,CMX7262_VCTRL_ENCDEC))
		pCmx7262->uError |= CMX7262_ENCODE_ERROR;
	else
	{
		// Set the soft copy of the mode before we enable the IRQ because this is used by the IRQ
		// to set the appropriate request flags.
		pCmx7262->uMode = CMX7262_ENCDEC_MODE;

		CMX7262_EnableIRQ(pCmx7262, IRQ+ODA+OV);		// ожидаем прерываний по следующим событиям:
																								// "Output Data Available" (доступны новые выходные данные)
																								// "Overflow" (переполнение выходного буфера ввиду того, что хост не успел вычитать данные)
	}	
}


void CMX7262_EncodeDecode_CBUS2Audio (CMX7262_TypeDef *pCmx7262)
{
	// PCM samples in through cbus port, encode, decode and out through audio port - in relation to the CMX7262
	CMX7262_Routing(pCmx7262, SRC_CBUS | DEST_AUDIO);

	#ifdef TEST_CMX7262_NOISE_GATE_IN_ENCDEC_MODE
	uData = (CMX7262_NOISEGATE_FRAMEDELAY_DEFAULT<<12) | CMX7262_NOISEGATE_THRESHOLD_DEFAULT;
	CBUS_Write16(NOISE_GATE_REG,&uData,1,pCmx7262->uInterface);
	#endif
	
	#ifdef SMART_PROTOTYPE
	//Включение звукового усилителя
	CMX7262_AudioPA(pCmx7262,ENABLE);
	#endif
	
	// The encoder+decoder is started, there will be a delay before we are requested to service it..
	if (!CMX7262_Transcode (pCmx7262,CMX7262_VCTRL_ENCDEC))
		pCmx7262->uError |= CMX7262_ENCODE_ERROR;
	else
	{
		// Set the soft copy of the mode before we enable the IRQ because this is used by the IRQ
		// to set the appropriate request flags.
		pCmx7262->uMode = CMX7262_ENCDEC_MODE;
		CMX7262_EnableIRQ(pCmx7262, IRQ+IDW+UF);    // ожидаем прерываний по следующим событиям:
																								// "Input Data Wanted" (устройство готово принять новые данные)
																								// "Underflow" (устройству не хватает данных для работы в непрерывном режиме)
		
		//enable underflow irq so we know when the call is over.
	}	
}

/* Тестовый режим для создания шаблонных сигналов: передача звукового сигнала микросхеме через CBUS, 
прием результата кодирования через CBUS */
void CMX7262_Encode_CBUS2CBUS (CMX7262_TypeDef *pCmx7262)
{
	// PCM samples in through CBUS and TWELP out through CBUS - in relation to the CMX7262
	CMX7262_Routing(pCmx7262, SRC_CBUS | DEST_CBUS);
	
	// The encoder is started, there will be a packet delay before  we are requested to service it..
	if (!CMX7262_Transcode (pCmx7262,CMX7262_VCTRL_ENCODE))
		pCmx7262->uError |= CMX7262_ENCODE_ERROR;
	else
	{
		// Set the soft copy of the mode before we enable the IRQ because this is used by the IRQ
		// to set the appropriate request flags.
		pCmx7262->uMode = CMX7262_ENCODE_MODE;
		CMX7262_EnableIRQ(pCmx7262, IRQ+ODA);
	}
}


/* Тестовый режим формирования гармонического сигнала на звуковом выходе*/
void CMX7262_Test_AudioOut (CMX7262_TypeDef *pCmx7262)
{
	uint16_t uData;
	uint16_t nFreq = CMX7262_FREQ_SIGNAL_IN_TESTMODE; 	//Частота формируемого колебания, Гц
	
	//Источник сигнала (в режиме теста звукового выхода должен игнорироваться) - CBUS,
	//адресат сигнала - аудиовыход
	CMX7262_Routing(pCmx7262, SRC_CBUS | DEST_AUDIO);
	
	#ifdef SMART_PROTOTYPE
	//Включение звукового усилителя
	CMX7262_AudioPA(pCmx7262,ENABLE);
	#endif

	#ifdef TEST_CMX7262_FADE_IN_TEST_MODE
	//Режим Fade In: сигнал нарастает не мгновенно, амплитуда в течение некоторого времени расет с 0 до полного размаха
	uData = 1;
	CBUS_Write16(SIGNAL_CONTROL,&uData,1,pCmx7262->uInterface);
	#endif
	
	//Код частоты NCO
	//Расчет кода частоты - п.8.1.9 Frequency Control документа D/7262_FI-1.x/4 August 2013 (datasheet)
	uData = floor((nFreq * UINT16_MAX)/CMX7262_FREQ_SAMPLING);	
	CBUS_Write16(FREQ_CONTROL,&uData,1,pCmx7262->uInterface);
	
	// The test mode is started, there will be a delay before we are requested to service it..
	if (!CMX7262_Transcode (pCmx7262,CMX7262_VCTRL_TEST))
		pCmx7262->uError |= CMX7262_ENCODE_ERROR;
	else
	{
		// Set the soft copy of the mode before we enable the IRQ because this is used by the IRQ
		// to set the appropriate request flags.
		pCmx7262->uMode = CMX7262_TEST_MODE;
		CMX7262_EnableIRQ(pCmx7262, IRQ+ODA);	//не знаю, какие прерывания нужны в этом режиме
	}
}


// Selects the codec mode and verifies that the mode change has taken
// effect by waiting on the REG done bit in the status register. The software
// copy of the mode is also updated. If the mode change does not take place
// within 15mS, a 0 is returned for failure, otherwise a 1 for success.
uint16_t CMX7262_Transcode(CMX7262_TypeDef *pCmx7262, uint16_t uMode)
{

	uint16_t uData;

	// Set the codec mode.
	CBUS_Write16(VCTRL_REG,(uint16_t *)&uMode,1,pCmx7262->uInterface);
	
	// Wait until we have confirmation of the mode being set.
	ClearCMX7262TimerCounter();
	while (CMX7262TimerCounter() < CMX7262_TRANSCODE_TIMEOUT)
	{
		CBUS_Read16 (IRQ_STATUS_REG,&uData,1,pCmx7262->uInterface);
		pCmx7262->uIRQ_STATUS_REG |= uData;
		if ((pCmx7262->uIRQ_STATUS_REG & REGDONE) == REGDONE)
		{
			// Clear the REGDONE bit in the shadow regsiter.
			pCmx7262->uIRQ_STATUS_REG &= (uint16_t)(~REGDONE);
			return 1;
		}
	}
	
	// If we get here we have timed out and the mode selection was not successful,
	// so return 0.
	return 0;
}


// Group all the hardware initialisation into one routine.
uint16_t CMX7262_InitHardware(CMX7262_TypeDef *pCmx7262)
{
	uint16_t uData;

	// Configure the clocks
	if(!CMX7262_ConfigClocks(pCmx7262))
		return 0;

	// Configure analog blocks
	CMX7262_AnalogBlocks(pCmx7262);
	// Setup the input and output gains.
	CMX7262_AudioInputGain(pCmx7262,CMX7262_INPUT_GAIN_DEFAULT);
	CMX7262_AudioOutputGain(pCmx7262,CMX7262_OUPUT_GAIN_DEFAULT);
	
	// Enable register write confirmation for VCTRL
	uData = 0x0008;
	CBUS_Write16(REG_DONE_SELECT,&uData,1,pCmx7262->uInterface);

	return 1;
}


// Configure the audio paths for the vocoder. There are several options, input and
// output samples can be routed through CBUS FIFOs or the analog ports. The
// parameter passed to this function configures this.
void CMX7262_Routing(CMX7262_TypeDef  *pCmx7262, uint16_t uData)
{
	CBUS_Write16(AUDIO_ROUTING_REG, (uint16_t *)&uData, 1, pCmx7262->uInterface);
}


// This routine sets the packet size (vocoder output frame  size) which is a variable
// used by the codec read and write routines. It also writes directly to the configuration
// register FEC, hard or soft decision data.
void CMX7262_Config(CMX7262_TypeDef  *pCmx7262, uint16_t uConfig)
{
	if((uConfig & FEC_MASK)==FEC)
	{
		// Calculate the size of a packet transfer based on the size of a frame and the number of
		// frames per packet.
		pCmx7262->uPacketSize = TWELP_FEC_HDD_FRAME_SIZE_BYTES * (FRAME_MASK & uConfig);
	}
	else
	{
		// Calculate the size of a packet transfer based on the size of a frame and the number of
		// frames per packet.
		pCmx7262->uPacketSize = TWELP_HDD_FRAME_SIZE_BYTES * (FRAME_MASK & uConfig);
	}
	CBUS_Write16(VCFG_REG,&uConfig,1,pCmx7262->uInterface);

}


// Configure the CMX7262 clocks. Returns 1 for a success, otherwise 0 for a failure.
uint16_t CMX7262_ConfigClocks(CMX7262_TypeDef  *pCmx7262)
{
	uint16_t data;

	// Flush by reading the status register.
	CBUS_Read16 (IRQ_STATUS_REG,&data,1,pCmx7262->uInterface);

	data = 0x210;	// Select program block 1.2
	CBUS_Write16 (VCTRL_REG,&data,1,pCmx7262->uInterface);

	#ifdef CMX7262_CLOCK_20MHZ
	data = 42;			// Set ref clk Divide in Rx or Tx Mode
	#else
	data = 40;			// Set ref clk Divide in Rx or Tx Mode
	#endif
	CBUS_Write16 (PROG_REG,&data,1,pCmx7262->uInterface);
	if(!CBUS_WaitBitSet16 (IRQ_STATUS_REG, PRG, pCmx7262->uInterface))
	{
		#ifdef USE_LEDS	
		//Индицируем светодиодом ошибку
		LED2_ON();
		#endif
		return 0;		// Program fail.
	}
	
	data = 208;		// Set PLL clk Divide in Rx or Tx Mode
	CBUS_Write16 (PROG_REG,&data,1,pCmx7262->uInterface);
	if(!CBUS_WaitBitSet16 (IRQ_STATUS_REG, PRG, pCmx7262->uInterface))
		return 0;		// Program fail.
	data = 0x41;		// Set Tx/Rx Internal Clock Divide
	CBUS_Write16 (PROG_REG,&data,1,pCmx7262->uInterface);
	if(!CBUS_WaitBitSet16 (IRQ_STATUS_REG, PRG, pCmx7262->uInterface))
		return 0;		// Program fail.
	data = FS_DIV;		// Set I/Q IO Clock Divide
	CBUS_Write16 (PROG_REG,&data,1,pCmx7262->uInterface);
	if(!CBUS_WaitBitSet16 (IRQ_STATUS_REG, PRG, pCmx7262->uInterface))
		return 0;		// Program fail

	data = 0xE10;	// Select program block 1.E
	CBUS_Write16(VCTRL_REG,&data,1,pCmx7262->uInterface);
	data = 0;				// XTAL Driver Enable
	CBUS_Write16(PROG_REG,&data,1,pCmx7262->uInterface);
	if(!CBUS_WaitBitSet16(IRQ_STATUS_REG, PRG, pCmx7262->uInterface))
		return 0;		// Program fail.

	return 1;			// Configuration was a success.

}


void CMX7262_AnalogBlocks(CMX7262_TypeDef *pCmx7262)
{
	uint16_t uData;

	// Power up the appropriate analog blocks - Start
	// DAC Pwr, OP Bias, SPKR1/SPKR2, Enable DrvPwr 1&2
	#ifndef CMX7262_SPKR1_OUT
	uData = 0x086A;
	#else
	uData = 0x088A;
	#endif	
	CBUS_Write16(ANAOUT_CONFIG,&uData,1,pCmx7262->uInterface);
	// Single ended uses ANAIN2, differential uses ANAIN1.
	// J24 Pins 1 to 2, 3 to 4 and 7 to 8, 9 to 10 need shorting.
	//uData = 0x0802; 	// ANAIN1 - Differential input
	// ADC Pwr, ANA Sw, ANAIN2 Pwr
	uData = 0x0A08;
	CBUS_Write16(ANAIN_CONFIG,&uData,1,pCmx7262->uInterface);
	// Power up the appropriate analog blocks - End

}


// Set the audio input gain. Note that the full 16 bit register for input and output is set by the parameter..
void CMX7262_AudioInputGain (CMX7262_TypeDef  *pCmx7262, uint16_t uGain)
{
	uint16_t uData;
	#ifdef DEBUG_CMX7262_MIC_MAXGAIN
	uData = (uint16_t)pCmx7262->sInputGain;
	uData = 7;
	#endif
	// Position the gain to ANAIN2
	uData = uGain << 8;
	CBUS_Write16(ANAIN_GAIN,&uData,1,pCmx7262->uInterface);
}


// Sets speaker output course gain.
// bit15 selects +6dB of gain.
// bits 0-6 (range limited to 0-59) selects attenuation in steps of -0.8dB, 0 = no attenuation,
// 59 *-0.8 = -47.2dB.
void CMX7262_AudioOutputGain (CMX7262_TypeDef  *pCmx7262, uint16_t uGain)
{
	CBUS_Write16(AOG3,&uGain,1,pCmx7262->uInterface);
}


// Read ths status register and align the shadow register to the hardware register.
void CMX7262_FlushStatusReg (CMX7262_TypeDef  *pCmx7262)
{
	uint16_t uData;
	// Flush by reading the status register.
	CBUS_Read16 (IRQ_STATUS_REG,&uData,1,pCmx7262->uInterface);
	// Align the shadow register to the hardware status register.
	CBUS_Read16 (IRQ_STATUS_REG,(uint16_t *)(&pCmx7262->uIRQ_STATUS_REG),1,pCmx7262->uInterface);
}


// This routine allows the interrupts to be set without over writing previous IRQ settings.

void CMX7262_EnableIRQ (CMX7262_TypeDef *pCmx7262, uint16_t uIRQ)
{
	// Set the bits in the interrupt enable shadow register, then write to the cbus
	// register in hardware.
	pCmx7262->uIRQ_ENABLE_REG |= uIRQ;
	CBUS_Write16 (IRQ_ENABLE_REG,&pCmx7262->uIRQ_ENABLE_REG,1,pCmx7262->uInterface);
}


void CMX7262_DisableIRQ (CMX7262_TypeDef *pCmx7262, uint16_t uIRQ)
{
	// Clear the mask bits in the interrupt enable shadow register, then write to the
	// cbus register in hardware.
	pCmx7262->uIRQ_ENABLE_REG &= ~uIRQ;
	CBUS_Write16(IRQ_ENABLE_REG,&pCmx7262->uIRQ_ENABLE_REG,1,pCmx7262->uInterface);
	pCmx7262->uIRQ_STATUS_REG &= ~uIRQ;
}


// This routine will set (disable) or clear (enable) the PA.
// Включение/выключение (shutdown) звукового усилителя для динамика
void CMX7262_AudioPA (CMX7262_TypeDef  *pCmx7262, FunctionalState eState)
{
	if (eState == ENABLE)
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0, GPIO_PIN_SET);
}


// These functions provide a clear interface to the DMR application calling them. They simply read and write
// data to and from the codec FIFOs and extract the interface and packet size from the Cmx7262 data structure.

void CMX7262_RxFIFO (CMX7262_TypeDef  *pCmx7262, uint8_t *pData)
{
	CBUS_Read8(CBUS_VOCODER_OUT,pData,pCmx7262->uPacketSize,pCmx7262->uInterface);
}

void CMX7262_TxFIFO (CMX7262_TypeDef  *pCmx7262, uint8_t *pData)
{
	CBUS_Write8(CBUS_VOCODER_IN,pData,pCmx7262->uPacketSize,pCmx7262->uInterface);
}


void CMX7262_RxFIFO_Audio (CMX7262_TypeDef  *pCmx7262, uint8_t *pData, uint8_t numFrames)
{
	CBUS_Read8(CBUS_AUDIO_OUT,pData,sizeof(uint16_t)*numFrames*CMX7262_AUDIOFRAME_SIZE_SAMPLES,pCmx7262->uInterface);
}

void CMX7262_TxFIFO_Audio (CMX7262_TypeDef  *pCmx7262, uint8_t *pData, uint8_t numFrames)
{
	CBUS_Write8(CBUS_AUDIO_IN,pData,sizeof(uint16_t)*numFrames*CMX7262_AUDIOFRAME_SIZE_SAMPLES,pCmx7262->uInterface);
}


// This is called from the IRQ. The parameter is void so that the function pointer that
// references this can handle different parameter types.
void CMX7262_IRQ (void *pData)
{
	uint16_t uTemp;
	CMX7262_TypeDef * pCmx7262;

	pCmx7262 = (CMX7262_TypeDef*)pData;

	// Read the status register into a shadow register.
	CBUS_Read16 (IRQ_STATUS_REG,&uTemp,1,pCmx7262->uInterface);
	pCmx7262->uIRQ_STATUS_REG |= uTemp;

	// Check the appropriate bits in the shadow register based on the codec mode.
	if(pCmx7262->uMode==CMX7262_ENCODE_MODE || pCmx7262->uMode==CMX7262_ENCDEC_MODE)
	{
		if( (pCmx7262->uIRQ_STATUS_REG & ODA) == ODA)
		{
			// Clear the bit in the shadow register.
			pCmx7262->uIRQ_STATUS_REG &= (uint16_t)(~ODA);
			// The control  flag should not be still set, if it is, there is something wrong so set the
			// error flag. This will be picked up by the SysTick handler.
			// Set the flag in the control field to request appropriate action by routines
			// in PendSV.
			if((pCmx7262->uIRQRequest & CMX7262_ODA) == CMX7262_ODA)
				pCmx7262->uError |= CMX7262_ODA_ERROR;
			pCmx7262->uIRQRequest |= CMX7262_ODA;
		}
	}
	
	// Select IRQ flags to check based on the codec mode.
	if(pCmx7262->uMode==CMX7262_DECODE_MODE || pCmx7262->uMode==CMX7262_ENCDEC_MODE)
	{
		if( (pCmx7262->uIRQ_STATUS_REG & IDW) == IDW)
		{
			 // Clear the bit in the shadow register.
			 pCmx7262->uIRQ_STATUS_REG &= (uint16_t)(~IDW);
			 // The control  flag should not be still set, if it is, there is something wrong so set the
			 // error flag. This will be picked up by the SysTick handler.
			 // Set the flag in the control field to request appropriate action by routines
			 // in PendSV.
			 if((pCmx7262->uIRQRequest & CMX7262_IDW) == CMX7262_IDW)
				 pCmx7262->uError |= CMX7262_IDW_ERROR;
			 pCmx7262->uIRQRequest |= CMX7262_IDW;
		}

		if( (pCmx7262->uIRQ_STATUS_REG & UF) == UF)
		{
			 // Clear the bit in the shadow register.
			 pCmx7262->uIRQ_STATUS_REG &= (uint16_t)(~UF);
			 // The control  flag should not be still set, if it is, there is something wrong so set the
			 // error flag. This will be picked up by the SysTick handler.
			 // Set the flag in the control field to request appropriate action by routines
			 // in PendSV.
			 //if((pCmx7262->uIRQRequest & UF) == UF)
			 //	 pCmx7262->uError |= CMX7262_UF_ERROR;
			 pCmx7262->uIRQRequest |= UF;
		}
                 
	}

	// If there are errors, set a bit in the error field which will be picked up by the SysTick.
	// Catch a data overflow.
	if( (pCmx7262->uIRQ_STATUS_REG & OV) == OV)
	{
		pCmx7262->uIRQ_STATUS_REG &= (uint16_t)(~OV);
		pCmx7262->uError |= CMX7262_OV_ERROR;
	}
	
	// Catch a data underflow.
	//if( (pCmx7262->uIRQ_STATUS_REG & UF) == UF)
	//{
	//	 pCmx7262->uIRQ_STATUS_REG &= (uint16_t)(~UF);
	//	 pCmx7262->uError |= CMX7262_UF_ERROR;
	//}
}


#ifndef SMART_PROTOTYPE
/**
	* @brief	Функция аппаратного сброса CMX7262
	* @note		Функция формирует 50мкс-ный импульс на ноге аппаратного сброса микросхемы
	*					и ожидает в течение 50мкс пока она выйдет в рабочий режим
	*/
void CMX7262_HardwareReset()
{
	CMX7262_RESET; 			// аппаратный сброс
	WaitTimeMCS(5e1); 	// задержка 50 мкс
	
	CMX7262_START; 			// запуск СС1120
	WaitTimeMCS(5e1); 	// ожидание, пока стартанет
}
#endif

//-------------------------------------------- CBUS FUNCTIONS --------------------------------------------------------


__IO uint32_t  CBUSTimeout = CBUS_TIME_OUT;

/**
  * @brief  Writes one byte to the CBUS.
  * @param  pBuffer : pointer to the buffer containing the data to be written to the CBUS.
  * @param  WriteAddr : CBUS's internal address to write to.
  * @param  NumByteToWrite: Number of bytes to write.
  * @retval None
  */
void CBUS_Write16(uint8_t uAddress, uint16_t *data_ptr, uint16_t uAccesses, uint8_t uInterface)
{
	uint16_t uAccessCount;

	// An IRQ routine includes a CBUS access to clear the status register. Therefore
	// we do not want a none IRQ CBUS accesses to be disturbed. Otherwise there
	// could be problems with the chip select lines directing MISO\MOSI to the wrong
	// CBUS interface. Therefore, we disable interrupts for the duration of the CBUS
	// access.

	#ifdef DISABLE_IRQ_FOR_CBUS_OPERATIONS
	__disable_irq();
	#endif

	/* Set chip select Low at the start of the transmission */
	CBUS_SetCSNLow(uInterface);

	/* Send the Address of the register */
	CBUS_SendByte(uAddress);

	for (uAccessCount=0; uAccessCount < uAccesses; uAccessCount++)
	{
		/* Send the data that will be written into the device (MSB First) */
		CBUS_SendByte((*data_ptr & 0xFF00)>>8);
		CBUS_SendByte(*data_ptr & 0x00FF);
		data_ptr++;
	}

	/* Set chip select High at the end of the transmission */
	CBUS_SetCSNHigh(uInterface);

	#ifdef DISABLE_IRQ_FOR_CBUS_OPERATIONS
	__enable_irq();
	#endif

}


void CBUS_Read16(uint8_t uAddress, uint16_t *data_ptr, uint16_t uAccesses, uint8_t uInterface)
{
	uint16_t uAccessCount;

	*data_ptr=0;

	// An IRQ routine includes a CBUS access to clear the status register. Therefore
	// we do not want a none IRQ CBUS accesses to be disturbed. Otherwise there
	// could be problems with the chip select lines directing MISO\MOSI to the wrong
	// CBUS interface. Therefore, we disable interrupts for the duration of the CBUS
	// access.
	#ifdef DISABLE_IRQ_FOR_CBUS_OPERATIONS
	__disable_irq();
	#endif

	/* Set chip select Low at the start of the transmission */

	CBUS_SetCSNLow(uInterface);

	// Setup the CBUS address to read from.
	CBUS_SendByte(uAddress);

	for (uAccessCount = 0; uAccessCount < uAccesses; uAccessCount++)
	{
		*data_ptr=0;
		 // Send dummy byte (0x00) to generate the SPI clock to CBUS (Slave device).
		 // This reads the first byte for 2 byte accesses.
		 *data_ptr = (CBUS_SendByte(CBUS_DUMMY_BYTE) << 8);

		 // Send dummy byte (0x00) to generate the SPI clock to CBUS (Slave device).
		 // This reads the second byte (for 2 byte accesses) or the first byte for single byte
		 // reads.
		 *data_ptr |= CBUS_SendByte(CBUS_DUMMY_BYTE);
		data_ptr++;
	}

	 /* Set chip select High at the end of the transmission */
	CBUS_SetCSNHigh(uInterface);

	#ifdef DISABLE_IRQ_FOR_CBUS_OPERATIONS
	__enable_irq();
	#endif

}


// Supports byte wide CBUS streaming and efficient packing of bytes.
void CBUS_Write8(uint8_t uAddress, uint8_t *data_ptr, uint16_t uAccesses, uint8_t uInterface)
{
	uint16_t uAccessCount;

	// An IRQ routine includes a CBUS access to clear the status register. Therefore
	// we do not want a none IRQ CBUS accesses to be disturbed. Otherwise there
	// could be problems with the chip select lines directing MISO\MOSI to the wrong
	// CBUS interface. Therefore, we disable interrupts for the duration of the CBUS
	// access.
	#ifdef DISABLE_IRQ_FOR_CBUS_OPERATIONS
	__disable_irq();
	#endif

	/* Set chip select Low at the start of the transmission */
	CBUS_SetCSNLow(uInterface);

	/* Send the Address of the register */
	CBUS_SendByte(uAddress);

	for (uAccessCount=0; uAccessCount < uAccesses; uAccessCount++)
	{
		/* Send the data that will be written into the device  */
		CBUS_SendByte(*data_ptr);
		data_ptr++;
	}

	/* Set chip select High at the end of the transmission */
	CBUS_SetCSNHigh(uInterface);

	#ifdef DISABLE_IRQ_FOR_CBUS_OPERATIONS
	__enable_irq();
	#endif
}


// Supports byte wide CBUS streaming and efficient packing of bytes.
void CBUS_Read8(uint8_t uAddress, uint8_t *data_ptr, uint16_t uAccesses, uint8_t uInterface)
{
	uint16_t uAccessCount;

	*data_ptr=0;

	// An IRQ routine includes a CBUS access to clear the status register. Therefore
	// we do not want a none IRQ CBUS accesses to be disturbed. Otherwise there
	// could be problems with the chip select lines directing MISO\MOSI to the wrong
	// CBUS interface. Therefore, we disable interrupts for the duration of the CBUS
	// access.
	#ifdef DISABLE_IRQ_FOR_CBUS_OPERATIONS
	__disable_irq();
	#endif

	/* Set chip select Low at the start of the transmission */
	CBUS_SetCSNLow(uInterface);
	
	// Setup the CBUS address to read from.
	CBUS_SendByte(uAddress);

	for (uAccessCount = 0; uAccessCount < uAccesses; uAccessCount++)
	{
		// Send dummy byte (0x00) to generate the SPI clock to CBUS (Slave device).
		// This reads the first byte for single byte  reads.
		*data_ptr = CBUS_SendByte(CBUS_DUMMY_BYTE);
		data_ptr++;
	}

	 /* Set chip select High at the end of the transmission */
	CBUS_SetCSNHigh(uInterface);

	#ifdef DISABLE_IRQ_FOR_CBUS_OPERATIONS
	__enable_irq();
	#endif	
}


// Continually read a CBUS register until the bits in the mask are set or the 1 second
// timeout expires. Returns 1 when the mask is matched. Otherwise 0 for a timeout.
uint16_t	CBUS_WaitBitSet16 (uint8_t Address, uint16_t uMask, uint8_t uInterface)
{
	uint16_t data;

	ClearHighPrecisionCounter();
	while (ReadHighPrecisionCounter()<1e6)
	{
		data = 0;
		CBUS_Read16(Address,&data,1,uInterface);
		if (uMask == (data & uMask))
		{
			return data;
		}
	}
	return 0;
}


// Continually read a CBUS register until the bits in the mask are clear or the 1 second
// timeout expires. Returns 1 when the bits in the mask are clear. Otherwise 0 for a timeout.
uint16_t	CBUS_WaitBitClr8(uint8_t Address, uint8_t uMask, uint8_t uInterface)
{
	uint8_t data;

	ClearHighPrecisionCounter();
	// Timeout set at 1 second.
	while (ReadHighPrecisionCounter()<1e6)
	{
		data = 0;
		CBUS_Read8(Address,&data,1,uInterface);
		if ((data & ~uMask)==0)
		{
			return 1;
		}
	}
	return 0;
}

/**
  * @brief  Sends a Byte through the SPI interface and return the Byte received
  *         from the SPI bus.
  * @param  Byte : Byte send.
  * @retval The received byte value
  */
uint8_t CBUS_SendByte(uint8_t byte)
{
	uint8_t nByteForRX;
	
  SPI_TransmitRecieveByte(hspi_CMX7262, byte, &nByteForRX);
	
	return nByteForRX;
}

/*
 * Set the CSN bit low based on the mask.
 */
void CBUS_SetCSNLow(uint8_t uMask)
{
	if(uMask==CBUS_INTERFACE_CMX7262)
		CMX7262_CSN_LOW();
}


/*
 * Set the CSN bit high based on the mask.
 */
void CBUS_SetCSNHigh(uint8_t uMask)
{
	if(uMask==CBUS_INTERFACE_CMX7262)	
		CMX7262_CSN_HIGH();
}

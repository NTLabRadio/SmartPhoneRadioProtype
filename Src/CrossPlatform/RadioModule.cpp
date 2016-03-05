#include "RadioModule.h"

extern CMX7262_TypeDef  g_CMX7262Struct;
extern CC1120_TypeDef  g_CC1120Struct;



RadioModule::RadioModule()
{
	RadioModuleState = RADIOMODULE_STATE_IDLE;
	
	//Тип радиоканала
	RadioChanType = RADIOCHAN_TYPE_IDLE;
	//Режим мощности ВЧ сигнала
	RadioSignalPower = RADIO_SIGNALPOWER_LOW;
	//Режим энергосбережения
	ARMPowerMode = ARM_POWERMODE_NORMAL;

	//Рабочие частоты передачи/приема
	NoTxFreqChan = DEFAULT_TX_FREQ_CHAN;
	NoRxFreqChan = DEFAULT_RX_FREQ_CHAN;
	ApplyRadioFreq();

	//Настройки аудио
	AudioInLevel = DEFAULT_AUDIO_IN_GAIN;
	AudioOutLevel = DEFAULT_AUDIO_OUT_GAIN;
	ApplyAudioSettings();
	
	//Уровень приема
	RSSILevel = 0;
	
	//Текущее состояние радиоканала
	RadioChanState = RADIOCHAN_STATE_IDLE;
	
	RadioAddress = DEFAULT_RADIO_ADDRESS;
}


uint8_t RadioModule::SetRadioChanType(uint8_t chanType)
{
	if(chanType<NUM_RADIOCHAN_TYPES)
	{
		RadioChanType	=	(en_RadioChanTypes)chanType;
		return(0);
	}
	else
		return(1);
}

uint8_t RadioModule::GetRadioChanType()
{
	return(RadioChanType);
}

uint8_t RadioModule::SetRadioSignalPower(uint8_t signalPower)
{
	if(signalPower<NUM_RADIO_SIGNALPOWER_MODES)
	{
		RadioSignalPower = (en_RadioSignalPowers)signalPower;
		return(0);
	}
	else
		return(1);		
}

uint8_t RadioModule::GetRadioSignalPower()
{
	return(RadioSignalPower);
}

uint8_t RadioModule::SetARMPowerMode(uint8_t powerMode)
{
	if(powerMode<NUM_ARM_POWERMODES)
	{
		ARMPowerMode = (en_ARMPowerModes)powerMode;
		return(0);
	}
	else
		return(1);		
}

uint8_t RadioModule::GetARMPowerMode()
{
	return(ARMPowerMode);
}

uint8_t RadioModule::SetTxFreqChan(uint16_t noFreqChan)
{
	NoTxFreqChan = noFreqChan;
	ApplyRadioFreq();
	
	return(0);
}

uint16_t RadioModule::GetTxFreqChan()
{
	return(NoTxFreqChan);
}

uint8_t RadioModule::SetRxFreqChan(uint16_t noFreqChan)
{
	NoRxFreqChan = noFreqChan;
	ApplyRadioFreq();
	
	return(0);	
}

uint16_t RadioModule::GetRxFreqChan()
{
	return(NoRxFreqChan);
}

uint8_t RadioModule::SetAudioInLevel(uint8_t audioLevel)
{
	AudioInLevel = audioLevel;
	ApplyAudioSettings();
	
	return(0);	
}

uint8_t RadioModule::GetAudioInLevel()
{
	return(AudioInLevel);
}

uint8_t RadioModule::SetAudioOutLevel(uint8_t audioLevel)
{
	AudioOutLevel = audioLevel;
	ApplyAudioSettings();
	
	return(0);	
}

uint8_t RadioModule::GetAudioOutLevel()
{
	return(AudioOutLevel);
}

uint8_t RadioModule::GetRSSILevel()
{
	return(RSSILevel);
}

uint8_t RadioModule::GetRadioChanState()
{
	return(RadioChanState);
}

uint8_t RadioModule::SetRadioChanState(uint8_t radioChanState)
{
	RadioChanState = (en_RadioChanStates)radioChanState;
	
	//Частоты передачи и приема могут отличаться. При смене режима устанавливаем новую рабочую частоту
	ApplyRadioFreq();
	
	return(0);
}

uint8_t RadioModule::GetRadioAddress()
{
	return(RadioAddress);
}

uint8_t RadioModule::SetRadioAddress(uint8_t address)
{
	RadioAddress = address;
	
	return(0);
}

uint8_t RadioModule::isTxMode()
{
	return(GetRadioChanState() == RADIOCHAN_STATE_TRANSMIT);	
}

uint8_t RadioModule::isRxMode()
{
	return((GetRadioChanState() == RADIOCHAN_STATE_WAIT_RECEIVE) ||
				 (GetRadioChanState() == RADIOCHAN_STATE_RECEIVE) );
}


uint16_t RadioModule::GetARMSoftVer()
{
	return(ARM_SOFT_VER);
}


void RadioModule::ApplyAudioSettings()
{
	uint16_t CMX7262AudioGainIn=0, CMX7262AudioGainOut=0;

	//По 3-битному значению кода определяем значение соответствуюшего регистра CMX7262
	CMX7262AudioGainOut = AudioOutGainCodeToCMX7262ValueReg(AudioOutLevel);
	
	//Записываем вычисленные значения регистров в CMX7262
	SetCMX7262AudioGains(CMX7262AudioGainIn, CMX7262AudioGainOut);
}


void RadioModule::ApplyRadioFreq()
{
	uint32_t lFreqValueHz=0;
	bool flFreqChanged = FALSE;

	if(RadioChanState==RADIOCHAN_STATE_TRANSMIT)
	{
		if(NoCurFreqChan!=NoTxFreqChan)
		{
			NoCurFreqChan = NoTxFreqChan;
			flFreqChanged = TRUE;
		}
	}
	else
	{
		if(NoCurFreqChan!=NoRxFreqChan)
		{
			NoCurFreqChan = NoRxFreqChan;
			flFreqChanged = TRUE;
		}
	}
	
	if(flFreqChanged)
	{
		//По значению кода частоты определяем значение частоты в Гц		
		lFreqValueHz = FreqCodeToHz(NoCurFreqChan);
		
		//Записываем значение частоты в СС1120
		SetCC1120Freq(lFreqValueHz);
	}
}

uint32_t RadioModule::FreqCodeToHz(uint16_t nFreqCode)
{
	return(RADIO_BASE_FREQ + RADIO_FREQCHAN_STEP*nFreqCode);
}

void RadioModule::SetCC1120Freq(uint32_t lFreq)
{	
	uint32_t setFreqCode = 0;
	uint8_t	FreqCode[3];
	
	setFreqCode = (65536 * (lFreq / 10000)/(F_XOSC/10000))* LO_DIVIDER;
	
	FreqCode[0] = (uint8_t)((setFreqCode & 0x00FF0000) >> 16);
  FreqCode[1] =	(uint8_t)((setFreqCode & 0x0000FF00) >> 8);
	FreqCode[2] = (uint8_t)(setFreqCode  & 0x000000FF);
	
	CC1120_FreqWrite(g_CC1120Struct.hSPI,FreqCode);
}


uint16_t RadioModule::AudioOutGainCodeToCMX7262ValueReg(uint8_t audioOutLevel)
{
	uint16_t regValue;
	
	if(audioOutLevel!=MAX_AUDIO_OUT_GAIN_CODE)
		regValue = CMX7262_MAX_AUDIO_OUT_GAIN_VALUE - CMX7262_STEP_AUDIO_OUT_GAIN_VALUE*audioOutLevel;
	else
		regValue = CMX7262_AUDIO_OUT_EXTRAGAIN_VALUE;
	
	return(regValue);
}

void RadioModule::SetCMX7262AudioGains(uint16_t CMX7262AudioGainIn, uint16_t CMX7262AudioGainOut)
{
	CMX7262_AudioOutputGain(&g_CMX7262Struct, CMX7262AudioGainOut);
	CMX7262_AudioInputGain(&g_CMX7262Struct);
}


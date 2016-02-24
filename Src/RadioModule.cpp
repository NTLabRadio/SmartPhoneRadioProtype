#include "RadioModule.h"


RadioModule::RadioModule()
{
	//Тип радиоканала
	RadioChanType = RADIOCHAN_TYPE_IDLE;
	//Режим мощности ВЧ сигнала
	RadioSignalPower = RADIO_SIGNALPOWER_LOW;
	//Режим энергосбережения
	ARMPowerMode = ARM_POWERMODE_NORMAL;

	//Рабочие частоты передачи/приема
	TxRadioFreq = 0;
	RxRadioFreq = 0;

	//Настройки аудио
	AudioInLevel = 3;
	AudioOutLevel = 3;
	
	//Уровень приема
	RSSILevel = 0;
	
	//Текущее состояние радиоканала
	RadioChanState = RADIOCHAN_STATE_IDLE;
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
	TxRadioFreq = noFreqChan;
	
	return(0);
}

uint16_t RadioModule::GetTxFreqChan()
{
	return(TxRadioFreq);
}

uint8_t RadioModule::SetRxFreqChan(uint16_t noFreqChan)
{
	RxRadioFreq = noFreqChan;
	
	return(0);	
}

uint16_t RadioModule::GetRxFreqChan()
{
	return(RxRadioFreq);
}

uint8_t RadioModule::SetAudioInLevel(uint8_t audioLevel)
{
	AudioInLevel = audioLevel;
	
	return(0);	
}

uint8_t RadioModule::GetAudioInLevel()
{
	return(AudioInLevel);
}

uint8_t RadioModule::SetAudioOutLevel(uint8_t audioLevel)
{
	AudioOutLevel = audioLevel;
	
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


uint16_t RadioModule::GetARMSoftVer()
{
	return(ARM_SOFT_VER);
}


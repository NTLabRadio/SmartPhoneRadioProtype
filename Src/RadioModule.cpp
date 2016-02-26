#include "RadioModule.h"

extern CMX7262_TypeDef  g_CMX7262Struct;



RadioModule::RadioModule()
{
	RadioModuleState = RADIOMODULE_STATE_IDLE;
	
	//��� �����������
	RadioChanType = RADIOCHAN_TYPE_IDLE;
	//����� �������� �� �������
	RadioSignalPower = RADIO_SIGNALPOWER_LOW;
	//����� ����������������
	ARMPowerMode = ARM_POWERMODE_NORMAL;

	//������� ������� ��������/������
	TxRadioFreq = 0;
	RxRadioFreq = 0;

	//��������� �����
	AudioInLevel = DEFAULT_AUDIO_IN_GAIN;
	AudioOutLevel = DEFAULT_AUDIO_OUT_GAIN;
	ApplyAudioSettings();
	
	//������� ������
	RSSILevel = 0;
	
	//������� ��������� �����������
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
	
	return(0);
}


uint16_t RadioModule::GetARMSoftVer()
{
	return(ARM_SOFT_VER);
}


void RadioModule::ApplyAudioSettings()
{
	uint16_t CMX7262AudioGainIn=0, CMX7262AudioGainOut=0;

	//�� 3-������� �������� ���� ���������� �������� ���������������� �������� CMX7262
	CMX7262AudioGainOut = AudioOutGainCodeToCMX7262ValueReg(AudioOutLevel);
	
	//���������� ����������� �������� ��������� � CMX7262
	SetCMX7262AudioGains(CMX7262AudioGainIn, CMX7262AudioGainOut);
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


#include "RadioModule.h"

extern CMX7262_TypeDef  g_CMX7262Struct;
extern CC1120_TypeDef  g_CC1120Struct;



RadioModule::RadioModule()
{
	SetRadioModuleState(RADIOMODULE_STATE_IDLE);
	
	//��� �����������
	//�� ��������� ������������� ����� ��������� ������, ��������� ���� ����� �������� �������������
	//(� ���� ������ ������ ����� ��������������)	
	SetRadioChanType(RADIOCHAN_TYPE_VOICE);
	
	//����� �������� �� �������
	#ifndef DEBUG_SET_SIGNALPOWER_HIGH_AS_DEFAULT
	SetRadioSignalPower(RADIO_SIGNALPOWER_LOW);
	#else
	SetRadioSignalPower(RADIO_SIGNALPOWER_HIGH);
	#endif
	
	//����� ����������������
	SetARMPowerMode(ARM_POWERMODE_NORMAL);
	
	#ifndef TEST_CMX7262
	//��������� �������� �������� ������
	SetRadioBaudRate(RADIO_BAUD_RATE_4800);
	
	//������� ������� ��������/������
	SetTxFreqChan(DEFAULT_TX_FREQ_CHAN);
	SetRxFreqChan(DEFAULT_RX_FREQ_CHAN);
	NoCurFreqChan = DEFAULT_RX_FREQ_CHAN;

	//��������� �����
	SetAudioInLevel(DEFAULT_AUDIO_IN_GAIN);
	SetAudioOutLevel(DEFAULT_AUDIO_OUT_GAIN);
	#endif
	
	//������� ������
	RSSILevel = 0;
	
	//������� ��������� �����������
	SetRadioChanState(RADIOCHAN_STATE_IDLE);
	
	//�������� ����� ��������
	SetTestMode(false);
	//������� �� ��������� - "���"
	SetTestPattern(SYMBOL_PATTERN_TONE);
	
	//����������� ����������� �����
	SetRadioAddress(DEFAULT_RADIO_ADDRESS);
	
	SetAsyncReqMaskParam(0);
}


uint8_t RadioModule::SetRadioModuleState(uint8_t state)
{
	if(state<NUM_RADIOMODULE_STATES)
	{
		RadioModuleState = (en_RadioModuleStates)state;
		return(0);
	}
	else
		return(1);	
}

uint8_t RadioModule::GetRadioModuleState()
{
	return(RadioModuleState);
}


void RadioModule::SwitchToIdleState()
{
	SetRadioModuleState(RADIOMODULE_STATE_IDLE);
	
	SetRadioChanState(RADIOCHAN_STATE_IDLE);
	
	//�� ��������� ������������� ����� ��������� ������, ��������� ���� ����� �������� �������������
	//(� ���� ������ ������ ����� ��������������)
	RadioChanType = RADIOCHAN_TYPE_VOICE;	
}


uint8_t RadioModule::SetTestMode(uint8_t isTestMode)
{
	if(isTestMode)
		IsRadioTestMode = true;
	else
		IsRadioTestMode	=	false;
	
	return(1);
}

uint8_t RadioModule::IsTestMode()
{
	return(IsRadioTestMode);
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

uint8_t RadioModule::SetTestPattern(uint8_t noPattern)
{
	if(noPattern<NUM_OF_SYMBOL_PATTERNS)
	{
		NoRadioTestPattern = (en_SymbolPatterns)noPattern;
		return(0);
	}
	else
		return(1);
}

uint8_t RadioModule::GetTestPattern()
{
	return(NoRadioTestPattern);
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
		ApplyRadioSignalPower();
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

uint8_t RadioModule::SetRadioBaudRate(uint8_t baudRate)
{
	if(baudRate<NUM_RADIO_BAUD_RATES)
	{
		RadioBaudRate = (en_RadioBaudRates)baudRate;
		ApplyRadioConfig();

		return(0);
	}
	else
		return(1);	
}

uint8_t RadioModule::GetRadioBaudRate()
{
	return(RadioBaudRate);
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
	
	//���� �� ��� ��� � ����� ���������� ���������� ��� �������� ���������
	if(! (MaskOfChangedParams & SPIMMessage::CmdReqParam::CHANSTATE_MASK_IN_REQ))
	{
		//��������� ��� � �����
		MaskOfChangedParams |= SPIMMessage::CmdReqParam::CHANSTATE_MASK_IN_REQ;
		//� ����������� ����� ������������� ����������
		MaskOfChangedParams &= AsyncReqMaskParam;
	}
	
	//������� �������� � ������ ����� ����������. ��� ����� ������ ������������� ����� ������� �������
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

	//�� 3-������� �������� ���� ���������� �������� ���������������� �������� CMX7262
	CMX7262AudioGainOut = AudioOutGainCodeToCMX7262ValueReg(AudioOutLevel);
	
	CMX7262AudioGainIn = AudioInLevel;
	
	//���������� ����������� �������� ��������� � CMX7262
	SetCMX7262AudioGains(CMX7262AudioGainIn, CMX7262AudioGainOut);
}

void RadioModule::ApplyRadioConfig()
{
	const CC1120regSetting_t *CC1120_Config;
	uint8_t nSizeConfig;
	
	//TODO ����� ����������� ������� ������, � ����� ��������� ��������� ���������� (��������/�����), 
	//� ����� ��������� ������� ��� � ��� ���������

	//TODO ���������, ����� ������������ ����������� � ������� ������. ���� ����� ������������ �� ����������
	//�� �������, �� ��������� ������ ���������
	
	switch(RadioBaudRate)
	{
		case RADIO_BAUD_RATE_4800:
			CC1120_Config = CC1120_Config_4800;
			nSizeConfig = sizeof(CC1120_Config_4800)/sizeof(CC1120regSetting_t);
			break;
		case RADIO_BAUD_RATE_9600:
			CC1120_Config = CC1120_Config_9600;
			nSizeConfig = sizeof(CC1120_Config_9600)/sizeof(CC1120regSetting_t);
			break;
		case RADIO_BAUD_RATE_19200:
			CC1120_Config = CC1120_Config_19200;
			nSizeConfig = sizeof(CC1120_Config_19200)/sizeof(CC1120regSetting_t);
			break;
		case RADIO_BAUD_RATE_48000:
			CC1120_Config = CC1120_Config_48000;
			nSizeConfig = sizeof(CC1120_Config_48000)/sizeof(CC1120regSetting_t);
			break;
		default:
			CC1120_Config = CC1120_Config_4800;
			nSizeConfig = sizeof(CC1120_Config_4800)/sizeof(CC1120regSetting_t);
			break;		
	}
	
	CC1120_SetConfig(g_CC1120Struct.hSPI, CC1120_Config, nSizeConfig);
	//������ � ������������� ���������� ������� ������� (�� �������, �������� � ������������ �� ���������)
	NoCurFreqChan = DEFAULT_RX_FREQ_CHAN;
	
	//���������� �������, ������������� �������������
	ApplyRadioFreq();
	
	SwitchToIdleState();
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
		//�� �������� ���� ������� ���������� �������� ������� � ��		
		lFreqValueHz = FreqCodeToHz(NoCurFreqChan);
		
		//���������� �������� ������� � ��1120
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
	CMX7262_AudioInputGain(&g_CMX7262Struct, CMX7262AudioGainIn);
}


uint8_t RadioModule::GetAsyncReqMaskParam()
{
	return(AsyncReqMaskParam);
}

uint8_t RadioModule::GetMaskOfChangedParams()
{
	uint8_t nRes = AsyncReqMaskParam & MaskOfChangedParams;
	MaskOfChangedParams = 0;
	
	return(nRes);
}

uint8_t RadioModule::SetAsyncReqMaskParam(uint8_t mask)
{
	AsyncReqMaskParam = mask;
	
	return(0);
}

void RadioModule::ApplyRadioSignalPower()
{
	#ifndef SMART_PROTOTYPE
	if(RadioSignalPower==RADIO_SIGNALPOWER_LOW)
	{
		//����������� �������� ������� CC1120: � �������� POWER_RAMP ��������� ��������� ��� Reserved, 
		//������� ���������� � SmartRF Studio		
		CC1120_PowerAmpWrite(g_CC1120Struct.hSPI, CC1120_PA_POWER_RAMP_IN_LOWPOW_MODE | 0x40);
		//Front-End ��������� � ����� Bypass
		SKY_BYP_HIGH();		
	}
	else
	{
		//����������� �������� ������� CC1120: � �������� POWER_RAMP ��������� ��������� ��� Reserved, 
		//������� ���������� � SmartRF Studio
		CC1120_PowerAmpWrite(g_CC1120Struct.hSPI, CC1120_PA_POWER_RAMP_IN_HIGHPOW_MODE | 0x40);
		//Front-End ��������� �� ������ Bypass � ���������� �����
		SKY_BYP_LOW();
	}
	#endif
}

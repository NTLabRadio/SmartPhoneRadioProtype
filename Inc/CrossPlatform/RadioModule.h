/**
  ******************************************************************************
  * @file    RadioModule.h
  * @brief   Файл, содержащий все необходимое для рабочего режима радиомодуля
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RADIOMODULE_H
#define __RADIOMODULE_H

#include <stdint.h>
#include <version.h>

#include "cc1120.h"
#include "cmx7262.h"
#include "SPIMMessage.h"
 
  
enum en_RadioChanTypes
{
	RADIOCHAN_TYPE_IDLE,	
	RADIOCHAN_TYPE_VOICE		=1,
	RADIOCHAN_TYPE_DATA			=2,
	RADIOCHAN_TYPE_RTKDATA	=3,
	NUM_RADIOCHAN_TYPES
};

enum en_RadioSignalPowers
{
	RADIO_SIGNALPOWER_LOW		=0,
	RADIO_SIGNALPOWER_HIGH	=1,
	NUM_RADIO_SIGNALPOWER_MODES
};

enum en_ARMPowerModes
{
	ARM_POWERMODE_NORMAL		=0,
	ARM_POWERMODE_SLEEP			=1,
	NUM_ARM_POWERMODES
};

enum en_RadioBaudRates
{
	RADIO_BAUD_RATE_4800		=0,
	RADIO_BAUD_RATE_9600		=1,
	RADIO_BAUD_RATE_19200		=2,
	RADIO_BAUD_RATE_48000		=3,
	NUM_RADIO_BAUD_RATES
};

enum en_RadioChanStates
{
	RADIOCHAN_STATE_IDLE,
	RADIOCHAN_STATE_RECEIVE,
	RADIOCHAN_STATE_WAIT_RECEIVE,
	RADIOCHAN_STATE_TRANSMIT
};

enum en_RadioModuleStates
{
	RADIOMODULE_STATE_IDLE,
	RADIOMODULE_STATE_TX_WAITING,
	RADIOMODULE_STATE_TX_RUNNING,
	RADIOMODULE_STATE_RX_WAITING,
	RADIOMODULE_STATE_RX_RUNNING,
	NUM_RADIOMODULE_STATES
};

typedef enum SymbolPatterns_en
{	
	SYMBOL_PATTERN_ZEROES,
	SYMBOL_PATTERN_TONE,
	NUM_OF_SYMBOL_PATTERNS
} en_SymbolPatterns;



class RadioModule
{
public:
	
  RadioModule();

	uint8_t SetRadioModuleState(uint8_t state);
	uint8_t GetRadioModuleState();

	void SwitchToIdleState();

	uint8_t SetRadioChanType(uint8_t chanType);
	uint8_t GetRadioChanType();

	uint8_t SetRadioSignalPower(uint8_t signalPower);
	uint8_t GetRadioSignalPower();

	uint8_t SetARMPowerMode(uint8_t powerMode);
	uint8_t GetARMPowerMode();

	uint8_t SetRadioBaudRate(uint8_t baudRate);
	uint8_t GetRadioBaudRate();

	uint8_t SetTxFreqChan(uint16_t noFreqChan);
	uint16_t GetTxFreqChan();

	uint8_t SetRxFreqChan(uint16_t noFreqChan);
	uint16_t GetRxFreqChan();

	uint8_t SetAudioInLevel(uint8_t audioLevel);
	uint8_t GetAudioInLevel();

	uint8_t SetAudioOutLevel(uint8_t audioLevel);
	uint8_t GetAudioOutLevel();

	uint8_t GetRSSILevel();
	
	uint8_t SetRadioChanState(uint8_t radioChanState);
	uint8_t GetRadioChanState();

	uint8_t SetTestMode(uint8_t isTestMode);
	uint8_t IsTestMode();

	uint8_t SetTestPattern(uint8_t noPattern);
	uint8_t GetTestPattern();

	uint8_t SetRadioAddress(uint8_t address);
	uint8_t GetRadioAddress();
	
	uint8_t GetAsyncReqMaskParam();
	uint8_t SetAsyncReqMaskParam(uint8_t mask);

	
	uint8_t GetMaskOfChangedParams();

	uint8_t isTxMode();
	uint8_t isRxMode();
	
	void ApplyAudioSettings();
	void ApplyRadioConfig();
	void ApplyRadioFreq();
	void ApplyRadioSignalPower();
	
	uint16_t GetARMSoftVer();

private:
	
	//Состояние радиомодуля (в т.ч. переходные состояния между режимами передачи и приема)
	en_RadioModuleStates RadioModuleState;

	//Тип радиоканала (речь/данные)
	en_RadioChanTypes RadioChanType;
	//Режим мощности ВЧ сигнала
	en_RadioSignalPowers	RadioSignalPower;
	//Режим энергосбережения
	en_ARMPowerModes ARMPowerMode;
	//Канальная скорость передачи данных
	en_RadioBaudRates RadioBaudRate;

	//Текущая рабочая частота радиомодуля
	uint16_t NoCurFreqChan;

	//Рабочие частоты передачи/приема
	uint16_t NoTxFreqChan;
	uint16_t NoRxFreqChan;

	uint8_t AudioInLevel;
	uint8_t AudioOutLevel;

	uint8_t RSSILevel;
	
	//Состояние радиоканала: - активный прием (прием и обработка полезных данных),
	//											 - дежурный прием (ожидание полезных данных),
	//											 - передача
	en_RadioChanStates RadioChanState;

	//Признак тестового режима
	uint8_t IsRadioTestMode;
	//Номер символьного шаблона для тестового режима
	en_SymbolPatterns NoRadioTestPattern;

	uint8_t RadioAddress;
	
	uint8_t AsyncReqMaskParam;
	uint8_t MaskOfChangedParams;

	struct RadioModuleSettings
	{
		en_RadioChanTypes radioChanType;
		en_RadioSignalPowers	radioSignalPower;
		en_ARMPowerModes powerMode;

		uint16_t noTxFreqChan;
		uint16_t noRxFreqChan;

		uint8_t audioInLevel;
		uint8_t audioOutLevel;

		uint8_t levelRSSI;
		en_RadioChanStates radioChanState;
	};

	static const uint32_t RADIO_BASE_FREQ 			= 410000000;
	static const uint32_t RADIO_FREQCHAN_STEP		= 25000;
	
	#ifndef SMART_PROTOTYPE
	static const uint8_t DEFAULT_AUDIO_IN_GAIN 	= 5;
	#else
	static const uint8_t DEFAULT_AUDIO_IN_GAIN 	= 3;
	#endif
	static const uint8_t DEFAULT_AUDIO_OUT_GAIN = 7;

	static const uint16_t DEFAULT_TX_FREQ_CHAN	= 960;
	static const uint16_t DEFAULT_RX_FREQ_CHAN	= 960;
	
	static const uint8_t DEFAULT_RADIO_ADDRESS	= 1;
	
//CMX7262
	static const uint8_t MAX_AUDIO_OUT_GAIN_CODE 						= 7;
	static const uint8_t CMX7262_MAX_AUDIO_OUT_GAIN_VALUE		= 59;
	static const uint8_t CMX7262_STEP_AUDIO_OUT_GAIN_VALUE 	= 9;
	static const uint16_t CMX7262_AUDIO_OUT_EXTRAGAIN_VALUE =	0x8000;

	uint16_t AudioOutGainCodeToCMX7262ValueReg(uint8_t audioOutLevel);
	void SetCMX7262AudioGains(uint16_t CMX7262AudioGainIn, uint16_t CMX7262AudioGainOut);

	uint32_t FreqCodeToHz(uint16_t nFreqCode);
	void SetCC1120Freq(uint32_t lFreq);

};
	 

#endif /* __RADIOMODULE_H */

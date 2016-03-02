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
 
enum en_RadioChanTypes
{
	RADIOCHAN_TYPE_IDLE,	
	RADIOCHAN_TYPE_VOICE			=1,
	RADIOCHAN_TYPE_DATA				=2,
	RADIOCHAN_TYPE_RTKDATA		=3,
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
	ARM_POWERMODE_NORMAL	=0,
	ARM_POWERMODE_SLEEP		=1,
	NUM_ARM_POWERMODES
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
	RADIOMODULE_STATE_RX_RUNNING
};

class RadioModule
{
public:
	
  RadioModule();

	en_RadioModuleStates RadioModuleState;

	uint8_t SetRadioChanType(uint8_t chanType);
	uint8_t GetRadioChanType();

	uint8_t SetRadioSignalPower(uint8_t signalPower);
	uint8_t GetRadioSignalPower();

	uint8_t SetARMPowerMode(uint8_t powerMode);
	uint8_t GetARMPowerMode();

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
	
	void ApplyAudioSettings();
	void ApplyRadioFreq();
	
	uint16_t GetARMSoftVer();

private:
	
	//Тип радиоканала
	en_RadioChanTypes RadioChanType;
	//Режим мощности ВЧ сигнала
	en_RadioSignalPowers	RadioSignalPower;
	//Режим энергосбережения
	en_ARMPowerModes ARMPowerMode;

	//Рабочие частоты передачи/приема
	uint16_t TxRadioFreq;
	uint16_t RxRadioFreq;

	uint8_t AudioInLevel;
	uint8_t AudioOutLevel;

	uint8_t RSSILevel;
	en_RadioChanStates RadioChanState;

	struct RadioModuleSettings
	{
		en_RadioChanTypes radioChanType;
		en_RadioSignalPowers	radioSignalPower;
		en_ARMPowerModes powerMode;

		uint16_t txRadioFreq;
		uint16_t rxRadioFreq;

		uint8_t audioInLevel;
		uint8_t audioOutLevel;

		uint8_t levelRSSI;
		en_RadioChanStates radioChanState;
	};

	#define	RADIO_BASE_FREQ			(410e6)
	#define RADIO_FREQCHAN_STEP	(25e3)
	
	#define DEFAULT_AUDIO_IN_GAIN		(3)
	#define DEFAULT_AUDIO_OUT_GAIN	(7)

//CMX7262
	#define MAX_AUDIO_OUT_GAIN_CODE						(7)
	#define CMX7262_MAX_AUDIO_OUT_GAIN_VALUE	(59)
	#define CMX7262_STEP_AUDIO_OUT_GAIN_VALUE	(9)
	#define CMX7262_AUDIO_OUT_EXTRAGAIN_VALUE	(0x80)

	uint16_t AudioOutGainCodeToCMX7262ValueReg(uint8_t audioOutLevel);
	void SetCMX7262AudioGains(uint16_t CMX7262AudioGainIn, uint16_t CMX7262AudioGainOut);

	uint32_t FreqCodeToHz(uint16_t nFreqCode);
	void SetCC1120Freq(uint32_t lFreq);

};
	 

#endif /* __RADIOMODULE_H */

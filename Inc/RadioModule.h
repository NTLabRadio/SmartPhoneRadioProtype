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

class RadioModule
{
public:
	
  RadioModule();

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
	
	uint8_t GetRadioChanState();
	
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

};
	 

#endif /* __RADIOMODULE_H */

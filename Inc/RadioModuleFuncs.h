/**
  ******************************************************************************
  * @file    RadioModuleFuncs.h
  * @brief   Файл, содержащий все необходимое для рабочего режима радиомодуля
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RADIOMODULEFUNCS_H
#define __RADIOMODULEFUNCS_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "DebugLog.h"
	 
	 
enum en_RadioChanTypes
{
	RADIOCHAN_TYPE_IDLE,	
	RADIOCHAN_TYPE_VOICE			=1,
	RADIOCHAN_TYPE_DATA				=2,
	RADIOCHAN_TYPE_RTKDATA		=3
};

enum en_RadioSignalPowers
{
	RADIO_SIGNALPOWER_LOW		=0,
	RADIO_SIGNALPOWER_HIGH	=1
};

enum en_ARMPowerModes
{
	ARM_POWERMODE_NORMAL	=0,
	ARM_POWERMODE_SLEEP		=1
};

//Тип радиоканала
extern en_RadioChanTypes g_RadioChanType;
//Режим мощности ВЧ сигнала
extern en_RadioSignalPowers	g_RadioSignalPower;
//Режим энергосбережения
extern en_ARMPowerModes g_RadioPowerMode;

//Рабочие частоты передачи/приема
extern uint16_t g_TxRadioFreq;
extern uint16_t g_RxRadioFreq;

extern uint8_t g_AudioInLevel;
extern uint8_t g_AudioOutLevel;
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __RADIOMODULEFUNCS_H */

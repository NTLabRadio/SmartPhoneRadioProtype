/**
  ******************************************************************************
  * @file    globals.h
  * @brief   Файл, содержащий глобальные настройки проекта SmartPhonePrototype
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GLOBALS_H
#define __GLOBALS_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <limits.h>	 
#include "cmx7262.h"
	 
#define TRUE 	(1)
#define FALSE (0)
	 
	 
	 
//Длительность звуковых данных одного вокодерного буфера, мс
#define CMX7262_BUFFER_DURATION_MS (60)

//Число буферов данных вокодера, накапливаемых радимодулем прежде чем инициализировать передачу
#define NUM_CMX7262_BUFFERS_INITACCUM_FOR_TX	(3)		//60 мс x 3 = 180 мс

//Число буферов данных вокодера в одном радиопакете
#define NUM_CMX7262_BUFFERS_IN_RADIOPACK	(3)				//60 мс x 3 = 180 мс

//Размер радиопакета в режиме речевого обмена, только речевые данные
#define RADIOPACK_VOICEMODE_SIZE 	(NUM_CMX7262_BUFFERS_IN_RADIOPACK * CMX7262_CODEC_BUFFER_SIZE)

//Размер радиопакета в режиме передачи данных
#define RADIOPACK_DATAMODE_SIZE 	(RADIOPACK_VOICEMODE_SIZE)

//Размер данных от вокодера, накапливаемый радимодулем прежде чем инициализировать передачу
#define SIZE_OF_DATA_FROM_CMX7262_INITACCUM_FOR_TX	(NUM_CMX7262_BUFFERS_INITACCUM_FOR_TX * CMX7262_CODEC_BUFFER_SIZE)

//Размер расширенного радиопакета в режиме речевого обмена, речевые данные + служебные
//Служебные данные - только 1-байтовый адрес, предваряющий речевые данные
#define RADIOPACK_MODE4800_EXTSIZE	(RADIOPACK_VOICEMODE_SIZE+5)

#define RADIOPACK_MODE9600_EXTSIZE	(RADIOPACK_MODE4800_EXTSIZE)
#define RADIOPACK_MODE19200_EXTSIZE	(RADIOPACK_MODE4800_EXTSIZE)
#define RADIOPACK_MODE48000_EXTSIZE	(RADIOPACK_MODE4800_EXTSIZE)

//Число статус-байтов, добавляемых СС1120 к полезным принятым данным и сообщающих уровень RSSI, LQI и результат проверки CRC
#define SIZE_OF_RADIO_STATUS	(2)

//Максимальный размер радиопакета, байт
#define MAX_RADIOPACK_SIZE	(128)


//Уровень выходного сигнала трансивера СС1120 в режиме пониженной мощности радиомодуля, дБм
//NO: Параметр может быть задан в пределах от -16 до +14 с шагом 0.5
#define CC1120_PA_DBM_IN_LOWPOW_MODE (14)

//Уровень выходного сигнала трансивера СС1120 в режиме повышенной мощности радиомодуля, дБм
//NO: Параметр может быть задан в пределах от -16 до +14 с шагом 0.5
#define CC1120_PA_DBM_IN_HIGHPOW_MODE (10)


//Выражение для вычисления значения настроечного параметра PA power ramp target level трансивера СС1120
//в зависимости от целевого уровня в дБ (см. User's Guide на CC1120, описание регистров)
#define CC1120_POWERRAMP_FROM_DBM(LEVEL_DBM) ((uint8_t)(2*LEVEL_DBM +35))

//Значение параметра PA power ramp target level трансивера СС1120 для режима пониженной мощности
#define CC1120_PA_POWER_RAMP_IN_LOWPOW_MODE		CC1120_POWERRAMP_FROM_DBM(CC1120_PA_DBM_IN_LOWPOW_MODE)
//Значение параметра PA power ramp target level трансивера СС1120 для режима повышенной мощности
#define CC1120_PA_POWER_RAMP_IN_HIGHPOW_MODE	CC1120_POWERRAMP_FROM_DBM(CC1120_PA_DBM_IN_HIGHPOW_MODE)

//Значение параметра PA power ramp target level трансивера СС1120 по умолчанию
#define CC1120_DEFAULT_PA_POWER_RAMP		(CC1120_PA_POWER_RAMP_IN_LOWPOW_MODE)

	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GLOBALS_H */

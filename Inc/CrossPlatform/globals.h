/**
  ******************************************************************************
  * @file    globals.h
  * @brief   ‘айл, содержащий глобальные настройки проекта SmartPhonePrototype
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
	 
	 
	 
//ƒлительность звуковых данных одного вокодерного буфера, мс
#define CMX7262_BUFFER_DURATION_MS (60)

//„исло буферов данных вокодера, накапливаемых радимодулем прежде чем инициализировать передачу
#define NUM_CMX7262_BUFFERS_INITACCUM_FOR_TX	(3)		//60 мс x 3 = 180 мс

//„исло буферов данных вокодера в одном радиопакете
#define NUM_CMX7262_BUFFERS_IN_RADIOPACK	(3)				//60 мс x 3 = 180 мс

//–азмер радиопакета в режиме речевого обмена, только речевые данные
#define RADIOPACK_VOICEMODE_SIZE 	(NUM_CMX7262_BUFFERS_IN_RADIOPACK * CMX7262_CODEC_BUFFER_SIZE)

//–азмер радиопакета в режиме передачи данных
#define RADIOPACK_DATAMODE_SIZE 	(RADIOPACK_VOICEMODE_SIZE)

//–азмер данных от вокодера, накапливаемый радимодулем прежде чем инициализировать передачу
#define SIZE_OF_DATA_FROM_CMX7262_INITACCUM_FOR_TX	(NUM_CMX7262_BUFFERS_INITACCUM_FOR_TX * CMX7262_CODEC_BUFFER_SIZE)

//–азмер расширенного радиопакета в режиме речевого обмена, речевые данные + служебные
//—лужебные данные - только 1-байтовый адрес, предвар€ющий речевые данные
#define RADIOPACK_MODE4800_EXTSIZE	(RADIOPACK_VOICEMODE_SIZE+5)

#define RADIOPACK_MODE9600_EXTSIZE	(RADIOPACK_MODE4800_EXTSIZE)
#define RADIOPACK_MODE19200_EXTSIZE	(RADIOPACK_MODE4800_EXTSIZE)
#define RADIOPACK_MODE48000_EXTSIZE	(RADIOPACK_MODE4800_EXTSIZE)

//„исло статус-байтов, добавл€емых ——1120 к полезным прин€тым данным и сообщающих уровень RSSI, LQI и результат проверки CRC
#define SIZE_OF_RADIO_STATUS	(2)

//ћаксимальный размер радиопакета, байт
#define MAX_RADIOPACK_SIZE	(128)
	 
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GLOBALS_H */

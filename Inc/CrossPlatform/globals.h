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
#define NUM_CMX7262_BUFFERS_INITACCUM_FOR_TX	(6)		//60 мс x 6 = 360 мс

//Число буферов данных вокодера в одном радиопакете
#define NUM_CMX7262_BUFFERS_IN_RADIOPACK	(3)				//60 мс x 3 = 180 мс

//Размер радиопакета в режиме речевого обмена, только речевые данные
#define RADIOPACK_VOICEMODE_SIZE 	(NUM_CMX7262_BUFFERS_IN_RADIOPACK*CMX7262_CODEC_BUFFER_SIZE)


//Размер расширенного радиопакета в режиме речевого обмена, речевые данные + служебные
//Служебные данные - только 1-байтовый адрес, предваряющий речевые данные
#define RADIOPACK_MODE4800_EXTSIZE	(RADIOPACK_VOICEMODE_SIZE+5)


#define MAX_RADIOPACK_SIZE	(128)	 
	 
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GLOBALS_H */

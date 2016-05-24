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


#include <limits.h>	 
#include "cmx7262.h"
	 
#define TRUE 	(1)
#define FALSE (0)
	 

// ------------------------- Memory Map ----------------------------
	 
#define MAX_SIZE_OF_CMX7262_IMAGE	 	(92*1024)	//92кБ
	 
#ifdef STM32F103xE	 //Макет
	//                   								Start											Space     							Current length
	#define ARM_CODE_ADDR     					((uint32_t)0x08000000)    //64kbytes  
	#define CMX7262_IMAGE_ADDR  				((uint32_t)0x08010000)	  //112kbytes 						~91kbytes
	#define RADIOMODULE_SETTINGS_ADDR	 	((uint32_t)0x0803F800)		//2kbytes								32 bytes
	
	#define CMX7262_IMAGE_IN_FLASH
#endif


#ifdef STM32F071xB	//Радиомодуль

#ifdef SIZE_OF_ARMFIRM_32KB
	//                   								Start											Space     							Current length
	#define ARM_CODE_ADDR     					((uint32_t)0x08000000)    //32kbytes  
	#define CMX7262_IMAGE_ADDR   				((uint32_t)0x08008000)	  //92kbytes 							~91kbytes
	#define RADIOMODULE_SETTINGS_ADDR		((uint32_t)0x0801F000)		//4kbytes								32 bytes
	
	#define CMX7262_IMAGE_IN_FLASH
#endif

#ifdef SIZE_OF_ARMFIRM_34KB
	//                   								Start											Space     							Current length
	#define ARM_CODE_ADDR     					((uint32_t)0x08000000)    //34kbytes  
	#define CMX7262_IMAGE_ADDR   				((uint32_t)0x08008800)	  //92kbytes 							~91kbytes
	#define RADIOMODULE_SETTINGS_ADDR		((uint32_t)0x0801F800)		//2kbytes								32 bytes
	
	#define CMX7262_IMAGE_IN_FLASH
#endif

#if !defined(SIZE_OF_ARMFIRM_32KB) && !defined(SIZE_OF_ARMFIRM_34KB)
	//                   								Start											Space     							Current length
	#define ARM_CODE_ADDR     					((uint32_t)0x08000000)    //127kbytes  
	#define RADIOMODULE_SETTINGS_ADDR		((uint32_t)0x0801FC00)		//1kbytes								32 bytes
	
	#define CMX7262_IMAGE_IN_EEPROM
#endif

#endif	 
	 
// --------------------------------------------------------------------
	 
//Длительность звуковых данных одного вокодерного буфера, мс
#define CMX7262_BUFFER_DURATION_MS (60)

//Число буферов данных вокодера в одном радиопакете
#define NUM_CMX7262_BUFFERS_IN_RADIOPACK	(3)				//60 мс x 3 = 180 мс

//Размер полезных данных радиопакета в режиме речевого обмена
#define RADIOPACK_VOICEMODE_PAYLOAD_SIZE 	(NUM_CMX7262_BUFFERS_IN_RADIOPACK * CMX7262_CODEC_BUFFER_SIZE)

//Размер радиопакета в режиме передачи данных
#define RADIOPACK_DATAMODE_PAYLOAD_SIZE 	(RADIOPACK_VOICEMODE_PAYLOAD_SIZE)

//Число буферов данных вокодера, накапливаемых радимодулем прежде чем инициализировать передачу
#define NUM_CMX7262_BUFFERS_INITACCUM_FOR_TX	(3)		//60 мс x 3 = 180 мс

//Размер данных от вокодера, накапливаемый радимодулем прежде чем инициализировать передачу
#define SIZE_OF_DATA_FROM_CMX7262_INITACCUM_FOR_TX	(NUM_CMX7262_BUFFERS_INITACCUM_FOR_TX * CMX7262_CODEC_BUFFER_SIZE)

//Максимальный размер радиопакета, байт
#ifndef RADIOPACK_SIZE_GREATER_THAN_127
	#define RADIOPACK_MAX_SIZE	(127)
#else
	#define RADIOPACK_MAX_SIZE	(255)
#endif

//Размер расширенного радиопакета в режиме речевого обмена, речевые данные + служебные
//#define RADIOPACK_DEFAULT_SIZE	(RADIOPACK_VOICEMODE_PAYLOAD_SIZE + RadioMessage::SIZE_OF_HEADER)
#define RADIOPACK_DEFAULT_SIZE	(RADIOPACK_VOICEMODE_PAYLOAD_SIZE + 5)

//Число статус-байтов, добавляемых СС1120 к полезным принятым данным и сообщающих уровень RSSI, LQI и результат проверки CRC
#define SIZE_OF_RADIO_STATUS	(2)


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


#endif /* __GLOBALS_H */

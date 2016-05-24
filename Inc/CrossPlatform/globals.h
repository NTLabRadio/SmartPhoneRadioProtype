/**
  ******************************************************************************
  * @file    globals.h
  * @brief   ����, ���������� ���������� ��������� ������� SmartPhonePrototype
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
	 
#define MAX_SIZE_OF_CMX7262_IMAGE	 	(92*1024)	//92��
	 
#ifdef STM32F103xE	 //�����
	//                   								Start											Space     							Current length
	#define ARM_CODE_ADDR     					((uint32_t)0x08000000)    //64kbytes  
	#define CMX7262_IMAGE_ADDR  				((uint32_t)0x08010000)	  //112kbytes 						~91kbytes
	#define RADIOMODULE_SETTINGS_ADDR	 	((uint32_t)0x0803F800)		//2kbytes								32 bytes
	
	#define CMX7262_IMAGE_IN_FLASH
#endif


#ifdef STM32F071xB	//�����������

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
	 
//������������ �������� ������ ������ ����������� ������, ��
#define CMX7262_BUFFER_DURATION_MS (60)

//����� ������� ������ �������� � ����� �����������
#define NUM_CMX7262_BUFFERS_IN_RADIOPACK	(3)				//60 �� x 3 = 180 ��

//������ �������� ������ ����������� � ������ �������� ������
#define RADIOPACK_VOICEMODE_PAYLOAD_SIZE 	(NUM_CMX7262_BUFFERS_IN_RADIOPACK * CMX7262_CODEC_BUFFER_SIZE)

//������ ����������� � ������ �������� ������
#define RADIOPACK_DATAMODE_PAYLOAD_SIZE 	(RADIOPACK_VOICEMODE_PAYLOAD_SIZE)

//����� ������� ������ ��������, ������������� ����������� ������ ��� ���������������� ��������
#define NUM_CMX7262_BUFFERS_INITACCUM_FOR_TX	(3)		//60 �� x 3 = 180 ��

//������ ������ �� ��������, ������������� ����������� ������ ��� ���������������� ��������
#define SIZE_OF_DATA_FROM_CMX7262_INITACCUM_FOR_TX	(NUM_CMX7262_BUFFERS_INITACCUM_FOR_TX * CMX7262_CODEC_BUFFER_SIZE)

//������������ ������ �����������, ����
#ifndef RADIOPACK_SIZE_GREATER_THAN_127
	#define RADIOPACK_MAX_SIZE	(127)
#else
	#define RADIOPACK_MAX_SIZE	(255)
#endif

//������ ������������ ����������� � ������ �������� ������, ������� ������ + ���������
//#define RADIOPACK_DEFAULT_SIZE	(RADIOPACK_VOICEMODE_PAYLOAD_SIZE + RadioMessage::SIZE_OF_HEADER)
#define RADIOPACK_DEFAULT_SIZE	(RADIOPACK_VOICEMODE_PAYLOAD_SIZE + 5)

//����� ������-������, ����������� ��1120 � �������� �������� ������ � ���������� ������� RSSI, LQI � ��������� �������� CRC
#define SIZE_OF_RADIO_STATUS	(2)


//������� ��������� ������� ���������� ��1120 � ������ ���������� �������� �����������, ���
//NO: �������� ����� ���� ����� � �������� �� -16 �� +14 � ����� 0.5
#define CC1120_PA_DBM_IN_LOWPOW_MODE (14)

//������� ��������� ������� ���������� ��1120 � ������ ���������� �������� �����������, ���
//NO: �������� ����� ���� ����� � �������� �� -16 �� +14 � ����� 0.5
#define CC1120_PA_DBM_IN_HIGHPOW_MODE (10)


//��������� ��� ���������� �������� ������������ ��������� PA power ramp target level ���������� ��1120
//� ����������� �� �������� ������ � �� (��. User's Guide �� CC1120, �������� ���������)
#define CC1120_POWERRAMP_FROM_DBM(LEVEL_DBM) ((uint8_t)(2*LEVEL_DBM +35))

//�������� ��������� PA power ramp target level ���������� ��1120 ��� ������ ���������� ��������
#define CC1120_PA_POWER_RAMP_IN_LOWPOW_MODE		CC1120_POWERRAMP_FROM_DBM(CC1120_PA_DBM_IN_LOWPOW_MODE)
//�������� ��������� PA power ramp target level ���������� ��1120 ��� ������ ���������� ��������
#define CC1120_PA_POWER_RAMP_IN_HIGHPOW_MODE	CC1120_POWERRAMP_FROM_DBM(CC1120_PA_DBM_IN_HIGHPOW_MODE)

//�������� ��������� PA power ramp target level ���������� ��1120 �� ���������
#define CC1120_DEFAULT_PA_POWER_RAMP		(CC1120_PA_POWER_RAMP_IN_LOWPOW_MODE)


#endif /* __GLOBALS_H */

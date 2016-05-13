/********************************************************************************
  * @file    trellisCoder.h
  * @brief   ����, ���������� ���������� �������� � �������, ����������� ���������
  *					�����������������	����������� � ������������� �������(�����������)-����.
	*  				2 ���� �������, ������������� � ���� ������, ����� ������������� �������
	*					��������� APCO25,Phase1, ����������� ��� ������������ ��������������� �
	*					����������������� ������. ������������� ������� - 1/2 � 3/4. 
	*					���������� ����� �������� � ���� �������� ����������� ������. �������� 
	*					������������� - �� ���� ��������� ������� � �������� ���������.
	*					������ ����� ����������� ��������� ��� ����������� ���������� � 
	*					4-����������� ������������ (����������� ����������� ��� �������, ������
	*					2 ������� ����� ����� �������� � ������������� �������������� ������) � 
	*					��� ������������ ��������� �������� ������� ������
	*					�������� ��������� ����������� ��. � ��������� TIA-102.BAAA-A
	*
	*******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
	*******************************************************************************/


#ifndef TRELLISCODER_H
#define TRELLISCODER_H


#include <limits.h>
#include <string.h>

#ifdef STM32F071xB
#include "stm32f0xx_hal.h"
#endif
	 
#ifdef STM32F103xE	 
#include "stm32f1xx_hal.h"
#endif

//2^x
#define POW2(x) (1<<(x))

//������ ����� ������ �� �����������, ���
#define SIZE_OF_UNCODED_FRAME_TRELLIS_1_2	(96)
#define SIZE_OF_UNCODED_FRAME_TRELLIS_3_4	(144)

//������ ����� ������ ����� �����������, ��� (���������� ��� ���������� 1/2 � 3/4)
#define SIZE_OF_CODED_FRAME_TRELLIS		(196)

//������ ��������� FSM (�������� ��������� ���������) ������, ���
//���� �� ������ ����� ������� ������� ������
#define SIZE_OF_STATE_TRELLIS_1_2		(2)
#define SIZE_OF_STATE_TRELLIS_3_4		(3)

#define NUM_STATES_TRELLIS_1_2 (POW2(SIZE_OF_STATE_TRELLIS_1_2))
#define NUM_STATES_TRELLIS_3_4 (POW2(SIZE_OF_STATE_TRELLIS_3_4))

//������ ��������� ������� ������, ���
#define SIZE_OF_OUTPUT_TRELLIS	(4)
//������ ��������� ���������
#define SIZE_OF_TRELLIS_OUTPUT_CONSTELLATION 		(SIZE_OF_OUTPUT_TRELLIS*SIZE_OF_OUTPUT_TRELLIS)

void trellisEnc1_2(const int8_t * const pDataIn, int8_t * const pDataOut);
void trellisEnc3_4(const int8_t * const pDataIn, int8_t * const pDataOut);
int16_t trellisDec1_2(const int8_t * const pDataIn, int8_t * const pDataOut);
int16_t trellisDec3_4(const int8_t * const pDataIn, int8_t * const pDataOut);

void TestTrellisCoder1_2();
void TestTrellisCoder3_4();

#endif // TRELLISCODER_H

/********************************************************************************
  * @file    trellisCoder.h
  * @brief   Файл, содержащий объявление объектов и функций, реализующих алгоритмы
  *					помехоустойчивого	кодирования и декодирования треллис(сверточного)-кода.
	*  				2 типа кодеров, реализованных в этом модуле, точно соответствуют кодерам
	*					стандарта APCO25,Phase1, применяемым для помехозащиты гарантированных и
	*					негарантированных данных. Эффективность кодеров - 1/2 и 3/4. 
	*					Реализация также включает в себя алгоритм перемежения данных. Алгоритм 
	*					декодирования - на базе алгоритма Витерби с жесткими решениями.
	*					Кроме стандартного алгоритма перемежения, оптимального для 4-позиционной 
	*					манипуляции (перемежение выполняется для дибитов, ошибка 2 смежных битов 
	*					может привести к невозможности восстановления данных) (директива 
	*					препроцессора APCO25_INTERLEAVER), реализован побитовый матричный алгоритм
	*					перемежения для применения с 2-позиционной манипуляцией
	*					Описание алгоритма кодирования см. в стандарте TIA-102.BAAA-A
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
#include <stdint.h>

//2^x
#define POW2(x) (1<<(x))

//Размер кадра данных до кодирования, бит
#define SIZE_OF_UNCODED_FRAME_TRELLIS_1_2	(96)
#define SIZE_OF_UNCODED_FRAME_TRELLIS_3_4	(144)

//Размер кадра данных после кодирования, бит (одинаковый для алгоритмов 1/2 и 3/4)
#define SIZE_OF_CODED_FRAME_TRELLIS		(196)

//Размер состояния FSM (автомата конечного состояния) кодера, бит
//Этот же размер имеют входные символы кодера
#define SIZE_OF_STATE_TRELLIS_1_2		(2)
#define SIZE_OF_STATE_TRELLIS_3_4		(3)

#define NUM_STATES_TRELLIS_1_2 (POW2(SIZE_OF_STATE_TRELLIS_1_2))
#define NUM_STATES_TRELLIS_3_4 (POW2(SIZE_OF_STATE_TRELLIS_3_4))

//Размер выходного символа кодера, бит
#define SIZE_OF_OUTPUT_TRELLIS	(4)
//Размер выходного созвездия
#define SIZE_OF_TRELLIS_OUTPUT_CONSTELLATION 		(SIZE_OF_OUTPUT_TRELLIS*SIZE_OF_OUTPUT_TRELLIS)

#ifndef APCO25_INTERLEAVER
	#define NUM_RAWS_IN_INTERLEAVE_MATRIX			(7)
	#define NUM_COLUMNS_IN_INTERLEAVE_MATRIX	(SIZE_OF_CODED_FRAME_TRELLIS/NUM_RAWS_IN_INTERLEAVE_MATRIX)
#endif

void trellisEnc1_2(const int8_t * const pDataIn, int8_t * const pDataOut);
void trellisEnc3_4(const int8_t * const pDataIn, int8_t * const pDataOut);
int16_t trellisDec1_2(const int8_t * const pDataIn, int8_t * const pDataOut);
int16_t trellisDec3_4(const int8_t * const pDataIn, int8_t * const pDataOut);

void InterleaveTrellisData(int8_t* pDataIn, int8_t* pDataOut);
void DeinterleaveTrellisData(int8_t* pDataIn, int8_t* pDataOut);

void TestTrellisCoder1_2(void);
void TestTrellisCoder3_4(void);

#endif // TRELLISCODER_H

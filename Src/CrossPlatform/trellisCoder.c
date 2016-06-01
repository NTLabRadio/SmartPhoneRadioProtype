#include "trellisCoder.h"

// Таблица переходов автомата конечного состояния для треллис-кодера 1/2 (Trellis Encoder State Transition Table)
const int8_t TableTransTrellis1_2[NUM_STATES_TRELLIS_1_2][NUM_STATES_TRELLIS_1_2] =
						{  0,15,12, 3,
						   4,11, 8, 7,
						  13, 2, 1,14,
						   9, 6, 5,10};

// Таблица переходов автомата конечного состояния для треллис-кодера 3/4 (Trellis Encoder State Transition Table)
const int16_t TableTransTrellis3_4[NUM_STATES_TRELLIS_3_4][NUM_STATES_TRELLIS_3_4] =
						 {{0, 8, 4,12, 2,10, 6,14},
							{4,12, 2,10, 6,14, 0, 8},
							{1, 9, 5,13, 3,11, 7,15},
							{5,13, 3,11, 7,15, 1, 9},
							{3,11, 7,15, 1, 9, 5,13},
							{7,15, 1, 9, 5,13, 3,11},
							{2,10, 6,14, 0, 8, 4,12},
							{6,14, 0, 8, 4,12, 2,10}};
							 
#ifdef APCO25_INTERLEAVER
// Таблица для перемежения дибитов (Interleave Table)
const int8_t TableInterleave[SIZE_OF_CODED_FRAME_TRELLIS/2] =
								 {0,1,8,9,16,17,24,25,
							   32,33,40,41,48,49,56,57,
							   64,65,72,73,80,81,88,89,
							   96,97,2,3,10,11,18,19,
							   26,27,34,35,42,43,50,51,
							   58,59,66,67,74,75,82,83,
							   90,91,4,5,12,13,20,21,
							   28,29,36,37,44,45,52,53,
							   60,61,68,69,76,77,84,85,
							   92,93,6,7,14,15,22,23,
							   30,31,38,39,46,47,54,55,
							   62,63,70,71,78,79,86,87,
							   94,95};
#endif								 
						   
//Constellation to Dibit Pair Mapping
//Кодер сопоставляет числам 0-15 (можно их назвать номерами точек созвездия) пары дибитов в соответствии со следующей таблицей
const int8_t ConstelToDibitPair[SIZE_OF_TRELLIS_OUTPUT_CONSTELLATION][2] =
						{	{0,2}, {2,2}, {1,3}, {3,3}, {3,2}, {1,2}, {2,3}, {0,3},
							{3,1}, {1,1},	{2,0}, {0,0},	{0,1}, {2,1},	{1,0} ,{3,0} };
				
//Полная таблица переходов для треллис-кодера 1/2 (Trellis Encoder State Transition Table + Constellation to Dibit Pair Mapping)
const int8_t TableTransMapAndTrellis1_2[NUM_STATES_TRELLIS_1_2][NUM_STATES_TRELLIS_1_2] =
						 {{  2, 12, 1, 15 },
							{  14, 0, 13, 3 },
							{  9, 7, 10, 4 },
							{  5, 11, 6, 8 }};

//Полная таблица переходов для треллис-кодера 3/4 (Trellis Encoder State Transition Table + Constellation to Dibit Pair Mapping)
const int8_t TableTransMapAndTrellis3_4[NUM_STATES_TRELLIS_3_4][NUM_STATES_TRELLIS_3_4] =
					   {{2,	13,	14,	1,	7,	8,	11,	4},
							{14,1,	7,	8,	11,	4,	2,	13},
							{10,5,	6,	9,	15,	0,	3,	12},
							{6,	9,	15,	0,	3,	12,	10,	5},
							{15,0,	3,	12,	10,	5,	6,	9},
							{3,	12,	10,	5,	6,	9,	15,	0},
							{7,	8,	11,	4,	2,	13,	14,	1},
							{11,4,	2,	13,	14,	1,	7,	8}};

								
//Функция кодирования данных кодом треллиса с эффективностью 1/2
//Параметры:
//	pDataIn -- указатель на входной массив данных для кодирования, значение каждого элемента =0 или 1;
//	pDataEncoded -- указатель на выходной массив данных после кодирования, значение каждого элемента =0 или 1;
void trellisEnc1_2(const int8_t * const pDataIn, int8_t * const pDataOut)
{
	uint16_t cntDataInBits, cntDataOutBits;
	int8_t curCoderInput;			//текущее значение входного дибита кодера (Final State Machine Input)
	int8_t curCoderState;			//текущее состояние конечного автомата (Current State Storage)
	int8_t nConstelPointOut;	//число 0-15, указывающее на выходную пару дибитов (Constellation Point Output)
	int8_t nDibitOut;					//значение выходного дибита
	
	int8_t pCoderOut[SIZE_OF_CODED_FRAME_TRELLIS];	//результат кодирования (без перемежения)
	memset(pCoderOut,0,SIZE_OF_CODED_FRAME_TRELLIS*sizeof(int8_t));
	
	curCoderState = 0;	//начальное состояние автомата - нулевое
	for(cntDataInBits=0,cntDataOutBits=0; cntDataInBits<SIZE_OF_UNCODED_FRAME_TRELLIS_1_2; cntDataOutBits+=SIZE_OF_OUTPUT_TRELLIS,cntDataInBits+=SIZE_OF_STATE_TRELLIS_1_2)
	{			
		curCoderInput = (pDataIn[cntDataInBits]<<1) + (pDataIn[cntDataInBits+1]);	//берем дибит из входных данных
		
		nConstelPointOut = TableTransTrellis1_2[curCoderState][curCoderInput];	//Входному значению автомата кодирования и его текущему состоянию
																																//ставится в соответствие число 0-15
		
		curCoderState = curCoderInput;											//Изменяем текущее состояние кодера на значение задержанного входного дибита

		//Число 0-15 преобразуем в пару дибитов		
		nDibitOut = ConstelToDibitPair[nConstelPointOut][0];			//первый дибит
		pCoderOut[cntDataOutBits] = (nDibitOut) & 1;
		pCoderOut[cntDataOutBits+1] = (nDibitOut>>1) & 1;
		
		nDibitOut = ConstelToDibitPair[nConstelPointOut][1];			//второй дибит
		pCoderOut[cntDataOutBits+2] = (nDibitOut) & 1;
		pCoderOut[cntDataOutBits+3] = (nDibitOut>>1) & 1;
	}
	
	//В конце к данным добавляем 2 нулевых flush-бита
	nConstelPointOut = TableTransTrellis1_2[curCoderState][0];	//Последнему (нулевому) значению автомата кодирования и его текущему состоянию
																									//ставим в соответствие число 0-15
	
	//Число 0-15 преобразуем в пару дибитов
	nDibitOut = ConstelToDibitPair[nConstelPointOut][0];
	pCoderOut[cntDataOutBits] = (nDibitOut) & 1;
	pCoderOut[cntDataOutBits+1] = (nDibitOut>>1) & 1;
	
	nDibitOut = ConstelToDibitPair[nConstelPointOut][1];
	pCoderOut[cntDataOutBits+2] = (nDibitOut) & 1;
	pCoderOut[cntDataOutBits+3] = (nDibitOut>>1) & 1;

	#ifndef TRELLIS_WITHOUT_INTERLEAVER
	//Перемежение
	InterleaveTrellisData(pCoderOut, pDataOut);
	#else
	memcpy(pDataOut,pCoderOut,SIZE_OF_CODED_FRAME_TRELLIS*sizeof(int8_t));
	#endif
}


//Функция кодирования данных кодом треллиса с эффективностью 3/4
//Параметры:
//	pDataIn -- указатель на входной массив данных для кодирования, значение каждого элемента =0 или 1;
//	pDataEncoded -- указатель на выходной массив данных после кодирования, значение каждого элемента =0 или 1;
void trellisEnc3_4(const int8_t * const pDataIn, int8_t * const pDataOut)
{
	uint16_t cntDataInBits, cntDataOutBits;
	int8_t curCoderInput;			//текущее значение входного дибита кодера (Final State Machine Input)
	int8_t curCoderState;			//текущее состояние конечного автомата (Current State Storage)
	int8_t nConstelPointOut;	//число 0-15, указывающее на выходную пару дибитов (Constellation Point Output)
	int8_t nDibitOut;					//значение выходного дибита

	int8_t pCoderOut[SIZE_OF_CODED_FRAME_TRELLIS];	//результат кодирования (без перемежения)
	memset(pCoderOut,0,SIZE_OF_CODED_FRAME_TRELLIS*sizeof(int8_t));

	curCoderState = 0;	//начальное состояние автомата - нулевое
	for (cntDataInBits=0,cntDataOutBits=0; cntDataInBits<SIZE_OF_UNCODED_FRAME_TRELLIS_3_4; cntDataOutBits+=SIZE_OF_OUTPUT_TRELLIS,cntDataInBits+=SIZE_OF_STATE_TRELLIS_3_4)
	{
		curCoderInput = (pDataIn[cntDataInBits]<<2)+(pDataIn[cntDataInBits+1]<<1)+(pDataIn[cntDataInBits+2]);	//берем триибит из входных данных

		nConstelPointOut = TableTransTrellis3_4[curCoderState][curCoderInput];	//Входному значению автомата кодирования и его текущему состоянию
																																//ставится в соответствие число 0-15

		curCoderState = curCoderInput;		//Изменяем текущее состояние кодера на значение задержанного входного дибита

		//Число 0-15 преобразуем в пару дибитов
		nDibitOut = ConstelToDibitPair[nConstelPointOut][0];		//первый дибит
		pCoderOut[cntDataOutBits] = (nDibitOut) & 1;
		pCoderOut[cntDataOutBits+1] = (nDibitOut>>1) & 1;

		nDibitOut = ConstelToDibitPair[nConstelPointOut][1];		//второй дибит
		pCoderOut[cntDataOutBits+2] = (nDibitOut) & 1;
		pCoderOut[cntDataOutBits+3] = (nDibitOut>>1) & 1;
	}

	nConstelPointOut = TableTransTrellis3_4[curCoderState][0];	//Последнему (нулевому) значению автомата кодирования и его текущему состоянию
																									//ставим в соответствие число 0-15

	//В конце к данным добавляем 2 нулевых flush-бита	
	nDibitOut = ConstelToDibitPair[nConstelPointOut][0];
	pCoderOut[cntDataOutBits] = (nDibitOut) & 1;
	pCoderOut[cntDataOutBits+1] = (nDibitOut>>1) & 1;
	
	nDibitOut = ConstelToDibitPair[nConstelPointOut][1];
	pCoderOut[cntDataOutBits+2] = (nDibitOut) & 1;
	pCoderOut[cntDataOutBits+3] = (nDibitOut>>1) & 1;

	#ifndef TRELLIS_WITHOUT_INTERLEAVER
	InterleaveTrellisData(pCoderOut, pDataOut);
	#else
	memcpy(pDataOut,pCoderOut,SIZE_OF_CODED_FRAME_TRELLIS*sizeof(int8_t));
	#endif	
}




// Расчет метрики для ребра (числа 1 в битовом представлении числа Data)
uint8_t hammingWeight(uint8_t nVectorValue)
{
	int8_t cntBits;
	uint8_t numOnes = 0;

	for(cntBits=0; cntBits<8; cntBits++)
		numOnes += (nVectorValue>>cntBits)&1;

/*	//подсчёт единиц за один такт процессора (работает хорошо при -o3 и в отсутствие оптимизации)
    asm("        MOV *(#_TempCntMet), AC0"); //гружу в AC0 байт, число единиц которого надо посчитать
    asm("        MOV #255, AC1"); //маска на байт
    asm("        BCNT AC0, AC1, TC1, T0"); //подсчёт единиц в регистре AC0; число единиц теперь помещается в регистр T0
    asm("        MOV T0, *(#_TempCntMet)"); //результат записываю в TempCntMet
*/
    return numOnes;
}

typedef struct SoManyWays
{
	//массив путей с лучшими метриками
	int8_t arr[(SIZE_OF_CODED_FRAME_TRELLIS+SIZE_OF_OUTPUT_TRELLIS)/SIZE_OF_OUTPUT_TRELLIS][NUM_STATES_TRELLIS_3_4];
} SoManyWays;


//Декодер кода треллиса 1/2
//Параметры:
//  pDataIn -- указатель на входной массив закодированной последовательности, значение каждого элемента =0 или 1;
//  pDataOut -- указатель на выходной массив декодированной последовательности, значение каждого элемента =0 или 1;
int16_t trellisDec1_2(const int8_t * const pDataIn, int8_t * const pDataOut)
{
	int16_t i, n, j;	
	int16_t arPathMetrics[NUM_STATES_TRELLIS_1_2];			//метрики лучших путей, входящих в каждую из вершин решетки треллиса
	int16_t arPrevPathMetrics[NUM_STATES_TRELLIS_1_2]; 	//метрики лучших путей на предыдущем шаге
	int16_t nMinPathMetric, nPathMetric;
	uint16_t step;
	SoManyWays ways;
	int8_t pDecoderIn[SIZE_OF_CODED_FRAME_TRELLIS];

	#ifndef TRELLIS_WITHOUT_INTERLEAVER
	//1. Деперемежение
	//NO: Кроме деперемежения 16-битных слов еще делается изменение порядка байтов в словах
	DeinterleaveTrellisData((int8_t*)pDataIn, pDecoderIn);
	#else
	memcpy(pDecoderIn,pDataIn,SIZE_OF_CODED_FRAME_TRELLIS*sizeof(int8_t));
	#endif

	memset(&ways,0,sizeof(SoManyWays));
	
	//2. Декодирование. Прямое прохождение по решетке
	for (n=0,step=0; n<SIZE_OF_CODED_FRAME_TRELLIS; n+=SIZE_OF_OUTPUT_TRELLIS,step++)
	{		
		//Выделяем 2 дибита
		int8_t nDibitPairVal;
		#ifdef APCO25_INTERLEAVER
		nDibitPairVal = (pDecoderIn[n]<<3) + (pDecoderIn[n+1]<<2) + (pDecoderIn[n+2]<<1) + (pDecoderIn[n+3]);
		#else
		nDibitPairVal = (pDecoderIn[n+1]<<3) + (pDecoderIn[n]<<2) + (pDecoderIn[n+3]<<1) + (pDecoderIn[n+2]);
		#endif

		memcpy(arPrevPathMetrics,arPathMetrics,NUM_STATES_TRELLIS_1_2*sizeof(int16_t));
		for(j=0; j<NUM_STATES_TRELLIS_1_2; j++) //j - номер обрабатываемой вершины решетки состояний
		{
			if(!step)	//На первом шаге все пути выходят из нулевого состояния
				nMinPathMetric = hammingWeight(TableTransMapAndTrellis1_2[0][j] ^ nDibitPairVal);	//в каждое состояние входит только одно ребро
			else
			{// Высчитываем минимальный вес среди всех путей, входящих в j-ую вершину
				nMinPathMetric = SHRT_MAX;
				for(i=0; i<NUM_STATES_TRELLIS_1_2; i++)
				{
					//Cчитаем суммарную метрику i-ого пути, продолженного в j-ую вершину
					nPathMetric = arPrevPathMetrics[i] + hammingWeight(TableTransMapAndTrellis1_2[i][j] ^ nDibitPairVal);
					
					if(nPathMetric<=nMinPathMetric) //определение ребра (с минимальной сумм. метрикой), входящего в j-ое состояние решетки
					{
						nMinPathMetric = nPathMetric;
						ways.arr[step][j] = i;  //фиксация выжившего пути
					}
				}
			}
			arPathMetrics[j] = nMinPathMetric;	//сохраняем метрику в массив, в котором хранятся метрики для следующего шага
		}
	}

	//3. Декодирование. Обратное прохождение по решетке
	//Формируем результат по наименьшей метрике
	int8_t nDibitVal = 0; //возвращаемся из нулевой вершины последней вертикали решетки к нулевой вершине певой вертикали решетки
	for (i=(SIZE_OF_CODED_FRAME_TRELLIS-SIZE_OF_OUTPUT_TRELLIS)/SIZE_OF_OUTPUT_TRELLIS; i>0; i--)
	{ //i меняется от 1 до 49, т.е. вертикаль ways.arr[0][0..3] не используется
		nDibitVal = ways.arr[i][nDibitVal];//номер вершины выжившего пути на предыдущем шаге
    //записываем результат декодирования в выходной массив
 	  pDataOut[i*SIZE_OF_STATE_TRELLIS_1_2-1] = nDibitVal&1;
	  pDataOut[i*SIZE_OF_STATE_TRELLIS_1_2-2] = (nDibitVal>>1)&1;
	}
	
	return arPathMetrics[0]; 	//в arPathMetrics[0] хранится метрика нулевого (правильного) пути (т.к. выхожу из нулевого состояния 
														//и в него же должен вернуться). она хранит число исправленных ошибок
}


//Декодер кода треллиса 3/4
//Параметры:
//  pDataIn -- указатель на входной массив закодированной последовательности, значение каждого элемента =0 или 1;
//  pDataOut -- указатель на выходной массив декодированной последовательности, значение каждого элемента =0 или 1;
int16_t trellisDec3_4(const int8_t * const pDataIn, int8_t * const pDataOut)
{
	int16_t i, n, j;	
	int16_t arPathMetrics[NUM_STATES_TRELLIS_3_4];			//метрики лучших путей, входящих в каждую из вершин решетки треллиса
	int16_t arPrevPathMetrics[NUM_STATES_TRELLIS_3_4]; 	//метрики лучших путей на предыдущем шаге
	int16_t nMinPathMetric, nPathMetric;
	uint16_t step;
	SoManyWays ways;
	int8_t pDecoderIn[SIZE_OF_CODED_FRAME_TRELLIS];

	#ifndef TRELLIS_WITHOUT_INTERLEAVER
	//1. Деперемежение
	//NO: Кроме деперемежения 16-битных слов еще делается изменение порядка байтов в словах
	DeinterleaveTrellisData((int8_t*)pDataIn, pDecoderIn);
	#else
	memcpy(pDecoderIn,pDataIn,SIZE_OF_CODED_FRAME_TRELLIS*sizeof(int8_t));
	#endif

	memset(&ways,0,sizeof(SoManyWays));
	
	//2. Декодирование. Прямое прохождение по решетке
	for (n=0,step=0; n<SIZE_OF_CODED_FRAME_TRELLIS; n+=SIZE_OF_OUTPUT_TRELLIS,step++)
	{		
		//Выделяем 2 дибита
		int8_t nDibitPairVal;
		#ifdef APCO25_INTERLEAVER
		nDibitPairVal = (pDecoderIn[n]<<3) + (pDecoderIn[n+1]<<2) + (pDecoderIn[n+2]<<1) + (pDecoderIn[n+3]);
		#else
		nDibitPairVal = (pDecoderIn[n+1]<<3) + (pDecoderIn[n]<<2) + (pDecoderIn[n+3]<<1) + (pDecoderIn[n+2]);
		#endif

		memcpy(arPrevPathMetrics, arPathMetrics, NUM_STATES_TRELLIS_3_4*sizeof(int16_t));
		for(j=0; j<NUM_STATES_TRELLIS_3_4; j++) //j- номер обрабатываемой вершины решетки состояний
		{
			if(!step)	//На первом шаге все пути выходят из нулевого состояния
				nMinPathMetric = hammingWeight(TableTransMapAndTrellis3_4[0][j] ^ nDibitPairVal);	//в каждое состояние входит только одно ребро
			else
			{// Высчитываем минимальный вес среди всех путей, входящих в j-ую вершину
				nMinPathMetric = SHRT_MAX;
				for(i=0; i<NUM_STATES_TRELLIS_3_4; i++)
				{
					//Cчитаем суммарную метрику i-ого пути, продолженного в j-ую вершину
					nPathMetric = arPrevPathMetrics[i] + hammingWeight(TableTransMapAndTrellis3_4[i][j] ^ nDibitPairVal);
					
					if(nPathMetric<=nMinPathMetric) //определение ребра (с минимальной сумм. метрикой), входящего в j-ое состояние решетки
					{
						nMinPathMetric = nPathMetric;
						ways.arr[step][j] = i;  //фиксация выжившего пути
					}
				}
			}
			arPathMetrics[j] = nMinPathMetric;	//сохраняем метрику в массив, в котором хранятся метрики для следующего шага
		}
	}

	//3. Декодирование. Обратное прохождение по решетке
	//Формируем результат по наименьшей метрике
	int8_t nTribitVal = 0; //возвращаемся из нулевой вершины последней вертикали решетки к нулевой вершине первой вертикали решетки
	for (i=(SIZE_OF_CODED_FRAME_TRELLIS-SIZE_OF_OUTPUT_TRELLIS)/SIZE_OF_OUTPUT_TRELLIS; i>0; i--)
	{ //i меняется от 1 до 49, т.е. вертикаль ways.arr[0][0..7] не используется
		nTribitVal = ways.arr[i][nTribitVal];//номер вершины выжившего пути на предыдущем шаге
    //записываем результат декодирования в выходной массив
 	  pDataOut[i*SIZE_OF_STATE_TRELLIS_3_4-1] = nTribitVal&1;
	  pDataOut[i*SIZE_OF_STATE_TRELLIS_3_4-2] = (nTribitVal>>1)&1;
		pDataOut[i*SIZE_OF_STATE_TRELLIS_3_4-3] = (nTribitVal>>2)&1;
	}
	
	return arPathMetrics[0]; 	//в arPathMetrics[0] хранится метрика нулевого (правильного) пути (т.к. выхожу из нулевого состояния 
														//и в него же должен вернуться). она хранит число исправленных ошибок
}


void InterleaveTrellisData(int8_t* pDataIn, int8_t* pDataOut)
{
	#ifdef APCO25_INTERLEAVER
	uint16_t cntDataOutDibits, cntDataOutBits;
	for(cntDataOutDibits=0,cntDataOutBits=0; cntDataOutDibits<SIZE_OF_CODED_FRAME_TRELLIS/2; cntDataOutDibits++,cntDataOutBits+=2)
	{
		pDataOut[cntDataOutBits] = pDataIn[(TableInterleave[cntDataOutDibits]<<1)+1];	
		pDataOut[cntDataOutBits+1] = pDataIn[(TableInterleave[cntDataOutDibits]<<1)];
	}
	#else
	uint8_t cntRaws, cntCloumns;
	int8_t InterleaveMatrix[NUM_RAWS_IN_INTERLEAVE_MATRIX][NUM_COLUMNS_IN_INTERLEAVE_MATRIX];

	//Заполняем по рядам матрицу перемежения
	uint16_t cntBits = 0;	
	for(cntRaws=0; cntRaws<NUM_RAWS_IN_INTERLEAVE_MATRIX; cntRaws++)
		for(cntCloumns=0; cntCloumns<NUM_COLUMNS_IN_INTERLEAVE_MATRIX; cntCloumns++)
			InterleaveMatrix[cntRaws][cntCloumns] = pDataIn[cntBits++];
	
	//Читаем матрицу перемежения по столбцам
	cntBits = 0;	
	for(cntCloumns=0; cntCloumns<NUM_COLUMNS_IN_INTERLEAVE_MATRIX; cntCloumns++)
		for(cntRaws=0; cntRaws<NUM_RAWS_IN_INTERLEAVE_MATRIX; cntRaws++)
			pDataOut[cntBits++] = InterleaveMatrix[cntRaws][cntCloumns];	
	#endif
}


void DeinterleaveTrellisData(int8_t* pDataIn, int8_t* pDataOut)
{
	#ifdef APCO25_INTERLEAVER
	uint16_t cntDataInBits, cntDataInDibits;
	for(cntDataInDibits=0,cntDataInBits=0; cntDataInDibits<SIZE_OF_CODED_FRAME_TRELLIS/2; cntDataInDibits++,cntDataInBits+=2)
	{
		pDataOut[(TableInterleave[cntDataInDibits]<<1)] = pDataIn[cntDataInBits];	
		pDataOut[(TableInterleave[cntDataInDibits]<<1)+1] = pDataIn[cntDataInBits+1];
	}
	#else
	uint8_t cntRaws, cntCloumns;
	int8_t InterleaveMatrix[NUM_RAWS_IN_INTERLEAVE_MATRIX][NUM_COLUMNS_IN_INTERLEAVE_MATRIX];

	//Заполняем матрицу перемежения по столбцам
	uint16_t cntBits = 0;	
	for(cntCloumns=0; cntCloumns<NUM_COLUMNS_IN_INTERLEAVE_MATRIX; cntCloumns++)
		for(cntRaws=0; cntRaws<NUM_RAWS_IN_INTERLEAVE_MATRIX; cntRaws++)
			InterleaveMatrix[cntRaws][cntCloumns] = pDataIn[cntBits++];
	
	//Читаем матрицу перемежения по рядам 
	cntBits = 0;	
	for(cntRaws=0; cntRaws<NUM_RAWS_IN_INTERLEAVE_MATRIX; cntRaws++)
		for(cntCloumns=0; cntCloumns<NUM_COLUMNS_IN_INTERLEAVE_MATRIX; cntCloumns++)
			pDataOut[cntBits++] = InterleaveMatrix[cntRaws][cntCloumns];
	#endif
}


#ifdef DEBUG_TRELLIS_DEC_3_4_SIRIUS_OPTIMIZATION
//Декодер кода треллиса 3/4
//Траектория выбирается в соответствии с принципом наименьшей суммы 
//ошибок по всем возможным траекториям, по которым нач сост=0, конечн сост=0
//Входные параметры:
//  pDataIn -- массив принятых битов (размер должен быть 196)
//  pDataOut -- массив исправленных битов (размер должен быть 144) 
int16_t trellisDec3_4_Sirius(const int8_t * const pDataIn, int8_t * const pDataOut)
{
	int16_t i,j,n;
	int8_t nDibitPairVal;
	uint8_t arTribitsOut[SIZE_OF_CODED_FRAME_TRELLIS/SIZE_OF_OUTPUT_TRELLIS] = {0};
	int16_t arPathMetrics[NUM_STATES_TRELLIS_3_4];			//метрики лучших путей, входящих в каждую из вершин решетки треллиса
	int16_t arPrevPathMetrics[NUM_STATES_TRELLIS_3_4]; 	//метрики лучших путей на предыдущем шаге
	int8_t arrayOptimWays[100];	//специальный массив, в который по определенному правилу на каждом шаге записываются 
															//пути с наименьшими метриками, входящие в каждое из состояний решетки треллиса (аналог arSoManyWays[])
	int16_t nPrevPathMetric, nPathMetric;
	
	uint8_t maskOfCandidateWays = 1, maskOfNextCandidateWays = 0;
	
	int8_t pDecoderIn[SIZE_OF_CODED_FRAME_TRELLIS];

	memset(arPrevPathMetrics,0,NUM_STATES_TRELLIS_3_4*sizeof(int16_t));
	memset(arrayOptimWays,0,100*sizeof(int8_t));

	#ifndef TRELLIS_WITHOUT_INTERLEAVER
	//Деперемежение	
	for(n=0,j=0; n<SIZE_OF_CODED_FRAME_TRELLIS/2; n++,j+=2)
	{
		pDecoderIn[(TableInterleave[n]<<1)+1] = pDataIn[j];	
		pDecoderIn[(TableInterleave[n]<<1)] = pDataIn[j+1];
	}
	#else
	memcpy(pDecoderIn,pDataIn,SIZE_OF_CODED_FRAME_TRELLIS*sizeof(int8_t));	
	#endif

	//Строим множество траекторий в массиве array,
	//используя значения кандидатов Cand, достижимых на предыдущем шаге,
	//и кол-во ошибок errcount[i] в принятых данных для каждого из состояний i.
	for(n=0; n<SIZE_OF_CODED_FRAME_TRELLIS; n+=SIZE_OF_OUTPUT_TRELLIS)
	{	  
		//Выделяем 2 дибита
		int8_t nDibitPairVal;
		#ifndef TRELLIS_WITHOUT_INTERLEAVER
		nDibitPairVal = (pDecoderIn[n]<<3) + (pDecoderIn[n+1]<<2) + (pDecoderIn[n+2]<<1) + (pDecoderIn[n+3]);
		#else		
		nDibitPairVal = (pDecoderIn[n+1]<<3) + (pDecoderIn[n]<<2) + (pDecoderIn[n+3]<<1) + (pDecoderIn[n+2]);
		#endif
		
		memset(arPathMetrics,SHRT_MAX,NUM_STATES_TRELLIS_3_4);
		
		if(maskOfCandidateWays)
		{ 
			register int16_t n2;
			//n2 = 8*([n/4]+1)
			n2=n>>2;
			n2++; 
			n2=n2<<3;
			
			//Находим для  всех достижимых на данном шаге кандидатов 
			//кол-во ошибок при переходе в следующие возможные состояния
			for (i=0; i<NUM_STATES_TRELLIS_3_4; i++)	//i - номер вершины решетки состояний, из которой исходит путь
				if (maskOfCandidateWays & (1<<i))
				{	
					nPrevPathMetric = arPrevPathMetrics[i];
					for (j=0; j<NUM_STATES_TRELLIS_3_4; j++)
					{
						//Cчитаем метрику пути, проходящего из i-й вершины в j-ю
						nPathMetric = hammingWeight(TableTransMapAndTrellis3_4[i][j] ^ nDibitPairVal);	//метрика последнего перехода
						
						//Если переход между предыдущим и текущим состояними имеет ошибку менее 3 бит (максимум - 4),
						//только тогда его рассматриваем как кандидата для продления пути. Типа, оптимизация
						if (nPathMetric<3)	
						{
							maskOfNextCandidateWays |= (1<<j);	//отмечаем текущее состояние, как вероятное
							
							nPathMetric += nPrevPathMetric;		//Суммируем метрику со всеми предыдущими переходами для данного пути
							if (nPathMetric < arPathMetrics[j])
							{
								arPathMetrics[j] = nPathMetric;	//В arPathMetrics[j] записываем минимальную метрику среди всех путей, входящих в j-ю вершину
								//#setvalue(arrayOptimWays,n2+j,i);					//Записываем в array[] текущий путь как оптимальный
							}
						}
					}
				}
		}
		maskOfCandidateWays = maskOfNextCandidateWays;
		if (n<192)
			memcpy(arPrevPathMetrics,arPathMetrics,8);//ошибка( if (n<192))
	}

	//Восстанавливаем траекторию, самую близкую к верной
	//Обратное прохождение решетки
	//#i = buildCorrection3_4(pDecoderIn, arTribitsOut, maskOfCandidateWays, arPrevPathMetrics, arrayOptimWays); 
	
	//записываем результат декодирования в выходной массив
	uint8_t cntTribits = 0;
	uint8_t nTribitVal;
	for (n=0; n<SIZE_OF_UNCODED_FRAME_TRELLIS_3_4/SIZE_OF_STATE_TRELLIS_3_4; n++)
	{
	  nTribitVal = arTribitsOut[n];
	  pDataOut[(cntTribits)+2] = (nTribitVal)&1;
	  pDataOut[(cntTribits)+1] = (nTribitVal>>1)&1;
	  pDataOut[(cntTribits)] = (nTribitVal>>2)&1;
	  cntTribits += SIZE_OF_STATE_TRELLIS_3_4;
	}

	//возвращаем количество исправленных ошибок
	return i;
}
#endif





#ifdef DEBUG_TRELLIS_CODER
void TestTrellisCoder1_2()
{
	//Исходное сообщение
	const int8_t MessageForTransfer[SIZE_OF_UNCODED_FRAME_TRELLIS_1_2] = 		
	{0,1,0,0,1,0,0,0, 0,0,0,0,1,0,0,0, 0,1,0,0,0,0,0,1, 0,0,0,0,0,0,1,0,
	 1,0,1,0,0,1,0,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,0,0,0,
	 0,0,0,0,0,1,1,1, 1,0,0,0,0,0,0,0, 1,0,1,0,1,0,1,0, 1,0,0,0,1,0,0,1};
	
	//Сообщение после преобразований кодирования и декодирования
	int8_t MessageTransfered[SIZE_OF_UNCODED_FRAME_TRELLIS_1_2];
	memset(MessageTransfered,0,SIZE_OF_UNCODED_FRAME_TRELLIS_1_2);
	
	//Закодированное сообщение
	int8_t EncodedData[SIZE_OF_CODED_FRAME_TRELLIS];
	
	//Кодируем
	trellisEnc1_2(MessageForTransfer, EncodedData);

	//Вносим канальные ошибки
	EncodedData[1] ^= 1;
	EncodedData[3] ^= 1; //EncodedData[4] ^= 1;
	EncodedData[10] ^= 1; EncodedData[18] ^= 1;
	EncodedData[48] ^= 1; EncodedData[52] ^= 1; EncodedData[56] ^= 1;
	EncodedData[94] ^= 1;

	//Декодируем
	int16_t numDetectedErrors = trellisDec1_2(EncodedData, MessageTransfered);
	
	//Сравниваем сообщение до и после кодирования
	int8_t CmpResult[SIZE_OF_UNCODED_FRAME_TRELLIS_1_2];
	uint8_t numErrors = 0;
	for(int8_t i=0; i<SIZE_OF_UNCODED_FRAME_TRELLIS_1_2; i++)
	{
		CmpResult[i] = (MessageForTransfer[i]==MessageTransfered[i] ? 0 : 1);
		if(CmpResult[i])
			numErrors++;
	}
}	

void TestTrellisCoder3_4()
{
	//Исходное сообщение
	const int8_t MessageForTransfer[SIZE_OF_UNCODED_FRAME_TRELLIS_3_4] = 		
	{0,1,0,0,1,0,0,0, 0,0,0,0,1,0,0,0, 0,1,0,0,0,0,0,1, 0,0,0,0,0,0,1,0,
	 1,0,1,0,0,1,0,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,0,0,0,
	 0,1,0,0,0,1,1,1, 1,0,0,0,0,0,0,0, 1,0,1,0,1,0,1,0, 1,0,0,0,1,0,0,1,
	 1,0,1,0,0,1,1,1, 1,0,0,0,0,0,1,1, 1,1,1,1,1,1,1,1, 0,1,0,0,1,0,0,1,
	 0,1,1,0,0,0,1,0, 0,0,1,1,1,0,0,1};
	
	//Сообщение после преобразований кодирования и декодирования
	int8_t MessageTransfered[SIZE_OF_UNCODED_FRAME_TRELLIS_3_4];
	memset(MessageTransfered,0,SIZE_OF_UNCODED_FRAME_TRELLIS_3_4);
	
	//Закодированное сообщение
	int8_t EncodedData[SIZE_OF_CODED_FRAME_TRELLIS];
	
	//Кодируем
	trellisEnc3_4(MessageForTransfer, EncodedData);

	//Вносим канальные ошибки
	
	EncodedData[1] ^= 1;
	EncodedData[3] ^= 1; EncodedData[4] ^= 1;
	//EncodedData[10] ^= 1; EncodedData[18] ^= 1;
	//EncodedData[48] ^= 1; EncodedData[56] ^= 1;
	EncodedData[94] ^= 1;
	EncodedData[126] ^= 1;

	//Декодируем
	int16_t numDetectedErrors = trellisDec3_4(EncodedData, MessageTransfered);
	
	//Сравниваем сообщение до и после кодирования
	int8_t CmpResult[SIZE_OF_UNCODED_FRAME_TRELLIS_3_4];
	uint16_t numErrors = 0;
	for(int16_t i=0; i<SIZE_OF_UNCODED_FRAME_TRELLIS_3_4; i++)
	{
		CmpResult[i] = (MessageForTransfer[i]==MessageTransfered[i] ? 0 : 1);
		if(CmpResult[i])
			numErrors++;
	}
}	

#endif

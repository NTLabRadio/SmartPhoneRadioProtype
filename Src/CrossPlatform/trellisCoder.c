#include "trellisCoder.h"

// Таблица для решетчатого кодирования 1/2 (Trellis Encoder State Transition Tables)
const int16_t table1_2[SIZE_OF_TRELLIS_FSM_STATE][SIZE_OF_TRELLIS_FSM_STATE] =
						{  0,15,12, 3,
						   4,11, 8, 7,
						  13, 2, 1,14,
						   9, 6, 5,10};

// Таблица для перемежения дибитов (Interleave Table)
const int16_t interleaveTable[SIZE_OF_CODED_FRAME/2]=
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
						   
//Constellation to Dibit Pair Mapping
//Кодер сопоставляет числам 0-15 (можно их назвать номерами точек созвездия) пары дибитов в соответствии со следующей таблицей
static int16_t ConstelToDibitPair[SIZE_OF_TRELLIS_OUTPUT_CONSTELLATION][2]=
						{	{0,2}, {2,2}, {1,3}, {3,3}, {3,2}, {1,2}, {2,3}, {0,3},
							{3,1}, {1,1},	{2,0}, {0,0},	{0,1}, {2,1},	{1,0} ,{3,0} };
								 
//Функция кодирования данных кодом треллиса с эффективностью 1/2
//Параметры:
//	message -- указатель на входной массив данных для кодирования, значение каждого элемента =0 или 1;
//	outMess -- указатель на выходной массив данных после кодирования, значение каждого элемента =0 или 1;
void trellisEnc1_2(int16_t * message, int16_t * out)
{
	int16_t i,k,j;
	int16_t currentcode;
	int16_t FSM;		//состояние автомата (Finite State Machine)
	int16_t out2dibit;
	int16_t outtmp;
	int16_t outMess[SIZE_OF_CODED_FRAME];
	memset(outMess,0,SIZE_OF_CODED_FRAME*sizeof(int16_t));
	
	FSM = 0;	//начальное состояние автомата - нулевое
	for(i=0,k=0; i<SIZE_OF_UNCODED_FRAME; k+=SIZE_OF_TRELLIS_FSM_STATE,i+=2)
	{			
		currentcode=(message[i]<<1) + (message[i+1]);	//значение входного дибита
		
		out2dibit=table1_2[FSM][currentcode];					//Входному значению автомата кодирования и его текущему состоянию
																									//ставится в соответствие число 0-15
		
		FSM=currentcode;															//Изменяем текущее состояние кодера на значение задержанного входного дибита

		//Число 0-15 преобразуем в пару дибитов		
		outtmp=ConstelToDibitPair[out2dibit][0];			//первый дибит
		outMess[k]=(outtmp) & 1;
		outMess[k+1]=(outtmp>>1) & 1;
		
		outtmp=ConstelToDibitPair[out2dibit][1];			//второй дибит
		outMess[k+2]=(outtmp) & 1;
		outMess[k+3]=(outtmp>>1) & 1;
	}
	
	//В конце к данным добавляем 2 нулевых flush-бита
	out2dibit=table1_2[FSM][0];			//Последнему (нулевому) значению автомата кодирования и его текущему состоянию
																	//ставим в соответствие число 0-15
	
	//Число 0-15 преобразуем в пару дибитов
	outtmp=ConstelToDibitPair[out2dibit][0];
	outMess[k]=(outtmp) & 1;
	outMess[k+1]=(outtmp>>1) & 1;
	outtmp=ConstelToDibitPair[out2dibit][1];
	outMess[k+2]=(outtmp) & 1;
	outMess[k+3]=(outtmp>>1) & 1;

	#ifndef TRELLIS_WITHOUT_INTERLEAVER
	//Перемежение
	for(k=0,j=0; k<SIZE_OF_CODED_FRAME/2; k++,j+=2)
	{
		out[j]=outMess[(interleaveTable[k]<<1)+1];	
		out[j+1]=outMess[(interleaveTable[k]<<1)];
	}
	#else
	memcpy(out,outMess,SIZE_OF_CODED_FRAME*sizeof(int16_t));
	#endif
}

//Решетка переходов для кода треллиса 1/2
const int16_t TrelStruct[SIZE_OF_TRELLIS_FSM_STATE][SIZE_OF_TRELLIS_FSM_STATE]=
{
    {  0x02, 0x0C, 0x01, 0x0F },
    {  0x0E, 0x00, 0x0D, 0x03 },
    {  0x09, 0x07, 0x0A, 0x04 },
    {  0x05, 0x0B, 0x06, 0x08 }
};

//расчёт метрики для ребра
uint16_t countMetrics(uint16_t Data)
{
	int16_t i,iTmp,cnt=0;
	iTmp = Data&0xFF;
	for(i=0;i<8;i++)
		cnt+=(iTmp>>i)&1;
    
/*	//подсчёт единиц за один такт процессора (работает хорошо при -o3 и в отсутствие оптимизации)
    asm("        MOV *(#_TempCntMet), AC0"); //гружу в AC0 байт, число единиц которого надо посчитать
    asm("        MOV #255, AC1"); //маска на байт
    asm("        BCNT AC0, AC1, TC1, T0"); //подсчёт единиц в регистре AC0; число единиц теперь помещается в регистр T0
    asm("        MOV T0, *(#_TempCntMet)"); //результат записываю в TempCntMet
*/
    return cnt;
}

typedef struct SoManyWays
{
	int16_t arr[SIZE_OF_CODED_FRAME/4][4];//массив выживших путей - решетка  
} *pSoManyWays;


//Декодер кода треллиса 1/2
//Параметры:
//  in -- указатель на входной массив закодированной последовательности, значение каждого элемента =0 или 1;
//  out -- указатель на выходной массив декодированной последовательности, значение каждого элемента =0 или 1;
//WARNING! Для выходного массива, на который указывает out, должна быть предвыделена память в размере SIZE_OF_CODED_FRAME элементов
//				 (а не SIZE_OF_UNCODED_FRAME!), поскольку массив используется для промежуточных вычислений
//WARNING! Входной массив in также используется для промежуточных вычисления и для него должна быть выделена память
//					на (SIZE_OF_CODED_FRAME+SIZE_OF_TRELLIS_FSM_STATE) элементов (а не SIZE_OF_CODED_FRAME!)
//WARNING! Значения элементов входного массива в результе выполнения функции не сохраняются!
int16_t trellisDec1_2(int16_t *in, int16_t *out)
{
	register int16_t i, b,n,j;	
	int16_t sumMetrics[SIZE_OF_TRELLIS_FSM_STATE]; //суммарные метрики выживших путей
	int16_t a[SIZE_OF_TRELLIS_FSM_STATE];//метрики каждого из четырех выживших пути
	int16_t min, tmp,step;  
	pSoManyWays way;

	#ifndef TRELLIS_WITHOUT_INTERLEAVER
	//1. Деперемежение
	//NO: Кроме деперемежения 16-битных слов еще делается изменение порядка байтов в словах
	for(n=0,j=0; n<SIZE_OF_CODED_FRAME/2; n++,j+=2)
	{
		out[(interleaveTable[n]<<1)]=in[j];	
		out[(interleaveTable[n]<<1)+1]=in[j+1];
	}
	#else
	memcpy(out,in,SIZE_OF_CODED_FRAME*sizeof(int16_t));
	#endif

	way = (pSoManyWays) in;	
	memset(in,0,(SIZE_OF_CODED_FRAME+SIZE_OF_TRELLIS_FSM_STATE)*sizeof(int16_t));
	
	//2. Декодирование. Прямое прохождение по решетке
	for (n=0,step=0; n<SIZE_OF_CODED_FRAME-1; n+=SIZE_OF_TRELLIS_FSM_STATE,step++)
	{
		//выделяем 2 дибита
		#ifndef TRELLIS_WITHOUT_INTERLEAVER
		b=(out[n]<<3)+(out[n+1]<<2)+(out[n+2]<<1)+(out[n+3]);
		#else
		b=(out[n+1]<<3)+(out[n]<<2)+(out[n+3]<<1)+(out[n+2]);
		#endif
		
	  {//строим решетку с каждым из четырех битов
			memcpy(sumMetrics,a,SIZE_OF_TRELLIS_FSM_STATE*sizeof(int16_t));
	    for(j=0; j<SIZE_OF_TRELLIS_FSM_STATE; j++) //j- номер обрабатываемой вершины решетки состояний
	    {   //стартуем из нулевого состояния
				min = countMetrics(TrelStruct[0][j]^b);	//метрику для каждого из 16-ти новых ребер
				if(step)
				{// высчитываем минимальный вес из четырех входных путей
					min=100;
		      for(i=0; i<SIZE_OF_TRELLIS_FSM_STATE; i++)
	        {
						tmp=(sumMetrics[i]+ countMetrics(TrelStruct[i][j]^b));//считаем суммарную метрику, которая входит в jj-ую вершину, из ii-того пути
		        if (tmp<=min) //определение ребра  (с минимальной сумм. метрикой), входящего в p-ое состояние решетки для каждого
		        {
							min=tmp;
	            way->arr[step][j]=i;  //фиксация выжившего пути  s=1..49
	          }
	        }
				}
	      a[j] = min;//сохраняем метрику в масив, в котором хранятся метрики для следующего шага
	    }
		}
	}

	//3. Декодирование. Обратное прохождение по решетке. 
	//Формируем результат по наименьшей метрике
	b=0; //возвращаемся из нулевой вершины последней вертикали решетки к нулевой вершине певой вертикали решетки
	for (i=SIZE_OF_CODED_FRAME/SIZE_OF_TRELLIS_FSM_STATE; i>0; i--)
	{ //i меняется от 1 до 49, т.е. вертикаль way[0][0..3] не используется
		b=way->arr[i][b];//номер вершины выжившего пути на предидущем шаге
    //записываем результат декодирования в выходной массив
 	  out[i*2-1]=b&1;
	  out[i*2-2]=(b>>1)&1;
	}
	memcpy(in,out,SIZE_OF_UNCODED_FRAME*sizeof(int16_t));
	
	return a[0]; 	//в a[0] хранится метрика нулевого (правильного) пути (т.к. выхожу из нулевого состояния и в него же должен вернуться) 
								//она хранит количество ошибок по нулевому пути
}



#ifdef DEBUG_TRELLIS_CODER
void TestTrellisCoder()
{
	//Сообщение для кодирования
	const int16_t MessageForEncode[SIZE_OF_CODED_FRAME] = 		
	{0,1,0,0,1,0,0,0, 0,0,0,0,1,0,0,0, 0,1,0,0,0,0,0,1, 0,0,0,0,0,0,1,0,
	 1,0,1,0,0,1,0,1, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 1,1,1,1,1,0,0,0,
	 0,0,0,0,0,1,1,1, 1,0,0,0,0,0,0,0, 1,0,1,0,1,0,1,0, 1,0,0,0,1,0,0,1,
	 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0};
	
	int16_t MessageDataDest[SIZE_OF_CODED_FRAME];
	memset(MessageDataDest,0,SIZE_OF_CODED_FRAME);		
	
	int16_t EncodedData[SIZE_OF_CODED_FRAME+SIZE_OF_TRELLIS_FSM_STATE];
	
	//Кодируем
	trellisEnc1_2((int16_t*)MessageForEncode, EncodedData);

	//Вносим ошибки
	EncodedData[4] ^= 1; EncodedData[3] ^= 1;
	EncodedData[10] ^= 1; EncodedData[18] ^= 1;
	EncodedData[48] ^= 1; EncodedData[52] ^= 1; EncodedData[56] ^= 1;

	//Декодируем
	int16_t numDetectedErrors = trellisDec1_2(EncodedData, MessageDataDest);
	
	//Сравниваем сообщение до и после кодирования
	int8_t CmpResult[SIZE_OF_UNCODED_FRAME];
	uint8_t numErrors = 0;
	for(int8_t i=0; i<SIZE_OF_UNCODED_FRAME; i++)
	{
		CmpResult[i] = (MessageForEncode[i]==MessageDataDest[i] ? 0 : 1);
		if(CmpResult[i])
			numErrors++;
	}
}	
#endif

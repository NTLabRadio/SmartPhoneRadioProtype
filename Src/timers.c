#include "timers.h"

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;


// This routine reads back two timers and combines the result into one 32 bit value
// ticked at 1uS. Using ReadHighPrecisionCounter and ClearHighPrecisionCounter the combination 
// of these timers create a 32 bit hardware counter being ticked at 1uS. These are used for 
// delays in the code. The reality is a counter which wraps at 1hr 11mins.
uint32_t ReadHighPrecisionCounter(void)
{
	uint32_t uCountResult = 0;
	
	uCountResult = __HAL_TIM_GetCounter(&htim3) << 16;
	uCountResult = uCountResult | __HAL_TIM_GetCounter(&htim2);
	
	return uCountResult;
}


void ClearHighPrecisionCounter(void)
{
	__HAL_TIM_SetCounter(&htim2,0);
	__HAL_TIM_SetCounter(&htim3,0);
	
}


void WaitTimeMCS(uint32_t lTimeMCS)
{
	ClearHighPrecisionCounter();
	while(ReadHighPrecisionCounter() < lTimeMCS);
}


uint32_t ReadCMX7262TimerCounter(void)
{
	return __HAL_TIM_GetCounter(&htim5);
}


void StartPeriphTimers()
{
	/* Стартуем высокоточный таймер (TIM2+TIM3) для контроля временных задержек низкоуровневых функций */
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);
	
	/* Стартуем таймер для контроля процессов управления микросхемой CMX7262 */
	HAL_TIM_Base_Start(&htim5);
}

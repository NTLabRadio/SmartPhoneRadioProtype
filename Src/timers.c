#include "timers.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim5;

/* Таймеры TIM2 и TIM3 реализуют высокоточный (точность - 1мкс) 32-битный таймер для оценки временных
интервалов в реализации низкоуровневых функций (передача байта по SPI, UART и т.п.) */

/* TIM2 init function */
void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = (uint16_t) ((SystemCoreClock) / 1e6) - 1;		//такт таймера - в 1 мкс
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim2);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

	//Таймер используется как master в каскадной цепочке таймеров для реализации 32-битного таймера
	//Слейвом выступает TIM3. Формируем для него сигнал тригера по истечению периода
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;								
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

}


/* TIM3 init function */
void MX_TIM3_Init(void)
{

  TIM_SlaveConfigTypeDef sSlaveConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim3);

	//Таймер используется как slave в каскадной цепочке таймеров для реализации 32-битного таймера
	//По переполнению счетчика master-таймера (TIM2) инкрементируется счетчик данного slave-таймера
	//Прерывание ITR1 используется как внешний тригер для инкрементирования значения счетчика
	sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;	
  sSlaveConfig.InputTrigger = TIM_TS_ITR1;
  HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

}


/* TIM5 init function */
void MX_TIM5_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim5.Instance = TIM5;
  htim5.Init.Prescaler = (uint16_t) ((SystemCoreClock)/1e5 - 1);	//такт таймера - в 10 мкс	
	htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 65535;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim5);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig);

}


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

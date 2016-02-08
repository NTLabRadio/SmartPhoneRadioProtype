/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"
#include "globals.h"
#include "cc1120.h"
#include "cmx7262.h"
#include "spi_periphery.h"
#include "timers.h"
#include "uart_intermodule.h"


/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

CMX7262_TypeDef  pCmx7262;

/* Private variables ---------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;

extern SPI_HandleTypeDef hspi1;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);

uint32_t cntTimerInit7262;

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
/* USER CODE END PFP */

int main(void)
{	
  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
  MX_TIM2_Init();
  MX_TIM3_Init();
	MX_TIM5_Init();

	/* Стартуем высокоточный таймер (TIM2+TIM3) для низкоуровневых функций */
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);
	
	/* Стартуем таймер для контроля процессов управления CMX7262 */
	HAL_TIM_Base_Start(&htim5);
	
  /* Initialize all configured peripherals */
	MX_GPIO_Init();
  MX_DMA_Init();
	
  MX_SPI1_Init();
  MX_USART1_UART_Init();

	UART_Receive_Pck_Wait(&huart1,NULL);
	
	
	#ifdef DEBUG_CHECK_PERIPH_MODULES_ON_STARTUP	//Проверка работоспособности периферийных модулей
	CC1120_CheckModule(&hspi1);
	CMX7262_CheckModule(&hspi1);
	#endif
	
	#ifdef DEBUG_CALCULATE_CMX7262_INIT_TIME	//Измерение времени инициализации CMX7262
	//Инициализация CMX7262 - долговременный процесс. Поэтому измененям точность таймера 
	//во избежание переполнения 16-битного счетчика таймера
	htim5.Init.Prescaler = (uint16_t) ((SystemCoreClock)/1e3 - 1);	//такт - в 1 мс
  HAL_TIM_Base_Init(&htim5);	
	
	cntTimerInit7262 = ReadCMX7262TimerCounter();
	#endif
	
	//Инициализация CMX7262: загрузка образа в память, начальная настройка
	CMX7262_Init(&pCmx7262, &hspi1);
	
	#ifdef DEBUG_CALCULATE_CMX7262_INIT_TIME	//Измерение времени инициализации CMX7262
	cntTimerInit7262 = ReadCMX7262TimerCounter() - cntTimerInit7262;
	printf("Time of CMX7262_Init() exec: %d s \r\n", cntTimerInit7262/1000);
	//Возвращаем исходные настройки таймера (точность - 10 мкс)
	MX_TIM5_Init();
	#endif
	
	//Перевод CMX7262 в режим Idle
	CMX7262_Idle(&pCmx7262);
	
	//Перевод CMX7262 в рабочий режим
	#ifdef TEST_CMX7262_ENCDEC_AUDIO2AUDIO_MODE
	CMX7262_EncodeDecode_Audio(&pCmx7262);	
	#else
	#ifdef TEST_CMX7262_AUDIO_TESTMODE
	CMX7262_Test_AudioOut(&pCmx7262);
	#else
	CMX7262_Encode(&pCmx7262);
	#endif
	#endif	
	
  /* Infinite loop */
  while (1)
  {
		

  }

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);		//прерывание - каждую мс

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


/* USART1 init function */
void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 57600;										// скорость передачи данных
  huart1.Init.WordLength = UART_WORDLENGTH_8B;		// размер кадра - 8 бит
  huart1.Init.StopBits = UART_STOPBITS_1;					// количество стоп битов - 1
  huart1.Init.Parity = UART_PARITY_NONE;					// контроль четности отключен
  huart1.Init.Mode = UART_MODE_TX_RX;							// режим работы: Tx+Rx
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;		// выключение аппаратного контроля потока
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);

}

/** 
  * Enable DMA controller clock
  */
void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

}

/** Pinout Configuration
*/
void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __GPIOA_CLK_ENABLE();
	
	__GPIOE_CLK_ENABLE();	
	HAL_GPIO_MspInit();

}


#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

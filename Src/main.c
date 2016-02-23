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

/* USER CODE BEGIN Includes */
#include "globals.h"
#include "cc1120.h"
#include "cmx7262.h"
#include "mathfuncs.h"
#include "RadioModule.h"
#include "SPIMMessage.h"
#include "uart_intermodule.h"
#include "version.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
extern UART_InitTypeDef DefaultUARTParams;

extern en_UARTstates UARTstate;

extern uint8_t pUARTRxSLIPPack[];
extern uint16_t nSizeSLIPPack;

CMX7262_TypeDef  g_CMX7262Struct;

uint8_t g_flCMX7262_IRQ_CHECKED = FALSE;

#define MAX_SIZE_OF_DATA_FROM_CMX7262 (512)
uint8_t pDataFromCMX7262[MAX_SIZE_OF_DATA_FROM_CMX7262];

#define MAX_SIZE_OF_DATA_TO_CMX7262 (4096)
uint8_t pDataToCMX7262[MAX_SIZE_OF_DATA_TO_CMX7262];
uint16_t nLengthDataToCMX7262 = 0;

#ifdef DEBUG_CMX7262_CNT_TX_AUDIO_BUF
uint16_t cntCMX7262TxAudioBuf = 0;
#endif


//������ ��� ��������� �������� ��������� SPIM-���������
SPIMMessage*	pObjSPIMmsgRcvd;
//������ ��� ������������ ��������� SPIM-��������� ��� ��������
//SPIMMessage*	pObjSPIMmsgToSend;

//����� ���� ������ �������������� ������ �� ���� ���������� � �������� �����������
RadioModule objRadioModule;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_TIM5_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void ProcessDataFromExtDev(void);
void FormAndSendAnswerToExtDev(SPIMMessage* SPIMmsgRcvd);
void FormAnswerToExtDev(SPIMMessage* SPIMCmdRcvd, SPIMMessage* SPIMBackCmdToSend);
void FormBodyOfAnswerToExtDev(SPIMMessage* SPIMCmdRcvd, uint8_t* pBodyData, uint8_t& bodySize);
void FormCurrentParamAnswer(SPIMMessage* SPIMCmdRcvd, uint8_t* pBodyData, uint8_t& bodySize);

void ProcessCMX7262State(void);

void SPIMInit(void);
void SPIMDeInit(void);

#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_EXTSIGNAL_FROM_UART
void ProcessAudioDataFromUART(void);
#endif

uint16_t GetARMSoftVer();
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  MX_USART1_UART_Init();

  /* USER CODE BEGIN 2 */
	// ������������� CS ������������ ��������� � ������� ���������
	CC1120_CSN_HIGH();
	CMX7262_CSN_HIGH();
	
	#ifdef DEBUG_USE_TL_LINES_FOR_CHECK_CMX7262_EVENTS
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
	#endif
	
	/* �������� ������������ ������ (TIM2+TIM3) ��� �������� ��������� �������� �������������� ������� */
	HAL_TIM_Base_Start(&htim2);
	HAL_TIM_Base_Start(&htim3);
	
	/* �������� ������ ��� �������� ��������� ���������� ����������� CMX7262 */
	HAL_TIM_Base_Start(&htim5);

	// �������������� ������ �� UART
	UART_InitInterface(&huart1);

	//�������������� ���, ��� ���������� ��� ��������� ������������� ������ SPIM
	SPIMInit();


	#ifdef DEBUG_CHECK_PERIPH_MODULES_ON_STARTUP	//�������� ����������������� ������������ �������
	CC1120_CheckModule(&hspi1);
	CMX7262_CheckModule(&hspi1);
	#endif
	

	//������������� CMX7262: �������� ������ � ������, ��������� ���������
	CMX7262_Init(&g_CMX7262Struct, &hspi1);


	//������� CMX7262 � ����� Idle
	CMX7262_Idle(&g_CMX7262Struct);

	//������� CMX7262 � ������� �����
	#ifdef TEST_CMX7262_ENCDEC_AUDIO2AUDIO_MODE
	CMX7262_EncodeDecode_Audio(&pCmx7262);	
	#endif

	#ifdef TEST_CMX7262_AUDIO_TESTMODE
	CMX7262_Test_AudioOut(&g_CMX7262Struct);
	#endif
	
	#ifdef TEST_CMX7262_ENCDEC_AUDIO2CBUS_MODE
	CMX7262_EncodeDecode_Audio2CBUS(&g_CMX7262Struct);
	#endif
	
	#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_MODE
		#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_INTERNAL_SIN
		CMX7262_EncodeDecode_CBUS2Audio(&g_CMX7262Struct);
		
		//��������� ����� ��������� �������� 1���
		FillBufByToneSignal((int16_t*)pDataToCMX7262,CMX7262_AUDIOFRAME_SIZE_SAMPLES,CMX7262_FREQ_SAMPLING,1000);
		CMX7262_TxFIFO_Audio(&g_CMX7262Struct,(uint8_t *)&pDataToCMX7262[0]);
		#endif
	#endif
	
	#ifdef TEST_CMX7262_ENC_MODE
	CMX7262_Encode(&g_CMX7262Struct);
	#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//���� �� UART ������� ������
		if(UARTstate==UART_DATA_RX_NEED_TO_PROCESS)
		{
			#ifdef DEBUG_USE_TL_LINES_FOR_CHECK_CMX7262_EVENTS
			HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_1);
			#endif
			
			//������������ ��
			ProcessDataFromExtDev();
			
			//���������, ��� ������ ����������
			UARTstate = UART_IDLE;
		}
		
		//���� ���� ���������� �� CMX7262
		if(g_flCMX7262_IRQ_CHECKED)
		{
			//������������ ����������: ���������, ��� ����� CMX7262
			CMX7262_IRQ(&g_CMX7262Struct);
			//���������� ����, ����� ���������� ��������� ����������
			g_flCMX7262_IRQ_CHECKED = FALSE;
		}
		
		//��������� ��������� ������ CMX7262: ��������/�����/����
		ProcessCMX7262State();
		
		#ifdef DEBUG_PERIODICALLY_READ_CMX7262_STATUS
		uint16_t uStatusRegValue = 0;
		// Read the status register into a shadow register.
		CBUS_Read16 (IRQ_STATUS_REG,&uStatusRegValue,1,g_CMX7262Struct.uInterface);
		printf("CMX7262 Status Reg=%x\n",uStatusRegValue);
		#endif
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

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

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}


/* SPI1 init function */
void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;						// ����� ������: ������������� full duplex
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;								// ������ ������ - 8 ���
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;									// ������������� �� ������� ������
  hspi1.Init.NSS = SPI_NSS_SOFT;													// ����������� CS (���������� (SPI_NSS_HARD_OUTPUT) �� �������, ��� �������������)
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32 ;	//������������ ������� SPI: 64���/32 = 2 ���
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;									// ������� ��� - ������
  hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;// CRC �� �����������
  hspi1.Init.CRCPolynomial = 10;
  HAL_SPI_Init(&hspi1);

}


/* ������� TIM2 � TIM3 ��������� ������������ (�������� - 1���) 32-������ ������ ��� ������ ���������
���������� � ���������� �������������� ������� (�������� ����� �� SPI, UART � �.�.) */

/* TIM2 init function */
void MX_TIM2_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = (uint16_t) ((SystemCoreClock) / 1e6) - 1;		//���� ������� - � 1 ���
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim2);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

	//������ ������������ ��� master � ��������� ������� �������� ��� ���������� 32-������� �������
	//������� ��������� TIM3. ��������� ��� ���� ������ ������� �� ��������� �������
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

	//������ ������������ ��� slave � ��������� ������� �������� ��� ���������� 32-������� �������
	//�� ������������ �������� master-������� (TIM2) ���������������� ������� ������� slave-�������
	//���������� ITR1 ������������ ��� ������� ������ ��� ����������������� �������� ��������
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
  sSlaveConfig.InputTrigger = TIM_TS_ITR1;
  HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

}

/* TIM4 init function */
void MX_TIM4_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = (uint16_t) ((SystemCoreClock)/1e5 - 1);	//���� ������� - � 10 ���	
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&htim4);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig);

}

/* TIM5 init function */
void MX_TIM5_Init(void)
{

  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

  htim5.Instance = TIM5;
  htim5.Init.Prescaler = (uint16_t) ((SystemCoreClock)/1e5 - 1);	//���� ������� - � 10 ���	
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


/* USART1 init function */
void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init = DefaultUARTParams;
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

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOE_CLK_ENABLE();
  __GPIOA_CLK_ENABLE();

  /*Configure GPIO pins : PE1 PE2 PE6 PE7 PE0 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PA2 PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

}


/* USER CODE BEGIN 4 */
void ProcessDataFromExtDev()
{
	#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_EXTSIGNAL_FROM_UART
	ProcessAudioDataFromUART();
	#endif
	
	pObjSPIMmsgRcvd->setMsg(pUARTRxSLIPPack,nSizeSLIPPack);
	
	if(pObjSPIMmsgRcvd->checkCRC())
	{
		#ifdef DEBUG_PRINTF_SPIM_DATA
		printf("Rcvd SPIM Message\n");
		printf("* Address: %d\n", pObjSPIMmsgRcvd->getAddress());
		printf("* Cmd ID: %x\n", pObjSPIMmsgRcvd->getIDCmd());
		printf("* No Msg: %d\n", pObjSPIMmsgRcvd->getNoMsg());
		#endif

		//��������� � ���������� �����, �������������� �������� ����� �������
		FormAndSendAnswerToExtDev(pObjSPIMmsgRcvd);
		
		//TODO ���������, �� ���� �� ������� ����� ��� ������� (�� ����������� ������)
		
		//TODO ��������� ������ ������� � ��������� �������
		switch(pObjSPIMmsgRcvd->getIDCmd())
		{
			case SPIM_CMD_NOP:
				break;
			case SPIM_CMD_SET_MODE:
				break;
			case SPIM_CMD_SEND_DATA_FRAME:
				break;
			case SPIM_CMD_TAKE_DATA_FRAME:
				break;	
			case SPIM_CMD_REQ_CURRENT_PARAM:
				break;
			case SPIM_CMD_SOFT_VER:
				break;			
			default:
				break;
		}
		
	}
	
	//������� ����� � ������������� ������� SLIP-������
	memset(pUARTRxSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
}


void FormAndSendAnswerToExtDev(SPIMMessage* SPIMmsgRcvd)
{
	SPIMMessage* SPIMmsgToSend = new SPIMMessage;

	FormAnswerToExtDev(SPIMmsgRcvd,SPIMmsgToSend);
	
	SendDataToUART(&huart1, SPIMmsgToSend->Data, SPIMmsgToSend->Size);
	
	delete SPIMmsgToSend;
}


void FormAnswerToExtDev(SPIMMessage* SPIMCmdRcvd, SPIMMessage* SPIMBackCmdToSend)
{
	//���������� ID ������ �� ID �������� �������
	uint8_t IDanswer = SPIMBackCmdToSend->IDBackCmd(SPIMCmdRcvd->getIDCmd());
	//������� - ���������� ����������� ����������
	uint8_t address = SPIM_ADDR_EXTDEV;
	//���������� ����� ������ ��������� � ������� �������� �������
	uint8_t noMsg = SPIMCmdRcvd->getNoMsg();
	
	//������� ��������� �� ���� ���������
	uint8_t* pBodyData = SPIMBackCmdToSend->Body;
	uint8_t bodySize = 0;

	FormBodyOfAnswerToExtDev(SPIMCmdRcvd,pBodyData,bodySize);
	
	SPIMBackCmdToSend->setHeader(bodySize,address,noMsg,IDanswer);
	
	SPIMBackCmdToSend->setBody(pBodyData,bodySize);
	
	SPIMBackCmdToSend->setCRC();
}


void FormBodyOfAnswerToExtDev(SPIMMessage* SPIMCmdRcvd, uint8_t* pBodyData, uint8_t& bodySize)
{
	bodySize = 0;
	
	switch(SPIMCmdRcvd->getIDCmd())
	{
		case SPIM_CMD_NOP:
			//���� ������ ������� ��������� (������ ���� ����� 1)
			if(SPIMCmdRcvd->getSizeBody())
			{
				//��������� ���� ������, ��������� �� ������� (� �������������) ����� �������
				bodySize = 1;
				*pBodyData = SPIMCmdRcvd->Body[0];
			}
			break;
		case SPIM_CMD_SET_MODE:
			//��������� �� ������������ ������ �������
			//TODO �������� ������������� ���������
			if(SPIMCmdRcvd->getSizeBody()==4)
			{
				//TODO ��������� CMX7262 ������� � ��������� ������� � ��������������� ������
				uint8_t nAudioVolume = SPIMCmdRcvd->Body[2]&0x07;
				CMX7262_AudioOutputGain(&g_CMX7262Struct,nAudioVolume);
			}			
			break;
		case SPIM_CMD_SEND_DATA_FRAME:
			break;
		case SPIM_CMD_TAKE_DATA_FRAME:
			break;	
		case SPIM_CMD_REQ_CURRENT_PARAM:
		{
			//TODO ������������ ���������, ��������������� ������������� ����������
			//���� ������ �����������, �� ���������� ����� �������� �� ���������������� �����������
			SPIMCmdRcvd->cmdReqParam.SetPointerToMessage(SPIMCmdRcvd);
			FormCurrentParamAnswer(SPIMCmdRcvd, pBodyData, bodySize);
			break;
		}
		case SPIM_CMD_SOFT_VER:
			uint16_t noSoftVersion;
			bodySize = sizeof(noSoftVersion);
			noSoftVersion = GetARMSoftVer();
			memcpy(pBodyData,&noSoftVersion,sizeof(noSoftVersion));
			break;
		default:
			break;
	}
}


void FormCurrentParamAnswer(SPIMMessage* SPIMCmdRcvd, uint8_t* pBodyData, uint8_t& bodySize)
{
	pBodyData[0] = SPIMCmdRcvd->cmdReqParam.MaskReqParam();
	bodySize=1;
	
	if(SPIMCmdRcvd->cmdReqParam.isOpModeReq())
	{
		//���������� ��������� �������� ������ �����������
		uint8_t radioChanType = objRadioModule.GetRadioChanType();
		uint8_t radioSignalPower = objRadioModule.GetRadioSignalPower();
		uint8_t powerMode = objRadioModule.GetARMPowerMode();

		//��������� ��� �������� ������
		uint8_t OpModeCode = SPIMCmdRcvd->cmdReqParam.OpModeCode(radioChanType,radioSignalPower,powerMode);
		
		pBodyData[bodySize] = OpModeCode;
		bodySize++;		
	}
	
	if(SPIMCmdRcvd->cmdReqParam.isAudioReq())
	{
		//����������� �������������� �����������
		uint8_t audioInLevel = objRadioModule.GetAudioInLevel();
		uint8_t audioOutLevel = objRadioModule.GetAudioOutLevel();
		
		//��������� ��� �������������
		uint8_t AudioCode = SPIMCmdRcvd->cmdReqParam.AudioCode(audioOutLevel,audioInLevel);
		
		pBodyData[bodySize] = AudioCode;
		bodySize++;
	}
	
	if(SPIMCmdRcvd->cmdReqParam.isRxFreqReq())
	{
		pBodyData[bodySize] = objRadioModule.GetRxFreqChan();
		bodySize++;
	}

	if(SPIMCmdRcvd->cmdReqParam.isTxFreqReq())
	{
		pBodyData[bodySize] = objRadioModule.GetTxFreqChan();
		bodySize++;
	}	
	
	if(SPIMCmdRcvd->cmdReqParam.isRSSIReq())
	{
		pBodyData[bodySize] = objRadioModule.GetRSSILevel();
		bodySize++;
	}		
	
	if(SPIMCmdRcvd->cmdReqParam.isChanStateReq())
	{
		pBodyData[bodySize] = objRadioModule.GetRadioChanState();
		bodySize++;
	}		
}


uint16_t GetARMSoftVer()
{
	return(ARM_SOFT_VER);
}


void ProcessCMX7262State()
{

	if((g_CMX7262Struct.uIRQRequest & CMX7262_ODA) == CMX7262_ODA)
	{
		//���������� ���� CMX7262_ODA
		g_CMX7262Struct.uIRQRequest = g_CMX7262Struct.uIRQRequest & ~CMX7262_ODA;
		
		//������ ������
		#ifdef TEST_CMX7262_ENC_MODE
		CMX7262_RxFIFO(&pCmx7262,(uint8_t *)&pDataFromCMX7262[0]);
		#endif
		#ifdef TEST_CMX7262_ENCDEC_AUDIO2CBUS_MODE
		CMX7262_RxFIFO_Audio(&g_CMX7262Struct,(uint8_t *)&pDataFromCMX7262[0]);			
		#endif
	}

	if((g_CMX7262Struct.uIRQRequest & CMX7262_IDW) == CMX7262_IDW)
	{
		#ifdef DEBUG_USE_TL_LINES_FOR_CHECK_CMX7262_EVENTS
		HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_2);
		#endif
		
		//���������� ���� CMX7262_IDW
		g_CMX7262Struct.uIRQRequest = g_CMX7262Struct.uIRQRequest & ~CMX7262_IDW;

		#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_MODE			
			#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_INTERNAL_SIN
			//��������� ����� ��������� �������� 1���
			FillBufByToneSignal((int16_t*)pDataToCMX7262,CMX7262_AUDIOFRAME_SIZE_SAMPLES,CMX7262_FREQ_SAMPLING,1000);
			#endif
		
			//�������� ����� �� CMX7262
			CMX7262_TxFIFO_Audio(&g_CMX7262Struct,(uint8_t *)&pDataToCMX7262[0]);
			#ifdef DEBUG_CMX7262_CNT_TX_AUDIO_BUF
			cntCMX7262TxAudioBuf++;
			#endif
		
			uint16_t nSizeOfTxBuf = sizeof(uint16_t)*CMX7262_AUDIOFRAME_SIZE_SAMPLES;		
			if(nLengthDataToCMX7262>=nSizeOfTxBuf)
			{
				//TODO �������� pDataToCMX7262 �� ��������� �����
				memmove(pDataToCMX7262,pDataToCMX7262+nSizeOfTxBuf,nLengthDataToCMX7262-nSizeOfTxBuf);
				nLengthDataToCMX7262-=nSizeOfTxBuf;
			}
			else
				printf("Bufer pDataToCMX7262 is Empty");
		
		#endif
	}

}
		

void SPIMInit()
{
	//������� ������� ��� ��������� � ������������ ��������� SPIM-���������
	pObjSPIMmsgRcvd = new SPIMMessage;
	//pObjSPIMmsgToSend  = new SPIMMessage;
}

void SPIMDeInit()
{
	//������� ������� ��� ��������� � ������������ ��������� SPIM-���������
	delete pObjSPIMmsgRcvd;
	//delete pObjSPIMmsgToSend;
}


#ifdef TEST_CMX7262_ENCDEC_CBUS2AUDIO_EXTSIGNAL_FROM_UART
void ProcessAudioDataFromUART()
{
	if((nSizeSLIPPack!=160)&&(nSizeSLIPPack!=0))
		printf("Size of Rcvd SLIP Pack isn't 160 bytes: %d\n",nSizeSLIPPack);
	
	if(nSizeSLIPPack==0)
		return;
	
	if(nLengthDataToCMX7262+nSizeSLIPPack<MAX_SIZE_OF_DATA_TO_CMX7262)
	{
		//�������� ������ � ������� FIFO ��� �������� �� CMX7262
		memcpy(pDataToCMX7262+nLengthDataToCMX7262,pUARTRxSLIPPack,nSizeSLIPPack);
		nLengthDataToCMX7262+=nSizeSLIPPack;
	}
	else
		printf("Bufer DataToCMX7262 is full. Data from UART is ignored");
	
	if((nLengthDataToCMX7262 >= CMX7262_NUM_AUDIO_FRAMES_FROM_UART_TO_START_TESTMODE*sizeof(int16_t)*CMX7262_AUDIOFRAME_SIZE_SAMPLES))
	{
		RadioModuleOpMode=OPMODE_TEST_CMX7262ENCDEC;
		
		//��������� CMX7262 � ����� EncDec
		CMX7262_EncodeDecode_CBUS2Audio(&g_CMX7262Struct);
		//�������� ����� �������� ������ �� CMX7262
		CMX7262_TxFIFO_Audio(&g_CMX7262Struct,(uint8_t *)&pDataToCMX7262[0]);
		#ifdef DEBUG_CMX7262_CNT_TX_AUDIO_BUF
		cntCMX7262TxAudioBuf++;
		#endif

		uint16_t nSizeOfTxBuf = sizeof(uint16_t)*CMX7262_AUDIOFRAME_SIZE_SAMPLES;		
		if(nLengthDataToCMX7262>=nSizeOfTxBuf)
		{
			//TODO �������� pDataToCMX7262 �� ��������� �����
			memmove(pDataToCMX7262,pDataToCMX7262+nSizeOfTxBuf,nLengthDataToCMX7262-nSizeOfTxBuf);
			nLengthDataToCMX7262-=nSizeOfTxBuf;
		}
		else
			printf("Bufer pDataToCMX7262 is Empty");
	}
}
#endif


/* USER CODE END 4 */

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

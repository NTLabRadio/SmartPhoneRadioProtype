/**
  ******************************************************************************
  * File Name          : mxconstants.h
  * Description        : This file contains the common defines of the application
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

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define LED1_Pin GPIO_PIN_7
#define LED1_GPIO_Port GPIOE
#define LED2_Pin GPIO_PIN_6
#define LED2_GPIO_Port GPIOE
#define PTT_Pin GPIO_PIN_8
#define PTT_GPIO_Port GPIOC
#define RESET_CC1120_Pin GPIO_PIN_15
#define RESET_CC1120_GPIO_Port GPIOB
#define CS_CC1120_Pin GPIO_PIN_4
#define CS_CC1120_GPIO_Port GPIOA
#define CS_CMX7262_Pin GPIO_PIN_2
#define CS_CMX7262_GPIO_Port GPIOA
#define IRQ_CC1120_Pin GPIO_PIN_0
#define IRQ_CC1120_GPIO_Port GPIOA
#define IRQ_CMX7262_Pin GPIO_PIN_1
#define IRQ_CMX7262_GPIO_Port GPIOA
#define AUDIO_PA_SHDN_Pin GPIO_PIN_0
#define AUDIO_PA_SHDN_GPIO_Port GPIOE
#define TL1_Pin GPIO_PIN_1
#define TL1_GPIO_Port GPIOE
#define TL2_Pin GPIO_PIN_2
#define TL2_GPIO_Port GPIOE
#define TL3_Pin GPIO_PIN_3
#define TL3_GPIO_Port GPIOE
#define TL4_Pin GPIO_PIN_4
#define TL4_GPIO_Port GPIOE
#define TL5_Pin GPIO_PIN_5
#define TL5_GPIO_Port GPIOE


/* USER CODE BEGIN Private defines */

#define PTT_PRESSED()		(!HAL_GPIO_ReadPin(PTT_GPIO_Port, PTT_Pin))

#define CC1120_START()  HAL_GPIO_WritePin(RESET_CC1120_GPIO_Port, RESET_CC1120_Pin, GPIO_PIN_SET)
#define CC1120_RESET()	HAL_GPIO_WritePin(RESET_CC1120_GPIO_Port, RESET_CC1120_Pin, GPIO_PIN_RESET)

#define LEDS_OFF()			{LED1_OFF(); LED2_OFF();}
#define LED1_ON()				HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET)
#define LED1_OFF()			HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET)
#define LED2_ON()				HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET)
#define LED2_OFF()			HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET)

#define TLS_LOW()				{TL1_LOW(); TL2_LOW(); TL3_LOW(); TL4_LOW(); TL5_LOW();}
#define TL1_HIGH()			HAL_GPIO_WritePin(TL1_GPIO_Port, TL1_Pin, GPIO_PIN_SET)
#define TL1_LOW()				HAL_GPIO_WritePin(TL1_GPIO_Port, TL1_Pin, GPIO_PIN_RESET)
#define TL2_HIGH()			HAL_GPIO_WritePin(TL2_GPIO_Port, TL2_Pin, GPIO_PIN_SET)
#define TL2_LOW()				HAL_GPIO_WritePin(TL2_GPIO_Port, TL2_Pin, GPIO_PIN_RESET)
#define TL3_HIGH()			HAL_GPIO_WritePin(TL3_GPIO_Port, TL3_Pin, GPIO_PIN_SET)
#define TL3_LOW()				HAL_GPIO_WritePin(TL3_GPIO_Port, TL3_Pin, GPIO_PIN_RESET)
#define TL4_HIGH()			HAL_GPIO_WritePin(TL4_GPIO_Port, TL4_Pin, GPIO_PIN_SET)
#define TL4_LOW()				HAL_GPIO_WritePin(TL4_GPIO_Port, TL4_Pin, GPIO_PIN_RESET)
#define TL5_HIGH()			HAL_GPIO_WritePin(TL5_GPIO_Port, TL5_Pin, GPIO_PIN_SET)
#define TL5_LOW()				HAL_GPIO_WritePin(TL5_GPIO_Port, TL5_Pin, GPIO_PIN_RESET)

// Definitions for IO port mapped as chip select (output).

#define CMX7262_SPI_CS_PIN              	GPIO_PIN_2                 	/* PA.02 */
#define CMX7262_SPI_CS_GPIO_PORT        	GPIOA                      	/* GPIOA */

// GPIO lines for SPI interfaces

#define CMX7262_SPI_SCK_PIN               GPIO_PIN_5                  /* PA.05 */
#define CMX7262_SPI_SCK_GPIO_PORT         GPIOA                       /* GPIOA */

#define CMX7262_SPI_MISO_PIN              GPIO_PIN_6                  /* PA.06 */
#define CMX7262_SPI_MISO_GPIO_PORT        GPIOA                       /* GPIOA */

#define CMX7262_SPI_MOSI_PIN              GPIO_PIN_7                  /* PA.07 */
#define CMX7262_SPI_MOSI_GPIO_PORT        GPIOA                       /* GPIOA */

//IRQ from CMX7262
#define CMX7262_IRQN_PIN              		GPIO_PIN_1                  /* PA.01 */
#define CMX7262_IRQN_PORT        					GPIOA                       /* GPIOA */



// Definitions for IO port mapped as chip select (output).

#define CC1120_SPI_CS_PIN            	   	GPIO_PIN_4              		/* PA.04 */
#define CC1120_SPI_CS_GPIO_PORT      	   	GPIOA                       /* GPIOA */

// GPIO lines for SPI interfaces

#define CC1120_SPI_SCK_PIN               	GPIO_PIN_5                  /* PA.05 */
#define CC1120_SPI_SCK_GPIO_PORT         	GPIOA                       /* GPIOA */

#define CC1120_SPI_MISO_PIN              	GPIO_PIN_6                  /* PA.06 */
#define CC1120_SPI_MISO_GPIO_PORT        	GPIOA                       /* GPIOA */

#define CC1120_SPI_MOSI_PIN              	GPIO_PIN_7                  /* PA.07 */
#define CC1120_SPI_MOSI_GPIO_PORT        	GPIOA                       /* GPIOA */

//IRQ from CC1120
#define CC1120_IRQN_PIN              			GPIO_PIN_0                  /* PA.00 */
#define CC1120_IRQN_PORT        					GPIOA                       /* GPIOA */


#define CMX7262_CSN_LOW()     HAL_GPIO_WritePin(CMX7262_SPI_CS_GPIO_PORT, CMX7262_SPI_CS_PIN, GPIO_PIN_RESET)
#define CMX7262_CSN_HIGH()    HAL_GPIO_WritePin(CMX7262_SPI_CS_GPIO_PORT, CMX7262_SPI_CS_PIN, GPIO_PIN_SET)
#define CC1120_CSN_LOW()     	HAL_GPIO_WritePin(CC1120_SPI_CS_GPIO_PORT, CC1120_SPI_CS_PIN, GPIO_PIN_RESET)
#define CC1120_CSN_HIGH()			HAL_GPIO_WritePin(CC1120_SPI_CS_GPIO_PORT, CC1120_SPI_CS_PIN, GPIO_PIN_SET)


/* USER CODE END Private defines */

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

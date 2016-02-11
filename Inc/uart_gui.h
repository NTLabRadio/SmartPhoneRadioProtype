/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_GUI_H
#define __UART_GUI_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"

#define TXBUFFERSIZE 					255 
#define RXBUFFERSIZE 					255
	 
#define CURRENT_UART 					huart1
#define CURRENT_SPI	 					hspi1	
	 
#define STM32F103_SELECT 			0x01
#define CC1120_SELECT 				0x02	 
	 
#define CHIP_ID_CC1120				0x01			/* запрос Chip ID трансивера */
#define CHIP_FW_VER	 					0x02			/* запрос версии firmware трансивера */
#define STATUS_CC1120	 				0x0A			/* запрос статусного байта трансивера */
#define CC1120_TX							0x07			/* перевод трансивера СС1120 в режим передачи */
#define	CC1120_IDLE						0x0B			/* перевод трансивера в режим IDLE */
#define CC1120_RX							0x0F			/* перевод трансивера СС1120 в режим приема */	 

	 
	 
	 
	 
#define CHECK_ID_CC1120_OK	 	0x01			/* верификация трансивера пройдена */
#define CHECK_ID_CC1120_FAIL	0x02			/* верификация трансивера не удалась */	 


#define FW_VERSION_FAIL				0x00			/* получить вервию firmware трансивера не удалось */



void GUI_rx (UART_HandleTypeDef *huart);	 
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void Device_Select (uint8_t select);
void GUI_Tx (UART_HandleTypeDef *huart, uint8_t *tx_buffer);
uint8_t CC1120_CheckCommand (uint8_t *command);	
void ChipIDCC1120Read ();
void ChipFWVersionRead ();
void STATUSCC1120Read ();
void CC1120RxSet();
void CC1120TxSet();
void CC1120IDLESet();



	 
	 
	 
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UART_GUI_H */

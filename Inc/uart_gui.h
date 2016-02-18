/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __UART_GUI_H
#define __UART_GUI_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_uart.h"

#define TXBUFFERSIZE 					0xFF
#define RXBUFFERSIZE 					0xFF
	 
#define CURRENT_UART 					huart1
#define CURRENT_SPI	 					hspi1	
	 
#define STM32F103_SELECT 			0x01
#define CC1120_SELECT 				0x02	 
	 
#define CHIP_ID_CC1120									0x01			/* запрос Chip ID трансивера */
#define CHIP_FW_VER	 										0x02			/* запрос версии firmware трансивера */
#define STATUS_CC1120	 									0x0A			/* запрос статусного байта трансивера */
#define CC1120_TX												0x09			/* перевод трансивера СС1120 в режим передачи */
#define	CC1120_IDLE											0x0B			/* перевод трансивера в режим IDLE */
#define CC1120_RX												0x0F			/* перевод трансивера СС1120 в режим приема */	 
#define CC1120_RESET										0x06			/* сброс трансивера СС1120 */
#define CC1120_FIFO_NUM_TXBYTES					0x03			/* чтение количества байтов TX FIFO */
#define CC1120_FIFO_NUM_RXBYTES					0x04			/* чтение количества байтов TX FIFO */
#define	CC1120_TX_FIFO_FLUSH						0x07			/* очистка TX FIFO */
#define	CC1120_RX_FIFO_FLUSH						0x08			/* очистка RX FIFO */
#define	CC1120_TX_FIFO_WRITE						0x11			/* запись в TX FIFO */
#define	CC112_MANUAL_CALIBRATION				0x05			/* ручная калиброка синтезатора CC1120 */
#define CC1120_MARCSTATE								0x0C			/* запрос состояния трансивера CC1120 */
#define CC1120_SFSTXON									0x0D			/* автоматическая калибровка синтезатора CC1120 */
#define CC1120_CONFIG_WRITE							0x12			/* выбор и запись конфигурации в трансивер */
#define CC1120_CONFIG_READ							0x13			/* чтение конфигурации трансивера и сравнение с записанной */
#define	CC1120_FREQ_WRITE								0x14			/* запись частоты в PLL CC1120 */
#define CC1120_FREQ_READ								0x15			/* чтение частоты PLL CC1120 */
#define CC1120_RX_FIFO_READ							0x10			/* чтение содержимого RX FIFO */
	 
	 
	 
#define CHECK_ID_CC1120_OK	 	0x01			/* верификация трансивера пройдена */
#define CHECK_ID_CC1120_FAIL	0x02			/* верификация трансивера не удалась */	 


#define FW_VERSION_FAIL				0x00			/* получить вервию firmware трансивера не удалось */



void GUI_rx (UART_HandleTypeDef *huart);



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void Device_Select (uint8_t select);


void GUI_Tx (UART_HandleTypeDef *huart, uint8_t *tx_buffer);



uint8_t CC1120_CheckCommand (uint8_t *command);	
void ChipIDCC1120Read (void);
void ChipFWVersionRead (void);
void STATUSCC1120Read (void);
void CC1120RxSet(void);
void CC1120TxSet(void);
void CC1120IDLESet(void);
void CC1120Reset(void);
void TxFIFONumBytesRead (void);
void TxFIFOFlush (void);
void TxFIFOWrite (uint8_t *data_ptr, uint8_t num_byte);
void ManualCalibration (void);
void MARCState (void);
void ConfigWrite(uint8_t *data_ptr);
void FreqWrite(uint8_t *freqSet);
void FreqRead(void);



	 
	 
	 
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UART_GUI_H */

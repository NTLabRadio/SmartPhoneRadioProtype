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
	 
#define CHIP_ID_CC1120									0x01			/* ������ Chip ID ���������� */
#define CHIP_FW_VER	 										0x02			/* ������ ������ firmware ���������� */
#define STATUS_CC1120	 									0x0A			/* ������ ���������� ����� ���������� */
#define CC1120_TX												0x09			/* ������� ���������� ��1120 � ����� �������� */
#define	CC1120_IDLE											0x0B			/* ������� ���������� � ����� IDLE */
#define CC1120_RX												0x0F			/* ������� ���������� ��1120 � ����� ������ */	 
#define CC1120_RESET										0x06			/* ����� ���������� ��1120 */
#define CC1120_FIFO_NUM_TXBYTES					0x03			/* ������ ���������� ������ TX FIFO */
#define CC1120_FIFO_NUM_RXBYTES					0x04			/* ������ ���������� ������ TX FIFO */
#define	CC1120_TX_FIFO_FLUSH						0x07			/* ������� TX FIFO */
#define	CC1120_RX_FIFO_FLUSH						0x08			/* ������� RX FIFO */
#define	CC1120_TX_FIFO_WRITE						0x11			/* ������ � TX FIFO */
#define	CC112_MANUAL_CALIBRATION				0x05			/* ������ ��������� ����������� CC1120 */
#define CC1120_MARCSTATE								0x0C			/* ������ ��������� ���������� CC1120 */
#define CC1120_SFSTXON									0x0D			/* �������������� ���������� ����������� CC1120 */
#define CC1120_CONFIG_WRITE							0x12			/* ����� � ������ ������������ � ��������� */
#define CC1120_CONFIG_READ							0x13			/* ������ ������������ ���������� � ��������� � ���������� */
#define	CC1120_FREQ_WRITE								0x14			/* ������ ������� � PLL CC1120 */
#define CC1120_FREQ_READ								0x15			/* ������ ������� PLL CC1120 */
#define CC1120_RX_FIFO_READ							0x10			/* ������ ����������� RX FIFO */
	 
	 
	 
#define CHECK_ID_CC1120_OK	 	0x01			/* ����������� ���������� �������� */
#define CHECK_ID_CC1120_FAIL	0x02			/* ����������� ���������� �� ������� */	 


#define FW_VERSION_FAIL				0x00			/* �������� ������ firmware ���������� �� ������� */



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

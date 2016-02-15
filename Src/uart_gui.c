#include "uart_gui.h"
#include "cc1120.h"

uint8_t UART_Tx_Buff [TXBUFFERSIZE];
uint8_t UART_Rx_Buff [RXBUFFERSIZE];

uint8_t ARM_command = 0;
uint8_t Payload_length = 0;
uint8_t	Counter_lines = 0;


extern SPI_HandleTypeDef CURRENT_SPI;
extern UART_HandleTypeDef CURRENT_UART;


void Device_Select (uint8_t select) // выбор устройства
{
 	switch (select)
	{
		case STM32F103_SELECT:
		
		break;
		
		case CC1120_SELECT:
		CC1120_CheckCommand (UART_Rx_Buff);	
		break;
	}
}




void ChipIDCC1120Read (void) // запрос на верификацию трансивера
{
		if (CC1120_CheckModule (&CURRENT_SPI)) // если ответ от трансивера 1, 
			{			
		UART_Tx_Buff[2] = CHECK_ID_CC1120_OK;
			} 
	else 
			{
				UART_Tx_Buff[2] = CHECK_ID_CC1120_FAIL;
			}
			
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);		
}

void ChipFWVersionRead (void) // запрос версии firmware трансивера
{
		uint8_t Num_fw_ver = 0; // возвращаемый номер верии програмного обеспечения трансивера
		Num_fw_ver = CC1120_CheckVersion (&CURRENT_SPI);
		
		if (Num_fw_ver) // если ответ от трансивера отличный от нуля, 
			{			
		UART_Tx_Buff[2] = Num_fw_ver; // номер версии трансивера
			} 
	else 
			{
				UART_Tx_Buff[2] = FW_VERSION_FAIL; // получить версию трансивера не удалось
			}	
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void STATUSCC1120Read (void) // запрос статуса трансивера
{
		UART_Tx_Buff[2] = CC1120_Status (&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void CC1120TxSet(void) // перевод трансивера в режим передачи
{
		UART_Tx_Buff[2] = CC1120_Tx(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void CC1120IDLESet(void) // перевод трансивера в режим IDLE
{
		UART_Tx_Buff[2] = CC1120_IDLE_set(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void CC1120RxSet(void) // перевод трансивера в режим приема
{
		UART_Tx_Buff[2] = CC1120_Rx(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void CC1120Reset(void) // сброс трансивера
{
		UART_Tx_Buff[2] = CC1120_Reset(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void TxFIFONumBytesRead (void) // чтение количества данных в FIFO TX
{
		uint8_t Num_TxFIFO_bytes = 0; // количество байтов в FIFO TX
		Num_TxFIFO_bytes = CC1120_TxFIFONumBytes(&CURRENT_SPI);
		
		if (Num_TxFIFO_bytes != TX_FIFO_FAIL) // если ответ от трансивера не 0xFF, 
			{			
		UART_Tx_Buff[2] = Num_TxFIFO_bytes; // количество байтов в TX FIFO
			} 
	else 
			{
				UART_Tx_Buff[2] = TX_FIFO_FAIL; // сообщение об ошибке чтения TX FIFO
			}	
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}


void TxFIFOFlush (void)
{
		UART_Tx_Buff[2] = CC1120_TxFIFOFlush(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}


void TxFIFOWrite (uint8_t *data_ptr, uint8_t num_byte)
{
		UART_Tx_Buff[2] = CC1120_TxFIFOWrite(&CURRENT_SPI, data_ptr, num_byte);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}






uint8_t CC1120_CheckCommand (uint8_t *command) // соответствие кода команды
{
	switch (command [0])
	{
		case CHIP_ID_CC1120: // верификация трансивера
		
		ChipIDCC1120Read();
		
		break;
	
		case CHIP_FW_VER: // верификация трансивера
		
		ChipFWVersionRead();
		
		break;

		case STATUS_CC1120: // запрос статусного байта
		
		STATUSCC1120Read ();
		
		break;

		case CC1120_TX: // превод трансивера в режим передачи
		
		CC1120TxSet();
		
		break;
		
		case CC1120_IDLE: // превод трансивера в режим IDLE
		
		CC1120IDLESet();
		
		break;
		
		case CC1120_RX: // превод трансивера в режим приема
		
		CC1120RxSet();
		
		break;
		
		case CC1120_RESET: // сброс трансивера
		
		CC1120Reset();
		
		break;
		
		case CC1120_FIFO_NUM_TXBYTES: // чтение количества байтов в TX FIFO
		
		TxFIFONumBytesRead();
		
		break;
		
		case CC1120_TX_FIFO_FLUSH: // чтение количества байтов в TX FIFO
		
		TxFIFOFlush();
		
		break;
		
		case CC1120_TX_FIFO_WRITE: // чтение количества байтов в TX FIFO
		
		TxFIFOWrite (command, Payload_length);
		
		break;
		
	}
	
	return 0;
}



	void GUI_rx (UART_HandleTypeDef *huart) // прием данных по UART
{
 
	 if ( HAL_UART_GetState(huart)== HAL_UART_STATE_READY) 
	 {	
					HAL_UART_Receive_DMA(huart, UART_Rx_Buff,0x02);
					while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) // ожидаем окончания приема
					{
					}
					ARM_command = UART_Rx_Buff [0]; // команда микроконтроллру (выбор устройства)
					Payload_length = UART_Rx_Buff [1]; // количество байтов данных нагрузки
	 }

	
	if ( HAL_UART_GetState(huart)== HAL_UART_STATE_READY) 
	 {
					HAL_UART_Receive_DMA(huart, UART_Rx_Buff,Payload_length); // принимаем данные		
					while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) // ожидаем окончания приема
					{
					}	
					Device_Select (ARM_command);
	 }
	
}


	void GUI_Tx (UART_HandleTypeDef *huart, uint8_t *tx_buffer) // передача данных по UART
{
	
	if ( HAL_UART_GetState(huart)== HAL_UART_STATE_READY) 
	{
						HAL_UART_Transmit_DMA(huart, tx_buffer, tx_buffer[1]);
						
						while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) // ожидаем окончания передачи
						{
						}
	}
}



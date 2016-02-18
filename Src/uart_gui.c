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
	//fifo_write_data_ptr 0 байт - команда трансиверу, 1 байт и далее - данные FIFO TX
	// убираем 0 элемент и сдвигаем все данные на байт влево

	for (uint8_t i = 0; i<(num_byte); i++)
	{
		data_ptr[i] = data_ptr[i+1];
	}
		UART_Tx_Buff[2] = CC1120_TxFIFOWrite(&CURRENT_SPI, data_ptr, num_byte-1);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void ManualCalibration (void)
{
		UART_Tx_Buff[2] = CC1120_ManualCalibration(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);

}

void MARCState (void)
{
		UART_Tx_Buff[2] = CC1120_MARCState(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void SFSTXON (void)	
{
		UART_Tx_Buff[2] = CC1120_SFSTXON_set(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void RxFIFONumBytesRead (void) // чтение количества данных в FIFO TX
{
		uint8_t Num_RxFIFO_bytes = 0; // количество байтов в FIFO TX
		Num_RxFIFO_bytes = CC1120_RxFIFONumBytes(&CURRENT_SPI);
		
		if (Num_RxFIFO_bytes != RX_FIFO_FAIL) // если ответ от трансивера не 0xFF, 
			{			
		UART_Tx_Buff[2] = Num_RxFIFO_bytes; // количество байтов в TX FIFO
			} 
	else 
			{
				UART_Tx_Buff[2] = RX_FIFO_FAIL; // сообщение об ошибке чтения TX FIFO
			}	
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void RxFIFOFlush (void)
{
		UART_Tx_Buff[2] = CC1120_RxFIFOFlush(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void ConfigWrite(uint8_t *data_ptr)
{
		// в data_ptr[1] содержится номер конфигурации
		switch (data_ptr [1])
		{
			case    0x01:
				
			UART_Tx_Buff[2] = CC1120_ConfigWrite(&CURRENT_SPI, CC1120_Config_4800, sizeof (CC1120_Config_4800)/sizeof(registerSetting_t));
			
			break;
			
			case    0x02:
				
			UART_Tx_Buff[2] = CC1120_ConfigWrite(&CURRENT_SPI, CC1120_Config_9600, sizeof (CC1120_Config_9600)/sizeof(registerSetting_t));
				
			break;
			
			case    0x03:
				
			UART_Tx_Buff[2] = CC1120_ConfigWrite(&CURRENT_SPI, CC1120_Config_19200, sizeof (CC1120_Config_19200)/sizeof(registerSetting_t));
				
			break;
			
			case    0x04:
				
			UART_Tx_Buff[2] = CC1120_ConfigWrite(&CURRENT_SPI, CC1120_Config_48000, sizeof (CC1120_Config_48000)/sizeof(registerSetting_t));
				
			break;
		}
		
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);

}

void ConfigReadCompare(uint8_t *data_ptr)
{
		// в data_ptr[1] содержится номер конфигурации
		switch (data_ptr [1])
		{
			case    0x01:
				
			UART_Tx_Buff[2] = CC1120_ConfigReadCompare(&CURRENT_SPI, CC1120_Config_4800, sizeof (CC1120_Config_4800)/sizeof(registerSetting_t));
			
			break;
			
			case    0x02:
				
			UART_Tx_Buff[2] = CC1120_ConfigReadCompare(&CURRENT_SPI, CC1120_Config_9600, sizeof (CC1120_Config_9600)/sizeof(registerSetting_t));
				
			break;
			
			case    0x03:
				
			UART_Tx_Buff[2] = CC1120_ConfigReadCompare(&CURRENT_SPI, CC1120_Config_19200, sizeof (CC1120_Config_19200)/sizeof(registerSetting_t));
				
			break;
			
			case    0x04:
				
			UART_Tx_Buff[2] = CC1120_ConfigReadCompare(&CURRENT_SPI, CC1120_Config_48000, sizeof (CC1120_Config_48000)/sizeof(registerSetting_t));
				
			break;
		}
		
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);

}


void FreqWrite(uint8_t *freqSet)
{
		//freqSet 0 байт - команда трансиверу, 1, 2, 3 байт - данные FREQ2, FREQ1, FREQ0
		// убираем 0 элемент и сдвигаем все данные на байт влево

			for (uint8_t i = 0; i<3; i++)
			{
				freqSet[i] = freqSet[i+1];
			}
	
		UART_Tx_Buff[2] = CC1120_FreqWrite (&CURRENT_SPI, freqSet);
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x03; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);


}

void FreqRead(void)
{
    UART_Tx_Buff[2] = (CC1120_FreqRead (&CURRENT_SPI))[0]; 	// FREQ2
		UART_Tx_Buff[3] = (CC1120_FreqRead (&CURRENT_SPI))[1]; 	// FREQ1
		UART_Tx_Buff[4] = (CC1120_FreqRead (&CURRENT_SPI))[2];	// FREQ0
		UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
		UART_Tx_Buff[1] = 0x05; // количество всех байтов в ответе	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);		

}

void RxFIFORead (void)
{
	uint8_t buffNumCount = 0;
	
	buffNumCount = (CC1120_RxFIFORead(&CURRENT_SPI))[0]; // количество байтов в буфере
	UART_Tx_Buff[0] = CC1120_SELECT; // подтверждение, что ответ от трансивера
	UART_Tx_Buff[1] = buffNumCount + 2; // количество всех байтов в ответе
	
			// сдвиг на 2 байта для вставки подтверждения ответа от трансивера и количество байтов в ответе
			for (uint8_t i=1; i<UART_Tx_Buff[1]; i++) // возможно. потеряется последний байт может надо +1, проверить!!!
			{
				UART_Tx_Buff[i+1] = (CC1120_RxFIFORead(&CURRENT_SPI))[i];
			}

	
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
		
		case CC112_MANUAL_CALIBRATION: // ручная калибровка синтезатора CC1120
			
		ManualCalibration ();
		
		break;
		
		case CC1120_MARCSTATE: // запрос состояния трансивера
		
		MARCState ();
		
		break;
		
		case CC1120_SFSTXON: // автоматическая калибровка синтезатора
		
		SFSTXON ();		
		
		break;
		
		case CC1120_FIFO_NUM_RXBYTES: // чтение количества байтов в RX FIFO
		
		RxFIFONumBytesRead();
		
		break;
		
		case CC1120_RX_FIFO_FLUSH: // очистка в RX FIFO
		
		RxFIFOFlush();
		
		break;
		
		case CC1120_CONFIG_WRITE: // запись конфигурации в трансивер
		
		ConfigWrite(command);
		
		break;
		
		case CC1120_CONFIG_READ: // чтение и сравнение конфигурации
		
		ConfigReadCompare(command);
		
		break;
		
		case CC1120_FREQ_WRITE: // запись частоты в трансивер
		
		FreqWrite(command);
		
		break;
		
		case CC1120_FREQ_READ: // чтение частоты трансивера
		
		FreqRead();
		
		break;
		
		case CC1120_RX_FIFO_READ: // чтение RX FIFO
		
		RxFIFORead ();
		
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



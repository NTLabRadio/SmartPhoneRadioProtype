#include "uart_gui.h"
#include "cc1120.h"

uint8_t UART_Tx_Buff [TXBUFFERSIZE];
uint8_t UART_Rx_Buff [RXBUFFERSIZE];

uint8_t ARM_command = 0;
uint8_t Payload_length = 0;
uint8_t Num_fw_ver = 0; // возвращаемый номер верии програмного обеспечения трансивера

extern SPI_HandleTypeDef CURRENT_SPI;
extern UART_HandleTypeDef CURRENT_UART;
extern uint8_t pCC1120RxData [255];


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

void ChipIDCC1120Read () // запрос на верификацию трансивера
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

void ChipFWVersionRead () // запрос версии firmware трансивера
{
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
	}
}


void GUI_rx (UART_HandleTypeDef *huart) // прием данных по UART
{
  HAL_UART_Receive_DMA(huart, UART_Rx_Buff,2); // принимаем 2 байта данных из UART. Команда контроллеру и количество байтов данных
	
	while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) // ожидаем окончания приема
	{
	}
	
	ARM_command = UART_Rx_Buff [0]; // команда микроконтроллру (выбор устройства)
	
	Payload_length = UART_Rx_Buff [1]; // количество байтов данных нагрузки
	
	HAL_UART_Receive_DMA(huart, UART_Rx_Buff,Payload_length); // принимаем данные
	
	while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) // ожидаем окончания приема
	{
	}
	
	Device_Select (ARM_command);
	
}


void GUI_Tx (UART_HandleTypeDef *huart, uint8_t *tx_buffer) // передача данных по UART
{
	HAL_UART_Transmit_DMA(huart, tx_buffer, tx_buffer[1]);
	while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) // ожидаем окончания передачи
	{
	}
	GUI_rx (huart); // переходим в режим ожидания приема данных
}



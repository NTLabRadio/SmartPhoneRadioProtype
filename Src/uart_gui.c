#include "uart_gui.h"
#include "cc1120.h"

uint8_t UART_Tx_Buff [TXBUFFERSIZE];
uint8_t UART_Rx_Buff [RXBUFFERSIZE];

uint8_t ARM_command = 0;
uint8_t Payload_length = 0;
uint8_t Num_fw_ver = 0; // ������������ ����� ����� ����������� ����������� ����������

extern SPI_HandleTypeDef CURRENT_SPI;
extern UART_HandleTypeDef CURRENT_UART;
extern uint8_t pCC1120RxData [255];


void Device_Select (uint8_t select) // ����� ����������
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

void ChipIDCC1120Read () // ������ �� ����������� ����������
{
		if (CC1120_CheckModule (&CURRENT_SPI)) // ���� ����� �� ���������� 1, 
			{			
		UART_Tx_Buff[2] = CHECK_ID_CC1120_OK;
			} 
	else 
			{
				UART_Tx_Buff[2] = CHECK_ID_CC1120_FAIL;
			}
			
		UART_Tx_Buff[0] = CC1120_SELECT; // �������������, ��� ����� �� ����������
		UART_Tx_Buff[1] = 0x03; // ���������� ���� ������ � ������	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void ChipFWVersionRead () // ������ ������ firmware ����������
{
		Num_fw_ver = CC1120_CheckVersion (&CURRENT_SPI);
		
		if (Num_fw_ver) // ���� ����� �� ���������� �������� �� ����, 
			{			
		UART_Tx_Buff[2] = Num_fw_ver; // ����� ������ ����������
			} 
	else 
			{
				UART_Tx_Buff[2] = FW_VERSION_FAIL; // �������� ������ ���������� �� �������
			}	
		UART_Tx_Buff[0] = CC1120_SELECT; // �������������, ��� ����� �� ����������
		UART_Tx_Buff[1] = 0x03; // ���������� ���� ������ � ������	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}


uint8_t CC1120_CheckCommand (uint8_t *command) // ������������ ���� �������
{
	switch (command [0])
	{
		case CHIP_ID_CC1120: // ����������� ����������
		
		ChipIDCC1120Read();
		
		break;
	
		case CHIP_FW_VER: // ����������� ����������
		
		ChipFWVersionRead();
		
		break;		
	}
}


void GUI_rx (UART_HandleTypeDef *huart) // ����� ������ �� UART
{
  HAL_UART_Receive_DMA(huart, UART_Rx_Buff,2); // ��������� 2 ����� ������ �� UART. ������� ����������� � ���������� ������ ������
	
	while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) // ������� ��������� ������
	{
	}
	
	ARM_command = UART_Rx_Buff [0]; // ������� ��������������� (����� ����������)
	
	Payload_length = UART_Rx_Buff [1]; // ���������� ������ ������ ��������
	
	HAL_UART_Receive_DMA(huart, UART_Rx_Buff,Payload_length); // ��������� ������
	
	while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) // ������� ��������� ������
	{
	}
	
	Device_Select (ARM_command);
	
}


void GUI_Tx (UART_HandleTypeDef *huart, uint8_t *tx_buffer) // �������� ������ �� UART
{
	HAL_UART_Transmit_DMA(huart, tx_buffer, tx_buffer[1]);
	while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) // ������� ��������� ��������
	{
	}
	GUI_rx (huart); // ��������� � ����� �������� ������ ������
}



#include "uart_gui.h"
#include "cc1120.h"

uint8_t UART_Tx_Buff [TXBUFFERSIZE];
uint8_t UART_Rx_Buff [RXBUFFERSIZE];

uint8_t ARM_command = 0;
uint8_t Payload_length = 0;
uint8_t	Counter_lines = 0;


extern SPI_HandleTypeDef CURRENT_SPI;
extern UART_HandleTypeDef CURRENT_UART;


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




void ChipIDCC1120Read (void) // ������ �� ����������� ����������
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

void ChipFWVersionRead (void) // ������ ������ firmware ����������
{
		uint8_t Num_fw_ver = 0; // ������������ ����� ����� ����������� ����������� ����������
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

void STATUSCC1120Read (void) // ������ ������� ����������
{
		UART_Tx_Buff[2] = CC1120_Status (&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // �������������, ��� ����� �� ����������
		UART_Tx_Buff[1] = 0x03; // ���������� ���� ������ � ������	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void CC1120TxSet(void) // ������� ���������� � ����� ��������
{
		UART_Tx_Buff[2] = CC1120_Tx(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // �������������, ��� ����� �� ����������
		UART_Tx_Buff[1] = 0x03; // ���������� ���� ������ � ������	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void CC1120IDLESet(void) // ������� ���������� � ����� IDLE
{
		UART_Tx_Buff[2] = CC1120_IDLE_set(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // �������������, ��� ����� �� ����������
		UART_Tx_Buff[1] = 0x03; // ���������� ���� ������ � ������	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void CC1120RxSet(void) // ������� ���������� � ����� ������
{
		UART_Tx_Buff[2] = CC1120_Rx(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // �������������, ��� ����� �� ����������
		UART_Tx_Buff[1] = 0x03; // ���������� ���� ������ � ������	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void CC1120Reset(void) // ����� ����������
{
		UART_Tx_Buff[2] = CC1120_Reset(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // �������������, ��� ����� �� ����������
		UART_Tx_Buff[1] = 0x03; // ���������� ���� ������ � ������	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}

void TxFIFONumBytesRead (void) // ������ ���������� ������ � FIFO TX
{
		uint8_t Num_TxFIFO_bytes = 0; // ���������� ������ � FIFO TX
		Num_TxFIFO_bytes = CC1120_TxFIFONumBytes(&CURRENT_SPI);
		
		if (Num_TxFIFO_bytes != TX_FIFO_FAIL) // ���� ����� �� ���������� �� 0xFF, 
			{			
		UART_Tx_Buff[2] = Num_TxFIFO_bytes; // ���������� ������ � TX FIFO
			} 
	else 
			{
				UART_Tx_Buff[2] = TX_FIFO_FAIL; // ��������� �� ������ ������ TX FIFO
			}	
		UART_Tx_Buff[0] = CC1120_SELECT; // �������������, ��� ����� �� ����������
		UART_Tx_Buff[1] = 0x03; // ���������� ���� ������ � ������	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}


void TxFIFOFlush (void)
{
		UART_Tx_Buff[2] = CC1120_TxFIFOFlush(&CURRENT_SPI);
		UART_Tx_Buff[0] = CC1120_SELECT; // �������������, ��� ����� �� ����������
		UART_Tx_Buff[1] = 0x03; // ���������� ���� ������ � ������	
		GUI_Tx (&CURRENT_UART, UART_Tx_Buff);
}


void TxFIFOWrite (uint8_t *data_ptr, uint8_t num_byte)
{
		UART_Tx_Buff[2] = CC1120_TxFIFOWrite(&CURRENT_SPI, data_ptr, num_byte);
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

		case STATUS_CC1120: // ������ ���������� �����
		
		STATUSCC1120Read ();
		
		break;

		case CC1120_TX: // ������ ���������� � ����� ��������
		
		CC1120TxSet();
		
		break;
		
		case CC1120_IDLE: // ������ ���������� � ����� IDLE
		
		CC1120IDLESet();
		
		break;
		
		case CC1120_RX: // ������ ���������� � ����� ������
		
		CC1120RxSet();
		
		break;
		
		case CC1120_RESET: // ����� ����������
		
		CC1120Reset();
		
		break;
		
		case CC1120_FIFO_NUM_TXBYTES: // ������ ���������� ������ � TX FIFO
		
		TxFIFONumBytesRead();
		
		break;
		
		case CC1120_TX_FIFO_FLUSH: // ������ ���������� ������ � TX FIFO
		
		TxFIFOFlush();
		
		break;
		
		case CC1120_TX_FIFO_WRITE: // ������ ���������� ������ � TX FIFO
		
		TxFIFOWrite (command, Payload_length);
		
		break;
		
	}
	
	return 0;
}



	void GUI_rx (UART_HandleTypeDef *huart) // ����� ������ �� UART
{
 
	 if ( HAL_UART_GetState(huart)== HAL_UART_STATE_READY) 
	 {	
					HAL_UART_Receive_DMA(huart, UART_Rx_Buff,0x02);
					while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) // ������� ��������� ������
					{
					}
					ARM_command = UART_Rx_Buff [0]; // ������� ��������������� (����� ����������)
					Payload_length = UART_Rx_Buff [1]; // ���������� ������ ������ ��������
	 }

	
	if ( HAL_UART_GetState(huart)== HAL_UART_STATE_READY) 
	 {
					HAL_UART_Receive_DMA(huart, UART_Rx_Buff,Payload_length); // ��������� ������		
					while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) // ������� ��������� ������
					{
					}	
					Device_Select (ARM_command);
	 }
	
}


	void GUI_Tx (UART_HandleTypeDef *huart, uint8_t *tx_buffer) // �������� ������ �� UART
{
	
	if ( HAL_UART_GetState(huart)== HAL_UART_STATE_READY) 
	{
						HAL_UART_Transmit_DMA(huart, tx_buffer, tx_buffer[1]);
						
						while (HAL_UART_GetState(huart) != HAL_UART_STATE_READY) // ������� ��������� ��������
						{
						}
	}
}



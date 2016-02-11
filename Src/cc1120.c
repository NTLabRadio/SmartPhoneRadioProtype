#include "cc1120.h"
#include "uart_gui.h"

SPI_HandleTypeDef *hspiCC1120 = NULL;

uint8_t pCC1120TxData[255];
uint8_t pCC1120RxData[255];




/**
  * @brief  ������ ChipID ���������� CC1120
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI, ����� �� ���������� �� ���)
  */
uint8_t CC1120_CheckModule(SPI_HandleTypeDef *hspi) 
{
	hspiCC1120 = hspi;
	
	
	//�������� CS	
	CC1120_CSN_LOW();
		
		
if (CC1120_Read (EXT_PARTNUMBER, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01))	// ���� ���� ������ ������ ������� �� SPI ���������� ������ �������
			{
				return 0;
			}
	
	
	//�������� 100 ���. ����� ������ ��� �������� �� SPI 6 ���� � �������� ���� 500 ���
	WaitTimeMCS(1e2);
	
	//��������� CS	
	CC1120_CSN_HIGH();
	
	//������ ������� � ����� 2 �������� 0x48, ��������������� ChipID ���������� CC1120
	if(pCC1120RxData[2]!=CC1120_ID)
		return 0;	
	
	return(1);
}



/**
  * @brief  ������ ������ firmware ���������� CC1120
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					����� ������ firmware - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
  */
uint8_t CC1120_CheckVersion(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Read (EXT_PARTVERSION, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01))	// ���� ���� ������ ������ ������� �� SPI ���������� ������ �������
			{
				return 0;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
		return pCC1120RxData[2];	
}




/**
  * @brief  ������ ���������� ����� ���������� CC1120
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					0x00 (IDLE)- �������� ����������;
	*					0x01 (RX)- �������� ����������;
	*					0x02 (TX)- �������� ����������;
	*					0x03 (FSTXON)- �������� ����������;
	*					0x04 (CALIBRATE)- �������� ����������;
	*					0x05 (SETTLING)- �������� ����������;
	*					0x06 (RX FIFO ERROR)- �������� ����������;
	*					0x07 (TX FIFO ERROR)- �������� ����������;
	*					0x08 - ������ ��� ���������� ������� (������ ���� SPI)
  */
uint8_t CC1120_Status(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	//�������� CS	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_STATUS, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
			{
				return STATUS_SPI_ERROR;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	// � ����� ������� �������� ������� 4 ����
	return (pCC1120RxData[0]>>4);
}


/**
  * @brief  ������� ���������� CC1120 � ����� ��������
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
  */
uint8_t CC1120_Tx(SPI_HandleTypeDef *hspi)
{
hspiCC1120 = hspi;
	
	//�������� CS	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_TX, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
			{
				return 0;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}


/**
  * @brief  ������� ���������� CC1120 � ����� ����������� IDLE
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
  */
uint8_t CC1120_IDLE_set(SPI_HandleTypeDef *hspi)
{
hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_IDLE, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
			{
				return 0;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}


/**
  * @brief  ������� ���������� CC1120 � ����� ������
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
  */
uint8_t CC1120_Rx(SPI_HandleTypeDef *hspi)
{
hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_RX, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
			{
				return 0;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}




















/**
  * @brief  ������� ������ ������ (�������� ���������/������ ��� FIFO-�������) � CC1120
	* @param  uGenAddress - ����� �������� ������� (0x00-0x3F):
	*												0x00-0x2E - ���������������� ��������, 
	*												0x2F - ����� ����� ��� ��������� ������������ ��������� ������������,
	*												0x30-0x3D - �������� ���� Command Strobe,
	*												0x3E - ����� ����� ������� ������� � ������ FIFO-�������,
	*												0x3F - ����� ������ TX FIFO;
	*					uExtAddress - �������������� ����� ������������ ��������� ������������:
	*												(�������� �������� �������� ������ ���� uGenAddress==0x2F ��� 
	*												uGenAddress==0x3E);
	*					bBurst - ����, ����������� �� ��� ��������: ���������(0)/��������(1) (single(0)/burst(1));
	*					data_ptr - ��������� �� ������, ������� ���������� ��������;
	*					uAccesses - ������ ������ (����� ����), ������� ���������� ��������
  * @note   
	* @retval ��������� ���������� �������:
	*					0 - �������� ����������;
	*					����� - ������ ��� ���������� ������� (������ ���� SPI, ������������ �������� ������� ������)
  */
ReadWriteRegTypeDef CC1120_Write (uint8_t uGenAddress, uint8_t uExtAddress, uint8_t bBurst, uint8_t *data_ptr, uint16_t uAccesses)
{
	uint8_t buff_index = 1; // ������ ������ ������
	uint8_t buff_count = 0; // ������ ������ ������
	
	
	switch (uExtAddress) // ������������ ������� ����� ������ ��� ��������������� ������
	{
		case EXT_ADDRESS:
			pCC1120TxData[0] = EXT_ADDRESS;
			pCC1120TxData[1] = uGenAddress;
			buff_index++;
		break;

		case REG_DMA:
			pCC1120TxData[0] = REG_DMA;
			pCC1120TxData[1] = uGenAddress;
			buff_index++;
		break;

		case REG_ADDRESS:
			pCC1120TxData[0] = uGenAddress;
		break;

    default:
			
			return (DataInMismatch); // ���� ��������� �������� ���� �������� �� ��������, �� ��������������, �� DMA 
		
		break;		
	
	}
	
	pCC1120TxData[0] = pCC1120TxData[0] | (WRITE_CC1120 << 7); // ��������� �������� ������ (����, ��� �� 0. ��� ��� ������������)
	
	if (bBurst) 
	{
		pCC1120TxData[0] = pCC1120TxData[0] |  (BURST << 6); // ��������� ������ �������� ������ 
	}		
	
	for (buff_count = buff_index; buff_count < uAccesses; buff_count++) // ������ ������ �� ��������� �� �������� ����.
		{
				pCC1120TxData[buff_count] = data_ptr[buff_count-buff_index];
		}
	//�������� ������ � ������������ ��������� �����
	if (SPI_TransmitRecieve(hspiCC1120, pCC1120TxData, pCC1120RxData, uAccesses+buff_index)) 
	{
			return (SPIBusy);
	} else {
			return (ReadWriteOk);
	}	
}




/**
  * @brief  ������� ������ ������ (�������� ���������/������ FIFO-�������) �� CC1120
	* @param  uGenAddress - ����� �������� ������� (0x00-0x3F):
	*												0x00-0x2E - ���������������� ��������, 
	*												0x2F - ����� ����� ��� ��������� ������������ ��������� ������������,
	*												0x30-0x3D - �������� ���� Command Strobe,
	*												0x3E - ����� ����� ������� ������� � ������ FIFO-�������,
	*												0x3F - ����� ������ RX FIFO;
	*					uExtAddress - �������������� ����� ������������ ��������� ������������:
	*												(�������� �������� �������� ������ ���� uGenAddress==0x2F ��� 
	*												uGenAddress==0x3E);
	*					bBurst - ����, ����������� �� ��� ��������: ���������(0)/��������(1) (single(0)/burst(1));
	*					data_ptr - ��������� �� ����������� ������;
	*					uAccesses - ������ ������ (����� ����), ������� ���������� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					0 - �������� ����������;
	*					����� - ������ ��� ���������� ������� (������ ���� SPI, ������������ �������� ������� ������)
  */
ReadWriteRegTypeDef CC1120_Read (uint8_t uGenAddress, uint8_t uExtAddress, uint8_t bBurst, uint8_t *data_ptr, uint8_t uAccesses)
{
	uint8_t buff_index = 1; // ������ ������ ������
	uint8_t buff_count = 0; // ������ ������ ������
	
	
	switch (uExtAddress) // ������������ ������� ����� ������ ��� ��������������� ������
	{
		case EXT_ADDRESS:
			pCC1120TxData[0] = EXT_ADDRESS;
			pCC1120TxData[1] = uGenAddress;
			buff_index++;
		break;

		case REG_DMA:
			pCC1120TxData[0] = REG_DMA;
			pCC1120TxData[1] = uGenAddress;
			buff_index++;
		break;

		case REG_ADDRESS:
			pCC1120TxData[0] = uGenAddress;
		break;

    default:
			
			return (DataInMismatch); // ���� ��������� �������� ���� �������� �� ��������, �� ��������������, �� DMA 
		
		break;		
	
	}
	
	pCC1120TxData[0] = pCC1120TxData[0] | (READ_CC1120 << 7); // ��������� �������� ������ (����, ��� �� 0. ��� ��� ������������)
	
	if (bBurst) 	
		{	
			pCC1120TxData[0] = pCC1120TxData[0] |  (BURST << 6); // ��������� ������ �������� ������ 
		}
		
	for (buff_count = buff_index; buff_count < uAccesses; buff_count++) // ������ ������ �� ��������� �� �������� ����.
		{
				pCC1120TxData[buff_count] = data_ptr[buff_count-buff_index];
		}
	//�������� ������ � ������������ ��������� �����
	if (SPI_TransmitRecieve(hspiCC1120, pCC1120TxData, pCC1120RxData, uAccesses+buff_index)) 
	{
			return (SPIBusy);
	} else {
			return (ReadWriteOk);
	}	
}

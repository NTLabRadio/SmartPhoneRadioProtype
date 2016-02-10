#include "cc1120.h"

SPI_HandleTypeDef *hspiCC1120 = NULL;

uint8_t pCC1120TxData[255];
uint8_t pCC1120RxData[255];


uint8_t CC1120_CheckModule(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
//	pCC1120TxData[0] = 0xAF;	pCC1120TxData[1] = 0x8F; pCC1120TxData[2] = 0x00;
//	pCC1120TxData[3] = 0xAF;	pCC1120TxData[4] = 0x90; pCC1120TxData[5] = 0x00;	
	
	//�������� CS	
	CC1120_CSN_LOW();
	
	//�������� ������ � ������������ ��������� �����
	//SPI_TransmitRecieve(hspiCC1120, pCC1120TxData, pCC1120RxData, 6);
	
	CC1120_Read (EXT_PARTNUMBER, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01);
	
	
	
	// �� ������ ������� ��������� ������ ������ �� SPI !!!!!
	
	//�������� 100 ���. ����� ������ ��� �������� �� SPI 6 ���� � �������� ���� 500 ���
	WaitTimeMCS(1e2);
	
	//��������� CS	
	CC1120_CSN_HIGH();
	
	//������ ������� � ����� 2 �������� 0x48, ��������������� ChipID ���������� CC1120
	if(pCC1120RxData[2]!=0x48)
		return 0;	
	
	return(1);
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

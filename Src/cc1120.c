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
  * @brief  ����� ���������� CC1120
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
  */
uint8_t CC1120_Reset(SPI_HandleTypeDef *hspi)
{
hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_RESET, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
			{
				return 0;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}


/**
  * @brief  ������ ���������� ������ � FIFO TX ���������� CC1120
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					���������� ������ � ������ - �������� ����������;
	*					0xFF - ������ ��� ���������� ������� (������ ���� SPI)
  */
uint8_t CC1120_TxFIFONumBytes(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Read (EXT_NUM_TXBYTES, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01))	// ���� ���� ������ ������ ������� �� SPI ���������� ������ �������
			{
				return TX_FIFO_FAIL;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
		return pCC1120RxData[2];	
}

/**
  * @brief  ������� FIFO TX ���������� CC1120
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
  */
uint8_t CC1120_TxFIFOFlush(SPI_HandleTypeDef *hspi)
{
hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_TX_FIFO_FLUSH, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
			{
				return 0;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}

/**
  * @brief  ������ ������ � FIFO TX ���������� CC1120
	* @param  *hspi - ����� ���������� SPI ��� ���������
	*					fifo_write_data_ptr - ��������� �� ������ ��� ������ � TX FIFO;
  * @note   
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
  */

uint8_t CC1120_TxFIFOWrite(SPI_HandleTypeDef *hspi, uint8_t *fifo_write_data_ptr, uint8_t tx_num)
{
		hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	//fifo_write_data_ptr 0 ���� - ������� ����������, 1 ���� � ����� - ������ FIFO TX
	// ������� 0 ������� � �������� ��� ������ �� ���� �����

	for (uint8_t i = 0; i<(tx_num); i++)
	{
		fifo_write_data_ptr[i] = fifo_write_data_ptr[i+1];
	}
	
	if (CC1120_Write (R_TX_FIFO_WRITE, REG_ADDRESS, NO_BURST, fifo_write_data_ptr,tx_num-1))
			{
				return 0;
			}
	
	WaitTimeMCS(10e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}

/**
  * @brief  ���������� PLL ���������� CC1120
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
  */

uint8_t CC1120_ManualCalibration(SPI_HandleTypeDef *hspi)
{
	uint8_t original_fs_cal2 = 0;
	uint8_t original_fs_cal2_new = 0;
	uint8_t calResult_for_vcdac_start_high[3];
	uint8_t calResult_for_vcdac_start_mid[3];
	hspiCC1120 = hspi;
	
	// ������ �������� FS_VCO2 = 0x00
	CC1120_CSN_LOW();
	pCC1120TxData[0] = 0x00;
	if (CC1120_Write (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			{
				return 0;
			}
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();		
	
	// ������ FS_CAL2 (VCDAC_START)
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_CAL2, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
			{
				return 0;
			}
	original_fs_cal2 = pCC1120RxData [2];
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();	
			
	 // ������ FS_CAL2 = original_fs_cal2 + 2		
	original_fs_cal2_new = original_fs_cal2 +2;
	pCC1120TxData[0] = original_fs_cal2_new;
	CC1120_CSN_LOW();		
	if (CC1120_Write (EXT_FS_CAL2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			{
				return 0;
			}		
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
			
	// ������ ���������� �����������
	CC1120_CSN_LOW();
	if (CC1120_Write (S_CAL, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
			{
				return 0;
			}
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
			
	// ������ FS_VCO2 � ���������� calResults_for_vcdac_start_high		
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
			{
				return 0;
			}
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	calResult_for_vcdac_start_high[0] = pCC1120RxData[2];	
			

	// ������ FS_VCO4 � ���������� calResults_for_vcdac_start_high	 FS_CHP 
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_VCO4, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
			{
				return 0;
			}
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	calResult_for_vcdac_start_high[1] = pCC1120RxData[2];			
			
	// ������ FS_CHP � ���������� calResults_for_vcdac_start_high
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_CHP, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
			{
				return 0;
			}
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	calResult_for_vcdac_start_high[2] = pCC1120RxData[2];	
			
	// ������ �������� FS_VCO2 = 0x00
	CC1120_CSN_LOW();
	pCC1120TxData[0] = 0x00;
	if (CC1120_Write (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			{
				return 0;
			}
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();

	// ������ FS_CAL2 = original_fs_cal2		
	pCC1120TxData[0] = original_fs_cal2;
	CC1120_CSN_LOW();		
	if (CC1120_Write (EXT_FS_CAL2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			{
				return 0;
			}		
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	
	// ������ ���������� �����������
	CC1120_CSN_LOW();
	if (CC1120_Write (S_CAL, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
			{
				return 0;
			}
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	
	// ������ FS_VCO2 � ���������� calResults_for_vcdac_start_mid		
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
			{
				return 0;
			}
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	calResult_for_vcdac_start_mid[0] = pCC1120RxData[2];

	// ������ FS_VCO4 � ���������� calResults_for_vcdac_start_mid
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_VCO4, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
			{
				return 0;
			}
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	calResult_for_vcdac_start_mid[1] = pCC1120RxData[2];			
	
	
	// ������ FS_CHP � ���������� calResults_for_vcdac_start_mid
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_CHP, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
			{
				return 0;
			}
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	calResult_for_vcdac_start_mid[2] = pCC1120RxData[2];

	// ��������� calResults_for_vcdac_start_high > calResults_for_vcdac_start_mid?
			
	if (calResult_for_vcdac_start_high[0] > calResult_for_vcdac_start_mid [0])
	{
		// ������ FS_VCO2, FS_VCO4, � FS_CHP ����������� � calResults_for_vcdac_start_high
	pCC1120TxData[0] = calResult_for_vcdac_start_high[0];
	CC1120_CSN_LOW();		
	if (CC1120_Write (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			{
				return 0;
			}		
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
		
	pCC1120TxData[0] = calResult_for_vcdac_start_high[1];
	CC1120_CSN_LOW();		
	if (CC1120_Write (EXT_FS_VCO4, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			{
				return 0;
			}		
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
			
	pCC1120TxData[0] = calResult_for_vcdac_start_high[2];
	CC1120_CSN_LOW();		
	if (CC1120_Write (EXT_FS_CHP, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			{
				return 0;
			}		
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();		
	
	}
	else
	{
		// ������ FS_VCO2, FS_VCO4, � FS_CHP ����������� � calResults_for_vcdac_start_mid
	pCC1120TxData[0] = calResult_for_vcdac_start_mid[0];
	CC1120_CSN_LOW();		
	if (CC1120_Write (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			{
				return 0;
			}		
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
		
	pCC1120TxData[0] = calResult_for_vcdac_start_mid[1];
	CC1120_CSN_LOW();		
	if (CC1120_Write (EXT_FS_VCO4, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			{
				return 0;
			}		
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
			
	pCC1120TxData[0] = calResult_for_vcdac_start_mid[2];
	CC1120_CSN_LOW();		
	if (CC1120_Write (EXT_FS_CHP, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			{
				return 0;
			}		
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	}
	
	return (1);	
			
}


/**
  * @brief  ��������� ������� ���������� CC1120
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*						MARC state:
	*						0x00 - SLEEP
	*						0x01 - IDLE
	*						0x02 - XOFF
	*						0x03 - BIAS_SETTLE_MC
	*						0x04 - REG_SETTLE_MC
	*						0x05 - MANCAL
	*						0x06 - BIAS_SETTLE
	*						0x07 - REG_SETTLE
	*						0x08 - STARTCAL
	*						0x09 - BWBOOST
	*						0x0A - FS_LOCK
	*						0x0B - IFADCON
	*						0x0C - ENDCAL
	*						0x0D - RX
	*						0x0E - RX_END
	*						0x0F - Reserved
	*						0x10 - TXRX_SWITCH
	*						0x11 - RX_FIFO_ERR
	*						0x12 - FSTXON
	*						0x13 - TX
	*						0x14 - TX_END
	*						0x15 - RXTX_SWITCH
	*						0x16 - TX_FIFO_ERR
	*						0x17 - IFADCON_TXRX
	* 					0x18 - SPI_ERROR
	*					 - ������ ��� ���������� ������� (������ ���� SPI)
  */

CC1120MARCSTATETypeDef CC1120_MARCState(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	CC1120_CSN_LOW();		
	if (CC1120_Read (EXT_MARCSTATE, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
			{
				return MARCSTATE_SPI_ERROR;
			}		
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
			
	return ((pCC1120RxData [2]) & 0x1F);		
}

/**
  * @brief  �������������� ���������� PLL ���������� CC1120
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
  */

 uint8_t CC1120_SFSTXON_set (SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_SFSTXON, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
			{
				return 0;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}	

/**
  * @brief  ������ ���������� ������ � FIFO RX ���������� CC1120
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					���������� ������ � ������ - �������� ����������;
	*					0xFF - ������ ��� ���������� ������� (������ ���� SPI)
  */
uint8_t CC1120_RxFIFONumBytes(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Read (EXT_NUM_RXBYTES, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01))	// ���� ���� ������ ������ ������� �� SPI ���������� ������ �������
			{
				return RX_FIFO_FAIL;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
		return pCC1120RxData[2];	
}

/**
  * @brief  ������� FIFO RX ���������� CC1120
	* @param  *hspi - ����� ���������� SPI ��� ���������
  * @note   
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
  */
uint8_t CC1120_RxFIFOFlush(SPI_HandleTypeDef *hspi)
{
hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_RX_FIFO_FLUSH, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
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
	}
	
	pCC1120TxData[0] = pCC1120TxData[0] | (WRITE_CC1120 << 7); // ��������� �������� ������ (����, ��� �� 0. ��� ��� ������������)
	
	if (bBurst) 
	{
		pCC1120TxData[0] = pCC1120TxData[0] |  (BURST << 6); // ��������� ������ �������� ������ 
	}		
	
	for (buff_count = buff_index; buff_count < uAccesses+1; buff_count++) // ������ ������ �� ��������� �� �������� ����.
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
			
	
	}
	
	pCC1120TxData[0] = pCC1120TxData[0] | (READ_CC1120 << 7); // ��������� �������� ������
	
	if (bBurst) 	
		{	
			pCC1120TxData[0] = pCC1120TxData[0] |  (BURST << 6); // ��������� ������ �������� ������ 
		}
		
	for (buff_count = buff_index; buff_count < uAccesses; buff_count++) // ������ ������ �� ��������� �� �������� ����.
		{
				pCC1120TxData[buff_count] = data_ptr[buff_count-buff_index];
		}
	//�������� ������ � ������������ ��������� �����
	if (SPI_TransmitRecieve(hspiCC1120, pCC1120TxData, data_ptr, uAccesses+buff_index)) 
	{
			return (SPIBusy);
	} else {
			return (ReadWriteOk);
	}	
}

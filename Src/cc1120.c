#include "cc1120.h"


SPI_HandleTypeDef *hspiCC1120 = NULL;

uint8_t pCC1120TxData[256];
uint8_t pCC1120RxData[256];

extern uint8_t g_flCC1120_IRQ_CHECKED;



/**
	* @brief	������ ChipID ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
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
		return 0;
	
	//�������� 100 ���. ����� ������ ��� �������� �� SPI 6 ���� � �������� ���� 500 ���
	WaitTimeMCS(1e2);
	
	//��������� CS	
	CC1120_CSN_HIGH();
	
	//���������, ������������� �� �������� �������� ChipID ���������� CC1120
	if(pCC1120RxData[0]!=CC1120_ID)
		return 0;	
	
	return(1);
}


uint16_t CC1120_Init(CC1120_TypeDef *pCC1120, SPI_HandleTypeDef *hspi)
{
	pCC1120->hSPI = hspi;
	
	pCC1120->TxState = CC1120_TX_STATE_STANDBY;
	
	//0. Pin Reset ���������� ������������� � ������� ���������
	CC1120_RESET_HIGH();	
	
	//1. Reset ����������
	CC1120_Reset(hspi);

	//2. ����� ����� ���������� ����� ��������/������ ������ ������
	g_flCC1120_IRQ_CHECKED = FALSE;
		
	WaitTimeMCS(10e2);
	
	//3. ������� ���������� � ����� IDLE
	CC1120_IDLE_set(hspi);
		
	//4. ������� Tx FIFO
	CC1120_TxFIFOFlush(hspi);
		
	//5. ������� Rx FIFO
	CC1120_RxFIFOFlush(hspi);

	//6. �������� ������������
	CC1120_ConfigWrite(hspi, CC1120_Config_4800, sizeof (CC1120_Config_4800)/sizeof(registerSetting_t));
		
	WaitTimeMCS(1e2);
		
	//7. �������� ������������
	CC1120_ConfigReadCompare(hspi, CC1120_Config_4800, sizeof (CC1120_Config_4800)/sizeof(registerSetting_t));
		
	//TODO ����� ������ ���� ������� ��������� ������� � �������� ����������� �������
	
	//9. ������ �������
	CC1120_Status(hspi);
	
	//10. ���������� PLL
	CC1120_ManualCalibration(hspi);

	WaitTimeMCS(1e2);
	
	//11. ��������� ���������� PLL
	CC1120_ManualCalibration(hspi);
	
	WaitTimeMCS(1e2);	
	
	return(1);
}

/* 			������� �������� ��� �������� ������:
			 1. ������� Tx FIFO CC1120_TxFIFOFlush
			 2. �������� ������� ������ � Tx FIFO CC1120_TxFIFOWrite
			 3. ������ ���������� ������ � Tx FIFO � ��������� � ������������� CC1120_TxFIFONumBytes ��� 
			 4. ������ ������� MARCSTATE (�������� �� ������ Tx FIFO) CC1120_MARCState
			 5. ������� � ����� Tx
			 6. �������� �� ������ �� �������� (�������) CC1120_MARCState
			 7. ��������� ���������� � �������� ����� ������ flCC1120_IRQ_CHECKED = TRUE
			 8. ����� ���������� ����� �������� ������ flCC1120_IRQ_CHECKED = FALSE
			 9. ������ �������. ������������� �������� � IDLE CC1120_Status
 */
uint16_t CC1120_TxData(CC1120_TypeDef *pCC1120, uint8_t* pDataBuf, uint16_t sizeBuf)
{
	//0. ������� � ����� IDLE
	//NO: ���� �� ���?
	CC1120_IDLE_set(pCC1120->hSPI); 
	
	//1. ������� Tx FIFO
	CC1120_TxFIFOFlush(pCC1120->hSPI);
		
	//2. �������� ������� ������ � Tx FIFO
	CC1120_TxFIFOWrite(pCC1120->hSPI, pDataBuf, sizeBuf);
				
	//3. �������� ������� ������ � TX FIFO
	CC1120_TxFIFONumBytes(pCC1120->hSPI);
				
	//4. ������� � ����� ��������
	CC1120_Tx(pCC1120->hSPI);
			
	//WaitTimeMCS(1e3);
	//CC1120_MARCState(pCC1120->hSPI);
	
	return(1);
}


uint16_t CC1120_RxData(CC1120_TypeDef *pCC1120, uint8_t* pDataBuf, uint16_t* sizeBuf)
{
	//��� ����� ���� �������� ��������� CC1120_MARCState() � ������������ ������� �������� ������ CC1120_RxFIFONumBytes()
	
	*sizeBuf = CC1120_RxFIFONumBytes(pCC1120->hSPI);
	
	//������ ������� ������ �� ��1120
	CC1120_RxFIFORead(pCC1120->hSPI);
	
	//������� �������� � ����� ������
	CC1120_Rx(pCC1120->hSPI);
	
	return(1);
}


/**
	* @brief	������ ������ firmware ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
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
		return 0;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return pCC1120RxData[0];	
}




/**
	* @brief	������ ���������� ����� ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
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
CC1120STATUSTypeDef CC1120_Status(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	//�������� CS	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_STATUS, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
		return STATUS_SPI_ERROR;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	// � ����� ������� �������� ������� 4 ����
	return ((CC1120STATUSTypeDef)(pCC1120RxData[0]>>4));
}


/**
	* @brief	������� ���������� CC1120 � ����� ��������
	* @param	*hspi - ����� ���������� SPI ��� ���������
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
	
	if (CC1120_Write (S_TX, REG_ADDRESS, NO_BURST, pCC1120TxData, 0x00))
		return 0;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}


/**
	* @brief	������� ���������� CC1120 � ����� ����������� IDLE
	* @param	*hspi - ����� ���������� SPI ��� ���������
	* @note	 
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
	*/
uint8_t CC1120_IDLE_set(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_IDLE, REG_ADDRESS, NO_BURST, pCC1120TxData, 0x00))
		return 0;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}


/**
	* @brief	������� ���������� CC1120 � ����� ������
	* @param	*hspi - ����� ���������� SPI ��� ���������
	* @note	 
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
	*/
uint8_t CC1120_Rx(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_RX, REG_ADDRESS, NO_BURST, pCC1120TxData, 0x00))
		return 0;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}


/**
	* @brief	����� ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
	* @note	 
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
	*/
uint8_t CC1120_Reset(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_RESET, REG_ADDRESS, NO_BURST, pCC1120TxData, 0x00))
		return 0;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}


/**
	* @brief	������ ���������� ������ � FIFO TX ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
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
		return TX_FIFO_FAIL;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return pCC1120RxData[0];
}

/**
	* @brief	������� FIFO TX ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
	* @note	 
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
	*/
uint8_t CC1120_TxFIFOFlush(SPI_HandleTypeDef *hspi)
{
hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_TX_FIFO_FLUSH, REG_ADDRESS, NO_BURST, pCC1120TxData, 0x00))
		return 0;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}

/**
	* @brief	������ ������ � FIFO TX ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
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
	
	if (CC1120_Write (R_ST_FIFO_ACCESS, REG_ADDRESS, BURST, fifo_write_data_ptr,tx_num))
		return 0;
	
	WaitTimeMCS(10e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}

/**
	* @brief	���������� PLL ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
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
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();		
	
	// ������ FS_CAL2 (VCDAC_START)
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_CAL2, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	original_fs_cal2 = pCC1120RxData [0];
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();	
			
	// ������ FS_CAL2 = original_fs_cal2 + 2		
	original_fs_cal2_new = original_fs_cal2 +2;
	pCC1120TxData[0] = original_fs_cal2_new;
	
	CC1120_CSN_LOW();		
	if (CC1120_Write (EXT_FS_CAL2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
			
	// ������ ���������� �����������
	CC1120_CSN_LOW();
	if (CC1120_Write (S_CAL, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
			
	WaitTimeMCS(10e2);
			
	// ������ FS_VCO2 � ���������� calResults_for_vcdac_start_high		
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	
	calResult_for_vcdac_start_high[0] = pCC1120RxData[0];	
	
	// ������ FS_VCO4 � ���������� calResults_for_vcdac_start_high	 FS_CHP 
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_VCO4, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	
	calResult_for_vcdac_start_high[1] = pCC1120RxData[0];			
			
	// ������ FS_CHP � ���������� calResults_for_vcdac_start_high
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_CHP, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	
	calResult_for_vcdac_start_high[2] = pCC1120RxData[0];	
			
	// ������ �������� FS_VCO2 = 0x00
	CC1120_CSN_LOW();
	pCC1120TxData[0] = 0x00;
	if (CC1120_Write (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();

	// ������ FS_CAL2 = original_fs_cal2		
	pCC1120TxData[0] = original_fs_cal2;
	CC1120_CSN_LOW();		
	if (CC1120_Write (EXT_FS_CAL2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	
	// ������ ���������� �����������
	CC1120_CSN_LOW();
	if (CC1120_Write (S_CAL, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
					
	WaitTimeMCS(10e2);
	
	// ������ FS_VCO2 � ���������� calResults_for_vcdac_start_mid		
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	calResult_for_vcdac_start_mid[0] = pCC1120RxData[0];

	// ������ FS_VCO4 � ���������� calResults_for_vcdac_start_mid
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_VCO4, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	calResult_for_vcdac_start_mid[1] = pCC1120RxData[0];			
	
	// ������ FS_CHP � ���������� calResults_for_vcdac_start_mid
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_CHP, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	calResult_for_vcdac_start_mid[2] = pCC1120RxData[0];

	// ��������� calResults_for_vcdac_start_high > calResults_for_vcdac_start_mid?
	if (calResult_for_vcdac_start_high[0] > calResult_for_vcdac_start_mid [0])
	{
		// ������ FS_VCO2, FS_VCO4, � FS_CHP ����������� � calResults_for_vcdac_start_high
		pCC1120TxData[0] = calResult_for_vcdac_start_high[0];
		
		CC1120_CSN_LOW();		
		if (CC1120_Write (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			return 0;
		WaitTimeMCS(1e2);
		CC1120_CSN_HIGH();
		
		pCC1120TxData[0] = calResult_for_vcdac_start_high[1];
		CC1120_CSN_LOW();		
		if (CC1120_Write (EXT_FS_VCO4, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			return 0;
		WaitTimeMCS(1e2);
		CC1120_CSN_HIGH();
			
		pCC1120TxData[0] = calResult_for_vcdac_start_high[0];
		CC1120_CSN_LOW();		
		if (CC1120_Write (EXT_FS_CHP, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			return 0;
		WaitTimeMCS(1e2);
		CC1120_CSN_HIGH();		
	}
	else
	{
		// ������ FS_VCO2, FS_VCO4, � FS_CHP ����������� � calResults_for_vcdac_start_mid
		pCC1120TxData[0] = calResult_for_vcdac_start_mid[0];
		CC1120_CSN_LOW();		
		if (CC1120_Write (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			return 0;
		WaitTimeMCS(1e2);
		CC1120_CSN_HIGH();
		
		pCC1120TxData[0] = calResult_for_vcdac_start_mid[1];
		CC1120_CSN_LOW();		
		if (CC1120_Write (EXT_FS_VCO4, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			return 0;
		WaitTimeMCS(1e2);
		CC1120_CSN_HIGH();
			
		pCC1120TxData[0] = calResult_for_vcdac_start_mid[2];
		CC1120_CSN_LOW();		
		if (CC1120_Write (EXT_FS_CHP, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
			return 0;
		WaitTimeMCS(1e2);
		CC1120_CSN_HIGH();
	}
	
	return (1);			
}


/**
	* @brief	��������� ������� ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
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
		return MARCSTATE_SPI_ERROR;
		
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
			
	return ((CC1120MARCSTATETypeDef)(pCC1120RxData[0] & 0x1F));		
}

/**
	* @brief	�������������� ���������� PLL ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
	* @note	 
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
	*/

uint8_t CC1120_SFSTXON_set (SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_SFSTXON, REG_ADDRESS, NO_BURST, pCC1120TxData, 0x00))
		return 0;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}	

/**
	* @brief	������ ���������� ������ � FIFO RX ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
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
		return RX_FIFO_FAIL;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return pCC1120RxData[0];	
}


/**
	* @brief	������� FIFO RX ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
	* @note	 
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
	*/
uint8_t CC1120_RxFIFOFlush(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_RX_FIFO_FLUSH, REG_ADDRESS, NO_BURST, pCC1120TxData, 0x00))
		return 0;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}


/**
	* @brief	������ ������������ � ��������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
	*	@param	*Config -��������� �� ������ ������ ������������
	*	@param	*configRegNum -���������� ������ � �������
	* @note	 
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
	*/
uint8_t CC1120_ConfigWrite(SPI_HandleTypeDef *hspi, const registerSetting_t *CC1120_Config, uint8_t configRegNum)
{
	hspiCC1120 = hspi;

	uint8_t	writeExtAddress; // �������������� ����� ��������
	uint8_t	writeAddress; // ����� ��������
	
	CC1120_CSN_LOW();
	
	// ���� ������ ���������
	for (uint16_t i=0; i<configRegNum; i++)
	{
		pCC1120TxData[0] = CC1120_Config[i].data; // �������� ��������
		writeExtAddress = ((CC1120_Config[i].addr & 0xFF00)>>8);
		writeAddress = CC1120_Config[i].addr & 0xFF;
		
		if (writeExtAddress) 
		{
			if (CC1120_Write (writeAddress, EXT_ADDRESS, NO_BURST, pCC1120TxData, 0x01)) // ���� ����������� ����� �� 0, �� ����� � ����������� �������
				return 0;
		}
		else
		{
			if (CC1120_Write (writeAddress, REG_ADDRESS, NO_BURST, pCC1120TxData, 0x01)) // ���� �������������� ����� 0, �� ����� � �������� �������
				return 0;
		}		
	}
		
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
		
	return (1);
}

/**
	* @brief	��������� ������������ ���������� CC1120 � �������� � ����������
	* @param	*hspi - ����� ���������� SPI ��� ���������
	*	@param	*Config -��������� �� ������ ������ ������������
	*	@param	*configRegNum -���������� ������ � �������
	* @note	 
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					2 - ������ ���������
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
	*/
uint8_t CC1120_ConfigReadCompare(SPI_HandleTypeDef *hspi, const registerSetting_t *CC1120_Config, uint8_t configRegNum)
{
	hspiCC1120 = hspi;
	uint8_t	readExtAddress; // �������������� ����� ��������
	uint8_t	readAddress; // ����� ��������
	
	CC1120_CSN_LOW();
	
	// ���� ������ ���������
	for (uint16_t i=0; i< configRegNum; i++)
	{
		readExtAddress = ((CC1120_Config[i].addr & 0xFF00)>>8);
		readAddress = CC1120_Config[i].addr & 0xFF;
				
		if (readExtAddress) 
		{
			if (CC1120_Read (readAddress, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01)) // ���� ����������� ����� �� 0, �� ����� � ����������� �������
				return 0;
		}
		else
		{
			if (CC1120_Read (readAddress, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x01)) // ���� �������������� ����� 0, �� ����� � �������� �������
				return 0;
		}

		if (pCC1120RxData[0] != CC1120_Config[i].data)
			return (2);
	}
			
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();

	return (1);
}

/**
	* @brief	�������� �������� � ������� ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
	*	@param	addrReg - ����� �������� uint16_t
	*	@param	dataReg - �������� �������� uint8_t
	* @note	 �� ������������� ����������. ��� ������ � ����������� � ������� ������ �� ���������. �� ��������.
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)
	*/
uint8_t CC1120_RegWrite (SPI_HandleTypeDef *hspi, uint16_t addrReg, uint8_t dataReg)
{
	hspiCC1120 = hspi;
	uint8_t writeExtAddress;
	uint8_t writeAddress;
	
	pCC1120TxData[0] = dataReg; // �������� ��������
	writeExtAddress = ((addrReg & 0xFF00)>>8);
	writeAddress = addrReg & 0xFF;
	
	CC1120_CSN_LOW();
	
	if (writeExtAddress) 
	{
		if (CC1120_Write (writeAddress, EXT_ADDRESS, NO_BURST, pCC1120TxData, 0x01)) // ���� ����������� ����� �� 0, �� ����� � ����������� �������
			return 0;
	}
	else
	{
		if (CC1120_Write (writeAddress, REG_ADDRESS, NO_BURST, pCC1120TxData, 0x01)) // ���� �������������� ����� 0, �� ����� � �������� �������
			return 0;
	}
	
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	
	return (1);
}



/**
	* @brief	��������� �������� �� �������� ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
	*	@param	addrReg - ����� �������� uint16_t
	*	@param	dataReg - �������� �������� uint8_t
	* @note		�� ������������� ����������. ��� ������ � ����������� � ������� ������ �� ���������. �� ��������.
	* @retval ��������� ���������� �������:
	*					������ �������� - �������� ����������;
	*					
	*/
uint8_t CC1120_RegRead (SPI_HandleTypeDef *hspi, uint16_t addrReg, uint8_t dataReg)
{
	hspiCC1120 = hspi;
	uint8_t	readExtAddress; // �������������� ����� ��������
	uint8_t	readAddress; // ����� ��������
	
	CC1120_CSN_LOW();
	readExtAddress = ((addrReg & 0xFF00)>>8);
	readAddress = addrReg & 0xFF;

	if (readExtAddress) 
		CC1120_Read (readAddress, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01); // ���� ����������� ����� �� 0, �� ����� � ����������� �������	
	else
		CC1120_Read (readAddress, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x01); // ���� �������������� ����� 0, �� ����� � �������� �������		
	
	return pCC1120RxData[0];
}

/**
	* @brief	�������� ������� � ��������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
	*	@param	*freq - ������ �� ������ �� ���������� �������
	* @note		�� ������������� ����������. ��� ������ � ����������� � ������� ������ �� ���������. �� ��������.
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)		
	*/
uint8_t CC1120_FreqWrite (SPI_HandleTypeDef *hspi, uint8_t *freq)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	/* 	
	 *	���������� � ������ burst. ��������� ����� EXT_FREQ2 (0x0C).
	 *	����� FREQ1 (0x0D) � FREQ0 (0x0E)														 
	*/
	
	if (CC1120_Write (EXT_FREQ2, EXT_ADDRESS, BURST, freq, 0x03))
	{
		return 0;
	}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);

}

/**
	* @brief	��������� ������� �� ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
	*	@param	*freq - ������ �� ������ �� ���������� �������
	* @note		�� ������������� ����������. ��� ������ � ����������� � ������� ������ �� ���������. �� ��������.
	* @retval ��������� ���������� �������:
	*					1 - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)		
	*/
uint8_t *CC1120_FreqRead (SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;

	CC1120_CSN_LOW();
	
	/* 	
	 *	��������� � ������ burst. ��������� ����� EXT_FREQ2 (0x0C).
	 *	����� FREQ1 (0x0D) � FREQ0 (0x0E)														 
	*/
	
	if (CC1120_Read (EXT_FREQ2, EXT_ADDRESS, BURST, pCC1120RxData, 0x03))
		return 0;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (pCC1120RxData);

}

/**
	* @brief	��������� ���������� ������ RX FIFO ���������� CC1120
	* @param	*hspi - ����� ���������� SPI ��� ���������
	* @note		�� ������������� ����������. ��� ������ � ����������� � ������� ������ �� ���������. �� ��������.
	* @retval ��������� ���������� �������:
	*					������ �� ������ �� ���������� - �������� ����������;
	*					0 - ������ ��� ���������� ������� (������ ���� SPI)		
	*/
uint8_t *CC1120_RxFIFORead(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	uint8_t RxFIFONumBytes = 0;
	
	// ������ ���������� ������ ������ � RxFIFO
	RxFIFONumBytes = CC1120_RxFIFONumBytes(hspi);
	if (!RxFIFONumBytes) 
	{
		pCC1120RxData[0] = 0x00;
		return (pCC1120RxData);
	}
	CC1120_CSN_LOW();
	
	if (CC1120_Read (R_ST_FIFO_ACCESS, REG_ADDRESS, BURST, pCC1120RxData, RxFIFONumBytes))
	{
		return 0;
	}
	
	// ����� ������ ������ �� 1 ������ � ������ � 0 ������� ���������� ������ Rx FIFO
	for (uint8_t i = RxFIFONumBytes; (i>0); i--)
			{
				pCC1120RxData[i+1] = pCC1120RxData[i];
			}
			pCC1120RxData[1] = pCC1120RxData[0];
			pCC1120RxData[0] = RxFIFONumBytes;
			
	WaitTimeMCS(10e2);
	
	CC1120_CSN_HIGH();
	
	return (pCC1120RxData);
}



/**
	* @brief	������� ������ ������ (�������� ���������/������ ��� FIFO-�������) � CC1120
	* @param	uGenAddress - ����� �������� ������� (0x00-0x3F):
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
	uint8_t buff_count = 0; // ������ ������ ������
	uint8_t buff_index = 1;
	
	
	switch (uExtAddress) // ������������ ������� ����� ������ ��� ��������������� ������
	{
		case EXT_ADDRESS:
			// �������� ������ �� 2 �������
			for (buff_count = 0; buff_count < uAccesses; buff_count++) // ������ ������ �� ��������� �� �������� ����.
			{
				pCC1120TxData[buff_count+2] = data_ptr[buff_count];
			}
		
			pCC1120TxData[0] = EXT_ADDRESS;
			pCC1120TxData[1] = uGenAddress;
			
			buff_index++;
		break;

		case REG_DMA:
			// �������� ������ �� 2 �������
			for (buff_count = 0; buff_count < uAccesses; buff_count++) // ������ ������ �� ��������� �� �������� ����.
			{
				pCC1120TxData[buff_count+2] = data_ptr[buff_count];
			}
			
			pCC1120TxData[0] = REG_DMA;
			pCC1120TxData[1] = uGenAddress;

			buff_index++;
		break;

		case REG_ADDRESS:
			// �������� ������ �� 1 �������
			for (buff_count = 0; buff_count < uAccesses; buff_count++) // ������ ������ �� ��������� �� �������� ����.
			{
				pCC1120TxData[buff_count+1] = data_ptr[buff_count];
			}
			
			pCC1120TxData[0] = uGenAddress;
		break;

		default:
			return (DataInMismatch); // ���� ��������� �������� ���� �������� �� ��������, �� ��������������, �� DMA 
	}

	
	pCC1120TxData[0] = pCC1120TxData[0] | (WRITE_CC1120 << 7); // ��������� �������� ������ (����, ��� �� 0. ��� ��� ������������)
	
	if (bBurst) 
	{
		pCC1120TxData[0] = pCC1120TxData[0] |	(BURST << 6); // ��������� ������ �������� ������ 
	}		
	
	//�������� ������ � ������������ ��������� �����
	if (SPI_TransmitRecieve(hspiCC1120, pCC1120TxData, pCC1120RxData, uAccesses+buff_index)) 
		return (SPIBusy);
	else
		return (ReadWriteOk);

}




/**
	* @brief	������� ������ ������ (�������� ���������/������ FIFO-�������) �� CC1120
	*
	* @param	uGenAddress - ����� �������� ������� (0x00-0x3F):
	*												0x00-0x2E - ���������������� ��������, 
	*												0x2F - ����� ����� ��� ��������� ������������ ��������� ������������,
	*												0x30-0x3D - �������� ���� Command Strobe,
	*												0x3E - ����� ����� ������� ������� � ������ FIFO-�������,
	*												0x3F - ����� ������ RX FIFO;
	* @param	uExtAddress - �������������� ����� ������������ ��������� ������������:
	*												(�������� �������� �������� ������ ���� uGenAddress==0x2F ��� 
	*												uGenAddress==0x3E);
	* @param	bBurst - ����, ����������� �� ��� ��������: ���������(0)/��������(1) (single(0)/burst(1));
	* @param	data_ptr - ��������� �� ����������� ������;
	* @param	uAccesses - ������ ������ (����� ����), ������� ���������� ���������
	*
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
		pCC1120TxData[0] = pCC1120TxData[0] |	(BURST << 6); // ��������� ������ �������� ������ 
	}
		
	for (buff_count = buff_index; buff_count < uAccesses; buff_count++) // ������ ������ �� ��������� �� �������� ����.
	{
		pCC1120TxData[buff_count] = 0x00;
	}

	//�������� ������ � ������������ ��������� �����
	if (SPI_TransmitRecieve(hspiCC1120, pCC1120TxData, data_ptr, uAccesses+buff_index)) 
		return (SPIBusy);
	
	// ����� ����������	
	for (uint8_t i = 0; i< (uAccesses+buff_index); i++)
		data_ptr[i] = data_ptr[i+buff_index];
	
	return (ReadWriteOk);

}

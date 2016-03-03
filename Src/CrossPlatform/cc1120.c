#include "cc1120.h"


SPI_HandleTypeDef *hspiCC1120 = NULL;

uint8_t pCC1120TxData[256];
uint8_t pCC1120RxData[256];

extern uint8_t g_flCC1120_IRQ_CHECKED;



/**
	* @brief	запрос ChipID трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI, ответ от трансивера не тот)
	*/
uint8_t CC1120_CheckModule(SPI_HandleTypeDef *hspi) 
{
	hspiCC1120 = hspi;
	
	//Опускаем CS	
	CC1120_CSN_LOW();
		
	if (CC1120_Read (EXT_PARTNUMBER, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01))	// если есть ошибки обмена данными по SPI возвращаем ошибку функции
		return 0;
	
	//Подождем 100 мкс. Этого хватит для передачи по SPI 6 байт с тактовой выше 500 кГц
	WaitTimeMCS(1e2);
	
	//Поднимаем CS	
	CC1120_CSN_HIGH();
	
	//Проверяем, соответствует ли принятое значение ChipID микросхемы CC1120
	if(pCC1120RxData[0]!=CC1120_ID)
		return 0;	
	
	return(1);
}


uint16_t CC1120_Init(CC1120_TypeDef *pCC1120, SPI_HandleTypeDef *hspi)
{
	pCC1120->hSPI = hspi;
	
	pCC1120->TxState = CC1120_TX_STATE_STANDBY;
	
	//0. Pin Reset микросхемы устанавливаем в высокое состояние
	CC1120_START();
	
	//1. Reset микросхемы
	CC1120_Reset(hspi);

	//2. Сброс флага прерывания конца передачи/начала приема пакета
	g_flCC1120_IRQ_CHECKED = FALSE;
		
	WaitTimeMCS(10e2);
	
	//3. Перевод микросхемы в режим IDLE
	CC1120_IDLE_set(hspi);
		
	//4. Очистка Tx FIFO
	CC1120_TxFIFOFlush(hspi);
		
	//5. Очистка Rx FIFO
	CC1120_RxFIFOFlush(hspi);

	//6. Загрузка конфигурации
	CC1120_ConfigWrite(hspi, CC1120_Config_4800, sizeof (CC1120_Config_4800)/sizeof(registerSetting_t));
		
	WaitTimeMCS(1e2);
		
	//7. Проверка конфигурации
	CC1120_ConfigReadCompare(hspi, CC1120_Config_4800, sizeof (CC1120_Config_4800)/sizeof(registerSetting_t));
		
	//TODO здесь должны быть функции установки частоты и проверки уставленной частоты
	
	//9. Запрос статуса
	CC1120_Status(hspi);
	
	//10. Калибровка PLL
	CC1120_ManualCalibration(hspi);

	WaitTimeMCS(1e2);
	
	//11. Повторная калибровка PLL
	CC1120_ManualCalibration(hspi);
	
	WaitTimeMCS(1e2);	
	
	return(1);
}

/* 			Порядок действий при отправке пакета:
			 1. Очистка Tx FIFO CC1120_TxFIFOFlush
			 2. Загрузка массива данных в Tx FIFO CC1120_TxFIFOWrite
			 3. Запрос количества байтов в Tx FIFO и сравнение с отправленными CC1120_TxFIFONumBytes или 
			 4. Запрос статуса MARCSTATE (проверка на ошибки Tx FIFO) CC1120_MARCState
			 5. Перевод в режим Tx
			 6. Проверка на ошибки по передаче (таймаут) CC1120_MARCState
			 7. Получение прерывания о передаче конца пакета flCC1120_IRQ_CHECKED = TRUE
			 8. Сброс прерывания конца передачи пакета flCC1120_IRQ_CHECKED = FALSE
			 9. Запрос статуса. Подтверждение перехода в IDLE CC1120_Status
 */
uint16_t CC1120_TxData(CC1120_TypeDef *pCC1120, uint8_t* pDataBuf, uint16_t sizeBuf)
{
	//0. перевод в режим IDLE
	//NO: Надо ли это?
	CC1120_IDLE_set(pCC1120->hSPI); 
	
	//1. Очистка Tx FIFO
	CC1120_TxFIFOFlush(pCC1120->hSPI);
		
	//2. Загрузка массива данных в Tx FIFO
	CC1120_TxFIFOWrite(pCC1120->hSPI, pDataBuf, sizeBuf);
				
	//3. Проверка размера данных в TX FIFO
	CC1120_TxFIFONumBytes(pCC1120->hSPI);
				
	//4. Перевод в режим передачи
	CC1120_Tx(pCC1120->hSPI);
			
	//WaitTimeMCS(1e3);
	//CC1120_MARCState(pCC1120->hSPI);
	
	return(1);
}


uint16_t CC1120_RxData(CC1120_TypeDef *pCC1120, uint8_t* pDataBuf, uint16_t* sizeBuf)
{
	//Тут может быть проверка состояния CC1120_MARCState() и адекватности размера принятых данных CC1120_RxFIFONumBytes()
	
	*sizeBuf = CC1120_RxFIFONumBytes(pCC1120->hSPI);
	
	//Чтение массива данных из СС1120
	uint8_t* RxPackData = CC1120_RxFIFORead(pCC1120->hSPI);
	
	for(int16_t i=0; i<*sizeBuf; i++)
		pDataBuf[i] = RxPackData[i];
	//memcpy(pDataBuf,RxPackData,*sizeBuf);
	
	//Перевод повторно в режим приема
	CC1120_Rx(pCC1120->hSPI);
	
	return(1);
}


/**
	* @brief	запрос версии firmware трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					номер версии firmware - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
	*/
uint8_t CC1120_CheckVersion(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Read (EXT_PARTVERSION, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01))	// если есть ошибки обмена данными по SPI возвращаем ошибку функции
		return 0;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return pCC1120RxData[0];	
}




/**
	* @brief	запрос статусного байта трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					0x00 (IDLE)- успешное выполнение;
	*					0x01 (RX)- успешное выполнение;
	*					0x02 (TX)- успешное выполнение;
	*					0x03 (FSTXON)- успешное выполнение;
	*					0x04 (CALIBRATE)- успешное выполнение;
	*					0x05 (SETTLING)- успешное выполнение;
	*					0x06 (RX FIFO ERROR)- успешное выполнение;
	*					0x07 (TX FIFO ERROR)- успешное выполнение;
	*					0x08 - ошибка при выполнении функции (занята шина SPI)
	*/
CC1120STATUSTypeDef CC1120_Status(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	//Опускаем CS	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_STATUS, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
		return STATUS_SPI_ERROR;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	// в байте статуса выделяем старшие 4 бита
	return ((CC1120STATUSTypeDef)(pCC1120RxData[0]>>4));
}


/**
	* @brief	перевод трансивера CC1120 в режим передачи
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
	*/
uint8_t CC1120_Tx(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	//Опускаем CS	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_TX, REG_ADDRESS, NO_BURST, pCC1120TxData, 0x00))
		return 0;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}


/**
	* @brief	перевод трансивера CC1120 в режим бездействия IDLE
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
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
	* @brief	перевод трансивера CC1120 в режим приема
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
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
	* @brief	сброс трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
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
	* @brief	чтение количества байтов в FIFO TX трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					количество байтов в буфере - успешное выполнение;
	*					0xFF - ошибка при выполнении функции (занята шина SPI)
	*/
uint8_t CC1120_TxFIFONumBytes(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Read (EXT_NUM_TXBYTES, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01))	// если есть ошибки обмена данными по SPI возвращаем ошибку функции
		return TX_FIFO_FAIL;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return pCC1120RxData[0];
}

/**
	* @brief	очистка FIFO TX трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
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
	* @brief	запись данных в FIFO TX трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	*					fifo_write_data_ptr - указатель на данные для записи в TX FIFO;
	* @note	 
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
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
	* @brief	Калибровка PLL трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
	*/
uint8_t CC1120_ManualCalibration(SPI_HandleTypeDef *hspi)
{
	uint8_t original_fs_cal2 = 0;
	uint8_t original_fs_cal2_new = 0;
	uint8_t calResult_for_vcdac_start_high[3];
	uint8_t calResult_for_vcdac_start_mid[3];
	hspiCC1120 = hspi;
	
	// запись значения FS_VCO2 = 0x00
	CC1120_CSN_LOW();
	pCC1120TxData[0] = 0x00;
	if (CC1120_Write (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();		
	
	// чтение FS_CAL2 (VCDAC_START)
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_CAL2, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	original_fs_cal2 = pCC1120RxData [0];
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();	
			
	// запись FS_CAL2 = original_fs_cal2 + 2		
	original_fs_cal2_new = original_fs_cal2 +2;
	pCC1120TxData[0] = original_fs_cal2_new;
	
	CC1120_CSN_LOW();		
	if (CC1120_Write (EXT_FS_CAL2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
			
	// запуск калибровки синтезатора
	CC1120_CSN_LOW();
	if (CC1120_Write (S_CAL, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
			
	WaitTimeMCS(10e2);
			
	// чтение FS_VCO2 и сохранение calResults_for_vcdac_start_high		
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	
	calResult_for_vcdac_start_high[0] = pCC1120RxData[0];	
	
	// чтение FS_VCO4 и сохранение calResults_for_vcdac_start_high	 FS_CHP 
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_VCO4, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	
	calResult_for_vcdac_start_high[1] = pCC1120RxData[0];			
			
	// чтение FS_CHP и сохранение calResults_for_vcdac_start_high
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_CHP, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	
	calResult_for_vcdac_start_high[2] = pCC1120RxData[0];	
			
	// запись значения FS_VCO2 = 0x00
	CC1120_CSN_LOW();
	pCC1120TxData[0] = 0x00;
	if (CC1120_Write (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();

	// запись FS_CAL2 = original_fs_cal2		
	pCC1120TxData[0] = original_fs_cal2;
	CC1120_CSN_LOW();		
	if (CC1120_Write (EXT_FS_CAL2, EXT_ADDRESS, NO_BURST, pCC1120TxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	
	// запуск калибровки синтезатора
	CC1120_CSN_LOW();
	if (CC1120_Write (S_CAL, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
					
	WaitTimeMCS(10e2);
	
	// чтение FS_VCO2 и сохранение calResults_for_vcdac_start_mid		
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_VCO2, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	calResult_for_vcdac_start_mid[0] = pCC1120RxData[0];

	// чтение FS_VCO4 и сохранение calResults_for_vcdac_start_mid
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_VCO4, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	calResult_for_vcdac_start_mid[1] = pCC1120RxData[0];			
	
	// чтение FS_CHP и сохранение calResults_for_vcdac_start_mid
	CC1120_CSN_LOW();
	if (CC1120_Read (EXT_FS_CHP, EXT_ADDRESS, NO_BURST, pCC1120RxData,0x01))
		return 0;
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	calResult_for_vcdac_start_mid[2] = pCC1120RxData[0];

	// сравнение calResults_for_vcdac_start_high > calResults_for_vcdac_start_mid?
	if (calResult_for_vcdac_start_high[0] > calResult_for_vcdac_start_mid [0])
	{
		// запись FS_VCO2, FS_VCO4, и FS_CHP сохраненных в calResults_for_vcdac_start_high
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
		// запись FS_VCO2, FS_VCO4, и FS_CHP сохраненных в calResults_for_vcdac_start_mid
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
	* @brief	Получение статуса трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
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
	*					 - ошибка при выполнении функции (занята шина SPI)
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
	* @brief	Автоматическая калибровка PLL трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
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
	* @brief	чтение количества байтов в FIFO RX трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					количество байтов в буфере - успешное выполнение;
	*					0xFF - ошибка при выполнении функции (занята шина SPI)
	*/
uint8_t CC1120_RxFIFONumBytes(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	if (CC1120_Read (EXT_NUM_RXBYTES, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01))	// если есть ошибки обмена данными по SPI возвращаем ошибку функции
		return RX_FIFO_FAIL;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return pCC1120RxData[0];	
}


/**
	* @brief	очистка FIFO RX трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note	 
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
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
	* @brief	Запись конфигурации в трансивер CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	*	@param	*Config -указатель на массив данных конфигурации
	*	@param	*configRegNum -количество данных в массиве
	* @note	 
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
	*/
uint8_t CC1120_ConfigWrite(SPI_HandleTypeDef *hspi, const registerSetting_t *CC1120_Config, uint8_t configRegNum)
{
	hspiCC1120 = hspi;

	uint8_t	writeExtAddress; // дополнительный адрес регистра
	uint8_t	writeAddress; // адрес регистра
	
	CC1120_CSN_LOW();
	
	// цикл записи регистров
	for (uint16_t i=0; i<configRegNum; i++)
	{
		pCC1120TxData[0] = CC1120_Config[i].data; // значение регистра
		writeExtAddress = ((CC1120_Config[i].addr & 0xFF00)>>8);
		writeAddress = CC1120_Config[i].addr & 0xFF;
		
		if (writeExtAddress) 
		{
			if (CC1120_Write (writeAddress, EXT_ADDRESS, NO_BURST, pCC1120TxData, 0x01)) // если расширенный адрес не 0, то пишем в расширенную область
				return 0;
		}
		else
		{
			if (CC1120_Write (writeAddress, REG_ADDRESS, NO_BURST, pCC1120TxData, 0x01)) // если дополнительный адрес 0, то пишем в основной регистр
				return 0;
		}		
	}
		
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
		
	return (1);
}

/**
	* @brief	Прочитать конфигурацию трансивера CC1120 и сравнить с записанной
	* @param	*hspi - выбор интерфейса SPI для обращения
	*	@param	*Config -указатель на массив данных конфигурации
	*	@param	*configRegNum -количество данных в массиве
	* @note	 
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					2 - ошибка сравнения
	*					0 - ошибка при выполнении функции (занята шина SPI)
	*/
uint8_t CC1120_ConfigReadCompare(SPI_HandleTypeDef *hspi, const registerSetting_t *CC1120_Config, uint8_t configRegNum)
{
	hspiCC1120 = hspi;
	uint8_t	readExtAddress; // дополнительный адрес регистра
	uint8_t	readAddress; // адрес регистра
	
	CC1120_CSN_LOW();
	
	// цикл записи регистров
	for (uint16_t i=0; i< configRegNum; i++)
	{
		readExtAddress = ((CC1120_Config[i].addr & 0xFF00)>>8);
		readAddress = CC1120_Config[i].addr & 0xFF;
				
		if (readExtAddress) 
		{
			if (CC1120_Read (readAddress, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01)) // если расширенный адрес не 0, то пишем в расширенную область
				return 0;
		}
		else
		{
			if (CC1120_Read (readAddress, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x01)) // если дополнительный адрес 0, то пишем в основной регистр
				return 0;
		}

		if (pCC1120RxData[0] != CC1120_Config[i].data)
		{
			#ifdef DEBUG_USE_LEDS
			LED2_ON();
			#endif
			return (2);
		}
	}
			
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();

	return (1);
}

/**
	* @brief	Записать значение в регистр трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	*	@param	addrReg - адрес регистра uint16_t
	*	@param	dataReg - значение регистра uint8_t
	* @note	 по необходимости переделать. Для работы с трансивером в штатном режиме не требуется. Не проверял.
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
	*/
uint8_t CC1120_RegWrite (SPI_HandleTypeDef *hspi, uint16_t addrReg, uint8_t dataReg)
{
	hspiCC1120 = hspi;
	uint8_t writeExtAddress;
	uint8_t writeAddress;
	
	pCC1120TxData[0] = dataReg; // значение регистра
	writeExtAddress = ((addrReg & 0xFF00)>>8);
	writeAddress = addrReg & 0xFF;
	
	CC1120_CSN_LOW();
	
	if (writeExtAddress) 
	{
		if (CC1120_Write (writeAddress, EXT_ADDRESS, NO_BURST, pCC1120TxData, 0x01)) // если расширенный адрес не 0, то пишем в расширенную область
			return 0;
	}
	else
	{
		if (CC1120_Write (writeAddress, REG_ADDRESS, NO_BURST, pCC1120TxData, 0x01)) // если дополнительный адрес 0, то пишем в основной регистр
			return 0;
	}
	
	WaitTimeMCS(1e2);
	CC1120_CSN_HIGH();
	
	return (1);
}



/**
	* @brief	Прочитать значение из регистра трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	*	@param	addrReg - адрес регистра uint16_t
	*	@param	dataReg - значение регистра uint8_t
	* @note		по необходимости переделать. Для работы с трансивером в штатном режиме не требуется. Не проверял.
	* @retval Результат выполнения функции:
	*					данные регистра - успешное выполнение;
	*					
	*/
uint8_t CC1120_RegRead (SPI_HandleTypeDef *hspi, uint16_t addrReg, uint8_t dataReg)
{
	hspiCC1120 = hspi;
	uint8_t	readExtAddress; // дополнительный адрес регистра
	uint8_t	readAddress; // адрес регистра
	
	CC1120_CSN_LOW();
	readExtAddress = ((addrReg & 0xFF00)>>8);
	readAddress = addrReg & 0xFF;

	if (readExtAddress) 
		CC1120_Read (readAddress, EXT_ADDRESS, NO_BURST, pCC1120RxData, 0x01); // если расширенный адрес не 0, то пишем в расширенную область	
	else
		CC1120_Read (readAddress, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x01); // если дополнительный адрес 0, то пишем в основной регистр		
	
	return pCC1120RxData[0];
}

/**
	* @brief	Записать частоту в трансивер CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	*	@param	*freq - ссылка на массив со значениями частоты
	* @note		по необходимости переделать. Для работы с трансивером в штатном режиме не требуется. Не проверял.
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)		
	*/
uint8_t CC1120_FreqWrite (SPI_HandleTypeDef *hspi, uint8_t *freq)
{
	hspiCC1120 = hspi;
	
	CC1120_CSN_LOW();
	
	/* 	
	 *	записываем в режиме burst. Начальный адрес EXT_FREQ2 (0x0C).
	 *	затем FREQ1 (0x0D) и FREQ0 (0x0E)														 
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
	* @brief	Прочитать частоту из трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	*	@param	*freq - ссылка на массив со значениями частоты
	* @note		по необходимости переделать. Для работы с трансивером в штатном режиме не требуется. Не проверял.
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)		
	*/
uint8_t *CC1120_FreqRead (SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;

	CC1120_CSN_LOW();
	
	/* 	
	 *	считываем в режиме burst. Начальный адрес EXT_FREQ2 (0x0C).
	 *	затем FREQ1 (0x0D) и FREQ0 (0x0E)														 
	*/
	
	if (CC1120_Read (EXT_FREQ2, EXT_ADDRESS, BURST, pCC1120RxData, 0x03))
		return 0;
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (pCC1120RxData);

}

/**
	* @brief	Прочитать содержимое буфера RX FIFO трансивера CC1120
	* @param	*hspi - выбор интерфейса SPI для обращения
	* @note		по необходимости переделать. Для работы с трансивером в штатном режиме не требуется. Не проверял.
	* @retval Результат выполнения функции:
	*					ссылка на массив со значениями - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)		
	*/
uint8_t *CC1120_RxFIFORead(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	uint8_t RxFIFONumBytes = 0;
	
	// запрос количества байтов данных в RxFIFO
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
/*	
	// сдвиг данных буфера на 1 индекс и запись в 0 элемент количество байтов Rx FIFO
	for (int8_t i = (RxFIFONumBytes+1); i<=0; i--)
	{
		pCC1120RxData[i+1] = pCC1120RxData[i];
	}
	pCC1120RxData[0] = RxFIFONumBytes;

	WaitTimeMCS(10e2);
*/		
	CC1120_CSN_HIGH();
	
	return (pCC1120RxData);
}



/**
	* @brief	Функция записи данных (значений регистров/данных для FIFO-буферов) в CC1120
	* @param	uGenAddress - адрес основной области (0x00-0x3F):
	*												0x00-0x2E - конфигурационные регистры, 
	*												0x2F - общий адрес для регистров расширенного адресного пространства,
	*												0x30-0x3D - регистры типа Command Strobe,
	*												0x3E - общий адрес прямого доступа к данным FIFO-буферов,
	*												0x3F - адрес буфера TX FIFO;
	*					uExtAddress - дополнительный адрес расширенного адресного пространства:
	*												(параметр является значимым только если uGenAddress==0x2F или 
	*												uGenAddress==0x3E);
	*					bBurst - флаг, указывающий на тип операции: одиночная(0)/пакетная(1) (single(0)/burst(1));
	*					data_ptr - указатель на данные, которые необходимо записать;
	*					uAccesses - размер данных (число байт), которые необходимо записать
	* @note	 
	* @retval Результат выполнения функции:
	*					0 - успешное выполнение;
	*					иначе - ошибка при выполнении функции (занята шина SPI, некорректное значение входных данных)
	*/
ReadWriteRegTypeDef CC1120_Write (uint8_t uGenAddress, uint8_t uExtAddress, uint8_t bBurst, uint8_t *data_ptr, uint16_t uAccesses)
{
	uint8_t buff_count = 0; // индекс записи данных
	uint8_t buff_index = 1;
	
	
	switch (uExtAddress) // формирование первого байта адреса или дополнительного адреса
	{
		case EXT_ADDRESS:
			// сдвинуть данные на 2 позиции
			for (buff_count = 0; buff_count < uAccesses; buff_count++) // запись данных со смещением на адресные поля.
			{
				pCC1120TxData[buff_count+2] = data_ptr[buff_count];
			}
		
			pCC1120TxData[0] = EXT_ADDRESS;
			pCC1120TxData[1] = uGenAddress;
			
			buff_index++;
		break;

		case REG_DMA:
			// сдвинуть данные на 2 позиции
			for (buff_count = 0; buff_count < uAccesses; buff_count++) // запись данных со смещением на адресные поля.
			{
				pCC1120TxData[buff_count+2] = data_ptr[buff_count];
			}
			
			pCC1120TxData[0] = REG_DMA;
			pCC1120TxData[1] = uGenAddress;

			buff_index++;
		break;

		case REG_ADDRESS:
			// сдвинуть данные на 1 позицию
			for (buff_count = 0; buff_count < uAccesses; buff_count++) // запись данных со смещением на адресные поля.
			{
				pCC1120TxData[buff_count+1] = data_ptr[buff_count];
			}
			
			pCC1120TxData[0] = uGenAddress;
		break;

		default:
			return (DataInMismatch); // если введенное значение типа регистра ни основной, ни дополнительный, ни DMA 
	}

	
	pCC1120TxData[0] = pCC1120TxData[0] | (WRITE_CC1120 << 7); // установка признака записи (знаю, что он 0. Это для единообразия)
	
	if (bBurst) 
	{
		pCC1120TxData[0] = pCC1120TxData[0] |	(BURST << 6); // установка режима поточной записи 
	}		
	
	//Передаем данные и одновременно принимаем ответ
	if (SPI_TransmitRecieve(hspiCC1120, pCC1120TxData, pCC1120RxData, uAccesses+buff_index)) 
		return (SPIBusy);
	else
		return (ReadWriteOk);

}




/**
	* @brief	Функция чтения данных (значений регистров/данных FIFO-буферов) из CC1120
	*
	* @param	uGenAddress - адрес основной области (0x00-0x3F):
	*												0x00-0x2E - конфигурационные регистры, 
	*												0x2F - общий адрес для регистров расширенного адресного пространства,
	*												0x30-0x3D - регистры типа Command Strobe,
	*												0x3E - общий адрес прямого доступа к данным FIFO-буферов,
	*												0x3F - адрес буфера RX FIFO;
	* @param	uExtAddress - дополнительный адрес расширенного адресного пространства:
	*												(параметр является значимым только если uGenAddress==0x2F или 
	*												uGenAddress==0x3E);
	* @param	bBurst - флаг, указывающий на тип операции: одиночная(0)/пакетная(1) (single(0)/burst(1));
	* @param	data_ptr - указатель на прочитанные данные;
	* @param	uAccesses - размер данных (число байт), которые необходимо прочитать
	*
	* @note	 
	* @retval Результат выполнения функции:
	*					0 - успешное выполнение;
	*					иначе - ошибка при выполнении функции (занята шина SPI, некорректное значение входных данных)
	*/
ReadWriteRegTypeDef CC1120_Read (uint8_t uGenAddress, uint8_t uExtAddress, uint8_t bBurst, uint8_t *data_ptr, uint8_t uAccesses)
{
	uint8_t buff_index = 1; // индекс буфера записи
	uint8_t buff_count = 0; // индекс записи данных
	
	
	switch (uExtAddress) // формирование первого байта адреса или дополнительного адреса
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
			return (DataInMismatch); // если введенное значение типа регистра ни основной, ни дополнительный, ни DMA 
	}
	
	
	pCC1120TxData[0] = pCC1120TxData[0] | (READ_CC1120 << 7); // установка признака чтения
	
	if (bBurst) 	
	{	
		pCC1120TxData[0] = pCC1120TxData[0] |	(BURST << 6); // установка режима поточной записи 
	}
		
	for (buff_count = buff_index; buff_count < uAccesses; buff_count++) // запись данных со смещением на адресные поля.
	{
		pCC1120TxData[buff_count] = 0x00;
	}

	//Передаем данные и одновременно принимаем ответ
	if (SPI_TransmitRecieve(hspiCC1120, pCC1120TxData, data_ptr, uAccesses+buff_index)) 
		return (SPIBusy);
	
	// сдвиг результата	
	for (uint8_t i = 0; i< (uAccesses+buff_index); i++)
		data_ptr[i] = data_ptr[i+buff_index];
	
	return (ReadWriteOk);

}


/**
	* @brief	Функция аппаратного сброса CC1120
	* @note		Функция формирует 50мкс-ный импульс на ноге аппаратного сброса микросхемы
	*					и ожидает в течение 50мкс пока она выйдет в рабочий режим
	*/
void CC1120_HardwareReset()
{
	CC1120_RESET(); 		// аппаратный сброс СС1120
	WaitTimeMCS(5e1); 	// задержка 50 мкс

	CC1120_START(); 		// запуск СС1120
	WaitTimeMCS(5e1); 	// ожидание, пока стабилизируется внутренний генератор
}


#include "cc1120.h"
#include "uart_gui.h"

SPI_HandleTypeDef *hspiCC1120 = NULL;

uint8_t pCC1120TxData[255];
uint8_t pCC1120RxData[255];




/**
  * @brief  запрос ChipID трансивера CC1120
	* @param  *hspi - выбор интерфейса SPI для обращения
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
			{
				return 0;
			}
	
	
	//Подождем 100 мкс. Этого хватит для передачи по SPI 6 байт с тактовой выше 500 кГц
	WaitTimeMCS(1e2);
	
	//Поднимаем CS	
	CC1120_CSN_HIGH();
	
	//Должны принять в байте 2 значение 0x48, соответствующее ChipID микросхемы CC1120
	if(pCC1120RxData[2]!=CC1120_ID)
		return 0;	
	
	return(1);
}



/**
  * @brief  запрос версии firmware трансивера CC1120
	* @param  *hspi - выбор интерфейса SPI для обращения
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
			{
				return 0;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
		return pCC1120RxData[2];	
}




/**
  * @brief  запрос статусного байта трансивера CC1120
	* @param  *hspi - выбор интерфейса SPI для обращения
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
uint8_t CC1120_Status(SPI_HandleTypeDef *hspi)
{
	hspiCC1120 = hspi;
	
	//Опускаем CS	
	CC1120_CSN_LOW();
	
	if (CC1120_Write (S_STATUS, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
			{
				return STATUS_SPI_ERROR;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	// в байте статуса выделяем старшие 4 бита
	return (pCC1120RxData[0]>>4);
}


/**
  * @brief  перевод трансивера CC1120 в режим передачи
	* @param  *hspi - выбор интерфейса SPI для обращения
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
	
	if (CC1120_Write (S_TX, REG_ADDRESS, NO_BURST, pCC1120RxData, 0x00))
			{
				return 0;
			}
	
	WaitTimeMCS(1e2);
	
	CC1120_CSN_HIGH();
	
	return (1);
}


/**
  * @brief  перевод трансивера CC1120 в режим бездействия IDLE
	* @param  *hspi - выбор интерфейса SPI для обращения
  * @note   
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
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
  * @brief  перевод трансивера CC1120 в режим приема
	* @param  *hspi - выбор интерфейса SPI для обращения
  * @note   
	* @retval Результат выполнения функции:
	*					1 - успешное выполнение;
	*					0 - ошибка при выполнении функции (занята шина SPI)
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
  * @brief  Функция записи данных (значений регистров/данных для FIFO-буферов) в CC1120
	* @param  uGenAddress - адрес основной области (0x00-0x3F):
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
		
		break;		
	
	}
	
	pCC1120TxData[0] = pCC1120TxData[0] | (WRITE_CC1120 << 7); // установка признака записи (знаю, что он 0. Это для единообразия)
	
	if (bBurst) 
	{
		pCC1120TxData[0] = pCC1120TxData[0] |  (BURST << 6); // установка режима поточной записи 
	}		
	
	for (buff_count = buff_index; buff_count < uAccesses; buff_count++) // запись данных со смещением на адресные поля.
		{
				pCC1120TxData[buff_count] = data_ptr[buff_count-buff_index];
		}
	//Передаем данные и одновременно принимаем ответ
	if (SPI_TransmitRecieve(hspiCC1120, pCC1120TxData, pCC1120RxData, uAccesses+buff_index)) 
	{
			return (SPIBusy);
	} else {
			return (ReadWriteOk);
	}	
}




/**
  * @brief  Функция чтения данных (значений регистров/данных FIFO-буферов) из CC1120
	* @param  uGenAddress - адрес основной области (0x00-0x3F):
	*												0x00-0x2E - конфигурационные регистры, 
	*												0x2F - общий адрес для регистров расширенного адресного пространства,
	*												0x30-0x3D - регистры типа Command Strobe,
	*												0x3E - общий адрес прямого доступа к данным FIFO-буферов,
	*												0x3F - адрес буфера RX FIFO;
	*					uExtAddress - дополнительный адрес расширенного адресного пространства:
	*												(параметр является значимым только если uGenAddress==0x2F или 
	*												uGenAddress==0x3E);
	*					bBurst - флаг, указывающий на тип операции: одиночная(0)/пакетная(1) (single(0)/burst(1));
	*					data_ptr - указатель на прочитанные данные;
	*					uAccesses - размер данных (число байт), которые необходимо прочитать
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
		
		break;		
	
	}
	
	pCC1120TxData[0] = pCC1120TxData[0] | (READ_CC1120 << 7); // установка признака записи (знаю, что он 0. Это для единообразия)
	
	if (bBurst) 	
		{	
			pCC1120TxData[0] = pCC1120TxData[0] |  (BURST << 6); // установка режима поточной записи 
		}
		
	for (buff_count = buff_index; buff_count < uAccesses; buff_count++) // запись данных со смещением на адресные поля.
		{
				pCC1120TxData[buff_count] = data_ptr[buff_count-buff_index];
		}
	//Передаем данные и одновременно принимаем ответ
	if (SPI_TransmitRecieve(hspiCC1120, pCC1120TxData, pCC1120RxData, uAccesses+buff_index)) 
	{
			return (SPIBusy);
	} else {
			return (ReadWriteOk);
	}	
}

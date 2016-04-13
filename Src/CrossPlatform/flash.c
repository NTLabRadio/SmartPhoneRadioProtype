#include "flash.h"

uint32_t curAddrWriteToFlash = 0;
uint16_t FirmCRC = 0;


uint8_t StartFirmLoadToFlash()
{
	HAL_FLASH_Unlock();
	
	for(uint8_t cntPages=0; cntPages<MAIN_PROGRAM_NUM_OF_PAGES; cntPages++)
		FLASH_PageErase(MAIN_PROGRAM_START_ADDRESS + cntPages*FLASH_PAGE_SIZE);
	
	CLEAR_BIT(FLASH->CR, FLASH_CR_PER);
	
	curAddrWriteToFlash = MAIN_PROGRAM_START_ADDRESS;
	FirmCRC = 0;
	
	return(1);
}

uint8_t FirmLoadToFlash(uint8_t* pDataFrame, uint16_t nSizeDataFrame)
{
	uint16_t nWord;
	
	//Если во фрейме есть полезные данные, копируем их в очередь для записи во флеш
	if(nSizeDataFrame)
	{
		for (uint32_t index = 0; index < nSizeDataFrame; index+=sizeof(uint16_t))
		{
			nWord = *(uint16_t *)(pDataFrame + index);
			HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, curAddrWriteToFlash + index, nWord);
			
			FirmCRC^=nWord;
		}
		
		curAddrWriteToFlash+=nSizeDataFrame;
	}	

	return(1);	
}

uint8_t FinishFirmLoadToFlash(uint16_t nFirmDumpCRC, uint16_t nFirmCmd)
{
	HAL_FLASH_Lock();
	
	if(nFirmDumpCRC==FirmCRC)
		return(1);
	else
		return(0);
}

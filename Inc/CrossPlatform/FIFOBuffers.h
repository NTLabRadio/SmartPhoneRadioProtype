/**
  ******************************************************************************
  * @file    FIFObuffers.h
  * @brief   Файл, содержащий функции для работы с буферами FIFO
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FIFOBUFFERS_H
#define __FIFOBUFFERS_H

#include <stdint.h>	 
#include <string.h>


uint8_t RemDataFromFIFOBuf(uint8_t* pFIFOBuf, uint16_t &nSizeFIFOBuf, uint16_t numRemBytes);
uint8_t AddDataToFIFOBuf(uint8_t* pFIFOBuf, uint16_t &nSizeFIFOBuf, uint8_t* pDataToAdd, uint16_t numAddBytes);
	 

#endif /* __FIFOBUFFERS_H */

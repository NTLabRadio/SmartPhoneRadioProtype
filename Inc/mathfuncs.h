/**
  ******************************************************************************
  * @file    mathfuncs.h
  * @brief   Файл, содержащий математические функции и константы 
							проекта SmartPhonePrototype
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MATHFUNCS_H
#define __MATHFUNCS_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>	 
	 
void FillBufByToneSignal(int16_t* pBuf, uint16_t nSizeBuf, uint16_t nFreqSampl, uint16_t nFreqTone);
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MATHFUNCS_H */

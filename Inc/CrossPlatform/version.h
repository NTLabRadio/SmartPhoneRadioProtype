/**
  ******************************************************************************
  * @file    version.h
  * @brief   Файл, содержащий номер текущей версии софта ARM
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __VERSION_H
#define __VERSION_H

#ifdef __cplusplus
 extern "C" {
#endif

#define	ARM_SOFT_MAJ_VER	(2)
#define	ARM_SOFT_MIN_VER	(0)
	 
#define	ARM_SOFT_VER	((ARM_SOFT_MAJ_VER<<8) | ARM_SOFT_MIN_VER)
	 
	 
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VERSION_H */

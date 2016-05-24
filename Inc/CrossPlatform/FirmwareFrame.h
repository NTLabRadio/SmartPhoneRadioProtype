/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** Этот файл описывает класс FirmwareFrame, включающий в себя набор основных
** методов и свойств для обработки кадров с данными прошивки в соответствии
** с протоколом SPIM
**
****************************************************************************/

#ifndef FIRMWAREFRAME_H
#define FIRMWAREFRAME_H

#include <stdint.h>
#include <string.h>

#include "SPIMMessage.h"

#ifdef DEFINE_FIRMWARE_FRAME_CLASS
class FirmwareFrame: public SPIMMessage 
{
public:
	FirmwareFrame(const uint8_t* pMsgData, uint16_t msgSize);
		
	uint8_t GetTypeOfFrame();

	uint16_t GetSizeOfFirmData();
	uint8_t* GetFirmData();

	uint16_t GetFirmCRC();
	uint16_t GetFirmCmd();

	typedef enum typeOfFrames_t
	{
		TYPE_OF_FRAME_HEADER,
		TYPE_OF_FRAME_FIRMDATA,
		TYPE_OF_FRAME_END,
		NUM_TYPE_OF_FRAMES
	} typeOfFrames_t;

private:

	typeOfFrames_t typeOfFrame;

	uint16_t sizeOfFirmData;
	uint8_t* pFirmData;	

	uint16_t FirmCRC;
	uint16_t FirmCmd;

	uint8_t SetTypeOfFrame(uint8_t nTypeOfFrame);

	static const uint16_t MAX_SIZE_OF_FIRM_DATA = (128);

	static const uint8_t IDFIRMFRAME_POS_IN_FRAME = (0);
	static const uint8_t FIRMTYPE_POS_IN_FRAME = (2);
	static const uint8_t FIRMNUMFRAMES_POS_IN_FRAME = (4);

	static const uint8_t SIZEFIRMDATA_POS_IN_FRAME = (2);
	static const uint8_t FIRMDATA_POS_IN_FRAME = (4);

	static const uint8_t FIRMCRC_POS_IN_FRAME = (2);
	static const uint8_t FIRMCMD_POS_IN_FRAME = (4);
};
#endif

#endif // FIRMWAREFRAME_H

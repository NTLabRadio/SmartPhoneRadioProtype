#include "FirmwareFrame.h"

#ifdef DEFINE_FIRMWARE_FRAME_CLASS

FirmwareFrame::FirmwareFrame(const uint8_t* pMsgData, uint16_t msgSize): SPIMMessage(pMsgData, msgSize)
{
	uint16_t nIDFrame = *(uint16_t*)(pMsgData+IDFIRMFRAME_POS_IN_FRAME);

	//В заголовочном и конечном фреймах данные прошивки не содержатся, 
	//по умолчанию нулим их, для TYPE_OF_FRAME_FIRMDATA они будут повторно проинициализированы
	sizeOfFirmData = 0;	
	pFirmData = NULL;
	
	//CRC дампа прошивки и команда прошивки содержатся только в конечном фрейме,
	//для остальных типов фреймов считаем, что они нулевые
	FirmCRC = 0;
	FirmCmd = 0;
	
	switch(nIDFrame)
	{
		case 0x0000:
			SetTypeOfFrame(TYPE_OF_FRAME_HEADER);
			
			break;
		case 0xFFFF:
			SetTypeOfFrame(TYPE_OF_FRAME_END);

			FirmCRC = *(uint16_t*)(pMsgData+FIRMCRC_POS_IN_FRAME);
			FirmCmd = *(uint16_t*)(pMsgData+FIRMCMD_POS_IN_FRAME);
		
			break;
		default:
		{
			SetTypeOfFrame(TYPE_OF_FRAME_FIRMDATA);
			
			sizeOfFirmData = *(uint16_t*)(pMsgData + SIZEFIRMDATA_POS_IN_FRAME);
			
			if(sizeOfFirmData<=MAX_SIZE_OF_FIRM_DATA)
				memcpy(pFirmData,pMsgData+FIRMDATA_POS_IN_FRAME,sizeOfFirmData);
			else
			{
				sizeOfFirmData = 0;
				pFirmData = NULL;
			}
		}
	}
	
	return;	
}



uint8_t FirmwareFrame::GetTypeOfFrame()
{
	return(typeOfFrame);
}


uint8_t FirmwareFrame::SetTypeOfFrame(uint8_t nTypeOfFrame)
{
	if(nTypeOfFrame>=NUM_TYPE_OF_FRAMES)
		return(1);
	
	typeOfFrame = (typeOfFrames_t)nTypeOfFrame;
	
	return(0);
}


uint16_t FirmwareFrame::GetSizeOfFirmData()
{
	return(sizeOfFirmData);
}


uint8_t* FirmwareFrame::GetFirmData()
{
	return(pFirmData);
}


uint16_t FirmwareFrame::GetFirmCRC()
{
	return(FirmCRC);
}


uint16_t FirmwareFrame::GetFirmCmd()
{
	return(FirmCmd);
}

#endif

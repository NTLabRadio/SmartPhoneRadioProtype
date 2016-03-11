/**
  ******************************************************************************
  * @file    QueDataFrames.h
  * @brief   ‘айл, содержащий описание класса QueDataFrames, предназначенного 
	*						дл€ работы с очеред€ми, элементами которых €вл€ютс€ массивы (кадры) 
	*						целочисленных данных произвольной длины
  ******************************************************************************
  *
  * COPYRIGHT(c) 2016 NTLab
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __QUEDATAFRAMES_H
#define __QUEDATAFRAMES_H

#include <stdint.h>
#include <string.h>


class QueDataFrames
{
public:
	QueDataFrames(uint16_t numFrames, uint16_t maxSizeFrame);
	~QueDataFrames();

	uint16_t GetNumFramesInQue();
	uint16_t GetMaxNumFrames();
	uint16_t GetMaxSizeOfFrame();

	uint16_t PushFrame(uint8_t* pData, uint16_t sizeOfData);
	uint16_t PopFrame(uint8_t* pData);
	uint8_t isEmpty();

private:

	uint16_t m_numFramesInQue;
	uint16_t m_maxNumFrames;
	uint16_t m_maxSizeFrame;

	//Ёлемент очереди
	typedef struct queFrame_t
	{
		queFrame_t *nextFrame, *prevFrame;
		uint8_t* pData;
		uint16_t sizeData;
	} queFrame_t;

	//ќчередь
	typedef struct queDataFrames_t
	{
		queFrame_t *firstFrame, *lastFrame;
	} queDataFrames_t;
	
	queDataFrames_t m_queFrames;

};



#endif /* __QUEDATAFRAMES_H */

#include "QueDataFrames.h"

#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
uint16_t g_cntCallPushFrame = 0;
uint16_t g_cntPushFramesInQue = 0;
#endif

QueDataFrames::QueDataFrames(uint16_t numFrames, uint16_t maxSizeFrame)
{
	m_queFrames.firstFrame = NULL;
	m_queFrames.lastFrame = NULL;
	
	m_numFramesInQue = 0;
	m_maxNumFrames = numFrames;
	m_maxSizeFrame = maxSizeFrame;

}

QueDataFrames::~QueDataFrames()
{
	while(m_numFramesInQue--)
	{
		//Удаляем данные последнего фрейма в очереди
		delete [] m_queFrames.lastFrame->pData;
		m_queFrames.lastFrame->sizeData = 0;

		//И делаем последним фрейм, предшествующий последнему
		queFrame_t* prevLastFrame = m_queFrames.lastFrame->prevFrame;
		m_queFrames.lastFrame = prevLastFrame;
		
		delete prevLastFrame;
	}	
	
	m_queFrames.firstFrame = NULL;
	m_queFrames.lastFrame = NULL;

	m_numFramesInQue = 0;
	m_maxNumFrames = 0;
	m_maxSizeFrame = 0;
}

uint16_t QueDataFrames::GetNumFramesInQue()
{
	return(m_numFramesInQue);
}

uint8_t QueDataFrames::isEmpty()
{
	if(!m_numFramesInQue)
		return(1);
	else
		return(0);
}
	
uint16_t QueDataFrames::GetMaxNumFrames()
{
	return(m_maxNumFrames);
}

uint16_t QueDataFrames::GetMaxSizeOfFrame()
{
	return(m_maxSizeFrame);
}

uint16_t QueDataFrames::PushFrame(uint8_t* pData, uint16_t sizeOfData)
{
	#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
	g_cntCallPushFrame++;
	#endif
	
	//Если размер кадра данных, который пытаемся запихнуть в очередь, превышает 
	//размер предвыделенных элементов очереди, возвращаем 0
	if(sizeOfData>m_maxSizeFrame)
		return(0);

	//Если очередь уже заполнена на 100%, возвращаем 0
	if(m_numFramesInQue==m_maxNumFrames)
		return(0);
	
	//Если передан нулевой указатель или нулевой размер, возвращаем 0
	if((pData==NULL) || (!sizeOfData))
		return(0);	
	
	//Создаем элемент очереди
	queFrame_t* curFrame = new queFrame_t;
	curFrame->nextFrame = NULL;
	curFrame->prevFrame = m_queFrames.lastFrame;
	
	//Наполняем его данными
	curFrame->pData = new uint8_t[sizeOfData];
	memcpy(curFrame->pData,pData,sizeOfData);
	curFrame->sizeData = sizeOfData;

	//Присоединяем его в хвост очереди
	if(m_queFrames.lastFrame)
	{
		m_queFrames.lastFrame->nextFrame = curFrame;
		m_queFrames.lastFrame = curFrame;
	}
	else
	{
		m_queFrames.firstFrame = curFrame;
		m_queFrames.lastFrame = curFrame;
	}
	m_numFramesInQue++;
	
	#ifdef DEBUG_CHECK_ERRORS_IN_RCV_RADIO_PACKS
	g_cntPushFramesInQue++;
	#endif
	
	return(sizeOfData);
}

uint16_t QueDataFrames::PopFrame(uint8_t* pData)
{
	uint16_t nSizeData;

	//Если в очереди нет ни одного элемента, возвращаем 0
	if(!m_numFramesInQue)
		return(0);

		//Если передан нулевой указатель, возвращаем 0
	if(pData==NULL)
		return(0);

	//Копируем данные первого фрейма
	memcpy(pData, m_queFrames.firstFrame->pData, m_queFrames.firstFrame->sizeData);
	nSizeData = m_queFrames.firstFrame->sizeData;

	//Удаляем данные первого фрейма в очереди
	delete [] m_queFrames.firstFrame->pData;
	m_queFrames.firstFrame->sizeData = 0;

	//И делаем в очереди первым фрейм, следующий за первым
	queFrame_t* oldFirstFrame = m_queFrames.firstFrame;
	m_queFrames.firstFrame = oldFirstFrame->nextFrame;

	delete oldFirstFrame;
	oldFirstFrame = NULL;
	
	m_numFramesInQue--;
	
	if(!m_numFramesInQue)
		m_queFrames.lastFrame = NULL;

	return(nSizeData);
}

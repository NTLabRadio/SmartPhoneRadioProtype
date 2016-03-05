#include "FIFObuffers.h"

uint8_t RemDataFromFIFOBuf(uint8_t* pFIFOBuf, uint16_t &nSizeFIFOBuf, uint16_t numRemBytes)
{
	if(!pFIFOBuf)
		return(0);
		
	if(!numRemBytes)
		return(0);	
	
	if(nSizeFIFOBuf<=numRemBytes)
		numRemBytes = nSizeFIFOBuf;

	memmove(pFIFOBuf,pFIFOBuf+numRemBytes,nSizeFIFOBuf-numRemBytes);
	nSizeFIFOBuf-=numRemBytes;
	memset(pFIFOBuf+nSizeFIFOBuf,0,numRemBytes);

	return(numRemBytes);	
}

uint8_t AddDataToFIFOBuf(uint8_t* pFIFOBuf, uint16_t &nSizeFIFOBuf, uint8_t* pDataToAdd, uint16_t numAddBytes)
{
	if((!pFIFOBuf) || (!pDataToAdd))
		return(0);
	
	if(!numAddBytes)
		return(0);		
	
	memcpy(&pFIFOBuf[nSizeFIFOBuf],pDataToAdd,numAddBytes);
	nSizeFIFOBuf+=numAddBytes;
	
	return(numAddBytes);
}

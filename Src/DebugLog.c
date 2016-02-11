#include <DebugLog.h>

#pragma import(__use_no_semihosting_swi)

volatile int ITM_RxBuffer=0x5AA55AA5; /* Buffer to transmit data towards debug system. */

struct __FILE { int handle; };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) 
{
	return (ITM_SendChar((uint32_t)ch));
}	

uint8_t ConvertHexIntToUTF8(uint8_t* pBufData, uint16_t nSizeData)
{
    int16_t cntBytes;
    uint8_t nByteHighValue, nByteLowValue;

    nSizeData*=2;

    for(cntBytes=nSizeData/2-1; cntBytes>=0; cntBytes--)
    {
        nByteHighValue = (pBufData[cntBytes]&0xF0)>>4;
        nByteLowValue = pBufData[cntBytes]&0x0F;

        if(nByteLowValue <= 0x9)
            pBufData[2*cntBytes+1] = nByteLowValue + 0x30;
        else if((nByteLowValue >= 0xA) && (nByteLowValue <= 0xF))
            pBufData[2*cntBytes+1] = nByteLowValue + 0x37;
        else
            return 0;

        if(nByteHighValue <= 0x9)
            pBufData[2*cntBytes] = nByteHighValue + 0x30;
        else if((nByteHighValue >= 0xA) && (nByteHighValue <= 0xF))
            pBufData[2*cntBytes] = nByteHighValue + 0x37;
        else
            return 0;
    }

    return 1;
}

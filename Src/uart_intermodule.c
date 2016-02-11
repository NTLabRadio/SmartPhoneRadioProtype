#include "uart_intermodule.h"


#define SIZE_OF_UART_RX_BUFFER (32)
uint8_t pUARTRxBuf[SIZE_OF_UART_RX_BUFFER];

#define MAX_SIZE_OF_SLIP_PACK_PAYLOAD	(128+4)		// � ������������ � ������������ ���������� SPIM (Smart Phone InterModule) 
																								// - ������������ ����� �������� ������ - 128 ����;
																								// - ������ ��������� ������ � ������ (���������, CRC) - 4 �����
uint8_t pDataSLIPPack[MAX_SIZE_OF_SLIP_PACK_PAYLOAD];


#define SIZE_OF_PCK (8)

int16_t cntRecPcks = -1;

SLIPInterface* objSLIPinterface;


void UART_InitInterface(UART_HandleTypeDef *huart)
{
	HAL_UART_Receive_IT(huart,pUARTRxBuf,1);

	objSLIPinterface = new SLIPInterface;

	//��������� � ����� ���������� ������
	memset(pDataSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
	objSLIPinterface->WaitForPack();
	//������������� ��������� SLIP-���������� - �������� ������ SLIP-������
}

void UART_DeInitInterface(UART_HandleTypeDef *huart)
{
	delete objSLIPinterface;
}






void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint16_t nSizeSLIPPack = 0;	
	uint8_t nCheckState = 0;
	
  /* Prevent unused argument(s) compilation warning */
  UNUSED(huart);
	
	//������������ �������� ������
	if(objSLIPinterface->CheckForSLIPData(pUARTRxBuf[0], &pDataSLIPPack[0], nSizeSLIPPack, nCheckState))
	{
		#ifdef DEBUG_PRINTF_SLIP_DATA
		printf("SLIP Pack is Rcvd\n * Payload of Pack:");
		
		ConvertHexIntToUTF8(pDataSLIPPack,nSizeSLIPPack);
		printf((const char*)pDataSLIPPack);
		
		printf("\n");
		#endif
		
		//������� ��������� �����
		memset(pDataSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
		objSLIPinterface->WaitForPack();
	}
	
	//������� ��������� ������
	HAL_UART_Receive_IT(huart,pUARTRxBuf,1);
	
	#ifdef DEBUG_UART_TEST_TXRX
	cntRecPcks++;
			
	if(cntRecPcks>=SIZE_OF_UART_RX_BUFFER/SIZE_OF_PCK)
		cntRecPcks=0;
	
	UART_Receive_Pck_Wait(huart, receiveUARTBuffer+cntRecPcks*SIZE_OF_PCK);	
	
	UART_Send_TestData(huart);
	#endif
}

#ifdef DEBUG_UART_TEST_TXRX
void UART_Receive_Pck_Wait(UART_HandleTypeDef *huart, uint8_t *pBufForRecData)
{
	if(pBufForRecData)
		HAL_UART_Receive_DMA(huart,pBufForRecData,SIZE_OF_PCK);
	else
		HAL_UART_Receive_DMA(huart,receiveUARTBuffer,SIZE_OF_PCK);
}

void UART_Send_TestData(UART_HandleTypeDef *huart)
{
	uint8_t pBufTestData[8] = {0x00, 0x06, 0x00, 0x00, 0x48, 0x00, 0x00, 0xBE};
		
	HAL_UART_Transmit_DMA(huart, pBufTestData, 8);
}
#endif

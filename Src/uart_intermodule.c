#include "uart_intermodule.h"


#define SIZE_OF_UART_RX_BUFFER (32)
uint8_t pUARTRxBuf[SIZE_OF_UART_RX_BUFFER];

#define MAX_SIZE_OF_SLIP_PACK_PAYLOAD	(128+4)		// � ������������ � ������������ ���������� SPIM (Smart Phone InterModule) 
																								// - ������������ ����� �������� ������ - 128 ����;
																								// - ������ ��������� ������ � ������ (���������, CRC) - 4 �����
uint8_t pDataSLIPPack[MAX_SIZE_OF_SLIP_PACK_PAYLOAD];

uint8_t UTF8DataSLIPPack[2*MAX_SIZE_OF_SLIP_PACK_PAYLOAD];

uint8_t pUARTTxBuf[2*MAX_SIZE_OF_SLIP_PACK_PAYLOAD];
uint16_t nSizeTxBuf;

#define SIZE_OF_PCK (8)

int16_t cntRecPcks = -1;

SLIPInterface* objSLIPinterface;


/**
  * @brief  ������� ������������� ������ ������ �� ���������� UART
	* @param  hspi - handle UART-����������, ��� �������� ����������� �������
	*
  * @note   ������� ��������� ��������� ���������� �� ����� ������ �
	*					������������ � ���������� SLIP. ������� SLIP-����������
	*					����������� � ������ SLIPInterface, ������ �������� ���������
	*					� UART_InitInterface(). ������� ����������� UART �� ������������
	*					����� ������ ��� ���������� �� �������� � SLIP-���������
	*
	* @retval void
  */
void UART_InitInterface(UART_HandleTypeDef *huart)
{
	//����������� ���������� �� ������� ������ ������ ����� �� UART
	HAL_UART_Receive_IT(huart,pUARTRxBuf,1);

	//������� ������ ��� SLIP-����������
	objSLIPinterface = new SLIPInterface;

	//��������� SLIP-��������� � ����� �������� SLIP-������
	memset(pDataSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
	objSLIPinterface->WaitForPack();
	
	//������������� ��������� SLIP-���������� - �������� SLIP-������
}

/**
  * @brief  ������� ��������������� ������ ������ �� ���������� UART
	* @param  hspi - handle UART-����������, ��� �������� ����������� �������
	*
  * @note   ������� ��������� �������� ������� SLIP-����������, "������������"
	*					� ���������� UART
	* @retval void
  */
void UART_DeInitInterface(UART_HandleTypeDef *huart)
{
	delete objSLIPinterface;
}


/**
  * @brief  Callback-�������, ���������� �� ����������, ����������� � ���, 
	*					��� ������� ������ �� ���������� UART
	* @param  hspi - handle UART-����������
	*
  * @note   ������� ������������ �������� ������ SLIP-����������, ������� �
	*					������ ���������� �������� ������, ��������� �� �� ��������� SLIP
	*
	* @retval void
  */
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
		
		memcpy(UTF8DataSLIPPack,pDataSLIPPack,nSizeSLIPPack);
		
		ConvertHexIntToUTF8(UTF8DataSLIPPack,nSizeSLIPPack);
		printf((const char*)UTF8DataSLIPPack);
		
		printf("\n");
		#endif

		#ifdef DEBUG_SEND_RCVD_SLIP_PACK
		objSLIPinterface->FormPack(pDataSLIPPack, nSizeSLIPPack, pUARTTxBuf, nSizeTxBuf);

			#ifdef DEBUG_PRINTF_SLIP_DATA
			printf("SLIP Pack Ready for Send\n * Pack Data:");

			memcpy(UTF8DataSLIPPack,pUARTTxBuf,nSizeTxBuf);
			
			ConvertHexIntToUTF8(UTF8DataSLIPPack,nSizeTxBuf);
			printf((const char*)UTF8DataSLIPPack);
			
			printf("\n");
			#endif

		HAL_UART_Transmit_DMA(huart, pUARTTxBuf, nSizeTxBuf);
		#endif
		
		//������� ��������� �����
		memset(pDataSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
		objSLIPinterface->WaitForPack();
		
	}
	
	//������� ��������� ������
	HAL_UART_Receive_IT(huart,pUARTRxBuf,1);

}

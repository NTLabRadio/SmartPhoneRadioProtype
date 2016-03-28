#include "uart_intermodule.h"

//��������� UART �� ���������
UART_InitTypeDef DefaultUARTParams = {115200,
																			UART_WORDLENGTH_8B,
																			UART_STOPBITS_1,
																			UART_PARITY_NONE,
																			UART_MODE_TX_RX,
																			UART_HWCONTROL_NONE,
																			UART_OVERSAMPLING_16
																			};


//������ ������ ��� ������ ������ �� UART
#define SIZE_OF_UART_RX_BUFFER (32)
//����� ��� ������ ������ �� UART
uint8_t pUARTRxBuf[SIZE_OF_UART_RX_BUFFER];

//�����, � ������� ���������� �������� ������ SLIP-������, ��������� �� UART 
uint8_t pUARTRxSLIPPack[MAX_SIZE_OF_SLIP_PACK_PAYLOAD];
//������ �������� ������ SLIP-������, ��������� �� UART
uint16_t nSizeSLIPPack;	

#ifdef DEBUG_PRINTF_SLIP_DATA
uint8_t UTF8DataSLIPPack[2*MAX_SIZE_OF_SLIP_PACK_PAYLOAD];
#endif


//����� �������� ������ ��� �������� � UART
uint8_t pUARTTxPayload[MAX_SIZE_OF_SLIP_PACK_PAYLOAD];
//������ ������ �������� ������ �� �������� � UART
uint16_t nSizeTxPayload;
																			
//����� ��� �������� ������ � UART
uint8_t pUARTTxBuf[(MAX_SIZE_OF_SLIP_PACK_PAYLOAD*3)/2];
//������ ������ �� �������� � UART
uint16_t nSizeTxBuf;

//������ ��� ������ � SLIP-�����������
SLIPInterface* objSLIPinterface;

//������� ��������� ��������� ��������� UART-���������
en_UARTstates UARTstate;

//Handle UART-���������� �������� ������������ ������
UART_HandleTypeDef *huartExtDev;

uint8_t Cplt_UART_DMA_Transmit = TRUE;


/**
  * @brief  ������� ������������� ������ ������ �� ���������� UART
	*
	* @param  hspi - handle UART-����������, ��� �������� ����������� �������
	*
  * @note   ������� ��������� ��������� ���������� �� ����� ������:
	*					�������������� ������� � ���������� ����������� ���������;
	* 				����������� UART �� ����� ������ � ������������� ��������� 
	*					��������� ��������� ��������� UART-���������
	*
	* @retval void
  */
void UART_InitInterface(UART_HandleTypeDef *huart)
{
	huartExtDev = huart;
	
	//�������������� ������� � ���������� ����������� ��������� 
	//����������������� �����
	InitSerialProtocol();
	
	//������� ������ �� UART
	WaitNextByteFromUART(huart);
	
	//������������� ��������� ��������� ��������� ��������� UART-���������
	UARTstate = UART_IDLE;
	
	//������������� ��������� SLIP-���������� - �������� SLIP-������
}

/**
  * @brief  ������� ��������������� ������ ������ �� ���������� UART
	*
	* @param  hspi - handle UART-����������, ��� �������� ����������� �������
	*
  * @note   ������� ��������� �������� ������� SLIP-����������, "������������"
	*					� ���������� UART
	* @retval void
  */
void UART_DeInitInterface(UART_HandleTypeDef *huart)
{
	huartExtDev = NULL;
	
	delete objSLIPinterface;
}


/**
  * @brief  ������� ������������� �������� � ���������� ����������� ���������
	*					����������������� �����
	* @param  ���
  * @note   ������� ��������� ��������� ���������� �� ����� ������ �
	*					������������ � ���������� SLIP. ������� SLIP-����������
	*					����������� � ������ SLIPInterface, ������ �������� ���������
	*					� InitSerialProtocol()
	* @retval void
  */
void InitSerialProtocol()
{
	//������� ������ ��� SLIP-����������
	objSLIPinterface = new SLIPInterface;

	//��������� SLIP-��������� � ����� �������� SLIP-������
	memset(pUARTRxSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
	nSizeSLIPPack = 0;
	objSLIPinterface->WaitForPack();
}

/**
  * @brief  ������� ��������������� �������� � ���������� ����������� ���������
	*					����������������� �����
	* @retval void
  */
void DeinitSerialProtocol()
{
	delete objSLIPinterface;
}


/**
  * @brief  Callback-�������, ���������� �� ����������, ����������� � ���, 
	*					��� ������� ������ �� ���������� UART
	*
	* @param  huart - handle UART-����������
	*
  * @note   ������� ������������ �������� ������ ����������� �����������
	*					����������� ���������� (SLIP-���������), ������� � ������ 
	*					���������� �������� ������, �������� �� ����
	*
	* @retval void
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	#ifdef DEBUG_USE_TL_LINES
	TL2_HIGH();
	#endif
	
	if(huart!=huartExtDev)
		return;
	
	//�������� �������� ������ ����������� ����������� ����������� ����������
	if(CheckForSerialProtocolData(pUARTRxSLIPPack,nSizeSLIPPack))
	{//���� ������� �������� ������
		#ifdef DEBUG_PRINTF_SLIP_DATA
		printf("SLIP Pack is Rcvd\n * Payload of Pack:");
		
		memcpy(UTF8DataSLIPPack,pUARTRxSLIPPack,nSizeSLIPPack);
		
		ConvertHexIntToUTF8(UTF8DataSLIPPack,nSizeSLIPPack);
		printf((const char*)UTF8DataSLIPPack);
		
		printf("\n");
		#endif

		#ifdef DEBUG_SEND_RCVD_SLIP_PACK
		objSLIPinterface->FormPack(pUARTRxSLIPPack, nSizeSLIPPack, pUARTTxBuf, nSizeTxBuf);

			#ifdef DEBUG_PRINTF_SLIP_DATA
			printf("SLIP Pack Ready for Send\n * Pack Data:");

			memcpy(UTF8DataSLIPPack,pUARTTxBuf,nSizeTxBuf);
			
			ConvertHexIntToUTF8(UTF8DataSLIPPack,nSizeTxBuf);
			printf((const char*)UTF8DataSLIPPack);
			
			printf("\n");
			#endif

		HAL_UART_Transmit_DMA(huart, pUARTTxBuf, nSizeTxBuf);
		#endif
		
		//���������, ��� �������� ������ ���������� ����������
		UARTstate = UART_DATA_RX_NEED_TO_PROCESS;
		
		//������� ��������� �����
		//memset(pUARTRxSLIPPack,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);
		objSLIPinterface->WaitForPack();
	}
	
	//������� ��������� ������
	WaitNextByteFromUART(huart);
	
	#ifdef DEBUG_USE_TL_LINES
	TL2_LOW();
	#endif

}


/**
  * @brief  ��������� UART-���������� �� �������� ��������� ������ ������
	*/
void WaitNextByteFromUART(UART_HandleTypeDef *huart)
{
	//������� ��������� 
	HAL_UART_Receive_DMA(huart,pUARTRxBuf,1);
}


/**
  * @brief  ����� � ������ ������, �������� �� UART, �������� ������,
	*					� ������� ������������� ��������� ��� ����������������� �����
	*
  * @note   ������� �������� ����������� ��������� ������, ������� 
	*					�������� �� ��������� pUARTRxBuf. � ������ ���������� 
	*					������ � ����� ������, ������� ���������� ��� ���������
	*					���������� ��������� ������ � �������� �������� �������� ������
	*					�� ��������� pUARTRxSLIPPack
	*
	* @retval ��������� ������:
	*					0 - SLIP-����� �� ������;
	*					1 - SLIP-����� ������ ���������;
	*/
uint8_t CheckForSerialProtocolData(uint8_t* pPayloadPackData, uint16_t& nSizePackData)
{
	uint8_t nCheckState = 0;
	nSizePackData = 0;
	
	//TODO �������� ������ ���������� � ���� � ��� �� �����, �� ��������� ���� ������ ����� �������� ��������
	// �������. ������� ������ ���������� �� ������������ � �������
	return(objSLIPinterface->CheckForSLIPData(*pUARTRxBuf, pPayloadPackData, nSizePackData, nCheckState));
}

/**
  * @brief  ������� �������� �������� ������ �������� ���������� ����������
	*
	* @param  pData - ��������� �� �������� ������, ������� ������ ���� �������� � UART
	* @param  nSizeData - ������ �������� ������, ����
	*
  * @note   ������� �� �������� ������ ��������� SLIP-����� � �������� ��� � UART
	*
	* @retval ���
	*/
void SendDataToExtDev(uint8_t* pData, uint16_t nSizeData)
{
	memcpy(pUARTTxPayload,pData,nSizeData);
	nSizeTxPayload = nSizeData;

	//������� ��������� �������� ���������� ������
	while(!Cplt_UART_DMA_Transmit)
	{
	}
	
	objSLIPinterface->FormPack(pUARTTxPayload, nSizeTxPayload, pUARTTxBuf, nSizeTxBuf);

		#ifdef DEBUG_PRINTF_SLIP_DATA
		printf("SLIP Pack Ready for Send\n * Pack Data:");

		memcpy(UTF8DataSLIPPack,pUARTTxBuf,nSizeTxBuf);
		
		ConvertHexIntToUTF8(UTF8DataSLIPPack,nSizeTxBuf);
		printf((const char*)UTF8DataSLIPPack);
		
		printf("\n");
		#endif

	HAL_UART_Transmit_DMA(huartExtDev, pUARTTxBuf, nSizeTxBuf);
	
	Cplt_UART_DMA_Transmit = FALSE;

}

 void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	Cplt_UART_DMA_Transmit = TRUE;
}

#include "RadioLogic.h"


//������ ����������� ��� ��������
uint8_t RadioPackForSend[MAX_RADIOPACK_SIZE];

//������ ��������� �����������
uint8_t RadioPackRcvd[MAX_RADIOPACK_SIZE+SIZE_OF_RADIO_STATUS];

#ifdef DEBUG_CHECK_ERRORS_IN_SEND_RADIO_PACKS				
uint16_t g_cntCC1120_TxDataErrors = 0;
#endif


void FormAndSendRadioPack(uint8_t* pPayloadData, uint16_t nPayloadSize, uint8_t nPayloadDataType)
{	
	RadioMessage RadioMsgToSend;
	
	//��������� ����������
	FormRadioPack(&RadioMsgToSend,pPayloadData,nPayloadSize,nPayloadDataType);
	
	//���������� �������������� ����� � ����
	#ifndef TEST_RADIO_IMITATE
	SendRadioPackToTansceiver(RadioMsgToSend.Data, RadioMsgToSend.Size);
	#else
	//� ������ �������������� �� ���������� � ����, � ����������� � ����� ��� ��������������� �� ���������� ��������
	//����������� ������ �������� ��������
	RadioImitator_TxData(pPayloadData, nPayloadSize);
	#endif
	
	return;
}


uint8_t SendRadioPackToTansceiver(uint8_t* pData, uint16_t nSizeData)
{
	if(!CC1120_TxData(&g_CC1120Struct, pData, nSizeData))
	{
		#ifdef DEBUG_CHECK_ERRORS_IN_SEND_RADIO_PACKS				
		g_cntCC1120_TxDataErrors++;
		#endif
		return(0);
	}

	return(1);
}


void FormRadioPack(RadioMessage* RadioPack, uint8_t* pPayloadData, uint16_t nPayloadSize, uint8_t nPayloadDataType)
{
	//������������� ����������������� �����
	uint8_t dstAddress = RADIO_BROADCAST_ADDR;
	//����������� ����� ����� �� �������� �����������
	uint8_t srcAddress = pobjRadioModule->GetRadioAddress();
	//��� ������������ ������
	uint8_t dataType = nPayloadDataType;
	//������ �������� ������ � ������
	uint8_t dataSize = nPayloadSize;
	
	//������ �����������
	uint8_t pBodyData[MAX_RADIOPACK_SIZE];
	memset(pBodyData,RADIOPACK_DATAMODE_SIZE,0);	
	//������ ������
	uint8_t nBodySize;
	switch(nPayloadDataType)
	{
		case RadioMessage::RADIO_DATATYPE_VOICE:
			nBodySize	= RADIOPACK_VOICEMODE_SIZE;
			break;
		case RadioMessage::RADIO_DATATYPE_CONF_DATA:
			nBodySize	= RADIOPACK_DATAMODE_SIZE;
			break;
		case RadioMessage::RADIO_DATATYPE_UNCONF_DATA:
			nBodySize	= RADIOPACK_DATAMODE_SIZE;
			break;		
		default:
			nBodySize	= RADIOPACK_DATAMODE_SIZE;
	}
	
	//�������� �������� ������ � ������ ���� ������, ��������� - ����
	memcpy(pBodyData,pPayloadData,nPayloadSize);
	
	//��������� ����� ��� ������������ ����������
	RadioPack->setHeader(dstAddress,srcAddress,dataType,dataSize);
	RadioPack->setBody(pBodyData,nBodySize);

	return;
}


void ProcessRadioPack(uint8_t* pPayloadData, uint16_t& nPayloadSize, uint8_t& nDataType)
{
	nPayloadSize = 0;
	
	uint16_t nSizeOfRecData = CC1120_RxFIFONumBytes(g_CC1120Struct.hSPI);
	
	//����, ����������� �� ��, ��� ������ �� ������ RxFIFO ���������
	uint8_t flDataRcvdFromCC1120 = false;
	
	//������ ������ �� ������ RxFIFO ������ ���� ������ ������ ���������� (��������� � �� ������� �������)
	if((nSizeOfRecData<=MAX_RADIOPACK_SIZE) && (nSizeOfRecData))
	{
		flDataRcvdFromCC1120 = true;
		CC1120_RxData(&g_CC1120Struct,RadioPackRcvd,&nSizeOfRecData);
	}

	//��������� ��������� �������� � ����� ������
	CC1120_Rx(g_CC1120Struct.hSPI);
	
	//���� �� ������ RxFIFO ������ �� ���������, �� � ������������ ������
	if(!flDataRcvdFromCC1120)
		return;	

	//� ����� ��������� ������ ������������� ������-�����, ������ ��
	//TODO ������� ������� ��������� ���� ������
	nSizeOfRecData-=SIZE_OF_RADIO_STATUS;
	
	RadioMessage RadioMsgRcvd(RadioPackRcvd,nSizeOfRecData);

	//������ ��������� ��������������
	uint8_t dstAddress = RadioMsgRcvd.getDstAddress();
	uint8_t srcAddress = RadioMsgRcvd.getSrcAddress();
	nDataType = RadioMsgRcvd.getDataType();
	
	//������ ���� ��������������
	nPayloadSize = RadioMsgRcvd.getBody(pPayloadData);

	return;
}


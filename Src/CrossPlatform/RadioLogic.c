#include "RadioLogic.h"


//������ ����������� ��� ��������
uint8_t RadioPackForSend[MAX_RADIOPACK_SIZE];

//������ ��������� �����������
uint8_t RadioPackRcvd[MAX_RADIOPACK_SIZE+SIZE_OF_RADIO_STATUS];


void FormAndSendRadioPack(uint8_t* pPayloadData, uint16_t nPayloadSize)
{	
	RadioMessage RadioMsgToSend;
	
	//��������� ����������
	FormRadioPack(&RadioMsgToSend,pPayloadData,nPayloadSize);
	
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


void SendRadioPackToTansceiver(uint8_t* pData, uint16_t nSizeData)
{
	CC1120_TxData(&g_CC1120Struct, pData, nSizeData);
}


void FormRadioPack(RadioMessage* RadioPack, uint8_t* pPayloadData, uint16_t nPayloadSize)
{
	//������������� ����������������� �����
	uint8_t dstAddress = RADIO_BROADCAST_ADDR;
	//����������� ����� ����� �� �������� �����������
	uint8_t srcAddress = pobjRadioModule->GetRadioAddress();
	//��� ������������ ������
	uint8_t dataType = RADIO_DATATYPE_VOICE;
	
	//��������� ����� ��� ������������ ����������
	RadioPack->setHeader(dstAddress,srcAddress,dataType);
	RadioPack->setBody(pPayloadData,nPayloadSize);

	return;
}


void ProcessRadioPack(uint8_t* pPayloadData, uint16_t& nPayloadSize)
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

	nPayloadSize = RadioMsgRcvd.getBody(pPayloadData);

	return;
}


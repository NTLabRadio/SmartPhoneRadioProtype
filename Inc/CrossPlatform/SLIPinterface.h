/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** ���� ���� ��������� ����� SLIPInterface, ���������� � ���� ����� ��������
** ������� � ������� ��� ���������� ���������� SLIP �������� ��������� RFC 1055
**
****************************************************************************/

#ifndef SLIPINTERFACE_H
#define SLIPINTERFACE_H

#include <stdint.h>
#include <limits.h>

//������������ ������ �������� �������� SLIP-�������
#define MAX_SIZE_OF_SLIP_PACK_PAYLOAD	(128+4)		// � ������������ � ������������ ���������� SPIM (Smart Phone InterModule)
                                                                                        // - ������������ ����� �������� ������ - 128 ����;
                                                                                        // - ������ ��������� ������ � ������ (���������, CRC) - 4 �����

class SLIPInterface
{
public:
  SLIPInterface();

	static uint8_t FindPackInData(uint8_t* pData, uint16_t nDataSize, uint8_t* pPayloadPackData, uint16_t& nPayloadPackSize, uint16_t& nPosEndOfPack);
	static uint8_t FormPack(uint8_t* pPayloadData, uint16_t nPayloadSize, uint8_t* pPackData, uint16_t& nPackSize, uint16_t nMaxPackSize = USHRT_MAX);

	uint8_t CheckForSLIPData(uint8_t nByte, uint8_t* pPayloadPackData, uint16_t &nPayloadPackSize, uint8_t &nCheckState);

	void WaitForPack();

	//������������ ������ SLIP-������
	//�������� SLIP ��������� ����-�������� ����� ����������� ������ ������ � 2 ���� + 2 ������� FEND
	static const uint16_t  MAX_SIZE_OF_PACK = (2*MAX_SIZE_OF_SLIP_PACK_PAYLOAD + 2);

private:
	//����������� ������� ��������� SLIP
	static const uint8_t FEND = (0xC0);      /* "Frame End" symbol, i.e. begin/end of packet */
	static const uint8_t FESC = (0xDB);      /* "Frame Escape" symbol */
	static const uint8_t TFEND = (0xDC);    /* "Transposed Frame End" */
	static const uint8_t TFESC = (0xDD);    /* "Transposed Frame Escape" */

	//��������� ���������� ���������� �������
	static const uint8_t RESULT_SUCCESS = (0);
	static const uint8_t RESULT_FAIL = (UCHAR_MAX);

	//��������� ��������� �������� ������ SLIP-������ � ������� ������ (������������ FindPackInData())
  enum en_FindPackStates
  {
		PACK_NO_FOUND,         //����� �� ������
		PACK_BEGIN_FOUND,    //������ �����-���� ������
		PACK_FOUND                //������ ���� �����
  };
	
	//��������� ��������� ���������� SLIP�
	enum en_InterfaceStates
	{
		STATE_IDLE,
		STATE_WAIT_FOR_BEGIN_OF_PACK,
		STATE_WAIT_FOR_END_OF_PACK
	};

	//��������� ���������� SLIP, �������������� ����� ������ ��������
	en_InterfaceStates InterfaceState;

	uint8_t BufForSLIPData[MAX_SIZE_OF_PACK];
	
	uint8_t* pPackData;	//������ ������, ����������� ����������� SLIP
	uint16_t nPackSize;	//������ ������ ������, ����������� ����������� SLIP
};

#endif // SLIPINTERFACE_H

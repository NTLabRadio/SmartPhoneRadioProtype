#include "slipinterface.h"

SLIPInterface::SLIPInterface()
{
	InterfaceState = STATE_IDLE;
	
	nPackSize = 0;
	pPackData = &BufForSLIPData[0];
}

void SLIPInterface::WaitForPack()
{
	InterfaceState = STATE_WAIT_FOR_BEGIN_OF_PACK;
	
	nPackSize = 0;
	pPackData = &BufForSLIPData[0];	
}

uint8_t SLIPInterface::CheckForSLIPData(uint8_t nByte, uint8_t* pPayloadPackData, uint16_t &nPayloadPackSize, uint8_t &nCheckState)
{
	uint16_t nPosEndOfPack;
	
	switch(InterfaceState)
	{
		case STATE_WAIT_FOR_BEGIN_OF_PACK:
			if(nByte==FEND)	//���� ������ ������ ������ ������
			{
				//�������� ��������� ���������� �� �������� ����� ������
				InterfaceState = STATE_WAIT_FOR_END_OF_PACK;

				//�������� ���������� ������
				nPackSize = 0;
				pPackData = &BufForSLIPData[0];
				
				*pPackData++ = nByte;
				nPackSize++;
			}
			break;
		case STATE_WAIT_FOR_END_OF_PACK:
			//��������, �� �������� �� ������� ������������� ������ ���������� ������ ������	
			if(nPackSize>=SIZE_BUF_FOR_SLIP_DATA-1)
			{
				nCheckState = 0xFF;
				return(0);
			}
		
			//����������� ������
			*pPackData++ = nByte;
			nPackSize++;
		
			if(nByte==FEND)	//���� ������ ������ ����� ������
			{
				//�������� �������� ���������� �� �����������
				InterfaceState = STATE_IDLE;				
				
				//������������ ����������� ������, �������� �������� ����-���������
				FindPackInData(&BufForSLIPData[0], nPackSize, pPayloadPackData, nPayloadPackSize, nPosEndOfPack);				
				
				nCheckState = InterfaceState;
				return(1);
			}				
			break;
		case STATE_IDLE:
			break;
	}
	
	nCheckState = InterfaceState;
	return(0);
}

//---------------------------------------------------------------------------------------------------
// ������� ������ SLIP ������ � ������� ������
//
// ������� ���� SLIP-�����, ������� � ������� ������� �������� �������, �������
// ���������� � ������������� ����������� �������� FEND. � ������ ����� 2-�������
// ESC-������������������ ���������� �� ��������������� �������
//
// ���������:
//  pData - ��������� �� ������ ������� ������;
//  nDataSize - ������ �������� �������, � ������;
//  pPackData - ��������� �� ������ ���������� ������;
//  nPackSize - ������ ������, � ������;
//  nPackEndPosInData - ������� ���������� ������� ���������� ������
//
// ������������ ��������:
//  0 - ����� �� ������ � ������;
//  1 - ������� ������ ������, �� �� ������ �����;
//  2 - ����� ������ � ������� ���������;
//  0xFFFF - ������ � ������� ���������� ������ (������ ������ FESC, �� ��������
//  � ESC-������������������)
//
// ��������! ������ ��� pPayloadPackData ������ ���� �������� ��������������
//---------------------------------------------------------------------------------------------------
uint8_t SLIPInterface::FindPackInData(uint8_t* pData, uint16_t nDataSize, uint8_t* pPayloadPackData, uint16_t& nPayloadPackSize, uint16_t& nPosEndOfPack)
{
    uint16_t cntBytes = nDataSize;
    enFindPackState stateFindAutom = PACK_NO_FOUND;
    nPayloadPackSize = 0;

    while(cntBytes--)
    {
        switch(*pData)
        {
            case FEND:
                if(stateFindAutom==PACK_NO_FOUND)
                {
                    stateFindAutom = PACK_BEGIN_FOUND;
                }
                else if(stateFindAutom==PACK_BEGIN_FOUND)
                {
                    nPosEndOfPack  = nDataSize - cntBytes;      //���������� ������� ����� ������, ����� ������� �� �� �������
                    stateFindAutom = PACK_FOUND;
                    cntBytes = 0;   //����� ������ � ��������� - ����� �������� �� ����� � ������� ��������� ���������� �������
                    break;
                }
                break;
            case FESC:
                if(stateFindAutom==PACK_BEGIN_FOUND)    //������������ ������ FESC ������, ���� ������� ������ ������ � ���� ��������� ��� ������
                {
                    //������ FESC ������ ���� ������ �������� 2-������� ESC-������������������
                    //��� ����������� ESC-������������������ ��������� ��������� ����
                    cntBytes--;
                    if(!cntBytes)   //���� ����������� ������
                    {
                        #ifdef QTDEBUG_SLIP_PRINTOUT_EXCEPTIONS
                        qDebug() << "WARNING! ::FindSLIPPackInData() � �������� ������ ������� ������, �� �� ������� ����� SLIP ������";
                        #endif
                        return(stateFindAutom); //������, ��� �����, ����� �� ��������� �� �����. ��������� ���������� �������
                    }

                    //����� ����� ������� ��������� �� ��������� ������ ������
                    pData++;

                    //������ ������ ESC-������������������ �� ���� ���� (FEND ��� FESC)
                    if(*pData == TFEND)
                    {
                        *pPayloadPackData++ = FEND;
                        nPayloadPackSize++;
                    }
                    else if (*pData == TFEND)
                    {
                        *pPayloadPackData++ = FESC;
                        nPayloadPackSize++;
                    }
                    else
                    {
                        #ifdef QTDEBUG_SLIP_PRINTOUT_EXCEPTIONS
                        qDebug() << "WARNING! ::FindSLIPPackInData() � �������������� SLIP-������ ������ �������� ������ FESC, �� �������� � ESC-������������������";
                        #endif
                        return(RES_FAIL);
                    }
                }
                break;
            default:
                //���� ������������ ����� � �������� ������ - �� FEND � FESC, �� ������ �������� ���
                if(stateFindAutom==PACK_BEGIN_FOUND)
                {
                    *pPayloadPackData++ = *pData;
                    nPayloadPackSize++;
                }
                break;
        }   //end of switch
				pData++;
    }   //end of while

    return(stateFindAutom);
}


//---------------------------------------------------------------------------------------------------
// ������� ������������ SLIP ������
//
// ������� ��������� � ������ � ����� �������� ������ ������ FEND � ���������
// �������� ����-��������� ��� �������
//
// ���������:
//  pPayloadData - ��������� �� ������ �������� �������� ������;
//  nPayloadSize - ������ �������� �������� ������, � ������;
//  pPackData - ��������� �� ������ ��������������� SLIP-������;
//  nPackSize - ������ ��������������� SLIP-������, � ������;
//
// ������������ ��������:
//  0 - ����� ������� �����������;
//  0xFFFF - ��� ������������ ������ ��������� ������, ��������� ������:
//      - ���� �� ����������, ���������� � �������- �������;
//      - ��������� ������������ ������ ������ - ������ ���������� ����������� (2);
//      - ��� ������������ �������� ����������� ���������� ������ ������
//
// ��������! ������ ��� pPackData ������ ���� �������� ��������������
//---------------------------------------------------------------------------------------------------
uint8_t SLIPInterface::FormPack(uint8_t* pPayloadData, uint16_t nPayloadSize, uint8_t* pPackData, uint16_t& nPackSize, uint16_t nMaxPackSize)
{
    nPackSize = 0;

    //�������� ����������, ���������� � �������
    if(!pPayloadData || !pPackData)
        return(RES_FAIL);

    //����� ������ �������� �� ����� ��� �� 2� �������� FEND
    if(nMaxPackSize<2)
        return(RES_FAIL);

    //������ ������ ������ - FEND
    *pPackData++ = FEND;
    nPackSize++;

    while(nPayloadSize--)
    {
        switch(*pPayloadData)
        {
            case FEND:
            case FESC:
                //� ����� ���������� �������� �� ����� 3 ��������: ESC-������������������ + ������ ����� ������ (FEND)
                //���������, �� �������� �� ������ ���������
                if(nPackSize>nMaxPackSize-3)
                {
                    #ifdef QTDEBUG_SLIP_PRINTOUT_EXCEPTIONS
                    qDebug() << "WARNING! ::FormSLIPPack() ������ ������������ SLIP ������ �������� ����������";
                    #endif
                    return(RES_FAIL);
                }

                //� ����� ���������� ESC-������������������
                nPackSize+=2;
                *pPackData++ = FESC;
                if(*pPayloadData==FEND)
                    *pPackData++ = TFEND;
                else
                    *pPackData++ = TFESC;

                pPayloadData++;
                break;
            default:
                //� ����� ���������� �������� �� ����� 2 ��������: ������ ������ + ������ ����� ������ (FEND)
                //���������, �� �������� �� ������ ���������
                if(nPackSize>nMaxPackSize-2)
                {
                    #ifdef QTDEBUG_SLIP_PRINTOUT_EXCEPTIONS
                    qDebug() << "WARNING! ::FormSLIPPack() ������ ������������ SLIP ������ �������� ����������";
                    #endif
                    return(RES_FAIL);
                }

                //� ����� ��������������� ���������� ������ �������� ��������
                nPackSize++;
                *pPackData++ = *pPayloadData;

                pPayloadData++;
                break;
        }   //end of switch
    }   //end of while

    //NO: ��� ���������� ���������� ������� � ����� ����������� ���������� ������ ������ ����� �� ����� ��������,
    //�������� ���� ��������� �����

    //��������� ������ ������ - FEND
    *pPackData++ = FEND;
    nPackSize++;

    return(RES_SUCCESS);
}

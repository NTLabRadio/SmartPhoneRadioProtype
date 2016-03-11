#include "SLIPinterface.h"

SLIPInterface::SLIPInterface()
{
	InterfaceState = STATE_IDLE;
	
	nPackSize = 0;
	pPackData = &BufForSLIPData[0];
}

/**
    * @brief  ������������� ��������� ������ SLIP-������
	* @param  ���
	*
    * @note   ������� �������������� ��������� ���������
	*					������ SLIP-������: ������������� ���������
	*					��������� ������, ���������� ������� �����������
	*					������, ������������� ��������� ������ �� ������
	*					��������������� ��������� ������
	*
	* @retval void
  */
void SLIPInterface::WaitForPack()
{
	InterfaceState = STATE_WAIT_FOR_BEGIN_OF_PACK;
	
	nPackSize = 0;
	pPackData = &BufForSLIPData[0];	
}


/**
    * @brief  ������� ������ SLIP-������ � ��������� ������
	*					� ���������� �� ���������� ������	�������� ������
	*
	* @param  nStreamDataByte - ������� ������ ��������� ������;
    * @param	pPayloadPackData - ���������, �� �������� ������ ����
	*					�������� �������� ������, ����������� �� ���������� 
	*					SLIP-������
	* @param	nPayloadPackSize - ������ �������� ������, ����;
	* @param	nCheckState - ������� ��������� �������� ������ 
	*					������ SLIP-���������:
	*						0 - idle-���������;
	*						1 - ����� ������ ������;
	*						2 - ����� ����� ������;
	*						255 - ������ � ������ (�������� ����������� 
	*						���������� ������ ������ ��� ��.)
	*
    * @note   ������� �������� ����������� ��������� ������ ��
	*					������� ���������� � ��� SLIP-������, ������� 
	*					���������� � ������������� ������������ ���������.
	*					� ������ ���������� ������ SLIP-������, �������
	*					������������� �������� ����������� �������� ������.
	*					� �������� ������ SLIP-������ ������� ���������
	*					�������� ����-��������� ��� ���� � �� ���������
	*					������ ���������� �������� ������ ������ ���������� 
	*					�������
	*
	* @retval ��������� ������:
	*					0 - SLIP-����� �� ������;
	*					1 - SLIP-����� ������ ���������;
  */
uint8_t SLIPInterface::CheckForSLIPData(uint8_t nStreamDataByte, uint8_t* pPayloadPackData, uint16_t &nPayloadPackSize, uint8_t &nCheckState)
{
	uint16_t nPosEndOfPack;
	
	switch(InterfaceState)
	{
		case STATE_WAIT_FOR_BEGIN_OF_PACK:
			if(nStreamDataByte==FEND)	//���� ������ ������ ������ ������
			{
				//�������� ��������� ���������� �� �������� ����� ������
				InterfaceState = STATE_WAIT_FOR_END_OF_PACK;

				//�������� ���������� ������
				nPackSize = 0;
				pPackData = &BufForSLIPData[0];
				
				*pPackData++ = nStreamDataByte;
				nPackSize++;
			}
			break;
		case STATE_WAIT_FOR_END_OF_PACK:
			//��������, �� �������� �� ������� ������������� ������ ���������� ������ ������	
            if(nPackSize>=MAX_SIZE_OF_PACK-1)
			{
				nCheckState = 0xFF;
				return(0);
			}
		
			//����������� ������
			*pPackData++ = nStreamDataByte;
			nPackSize++;
		
			if(nStreamDataByte==FEND)	//���� ������ ������ ����� ������
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


/**
    * @brief  ������� ������ SLIP ������ � ������� ������
	*
	* @param  pData - ��������� �� ������ ������� ������;
	* @param  nDataSize - ������ �������� �������, � ������;
	* @param  pPackData - ��������� �� ������ ���������� ������;
	* @param  nPackSize - ������ ������, � ������;
	* @param	nPackEndPosInData - ������� ���������� ������� ���������� ������
	*
    * @note   ������� ���� SLIP-����� � ������ ������� ������. ����� �����������
    *                  ������� � ������� ������� �������� �������. ��� ���������� ������
    *                  SLIP-������ ����� ������������.
    * 					��������� SLIP-������ �������� ����������� ������ FEND, � ��������
    *                  ����� ���������� � ������� �� �������������.
    *					������� ���������� �������� �������� ������, ��� �������� FEND �
    *                  � ����������� ��������� ����-�������� ��� �������
	* @note		��������! ������ ��� pPayloadPackData ������ ���� �������� 
	*					��������������
	*
	* @retval ��������� ������:
	*   			0 - ����� �� ������ � ������;
	*   			1 - ������� ������ ������, �� �� ������ �����;
	*   			2 - ����� ������ � ������� ���������;
	*   			0xFFFF - ������ � ������� ���������� ������ (������ ������ FESC, �� ��������
	*   			� ESC-������������������)
  */
uint8_t SLIPInterface::FindPackInData(uint8_t* pData, uint16_t nDataSize, uint8_t* pPayloadPackData, uint16_t& nPayloadPackSize, uint16_t& nPosEndOfPack)
{
    uint16_t cntBytes = nDataSize;
    en_FindPackStates stateFindAutom = PACK_NO_FOUND;
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
                    else if (*pData == TFESC)
                    {
                        *pPayloadPackData++ = FESC;
                        nPayloadPackSize++;
                    }
                    else
                    {
                        #ifdef QTDEBUG_SLIP_PRINTOUT_EXCEPTIONS
                        qDebug() << "WARNING! ::FindSLIPPackInData() � �������������� SLIP-������ ������ �������� ������ FESC, �� �������� � ESC-������������������";
                        #endif
                        return(RESULT_FAIL);
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


/**
    * @brief  ������� ������������ SLIP ������
	*
	* @param  pPayloadData - ��������� �� ������ �������� �������� ������;
	* @param  nPayloadSize - ������ �������� �������� ������, � ������;
	* @param  pPackData - ��������� �� ������ ��������������� SLIP-������;
	* @param  nPackSize - ������ ��������������� SLIP-������, � ������;
	*
    * @note   ������� ��������� � ������ � ����� �������� ������ ������ FEND
    *					� ��������� �������� ����-��������� ��� �������
    * @note ��������! ������ ��� pPackData ������ ���� �������� ��������������
	*
	* @retval ��������� ���������� �������:
	*					0 - ����� ������� �����������;
	*  				0xFFFF - ��� ������������ ������ ��������� ������, ��������� ������:
    *                  - ���� �� ����������, ���������� � �������- �������;
    *                  - ��������� ������������ ������ ������ - ������ ���������� ����������� (2);
	*  			    - ��� ������������ �������� ����������� ���������� ������ ������
  */
uint8_t SLIPInterface::FormPack(uint8_t* pPayloadData, uint16_t nPayloadSize, uint8_t* pPackData, uint16_t& nPackSize, uint16_t nMaxPackSize)
{
    nPackSize = 0;

    //�������� ����������, ���������� � �������
    if(!pPayloadData || !pPackData)
        return(RESULT_FAIL);

    //����� ������ �������� �� ����� ��� �� 2� �������� FEND
    if(nMaxPackSize<2)
        return(RESULT_FAIL);

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
                    return(RESULT_FAIL);
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
                    return(RESULT_FAIL);
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

    return(RESULT_SUCCESS);
}

#include "RadioModuleFuncs.h"


//��� �����������
en_RadioChanTypes g_RadioChanType = RADIOCHAN_TYPE_IDLE;
//����� �������� �� �������
en_RadioSignalPowers	g_RadioSignalPower = RADIO_SIGNALPOWER_LOW;
//����� ����������������
en_ARMPowerModes g_RadioPowerMode = ARM_POWERMODE_NORMAL;

//������� ������� ��������/������
uint16_t g_TxRadioFreq = 0;
uint16_t g_RxRadioFreq = 0;

uint8_t g_AudioInLevel = 0;
uint8_t g_AudioOutLevel = 0;

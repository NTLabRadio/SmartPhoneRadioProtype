/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** ���� ���� ��������� ����� SPIMMessage, ���������� � ���� ����� ��������
** ������� � ������� ��� ���������� ����������� ���������� SPIM (Smart Phone 
** InterModule). ����� ��������� ����������� � ������������ ��������� �������, 
** ������������� ����������� SPIM
**
****************************************************************************/

#ifndef SPIMMESSAGE_H
#define SPIMMESSAGE_H

#include <stdint.h>
#include <string.h>

enum en_SPIMcmds
{
	SPIM_CMD_NOP										=0x00,
	SPIM_CMD_NOP_BACK								=0x01,
	SPIM_CMD_SET_MODE								=0x02,
	SPIM_CMD_SET_MODE_BACK					=0x03,
	SPIM_CMD_SEND_DATA_FRAME				=0x04,
	SPIM_CMD_SEND_DATA_FRAME_BACK		=0x05,
	SPIM_CMD_TAKE_DATA_FRAME				=0x06,
	SPIM_CMD_TAKE_DATA_FRAME_BACK		=0x07,
	SPIM_CMD_REQ_CURRENT_PARAM 			=0x08,
	SPIM_CMD_REQ_CURRENT_PARAM_BACK =0x09,
	SPIM_CMD_SOFT_VER								=0x0A,
	SPIM_CMD_SOFT_VER_BACK					=0x0B
};

enum en_SPIMaddrs
{
	SPIM_ADDR_STM32									=0x1,		//���������� STM32 �������� ���������� (����������)
	SPIM_ADDR_EXTDEV								=0x2		//������� ����������� ���������� (��������� NT1004, �� ��� ��.)
};

enum en_SPIMReqTypes
{
	SPIM_REQTYPE_SINGLE							=0,			//��������� ���������� ������ - ������, ����� �� ������� ������ ���� ������ ������� 
																					//����������� ���������� � ������ ��������� �������
	SPIM_REQTYPE_ASYNC							=1			//����������� ������ - ������, � ����� �� ������� �������� (��������, RSSI) ���������� 
																					//������� ����������� � ������������ ������ �������, ��� ��������� ��� ��������
};


class SPIMMessage
{
public:
	
	SPIMMessage();
	SPIMMessage(const uint8_t* pMsgData, uint16_t msgSize);
	~SPIMMessage();

	uint8_t* Data;
	uint8_t Size;

	uint8_t* Body;

	uint8_t setHeader(uint8_t bodySize, uint8_t address, uint8_t noMsg, uint8_t IDcmd);
	uint8_t setBody(uint8_t* pBodyData, uint8_t bodySize);
	uint8_t setCRC();
	uint8_t setMsg(uint8_t* pMsgData, uint8_t msgSize);

	uint8_t getHeader(uint8_t* pHeaderData);
	uint8_t getBody(uint8_t* pBodyData);
	uint8_t getCRC();

	uint8_t checkCRC();

	uint8_t getAddress();
	uint8_t getNoMsg();
	uint8_t getSizeBody();
	uint8_t getIDCmd();
	
	uint8_t IDBackCmd(uint8_t IDCmd);
	
	class CmdReqParam
	{
		public:
			static uint8_t OpModeCode(uint8_t RadioChanType, uint8_t SignalPower, uint8_t ARMPowerMode);
			static uint8_t AudioCode(uint8_t AudioOutLevel, uint8_t AudioInLevel);
			
			void SetPointerToMessage(SPIMMessage* mes);
		
			uint8_t MaskReqParam();
		
			uint8_t isOpModeReq();
			uint8_t isAudioReq();
			uint8_t isRxFreqReq();
			uint8_t isTxFreqReq();
			uint8_t isRSSIReq();
			uint8_t isChanStateReq();
		
		private:
			SPIMMessage* objSPIMMessage;			
		
			//--------- ��� �������� ������ ---------------
			//��� �����������
			#define SHIFT_RADIOCHANTYPE_IN_OPMODECODE		(0)
			#define MASK_RADIOCHANTYPE_IN_OPMODECODE		(3)
			//�������� ������� �����������
			#define SHIFT_SIGNALPOWER_IN_OPMODECODE			(3)
			#define MASK_SIGNALPOWER_IN_OPMODECODE			(1)
			//����� ���������������� ARM
			#define SHIFT_ARMPOWERMODE_IN_OPMODECODE		(4)
			#define MASK_ARMPOWERMODE_IN_OPMODECODE			(1)

			//------ ��� �������� ��������������� ---------
			//�������� ��������� ������
			#define SHIFT_OUTLEVEL_IN_AUDIOCODE					(0)
			#define MASK_OUTLEVEL_IN_AUDIOCODE					(7)
			//�������� ��������� �����
			#define SHIFT_INLEVEL_IN_AUDIOCODE					(3)
			#define MASK_INLEVEL_IN_AUDIOCODE						(7)
		
			//����� ������������� ����������
			#define OPMODE_MASK_IN_REQ						(1<<1)
			#define AUDIO_MASK_IN_REQ							(1<<2)
			#define TXFREQ_MASK_IN_REQ						(1<<3)
			#define RXFREQ_MASK_IN_REQ						(1<<4)
			#define RSSI_MASK_IN_REQ							(1<<5)
			#define CHANSTATE_MASK_IN_REQ					(1<<5)			
	} cmdReqParam;	

private:

	//������������ ������ ���� ���������, ����
	#define MAX_SIZE_OF_BODY     (128)
	//������ ���������, ����
	#define SIZE_OF_HEADER       (3)
	//������ ���� CRC, ����
	#define SIZE_OF_CRC          (1)

	//������������ ������ ����� ���������, ����
	#define MAX_SIZE_OF_MSG     (SIZE_OF_HEADER + MAX_SIZE_OF_BODY + SIZE_OF_CRC)
	//����������� ������ ����� ���������, ����
	#define MIN_SIZE_OF_MSG     (SIZE_OF_HEADER + SIZE_OF_CRC)

	struct structSPIMMsgHeader {
		uint8_t bodySize;               // ������ ����, ����
		uint8_t adress          :4;     // ������� ���������
		uint8_t noMsg          	:4;     // ������� ��������� (�� ������ 4)
		uint8_t IDCmd;                  // ������������� �������
	};
	
	uint8_t SPIMmsgData[MAX_SIZE_OF_MSG];
	uint8_t* SPIMHeaderData;
	uint8_t* SPIMbodyData;
	uint8_t SPIMbodySize;
	uint8_t* SPIMCRC;

	uint8_t CRC_Calc(uint8_t* pData, uint8_t sizeData);

};

#endif // SPIMMESSAGE_H

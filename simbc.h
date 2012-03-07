#ifndef _SIMBC_H
#define _SIMBC_H 
#include "reg.h"
#include "mem.h"
#include "simbus.h"

#define BC_STACK_A_ADDR 0
#define BC_STACK_A_LEN 0X0100
#define BC_STACK_POINTER_A_ADDR 0X0100
#define BC_MSG_CNT_A_ADDR 0X0101
#define BC_STACK_POINTER_A_INITIAL_ADDR 0X0102
#define BC_MSG_CNT_A_INITIAL_ADDR 0X0103

#define BC_STACK_B_ADDR 0X0F00
#define BC_STACK_B_LEN 0X0100
#define BC_STACK_POINTER_B_ADDR 0X0104
#define BC_MSG_CNT_B_ADDR 0X0105
#define BC_STACK_POINTER_B_INITIAL_ADDR 0X0106
#define BC_MSG_CNT_B_INITIAL_ADDR 0X0107

//Totally 94 blocks
#define BC_MSG_BLOCK_ADDR 0X0108
#define BC_MSG_BLOCK_PER_LEN 0X0026
#define BC_MSG_BLOCK_LEN 0X0DF4

#define BC_UNUSED_AREA_ADDR 0X0EFC
#define BC_UNUSED_AREA_LEN 0X0004

#define BC_ENHANCED_MODE (sim61580->configReg_3&0X8000)
#define BC_MSGRETRYING (sim61580->MsgRetryING)

enum msgtype{BC_RT,RT_RT,BCAST,RT_RT_BCAST,MODE_CODE,INVALID_TYPE,MODE_CODE_BCAST};



class SimBC:public Reg, public Mem
{
private:
	Bus m_busChannelA;
	Bus m_busChannelB;

	UINT16 m_bcCurrentMsgCyc;
	UINT16 m_bcCurrentMsgRetryCyc;

	UINT16 m_bcCurrentMsgCycCount;
	bool m_bcIsFirstForAreaA;
	UINT16 m_bcFrameMsgCountA;
	UINT16 m_bcFrameCountA;

	UINT16 m_bcFrameCountB;
	UINT16 m_bcFrameMsgCountB;
	bool m_bcIsFirstForAreaB;
	bool m_bcIsRetrying;
	UINT16 m_msgTimeGap;
	UINT16 m_msgBlockAddr;
	UINT16 m_msgCmdWord;
	UINT16 m_msgSecondCmdWord;

	
	UINT16 m_currentRetryCount;
	UINT16 m_bcCurrentMsgTimeConsuming;
	UINT16 m_bcCurrentIdleTime;
	UINT16 m_bcCurrentFrameTimeConsuming;

	

public:
	SimBC(void);
	SimBC(char *fileName);
	UINT32 OnData(UINT32, void *);
	virtual UINT16 memDump();
	virtual INT16 initRegAddress(void);
	void bcStep(void);
	UINT16 bcStratMsg(void);
	UINT16 bcEndMsg(void);
	UINT16 bcAnalyzeMsg(void);
	bool bcIsRightTimeToStartMSG();
	
	//Transfer function, the bcWordTransfer is the main transfer function, 
	//and it will check the type of the msg to invoke the suitable function 
	//to finish the transfer 
	UINT16 bcWordTransfer(void);
	UINT16 bcProcessStatusWord(UINT16,UINT16);

	UINT16 bcBCRTTransfer(void);
	UINT16 bcRTBCTransfer(void);

	UINT16 bcRTRTTransfer(void);
	UINT16 bcModeCodeNoDataTransfer(void);
	UINT16 bcModeCodeTXDataTransfer(void);
	
	UINT16 bcModeCodeRXDataTransfer(void);
	UINT16 bcBroadcastTransfer(void);
	UINT16 bcRTBroadcastTransfer(void);
	UINT16 bcModeCodeNoDataBroadcastTransfer(void);
	UINT16 bcModeCodeDataBroadcastTransfer(void);

	UINT16 bcRecvImproperWord();
	UINT16 bcRecvTimeout();

	UINT16 initForTest();

	void bcSetBus(UINT16 type,UINT16 time,UINT16 data,UINT16 isFull);


	

	
};

#endif
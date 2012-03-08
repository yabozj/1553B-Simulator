#ifndef _SIMRT_H
#define _SIMRT_H

#define REG_EXTERN extern
#include "reg.h"
#include "mem.h"
#include "simbus.h"
#include "transException.h"

#define RT_STACK_A_ADDR 0
#define RT_STACK_A_LEN 0X0100
#define RT_STACK_POINTER_A_ADDR 0X0100
#define RT_STACK_POINTER_B_ADDR 0X0104

//Definition for mode code selective interrupt
#define RT_MODE_CODE_SELECTIVE_INTERRUPT_TABLE_ADDR 0X0108
#define RT_MODE_CODE_SELECTIVE_INTERRUPT_TABLE_LEN 0X0008

#define RT_MODE_CODE_INTERRUPT_MODE_RX_LSB 0X0108
#define RT_MODE_CODE_INTERRUPT_MODE_RX_MSB 0X0109

#define RT_MODE_CODE_INTERRUPT_MODE_TX_LSB 0X010A
#define RT_MODE_CODE_INTERRUPT_MODE_TX_MSB 0X010B

#define RT_MODE_CODE_INTERRUPT_MODE_BCAST_RX_LSB 0X010C
#define RT_MODE_CODE_INTERRUPT_MODE_BCAST_RX_MSB 0X010D

#define RT_MODE_CODE_INTERRUPT_MODE_BCAST_TX_LSB 0X010E
#define RT_MODE_CODE_INTERRUPT_MODE_BCAST_TX_MSB 0X010F



//Definition for mode code
#define RT_MODE_CODE_DATA_ADDR 0X0110
#define RT_MODE_CODE_DATA_LEN 0X0030

#define RT_MODECODE_DATA_RECV_ADDR 0X0110
#define RT_MODECODE_DATA_RECV_LEN 0X0010

#define RT_MODECODE_DATA_TRANSMIT_ADDR 0X0120
#define RT_MODECODE_DATA_TRANSMIT_LEN 0X0010

#define RT_MODECODE_DATA_BCAST_ADDR 0X0130
#define RT_MODECODE_DATA_BCAST_LEN 0X0010


//Define lookup table A area
#define RT_LOOKUP_TABLE_A_ADDR 0X0140
#define RT_LOOKUP_TABLE_A_LEN 0X0080

#define RT_RX_LOOKUP_TABLE_A_ADDR 0X0140
#define RT_RX_LOOKUP_TABLE_A_LEN 0X0020

#define RT_TX_LOOKUP_TABLE_A_ADDR 0X0160
#define RT_TX_LOOKUP_TABLE_A_LEN 0X0020

#define RT_BCAST_LOOKUP_TABLE_A_ADDR 0X0180
#define RT_BCAST_LOOKUP_TABLE_A_LEN 0X0020

#define RT_CONTROLWORD_LOOKUP_TABLE_A_ADDR 0X01A0
#define RT_CONTROLWORD_LOOKUP_TABLE_A_LEN 0X0020


//Define lookup table B area
#define RT_LOOKUP_TABLE_B_ADDR 0X01C0
#define RT_LOOKUP_TABLE_B_LEN 0X0080

#define RT_RX_LOOKUP_TABLE_B_ADDR 0X01C0
#define RT_RX_LOOKUP_TABLE_B_LEN 0X0020

#define RT_TX_LOOKUP_TABLE_B_ADDR 0X01E0
#define RT_TX_LOOKUP_TABLE_B_LEN 0X0020

#define RT_BCAST_LOOKUP_TABLE_B_ADDR 0X0200
#define RT_BCAST_LOOKUP_TABLE_B_LEN 0X0020

#define RT_CONTROLWORD_LOOKUP_TABLE_B_ADDR 0X0220
#define RT_CONTROLWORD_LOOKUP_TABLE_B_LEN 0X0020


#define RT_BUSY_BIT_LOOKUP_TABLE_ADDR 0X0240
#define RT_BUSY_BIT_LOOKUP_TABLE_LEN 0X0008

#define RT_UNUSED_AREA_ADDR 0X0248
#define RT_UNUSED_AREA_LEN 0X0018


#define RT_MSG_BLOCK_ADDR_0_4_ENHANCE_ADDR 0X0260
#define RT_MSG_BLOCK_PER_LEN 0X0020
#define RT_MSG_BLOCK_ADDR_0_4_ENHANCE_LEN 0X00A0

#define RT_MSG_BLOCK_ADDR_5_100_ENHANCE_ADDR 0X0400
#define RT_MSG_BLOCK_ADDR_5_100_ENHANCE_LEN 0X0C00

//For none enhancement mode only
#define RT_MSG_BLOCK_ADDR_0_5_ADDR 0X0240
#define RT_MSG_BLOCK_ADDR_0_5_LEN 0X00C0

#define RT_MSG_BLOCK_ADDR_6_93_ADDR 0X0400
#define RT_MSG_BLOCK_ADDR_6_93_LEN 0X0B00

#define RT_COMMAND_ILLEGALIZAION_TABLE_ADDR 0X0300
#define RT_COMMAND_ILLEGALIZAION_TABLE_LEN 0X0100

//For none enhancement mode only
#define RT_STACK_B_ADDR 0X0F00
#define RT_STACK_B_LEN 0X0100


#define RT_ENHANCED_MODE (sim61580->configReg_3&0X8000)

#define RT_ENHANCED_RT_MEM_MANAGEMENT (sim61580->configReg_2&0X0001)
#define RT_ENHANCED_MODE_CODE_HANDLING ((ENHANCED_RT_MEM_MANAGEMENT&&(sim61580->configReg_3&0X0001)))

enum{vectorWordModeCode=16,lastCmdWordModeCode=18,BITWordModeCode=19};


class SimRT:public Reg, public Mem, public Exception1553B
{
private:
	SimRT *m_rtBackUp;
	Bus m_busChannelA;
	Bus m_busChannelB;


	
	UINT16 m_rtDataIndex;
	UINT16 m_rtCurrentMsgCycCount;

	UINT16 m_rtCurrentMsgCyc;
	

	
	UINT16 m_rtCurrentMsgCmdWord;
	UINT16 m_rtMsgBlockAddr;
	UINT16 m_rtAddress;
	UINT16 m_mLastStatusReg;	

	UINT16 m_rtProcessedMsgCount;
	char rtDescription[32];
	

public:
	SimRT(UINT16 addr);
	SimRT(UINT16 addr,char *fileName);
	~SimRT();
	void rtRestore();
	void rtSave();
	UINT16 rtDump(int len, void * buffAddr);
	UINT16 rtAddress()
	{
		return m_rtAddress;
	}
	virtual UINT16 memDump();
	virtual INT16 initRegAddress(void);
	virtual void genIRQ(void);
	UINT16 RTStep(void);
	UINT16 RTReceiveCMD();	
	UINT16 RTSingleWordTransfer();
	
	UINT16 RTModeCodeWithDataTransferTX();
	UINT16 RTModeCodeWithDataTransferRX();
	UINT16 RTModeCodeWithOutDataTransfer();

	UINT16 RTTXTransfer();
	UINT16 RTRXTransfer();
	

	UINT16 RTStartMsg();
	UINT16 RTReturnStatusWord();
	UINT16 RTEndMsg();
	UINT16 RTCheckCMDType(UINT16 cmd);
	UINT16 RTLookupIllegalizationTable(UINT16 cmd);
	UINT16 RTLookupBusyTable(UINT16 cmd);
	
	UINT16 RTLoadSubaddressControlWordAndDataAddress(UINT16 cmd);
	UINT16 RTUpdateLookupTable(UINT16 cmd);

	UINT32 OnData(UINT32, void *);
	
	UINT16 rtRecvImproperWord(UINT16 actualType,UINT16 expectType,UINT16 data);
	UINT16 rtRecvTimeout();

	UINT16 initForTest();
	UINT16 configReg_5_write(UINT16 data);
	struct TransException checkIfException();
	UINT32 CheckRecvHook(UINT32 len,void *recvData);
};


#endif



#ifndef _REG_H
#define _REG_H

#include "definition.h"


#define INT_MASK_REG_WR_ADDR 0x00
#define CONFIG_REG_1_WR_ADDR 0x01
#define CONFIG_REG_2_WR_ADDR 0x02

#define START_RESET_REG_W_ADDR 0x03//write only
#define CMD_STACK_PTR_REG_R_ADDR 0x03//read only

#define BC_CTRL_WORD_REG_WR_ADDR 0x04
#define RT_SUBADDR_CTRL_WORD_REG_WR_ADDR 0x04

#define TIME_TAG_REG_WR_ADDR 0x05
#define INT_STATUS_REG_R_ADDR 0x06//read
#define CONFIG_REG_3_WR_ADDR 0x07
#define CONFIG_REG_4_WR_ADDR 0x08
#define CONFIG_REG_5_WR_ADDR 0x09

#define RT_DATA_STACK_ADDR_REG_WR_ADDR 0X0A//RT
#define MT_DATA_STACK_ADDR_REG_WR_ADDR 0X0A//MT 

#define BC_FRAME_TIME_REMAIN_REG_R_ADDR 0x0B
#define BC_MSG_TIME_REMAIN_REG_R_ADDR 0x0C

#define BC_FRAME_TIME_REG_WR_ADDR 0x0D
#define RT_LAST_CMD_REG_WR_ADDR 0x0D
#define MT_TRIGGER_WORD_REG_WR_ADDR 0x0D

#define RT_STATUS_REG_R_ADDR 0x0E
#define RT_BIT_WORD_REG_R_ADDR 0x0F

class Reg
{
protected:
	UINT16 intMaskReg;
	UINT16 configReg_1;
	UINT16 configReg_2;
	UINT16 cmdStackPtrReg;
	UINT16 startResetReg;
	UINT16 bcCtrlWordReg;
	UINT16 rtSubAddrCtrlWordReg;
	UINT16 timeTagReg;
	UINT16 intStatusReg;
	UINT16 configReg_3;
	UINT16 configReg_4;
	UINT16 configReg_5;
	UINT16 rtDataStackAddrReg;
	UINT16 mtDataStackAddrReg;
	UINT16 bcFrameTimeRemainReg;
	UINT16 bcMsgTimeRemainReg;
	UINT16 bcFrameTimeReg;
	UINT16 rtLastCmdReg;
	UINT16 mtTriggerWordReg;
	UINT16 rtStatusWordReg;
	UINT16 rtBITWordReg;

	UINT64 timeRecorder;

	bool isHalted;
	typedef UINT16 (Reg:: *decodefun_w)(UINT16);
	typedef UINT16 (Reg:: *decodefun_r)(void);
	decodefun_r regRead[16];
	decodefun_w regWrite[16];
	UINT16 updateTimeReg();

public:
	UINT16 clearRegs(bool isInit);
	UINT16 regReadFromAddr(UINT16);
	UINT16 regWriteToAddr(UINT16,UINT16);

	UINT16 intMaskReg_read(void);
	UINT16 intMaskReg_write(UINT16);

	
	UINT16 configReg_1_read(void);
	UINT16 configReg_1_write(UINT16);

	
	UINT16 configReg_2_read(void);
	UINT16 configReg_2_write(UINT16);

	 
	UINT16 cmdStackPtrReg_read(void);
	
	UINT16 startResetReg_write(UINT16);

 
	UINT16 bcCtrlWordReg_read(void);
	UINT16 bcCtrlWordReg_write(UINT16);
	
	UINT16 rtSubAddrCtrlWordReg_read(void);
	UINT16 rtSubAddrCtrlWordReg_write(UINT16);

	 
	UINT16 timeTagReg_read(void);
	UINT16 timeTagReg_write(UINT16);

	
	UINT16 intStatusReg_read(void);

	
	UINT16 configReg_3_read(void);
	UINT16 configReg_3_write(UINT16);

	
	UINT16 configReg_4_read(void);
	UINT16 configReg_4_write(UINT16);

	
	UINT16 configReg_5_read(void);
	UINT16 configReg_5_write(UINT16);

	
	UINT16 rtDataStackAddrReg_read(void);
	UINT16 rtDataStackAddrReg_write(UINT16);
	
	UINT16 mtDataStackAddrReg_read(void);
	UINT16 mtDataStackAddrReg_write(UINT16);

	
	UINT16 bcFrameTimeRemainReg_read(void);

	
	UINT16 bcMsgTimeRemainReg_read(void);

	
	UINT16 bcFrameTimeReg_read(void);
	UINT16 bcFrameTimeReg_write(UINT16);
	
	UINT16 rtLastCmdReg_read(void);
	UINT16 rtLastCmdReg_write(UINT16);

	UINT16 mtTriggerWordReg_read(void);
	UINT16 mtTriggerWordReg_write(UINT16);

	
	UINT16 rtStatusWordReg_read(void);

	
	UINT16 rtBITWordReg_read(void);

	virtual INT16 initRegAddress(void) = 0;
};











#endif
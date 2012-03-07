#include "stdafx.h"
#include "simmt.h"
extern SimMT *mt;

SimMT::SimMT(void)
{
	llogDebug("MT","Init MT");
	initRegAddress();
	clearRegs(true);
	
	
	
}
SimMT::SimMT(char *fileName)
{
	loadConfiguration(fileName,true);
	//SimBC::SimBC(*bc);
	*this = *mt;
	delete mt;
	mt = NULL;
}
UINT16 SimMT::memDump()
{
	return 0;
}

INT16 SimMT::initRegAddress(void)
{
	//Need to be optimized
	regRead[INT_MASK_REG_WR_ADDR] = &Reg::intMaskReg_read;
	regRead[CONFIG_REG_1_WR_ADDR] = &Reg::configReg_1_read;
	regRead[CONFIG_REG_2_WR_ADDR] = &Reg::configReg_2_read;
	regRead[CMD_STACK_PTR_REG_R_ADDR] = &Reg::cmdStackPtrReg_read;
	regRead[4] = NULL;
	regRead[TIME_TAG_REG_WR_ADDR] = &Reg::timeTagReg_read;
	regRead[INT_STATUS_REG_R_ADDR] = &Reg::intStatusReg_read;
	regRead[CONFIG_REG_3_WR_ADDR] = &Reg::configReg_3_read;
	regRead[CONFIG_REG_4_WR_ADDR] = &Reg::configReg_4_read;
	regRead[CONFIG_REG_5_WR_ADDR] = &Reg::configReg_5_read;
	regRead[MT_DATA_STACK_ADDR_REG_WR_ADDR] = &Reg::mtDataStackAddrReg_read;
	regRead[11] = NULL;
	regRead[12] = NULL;
	regRead[MT_TRIGGER_WORD_REG_WR_ADDR] = &Reg::mtTriggerWordReg_read;
	regRead[14] = NULL;
	regRead[15] = NULL;

	regWrite[INT_MASK_REG_WR_ADDR] = &Reg::intMaskReg_write;
	regWrite[CONFIG_REG_1_WR_ADDR] = &Reg::configReg_1_write;
	regWrite[CONFIG_REG_2_WR_ADDR] = &Reg::configReg_2_write;
	regWrite[START_RESET_REG_W_ADDR] = &Reg::startResetReg_write;
	regWrite[4] = NULL;
	regWrite[TIME_TAG_REG_WR_ADDR] = &Reg::timeTagReg_write;
	regWrite[6] = NULL;
	regWrite[CONFIG_REG_3_WR_ADDR] = &Reg::configReg_3_write;
	regWrite[CONFIG_REG_4_WR_ADDR] = &Reg::configReg_4_write;
	regWrite[CONFIG_REG_5_WR_ADDR] = &Reg::configReg_5_write;
	regWrite[MT_DATA_STACK_ADDR_REG_WR_ADDR] = &Reg::mtDataStackAddrReg_write;
	regWrite[11] = NULL;
	regWrite[12] = NULL;
	regWrite[MT_TRIGGER_WORD_REG_WR_ADDR] = &Reg::mtTriggerWordReg_write;
	regWrite[14] = NULL;
	regWrite[15] = NULL;

	return 0;
}

void SimMT::mtStep(void) 
{
	
}
UINT32 SimMT::OnData(UINT32, void *)
{
	return 0;
}
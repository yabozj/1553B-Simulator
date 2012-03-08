#include "stdafx.h"
#include "simmt.h"
#include "wordParser.h"
extern SimMT *mt;

SimMT::SimMT(void)
{
	llogDebug("MT","Init MT");
	initRegAddress();
	clearRegs(true);
	
	m_mtBackUp = NULL;
	m_pointer = 0;
}
SimMT::SimMT(char *fileName)
{
	loadConfiguration(fileName,true);
	//SimBC::SimBC(*bc);
	*this = *mt;
	delete mt;
	mt = NULL;
	m_mtBackUp = NULL;
	m_pointer = 0;
}

SimMT::~SimMT()
{
	if(m_mtBackUp) 
	{
		m_mtBackUp->m_mtBackUp = NULL;
		delete m_mtBackUp;
		m_mtBackUp = NULL;
	}
}

void SimMT::mtRestore()
{
	if (m_mtBackUp)
	{
		*this = *m_mtBackUp;
	}
}

void SimMT::mtSave()
{
	if (m_mtBackUp)
	{
		*m_mtBackUp = *this;
	}
	else {
		m_mtBackUp = new SimMT;
		*m_mtBackUp = *this;
	}

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
	UINT16 busData[4];
	UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
	//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
	if (!retValue)
	{
		//wordParse("MT  ",busData[0],busData[2],busData[1]);
		
		
		UINT16 idWord = 0;
		UINT16 word = busData[0];
		if (busData[0] == DATA_TYPE_STATUS_WORD)
		{
			UINT16 isError = busData[0] & 0x0400;
			if(isError) 
			{
				idWord = 0x00B1;
			}
			else
			{
				idWord = 0x00A1;
			}
			
		}
		else if (busData[0] == DATA_TYPE_COMMAND_WORD)
		{
			UINT16 subAddress=(word>>5)&0X001F;
			UINT16 t_r=(word&0X0400) >> 9;
			UINT16 address=word>>11;
			UINT16 wordCount_modeCode = word&0X001F;
			bool isModeCode = subAddress == 0 || subAddress == 31;
			idWord = 0x0088;
			if (address != 0x31)
			{
				idWord = 0x00A8;
			}
			if(!isModeCode) 
			{
				idWord |= 0x0001;
			}
		}
		else if (busData[0] == DATA_TYPE_DATA_WORD)
		{
			idWord = 0x00A1;
		}
		else {
			return;
		}
		memWrite((m_pointer%MEMSIZE),busData[1]);
		memWrite(((m_pointer ++ )%MEMSIZE),busData[1]);
	}
	
}
UINT32 SimMT::OnData(UINT32, void *)
{
	return 0;
}
UINT16 SimMT::mtDump(int len, void *buffAddr)
{
	if (sizeof(bu61580_sharemem_struct) > len)
	{
		llogWarn("BCDump","Needs larger buffer size");
		return 1;
	}
	bu61580_sharemem_struct *buffStruct = (bu61580_sharemem_struct*)buffAddr;
	for(int i = 0; i <= 0xf; i++)
	{
		if (Reg::regRead[i])
		{
			buffStruct->reg[i] = (this->*Reg::regRead[i])();
		}
		else
		{
			buffStruct->reg[i] = 0;
		}

	}
	for (int i = 0; i<= 0xfff; i++)
	{
		buffStruct->mem[i] = Mem::mem[i];
	}
	return 0;
}

struct TransException SimMT::checkIfException()
{
	struct TransException transExcep = {0,0,TimeOutException,FALSE};

	return transExcep;
}

UINT32 SimMT::CheckRecvHook(UINT32 len,void *recvData)
{
	return ::CheckRecv(len,recvData);
}


void SimMT::genIRQ(void)
{

}
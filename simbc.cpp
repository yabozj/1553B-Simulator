#include "stdafx.h"
#include "simbc.h"
#include "wordParser.h"
extern SimBC *bc;
SimBC::SimBC(void)
{
	llogDebug("BC","Init BC");
	initRegAddress();
	clearRegs(true);
	//initForTest();
	
	m_bcFrameMsgCountA = 0;
	
	m_bcFrameMsgCountB = 0;
	m_busChannelA.m_busFull = 0;
	m_busChannelB.m_busFull = 0;
	Reg::timeRecorder = 0;
	m_bcCurrentFrameTimeConsuming = 0;
	m_bcCurrentMsgTimeConsuming = 0;
	m_bcCurrentIdleTime = 0;
	m_bcIsFirstForAreaB = true;
	m_bcIsFirstForAreaA = true;
	
	m_bcIsRetrying = false;
	m_bcCurrentMsgCyc = 0;
	m_bcCurrentMsgRetryCyc = 0;
	m_bcCurrentMsgCycCount = 0;
	m_currentRetryCount = 0;
	m_bcFrameCountA = 0;
	m_bcFrameCountB = 0;
	m_bcProcessedMsgCount = 0;
	for(int i = 0; i < MEMSIZE; i++)
	{
		Mem::mem[i] = 0;
	}
	m_bcBackUp = NULL;
}
SimBC::SimBC(char *fileName)
{
	loadConfiguration(fileName,true);
	//SimBC::SimBC(*bc);
	*this = *bc;
	delete bc;
	bc = NULL;
	m_bcBackUp = NULL;
}

SimBC::~SimBC()
{
	if(m_bcBackUp) 
	{
		m_bcBackUp->m_bcBackUp = NULL;
		delete m_bcBackUp;
		m_bcBackUp = NULL;
	}
}

void SimBC::bcRestore()
{
	if (m_bcBackUp)
	{
		*this = *m_bcBackUp;
	}
	
}

void SimBC::bcSave()
{
	if (m_bcBackUp)
	{
		*m_bcBackUp = *this;
	}
	else {
		m_bcBackUp = new SimBC;
		*m_bcBackUp = *this;
	}
}

INT16 SimBC::initRegAddress(void)
{
	//Need to be optimized
	//Init the read reg
	Reg::regRead[INT_MASK_REG_WR_ADDR] = &Reg::intMaskReg_read;
	Reg::regRead[CONFIG_REG_1_WR_ADDR] = &Reg::configReg_1_read;
	Reg::regRead[CONFIG_REG_2_WR_ADDR] = &Reg::configReg_2_read;
	Reg::regRead[CMD_STACK_PTR_REG_R_ADDR] = &Reg::cmdStackPtrReg_read;
	Reg::regRead[BC_CTRL_WORD_REG_WR_ADDR] = &Reg::bcCtrlWordReg_read;
	Reg::regRead[TIME_TAG_REG_WR_ADDR] = &Reg::timeTagReg_read;
	Reg::regRead[INT_STATUS_REG_R_ADDR] = &Reg::intStatusReg_read;
	Reg::regRead[CONFIG_REG_3_WR_ADDR] = &Reg::configReg_3_read;
	Reg::regRead[CONFIG_REG_4_WR_ADDR] = &Reg::configReg_4_read;
	Reg::regRead[CONFIG_REG_5_WR_ADDR] = &Reg::configReg_5_read;
	Reg::regRead[10] = NULL;
	Reg::regRead[BC_FRAME_TIME_REMAIN_REG_R_ADDR] = &Reg::bcFrameTimeRemainReg_read;
	Reg::regRead[BC_MSG_TIME_REMAIN_REG_R_ADDR] = &Reg::bcMsgTimeRemainReg_read;
	Reg::regRead[BC_FRAME_TIME_REG_WR_ADDR] = &Reg::bcFrameTimeReg_read;
	Reg::regRead[14] = NULL;
	Reg::regRead[15] = NULL;

	//Init the write reg
	Reg::regWrite[INT_MASK_REG_WR_ADDR] = &Reg::intMaskReg_write;
	Reg::regWrite[CONFIG_REG_1_WR_ADDR] = &Reg::configReg_1_write;
	Reg::regWrite[CONFIG_REG_2_WR_ADDR] = &Reg::configReg_2_write;
	Reg::regWrite[START_RESET_REG_W_ADDR] = &Reg::startResetReg_write;
	Reg::regWrite[BC_CTRL_WORD_REG_WR_ADDR] = &Reg::bcCtrlWordReg_write;
	Reg::regWrite[TIME_TAG_REG_WR_ADDR] = &Reg::timeTagReg_write;
	Reg::regWrite[6] = NULL;
	Reg::regWrite[CONFIG_REG_3_WR_ADDR] = &Reg::configReg_3_write;
	Reg::regWrite[CONFIG_REG_4_WR_ADDR] = &Reg::configReg_4_write;
	Reg::regWrite[CONFIG_REG_5_WR_ADDR] = &Reg::configReg_5_write;
	Reg::regWrite[10] = NULL;
	Reg::regWrite[11] = NULL;
	Reg::regWrite[12] = NULL;
	Reg::regWrite[BC_FRAME_TIME_REG_WR_ADDR] = &Reg::bcFrameTimeReg_write;
	Reg::regWrite[14] = NULL;
	Reg::regWrite[15] = NULL;

	return 0;
}

UINT16 SimBC::memWrite(UINT16 addr, UINT16 data) 
{
	

	UINT16 returnVal = Mem::memWrite(addr,data);
	if(Reg::configReg_1 & 0x2000) 
	{
		if(addr == BC_MSG_CNT_B_ADDR)
			llogInfo("BC MsgB","Msg Count : %u",0xffff - data);
	}
	else 
	{
		if(addr == BC_MSG_CNT_A_ADDR)
		llogInfo("BC MsgA","Msg Count : %u",0xffff - data);
	}
	return returnVal;
}

UINT16 SimBC::bcDump(int len, void * buffAddr)
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

UINT32 SimBC::OnData(UINT32 len, void * data) 
{
	//memDump();
	if(len < (4 + 32) * sizeof(UINT16)) {
		return 2;//Insufficient Buffer size
	}
	if (Reg::bcCtrlWordReg & 0X0080) 
	{ 
		if(!m_bcIsRetrying)
		{
			
			return m_busChannelB.OnData(len, data);
		}
		else if(m_currentRetryCount == 1)
		{
			if(Reg::configReg_4 & 0x0100)
			{
				return m_busChannelA.OnData(len, data);
			}
			else
			{
				return m_busChannelB.OnData(len, data);
			}
		}
		else if(m_currentRetryCount == 2)
		{
			if(Reg::configReg_4 & 0x0080)
			{
				return m_busChannelA.OnData(len, data);
			}
			else
			{
				return m_busChannelB.OnData(len, data);
			}
		}

	}
	else 
	{
		return m_busChannelA.OnData(len, data);
	}
	return 0;
	
}
UINT16 SimBC::memDump()
{
	UINT16 msgCount = 0;
	if(Reg::configReg_1 & 0x2000) 
	{
		
		msgCount = memRead(BC_MSG_CNT_B_ADDR);
		
	}
	else 
	{
		
		msgCount = memRead(BC_MSG_CNT_A_ADDR);
	}
	llogInfo("Mem Dump","Msg count:0x%X",msgCount);
	return 0;
}
void SimBC::bcStep(void)
{
	if (!Reg::isHalted) 
	{
		//llogDebug("BC","Step");
		//Init a msg transfer
		if ((m_bcIsRetrying && !m_bcCurrentMsgRetryCyc) || !m_bcCurrentMsgCyc) 
		{
			
			if (m_bcIsRetrying || bcIsRightTimeToStartMSG())
			{
				if (!m_bcIsRetrying)
				{
					
					bcStratMsg();
				}
				bcWordTransfer();// After execute this function,m_bcCurrentMsgCycCount will be the right value, and m_bcCurrentMsgCyc will be 1.
			}
		}
		else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc) || m_bcCurrentMsgCyc) //If BC has message to transfer, the m_bcCurrentMsgCyc will greater than 0
		{
			bcWordTransfer();
			if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc && m_bcCurrentMsgRetryCyc == m_bcCurrentMsgCycCount) || (m_bcCurrentMsgCyc && m_bcCurrentMsgCyc == m_bcCurrentMsgCycCount)) 
			{
				bcEndMsg();
				
			}
		}
		else //It is not the right time to transfer data
		{
			
		}
		
	}
	else //BC is halted
	{
		
	}
}

bool SimBC::bcIsRightTimeToStartMSG()
{
	m_bcIsRetrying = 0;
	m_currentRetryCount = 0;
	m_bcCurrentMsgRetryCyc = 0;
	if(Reg::configReg_1 & 0x2000) 
	{
		//Begin to send a frame, initialize the variables
		if (!m_bcFrameMsgCountB || (m_bcFrameMsgCountB == 0xFFFF && (Reg::configReg_3 & 0x8000) && (Reg::configReg_1 & 0x0100))) //The first init or auto repeat 
		{
			//Enhance mode and auto repeat frame
			if ((Reg::configReg_3 & 0x8000) && (Reg::configReg_1 & 0x0100) ) 
			{
				UINT16 initStackAddrForB = memRead(BC_STACK_POINTER_B_INITIAL_ADDR);
				UINT16 initMsgCountForB = memRead(BC_MSG_CNT_B_INITIAL_ADDR);
				memWrite(BC_STACK_POINTER_B_ADDR,initStackAddrForB);
				memWrite(BC_MSG_CNT_B_ADDR,initMsgCountForB);
			}
			m_bcIsFirstForAreaB = true;
			
			//Init the msg count, and the msg sequence
			
			m_bcFrameMsgCountB = memRead(BC_MSG_CNT_B_ADDR);
		}
		//Msg count is zero
		UINT16 msgCountB = memRead(BC_MSG_CNT_B_ADDR);
		if (0xFFFF == msgCountB|| !msgCountB) 
		{
			return false;
		}
		//Read the msg block pointer
	
	}
	else 
	{
		if (!m_bcFrameMsgCountA || (m_bcFrameMsgCountB == 0xFFFF && (Reg::configReg_3 & 0x8000) && (Reg::configReg_1 & 0x0100))) 
		{
			//Enhance mode and auto repeat frame
			if ((Reg::configReg_3 & 0x8000) && (Reg::configReg_1 & 0x0100) ) 
			{
				UINT16 initStackAddrForA = memRead(BC_STACK_POINTER_A_INITIAL_ADDR);
				UINT16 initMsgCountForA = memRead(BC_MSG_CNT_A_INITIAL_ADDR);
				memWrite(BC_STACK_POINTER_A_ADDR,initStackAddrForA);
				memWrite(BC_MSG_CNT_A_ADDR,initMsgCountForA);
			}
			//Init the msg count, and the msg sequence
			m_bcIsFirstForAreaA = true;
			
			m_bcFrameMsgCountA = memRead(BC_MSG_CNT_A_ADDR);
		}
		UINT16 msgCountA = memRead(BC_MSG_CNT_A_ADDR);
		if (0xFFFF == msgCountA || !msgCountA) 
		{
			return false;
		}
		

	}
	
	if ((Reg::configReg_3 & 0X8000) && (Reg::configReg_1 & 0X0010))//Check msg gap
	{
		if(Reg::configReg_1 & 0x2000) 
		{
			if(m_bcIsFirstForAreaB && !m_bcFrameCountB)
			{
				m_bcIsFirstForAreaB = false;
				m_currentRetryCount = 0;
				m_bcFrameCountB ++;
				m_bcCurrentFrameTimeConsuming = 0;
				m_bcCurrentMsgTimeConsuming = 0;
				return true;
			}
			
		}
		else
		{
			if(m_bcIsFirstForAreaA && !m_bcFrameCountA)
			{
				m_bcIsFirstForAreaA = false;
				m_currentRetryCount = 0;
				m_bcFrameCountA ++;
				m_bcCurrentFrameTimeConsuming = 0;
				m_bcCurrentMsgTimeConsuming = 0;
				return true;
			}
		}
		if((Reg::configReg_3 & 0x8000) && (Reg::startResetReg & 0x0100))//Auto repeat frame
		{
			if((m_bcIsFirstForAreaA && m_bcFrameCountA)||(m_bcIsFirstForAreaB && m_bcFrameCountB)) 
			{
				if(Reg::bcFrameTimeReg * 100 <= m_bcCurrentFrameTimeConsuming + m_bcCurrentIdleTime + 20)//Check if frame time is ok
				{
					INT32 timeElapsed = Reg::bcFrameTimeReg * 100 - m_bcCurrentFrameTimeConsuming - m_bcCurrentIdleTime; 
					if(timeElapsed)
					{
						Reg::timeRecorder += timeElapsed;
						m_bcCurrentIdleTime += timeElapsed;
						updateTimeReg();
					}
					return true;
				}
				else
				{
					m_bcCurrentIdleTime += 20;
					Reg::timeRecorder += 20;
					updateTimeReg();
					return false;
				}
			}
			
		}
		
		UINT16 timeGap = memRead(Reg::cmdStackPtrReg + 2);
		
		if (timeGap <= m_bcCurrentMsgTimeConsuming + m_bcCurrentIdleTime + 20)
		{
			INT32 timeElapsed = timeGap - m_bcCurrentMsgTimeConsuming - m_bcCurrentIdleTime; 
			if (timeElapsed)
			{
				Reg::timeRecorder += timeElapsed;
				m_bcCurrentIdleTime += timeElapsed;
				updateTimeReg();
			}
			m_bcCurrentMsgTimeConsuming = 0;
			m_currentRetryCount = 0;
			return true;
		}
		else
		{
			m_bcCurrentIdleTime += 20;
			Reg::timeRecorder += 20;
			updateTimeReg();
			return false;
		}
	}
	m_currentRetryCount = 0;
	return true;
}

UINT16 SimBC::bcStratMsg(void) 
{
	llogInfo("MSG Begin       ","=============================================");
	genIRQ();
	m_currentRetryCount = 0;
	m_bcCurrentMsgTimeConsuming = 0;
	m_bcCurrentIdleTime = 0;
	//Current area is B
	if(Reg::configReg_1 & 0x2000) 
	{	
		//Read the msg block pointer, and write back to the memory after decreasing the msg count
		Reg::cmdStackPtrReg = memRead(BC_STACK_POINTER_B_ADDR);
	}
	else 
	{
		//Read the msg block pointer, and write back to the memory after decreasing the msg count
		Reg::cmdStackPtrReg = memRead(BC_STACK_POINTER_A_ADDR);
	}
	bcAnalyzeMsg();
	return 0;
}

UINT16 SimBC::bcEndMsg(void)
{
	m_bcProcessedMsgCount ++;
	//Current active area is b
	if (Reg::configReg_1 & 0x2000) 
	{
		memWrite(BC_STACK_POINTER_B_ADDR,Reg::cmdStackPtrReg + 4);
		UINT16 msgCountForB = memRead(BC_MSG_CNT_B_ADDR);
		memWrite(BC_MSG_CNT_B_ADDR, msgCountForB + 1);
		m_bcFrameMsgCountB ++;

	}
	else 
	{
		memWrite(BC_STACK_POINTER_A_ADDR,Reg::cmdStackPtrReg + 4);
		UINT16 msgCountForA = memRead(BC_MSG_CNT_A_ADDR);
		memWrite(BC_MSG_CNT_A_ADDR, msgCountForA + 1);
		m_bcFrameMsgCountA ++;
	}
	m_bcCurrentMsgCyc = 0;
	m_bcCurrentMsgCycCount = 0;
	m_bcIsRetrying = 0;
	m_bcCurrentMsgRetryCyc = 0;
	m_currentRetryCount = 0;
	if(Reg::startResetReg & 0x0040)
	{
		Reg::isHalted = true;
	}
	if((Reg::startResetReg & 0x0020) && 0)
	{

	}
	genIRQ();
	llogInfo("MSG End         ","=============================================\n");
	return 0;
}



UINT16 SimBC::bcAnalyzeMsg(void)
{
	UINT16 blockStatusWord = memRead(Reg::cmdStackPtrReg);
	//Not end of message
	blockStatusWord &= 0x7fff;
	//Start of message
	blockStatusWord |= 0x4000;
	//Init the cyc count
	m_bcCurrentMsgCyc = 0; 
	m_msgBlockAddr = memRead(Reg::cmdStackPtrReg + 3);	
	UINT16 blockCtrlWord = memRead(m_msgBlockAddr);
	Reg::bcCtrlWordReg = blockCtrlWord;
	
	UINT16 timeRegValue = Reg::timeTagReg;
	memWrite(Reg::cmdStackPtrReg + 1, timeRegValue);
	memWrite(Reg::cmdStackPtrReg, blockStatusWord);

	m_msgCmdWord = memRead(m_msgBlockAddr + 1);


	UINT16 address=m_msgCmdWord>>11;
	UINT16 wordCount_modeCode=m_msgCmdWord&0X001F;
	UINT16 t_r=m_msgCmdWord&0X0400;
	UINT16 subAddress=(m_msgCmdWord>>5)&0X001F;
	if (subAddress == 0 || subAddress == 31)
	{
		
		blockCtrlWord &= 0xFFF8;
		blockCtrlWord |= 0x0004;
		if (address == 31 )
		{
			blockCtrlWord |= 0x0002;
			
		}
		
	}
	else if (address == 31)
	{
		if (!t_r)
		{
			blockCtrlWord &= 0xFFF8;
			blockCtrlWord |= 0x0002;
		}
		
			
	}
	memWrite(m_msgBlockAddr,blockCtrlWord);
	Reg::bcCtrlWordReg = blockCtrlWord;


	//Analyze the msg type
	switch(blockCtrlWord & 0X0007) 
	{
		case BC_RT: 
			{
				//At least, it should be 2;
				m_bcCurrentMsgCycCount = 2;
				
				if (m_msgCmdWord & 0X0001F) 
				{
					m_bcCurrentMsgCycCount += (m_msgCmdWord & 0x0001f);//Count of data
				}
				else 
				{
					m_bcCurrentMsgCycCount += 32;
				}
				//for loop back last data word and wait cyc
				m_bcCurrentMsgCycCount += 2;

				break;
			}
		case RT_RT: 
			{
				m_bcCurrentMsgCycCount = 4;
				
				m_msgSecondCmdWord = memRead(m_msgBlockAddr + 2);
				if (m_msgCmdWord & 0X0001F) 
				{
					m_bcCurrentMsgCycCount += (m_msgCmdWord & 0x0001f);//Count of data
				}
				else 
				{
					m_bcCurrentMsgCycCount += 32;
				}
				//For loop back transmit command word and wait status word
				m_bcCurrentMsgCycCount += 2;
				
				break;
			}
		case BCAST: 
			{		
				m_bcCurrentMsgCycCount = 1;
				
				if (m_msgCmdWord & 0X0001F) 
				{
					m_bcCurrentMsgCycCount += (m_msgCmdWord & 0x0001f);//Count of data
				}
				else 
				{
					m_bcCurrentMsgCycCount += 32;
				}
				//For loop back last data word
				m_bcCurrentMsgCycCount += 1;
				
				break;
			}
		case RT_RT_BCAST: 
			{
				m_bcCurrentMsgCycCount  = 3;
				
				m_msgSecondCmdWord = memRead(m_msgBlockAddr + 2);
				if (m_msgCmdWord & 0X0001F) 
				{
					m_bcCurrentMsgCycCount += (m_msgCmdWord & 0x0001f);//Count of data
				}
				else 
				{
					m_bcCurrentMsgCycCount += 32;
				}
				//Loop back for transmit command word and wait status word
				m_bcCurrentMsgCycCount += 2;
				
				break;
			}
		case MODE_CODE: 
			{	
				
				//Loop back ans wait status word
				m_bcCurrentMsgCycCount = 2;
				//Has a data word
				if ((m_msgCmdWord & 0x001f) >= 16) 
				{
					m_bcCurrentMsgCycCount += 3;
				} 
				else 
				{
					m_bcCurrentMsgCycCount += 2;
				}
			
				break;
			}
		case MODE_CODE_BCAST: 
			{
				
				//Loop back
				m_bcCurrentMsgCycCount = 1;
				if ((m_msgCmdWord & 0x001f) >= 16) 
				{
					m_bcCurrentMsgCycCount += 2;
				} 
				else 
				{
					m_bcCurrentMsgCycCount += 1;
				}
			
				break;
			}
		default:
			break; 
	}	
	return 0;
}

UINT16 SimBC::bcWordTransfer(void)
{
	UINT16 blockCtrlWord = Reg::bcCtrlWordReg;
	switch(blockCtrlWord & 0X0007) 
	{
		case BC_RT: 
			{
				//Transmit 
				if (m_msgCmdWord & 0x0400) 
				{
					bcRTBCTransfer();
				}
				else {
					bcBCRTTransfer();
				}
				break;
			}
		case RT_RT: 
			{
				bcRTRTTransfer();
				break;
			}
		case BCAST: 
			{		
				bcBroadcastTransfer();
				break;
			}
		case RT_RT_BCAST: 
			{
				bcRTBroadcastTransfer();
				break;
			}
		case MODE_CODE: 
			{
				if ((m_msgCmdWord & 0x001f) >= 16) 
				{
					if (m_msgCmdWord & 0x0400) 
					{
						bcModeCodeTXDataTransfer();
					}
					else 
					{
						bcModeCodeRXDataTransfer();
					}
				} 
				else 
				{
					bcModeCodeNoDataTransfer();
				}

				break;
			}
		case MODE_CODE_BCAST: 
			{
				if ((m_msgCmdWord & 0x001f) >= 16) 
				{
					bcModeCodeDataBroadcastTransfer();
				} 
				else 
				{
					bcModeCodeNoDataBroadcastTransfer();
				}

				break;
			}
		default:
			break; 
	}

	
	return 0;
}


UINT16 SimBC::bcProcessStatusWord(UINT16 statusWord, UINT16 commandWord)
{
	//wordParse("BC",DATA_TYPE_STATUS_WORD,Reg::timeTagReg,statusWord);
	llogDebug("BC","Process status word");
	Reg::timeRecorder += 10;
	updateTimeReg();
	m_bcCurrentMsgTimeConsuming += 10;
	m_bcCurrentFrameTimeConsuming += 10;
	UINT16 responseAddress = (statusWord & 0xf800) >> 11;
	UINT16 expectAddress = (commandWord & 0xf800) >> 11;
	if(expectAddress == responseAddress)
	{
		llogDebug("BC","Responded RT address is right");
		UINT16 isError = statusWord & 0x0400;
		UINT16 isBusy = statusWord & 0x0008;
		if(isError || isBusy)
		{
			llogWarn("BC","Responded RT is busy or error!");
			
			//Enhance mode and retry enabled
			if((Reg::configReg_3 & 0x8000) && (Reg::configReg_1 &0x0010))
			{
				//Expand control word and retry enabled control word
				if(((Reg::configReg_4 & 0x1000) && (Reg::bcCtrlWordReg & 0x0100)) 
					|| (!(Reg::configReg_4 & 0x1000)))
				{
					UINT16 maxRetryCount = (Reg::configReg_1 & 0x0008)? 2:1;
					if(m_currentRetryCount < maxRetryCount)
					{
						llogInfo("BC","maxRetryCount=%u,and m_currentRetryCount=%u,BC will retry",maxRetryCount,m_currentRetryCount);
						m_bcIsRetrying = true;
						m_currentRetryCount ++;
						m_bcCurrentMsgRetryCyc = 0;
						return 1;
					}
				}
			} 

						
			m_bcIsRetrying = false;
			m_bcCurrentMsgCyc = 0;
			m_bcCurrentMsgRetryCyc = 0;
			m_bcCurrentMsgCycCount = 0;
			m_currentRetryCount = 0;
			if((Reg::configReg_1 & 0x1000) || (Reg::configReg_1 & 0x0900)) //Message stop on error or frame stop on error(if auto retry frame)
			{
				llogFatal("BC","BC will halted!");
				Reg::isHalted = true;
				return 1;

			}
			
			llogWarn("BC","m_currentRetryCount exceeds maxRetryCount or doesn't allow retry,Skip this message to next message");
			m_currentRetryCount = 0;
			bcEndMsg();
			return 1;
		}
		llogDebug("BC","Responded status word OK");
		return 0;
	}
	else
	{
		llogError("BC","The Responded RT address is wrong");
		//Enhance mode and retry enabled
		if((Reg::configReg_3 & 0x8000) && (Reg::configReg_1 &0x0010))
		{
			//Expand control word and retry enabled control word
			if(((Reg::configReg_4 & 0x1000) && (Reg::bcCtrlWordReg & 0x0100)) 
				|| (!(Reg::configReg_4 & 0x1000)))
			{
				UINT16 maxRetryCount = (Reg::configReg_1 & 0x0008)? 2:1;
				if(m_currentRetryCount < maxRetryCount)
				{
					llogInfo("BC","maxRetryCount=%u,and m_currentRetryCount=%u,BC will retry",maxRetryCount,m_currentRetryCount);
					m_bcIsRetrying = true;
					m_currentRetryCount ++;
					m_bcCurrentMsgRetryCyc = 0;
					return 1;
				}
			}
		}


		m_bcIsRetrying = false;
		m_bcCurrentMsgCyc = 0;
		m_bcCurrentMsgRetryCyc = 0;
		m_bcCurrentMsgCycCount = 0;
		m_currentRetryCount = 0;
		if((Reg::configReg_1 & 0x1000) || (Reg::configReg_1 & 0x0900)) //Message stop on error or frame stop on error(if auto retry frame)
		{
			Reg::isHalted = true;
			llogInfo("BC","BC will retry");
			return 1;

		}
		
		llogWarn("BC","m_currentRetryCount exceeds maxRetryCount or doesn't allow retry,Skip this message to next message");
		m_currentRetryCount = 0;
		bcEndMsg();
		return 1;
	}
	return 0;
}






UINT16 SimBC::bcBCRTTransfer(void)
{
	UINT16 recvCmdWord = memRead(m_msgBlockAddr + 1);
	UINT16 dataCount = 0;
	UINT16 time = Reg::timeRecorder & 0xffff;
	memWrite(Reg::cmdStackPtrReg + 1, time);
	if (recvCmdWord & 0X0001F) 
	{
		dataCount = (recvCmdWord & 0x0001f);//Count of data
	}
	else 
	{
		dataCount = 32;
	}

	//Recv command
	if ((m_bcIsRetrying && !m_bcCurrentMsgRetryCyc) || !m_bcCurrentMsgCyc) 
	{
		bcSetBus(DATA_TYPE_COMMAND_WORD,time,recvCmdWord,1);
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc <= dataCount) || m_bcCurrentMsgCyc <= dataCount) {//Data word
		UINT16 data;
		if(m_bcIsRetrying) 
		{
			data = memRead(m_msgBlockAddr + m_bcCurrentMsgRetryCyc + 1);
		}
		else
		{
			data = memRead(m_msgBlockAddr + m_bcCurrentMsgCyc + 1);
		}
		
		bcSetBus(DATA_TYPE_DATA_WORD,time,data,1);
		
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == (dataCount + 1)) || m_bcCurrentMsgCyc == (dataCount + 1)) //Last Data Word Loop Back
	{ 
			UINT16 lastDataWord = memRead(m_msgBlockAddr + dataCount + 1);
			memWrite(m_msgBlockAddr + dataCount + 2, lastDataWord);
			if(m_bcIsRetrying)
			{
				m_bcCurrentMsgRetryCyc ++;
			}
			else
			{
				m_bcCurrentMsgCyc ++;
			}
			//Loop back word, step again
			bcStep();
			return 1;
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == (dataCount + 2)) || m_bcCurrentMsgCyc == (dataCount + 2)) //Wait Status Word
	{

	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == (dataCount + 3)) || m_bcCurrentMsgCyc == (dataCount + 3)) //Status Word
	{ 
		
		UINT16 busData[4];
		UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!retValue)	
		{
			wordParse("BC  ",busData[0],busData[2],busData[1]);
			if (busData[0] != DATA_TYPE_STATUS_WORD) 
			{
				//Report a error
				bcRecvImproperWord();
				return 1;
			}
			else
			{
				memWrite(m_msgBlockAddr + dataCount + 3,busData[1]);
				if(bcProcessStatusWord(busData[1],m_msgCmdWord))
				{
					return 1;
				}
			}
		}
		else {
			//Report a error for timeout
			bcRecvTimeout();
			return 1;
		}
	}
	
	if(m_bcIsRetrying)
	{
		m_bcCurrentMsgRetryCyc ++;
	}
	else
	{
		m_bcCurrentMsgCyc ++;
	}
	Reg::timeRecorder += 20;//For Loop test, Reg::timeRecorder will not increase 
	updateTimeReg();
	m_bcCurrentMsgTimeConsuming += 20;
	m_bcCurrentFrameTimeConsuming += 20;
	return 0;
}
UINT16 SimBC::bcRTBCTransfer(void)
{
	UINT16 transCmdWord = memRead(m_msgBlockAddr + 1);
	UINT16 dataCount = 0;
	UINT16 time = Reg::timeRecorder & 0xFFFF;;
	memWrite(Reg::cmdStackPtrReg + 1, time);
	if (transCmdWord & 0X0001F) 
	{
		dataCount = (transCmdWord & 0x0001f);//Count of data
	}
	else 
	{
		dataCount = 32;
	}
	//Trans Command word
	if ((m_bcIsRetrying && !m_bcCurrentMsgRetryCyc) || !m_bcCurrentMsgCyc) 
	{
		bcSetBus(DATA_TYPE_COMMAND_WORD,time,transCmdWord,1);	
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 1) || m_bcCurrentMsgCyc == 1) //Loop back trans command word
	{ 
		UINT16 lastTransCommandWord = memRead(m_msgBlockAddr + 1);
		memWrite(m_msgBlockAddr + 2, lastTransCommandWord);
		if(m_bcIsRetrying)
		{
			m_bcCurrentMsgRetryCyc ++;
		}
		else
		{
			m_bcCurrentMsgCyc ++;
		}
		///Loop back trans command word,Step again
		bcStep();
		return 1;

	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 2) || m_bcCurrentMsgCyc == 2) //Wait Status word
	{

	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 3) || m_bcCurrentMsgCyc == 3) //Status word
	{ 
		UINT16 busData[4];
		UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!retValue)
		{
			wordParse("BC  ",busData[0],busData[2],busData[1]);
			if (busData[0] != DATA_TYPE_STATUS_WORD) 
			{
				//Report a error
				bcRecvImproperWord();
				return 1;
			}
			else
			{
				memWrite(m_msgBlockAddr + 3,busData[1]);
				if(bcProcessStatusWord(busData[1],m_msgCmdWord))
				{
					return 1;
				}
			}
			
		}
		else 
		{
			//Report a error for timeout
			bcRecvTimeout();
			return 1;
		}

	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc <= (dataCount + 3)) || m_bcCurrentMsgCyc <= (dataCount + 3)) //Data word
	{
		UINT16 busData[4];
		UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!retValue)
		{
			wordParse("BC  ",busData[0],busData[2],busData[1]);
			if (busData[0] != DATA_TYPE_DATA_WORD) 
			{
				//Report a error
				bcRecvImproperWord();
				return 1;
			}
			else
			{
				if(m_bcIsRetrying)
				{
					memWrite(m_msgBlockAddr + m_bcCurrentMsgRetryCyc + 1,busData[1]);
				}
				else
				{
					memWrite(m_msgBlockAddr + m_bcCurrentMsgCyc + 1,busData[1]);
				}
			}			
		}
		else 
		{
			//Report a error for timeout
			bcRecvTimeout();
			return 1;
		}
	}
	if(m_bcIsRetrying)
	{
		m_bcCurrentMsgRetryCyc ++;
	}
	else
	{
		m_bcCurrentMsgCyc ++;
	}
	Reg::timeRecorder += 20;//For Loop test, Reg::timeRecorder will not increase 
	updateTimeReg();
	m_bcCurrentMsgTimeConsuming += 20;
	m_bcCurrentFrameTimeConsuming += 20;
	return 0;
}

UINT16 SimBC::bcRTRTTransfer(void)
{
	UINT16 recvCmdWord = memRead(m_msgBlockAddr + 1);
	UINT16 transCmdWord = memRead(m_msgBlockAddr + 2);
	UINT16 dataCount = 0;
	UINT16 time = Reg::timeRecorder & 0xffff;
	memWrite(Reg::cmdStackPtrReg + 1, time);
	if (recvCmdWord & 0X0001F) 
	{
		dataCount = (recvCmdWord & 0x0001f);//Count of data
	}
	else 
	{
		dataCount = 32;
	}
	//Recv Command word
	if ((m_bcIsRetrying && !m_bcCurrentMsgRetryCyc) || !m_bcCurrentMsgCyc) 
	{
		bcSetBus(DATA_TYPE_COMMAND_WORD,time,recvCmdWord,1);	
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 1) || m_bcCurrentMsgCyc == 1) //Trans Command Word
	{ 
		bcSetBus(DATA_TYPE_COMMAND_WORD,time,transCmdWord,1);
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 2) || m_bcCurrentMsgCyc == 2)//Loop back Trans command Word
	{ 
		UINT16 lastTransCommandWord = memRead(m_msgBlockAddr + 2);
		memWrite(m_msgBlockAddr + 3, lastTransCommandWord);
		if(m_bcIsRetrying)
		{
			m_bcCurrentMsgRetryCyc ++;
		}
		else
		{
			m_bcCurrentMsgCyc ++;
		}
		//Loop back Trans command Word, step again
		bcStep();
		return 1;

	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 3) || m_bcCurrentMsgCyc == 3) //Wait Trans status word
	{

	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 4) || m_bcCurrentMsgCyc == 4) //Trans status word
	{
		UINT16 busData[4];
		UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!retValue)
		{
			wordParse("BC  ",busData[0],busData[2],busData[1]);
			if (busData[0] != DATA_TYPE_STATUS_WORD) 
			{
				//Report a error
				bcRecvImproperWord();
				return 1;
			}
			else
			{
				memWrite(m_msgBlockAddr + 4,busData[1]);
				if(bcProcessStatusWord(busData[1],m_msgSecondCmdWord))
				{
					return 1;
				}
			}
		}
		else 
		{
			//Report a error for timeout
			bcRecvTimeout();
			return 1;
		}
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc < (dataCount + 5)) || m_bcCurrentMsgCyc < (dataCount + 5)) //Data word
	{
		UINT16 busData[4];
		UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!retValue)
		{
			wordParse("BC  ",busData[0],busData[2],busData[1]);
			if (busData[0] != DATA_TYPE_DATA_WORD) 
			{
				//Report a error
				bcRecvImproperWord();
				return 1;
			}
			else
			{
				if(m_bcIsRetrying)
				{
					 memWrite(m_msgBlockAddr + m_bcCurrentMsgRetryCyc + 1,busData[1]);
				}
				else
				{
					memWrite(m_msgBlockAddr + m_bcCurrentMsgCyc + 1,busData[1]);
				}
			}
			

		}
		else 
		{
			//Report a error for timeout
			bcRecvTimeout();
			return 1;
		}
		
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == (dataCount + 5)) || m_bcCurrentMsgCyc == (dataCount + 5)) //Recv status word
	{ 
		UINT16 busData[4];
		UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!retValue)
		{
			wordParse("BC  ",busData[0],busData[2],busData[1]);
			if (busData[0] != DATA_TYPE_STATUS_WORD) 
			{
				//Report a error
				bcRecvImproperWord();
				return 1;
			}
			else
			{
				memWrite(m_msgBlockAddr + dataCount + 4,busData[1]);
				if(bcProcessStatusWord(busData[1],m_msgCmdWord))
				{
					return 1;
				}
			}
			

		}
		else 
		{
			//Report a error for timeout
			bcRecvTimeout();
			return 1;
		}
	}
	if(m_bcIsRetrying)
	{
		 m_bcCurrentMsgRetryCyc ++;
	}
	else
	{
		m_bcCurrentMsgCyc ++;
	}
	Reg::timeRecorder += 20;//For Loop test, Reg::timeRecorder will not increase 
	updateTimeReg();
	m_bcCurrentMsgTimeConsuming += 20;
	m_bcCurrentFrameTimeConsuming += 20;
	return 0;
}
UINT16 SimBC::bcModeCodeNoDataTransfer(void)
{
	UINT16 modeCmdWord = memRead(m_msgBlockAddr + 1);
	UINT16 time = Reg::timeRecorder & 0xFFFF;
	memWrite(Reg::cmdStackPtrReg + 1, time);
	if ((m_bcIsRetrying && !m_bcCurrentMsgRetryCyc) || !m_bcCurrentMsgCyc) 
	{
		bcSetBus(DATA_TYPE_COMMAND_WORD,time,modeCmdWord,1);
	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 1) || m_bcCurrentMsgCyc == 1) //Loop back
	{
		UINT16 lastModeCommandWord = memRead(m_msgBlockAddr + 1);
		memWrite(m_msgBlockAddr + 2,lastModeCommandWord);
		if(m_bcIsRetrying)
		{
			m_bcCurrentMsgRetryCyc ++;
		}
		else
		{
			m_bcCurrentMsgCyc ++;
		}
		bcStep();
		return 1;


	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 2) || m_bcCurrentMsgCyc == 2) 
	{
		//Wait status word
	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 3) || m_bcCurrentMsgCyc == 3) 
	{
		UINT16 busData[4];
		UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!retValue)
		{
			wordParse("BC  ",busData[0],busData[2],busData[1]);
			if (busData[0] != DATA_TYPE_STATUS_WORD) 
			{
				//Report a error
				bcRecvImproperWord();
				return 1;
			}
			else
			{
				memWrite(m_msgBlockAddr + 3,busData[1]);
				if(bcProcessStatusWord(busData[1],m_msgCmdWord))
				{
					return 1;
				}
			}
		}
		else 
		{
			//Report a error for timeout
			bcRecvTimeout();
			return 1;
		}
	}
	if(m_bcIsRetrying)
	{
		m_bcCurrentMsgRetryCyc ++;
	}
	else
	{
		m_bcCurrentMsgCyc ++;
	}
	Reg::timeRecorder += 20;//For Loop test, Reg::timeRecorder will not increase 
	updateTimeReg();
	m_bcCurrentMsgTimeConsuming += 20;
	m_bcCurrentFrameTimeConsuming += 20;
	return 0;
}
UINT16 SimBC::bcModeCodeTXDataTransfer(void)
{
	UINT16 modeTransCmdWord = memRead(m_msgBlockAddr + 1);
	UINT16 time = Reg::timeRecorder & 0xFFFF;
	memWrite(Reg::cmdStackPtrReg + 1, time);

	if ((m_bcIsRetrying && !m_bcCurrentMsgRetryCyc) || !m_bcCurrentMsgCyc) 
	{
		bcSetBus(DATA_TYPE_COMMAND_WORD,time,modeTransCmdWord,1);
	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 1) || m_bcCurrentMsgCyc == 1) //Loop back
	{ 
		UINT16 lastTransModeCommandWord = memRead(m_msgBlockAddr + 1);
		memWrite(m_msgBlockAddr + 2,lastTransModeCommandWord);
		if(m_bcIsRetrying)
		{
			m_bcCurrentMsgRetryCyc ++;
		}
		else
		{
			m_bcCurrentMsgCyc ++;
		}
		bcStep();
		return 1;

	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 2) || m_bcCurrentMsgCyc == 2) 
	{
		//Wait status word
	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 3) || m_bcCurrentMsgCyc == 3) 
	{
		UINT16 busData[4];
		UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!retValue)
		{
			wordParse("BC  ",busData[0],busData[2],busData[1]);
			if (busData[0] != DATA_TYPE_STATUS_WORD) {
				//Report a error
				bcRecvImproperWord();
				return 1;
			}
			else
			{
				memWrite(m_msgBlockAddr + 3,busData[1]);
				if(bcProcessStatusWord(busData[1],m_msgCmdWord))
				{
					return 1;
				}
			}
			

		}
		else 
		{
			//Report a error for timeout
			bcRecvTimeout();
			return 1;
		}
	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 4) || m_bcCurrentMsgCyc == 4)  
	{
		UINT16 busData[4];
		UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!retValue)
		{
			wordParse("BC",busData[0],busData[2],busData[1]);
			if (busData[0] != DATA_TYPE_DATA_WORD) {
				//Report a error
				bcRecvImproperWord();
				return 1;
			}
			else
			{
				memWrite(m_msgBlockAddr + 4,busData[1]);
			}
		}
		else {
			//Report a error for timeout
			bcRecvTimeout();
			return 1;
		}
	}
	if(m_bcIsRetrying)
	{
		 m_bcCurrentMsgRetryCyc ++;
	}
	else
	{
		m_bcCurrentMsgCyc ++;
	}
	Reg::timeRecorder += 20;//For Loop test, Reg::timeRecorder will not increase 
	updateTimeReg();
	m_bcCurrentMsgTimeConsuming += 20;
	m_bcCurrentFrameTimeConsuming += 20;
	return 0;
}

UINT16 SimBC::bcModeCodeRXDataTransfer(void)
{
	UINT16 modeRecvCmdWord = memRead(m_msgBlockAddr + 1);
	UINT16 time = Reg::timeRecorder & 0xFFFF;
	memWrite(Reg::cmdStackPtrReg + 1, time);
	if ((m_bcIsRetrying && (!m_bcCurrentMsgRetryCyc)) || !m_bcCurrentMsgCyc) 
	{
		bcSetBus(DATA_TYPE_COMMAND_WORD,time,modeRecvCmdWord,1);
	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 1) || m_bcCurrentMsgCyc == 1) 
	{
		UINT16 data = memRead(m_msgBlockAddr + 2);
		bcSetBus(DATA_TYPE_DATA_WORD,time,data,1);
	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 2) || m_bcCurrentMsgCyc == 2) //Loop back 
	{
		UINT16 data = memRead(m_msgBlockAddr + 2);
		memWrite(m_msgBlockAddr + 3,data);
		if(m_bcIsRetrying)
		{
			m_bcCurrentMsgRetryCyc ++;
		}
		else
		{
			m_bcCurrentMsgCyc ++;
		}
		bcStep();
		return 1;
	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 3) || m_bcCurrentMsgCyc == 3) 
	{
		//Wait status word
	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 4) || m_bcCurrentMsgCyc == 4) 
	{
		UINT16 busData[4];
		UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!retValue)
		{
			wordParse("BC  ",busData[0],busData[2],busData[1]);
			if (busData[0] != DATA_TYPE_STATUS_WORD) 
			{
				//Report a error
				bcRecvImproperWord();
				return 1;
			}
			else
			{
				memWrite(m_msgBlockAddr + 4,busData[1]);
				if(bcProcessStatusWord(busData[1],m_msgCmdWord))
				{
					return 1;
				}
			}
		}
		else 
		{
			//Report a error for timeout
			bcRecvTimeout();
			return 1;
		}
	}
	if(m_bcIsRetrying)
	{
		 m_bcCurrentMsgRetryCyc ++;
	}
	else
	{
		m_bcCurrentMsgCyc ++;
	}
	Reg::timeRecorder += 20;//For Loop test, Reg::timeRecorder will not increase 
	updateTimeReg();
	m_bcCurrentMsgTimeConsuming += 20;
	m_bcCurrentFrameTimeConsuming += 20;
	return 0;
}
UINT16 SimBC::bcBroadcastTransfer(void)
{
	UINT16 broadCmdWord = memRead(m_msgBlockAddr + 1);
	UINT16 time = Reg::timeRecorder & 0xFFFF;
	memWrite(Reg::cmdStackPtrReg + 1, time);
	UINT16 dataCount = 0;
	if (broadCmdWord & 0X0001F) 
	{
		dataCount = (broadCmdWord & 0x0001f);//Count of data
	}
	else 
	{
		dataCount = 32;
	}
	if ((m_bcIsRetrying && !m_bcCurrentMsgRetryCyc) || !m_bcCurrentMsgCyc) 
	{
		bcSetBus(DATA_TYPE_COMMAND_WORD,time,broadCmdWord,1);
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc <= dataCount) || m_bcCurrentMsgCyc <= dataCount) 
	{
		UINT16 data;
		if(m_bcIsRetrying)
		{
			 data = memRead(m_msgBlockAddr + m_bcCurrentMsgRetryCyc + 1);
		}
		else
		{	
			data = memRead(m_msgBlockAddr + m_bcCurrentMsgCyc + 1);
		}
		
		bcSetBus(DATA_TYPE_DATA_WORD,time,data,1);
		
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == (dataCount + 1)) || m_bcCurrentMsgCyc == (dataCount + 1)) //Loop back(This is the last word, do nothing)
	{
		INT16 data = memRead(m_msgBlockAddr + dataCount +1);
		memWrite(m_msgBlockAddr + dataCount + 2,data);
	}
	if(m_bcIsRetrying)
	{
		 m_bcCurrentMsgRetryCyc ++;
	}
	else
	{
		m_bcCurrentMsgCyc ++;
	}
	Reg::timeRecorder += 20;//For Loop test, Reg::timeRecorder will not increase 
	updateTimeReg();
	m_bcCurrentMsgTimeConsuming += 20;
	m_bcCurrentFrameTimeConsuming += 20;
	return 0;
}
UINT16 SimBC::bcRTBroadcastTransfer(void)
{
	UINT16 broadRxCmdWord = memRead(m_msgBlockAddr + 1);
	UINT16 time = Reg::timeRecorder & 0xFFFF;
	memWrite(Reg::cmdStackPtrReg + 1, time);
	UINT16 dataCount = 0;
	if (broadRxCmdWord & 0X0001F) 
	{
		dataCount = (broadRxCmdWord & 0x0001f);//Count of data
	}
	else 
	{
		dataCount = 32;
	}
	if ((m_bcIsRetrying && !m_bcCurrentMsgRetryCyc) || !m_bcCurrentMsgCyc) 
	{
		bcSetBus(DATA_TYPE_COMMAND_WORD,time,broadRxCmdWord,1);
		
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 1) || m_bcCurrentMsgCyc == 1) 
	{
		UINT16 transCommand = memRead(m_msgBlockAddr + 2);
		
		bcSetBus(DATA_TYPE_COMMAND_WORD,time,transCommand,1);
		
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 2) || m_bcCurrentMsgCyc ==  2) //Loop back
	{ 
		UINT16 lastTransCommand = memRead(m_msgBlockAddr + 2);
		memWrite(m_msgBlockAddr + 3, lastTransCommand);
		if(m_bcIsRetrying)
		{
			m_bcCurrentMsgRetryCyc ++;
		}
		else
		{
			m_bcCurrentMsgCyc ++;
		}
		bcStep();
		return 1;
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 3) || m_bcCurrentMsgCyc ==  3) 
	{
		//Wait status word
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 4) || m_bcCurrentMsgCyc ==  4) 
	{
		UINT16 busData[4];
		UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!retValue)
		{
			wordParse("BC  ",busData[0],busData[2],busData[1]);
			if (busData[0] != DATA_TYPE_STATUS_WORD) 
			{
				//Report a error
				bcRecvImproperWord();
				return 1;
			}
			else
			{
				memWrite(m_msgBlockAddr + 4,busData[1]);
				if(bcProcessStatusWord(busData[1],m_msgSecondCmdWord))
				{
					return 1;
				}
			}

		}
		else 
		{
			//Report a error for timeout
			bcRecvTimeout();
			return 1;
		}
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc <= (dataCount + 4)) || m_bcCurrentMsgCyc <= (dataCount + 4)) 
	{
		UINT16 busData[4];
		UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!retValue)
		{
			wordParse("BC  ",busData[0],busData[2],busData[1]);
			if (busData[0] != DATA_TYPE_DATA_WORD)
			{
				//Report a error
				bcRecvImproperWord();
				return 1;
			}
			else
			{
				if(m_bcIsRetrying)
				{
					memWrite(m_msgBlockAddr + m_bcCurrentMsgRetryCyc + 1,busData[1]);
				}
				else
				{
					memWrite(m_msgBlockAddr + m_bcCurrentMsgCyc + 1,busData[1]);
				}
			}
		}
		else {
			//Report a error for timeout
			bcRecvTimeout();
			return 1;
		}
	}
	if(m_bcIsRetrying)
	{
		m_bcCurrentMsgRetryCyc ++;
	}
	else
	{
		m_bcCurrentMsgCyc ++;
	}
	Reg::timeRecorder += 20;//For Loop test, Reg::timeRecorder will not increase 
	updateTimeReg();
	m_bcCurrentMsgTimeConsuming += 20;
	m_bcCurrentFrameTimeConsuming += 20;
	return 0;
}
UINT16 SimBC::bcModeCodeNoDataBroadcastTransfer(void)
{
	UINT16 broadModeCmdWord = memRead(m_msgBlockAddr + 1);
	UINT16 time = Reg::timeRecorder & 0xFFFF;
	memWrite(Reg::cmdStackPtrReg + 1, time);
	if ((m_bcIsRetrying && !m_bcCurrentMsgRetryCyc) || !m_bcCurrentMsgCyc) 
	{
		bcSetBus(DATA_TYPE_COMMAND_WORD,time,broadModeCmdWord,1);
	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 1) || m_bcCurrentMsgCyc == 1) //Loop back(This is the last word, Do nothing)
	{
		UINT16 lastModeCommand = memRead(m_msgBlockAddr + 1);
		memWrite(m_msgBlockAddr + 2, lastModeCommand);
	}

	if(m_bcIsRetrying)
	{
		m_bcCurrentMsgRetryCyc ++;
	}
	else
	{
		m_bcCurrentMsgCyc ++;
	}
	Reg::timeRecorder += 20;//For Loop test, Reg::timeRecorder will not increase 
	updateTimeReg();
	m_bcCurrentMsgTimeConsuming += 20;
	m_bcCurrentFrameTimeConsuming += 20;
	return 0;
}
UINT16 SimBC::bcModeCodeDataBroadcastTransfer(void)
{
	UINT16 broadModeCmdWord = memRead(m_msgBlockAddr + 1);
	UINT16 time = Reg::timeRecorder & 0xFFFF;
	memWrite(Reg::cmdStackPtrReg + 1, time);
	if ((m_bcIsRetrying && !m_bcCurrentMsgRetryCyc) || !m_bcCurrentMsgCyc) 
	{ 
		bcSetBus(DATA_TYPE_COMMAND_WORD,time,broadModeCmdWord,1);
	}
	else if((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 1) || m_bcCurrentMsgCyc == 1)
	{
		UINT16 data = memRead(m_msgBlockAddr + 2); 
		bcSetBus(DATA_TYPE_DATA_WORD,time,data,1);
	
	}
	else if ((m_bcIsRetrying && m_bcCurrentMsgRetryCyc == 2) || m_bcCurrentMsgCyc == 2) //Loop back (This is the last word, Do nothing)
	{
		UINT16 lastModeCommand = memRead(m_msgBlockAddr + 2);
		memWrite(m_msgBlockAddr + 3, lastModeCommand);
	}
	if(m_bcIsRetrying)
	{
		m_bcCurrentMsgRetryCyc ++;
	}
	else
	{
		m_bcCurrentMsgCyc ++;
	}
	Reg::timeRecorder += 20;//For Loop test, Reg::timeRecorder will not increase 
	updateTimeReg();
	m_bcCurrentMsgTimeConsuming += 20;
	m_bcCurrentFrameTimeConsuming += 20;
	return 0;
}

UINT16 SimBC::bcRecvImproperWord()
{
	llogError("BC","Received a improper word"); 
	genIRQ();
	//Enhance mode and retry enabled
	if((Reg::configReg_3 & 0x8000) && (Reg::configReg_1 &0x0010))
	{
		//Expand control word and retry enabled control word
		if(((Reg::configReg_4 & 0x1000) && (Reg::bcCtrlWordReg & 0x0100)) 
			|| (!(Reg::configReg_4 & 0x1000)))
		{
			UINT16 maxRetryCount = (Reg::configReg_1 & 0x0008)? 2:1;
			if(m_currentRetryCount < maxRetryCount)
			{
				llogInfo("BC","maxRetryCount=%u,and m_currentRetryCount=%u,BC will retry",maxRetryCount,m_currentRetryCount);
				m_bcIsRetrying = true;
				m_currentRetryCount ++;
				m_bcCurrentMsgRetryCyc = 0;
				return 0;
			}
		}
	}
	
	m_bcIsRetrying = false;
	m_bcCurrentMsgCyc = 0;
	m_bcCurrentMsgRetryCyc = 0;
	m_bcCurrentMsgCycCount = 0;
	m_currentRetryCount = 0;
	if((Reg::configReg_1 & 0x1000) || (Reg::configReg_1 & 0x0900)) //Message stop on error or frame stop on error(if auto retry frame)
	{
		Reg::isHalted = true;
		llogFatal("BC","BC will halted!");
		return 1;

	}
	
	llogWarn("BC","m_currentRetryCount exceeds maxRetryCount or doesn't allow retry,Skip this message to next message");
	m_currentRetryCount = 0;
	bcEndMsg();
	return 1;
}
UINT16 SimBC::bcRecvTimeout()
{
	llogInfo("BC","Waiting Recv timeout!");
	genIRQ();
	Reg::timeRecorder += 15;//For Loop test, Reg::timeRecorder will not increase 
	updateTimeReg();
	m_bcCurrentMsgTimeConsuming += 15;
	m_bcCurrentFrameTimeConsuming += 15;

	//Enhance mode and retry enabled
	if((Reg::configReg_3 & 0x8000) && (Reg::configReg_1 &0x0010))
	{
			//Expand control word and retry enabled control word
		if(((Reg::configReg_4 & 0x1000) && (Reg::bcCtrlWordReg & 0x0100)) 
			|| (!(Reg::configReg_4 & 0x1000)))
		{
			UINT16 maxRetryCount = (Reg::configReg_1 & 0x0008)? 2:1;
			if(m_currentRetryCount < maxRetryCount)
			{
				llogInfo("BC","maxRetryCount=%u,and m_currentRetryCount=%u,BC will retry",maxRetryCount,m_currentRetryCount);
				m_bcIsRetrying = true;
				m_currentRetryCount ++;
				m_bcCurrentMsgRetryCyc = 0;
				return 0;
			}
			else
			{
				llogInfo("BC","maxRetryCount=%u,and m_currentRetryCount=%u,BC will retry",maxRetryCount,m_currentRetryCount);
				
			}
		}
	}



	m_bcIsRetrying = false;
	m_bcCurrentMsgCyc = 0;
	m_bcCurrentMsgRetryCyc = 0;
	m_bcCurrentMsgCycCount = 0;
	m_currentRetryCount = 0;
	if((Reg::configReg_1 & 0x1000) || (Reg::configReg_1 & 0x0900)) //Message stop on error or frame stop on error(if auto retry frame)
	{
		Reg::isHalted = true;
		llogFatal("BC","BC will halted!");
		return 1;

	}
	
	llogWarn("BC","m_currentRetryCount exceeds maxRetryCount or doesn't allow retry,Skip this message to next message");
	m_currentRetryCount = 0;
	bcEndMsg();
	return 1;
}


UINT16 SimBC::initForTest()
{
	//software reset
	Reg::startResetReg = 0x0001;
	//set enhance mode 
	Reg::configReg_3 = 0x8000;
	//enable interrupts for bc control word(individual messages) and bc end of frame
	Reg::intMaskReg = 0x0018;
	//configure for bc mode, enables variable message gap time and message retry
	Reg::configReg_1 = 0x0030;
	//Enable 256 boundries disabled, level intrupput
	Reg::configReg_2 = 0x0408;
	//enable expanded bc control word, valid busy/no data, valid message error/no data
	Reg::configReg_4 = 0x0408;
	//enable expanded zero crossing, programes bc response timeout 22.5us
	Reg::configReg_5 = 0x0A00;

	Reg::timeRecorder = 0;
	Reg::timeTagReg = 0;

	Mem::mem[BC_STACK_POINTER_A_ADDR] = 0;
	Mem::mem[BC_MSG_CNT_A_ADDR] = 0xfffc;
	Mem::mem[BC_STACK_A_ADDR] = 0;
	Mem::mem[BC_STACK_A_ADDR + 1] = 0;
	Mem::mem[BC_STACK_A_ADDR + 2] = 0x0320;
	Mem::mem[BC_STACK_A_ADDR + 3] = 0x0108;

	Mem::mem[BC_STACK_A_ADDR + 4] = 0;
	Mem::mem[BC_STACK_A_ADDR + 5] = 0;
	Mem::mem[BC_STACK_A_ADDR + 6] = 0x03e8;
	Mem::mem[BC_STACK_A_ADDR + 7] = 0x0110;

	Mem::mem[BC_STACK_A_ADDR + 8] = 0;
	Mem::mem[BC_STACK_A_ADDR + 9] = 0;
	Mem::mem[BC_STACK_A_ADDR + 0x000A] = 0;
	Mem::mem[BC_STACK_A_ADDR + 0x000B] = 0x0119;

	Mem::mem[0x0108] = 0x0180;
	Mem::mem[0x0109] = 0x0824;//RT address is 1 and sub Address is 1,Data count is 4.
	Mem::mem[0x010a] = 0x1111;
	Mem::mem[0x010b] = 0x2222;
	Mem::mem[0x010c] = 0x3333;
	Mem::mem[0x010d] = 0x4444;
	Mem::mem[0x010e] = 0;
	Mem::mem[0x010f] = 0;


	
	Mem::mem[0x0110] = 0x0101;
	Mem::mem[0x0111] = 0x1863;//RT address is 3 and sub Address is 3,Data count is 3
	Mem::mem[0x0112] = 0x1443;//RT address is 2 and sub Address is 2,Data count is 3
	Mem::mem[0x0113] = 0x0;
	Mem::mem[0x0114] = 0;
	Mem::mem[0x0115] = 0;
	Mem::mem[0x0116] = 0;
	Mem::mem[0x0117] = 0;
	Mem::mem[0x0118] = 0;


	
	Mem::mem[0x0119] = 0x0084;//RT address is 4 and sub Address is 0,mode code
	Mem::mem[0x011a] = 0x2011;
	Mem::mem[0x011b] = 0xabcd;
	Mem::mem[0x011c] = 0;
	Mem::mem[0x011d] = 0;


	Reg::startResetReg = 0x0002;


	Reg::isHalted = false;
	return 0;

}

void SimBC::bcSetBus(UINT16 type,UINT16 time,UINT16 data,UINT16 isFull)
{
	wordParse("BC  ",type,time,data);
	if (Reg::bcCtrlWordReg & 0X0080) 
	{ 
		if(!m_bcIsRetrying)
		{
			m_busChannelB.setBusData(type,time,data,isFull);
		}
		else if(m_currentRetryCount == 1)
		{
			if(Reg::configReg_4 & 0x0100)
			{
				m_busChannelA.setBusData(type,time,data,isFull);
			}
			else
			{
				m_busChannelB.setBusData(type,time,data,isFull);
			}
		}
		else if(m_currentRetryCount == 2)
		{
			if(Reg::configReg_4 & 0x0080)
			{
				m_busChannelA.setBusData(type,time,data,isFull);
			}
			else
			{
				m_busChannelB.setBusData(type,time,data,isFull);
			}
		}

	}
	else 
	{
		m_busChannelA.setBusData(type,time,data,isFull);
	}
}

struct TransException SimBC::checkIfException()
{
	struct TransException transExcep = {0,0,TimeOutException,FALSE};

	UINT16 msgCount = m_bcProcessedMsgCount + 1;
	for (UINT16 i = 0; i < 64; i++)
	{
		struct TransException tempExcep = Exception1553B::m_exceptionArray[i];
		if (tempExcep.isError && tempExcep.messageIndex == msgCount)
		{
			if (m_bcIsRetrying)
			{
				if(tempExcep.messageCycCount == m_bcCurrentMsgRetryCyc) 
				{
					transExcep = tempExcep;
					Exception1553B::m_exceptionArray[i].isError = FALSE;
					break;
				}
			}
			else 
			{
				if(tempExcep.messageCycCount == m_bcCurrentMsgCyc)
				{
					transExcep = tempExcep;
					Exception1553B::m_exceptionArray[i].isError = FALSE;
					break;
				}
			}
		}
	}

	return transExcep;
}

UINT32 SimBC::CheckRecvHook(UINT32 len,void *recvData)
{
	struct TransException  transExcep = checkIfException();
	if (transExcep.isError)
	{
		if (transExcep.exceptionType == TimeOutException)
		{		
			return 1;

		}
		else 
		{
			(*((UINT16*)recvData + 0)) = DATA_TYPE_UNDEFINED_WORD;
			return 0;
		}
	}
	else 
	{
		return ::CheckRecv(len,recvData);
	}
	
}

void SimBC::genIRQ(void)
{
	GenIRQ();
}
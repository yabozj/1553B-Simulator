
#include "stdafx.h"
#include "simrt.h"
#include "wordParser.h"
#include <string.h>
extern SimRT *rt;
SimRT::SimRT(UINT16 addr)
{
	m_rtAddress = addr;
	llogDebug("RT","Init RT");
	initRegAddress();
	clearRegs(true);
	
	m_rtCurrentMsgCycCount = 0;
	m_rtCurrentMsgCyc = 0;
	Reg::timeRecorder = 0;
	m_busChannelA.m_busFull = 0;
	m_busChannelB.m_busFull = 0;
	m_rtProcessedMsgCount = 0;
	for(int i = 0; i < MEMSIZE; i++)
	{
		//The default value is 0xCDCD in VC, while in GCC, that is 0x0000
		Mem::mem[i] = 0;
	}
	memset(rtDescription,0,32);
	m_rtBackUp = NULL;
	m_rtDataIndex = 0;
	sprintf(rtDescription,"%s %d","RT",addr);

	
}
SimRT::SimRT(UINT16 addr,char *fileName)
{
	loadConfiguration(fileName,true);
	*this = *rt;
	//SimRT::SimRT(*rt);
	delete rt;
	rt = NULL;
	m_rtBackUp = NULL;
}

SimRT::~SimRT()
{
	if(m_rtBackUp) 
	{
		m_rtBackUp->m_rtBackUp = NULL;
		delete m_rtBackUp;
		m_rtBackUp = NULL;
	}

}

void SimRT::rtRestore()
{
	if (m_rtBackUp)
	{
		*this = *m_rtBackUp;
	}
}

void SimRT::rtSave()
{
	if (m_rtBackUp)
	{
		*m_rtBackUp = *this;
	}
	else {
		m_rtBackUp = new SimRT(m_rtAddress);
		*m_rtBackUp = *this;
	}
}

INT16 SimRT::initRegAddress(void)
{
	//Need to be optimized
	Reg::regRead[INT_MASK_REG_WR_ADDR] = &Reg::intMaskReg_read;
	Reg::regRead[CONFIG_REG_1_WR_ADDR] = &Reg::configReg_1_read;
	Reg::regRead[CONFIG_REG_2_WR_ADDR] = &Reg::configReg_2_read;
	Reg::regRead[CMD_STACK_PTR_REG_R_ADDR] = &Reg::cmdStackPtrReg_read;
	Reg::regRead[RT_SUBADDR_CTRL_WORD_REG_WR_ADDR] = &Reg::rtSubAddrCtrlWordReg_read;
	Reg::regRead[TIME_TAG_REG_WR_ADDR] = &Reg::timeTagReg_read;
	Reg::regRead[INT_STATUS_REG_R_ADDR] = &Reg::intStatusReg_read;
	Reg::regRead[CONFIG_REG_3_WR_ADDR] = &Reg::configReg_3_read;
	Reg::regRead[CONFIG_REG_4_WR_ADDR] = &Reg::configReg_4_read;
	Reg::regRead[CONFIG_REG_5_WR_ADDR] = &Reg::configReg_5_read;
	Reg::regRead[RT_DATA_STACK_ADDR_REG_WR_ADDR] = &Reg::rtDataStackAddrReg_read;
	Reg::regRead[11] = NULL;
	Reg::regRead[12] = NULL;
	Reg::regRead[RT_LAST_CMD_REG_WR_ADDR] = &Reg::rtLastCmdReg_read;
	Reg::regRead[RT_STATUS_REG_R_ADDR] = &Reg::rtStatusWordReg_read;
	Reg::regRead[RT_BIT_WORD_REG_R_ADDR] = &Reg::rtBITWordReg_read;


	Reg::regWrite[INT_MASK_REG_WR_ADDR] = &Reg::intMaskReg_write;
	Reg::regWrite[CONFIG_REG_1_WR_ADDR] = &Reg::configReg_1_write;
	Reg::regWrite[CONFIG_REG_2_WR_ADDR] = &Reg::configReg_2_write;
	Reg::regWrite[START_RESET_REG_W_ADDR] = &Reg::startResetReg_write;
	Reg::regWrite[RT_SUBADDR_CTRL_WORD_REG_WR_ADDR] = &Reg::rtSubAddrCtrlWordReg_write;
	Reg::regWrite[TIME_TAG_REG_WR_ADDR] = &Reg::timeTagReg_write;
	Reg::regWrite[6] = NULL;
	Reg::regWrite[CONFIG_REG_3_WR_ADDR] = &Reg::configReg_3_write;
	Reg::regWrite[CONFIG_REG_4_WR_ADDR] = &Reg::configReg_4_write;
	Reg::regWrite[CONFIG_REG_5_WR_ADDR] = &Reg::configReg_5_write;
	Reg::regWrite[RT_DATA_STACK_ADDR_REG_WR_ADDR] = &Reg::rtDataStackAddrReg_write;
	Reg::regWrite[11] = NULL;
	Reg::regWrite[12] = NULL;
	Reg::regWrite[RT_LAST_CMD_REG_WR_ADDR] = &Reg::rtLastCmdReg_write;
	Reg::regWrite[14] = NULL;
	Reg::regWrite[15] = NULL;

	return 0;
}

UINT16 SimRT::RTStep(void)
{

	if (!Reg::isHalted) 
	{
		//llogDebug("RT","Step");
		UINT16 busData[4]; 
		
		//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
		if (!m_rtCurrentMsgCyc && !CheckRecvHook(4*sizeof(UINT16),&busData)) //CYC is 1 and the first word is command word
		{  
			if(busData[0] == DATA_TYPE_COMMAND_WORD) //Check word type 
			{
				//wordParse(rtDescription,busData[0],busData[2],busData[1]);
				UINT16 cmd = busData[1];
				UINT16 address = cmd>>11;
				if (address == m_rtAddress || address == 31)
				{
					m_rtCurrentMsgCmdWord = busData[1];
					RTReceiveCMD();    //Init the CYC count
					RTStartMsg();
					RTSingleWordTransfer();//Handle the command word
				}
				
			}
			else //The word type is not a command word, and RT will not init a transfer
			{
				
			}
		}		//Whenever CYC is not 0, that means RT get start for word transfer
		else if (m_rtCurrentMsgCyc)
		{
			if(m_rtCurrentMsgCyc < m_rtCurrentMsgCycCount)
			{
				RTSingleWordTransfer();
			}
			if(m_rtCurrentMsgCyc >= m_rtCurrentMsgCycCount)
			{
				RTEndMsg();
			}
			
		}
		
		return 0;
	}
	return 0;
}

UINT16 SimRT::memDump()
{
	return 0;
}

UINT16 SimRT::rtDump(int len, void * buffAddr)
{
	if (sizeof(bu61580_sharemem_struct) > len)
	{
		llogWarn("RTDump","Needs larger buffer size");
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

UINT16 SimRT::RTReceiveCMD()
{
	RTCheckCMDType(m_rtCurrentMsgCmdWord);//Init the CYC count
	return 0;
}
UINT16 SimRT::RTSingleWordTransfer()
{
	UINT16 cmd = m_rtCurrentMsgCmdWord;
	UINT16 subAddress=(cmd>>5)&0X001F;
	UINT16 t_r=(cmd&0X0400) >> 9;
	UINT16 address=cmd>>11;
	UINT16 wordCount_modeCode = cmd&0X001F;
	UINT16 isMyAddress = address == m_rtAddress?0x0004:0x0000;
	UINT16 mode4 = wordCount_modeCode >> 4;
	UINT16 mode0_3 = wordCount_modeCode & 0x0F;
	UINT16 modeSelectiveIntAddr = 0x0108|isMyAddress|mode4|t_r;

	genIRQ();
	if(subAddress == 0 || subAddress == 31)
	{
		UINT16 intData = memRead(modeSelectiveIntAddr);
		if((intData >> mode0_3) & 0X1)    //A mode code selective int
		{
			Reg::intStatusReg |= 0x0002;
		}

		if (wordCount_modeCode < 0X10)
		{
			RTModeCodeWithOutDataTransfer();
		}
		else
		{
			if(t_r)
			{
				RTModeCodeWithDataTransferTX();
			}
			else
			{
				RTModeCodeWithDataTransferRX();
			}
		}
	}
	else
	{
		if(t_r)
		{
			RTTXTransfer();
		}
		else
		{
			RTRXTransfer();
		}
	}
	m_rtCurrentMsgCyc++;
	return 0;
}

UINT16 SimRT::RTModeCodeWithDataTransferTX()
{
	UINT16 cmd = m_rtCurrentMsgCmdWord;
	UINT16 address=cmd>>11;
	UINT16 wordCount_modeCode=cmd&0X001F;
	UINT16 t_r=cmd&0X0400;

	UINT16 timeRegValue = Reg::timeTagReg;

	UINT16 regDataForConfig3 = Reg::configReg_3;
	UINT16 regDataForConfig4 = Reg::configReg_4;
	UINT16 regStatusValue = Reg::rtStatusWordReg;
	UINT16 blockData = memRead(m_rtMsgBlockAddr);

	UINT16 isEnhanceMode = regDataForConfig3&0X8000;
	bool isEnhanceModeCodeHandle = isEnhanceMode && (regDataForConfig3&0X0001);
	UINT16 transData;
	UINT16 stackAddr = Reg::rtDataStackAddrReg;

	if(((regStatusValue&0X0008) || (blockData&0X0040)) && wordCount_modeCode != 0X13)
	{
		//Will not transmit
	}
	else
	{
		if (!m_rtCurrentMsgCyc) 
		{
			//This is the command word
		}
		else if(m_rtCurrentMsgCyc == 2)//RT will transmit the data word
		{ 
			switch (wordCount_modeCode)
			{
			case 0X10:
				if ( isEnhanceModeCodeHandle)
				{
					transData = memRead(RT_MODECODE_DATA_TRANSMIT_ADDR);               
				}
				else
				{
					transData = memRead(stackAddr);       
				}


				break;
			case 0X12://Transmit Last Command Word
				transData = Reg::rtLastCmdReg;       
				
				break;
			case 0X13://Transmit BIT Word
				if (!(regStatusValue&0X0008) || !(isEnhanceMode&&(regDataForConfig4&0X4000)))
				{
					/*if (isEnhanceModeCodeHandle)
					{
						transData = memRead(RT_MODECODE_DATA_TRANSMIT_ADDR + 3);               
					}
					else
					{
						transData = memRead(stackAddr);       
					}*/
					transData = Reg::rtBITWordReg;
				}               

				break;
			default:
				break;
			}

			
			//wordParse(rtDescription,DATA_TYPE_DATA_WORD,timeRegValue,transData);
			if (Reg::rtBITWordReg & 0X0100) 
			{
				m_busChannelB.setBusData(DATA_TYPE_DATA_WORD,timeRegValue,transData,1);
			}
			else {
				m_busChannelA.setBusData(DATA_TYPE_DATA_WORD,timeRegValue,transData,1);
			}
		}
	}
	
	if (!m_rtCurrentMsgCyc)
	{
		m_rtCurrentMsgCyc ++;
		//RTSingleWordTransfer();
		RTReturnStatusWord();
		//m_rtCurrentMsgCyc --; //This is a trick  to avoid the m_rtCurrentMsgCyc increace twice
		if((regStatusValue&0X0010) || (blockData&0X0040))
		{
			RTEndMsg();
			m_rtCurrentMsgCyc --;
		}
	}
	else if(m_rtCurrentMsgCyc == 1)
	{
		
	}
	return 0;
}
UINT16 SimRT::RTModeCodeWithDataTransferRX()
{
	UINT16 cmd = m_rtCurrentMsgCmdWord;
	UINT16 address=cmd>>11;
	UINT16 wordCount_modeCode=cmd&0X001F;
	UINT16 t_r=cmd&0X0400;
	UINT16 timeRegValue = Reg::timeTagReg;

	UINT16 regDataForConfig3 = Reg::configReg_3;

	UINT16 regStatusValue = Reg::rtStatusWordReg;
	UINT16 blockData = memRead(m_rtMsgBlockAddr);

	UINT16 isEnhanceMode = regDataForConfig3&0X8000;
	UINT16 isEnhanceModeCodeHandle = regDataForConfig3&0X0001;


	if((regStatusValue&0X0008 && regDataForConfig3&0x8008) || (blockData&0X0040 && regDataForConfig3&0X8010)) //If busy and busy store disbaled; invalid command and invalid command store disabled
	{
		//Will not Store
	}
	else
	{
		//RT will recv the data word
		if(m_rtCurrentMsgCyc == 1) 
		{
			UINT16 busData[4];
			UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
			//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
			if (retValue) //Recv is null
			{
				//Report a error!
				rtRecvTimeout();
				return 1;
			}
			else if (busData[0] != DATA_TYPE_DATA_WORD) //Is not a data word
			{	//wordParse(rtDescription,busData[0],busData[2],busData[1]);
				//Report a error
				rtRecvImproperWord(busData[0],DATA_TYPE_DATA_WORD,busData[1]);
				return 1;
				
			}   
			//wordParse(rtDescription,busData[0],busData[2],busData[1]);
			UINT16 currentDataWord = busData[1];

			if (isEnhanceMode && isEnhanceModeCodeHandle)
			{
				if (address == 31)
				{
					memWrite((UINT16)(RT_MODECODE_DATA_BCAST_ADDR + (wordCount_modeCode&0X0007)),currentDataWord);       
				}
				else
				{
					memWrite((UINT16)(RT_MODECODE_DATA_RECV_ADDR + (wordCount_modeCode&0X0007)),currentDataWord);   
				}
				memWrite(m_rtMsgBlockAddr + 2,currentDataWord);

			}
			else
			{
				UINT16 stackAddr = Reg::rtDataStackAddrReg;
				memWrite(stackAddr,currentDataWord);
				
			}
		}

	}
	if(m_rtCurrentMsgCyc == 1) 
	{
		m_rtCurrentMsgCyc++;
		//RTSingleWordTransfer();
		if(m_rtCurrentMsgCyc == 2 && address != 31)
		{
			RTReturnStatusWord();
			//m_rtCurrentMsgCyc --;
		}
	}
	/*else if(m_rtCurrentMsgCyc == 2 && address != 31)
	{
		RTReturnStatusWord();
		m_rtCurrentMsgCyc --; //This is a trick  to avoid the m_rtCurrentMsgCyc increace twice
	}*/
	return 0;
}

UINT16 SimRT::RTModeCodeWithOutDataTransfer()
{
	UINT16 cmd = m_rtCurrentMsgCmdWord;
	UINT16 address=cmd>>11;
	UINT16 wordCount_modeCode=cmd&0X001F;
	UINT16 t_r=cmd&0X0400;
	UINT16 timeRegValue = Reg::timeTagReg;
	UINT16 blockData = memRead(m_rtMsgBlockAddr);

	UINT16 stackAddr = Reg::rtDataStackAddrReg;
	UINT16 regData = Reg::rtStatusWordReg;
	if (!m_rtCurrentMsgCyc) 
	{
		if (wordCount_modeCode == 2)//Transmit Status Word
		{

		}
		else
		{
			if ((regData&0X0010) || (blockData&0X00400))
			{
				switch(wordCount_modeCode)
				{
				case 0://Dynamic Bus Control
					break;
				case 1://Synchronize
					break;
				case 3://Initiate Self Test
					break;
				case 4://Transmitter Shutdown
					break;
				case 5://Override Transmitter Shutdown
					break;
				case 6://Inhibit Terminal Flag
					break;
				case 7://Override Inhibit Terminal Flag
					break;
				case 8://Reset Remote Terminal
					break;

				}
			}
		}
// 		if (address != 31)
// 		{
// 			m_rtCurrentMsgCyc ++;
// 			RTSingleWordTransfer();
// 		}
		
	}
	if(m_rtCurrentMsgCyc == 0 && address != 31)
	{
		RTReturnStatusWord();
		m_rtCurrentMsgCyc++;
		
	}
	return 0;
}

UINT16 SimRT::RTTXTransfer()
{



	UINT16 cmd = m_rtCurrentMsgCmdWord;
	UINT16 subAddress=(cmd>>5)&0X001F;
	UINT16 time = Reg::timeTagReg;
	if (!m_rtCurrentMsgCyc)
	{
		m_rtCurrentMsgCyc ++;
		//RTSingleWordTransfer();
		RTReturnStatusWord();
		if ((Reg::rtStatusWordReg&0X0008) || (memRead(m_rtMsgBlockAddr)&0X0040))//if illegalized or busy, data will not transfer
		{
			RTEndMsg();
			m_rtCurrentMsgCyc = -1;
		}
	}
	else if (m_rtCurrentMsgCyc == 1) 
	{
		//RTReturnStatusWord();
		//m_rtCurrentMsgCyc --; //This is a trick  to avoid the m_rtCurrentMsgCyc increace twice
		
	}   
	else if(m_rtCurrentMsgCyc > 1 && m_rtCurrentMsgCyc < m_rtCurrentMsgCycCount)
	{
		
		if (!(Reg::rtStatusWordReg&0X0400) && !(memRead(m_rtMsgBlockAddr)&0X0040))
		{
			UINT16 stackAddr = Reg::rtDataStackAddrReg;
			UINT16 data =  memRead(stackAddr+ m_rtDataIndex);

			llogInfo("Trans","0x%x:0x%x",stackAddr + m_rtDataIndex,data);
			
			//wordParse(rtDescription,DATA_TYPE_DATA_WORD,time,data);
			if (Reg::rtBITWordReg & 0X0100) {
				
				m_busChannelB.setBusData(DATA_TYPE_DATA_WORD,time,data,1);
			}
			else 
			{
				m_busChannelA.setBusData(DATA_TYPE_DATA_WORD,time,data,1);
			}
			

		}
		m_rtDataIndex++;
	}



	return 0;
}
UINT16 SimRT::RTRXTransfer()
{
	UINT16 cmd = m_rtCurrentMsgCmdWord;

	UINT16 address=cmd>>11;
	
	UINT16 regData = Reg::rtStatusWordReg;
	UINT16 regDataForConfig3 = Reg::configReg_3;
	UINT16 blockData = memRead(m_rtMsgBlockAddr);

	UINT16 busData[4];
	if (m_rtCurrentMsgCyc > 0 &&((( m_rtCurrentMsgCyc < m_rtCurrentMsgCycCount - 1 && address == m_rtAddress)) || (address == 31 &&  m_rtCurrentMsgCyc < m_rtCurrentMsgCycCount)))  //RT in date word CYC
	{
		if((regData&0X0008 && regDataForConfig3&0x8008) || (blockData&0X0040 && regDataForConfig3&0X8010))//Busy or invalid
		{
			UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
			//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
			if (!retValue)
			{
				//wordParse(rtDescription,busData[0],busData[2],busData[1]);
				if(busData[0] == DATA_TYPE_COMMAND_WORD)
				{
					UINT16 commandAddress = busData[1] >> 11;
					if( commandAddress == m_rtAddress || commandAddress == 31)
					{
						m_rtCurrentMsgCmdWord = busData[1];
						m_rtProcessedMsgCount --;//Trick
						RTEndMsg();
						RTReceiveCMD();    //Reinit the CYC count
						RTStartMsg();
						RTSingleWordTransfer();
						m_rtCurrentMsgCyc --;
					}
					else if (m_rtCurrentMsgCyc == 1)
					{
						m_rtCurrentMsgCycCount += 2;
						blockData |= 0x0800; //RT2RT format
						memWrite(m_rtMsgBlockAddr,blockData);
					}
				}
			}
		}
		else
		{
			UINT16 retValue = CheckRecvHook(4*sizeof(UINT16),&busData);
			//llogDebug("EXTERN FUNCTION CALL","CheckRecv(%u,0x%x) = %u",4*sizeof(UINT16),&busData,retValue);
			
			if (!retValue)
			{
				//wordParse(rtDescription,busData[0],busData[2],busData[1]);
				if(busData[0] == DATA_TYPE_COMMAND_WORD)//RT2RT transfer, and this is the second command:Trans command
				{
					UINT16 commandAddress = busData[1] >> 11;
					if( commandAddress == m_rtAddress || commandAddress == 31)
					{
						m_rtCurrentMsgCmdWord = busData[1];
						m_rtProcessedMsgCount --; //Trick
						RTEndMsg();
						RTReceiveCMD();    //Reinit the CYC count
						RTStartMsg();
						RTSingleWordTransfer();
						m_rtCurrentMsgCyc --;
					}
					else
					{
						m_rtCurrentMsgCycCount += 2; //Increase the m_rtCurrentMsgCycCount, NOT the m_rtCurrentMsgCyc!!! Transmit command and Transmit status word
						
						blockData |= 0x0800; //RT2RT format
						memWrite(m_rtMsgBlockAddr,blockData);
						
						
					}
				}
				else if(busData[0] == DATA_TYPE_STATUS_WORD /*&& m_rtCurrentMsgCyc == 1 && address == 31*/)//BroadCast Transmit command status
				{
					
				}
				else if(busData[0] == DATA_TYPE_DATA_WORD)//Current word is data word, and the cyc is bwtween 1 and rtCurrentMsgCyccount - 1 (if RTRT, cyc is between 3 and rtCurrentMsgCyccount - 1)
				{
					UINT16 dataWord = busData[1];
					
					UINT16 stackAddr = Reg::rtDataStackAddrReg;
					llogInfo("Recv","0x%x:0x%x",stackAddr + m_rtDataIndex ,dataWord);

					memWrite(stackAddr + m_rtDataIndex,dataWord);
					
					m_rtDataIndex++;
				}
				else 
				{
					rtRecvImproperWord(busData[0],DATA_TYPE_DATA_WORD,busData[1]);
					//return 1;
				}
			}
			else
			{
				//Recv is null
				//Report a error
				rtRecvTimeout();
				//return 1;
			}	
		}
	}
	if (m_rtCurrentMsgCyc == (m_rtCurrentMsgCycCount - 2) && address != 31)
	{
		m_rtCurrentMsgCyc++;
		RTReturnStatusWord();
		//RTSingleWordTransfer();
	}
	/*else if(m_rtCurrentMsgCyc == (m_rtCurrentMsgCycCount - 1) && address != 31)
	{
		RTReturnStatusWord();
		m_rtCurrentMsgCyc --; //This is a trick  to avoid the m_rtCurrentMsgCyc increace twice
	}*/

	return 0;
}
UINT16 SimRT::RTEndMsg()
{
	m_rtCurrentMsgCyc = 0;
	m_rtCurrentMsgCycCount = 0;
	Reg::rtLastCmdReg = m_rtCurrentMsgCmdWord;
	m_rtProcessedMsgCount ++;
	RTUpdateLookupTable(m_rtCurrentMsgCmdWord);
	return 0;
}
UINT16 SimRT::RTCheckCMDType(UINT16 cmdWord)
{
	UINT16 address=cmdWord>>11;
	UINT16 subAddress=(cmdWord>>5)&0X001F;
	UINT16 wordCount_modeCode=cmdWord&0X001F;
	UINT16 t_r=cmdWord&0X0400;
	m_rtCurrentMsgCycCount = 1; //For first command word
	if (!subAddress || subAddress == 31)//Mode code
	{
		if (t_r)
		{
			//with data transmit
			if (wordCount_modeCode>0X0010)
			{
				m_rtCurrentMsgCycCount += 2; //Data word and Status word
			}
			//without data
			else
			{
				m_rtCurrentMsgCycCount += 1;
			}
		}   
		else
		{
			m_rtCurrentMsgCycCount += 2;
		}
	}
	else
	{
		if(wordCount_modeCode==0)
		{
			m_rtCurrentMsgCycCount += (1 + 32);//Data word and Status word
		}
		else
		{
			m_rtCurrentMsgCycCount += (1 + wordCount_modeCode);   
		}
	}

	if(address == 31)
	{
		m_rtCurrentMsgCycCount -= 1; //Remove the status word(Will handle the RTRT(broadcast) transfer in RTRXTransfer function)
	}

	m_rtCurrentMsgCyc = 0;

	return 0;
}

UINT16 SimRT::RTStartMsg()
{
	m_rtDataIndex = 0;
	genIRQ();
	if (Reg::configReg_1 & 0x2000) 
	{
		m_rtMsgBlockAddr = memRead(RT_STACK_POINTER_B_ADDR);
		memWrite(RT_STACK_POINTER_B_ADDR,m_rtMsgBlockAddr + 4);
	}
	else 
	{
		m_rtMsgBlockAddr = memRead(RT_STACK_POINTER_A_ADDR);
		memWrite(RT_STACK_POINTER_A_ADDR,m_rtMsgBlockAddr + 4);
	}
	memWrite(m_rtMsgBlockAddr,0X4000);//Block status word
	Reg::rtStatusWordReg = 0;
	RTLoadSubaddressControlWordAndDataAddress(m_rtCurrentMsgCmdWord);
	memWrite(m_rtMsgBlockAddr + 3,m_rtCurrentMsgCmdWord);
	if (RTLookupIllegalizationTable(m_rtCurrentMsgCmdWord)) 
	{
		//Command is invalid

	}
	if (RTLookupBusyTable(m_rtCurrentMsgCmdWord)) 
	{
		//Subsystem is busy
	}
	
	return 0;
}

UINT16 SimRT::RTLoadSubaddressControlWordAndDataAddress(UINT16 cmdWord)
{

	UINT16 address=cmdWord>>11;
	UINT16 subAddress=(cmdWord>>5)&0X001F;
	UINT16 wordCount_modeCode=cmdWord&0X001F;
	UINT16 t_r=cmdWord&0X0400;
	UINT16 regDataForConfig3 = Reg::configReg_3;
	UINT16 isEnhanceMemManage = Reg::configReg_2&0X02;
	UINT16 isEnhanceMode = regDataForConfig3&0X8000;

	bool isEnhanceModeCodeHandle =isEnhanceMode &&(Reg::configReg_3 & 0x0001);
	bool isModeCode = subAddress == 0 || subAddress == 31;
	bool isModeCodeWithoutData = isModeCode && ((cmdWord&0X1F) <= 8);

	if (isEnhanceMemManage && !(isEnhanceModeCodeHandle && isEnhanceMode && isModeCode))//Load RT control word, if condition is true
	{   
		if (Reg::configReg_1 & 0x2000)
		{
			Reg::rtSubAddrCtrlWordReg = memRead((UINT16)(RT_CONTROLWORD_LOOKUP_TABLE_B_ADDR + subAddress));
		}
		else
		{
			Reg::rtSubAddrCtrlWordReg = memRead((UINT16)(RT_CONTROLWORD_LOOKUP_TABLE_A_ADDR + subAddress));
		}
	}

	UINT16 subAddressData = 0;
	if (!(isModeCode && isEnhanceModeCodeHandle && isEnhanceMode) && !isModeCodeWithoutData)
	{
		if (Reg::configReg_1 & 0x2000)
		{
			/*if (address == 31)
			{
				subAddressData = memRead((UINT16)(RT_BCAST_LOOKUP_TABLE_B_ADDR + subAddress));
			}
			else if (t_r)
			{
				subAddressData = memRead((UINT16)(RT_TX_LOOKUP_TABLE_B_ADDR + subAddress));
			}
			else if(!t_r)
			{
				subAddressData = memRead((UINT16)(RT_RX_LOOKUP_TABLE_B_ADDR + subAddress));
			}*/
			subAddressData = memRead((UINT16)(RT_RX_LOOKUP_TABLE_B_ADDR + subAddress));
		}
		else
		{
			/*if (address == 31)
			{
				subAddressData = memRead((UINT16)(RT_BCAST_LOOKUP_TABLE_A_ADDR + subAddress));
			}
			else if (t_r)
			{
				subAddressData = memRead((UINT16)(RT_TX_LOOKUP_TABLE_A_ADDR + subAddress));
			}
			else if(!t_r)
			{
				subAddressData = memRead((UINT16)(RT_RX_LOOKUP_TABLE_A_ADDR + subAddress));
			}*/
			subAddressData = memRead((UINT16)(RT_TX_LOOKUP_TABLE_A_ADDR + subAddress));
		}

		if(!m_rtCurrentMsgCyc)//This is the first init
		{
			memWrite(m_rtMsgBlockAddr + 2,subAddressData);
		}
		Reg::rtDataStackAddrReg = subAddressData;
	}


	return 0;
}

UINT16 SimRT::RTLookupIllegalizationTable(UINT16 cmdWord)
{
	UINT16 address=cmdWord>>11;
	UINT16 subAddress=(cmdWord>>5)&0X001F;
	UINT16 wordCount_modeCode=cmdWord&0X001F;
	UINT16 t_r=cmdWord&0X0400;

	UINT16 illegalAddress=0X0300;


	//not broadcast
	if (address == m_rtAddress)
	{
		illegalAddress |= 0X0080;   
	}
	//t/r
	if (t_r)
	{
		illegalAddress |= 0X0040;
	}
	//sub address
	illegalAddress |= (subAddress<<1);
	//wc/mc
	illegalAddress|=((wordCount_modeCode&0X10) >> 4);

	UINT16 data = memRead(illegalAddress);
	bool isValid = true;
	if((data >> (wordCount_modeCode&0X0F))&0X01)
	{
		isValid = false;
	}

	if (!isValid)
	{
		Reg::rtStatusWordReg |= 0X0400;
		UINT16 blockData = memRead(m_rtMsgBlockAddr);
		blockData |= 0X0040;
		memWrite(m_rtMsgBlockAddr,blockData);
		return 1;
	}
	return 0;       
}
UINT16 SimRT::RTLookupBusyTable(UINT16 cmdWord)
{
	if(!(Reg::configReg_1 & 0x0400))
	{
		Reg::rtStatusWordReg |= 0X0008;
		return 1;
	}
	UINT16 address=cmdWord>>11;
	UINT16 subAddress=(cmdWord>>5)&0X001F;
	UINT16 sa4 = subAddress >> 4;
	UINT16 wordCount_modeCode=cmdWord&0X001F;
	UINT16 t_r=(cmdWord&0X0400) >> 9;
	UINT16 baseAddress = 0X0240;
	if (address == 31)
	{
		baseAddress|=0X0004;
	}
	baseAddress |= t_r;

	baseAddress |= sa4;


	UINT16 busyAddress=baseAddress;
	UINT16 busyEntry = memRead(busyAddress);


	if((busyEntry >> (subAddress&0X0F))&0X01)
	{
		Reg::rtStatusWordReg |= 0X0008;
		return 1;
	}   
	return 0;

}



UINT16 SimRT::RTUpdateLookupTable(UINT16 cmd)
{

	
	UINT16 address = cmd >> 11;
	UINT16 subAddress=(cmd >> 5)&0X001F;

	UINT16 t_r=cmd&0X0400;

	UINT16 stackAddr = Reg::rtDataStackAddrReg;
	UINT16 stackInitAddr = memRead(m_rtMsgBlockAddr + 2);

	UINT16 regDataForConfig3 = Reg::configReg_3;
	UINT16 isEnhanceMemManage = Reg::configReg_2&0X02;
	UINT16 isEnhanceMode = regDataForConfig3&0X8000;
	
	UINT16 isEnhanceModeCodeHandle = regDataForConfig3&0X01;
	UINT16 isModeCode = subAddress == 0 || subAddress == 31;
	UINT16 isModeCodeWithoutData = isModeCode || (cmd&0X1F) <= 8;
	UINT16 isSaDoubleBuffer = Reg::configReg_2&0X1000;

	if (isEnhanceMode)
	{
		if (isEnhanceMemManage && isEnhanceModeCodeHandle) //No need to update the subaddress table
		{

		}
		else if (address == 31)
		{
			if (isEnhanceMemManage && (Reg::rtSubAddrCtrlWordReg&0X0007) >= 1) //circle buff
			{

				if((stackAddr - stackInitAddr) == (128 - 32))
				{
					//RT circular buffer rollover, gen a int
					Reg::intStatusReg |= 0X0020;/////////////
					stackAddr = stackInitAddr ;
				}
				else
				{
					stackAddr += 32;
				}
				Reg::rtDataStackAddrReg = stackAddr;
				if (Reg::configReg_1 & 0x2000)
				{
					memWrite((UINT16)(RT_BCAST_LOOKUP_TABLE_B_ADDR + subAddress),stackAddr);               
				}
				else
				{
					memWrite((UINT16)(RT_BCAST_LOOKUP_TABLE_A_ADDR + subAddress),stackAddr);   
				}

			}
			else if ((isEnhanceMemManage && isSaDoubleBuffer && (Reg::rtSubAddrCtrlWordReg&0X8007) == 0X8000) || isSaDoubleBuffer)
			{//double buffer, and the last condition is to check if global double buffer
				stackAddr ^= 0X0020;
				Reg::rtDataStackAddrReg = stackAddr;
				memWrite(m_rtMsgBlockAddr + 2,stackAddr);
				if (Reg::configReg_1 & 0x2000)
				{
					memWrite((UINT16)(RT_BCAST_LOOKUP_TABLE_B_ADDR + subAddress),stackAddr);   
				}
				else
				{
					memWrite((UINT16)(RT_BCAST_LOOKUP_TABLE_A_ADDR + subAddress),stackAddr);
				}
			}       
		}
		else if (t_r)
		{       
			if(isEnhanceMemManage && ((Reg::rtSubAddrCtrlWordReg&0X1C00) >> 10) >= 1) //circle buff
			{
				llogInfo("RTUpdateLookupTable","Trans::Circle buff");
				if((stackAddr - stackInitAddr) == (128 - 32))
				{
					Reg::intStatusReg |= 0X0020;
					stackAddr = stackInitAddr;
				}
				else
				{
					stackAddr += 32;
				}

				Reg::rtDataStackAddrReg = stackAddr;

				if (Reg::configReg_1 & 0x2000)
				{
					memWrite((UINT16)(RT_TX_LOOKUP_TABLE_B_ADDR + subAddress),stackAddr);               
				}
				else
				{
					memWrite((UINT16)(RT_TX_LOOKUP_TABLE_A_ADDR + subAddress),stackAddr);   
				}
			}
		}
		else if(!t_r)
		{

			if (isEnhanceMemManage && ((Reg::rtSubAddrCtrlWordReg&0X00E0) >> 5 ) >= 1) //circle buff
			{
				llogInfo("RTUpdateLookupTable","Recv::Circle buff");

				if((stackAddr - stackInitAddr) == (128 - 32))
				{
					Reg::intStatusReg |= 0X0020;
					stackAddr = stackInitAddr;
				}
				else
				{
					stackAddr += 32;
				}

				Reg::rtDataStackAddrReg = stackAddr;


				if (Reg::configReg_1 & 0x2000)
				{
					memWrite((UINT16)(RT_RX_LOOKUP_TABLE_B_ADDR + subAddress),stackAddr);               
				}
				else
				{
					memWrite((UINT16)(RT_RX_LOOKUP_TABLE_A_ADDR + subAddress),stackAddr);   
				}
			}
			else if ((isEnhanceMemManage && isSaDoubleBuffer && (Reg::rtSubAddrCtrlWordReg&0X80E0) == 0X8000) || isSaDoubleBuffer) //double buffer
			{

				llogInfo("RTUpdateLookupTable","Recv::Double buff");
				stackAddr ^= 0X0020;
				Reg::rtDataStackAddrReg = stackAddr;
				memWrite(m_rtMsgBlockAddr + 2,stackAddr);
				if (Reg::configReg_1 & 0x2000)
				{
					memWrite((UINT16)(RT_RX_LOOKUP_TABLE_B_ADDR + subAddress),stackAddr);   
				}
				else
				{
					memWrite((UINT16)(RT_RX_LOOKUP_TABLE_A_ADDR + subAddress),stackAddr);
				}

			}   
		}

	}

	return 0;
}

UINT16 SimRT::RTReturnStatusWord()
{
	//llogDebug("RT","RT 0x%02x return Status Word",m_rtAddress);
	genIRQ();
	UINT16 cmdWord = m_rtCurrentMsgCmdWord;
	UINT16 address=cmdWord>>11;
	UINT16 subAddress=(cmdWord>>5)&0X001F;
	UINT16 wordCount_modeCode=cmdWord&0X001F;
	UINT16 t_r=cmdWord&0X0400;

	UINT16 statusRegValue = Reg::rtStatusWordReg;
	UINT16 transData;
	UINT16 time = Reg::timeTagReg;
	if (subAddress == 31 || subAddress == 0)
	{
		if (wordCount_modeCode == 2 || (wordCount_modeCode == 18 && !(statusRegValue&0X0408)))//Transmit Last Command Word   Transmit Status Word
		{
			transData = m_mLastStatusReg;
			Reg::rtStatusWordReg = transData;

		}
		else
		{
			transData = statusRegValue;
		}
	}
	else
	{
		transData = statusRegValue;
	}
	transData &= 0x07ff;
	transData |= (m_rtAddress << 11);
	//wordParse(rtDescription,DATA_TYPE_STATUS_WORD,time,transData);
	if (Reg::rtBITWordReg & 0X0100) 
	{
		m_busChannelB.setBusData(DATA_TYPE_STATUS_WORD,time,transData,1);
	}
	else 
	{
		m_busChannelA.setBusData(DATA_TYPE_STATUS_WORD,time,transData,1);
	}
	m_mLastStatusReg = transData;

	return 0;
}

UINT32 SimRT::OnData(UINT32 len, void * data)
{
	if(len < (4 + 32) * sizeof(UINT16)) {
		return 2;//Insufficient Buffer size
	}
	//Current channel is B
	if (Reg::rtBITWordReg & 0X0100) 
	{
		return m_busChannelB.OnData(len, data);
	}
	else {
		return m_busChannelA.OnData(len, data);
	}
	return 0;
}

UINT16 SimRT::rtRecvImproperWord(UINT16 actualType,UINT16 expectType,UINT16 data)
{
	//llogDebug("RT","Receive Improper Word");
	genIRQ();
	if (actualType == DATA_TYPE_COMMAND_WORD)
	{
		UINT16 commandAddress = data >> 11;
		if( commandAddress == m_rtAddress || commandAddress == 31)
		{
			m_rtCurrentMsgCmdWord = data;
			RTEndMsg();
			RTReceiveCMD();    //Reinit the CYC count
			RTStartMsg();
			RTSingleWordTransfer();
			m_rtCurrentMsgCyc -- ;
			
			return 0;
		}
	}
	


	Reg::rtStatusWordReg |= 0x0400;
	UINT16 blockData = memRead(m_rtMsgBlockAddr);
	blockData |= 0x1000;
	//m_rtCurrentMsgCyc = 0;
	memWrite(m_rtMsgBlockAddr,blockData);
	return 0;
}
UINT16 SimRT::rtRecvTimeout()
{
	//llogDebug("RT","Receive Time out");
	genIRQ();
	Reg::rtStatusWordReg |= 0x0400;
	UINT16 blockData = memRead(m_rtMsgBlockAddr);
	blockData |= 0x1200;
	//m_rtCurrentMsgCyc = 0;
	memWrite(m_rtMsgBlockAddr,blockData);
	return 0;
}


UINT16 SimRT::initForTest()
{
	//Software reset 
	Reg::startResetReg = 0x0001;
	//Set enhance mode
	Reg::configReg_3 = 0x8000;
	
	Reg::configReg_1 = 0x0400;
	for(int i = 0; i < MEMSIZE; i++)
	{
		Mem::mem[i] = 0;
	}
	Mem::mem[RT_STACK_POINTER_A_ADDR] = 0;//Command Stack pointer A
	Mem::mem[RT_RX_LOOKUP_TABLE_A_ADDR + 0] = 0x0260;//Subaddress 0
	Mem::mem[RT_RX_LOOKUP_TABLE_A_ADDR + 1] = 0x0400;
	Mem::mem[RT_RX_LOOKUP_TABLE_A_ADDR + 2] = 0x0500;
	Mem::mem[RT_RX_LOOKUP_TABLE_A_ADDR + 3] = 0x0600;


	Mem::mem[RT_TX_LOOKUP_TABLE_A_ADDR + 0] = 0x0700;//Subaddress 0
	Mem::mem[RT_TX_LOOKUP_TABLE_A_ADDR + 1] = 0x0800;
	Mem::mem[RT_TX_LOOKUP_TABLE_A_ADDR + 2] = 0x0900;
	Mem::mem[RT_TX_LOOKUP_TABLE_A_ADDR + 3] = 0x0a00;


	Mem::mem[RT_BCAST_LOOKUP_TABLE_A_ADDR + 0] = 0x0b00;//Subaddress 0
	Mem::mem[RT_BCAST_LOOKUP_TABLE_A_ADDR + 1] = 0x0c00;
	Mem::mem[RT_BCAST_LOOKUP_TABLE_A_ADDR + 2] = 0x0d00;
	Mem::mem[RT_BCAST_LOOKUP_TABLE_A_ADDR + 3] = 0x0e00;
	
	Reg::isHalted = false;


	return 0;
}
UINT16 SimRT::configReg_5_write(UINT16 data)
{
	//Reg::configReg_5_write(data);
	data &= 0xffc0;
	UINT16 RTaddrP = ((m_rtAddress>>4) & 0x1)   + ((m_rtAddress>>3) & 0x01) + ((m_rtAddress>>2) & 0x01) + ((m_rtAddress>>1) & 0x01) + (m_rtAddress & 0x01);
	if (RTaddrP % 2)
	{
		data |= (m_rtAddress<<1);
	}
	else
	{
		data |= ((m_rtAddress<<1) | 0x01);
	}
	Reg::configReg_5_write(data);
	return 0;
}

struct TransException SimRT::checkIfException()
{
	struct TransException transExcep = {0,0,TimeOutException,FALSE};

	UINT16 msgCount = m_rtProcessedMsgCount + 1;
	for (UINT16 i = 0; i < 64; i++)
	{
		struct TransException tempExcep = Exception1553B::m_exceptionArray[i];
		if (tempExcep.isError && tempExcep.messageIndex == msgCount)
		{
			
			if(tempExcep.messageCycCount == m_rtCurrentMsgCyc) 
			{
				transExcep = tempExcep;
				Exception1553B::m_exceptionArray[i].isError = FALSE;
				break;
			}
			
		}
	}

	return transExcep;
}

UINT32 SimRT::CheckRecvHook(UINT32 len,void *recvData)
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

void SimRT::genIRQ(void)
{
	GenIRQ();
}
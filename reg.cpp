#include "stdafx.h"

#include "reg.h"
UINT16 Reg::clearRegs(bool isInit)
{
	for(int i = 0; i < 16; i++) {
		if (!isInit && i == START_RESET_REG_W_ADDR) {
			continue;
		}
		else if(!regWrite[i]) {
			//Register does not exist
			continue;
		}
		(this->*regWrite[i])(0);
	}
	return 0;
}
UINT16 Reg::regReadFromAddr(UINT16 addr)
{
	if(addr > 15) {
		llogError("Register","Register 0x%x does not exist!",addr);
		return 0;
	}
	else if(!regRead[addr]) {
		llogError("Register","Register 0x%x is not readable!",addr);
		return 0;
	}
	return (this->*regRead[addr])();
	
}

UINT16 Reg::regWriteToAddr(UINT16 addr, UINT16 data)
{
	if(addr > 15) {
		llogError("Register","Register 0x%x does not exist!",addr);
		return 1;
	}
	else if(!regWrite[addr]) {
		llogError("Register","Register 0x%x is not writeable!",addr);
		return 1;
	}
	return (this->*regWrite[addr])(data);
}

UINT16 Reg::intMaskReg_read(void)
{
	return intMaskReg; 
}
UINT16 Reg::intMaskReg_write(UINT16 data)
{
	intMaskReg = data;
	return 0;
}


UINT16 Reg::configReg_1_read(void)
{
	return configReg_1;
}
UINT16 Reg::configReg_1_write(UINT16 data)
{
	configReg_1 = data;
	if (configReg_3 & 0x8000)
	{
		if ((data & 0xC000) == 0x4000 || (data & 0xC000) == 0x8000)
		{
			isHalted = false;
		}
	}
	return 0;
}



UINT16 Reg::configReg_2_read(void)
{
	return configReg_2;
}
UINT16 Reg::configReg_2_write(UINT16 data)
{
	configReg_2 = data;
	return 0;
}


UINT16 Reg::cmdStackPtrReg_read(void)
{
	return cmdStackPtrReg;
}

UINT16 Reg::startResetReg_write(UINT16 data)
{
	startResetReg = data;
	if (data & 0X0002)
	{
		if (!(configReg_1 & 0XC000)) // BC mode
		{
			isHalted = false;
		}
		else if ((configReg_1 & 0XC000) == 0X4000 && !(configReg_3  & 0x8000))
		{
			isHalted = false;
		}
		
	}
	else if (data & 0X0001)//Reset
	{
		isHalted = true;
	}
	return 0;	
}


UINT16 Reg::bcCtrlWordReg_read(void)
{
	return bcCtrlWordReg;
}
UINT16 Reg::bcCtrlWordReg_write(UINT16 data)
{
	bcCtrlWordReg = data;
	return 0;
}

UINT16 Reg::rtSubAddrCtrlWordReg_read(void)
{
	return rtSubAddrCtrlWordReg;
}
UINT16 Reg::rtSubAddrCtrlWordReg_write(UINT16 data)
{
	rtSubAddrCtrlWordReg = data;
	return 0;
}


UINT16 Reg::timeTagReg_read(void)
{
	return timeTagReg;
}
UINT16 Reg::timeTagReg_write(UINT16 data)
{
	timeTagReg = data;
	return 0;
}


UINT16 Reg::intStatusReg_read(void)
{
	return intStatusReg;
}


UINT16 Reg::configReg_3_read(void)
{
	return configReg_3;
}
UINT16 Reg::configReg_3_write(UINT16 data)
{
	configReg_3 = data;
	return 0;
}


UINT16 Reg::configReg_4_read(void)
{
	return configReg_4;
}
UINT16 Reg::configReg_4_write(UINT16 data)
{
	configReg_4 = data;
	return 0;
}


UINT16 Reg::configReg_5_read(void)
{
	return configReg_5;
}
UINT16 Reg::configReg_5_write(UINT16 data)
{
	configReg_5 = data;
	return 0;
}


UINT16 Reg::rtDataStackAddrReg_read(void)
{
	return rtDataStackAddrReg;
}
UINT16 Reg::rtDataStackAddrReg_write(UINT16 data)
{
	rtDataStackAddrReg = data;
	return 0;
}

UINT16 Reg::mtDataStackAddrReg_read(void)
{
	return mtDataStackAddrReg;
}
UINT16 Reg::mtDataStackAddrReg_write(UINT16 data)
{
	mtDataStackAddrReg = data;
	return 0;
}


UINT16 Reg::bcFrameTimeRemainReg_read(void)
{
	return bcFrameTimeRemainReg;
}


UINT16 Reg::bcMsgTimeRemainReg_read(void)
{
	return bcMsgTimeRemainReg;
}


UINT16 Reg::bcFrameTimeReg_read(void)
{
	return bcFrameTimeReg;
}
UINT16 Reg::bcFrameTimeReg_write(UINT16 data)
{
	bcFrameTimeReg = data;
	return 0;
}

UINT16 Reg::rtLastCmdReg_read(void)
{
	return rtLastCmdReg;
}
UINT16 Reg::rtLastCmdReg_write(UINT16 data)
{
	rtLastCmdReg = data;
	return 0;
}

UINT16 Reg::mtTriggerWordReg_read(void)
{
	return mtTriggerWordReg;
}
UINT16 Reg::mtTriggerWordReg_write(UINT16 data)
{
	mtTriggerWordReg = data;
	return 0;
}


UINT16 Reg::rtStatusWordReg_read(void)
{
	return rtStatusWordReg;
}


UINT16 Reg::rtBITWordReg_read(void)
{
	return rtBITWordReg;
}


UINT16 Reg::updateTimeReg()
{
	UINT16 resolution;
	switch ((Reg::configReg_2 & 0X0380)>>7)
	{
	case 0X000://64us
		resolution = 64;
		break;
	case 0X001://32us
		resolution = 32;
		break;
	case 0X010://16us
		resolution = 16;
		break;
	case 0X011://8us
		resolution = 8;
		break;
	case 0X100://4us
		resolution = 4;
		break;
	case 0X101://2us
		resolution = 2;
		break;
	default:
		resolution = 0;
		break;
	}
	if (resolution)
	{
		Reg::timeTagReg = Reg::timeRecorder / resolution;
		return 0;
	}
	return 1;

}
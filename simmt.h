#ifndef _SIMMT_H
#define _SIMMT_H

#include "reg.h"
#include "mem.h"
#include "simbus.h"
#define MT_STACK__ADDR 0
#define MT_STACK_LEN 0X1000

#define MT_STACK_POINTER 0X0100

class SimMT:public Reg, public Mem
{
private:
	Bus busChannelA;
	Bus busChannelB;
public:
	SimMT(void);
	SimMT(char *filename);
	virtual UINT16 memDump();
	virtual INT16 initRegAddress(void);
	void mtStep(void);
	UINT32 OnData(UINT32, void *);
};
#endif
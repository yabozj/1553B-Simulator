#ifndef _SIMMT_H
#define _SIMMT_H

#include "reg.h"
#include "mem.h"
#include "simbus.h"
#include "transException.h"
#define MT_STACK__ADDR 0
#define MT_STACK_LEN 0X1000

#define MT_STACK_POINTER 0X0100

class SimMT:public Reg, public Mem, public Exception1553B
{
private:
	SimMT *m_mtBackUp;
	Bus busChannelA;
	Bus busChannelB;
	UINT16 m_pointer;
public:
	SimMT(void);
	SimMT(char *filename);
	~SimMT();
	void mtRestore();
	void mtSave();
	virtual UINT16 memDump();
	UINT16 mtDump(int len, void *buffAddr);
	virtual INT16 initRegAddress(void);
	virtual void genIRQ(void);
	void mtStep(void);
	UINT32 OnData(UINT32, void *);
	struct TransException checkIfException();
	UINT32 CheckRecvHook(UINT32 len,void *recvData);
};
#endif
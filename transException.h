#ifndef _EXCEPTION1553B_H
#define _EXCEPTION1553B_H

#include "definition.h"
class SimMT;
class SimBC;
class SimRT;
enum TransExceptionEnum{
	TimeOutException,
	DataTypeUnMatchException
};
struct TransException 
{
	UINT16 messageIndex;
	UINT16 messageCycCount;
	enum TransExceptionEnum exceptionType;
	BOOL isError;
};



class Exception1553B
{
protected:
	struct TransException m_exceptionArray[64];
// 	friend class SimMT;
// 	friend class SimBC;
// 	friend class SimRT;
public:
	Exception1553B();
	UINT16 addException(struct TransException &excep);
	virtual struct TransException checkIfException()=0;
	virtual UINT32 CheckRecvHook(UINT32 len,void *recvData) = 0;
};

#endif

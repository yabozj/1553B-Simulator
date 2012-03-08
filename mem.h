#ifndef _MEM_H
#define _MEM_H

#include "definition.h"

#define MEMSIZE 4*1024

class Mem
{
protected:
	UINT16 mem[MEMSIZE];
	
public:
	
	virtual UINT16 memRead(UINT16);
	virtual UINT16 memWrite(UINT16, UINT16);
	UINT16 memWriteBlock(UINT16, UINT16 *, UINT16);
	virtual UINT16 memDump() = 0;
};
extern UINT16 loadConfiguration(const char * filePath,bool isInternalTest);
#endif

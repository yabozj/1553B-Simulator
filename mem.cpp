#include "stdafx.h"
#include "mem.h"

UINT16 Mem::memRead(UINT16 addr)
{
	addr &= 0xfff;
	if (addr < MEMSIZE) {
		return mem[addr];
	}
	else {
		llogError("Memory","Memory Address 0x%x Out of bound!",addr);
		return 0;
	}
}

UINT16 Mem::memWrite(UINT16 addr, UINT16 data) 
{
	addr &= 0xfff;
	//llogDebug("Memory","Write 0x%x at address 0x%x",data,addr);
	if (addr < MEMSIZE) {
		mem[addr] = data;
		return 0;
	}
	else {
		llogError("Memory","Memory Address 0x%x Out of bound!",addr);
		return 1;
	}
}

UINT16 Mem::memWriteBlock(UINT16 baseAddr, UINT16 *data, UINT16 len)
{
	baseAddr &= 0xfff;
	if ((baseAddr + len - 1)< MEMSIZE) {
		for (int i = 0; i < len; i++ ) {
			//llogDebug("Memory","Write 0x%x at address 0x%x",baseAddr + i,data[i]);
			mem[baseAddr + i] = data[i];
		}
		return 0;
	}
	else {
		llogError("Memory","Memory Block Base 0x%x , Len 0x%x Out of bound!",baseAddr,len);
		return 1;
	}
}
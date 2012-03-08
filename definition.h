#ifndef _DEFINITION_H
#define _DEFINITION_H
#include "log.h"
#ifndef UINT8
typedef unsigned char UINT8;
#endif

#ifndef INT8
typedef signed char INT8;
#endif

#ifndef UINT16
typedef unsigned short UINT16;
#endif

#ifndef INT16
typedef short INT16;
#endif

#ifndef UINT32
typedef unsigned int UINT32;
#endif

#ifndef INT32
typedef int INT32;
#endif



#ifdef WIN32  
#include <windows.h>
#ifndef UINT64
typedef unsigned __int64 UINT64;
#endif
#ifndef INT64
typedef __int64 INT64;
#endif
#else
#define _TCHAR char
#define _tmain main
#ifndef UINT64
#define UINT64 unsigned long long
#endif
#ifndef INT64
#define INT64 signed long long
#endif
#ifndef BOOL
typedef int BOOL;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
#endif


#ifndef NULL
#define NULL 0
#endif

typedef void (*sim61580irq)(); 
typedef UINT32 (*pfun_RecvCheck)(UINT32 len,void *recvData);

typedef struct {
	UINT16  reg[0x10];
	UINT16  mem[0x1000];
} bu61580_sharemem_struct;
#endif
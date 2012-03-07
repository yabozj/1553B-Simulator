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
#endif


#ifndef NULL
#define NULL 0
#endif

typedef void (*sim61580irq)(); 
typedef UINT32 (*pfun_RecvCheck)(UINT32 len,void *recvData);


#endif
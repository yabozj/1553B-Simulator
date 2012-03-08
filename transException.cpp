#include "stdafx.h"
#include "transException.h"

Exception1553B::Exception1553B()
{
	for (UINT16 i = 0; i < 64; i++)
	{
		m_exceptionArray[i].isError = FALSE;
	}
}
UINT16 Exception1553B::addException(struct TransException &excep)
{
	UINT16 i = 0;
	for (; i < 64; i++)
	{
		if (!m_exceptionArray[i].isError)
		{
			break;
		}
	}
	if (i < 64)
	{
		m_exceptionArray[i] = excep;
		return 0;
	}
	
	return 1;
}


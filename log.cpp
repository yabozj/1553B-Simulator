#include "stdafx.h"

#include "log.h"
#include <stdarg.h>
static const char* levelBuff[5] = {"DEBUG:","INFO:","WARN:","ERROR:","FATAL"};
Log::Log()
{
	index = 0;
}
Log::Log(Log& log)
{
	index = 0;
}
Log* Log::logPtr = NULL;
bool Log::loglog(const char *tag,enum LogLevel level,const char *message)
{
	
	if (level >= _LOG_LEVEL_) 
	{
		
		if (_LOG_WRITE_FILE_)
		{
			char logName[128] = {0};
			sprintf(logName,"61580Log%d.log",index);
			FILE *logFile = fopen(logName,"a");
			if (!logFile)
			{
				return false;
			}
			fprintf(logFile,"%s##%s##%s\n",levelBuff[level],tag,message);
			fclose(logFile);
		}
		else
		{
			printf("%s##%s##%s\n",levelBuff[level],tag,message);
		}

		return true;
	}
	return false;

}



Log* Log::sharedLog()
{
	if (!logPtr)
	{
		 logPtr = new Log;
	}
	return logPtr;
}

bool Log::logFatal(const char *tag,const char *format, ...)
{
	char buff[512] = {0};
	va_list args;
	va_start(args, format);
	vsprintf(buff,format,args);
	va_end(args);
	return loglog(tag,__FATAL__,buff);


}
bool Log::logError(const char *tag,const char *format, ...)
{
	char buff[512] = {0};
	va_list args;
	va_start(args, format);
	vsprintf(buff,format,args);
	va_end(args);
	return loglog(tag,__ERROR__,buff);
}
bool Log::logWarn(const char *tag,const char *format, ...)
{
	char buff[512] = {0};
	va_list args;
	va_start(args, format);
	vsprintf(buff,format,args);
	va_end(args);
	return loglog(tag,__WARN__,buff);
}
bool Log::logInfo(const char *tag,const char *format, ...)
{
	char buff[512] = {0};
	va_list args;
	va_start(args, format);
	vsprintf(buff,format,args);
	va_end(args);
	return loglog(tag,__INFO__,buff);
}
bool Log::logDebug(const char *tag,const char *format, ...)
{
	char buff[512] = {0};
	va_list args;
	va_start(args, format);
	vsprintf(buff,format,args);
	va_end(args);
	return loglog(tag,__DEBUG__,buff);
}

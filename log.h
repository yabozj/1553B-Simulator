#ifndef _LOG_H
#define _LOG_H


enum LogLevel{__DEBUG__,__INFO__,__WARN__,__ERROR__,__FATAL__};

#ifdef _LOG_LEVEL_DEBUG_
#   define _LOG_LEVEL_ __DEBUG__
#elif   defined  _LOG_LEVEL_INFO_
#   define _LOG_LEVEL_ __INFO__
#elif   defined  _LOG_LEVEL_WARN_
#   define _LOG_LEVEL_ __WARN__
#elif   defined  _LOG_LEVEL_ERROR_
#   define _LOG_LEVEL_ __ERROR__
#elif   defined  _LOG_LEVEL_FATAL_
#   define _LOG_LEVEL_ __FATAL__
#else
#   define _LOG_LEVEL_ __WARN__
#endif


class Log
{
private:
	Log();
	Log(Log&);
	static Log *logPtr;
	bool loglog(const char *tag,enum LogLevel level,const char *message);
	unsigned int index;
public:

	static Log* sharedLog();
	bool logFatal(const char *tag,const char *format, ...);
	bool logError(const char *tag,const char *format, ...);
	bool logWarn(const char *tag,const char *format, ...);
	bool logInfo(const char *tag,const char *format, ...);
	bool logDebug(const char *tag,const char *format, ...);


};

#ifdef _LOG_WRITE_FILE_
#   undef _LOG_WRITE_FILE_
#   define _LOG_WRITE_FILE_ true
#else
#   define _LOG_WRITE_FILE_ false
#endif 



#ifdef __RELEASE__VERSION__
#   define logFatal(tag, ...)
#   define logError(tag, ...)
#   define logWarn(tag, ...)
#   define logInfo(tag, ...)
#   define logDebug(tag, ...)
#else
#   define llogFatal(tag, ...) Log::sharedLog()->logFatal(tag,__VA_ARGS__)
#   define llogError(tag, ...) Log::sharedLog()->logError(tag,__VA_ARGS__)
#   define llogWarn(tag, ...) Log::sharedLog()->logWarn(tag,__VA_ARGS__)
#   define llogInfo(tag, ...) Log::sharedLog()->logInfo(tag,__VA_ARGS__)
#   define llogDebug(tag, ...) Log::sharedLog()->logDebug(tag,__VA_ARGS__)

#endif


#endif
#ifndef _PARSER_H
#define _PARSER_H

#include "simbus.h"

typedef struct  
{
	char *name;
	UINT16 modeCode;
	bool allowBroadcast;
	bool t_r;
} ModeCodeDS;

class WordParser
{
private:
	WordParser();
	WordParser(WordParser&);
	static WordParser *parserPtr;
	
public:
	bool parse(const char * msg,UINT16 type,UINT16 time,UINT16 word);
	static WordParser* sharedParser();
};

#ifdef _LOG_WRITE_FILE_
#   undef _LOG_WRITE_FILE_
#   define _LOG_WRITE_FILE_ YES
#else
#   define _LOG_WRITE_FILE_ NO
#endif 



#ifdef __RELEASE__VERSION__
#   define wordParse(type,word)
#else
#   define wordParse(msg,type,time,word) WordParser::sharedParser()->parse(msg,type,time,word)


#endif


#endif
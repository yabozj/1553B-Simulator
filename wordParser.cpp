#include "stdafx.h"
#include "wordParser.h"

char *modeCodeName[32] = {"Dynamic Bus Control","Synchronize","Transmit Status Word","Initiate Self Test","Transmitter Shutdown",
	"Override Transmitter Shutdown","Inhibit Terminal Flag","Override Inhibit Terminal Flag","Reset Remote Terminal","Reserved",
	"Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Transmit Vector Word","Synchronize (w/data)",
	"Transmit Last Command Word","Transmit BIT Word","Selected Transmitter Shutdown","Override Selected Transmitter Shutdown",
	"Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved","Reserved"};

ModeCodeDS g_modeCodeDS[32] = {
	{modeCodeName[0x00],0x00,false,true},
	{modeCodeName[0x01],0x01,true,true},
	{modeCodeName[0x02],0x02,false,true},
	{modeCodeName[0x03],0x03,true,true},
	{modeCodeName[0x04],0x04,true,true},
	{modeCodeName[0x05],0x05,true,true},
	{modeCodeName[0x06],0x06,true,true},
	{modeCodeName[0x07],0x07,true,true},
	{modeCodeName[0x08],0x08,true,true},
	{modeCodeName[0x09],0x09,false,true},
	{modeCodeName[0x0A],0x0A,false,true},
	{modeCodeName[0x0B],0x0B,false,true},
	{modeCodeName[0x0C],0x0C,false,true},
	{modeCodeName[0x0D],0x0D,false,true},
	{modeCodeName[0x0E],0x0E,false,true},
	{modeCodeName[0x0F],0x0F,false,true},
	{modeCodeName[0x10],0x10,false,true},
	{modeCodeName[0x11],0x11,true,false},
	{modeCodeName[0x12],0x12,false,true},
	{modeCodeName[0x13],0x13,false,true},
	{modeCodeName[0x14],0x14,true,false},
	{modeCodeName[0x15],0x15,true,false},
	{modeCodeName[0x16],0x16,false,false},
	{modeCodeName[0x17],0x17,false,false},
	{modeCodeName[0x18],0x18,false,false},
	{modeCodeName[0x19],0x19,false,false},
	{modeCodeName[0x1A],0x1A,false,false},
	{modeCodeName[0x1B],0x1B,false,false},
	{modeCodeName[0x1C],0x1C,false,false},
	{modeCodeName[0x1D],0x1D,false,false},
	{modeCodeName[0x1E],0x1E,false,false},
	{modeCodeName[0x1F],0x1F,false,false},
	
};

WordParser::WordParser()
{

}
WordParser::WordParser(WordParser& log)
{

}
WordParser* WordParser::parserPtr = NULL;
bool WordParser::parse(const char *msg,UINT16 type,UINT16 time,UINT16 word)
{
	char tagBuff[128] = {0};
	sprintf(tagBuff,"%s:%s","Word Parser",msg);
	if (type == DATA_TYPE_COMMAND_WORD)
	{
		//Parse the command word
		
		UINT16 subAddress=(word>>5)&0X001F;
		UINT16 t_r=(word&0X0400) >> 9;
		UINT16 address=word>>11;
		UINT16 wordCount_modeCode = word&0X001F;
		bool isModeCode = subAddress == 0 || subAddress == 31;
		
		if (isModeCode)
		{
			ModeCodeDS currentDS = g_modeCodeDS[wordCount_modeCode];
			char *t_rString = NULL;
			if (t_r)
			{
				if (currentDS.t_r)
				{
					t_rString = "Transmit";
				}
				else
				{
					t_rString = "Transmit (Should be Receive)";
				}
			}
			else
			{
				if (currentDS.t_r)
				{
					t_rString = "Receive (Should be Transmit)";
				}
				else
				{
					t_rString = "Receive";
				}
			}
			char address_string[32] = {0};

			if (address == 0x1f)
			{
				if (currentDS.allowBroadcast)
				{
					sprintf(address_string,"Broadcast");
				}
				else 
				{
					sprintf(address_string,"Broadcast (May not allowed)");
				}
			}
			else
			{
				sprintf(address_string,"to RT 0x%02x",address);
			}
			
			llogInfo(tagBuff,"Time 0x%04x,Mode Code 0x%02x Command:%s,%s,%s",time,currentDS.modeCode,currentDS.name,t_rString,address_string);
		}
		else
		{
			if (!wordCount_modeCode)
			{
				wordCount_modeCode = 32;
			}
			char countString[32] = {0};
			sprintf(countString,"Word count 0x%02x",wordCount_modeCode);
			char address_string[32] = {0};

			if (address == 0x1f)
			{
				sprintf(address_string,"Broadcast");
			}
			else
			{
				sprintf(address_string,"to RT 0x%02x",address);
			}
			
			llogInfo(tagBuff,"Time 0x%04x,%s Command:%s,%s",time,t_r? "Transmit":"Receive",address_string,countString);
		}

	}
	else if (type == DATA_TYPE_DATA_WORD)
	{
		//Parse the data word
		llogInfo(tagBuff,"Data Word:0x%02x",word);

	}
	else if (type == DATA_TYPE_STATUS_WORD)
	{
		//Parse the status word
		llogInfo(tagBuff,"Status Word:0x%02x",word);
	}
	
	return false;

}

WordParser* WordParser::sharedParser()
{
	if (!parserPtr)
	{
		parserPtr = new WordParser;
	}
	return parserPtr;
}


#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include <app/Message.h>
#include <support/List.h>
#include <support/String.h>
#include <storage/File.h>
#include <interface/Rect.h>
#include <support/Locker.h>
#include <stdio.h>



/*
	%d %i 	    Decimal signed integer.
    %o			Octal integer.
    %x %X		Hex integer.
    %u			Unsigned integer.
    %c			Character.
    %s			String. See below.
    %f			double
    %p			pointer.
    %b			bool
    %r			*Rect
*/



/**
 * @class Debug
 * @brief  Provide a tool for effective Debbunging ;-)
 */
enum debug_kind{D_INFO,D_DEBUG,D_ERROR};
 
class Debugger 
{

public:
						Debugger(char *newGlobalName);
						~Debugger(void);
			void		Info(const char* group,const char *format, ...) const;
			void		Debug(const char* group,const char *format, ...) const;
			void		Error(const char* group,const char *format, ...) const;
			BList*		GetGroupList(void);
			void		EnableAllGroups(void){disabledGroups->MakeEmpty();allDisabled=false;};
			void		EnableGroup(char *group);
			void		DisableGroup(char *group);	
			void		DisableAllGroups(void){allDisabled=true;};
			BMessage*	GetErrors(char *group);
			BMessage* 	GetDebugs(char *group);
			BMessage*	GetInfos(char *group);
			BMessage*	GetMessages(char *group);
			void		SetTerminalOutput(bool on){terminalOutput=on;};
			void		SetFile(BFile *output){file=output;};

protected:
private:
			BList*		disabledGroups;
			BMessage*	allMessages;
			bool		terminalOutput;
			bool		allDisabled;
			BFile		*file;
			char		*globalName;
			BLocker		*locker;
};
#endif

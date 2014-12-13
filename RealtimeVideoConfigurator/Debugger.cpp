#include "Debugger.h"
#include <storage/Entry.h>
#include <storage/File.h>
#include <media/MediaDefs.h>
#include <time.h>
static const char* formatings[]={"%d","%x","%u","%f","%ld","%lx","%lu","%lf","%c","%s","%p","%b","%r",NULL};


Debugger::Debugger(char *newGlobalName)
{
	locker=new BLocker("Debug_logger");
	locker->Lock(); 
	globalName=newGlobalName;
	BEntry *tester=new BEntry();
	entry_ref ref;
	//preparation to a Algorihtm wich dosent overite existinglogFiles...
	uint32 i=0;
	char *path = new char[B_PATH_NAME_LENGTH];
	sprintf(path,"/var/log/%s.log",globalName);
	::get_ref_for_path(path, &ref);
	tester->SetTo(&ref);
	/*	do
	{
	
		sprintf(path,"/var/log/%s.%llu.raw_video.%ld",globalName,system_time(),i);
				i++;
	}
	while (tester->Exists());*/
	terminalOutput	= true;
	allDisabled		= false;
	allMessages		= new BMessage();
	file			= new BFile(tester,B_WRITE_ONLY|B_CREATE_FILE);
	status_t err	= file->InitCheck();
	if (err)
		printf("Debugger Error");
	disabledGroups	= new BList();
	locker->Unlock(); 
}

Debugger::~Debugger(void)
{
	delete disabledGroups;
	delete allMessages;
	delete file;
	delete globalName;
}

void Debugger::Info(const char* group,const char *format, ...) const
{
	if (locker->Lock())
	{
	BString		formater=format;
	int32		offset=0;
	int32		newOffset=0;
	int32 		wich_format;
	bool		found=false;
	BMessage*	tmpMessage=new BMessage(D_INFO);
	tmpMessage->AddInt64("when",system_time());
	va_list params; 
	va_start(params,format); 
	char **formatList = (char **)formatings;
	//slow but clean ;-)
	while (offset != B_ERROR) 
	{
		formatList = (char **)formatings;
		found=false;
		newOffset=formater.IFindFirst("%",offset);
		wich_format=0;
		if (newOffset!=B_ERROR)
		{
			while ((*formatList)&&(found==false))
			{
				//check if it is a valid tag
				if (formater.IFindFirst(*formatList,offset)==newOffset)
				{
					BString tmpString;
					switch(wich_format) 
					{
						case 0:
							tmpString << va_arg(params, int);
							break;
						case 1:
							tmpString <<va_arg(params, uhex16);
							break;
						case 2:
							tmpString << va_arg(params, unsigned int);
							break;
						case 3:
							tmpString << va_arg(params, float);
							break;
						case 4:
							tmpString <<va_arg(params, int32);
							break;
						case 5:
							tmpString << va_arg(params, uhex32);
							break;
						case 7:
							tmpString << va_arg(params, uint32);
							break;
						case 8:
							//formater <<va_arg(params, double);
							break;
						case 9:
							tmpString << va_arg(params,const char *);
							break;
						case 10:
							tmpString <<va_arg(params, const char *);
							break;
						case 11:
							const char *boolWert;
							if (va_arg(params,bool))
							  boolWert="TRUE";
							else
							  boolWert="FALSE";
							tmpString <<  boolWert;
							break;
						case 12:
							BRect* rect;
							rect=va_arg(params, BRect*);
							tmpString << "Rect(";
							tmpString << rect->left;
							tmpString << ", ";
							tmpString << rect->top;
							tmpString << ", ";
							tmpString << rect->right;
							tmpString << ", ";
							tmpString << rect->bottom;
							tmpString << ")";
							break;
	/*case 13:
							media_format *mf=&va_arg(params,media_format);
							
							tmpString << "type=";
							switch (mf->type)
							{
								case B_MEDIA_RAW_AUDIO:
								break;
								case B_MEDIA_RAW_VIDEO:
									tmpString << "RawVideo";
									tmpString << " ,field_rate=";
									tmpString << mf->u.raw_video.field_rate;
									tmpString << " ,interlace=";
									tmpString << mf->u.raw_video.interlace;
									tmpString << " ,orientation=";
									tmpString << mf->u.raw_video.orientation;
									tmpString << " ,width_aspect=";
									tmpString << mf->u.raw_video.pixel_width_aspect;
									tmpString << " ,height_aspect=";
									tmpString << mf->u.raw_video.pixel_height_aspect;
									tmpString << " ,colorspace=";
									tmpString << mf->u.raw_video.display.format;
									tmpString << " ,width=";
									tmpString << mf->u.raw_video.display.line_width;
									tmpString << " ,height=";
									tmpString << mf->u.raw_video.display.line_count;
								break;
							}
							break;*/
					}
					formater.IReplaceFirst(*formatList,tmpString.String());
				}
				else
				{
					formatList++;
					wich_format++;
				}
			}
		}
		else
			offset=newOffset;

	}
	va_end(params);
	if ((terminalOutput)&&(!allDisabled))
	{
		//**check if this group contains to a disabled  Group
		printf("%s::%s\n",group,formater.String());
	}
	tmpMessage->AddString(group,formater.String());
	tmpMessage->AddInt64("when",system_time());
	if (file)
	{
		tmpMessage->Flatten(file);
	}
	allMessages->AddMessage("INFO",tmpMessage);
	locker->Unlock(); 
	}
}


void Debugger::Debug(const char* group,const char *format, ...) const
{
	if(locker->Lock())
	{
	BString	formater=format;
	int32	offset=0;
	int32	newOffset=0;
	int32 	wich_format;
	bool	found=false;
 	va_list params; 
	va_start(params,format); 
	char **formatList = (char **)formatings;
	BMessage*	tmpMessage=new BMessage(D_DEBUG);
	tmpMessage->AddInt64("when",system_time());

	//slow but clean ;-)
	while (offset != B_ERROR) 
	{
		formatList = (char **)formatings;
		found=false;
		newOffset=formater.IFindFirst("%",offset);
		wich_format=0;
		if (newOffset!=B_ERROR)
		{
			while ((*formatList)&&(found==false))
			{
				//check if it is a valid tag
				if (formater.IFindFirst(*formatList,offset)==newOffset)
				{
					BString tmpString;
					switch(wich_format) 
					{
						case 0:
							tmpString << va_arg(params, int);
							break;
						case 1:
							tmpString <<va_arg(params, uhex32);
							break;
						case 2:
							tmpString << va_arg(params, unsigned int);
							break;
						case 3:
							tmpString << va_arg(params, float);
							break;
						case 4:
							tmpString <<va_arg(params, int32);
							break;
						case 5:
							tmpString << va_arg(params, uhex32);
							break;
						case 7:
							tmpString << va_arg(params, uint32);
							break;
						case 8:
							//formater <<va_arg(params, double);
							break;
						case 9:
							tmpString << va_arg(params,const char *);
							break;
						case 10:
							tmpString <<va_arg(params, const char *);
							break;
						case 11:
							const char *boolWert;
							if (va_arg(params,bool))
							  boolWert="TRUE";
							else
							  boolWert="FALSE";
							tmpString <<  boolWert;
							break;
						case 12:
							BRect* rect;
							rect=va_arg(params, BRect*);
							tmpString << "Rect(";
							tmpString << rect->left;
							tmpString << ", ";
							tmpString << rect->top;
							tmpString << ", ";
							tmpString << rect->right;
							tmpString << ", ";
							tmpString << rect->bottom;
							tmpString << ")";
							break;
	/*case 13:
							media_format *mf=&va_arg(params,media_format);
							
							tmpString << "type=";
							switch (mf->type)
							{
								case B_MEDIA_RAW_AUDIO:
								break;
								case B_MEDIA_RAW_VIDEO:
									tmpString << "RawVideo";
									tmpString << " ,field_rate=";
									tmpString << mf->u.raw_video.field_rate;
									tmpString << " ,interlace=";
									tmpString << mf->u.raw_video.interlace;
									tmpString << " ,orientation=";
									tmpString << mf->u.raw_video.orientation;
									tmpString << " ,width_aspect=";
									tmpString << mf->u.raw_video.pixel_width_aspect;
									tmpString << " ,height_aspect=";
									tmpString << mf->u.raw_video.pixel_height_aspect;
									tmpString << " ,colorspace=";
									tmpString << mf->u.raw_video.display.format;
									tmpString << " ,width=";
									tmpString << mf->u.raw_video.display.line_width;
									tmpString << " ,height=";
									tmpString << mf->u.raw_video.display.line_count;
								break;
							}
							break;*/
					}
					formater.IReplaceFirst(*formatList,tmpString.String());
				}
				else
				{
					formatList++;
					wich_format++;
				}
			}
		}
		else
			offset=newOffset;
	}
	va_end(params);
	if ((terminalOutput)&&(!allDisabled))
	{
		//**check if this group contains to a disabled  Group
		printf("\t%s::%s\n",group,formater.String());
	}
	tmpMessage->AddString(group,formater.String());
	tmpMessage->AddInt64("when",system_time());
	if (file)
	{
		tmpMessage->Flatten(file);
	}
	allMessages->AddMessage("DEBUG",tmpMessage);
	locker->Unlock(); 
	}
}





void Debugger::Error(const char* group,const char *format, ...) const
{
	if(locker->Lock())
	{
	BString		formater=format;
	int32		offset=0;
	int32		newOffset=0;
	int32 		wich_format;
	bool		found=false;
	BMessage*	tmpMessage=new BMessage(D_ERROR);
	tmpMessage->AddInt64("when",system_time());
	va_list params; 
	va_start(params,format); 
	char **formatList = (char **)formatings;
	//slow but clean ;-)
	while (offset != B_ERROR) 
	{
		formatList = (char **)formatings;
		found=false;
		newOffset=formater.IFindFirst("%",offset);
		wich_format=0;
		if (newOffset!=B_ERROR)
		{
			while ((*formatList)&&(found==false))
			{
				//check if it is a valid tag
				if (formater.IFindFirst(*formatList,offset)==newOffset)
				{
					BString tmpString;
					switch(wich_format) 
					{
						case 0:
							tmpString << va_arg(params, int);
							break;
						case 1:
							tmpString <<va_arg(params, uhex32);
							break;
						case 2:
							tmpString << va_arg(params, unsigned int);
							break;
						case 3:
							tmpString << va_arg(params, float);
							break;
						case 4:
							tmpString <<va_arg(params, int32);
							break;
						case 5:
							tmpString << va_arg(params, uhex32);
							break;
						case 7:
							tmpString << va_arg(params, uint32);
							break;
						case 8:
							//formater <<va_arg(params, double);
							break;
						case 9:
							tmpString << va_arg(params,const char *);
							break;
						case 10:
							tmpString <<va_arg(params, const char *);
							break;
						case 11:
							const char *boolWert;
							if (va_arg(params,bool))
							  boolWert="TRUE";
							else
							  boolWert="FALSE";
							tmpString <<  boolWert;
							break;
						case 12:
							BRect* rect;
							rect=va_arg(params, BRect*);
							tmpString << "Rect(";
							tmpString << rect->left;
							tmpString << ", ";
							tmpString << rect->top;
							tmpString << ", ";
							tmpString << rect->right;
							tmpString << ", ";
							tmpString << rect->bottom;
							tmpString << ")";
							break;
						/*case 13:
							media_format *mf=&va_arg(params,media_format);
							
							tmpString << "type=";
							switch (mf->type)
							{
								case B_MEDIA_RAW_AUDIO:
								break;
								case B_MEDIA_RAW_VIDEO:
									tmpString << "RawVideo";
									tmpString << " ,field_rate=";
									tmpString << mf->u.raw_video.field_rate;
									tmpString << " ,interlace=";
									tmpString << mf->u.raw_video.interlace;
									tmpString << " ,orientation=";
									tmpString << mf->u.raw_video.orientation;
									tmpString << " ,width_aspect=";
									tmpString << mf->u.raw_video.pixel_width_aspect;
									tmpString << " ,height_aspect=";
									tmpString << mf->u.raw_video.pixel_height_aspect;
									tmpString << " ,colorspace=";
									tmpString << mf->u.raw_video.display.format;
									tmpString << " ,width=";
									tmpString << mf->u.raw_video.display.line_width;
									tmpString << " ,height=";
									tmpString << mf->u.raw_video.display.line_count;
								break;
							}
							break;*/
					}
					formater.IReplaceFirst(*formatList,tmpString.String());
				}
				else
				{
					formatList++;
					wich_format++;
				}
			}
		}
		else
			offset=newOffset;

	}
	va_end(params);
	if ((terminalOutput)&&(!allDisabled))
	{
		//**check if this group contains to a disabled  Group
		printf("******************** E R R O R ***********\n");
		printf("\t%s::%s\n",group,formater.String());
		printf("******************************************\n");
	}
	tmpMessage->AddString(group,formater);
	tmpMessage->AddInt64("when",system_time());
	if (file)
	{
		tmpMessage->Flatten(file);
	}		

	allMessages->AddMessage("ERROR",tmpMessage);
	locker->Unlock();
	}
}



void Debugger::DisableGroup(char *group)
{
	if (group!=NULL)
		disabledGroups->AddItem(group);
}



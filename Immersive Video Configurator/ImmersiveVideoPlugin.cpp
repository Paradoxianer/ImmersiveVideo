#include <TextControl.h>
#include <Window.h>
#include <stdio.h>
#include <kernel/image.h>
#include <Resources.h>
#include <File.h>
#include <Bitmap.h>
#include "ImmersiveVideoPlugin.h"
//#include "RVCDefs.h"

//const Debugger	*myDebugger			= new Debugger("Immersive Video Plugin");


ImmersiveVideoPlugin::ImmersiveVideoPlugin(image_id id):BHandler("Pluginhandler")
{
	//INFO("ImmersiveVideoPlugin","ImmersiveVideoPlugin()");
	mId=id;
	lTable=NULL;
	outDim=NULL;
	inDim=NULL;
	view=NULL;
	aboutView=NULL;
	bmp=NULL;
	messenger=NULL;
	LoadIcon();
}

ImmersiveVideoPlugin::~ImmersiveVideoPlugin()
{
//	INFO("ImmersiveVideoPlugin","~ImmersiveVideoPlugin()");
	delete messenger;
	delete lTable;
	delete outDim;
	delete inDim;
	delete view;
	delete aboutView;
	delete bmp;
}


status_t ImmersiveVideoPlugin::Archive(BMessage *into, bool deep = true) const
{
//	BHandler::Archive(into,deep);
	/*if (locker->Lock())
	{*/
//	INFO("ImmersiveVideoPlugin","Archive()");

		view->Window()->Lock();
		aboutView->Window()->Lock();
		if ((view)&&(aboutView)){
		 	BMessage *tmpMsg=new BMessage();
			view->Archive(tmpMsg,deep);
			into->AddMessage("PluginView",tmpMsg);
			aboutView->Archive(tmpMsg,deep);
		//	tmpMsg->PrintToStream();
			into->AddMessage("AboutView",tmpMsg);
			return B_OK;
		}
		else
			return B_ERROR;
		view->Window()->Unlock();
		aboutView->Window()->Unlock();
		/*locker->Unlock();
	}*/
}

const char* ImmersiveVideoPlugin::GetName()
{
//	INFO("ImmersiveVideoPlugin","GetName()");
	return "none";
}

BBitmap* ImmersiveVideoPlugin::GetIcon()
{
//	INFO("ImmersiveVideoPlugin","GetIcon()");
	return bmp;
}

const char* ImmersiveVideoPlugin::GetHelpFile()
{
//	INFO("ImmersiveVideoPlugin","GetHelpFile()");
	return NULL;
}

const char* ImmersiveVideoPlugin::GetVersion()
{
//	INFO("ImmersiveVideoPlugin","GetVersion()");
	return "0";
}


BView* ImmersiveVideoPlugin::GetView()
{
//	INFO("ImmersiveVideoPlugin","GetView()");
	return view;
}

BView* ImmersiveVideoPlugin::GetAboutView()
{
//	INFO("ImmersiveVideoPlugin","GetAboutView()");
	return aboutView;
}

void ImmersiveVideoPlugin::Init()
{
//	INFO("ImmersiveVideoPlugin","Init()");
	locker		= new BLocker();
	view		= new BView(BRect(10,10,100,100),"PluginView",B_FOLLOW_ALL_SIDES,0);
	aboutView	= new BView(BRect(0,0,100,50),"AboutPlugin",B_FOLLOW_ALL_SIDES,0);
}

void ImmersiveVideoPlugin::Init(BMessage *message)
{
//	INFO("ImmersiveVideoPlugin","Init(BMessage)");
	locker		= new BLocker();	
	BMessage *tmp=new BMessage();
	message->FindMessage("PluginView",tmp);
	view=new BView(tmp);
	tmp->MakeEmpty();
	message->FindMessage("AboutPlugin",tmp);
	aboutView=new BView(tmp);
}

BPoint*	ImmersiveVideoPlugin::GetLookUpTable()
{
	/*if (locker->Lock())
	{*/
		return lTable;
		/*locker->Unlock();
	}*/
}


status_t	ImmersiveVideoPlugin::DeleteLookUpTable()
{
	/*if (locker->Lock())
	{*/
//	INFO("ImmersiveVideoPlugin","DeleteLookUpTable()");
		status_t err;
		if (lTable)
		{
			err=rtm_free(lTable);
			if (err == B_OK)
			{
				err=rtm_delete_pool(pool);
				if (err == B_OK)
				{
					pool=NULL;
//					DE_BUG("ImmersiveVideoPlugin","Lookuptable fregegeben");
				}
				else
					0==0;
//					ERROR("ImmersiveVideoPlugin","rtm_free %s",strerror(err));
			}
			else
				0==0;
//				ERROR("ImmersiveVideoPlugin","rtm_free %s",strerror(err));
		}
		lTable=NULL;
		return err;
		/*locker->Unlock();
	}*/
}

void ImmersiveVideoPlugin::SetInputDimension(BRect* inputDimension)
{
//	INFO("ImmersiveVideoPlugin","SetInputDimension(%r)",inputDimension);
	inDim=inputDimension;
}
void ImmersiveVideoPlugin::SetOutputDimension(BRect* outputDimension)
{
//	INFO("ImmersiveVideoPlugin","SetOutputDimension(%r)",outputDimension);
	outDim=outputDimension;
	outDim->PrintToStream();
}

int32 ImmersiveVideoPlugin::RunCalculation(void *castToThis)
{
//	INFO("ImmersiveVideoPlugin","RunCalculation()");
	ImmersiveVideoPlugin *plg=reinterpret_cast<ImmersiveVideoPlugin*>(castToThis); 
	plg->Run(); 
	//send LOOK_UP_TABLE_CALCULATED
	BMessage *message=new BMessage('LUTC');
	status_t err=message->AddPointer("who",plg);
	err = plg->messenger->SendMessage(message);
 	return 0;
}

void ImmersiveVideoPlugin::Run()
{
//	INFO("ImmersiveVideoPlugin","Run()");
	status_t err = B_OK;
	if (messenger==NULL)
		messenger=new BMessenger(Looper(),NULL,NULL);
	
	if (outDim&&inDim)
	{
		lTableSize=(outDim->IntegerWidth()+1)*(outDim->IntegerHeight()+1);
		uint64 size=lTableSize*sizeof(BPoint);
		err=rtm_create_pool(&pool,size,"lookUpTable");
		lTable=(BPoint *)rtm_alloc(pool,size);
//		DE_BUG("ImmersiveVideoPlugin","Lookuptable alloziert %s",strerror(err));
		//falls es Probleme beim Erstellen des Echtzeitspeichers gab
		if (lTable==NULL) 
		{
			lTable=new BPoint[lTableSize];
//			ERROR("ImmersiveVideoPlugin","Lookuptable allocProbleme");	
		}
	}
}

void ImmersiveVideoPlugin::Undo(){}
void ImmersiveVideoPlugin::Redo(){}

bool ImmersiveVideoPlugin::UnAndRedoPossible(void)
{
	return false;
}
void ImmersiveVideoPlugin::DoCopy(){}
void ImmersiveVideoPlugin::DoPaste(){}
void ImmersiveVideoPlugin::DoCut(){}

bool ImmersiveVideoPlugin::ToolsPossible(void)
{
	return false;
}
void ImmersiveVideoPlugin::MessageReceived(BMessage *message)
{
	switch(message->what) 
	{
		case B_UNDO:
			Undo();
			break;
		case B_CUT:
			DoCut();
			break;
		case B_COPY:
			DoCopy();
			break;
		case B_PASTE:
			DoPaste();
			break;
		default:
			view->MessageReceived(message);
			break;
	}
}

void ImmersiveVideoPlugin::LoadIcon()
{
//	INFO("ImmersiveVideoPlugin","LoadIcon()");
	image_info	*info=new image_info;
	size_t	size;
	const void	*data;
	get_image_info(mId,info);
	BResources *res=new BResources(new BFile((const char*)info->name,B_READ_ONLY));
	if (bmp==NULL)
		bmp=new BBitmap(BRect(0.0, 0.0, 15.0, 15.0),B_CMAP8);
	data=res->LoadResource((type_code)'MICN',"BEOS:M:STD_ICON",&size);
	if (data)
	{
		bmp->Lock();
		bmp->SetBits(data,size,0,B_CMAP8);
		bmp->Unlock();
	}
}

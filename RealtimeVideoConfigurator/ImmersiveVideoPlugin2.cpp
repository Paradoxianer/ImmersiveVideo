#include <TextControl.h>
#include <Window.h>
#include <stdio.h>
#include <kernel/image.h>
#include <Resources.h>
#include <File.h>
#include <Bitmap.h>
#include "ImmersiveVideoPlugin2.h"
#include "RVCDefs.h"

ImmersiveVideoPlugin2::ImmersiveVideoPlugin2(image_id id):ImmersiveVideoPlugin(id)
{
	INFO("ImmersiveVideoPlugin2","ImmersiveVideoPlugin2()");
}

ImmersiveVideoPlugin2::~ImmersiveVideoPlugin2()
{
	INFO("ImmersiveVideoPlugin2","~ImmersiveVideoPlugin2()");
}


status_t ImmersiveVideoPlugin2::Archive(BMessage *into, bool deep = true) const
{
	INFO("ImmersiveVideoPlugin2","Archive()");
	status_t err;
	err	= ImmersiveVideoPlugin::Archive(into,deep);
//**archive GlassView 
//** Archive BBitmap??
	return err;
}


void ImmersiveVideoPlugin2::Init()
{
	INFO("ImmersiveVideoPlugin2","Init()");
	ImmersiveVideoPlugin::Init();
/*	glassView=new BView(BRect(0,0,100,100),"glassView",B_FOLLOW_ALL_SIDES,0);
	glassView->SetViewColor(B_TRANSPARENT_32_BIT);*/
	configView=NULL;
	configBitmap=NULL;
}

void ImmersiveVideoPlugin2::Init(BMessage *message)
{
	INFO("ImmersiveVideoPlugin2","Init(BMessage)");
	ImmersiveVideoPlugin::Init(message);
	//glassView Rausholen 	
	
}

void ImmersiveVideoPlugin2::MessageReceived(BMessage *message)
{
	INFO("ImmersiveVideoPlugin2","MessageReceived()");
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


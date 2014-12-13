#include <TextControl.h>
#include <image.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "SpiegePlugin.h"
#include "Constants.h"




SpiegePlugin::SpiegePlugin(image_id id):PanoflexPlugin(id){}

const char* SpiegePlugin::GetName(){
	return "Spiege";
}

BBitmap* SpiegePlugin::GetIcon(){
	return NULL;
}

const char* SpiegePlugin::GetHelpFile(){
	return NULL;
}

const char* SpiegePlugin::GetVersion(){
	return "0.0.1a";
}



void SpiegePlugin::Init(){
	PanoflexPlugin::Init();
	view->ResizeTo(200,200);
	view->SetViewColor(216,216,216);
	a=new BTextControl(BRect(0,5,230,20),"a","Äußere Entzerrung:","1,0",new BMessage(),B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW|B_PULSE_NEEDED);
	b=new BTextControl(BRect(0,25,230,20),"b","Mittlere Entzerrung:","1,0",new BMessage(),B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW|B_PULSE_NEEDED);
	c=new BTextControl(BRect(0,45,230,20),"c","Innere Entzerrung:","1,0",new BMessage(),B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW|B_PULSE_NEEDED);
	d=new BTextControl(BRect(0,65,230,20),"d","Skalierung","1,0",new BMessage(),B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW|B_PULSE_NEEDED);
	view->AddChild(a);
	view->AddChild(b);
	view->AddChild(c);
	view->AddChild(d);
	printf("Bin in Init\n");
	BMessage *test=new BMessage;
	Archive(test,true);
	test->PrintToStream();
}

void SpiegePlugin::Init(BMessage *message){
	PanoflexPlugin::Init(message);

}

void SpiegePlugin::MessageReceived(BMessage *message){
}

void SpiegePlugin::Run(){
	//*************  GAAAANz Wichtig    ********************//
	PanoflexPlugin::Run();
	float x,y
	if (lTable!=NULL){
		    for (y=0;y<oDim->Height();y++){
              for (x=0;x<oDim->Width();x++){
              	status=(x*iDim->Width())+y;
         		offset=((y*oDim->Width())+x);
				lTable[offset].x=oDim->Widh()-x;
				lTable[offset].y=oDim->Height()-y;
              }
            }
	 	}
}
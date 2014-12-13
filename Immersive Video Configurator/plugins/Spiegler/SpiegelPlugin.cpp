#include <TextControl.h>
#include <StringView.h>
#include <image.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "SpiegelPlugin.h"
#include "Constants.h"




SpiegelPlugin::SpiegelPlugin(image_id id):ImmersiveVideoPlugin(id){}

const char* SpiegelPlugin::GetName(){
	return "Spiegel";
}

BBitmap* SpiegelPlugin::GetIcon(){
	return NULL;
}

const char* SpiegelPlugin::GetHelpFile(){
	return NULL;
}

const char* SpiegelPlugin::GetVersion(){
	return "0.0.1a";
}



void SpiegelPlugin::Init(){
	ImmersiveVideoPlugin::Init();
//	view->ResizeTo(200,200);
	view->SetViewColor(216,216,216);
	menu=new BPopUpMenu("Vertical");
	BMenuItem *item;
	menu->AddItem(item=new BMenuItem("Horizontal",NULL));
	menu->AddItem(new BMenuItem("Vertical",NULL));
	item->SetMarked(true);
	BMenuField	*v_h_choose=new BMenuField(BRect(5,5,100,30),"v_h_chooser",NULL,menu);
	view->AddChild(v_h_choose);
	//view->AddChild(new BStringView(BRect(5,5,100,30),"sView","Diese Plugin hat keine Einstellmöglichkeiten"));

	//view->AddChild(v_h_choose);
}

void SpiegelPlugin::Init(BMessage *message){
	ImmersiveVideoPlugin::Init(message);
	menu=dynamic_cast<BPopUpMenu *>((dynamic_cast<BMenuField *>(view->FindView("v_h_chooser")))->Menu());
}

void SpiegelPlugin::MessageReceived(BMessage *message){
}

void SpiegelPlugin::Run(){
	//*************  GAAAANz Wichtig    ********************//
	ImmersiveVideoPlugin::Run();
	uint32 x,y;
	float xfaktor=oDim->Width()/iDim->Width();
	float yfaktor=oDim->Height()/iDim->Height();
	//printf("XFaktor=%f\tyFaktor=%f\n",xfaktor,yfaktor);
	uint32 offset;
	
	int32 method=menu->IndexOf(menu->FindMarked());
	if (lTable!=NULL){
		if (method==0){
			for (y=0;y<=oDim->Height();y++){
				for (x=0;x<=oDim->Width();x++){
        	 		offset=(uint32)((y*(iDim->Width()))+x);
         			status=offset;
					lTable[offset].x=x/xfaktor;
					lTable[offset].y=(iDim->Height())-(((float)y)/yfaktor);
            	 }
            }
        }
        else{
        	for (y=0;y<oDim->Height();y++){
				for (x=0;x<oDim->Width();x++){
        	 		offset=(uint32)((y*(iDim->Width()))+x);
         			status=offset;
					lTable[offset].x=((iDim->Width())-(((float)x)/xfaktor));
//					if (lTable[offset].x<0) lTable[offset].x=0;
					lTable[offset].y=(((float)y)/yfaktor);
	//				if (lTable[offset].y<0) lTable[offset].y=0;
/*					lTable[offset].x=x/2;
					lTable[offset].y=y;*/
            	 }
            }
        
        }
 	}
}

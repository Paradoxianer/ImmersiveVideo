#include <TextControl.h>
#include <image.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <TranslationUtils.h>
#include "FisheyePlugin.h"
#include "Constants.h"

FisheyePlugin::FisheyePlugin(image_id id):ImmersiveVideoPlugin(id){}

const char* FisheyePlugin::GetName(){
	return "Fisheye";
}

const char* FisheyePlugin::GetHelpFile(){
	return NULL;
}

const char* FisheyePlugin::GetVersion(){
	return "0.0.1a";
}



void FisheyePlugin::Init(){
	ImmersiveVideoPlugin::Init();
	view->ResizeTo(220,220);
	view->SetViewColor(216,216,216);
	dx=new BTextControl(BRect(0,5,215,20),"dx2","Mittelpunkt x verschieben:","0.0",new BMessage(),B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW|B_PULSE_NEEDED);
	dy=new BTextControl(BRect(0,25,215,40),"dy2","Mittelpunkt y verschiebe:","0.0",new BMessage(),B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW|B_PULSE_NEEDED);
	rap=new BTextControl(BRect(0,65,215,80),"rap2","Äußerer Radius (0 - 1.0):","1.0",new BMessage(),B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW|B_PULSE_NEEDED);
	view->AddChild(dx);
	view->AddChild(dy);

	view->AddChild(rap);
}

void FisheyePlugin::Init(BMessage *message){
	ImmersiveVideoPlugin::Init(message);
	dx=dynamic_cast<BTextControl *>(view->FindView("dx2"));
	dy=dynamic_cast<BTextControl *>(view->FindView("dy2"));
	rap=dynamic_cast<BTextControl *>(view->FindView("rap2"));

}

void FisheyePlugin::MessageReceived(BMessage *message){
}

void FisheyePlugin::Run(){
	//*************  GAAAANz Wichtig    ********************//
	ImmersiveVideoPlugin::Run();
	uint32 x,y,offset;
	double deltaX,deltaY,prozent;
	deltaX=atof(dx->Text());
	deltaY=atof(dy->Text());
	prozent=atof(rap->Text());
//	if (rStart>rStop)
	if (lTable!=NULL)
	{
		double xn,yn;
		double ixm=(iDim->Width()/2.0);
		double iym=(iDim->Height()/2.0);
	    double oxm=(oDim->Width()/2.0);
		double oym=(oDim->Height()/2.0);
		for (y=0;y<oDim->Height();y++)
		{
			yn=(y/oDim->Height())*iDim->Height()+deltaY;
			for (x=0;x<=oDim->Width();x++)
			{
				//
				xn=((x-oxm)/oxm)*cos(((y-oym)/oym*(M_PI/2))*prozent)*iym+ixm+deltaX;
//				xn=(x/oDim->Width())*iDim->Width();	
			//	xn=((x-oxm)/oym);//*((y-oym)/oym);
				if (xn>(iDim->Width()))
				  xn=iDim->Width();
				if (yn>(iDim->Height()))
				  yn=iDim->Height(); 
				if (xn<0)
				  xn=0;
				if (yn<0)
				  yn=0; 
				offset=(y*(oDim->IntegerWidth()+1))+((oDim->Width())-x);
				status=offset;
				lTable[offset].x=xn;
				if (lTable[offset].x>iDim->Width()) lTable[offset].x=iDim->Width();
				lTable[offset].y=yn;
				if (lTable[offset].y>iDim->Height()) lTable[offset].y=iDim->Height();
			}
		}

	}
}

/*		    double r,w,rn;
		    float xn=0,yn=0;


	    	//Calculate the middel of the InputImage
	    	double ixm=(iDim->Width()/2.0);
		    double iym=(iDim->Height()/2.0);
	    	double oxm=(oDim->Width()/2.0);
		    double oym=(oDim->Height()/2.0);

			double radiusLaenge=(rStop-rStart);

	    	//Calculate the max Radius...
	    	double imaxr=iym;
		    for (y=0;y<oym;y++)
		    {
		      r=(oym-y)/oym;
//            r=((y/oDim->Height())*radiusLaenge)+rStart;
              for (x=0;x<oDim->Width();x++)
              {
                w=M_PI-(x/oDim->Width())*M_PI;
				xn=(float)((iym*r*sin(w))+ixm)+deltaX;
                yn=((float)((iym*r*cos(w))+iym)+deltaY);

                if (xn>(iDim->Width()-1))
				  xn=iDim->Width()-1;
				if (yn>(iDim->Height()-1))
				  yn=iDim->Height()-1; 
				if (xn<0)
				  xn=0;
				if (yn<0)
				  yn=0; 
				offset=(y*oDim->IntegerWidth())+x;
				status=offset;
				lTable[offset].x=xn;
				if (lTable[offset].x>iDim->Width()) lTable[offset].x=iDim->Width()-1;
				lTable[offset].y=yn;
				if (lTable[offset].y>iDim->Height()) lTable[offset].y=iDim->Height()-1;
              }
			  r=y/oym;
//              r=(((oDim->Height()-y)/oDim->Height())*radiusLaenge)+rStart;
	 		  //(((y+(oDim->Height()/2))/oDim->Height())*radiusLaenge)+rStart;
              for (x=0;x<oDim->Width();x++)
              {
				//w=((x/oDim->Width())*M_PI)-(M_PI*2);
				w=M_PI+((x/oDim->Width())*M_PI);
                //w=(x/(oDim->Width()/2))*M_PI*2;
				xn=(float)((iym*r*sin(w))+ixm)+deltaX;
                yn=((float)((iym*r*cos(w))+iym)+deltaY);

                if (xn>(iDim->Width()-1))
				  xn=iDim->Width()-1;
				if (yn>(iDim->Height()-1))
				  yn=iDim->Height()-1; 
				if (xn<0)
				  xn=0;
				if (yn<0)
				  yn=0; 
				offset=((y)*oDim->IntegerWidth())+x;;
				status=offset;
				lTable[offset].x=xn;
				if (lTable[offset].x>iDim->Width()) lTable[offset].x=iDim->Width()-1;
				lTable[offset].y=yn;
				if (lTable[offset].y>iDim->Height()) lTable[offset].y=iDim->Height()-1;
              }
	 		}*/
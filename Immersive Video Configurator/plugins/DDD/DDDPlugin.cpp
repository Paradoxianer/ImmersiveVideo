#include <TextControl.h>
#include <StringView.h>
#include <image.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "DDDPlugin.h"
#include "Constants.h"




DDDPlugin::DDDPlugin(image_id id):ImmersiveVideoPlugin(id){}

const char* DDDPlugin::GetName(){
	return "DDD";
}


const char* DDDPlugin::GetHelpFile(){
	return NULL;
}

const char* DDDPlugin::GetVersion(){
	return "0.0.1a";
}



void DDDPlugin::Init(){
	ImmersiveVideoPlugin::Init();
	view->ResizeTo(200,200);
	view->SetViewColor(216,216,216);
	//Auswahlsmenue (3D,Entzerren oder Beides)
	menu=new BPopUpMenu("Methode");
	BMenuItem *item;
	menu->AddItem(item=new BMenuItem("Entzerren + 3D",NULL));
	menu->AddItem(new BMenuItem("Nur Entzerren",NULL));
	menu->AddItem(new BMenuItem("Nur 3D",NULL));
	item->SetMarked(true);
	BMenuField	*v_h_choose=new BMenuField(BRect(5,5,100,30),"e_3d_chooser",NULL,menu);
	//nun Auswahl, wie gross entzerrtes Bild sein soll
	xzerr=new BTextControl(BRect(0,50,100,20),"xzerr","X:","1.0",new BMessage(),B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW|B_PULSE_NEEDED);
	yzerr=new BTextControl(BRect(0,70,100,20),"yzerr","Y:","0.37",new BMessage(),B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW|B_PULSE_NEEDED);
	//und die Zoomfaktoreingabe
	zoom=new BTextControl(BRect(0,100,100,20),"zoom","Zoom:","1.0",new BMessage(),B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW|B_PULSE_NEEDED);
	view->AddChild(v_h_choose);
	view->AddChild(xzerr);
	view->AddChild(yzerr);
	view->AddChild(zoom);
}

void DDDPlugin::Init(BMessage *message){
	ImmersiveVideoPlugin::Init(message);
	menu=dynamic_cast<BPopUpMenu *>((dynamic_cast<BMenuField *>(view->FindView("e_3d__chooser")))->Menu());
	xzerr=dynamic_cast<BTextControl *>(view->FindView("xzerr"));
	yzerr=dynamic_cast<BTextControl *>(view->FindView("yzerr"));
	zoom=dynamic_cast<BTextControl *>(view->FindView("zoom"));
}

void DDDPlugin::MessageReceived(BMessage *message){
}

void DDDPlugin::Run()
{
	//*************  GAAAANz Wichtig    ********************//
	ImmersiveVideoPlugin::Run();
	double z,xh,yh,l,xn,yn,r,w;
	double xmax = 0;
	double ymax = 0;
	uint32 x,y,offset,offset2;
	BPoint *helpTable;
	int32 method=menu->IndexOf(menu->FindMarked());
	if (lTable!=NULL){
		printf("\n\nUmwandlung beginnt....\n");
		z=atof(zoom->Text());
		float xdim = iDim->Width() * atof(xzerr->Text());
		float ydim = iDim->Height() * atof(yzerr->Text());
		uint32 ixdim = (int) xdim;
		uint32 iydim = (int) ydim;
		if(method==0) {
			//Help-LUTs erzeugen
			int helpTableSize=(ixdim+1)*(iydim+1);
			helpTable=new BPoint[helpTableSize];
		}		
		if((method==0)||(method==1)) {
			//zu allererst natuerlich das Bild entzerren
			double deltaX = 0;
			double deltaY = 0;
			double rStart = 0.2;
			double rStop = 0.9;
	    	//Calculate the middel of the InputImage
	    	double ixm=(iDim->Width()/2.0);
		    double iym=(iDim->Height()/2.0);
			double radiusLaenge=(rStop-rStart);
	    	//Calculate the max Radius...
		    for (y=0;y<=iydim;y++)
		    {
              r=((y/ydim)*radiusLaenge)+rStart;
              for (x=0;x<=ixdim;x++)
              {
                w=(x/xdim)*M_PI*2;
				xn=(float)((iym*r*sin(w))+ixm)+deltaX;
                yn=((float)((iym*r*cos(w))+iym)+deltaY);
                if (xn>iDim->Width())
				  xn=iDim->Width();
				if (yn>iDim->Height())
				  yn=iDim->Height(); 
				if (xn<0)
				  xn=0;
				if (yn<0)
				  yn=0; 
				offset=((((iydim)-y)*(ixdim))+(ixdim-x));
				if(method==0) {
					helpTable[offset].x = xn;
					helpTable[offset].y = yn;
				}
				else {
					lTable[offset].x = xn;
					lTable[offset].y = yn;
					oDim->Set(0,0,ixdim-1,iydim-1); //*
				}
              }
            }

			printf("\nEntzerrung erfolgreich\n");
		}
		if((method==0)||(method==2)) {
			//jetzt wirds 3D....
			//help-Table ist mit ixdim*iydim Werten gefuellt
			//erstmal Grenzen austesten -> Maximalwerte je nach Zoom
			printf("Methode: %i\n",method);
			
			if(method==2) 
			{
				ixdim = iDim->IntegerWidth();
				iydim = iDim->IntegerHeight();
			}
			printf("ixdim:\t%i\niydim:\t%i\n",ixdim,iydim);

			for (y=0;y<=oDim->Height();y++){
				for (x=0;x<=oDim->Width();x++){
         	//		offset=(uint32)((y*(oDim->Width()))+x);
					xh =  (2*(x/oDim->Width()))-1; //xh zw. -1 und 1
					yh =  (2*(y/oDim->Height()))-1; //yh zw. -1 und 1
					l = sqrt(xh*xh+yh*yh+z*z);
   		   			xn = xh/l; 
     				yn = yh/l;
					if(fabs(xn)>xmax) xmax = fabs(xn);
					if(fabs(yn)>ymax) ymax = fabs(yn);
				}
			}
			printf("\nMaximalwerte berechnet:");
			printf("\nxmax = %f", xmax);
			printf("\nymax = %f\n", ymax);
			//und nun mit Hilfe der bekannten "Maximalwerte" eine LUT erzeugen
			for (y=0;y<=oDim->Height();y++){
				for (x=0;x<=oDim->Width();x++){
   		      		offset=(uint32)((y*oDim->Width())+x);
					xh =  (2*(x/oDim->Width()))-1.0; //xh zw. -1 und 1
					yh =  (2*(y/oDim->Height()))-1.0; //yh zw. -1 und 1
					l = sqrt(xh*xh+yh*yh+z*z);
      				//xn und yn koennen jetzt auf Wert zw. -1 und 1 gesetzt werden
      				xn = xh/(l*xmax); 
     				yn = yh/(l*ymax);
					//jetzt auf gewuenschte Groesse transformieren (0 bis ix(y)dim)
					xn = (xn+1)*(ixdim/2);
					yn = (yn+1)*(iydim/2);
	                if (xn>ixdim-1)
					  xn=ixdim-1;
					if (yn>iydim-1)
					  yn=iydim-1; 
					if (xn<0)
					  xn=0;
					if (yn<0)
					  yn=0; 				
					//und ab in die LUT				
					if(method==0)
					{
						//offset
//						offset2 = (((yh*(ixdim/2))*((float)ixdim))+(xh*(ixdim/2)));
						offset2 = (uint32) (yn*(float)ixdim+xn);
						xn = helpTable[offset2].x;
						yn = helpTable[offset2].y;
						
						//lTable[offset]=helpTable[offset2];
					}
					lTable[offset].x = xn;
					lTable[offset].y = yn;
				}
        	}
        	printf("\n3D erfolgreich\n");
		}
		if(method == 0) delete helpTable;
		printf("\nUmwandlung beendet.\n");
	}
}

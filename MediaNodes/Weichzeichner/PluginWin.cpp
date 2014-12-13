/* PluginWin.cpp */

#include <Box.h>
#include <Application.h>
#include <Window.h>
#include <stdio.h>
#include "PluginWin.h"
#include "Constants.h"


/*-------------------------------------------------------------------------*/

PluginWin::PluginWin(BHandler* hand, ImmersiveVideoPlugin* pli)
	: BWindow(BRect(50,450,300,700), "PluginWin", B_FLOATING_WINDOW,0){
	messenger =new BMessenger(hand);
	plugin=pli;	
	pluginView=NULL;
	BView* tv = new BView(Bounds(), "tweakview", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
	bb = new BBox(BRect(3,4,Bounds().Width()-3, Bounds().Height()-4),"BoxView",B_FOLLOW_ALL_SIDES);

	tv->AddChild(bb);
	//**Add the View from the PlugIN
	//printf("jetzt kommt GetView...\n");
	pluginView=pli->GetView();
	//printf("geschafft %o \n",pluginView);
	bb->AddChild(pluginView);
//	pli->SetView(bb);
	//printf("hinzugefügt\n");


	tv->SetViewColor(216,216,216);
	//Lock();
	AddChild(tv);
	//dem Fenster als Titel den Namen des aktuellen Plugins verpassen
	SetTitle(pli->GetName());
	SetFeel(B_NORMAL_WINDOW_FEEL);	
	Show();
}
/*-------------------------------------------------------------------------*/
void PluginWin::SetPlugin(ImmersiveVideoPlugin* pli)
{
	plugin=pli;
	Lock();
	//erst vorheriges View entfernen
	bb->RemoveChild(pluginView); 
	pluginView=pli->GetView();
	//danach das View des aktuellen Plugins hinzufügen
	bb->AddChild(pluginView);
	SetTitle(pli->GetName());
	Unlock();
}

void 
PluginWin::MessageReceived(BMessage* msg)
{
	switch(msg->what)
	{
	case LOOK_UP_TABLE_CALCULATED:
	{
		/*da wir das aktuelle Ziel von Narichten sein können leiten
		wir die Information, dass die Lookuptable  vertig ist an die 
		Hauptanwendung weiter -> da dort die Bilder sind und die Entzerrung
		statt findet*/
		//**alle anderen verständigen
		messenger->SendMessage(msg);
		break;
	}
/*	 case :
	 break;*/
	 default:
	 	//und andere Narichten lassen wir vom Fenster bearbeiten
	 	BWindow::MessageReceived(msg);
	 	break;
	}
}
/*-------------------------------------------------------------------------*/

void PluginWin::Quit() 
{
	delete messenger;
	BWindow::Quit();
}
/*-------------------------------------------------------------------------*/

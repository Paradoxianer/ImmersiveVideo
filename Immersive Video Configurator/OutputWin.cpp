/* PluginWin.cpp */


#include <Application.h>
#include <Box.h>
#include <ScrollView.h>
#include <stdio.h>
#include "OutputWin.h"
#include "Constants.h"


/*-------------------------------------------------------------------------*/

OutputWin::OutputWin(BWindow* hand)
	: BWindow(BRect(300,50,600,600), "Bild", B_FLOATING_WINDOW, 0)
{
	//B_WILL_DRAW | B_FRAME_EVENTS|B_WILL_ACCEPT_FIRST_CLICK
	handler = hand;
	BRect rect=Bounds();
	//wir misbrauchen eine Menuebar als Statuszeile
	bottomLine = new BMenuBar(BRect(0,536,60,550), "bottom_line",B_FOLLOW_LEFT|B_FOLLOW_BOTTOM, B_ITEMS_IN_ROW,false);
	//und bauen darin ein Popupmenue ein, welches Standartwerte für Zoom usw enthält.
	BPopUpMenu *zoom=new BPopUpMenu("zoom");
	//finetuning damit die Statuszeile nicht so riesengro ist.
	BFont *font=new BFont();
	zoom->GetFont(font);
	font->SetSize(font->Size()-3);
	zoom->SetFont(font);
	bottomLine->SetFont(font);
	BMessage *scale=new BMessage(SCALE);
	scale->AddInt16("to",800);
	zoom->AddItem(new BMenuItem("800%",scale));
	scale=new BMessage(SCALE);
	scale->AddInt16("to",400);
	zoom->AddItem(new BMenuItem("400%",scale));
	scale=new BMessage(SCALE);
	scale->AddInt16("to",200);
	zoom->AddItem(new BMenuItem("200%",scale));
	scale=new BMessage(SCALE);	
	scale->AddInt16("to",175);
	zoom->AddItem(new BMenuItem("175%",scale));
	scale=new BMessage(SCALE);
	scale->AddInt16("to",150);
	zoom->AddItem(new BMenuItem("150%",scale));
	scale=new BMessage(SCALE);
	scale->AddInt16("to",125);
	zoom->AddItem(new BMenuItem("125%",scale));
	scale=new BMessage(SCALE);
	scale->AddInt16("to",100);
	zoom->AddItem(new BMenuItem("100%",scale));
	scale=new BMessage(SCALE);
	scale->AddInt16("to",75);	
	zoom->AddItem(new BMenuItem("75%",scale));
	scale=new BMessage(SCALE);
	scale->AddInt16("to",50);		
	zoom->AddItem(new BMenuItem("50%",scale));
	scale=new BMessage(SCALE);
	scale->AddInt16("to",25);	
	zoom->AddItem(new BMenuItem("25%",scale));
	bottomLine->AddItem(zoom);

	rect=Bounds();
	rect.bottom-=(bottomLine->Bounds().Height()+1);
	rect.right-=(bottomLine->Bounds().Height()+1);
	
	//das View welches das Bild darstellt
	bitmapView=new BitmapView(rect,"BitmapView",B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS||B_FULL_UPDATE_ON_RESIZE);
	/*die Scrollbars müssen registriert sein beim BView, damit sie nicht das View
	bewegen sondern nur den "Ausschnitt" aus dem Bild -> das spart extrem viel
	Memory und auch Zeit beim Großzoomen von Bildern*/
	vScroller=new BScrollBar(BRect(Bounds().Width()-bottomLine->Bounds().Height(),0,300,rect.bottom),"vScroller",bitmapView,0,100,B_VERTICAL);	
	hScroller=new BScrollBar(BRect(60,536,300,550),"hScroller",bitmapView,0,100,B_HORIZONTAL);	
	bitmapView->RegisterScrollBar(hScroller);
	bitmapView->RegisterScrollBar(vScroller);
	AddChild(vScroller);	
	AddChild(bitmapView);
	AddChild(hScroller);

	AddChild(bottomLine);
	SetFeel(B_NORMAL_WINDOW_FEEL);
	Show();
}
/*-------------------------------------------------------------------------*/
void OutputWin::SetBitmap(BBitmap *bmp)
{
	/*ein neues Bitmap wird sofort an das View, welches das Bitmap zeichnet 
	weitergegeben*/
	bitmapView->SetBitmap(bmp);
	/* und ausnahmsweise mal ein korrektes Lock des Fensters 
	(damit das Zeichnen nicht irgenwie mittendrin passiert und 
	ein unschönes falkern ernsteht*/
	if (Lock())
	{
		//und zeicihne alles
		bitmapView->Draw(bitmapView->Bounds());
		Unlock();
	}

}

void 
OutputWin::MessageReceived(BMessage* msg){
	int16 tmp;
	switch(msg->what)
	{
	 //wenn irgenein Menueeintrag aus dem Skaliermenue angelklickt wurde
	 case SCALE:
	 	//raussuchen, welche skalierung gewünscht wurde
	 	msg->FindInt16("to",&tmp);
	 	//printf("Scale to\t%d Prozent\n",tmp);
	 	//und das ans Bitmapview weitergeben, welches das ganze dann umsetzt
	 	bitmapView->SetScale(tmp);
	 break;
	 default:
	 	BWindow::MessageReceived(msg);
	 	break;
	}
}
/*-------------------------------------------------------------------------*/



void OutputWin::Quit() 
{
	/* alert the application of our impending demise */
	be_app_messenger.SendMessage(OUTPUT_WIN_QUIT);
	BWindow::Quit();
}
/*-------------------------------------------------------------------------*/

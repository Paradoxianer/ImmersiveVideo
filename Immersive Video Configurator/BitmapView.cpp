#include <Window.h>

#include <stdio.h>

#include "BitmapView.h"

BitmapView::BitmapView(BRect frame,	const char	*name, uint32 resizeMask, uint32 flags)	: BView(frame, name, resizeMask, flags= B_WILL_DRAW | B_FRAME_EVENTS|B_PULSE_NEEDED)
{
	drawBitmap=NULL;
	faktor=1;
	bmpStartPoint=BPoint(0,0);
	scrollBars[0]=NULL;
	scrollBars[1]=NULL;
	SetHighColor(255,255,255,255);
	
}
BitmapView::~BitmapView()
{
}

void BitmapView::SetBitmap(BBitmap *bmp)
{
	drawBitmap=bmp;
	if (bmp!=NULL){
		if(Window()->Lock()){
			if (scrollBars[0])
				scrollBars[0]->SetRange(0,(drawBitmap->Bounds().Width()/faktor)-Bounds().Width());
			if (scrollBars[1])
				scrollBars[1]->SetRange(0,(drawBitmap->Bounds().Height()/faktor)-Bounds().Height());
			Window()->Unlock();
		}
	}

/*	if(Window()->Lock()){
		ResizeTo(faktor*drawBitmap->Bounds().Width(),faktor*drawBitmap->Bounds().Height());
		Window()->Unlock();
	}*/
}

void BitmapView::FrameResized(float	width,	float	height)
{
	if(Window()->Lock())
	{
		if (scrollBars[0])
			scrollBars[0]->SetRange(0,(drawBitmap->Bounds().Width()/faktor)-Bounds().Width());
		if (scrollBars[1])
			scrollBars[1]->SetRange(0,(drawBitmap->Bounds().Height()/faktor)-Bounds().Height());
		Window()->Unlock();
	}
	Draw(Bounds());
}

BBitmap	*BitmapView::GetBitmap()
{
	return drawBitmap;
}

void BitmapView::Draw(BRect	updateRect)
{
	Window()->Lock();
	//FillRect(updateRect);
	BRect source=BRect(faktor*updateRect.left,faktor*updateRect.top,faktor*updateRect.right,faktor*updateRect.bottom);
	source.OffsetBy(bmpStartPoint.x*faktor,bmpStartPoint.y*faktor);
	//SetViewBitmap(drawBitmap,source,updateRect);
	if (drawBitmap != NULL)
	{
		DrawBitmap(drawBitmap,source,updateRect);
	}

	float bWidth=drawBitmap->Bounds().Width()/faktor;
	float bHeight=drawBitmap->Bounds().Height()/faktor;
	if (bWidth<updateRect.Width())
	{
		FillRect(BRect(bWidth,0,updateRect.Width(),updateRect.Height()));
	}
	if (bHeight<updateRect.Height())
	{
		FillRect(BRect(0,bHeight,bWidth,updateRect.Height()));
	}

/*	if(source.Width()<updateRect.Width()){
		FillRect(BRect(source,0,updateRect.Width(),source.Height()-updateRect.Height()));
	}*/

	Window()->Unlock();
//	BView::Draw(updateRect);
}

void BitmapView::SetScale(int percent)
{
	faktor=100.0/(float)percent;
	if(Window()->Lock())
	{
		if (scrollBars[0])
		{
			scrollBars[0]->SetRange(0,(drawBitmap->Bounds().Width()/faktor)-Bounds().Width());
			printf("SetRange(%lf,%lf)\n",0,(drawBitmap->Bounds().Width()/faktor)-Bounds().Height());
			scrollBars[0]->SetSteps((drawBitmap->Bounds().Width()/faktor)/100,(drawBitmap->Bounds().Width()/faktor)/10);
		}
		if (scrollBars[1])
		{
			scrollBars[1]->SetRange(0,(drawBitmap->Bounds().Height()/faktor)-Bounds().Height());
			printf("SetRange(%lf,%lf)\n",0,(drawBitmap->Bounds().Height()/faktor)-Bounds().Height());
			scrollBars[1]->SetSteps((drawBitmap->Bounds().Height()/faktor)/100,(drawBitmap->Bounds().Height()/faktor)/10);
		}
			
		Window()->Unlock();
	}
	Draw(Bounds());

//	 ResizeTo(faktor*drawBitmap->Bounds().Width(),faktor*drawBitmap->Bounds().Height());
}
void BitmapView::ScrollBy(float dh,float dv)
{
	//printf("BPoint by: x=%f\ty=%f\n",dh,dv);
	bmpStartPoint+=BPoint(dh,dv);
	Draw(Bounds());
}
void BitmapView::ScrollTo(BPoint where)
{
//	printf("BPoint to: x=%f\ty=%f\n",where.x,where.y);
	if (where.x==0)
		bmpStartPoint.y=where.y;
	else
		bmpStartPoint.x=where.x;
	Draw(Bounds());
}


void BitmapView::RegisterScrollBar(BScrollBar *scrlBar)
{
	if (scrlBar->Orientation()==B_HORIZONTAL)
		scrollBars[0]=scrlBar;
	else
		scrollBars[1]=scrlBar;
}

void BitmapView::MessageReceived(BMessage* msg)
{
 	Window()->MessageReceived(msg);
}


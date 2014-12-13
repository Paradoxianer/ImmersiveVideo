#include <media/MediaRoster.h>
#include <media/MediaDefs.h>
#include <media/MediaNode.h>
#include <stdio.h>
#include "NavigationView.h"

#include "Cursors.h"

NavigationView::NavigationView(NavigationVideoFilter *navigationsFilter): BView(BRect(0,0,720,526),"NavigationView",B_FOLLOW_ALL_SIDES,B_NAVIGABLE_JUMP|B_NAVIGABLE)
//NavigationView::NavigationView(): BView(BRect(0,0,720,526),"NavigationView",B_FOLLOW_ALL_SIDES,B_NAVIGABLE_JUMP|B_NAVIGABLE)
{
	startDrag	= NULL;
	left		= new BCursor(cursorLeft);
	right		= new BCursor(cursorRight);
	up			= new BCursor(cursorUp);
	down		= new BCursor(cursorDown);
	leftup		= new BCursor(cursorLeft_Up);
	rightup		= new BCursor(cursorRight_Up);
	leftdown	= new BCursor(cursorLeft_Down);
	rightdown	= new BCursor(cursorRightDown);
	if (navigationsFilter != NULL)
	{
		BParameterWeb *web=navigationsFilter->Web();
		BParameterGroup *all	= web->GroupAt(0);
		printf("all->CountGroups %ld\n",all->CountGroups());
		BParameterGroup *speed	= all->GroupAt(1);
		xSpeed=speed->ParameterAt(0);
		ySpeed=speed->ParameterAt(1);		
	}
	else
	{
		status_t		err		= B_OK;
		BMediaRoster 	*roster	= BMediaRoster::Roster(&err);
		BParameterWeb	*web	= new BParameterWeb();
		if (!roster || (err != B_OK))
		{
			//** ERROR Check
			shutdown_media_server();
			launch_media_server();
		}
		int32 ln_count=30;
		live_node_info *ln_info = new live_node_info[ln_count];
		roster->GetLiveNodes(ln_info,&ln_count,NULL,NULL,"NavigationVideoFilter",0);
		if (ln_count>0)
		{
			//take the first Node we coud find
			roster->GetParameterWebFor(ln_info[0].node,&web);
			printf("web->CountGroups %ld\n",web->CountGroups());
			BParameterGroup *all	= web->GroupAt(0);
			printf("all->CountGroups %ld\n",all->CountGroups());
			BParameterGroup *speed	= all->GroupAt(1);
			xSpeed=speed->ParameterAt(0);
			ySpeed=speed->ParameterAt(1);		
		}
	}
}

void NavigationView:: MouseDown(BPoint point)
{
	startDrag=new BPoint(point);
}

void NavigationView::MouseMoved(BPoint point, uint32 transit, const BMessage *message)
{
	if (startDrag)
	{
		float deltaX 	= point.x-startDrag->x;
		float x_speed	= deltaX/10;
		float deltaY 	= point.y-startDrag->y;
		float y_speed	= deltaY/10;
		float distance	= (deltaX*deltaX)+(deltaY*deltaY);
		if (distance>10)
		{
			xSpeed->SetValue(&x_speed,sizeof(x_speed),0);
			ySpeed->SetValue(&y_speed,sizeof(y_speed),0);
			float alpha	= deltaY/deltaX;
			float beta	= deltaX/deltaY;
			if ((beta > -0.5) && (beta <0.5))
			{
				if (deltaY>0)
					SetViewCursor(down,true);
				else
					SetViewCursor(up,true);
			}
			else if ((beta<=1) || (beta>=1))
			{
				if ((alpha >- 0.5) && (alpha < 0.5))
				{
					if (deltaX>0)
						SetViewCursor(right,true);
					else
						SetViewCursor(left,true);
				}
				else if (deltaX>0)
				{
					if (deltaY>0)
						SetViewCursor(rightdown,true);
					else
						SetViewCursor(rightup,true);
				}
				else
				{
					if (deltaY>0)
						SetViewCursor(leftdown,true);
					else
						SetViewCursor(leftup,true);
				}	
			}
			
		}
	}
}

void NavigationView::MouseUp(BPoint point)
{
	float x_speed	= 0;
	float y_speed	= 0;
	SetViewCursor(B_CURSOR_SYSTEM_DEFAULT);
	xSpeed->SetValue(&x_speed,sizeof(x_speed),0);
	ySpeed->SetValue(&y_speed,sizeof(y_speed),0);
	startDrag=NULL;
}

void NavigationView:: KeyDown(const char *bytes, int32 numBytes)
{
}

void NavigationView::KeyUp(const char *bytes, int32 numBytes)
{
}
void NavigationView::AttachedToWindow(void)
{
}

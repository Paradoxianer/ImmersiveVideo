#ifndef NAVIGATION_VIEW_H
#define NAVIGATION_VIEW_H

#include <app/Cursor.h>
#include <interface/View.h>
#include <interface/Point.h>
#include <media/ParameterWeb.h>

#include "NavigationVideoFilter.h"

class NavigationView : public BView
{

public:
//						NavigationView();
						NavigationView(NavigationVideoFilter *navigationsFilter);
	virtual	void		MouseDown(BPoint point);
	virtual	void		MouseMoved(BPoint point, uint32 transit, const BMessage *message);
	virtual	void		KeyDown(const char *bytes, int32 numBytes);
	virtual	void		KeyUp(const char *bytes, int32 numBytes);
	virtual	void		MouseUp(BPoint point);
	virtual	void		AttachedToWindow(void);


protected:
			BPoint*		startDrag;
			BCursor*	left, *right, *up, *down,*leftup,*rightup,*leftdown,*rightdown;
			BParameter	*xSpeed, *ySpeed;
private:
};
#endif

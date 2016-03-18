#ifndef _BITMAP_VIEW_H
#define _BITMAP_VIEW_H

#include <View.h>
#include <Bitmap.h>
#include <ScrollBar.h>




class BitmapView : public BView {
public: BitmapView(BRect	frame, const char	*name, uint32 resizeMask,uint32	flags = B_WILL_DRAW | B_FRAME_EVENTS||B_PULSE_NEEDED);
	virtual			~BitmapView();
	virtual	void	SetBitmap(BBitmap *bmp);
	virtual void	FrameResized(float width, float height);
	virtual void	Draw(BRect updateRect);
	BBitmap			*GetBitmap(void);
	void			SetScale(int percent);
	void			RegisterScrollBar(BScrollBar *scrlBar);
	void			ScrollBy(float dh,float dv);
	virtual void	ScrollTo(BPoint where); 
 	void MessageReceived(BMessage* msg);

private:


private:
	BBitmap*		drawBitmap;
	float			faktor;
	BScrollBar		*vScroller;
	BScrollBar		*hScroller;
	BPoint			bmpStartPoint;
	BScrollBar		*scrollBars[2];
		
};

#endif

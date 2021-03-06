#ifndef _BITMAP_FILTER_h
#define _BITMAP_FILTER_h

#include <View.h>

class BitmapView : public BView
{
public:
	BitmapView(BBitmap* bitmap, BRect frame, const char* name,
		uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW);
	~BitmapView();
	
	BBitmap* Bitmap();
	void SetBitmap(BBitmap* bitmap);
	
	void AttachedToWindow();
	void Draw(BRect updateRect);
	void ResizeToPreferred();
	void GetPreferredSize(float* width, float* height);
	
private:
	BBitmap* m_pBitmap;
};

#endif /* _BitmapView_h */
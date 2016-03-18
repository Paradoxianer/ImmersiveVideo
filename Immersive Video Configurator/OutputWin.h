/* PluginWin.h */
#include <Bitmap.h>
#include <Locker.h>
#include <Message.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <ScrollBar.h>

#include <View.h>
#include <Window.h>

#include "BitmapView.h"


class OutputWin : public BWindow
{
 public:
 	OutputWin(BWindow* hand);
 	void SetBitmap(BBitmap *bmp);
 	void MessageReceived(BMessage* msg);
	void Quit();
	 
 private:
 	BitmapView*		bitmapView;
 	BWindow*		handler;
	BMenuBar		*bottomLine;
	BScrollBar		*vScroller;
	BScrollBar		*hScroller;
};


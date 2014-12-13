//
// ImmersiveVideo Plugin
//
//
#ifndef __I_2_PLUGIN_H__
#define __I_2_PLUGIN_H__
#include <app/Handler.h>
#include <kernel/image.h>
#include <interface/Bitmap.h>
#include <interface/Point.h>
#include <interface/Rect.h>
#include <interface/View.h>
#include <media/RealtimeAlloc.h>
#include "ImmersiveVideoPlugin.h"
#include "VideoDrawView.h"

class ImmersiveVideoPlugin2 : public ImmersiveVideoPlugin
{ 
	public:
	ImmersiveVideoPlugin2(image_id id);
	~ImmersiveVideoPlugin2();
	/**saves all necesary Options in the BMessage Objekt into!!
	deep deep means if it shoud go rekursiv into the Options strukture*/
	virtual	status_t		Archive(BMessage *into, bool deep = true) const; 
	/* Init is called after the Plugin was loaded, here you can 
	do all init stuff you want to
	*/
	virtual		void		Init(void);
	/** Init with a BMessage as argument is called after the Plugin was loaded
		and if there was a config file (flattend BMessage)
		in /boot/home/config/settings/Immersive Video/config
		make shure that all things you need hav a valid value.
		Tipp:
		the view are stores automatikally all Views attached to him
		so if you have eg. a BTextView*  myTextView added under the
		Name "MyTextView" you can
		init it with : myTextView=view->FindChild("MyTextView");
	*/
	virtual		void		Init(BMessage *message);
	virtual		void		SetConfigBitmap(BBitmap* bitmap){configBitmap=bitmap;};
	virtual		VideoDrawView*		GetConfigView(void){return configView;};

	virtual		void		MessageReceived(BMessage *message);

		VideoDrawView		*configView;

		BBitmap		*configBitmap;

	protected:
};
#endif

//
// Panoflex Plugin
//
//
#ifndef __SPLUGIN_H__
#define __SPLUGIN_H__
#include <View.h>
#include <Bitmap.h>
#include <Rect.h>
#include <image.h>
#include <Window.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include "ImmersiveVideoPlugin.h"


class SpiegelPlugin: public ImmersiveVideoPlugin{ 
	public:
	SpiegelPlugin(image_id id);
	const char*	GetName(void);
	BBitmap*	GetIcon(void);
	const char*	GetHelpFile(void);
	const char*	GetVersion(void);
	void		Init(void);
	void		Init(BMessage *message);
	void		MessageReceived(BMessage *message);
	void		Run();
	
	BPopUpMenu *menu;

};
#endif

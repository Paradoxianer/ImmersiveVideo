//
// ImmersiveVideo Plugin
//
//
#ifndef __FPLUGIN_H__
#define __FPLUGIN_H__
#include <View.h>
#include <Rect.h>
#include <image.h>
#include <Window.h>
#include <TextControl.h>
#include "ImmersiveVideoPlugin.h"


class FisheyePlugin: public ImmersiveVideoPlugin{ 
	public:
		FisheyePlugin(image_id id);
		const char*	GetName(void);
		const char*	GetHelpFile(void);
		const char*	GetVersion(void);
		void		Init(void);
		void		Init(BMessage *message);
		void		MessageReceived(BMessage *message);
		void		Run();

		BTextControl		*dx,*dy,*rap;
};
#endif

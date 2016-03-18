//
// ImmersiveVideo Standart Window class
//
// This class defines the hello world window.
//
#ifndef __PWINDOW_H__
#define __PWINDOW_H__


#include <storage/FindDirectory.h>
#include <interface/ListView.h>
#include <interface/Window.h>
#include <interface/ScrollView.h>
#include <interface/StringView.h>
#include <interface/TextControl.h>
#include <interface/PictureButton.h>
#include "PluginWin.h"	
#include "TranslatorSavePanel.h"
#include "OutputWin.h"

#define PRINTF printf
class ImmersiveVideoWindow : public BWindow {
	public:
							ImmersiveVideoWindow(BRect frame);
							ImmersiveVideoWindow(BRect frame, entry_ref *ref);
							~ImmersiveVideoWindow();
		virtual bool		QuitRequested();
		virtual void		MessageReceived(BMessage *message);
		status_t			Save(BMessage *message);
		void				SelectionChanged(BMessage *message);		
		void				SetInputBounds(BRect *rect);
		BRect				*GetInPutBounds();
		BRect				*GetOutPutBounds();
		status_t			SaveConfig();
//		status_t			testInRequest;

		
	protected:
		PluginWin			*pluginWindow;
	
	private:
		void				_InitWindow(void);
		void				_Init(void);
		void				LoadPlugins(void);		

		float				relation;
		int					unitType;
		
		BMenuBar			*menubar;
		BMenuItem			*saveitem;
		BMenu				*edit;
		BMessage			*savemessage;
		



		BListView			*listView;
		BScrollView			*scrollView;
		
		BStringView			*inputHeight;
		BStringView			*inputWidth;
		BTextControl		*outputHeight;
		BTextControl		*outputWidth;		
		BPictureButton		*depent;

};
#endif

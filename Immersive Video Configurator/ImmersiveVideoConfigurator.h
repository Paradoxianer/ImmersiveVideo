//
// TextApp class
//
// This class, derived from BApplication, defines the
// Hello World application itself.
//
/*
	Copyright 1999, Be Incorporated.   All Rights Reserved.
	This file may be used under the terms of the Be Sample Code License.
*/

#ifndef __ImmersiveVideoCONFIGURATOR_H__
#define __ImmersiveVideoCONFIGURATOR_H__

#include <interface/Alert.h>
#include <Application.h>
#include <Bitmap.h>
#include <FilePanel.h>
#include <Message.h>
#include <Point.h>
#include <translation/TranslationUtils.h>
#include <translation/TranslatorRoster.h>
#include <translation/BitmapStream.h>

#include "TranslatorSavePanel.h"
#include "ImmersiveVideoWindow.h"
#include "ImmersiveVideoPlugin.h"


extern const char *APP_SIGNATURE;

class ImmersiveVideoConfigurator : public BApplication {
	public:
							ImmersiveVideoConfigurator();
		void				ArgvReceived(int32 argc, char** argv);
		virtual void		MessageReceived(BMessage *message);
		virtual void		RefsReceived(BMessage *message);
		virtual void		Calc();
				void		CalcInt32();
				void		CalcInt16();
				void		CalcInt8();				
		static	int32		CalcAnstupser(void *);
//		status_t			Save(entry_ref *ref, BMessage *archive);
		status_t			Save(const char *name, BMessage *archive);
		status_t			Load(entry_ref *ref);
		status_t			Open(entry_ref *ref);
		status_t			Export(BMessage *message);

	private:
		ImmersiveVideoWindow		*window;
		OutputWin			*outputWindow;
		BFilePanel 			*openPanel;
		BFilePanel 			*savePanel;
		TranslatorSavePanel *exportPanel;

		BFile				*saveFile;
		BBitmap				*original;
		BBitmap				*neu;
		BPoint				*lTable;
		uint32				lTableSize;
};
#endif

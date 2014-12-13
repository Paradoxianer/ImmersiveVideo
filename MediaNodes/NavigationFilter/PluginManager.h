#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <app/Handler.h>
#include <interface/Rect.h>
#include <media/Controllable.h>
#include <media/MediaEventLooper.h>
#include <media/ParameterWeb.h>
#include <media/RealtimeAlloc.h>
#include <storage/Directory.h>
#include <storage/Entry.h>
#include <storage/File.h>
#include <storage/FindDirectory.h>
#include <storage/Path.h>
#include <support/List.h>


#include "Constants.h"
#include "NavigationVideoFilter.h"


class PluginManager:	public BLooper
{
public:
						PluginManager(NavigationVideoFilter *iVFilter);
						~PluginManager(void);
	BPoint*				GetLookUpTable(void);
	uint32*				GetLookUpTableInt(void);
	void				SetInputRect(BRect *inputRect){iDim=inputRect;};
	void				SetOutputRect(BRect *outputRect){oDim=outputRect;};
	BRect*				GetInputRect(void){return iDim;};
	BRect*				GetOutputRect(void){return oDim;};
	BParameterWeb	 	*GetParameterWeb(void){return web;};
	void				MessageReceived(BMessage *message);
	void				RunCalc();
	//void				Run();	
	/*eigentlich Funktionen von BControllable, aber da diese Klasse
	für die Lookuptable zuständig ist übernimmt diese Klasse
	alle Configurartionseinstellungen.
	Um das zu gewährleisten werden in der aufrufenden Klasse
	die Funtktionen gemappt.
	*/
	virtual status_t		GetParameterValue(int32 id, bigtime_t *last_change,
							void *value, size_t *size);
	virtual void			SetParameterValue(int32 id, bigtime_t when,
							const void *value, size_t size);

private:
	void					Init();
	void					ChangeSelect();

	void					LoadPlugins(void);
	BParameterWeb 			*web;
	//BPoint				*lTable;
	BRect					*iDim;
	BRect					*oDim;
	BList					*plugins;
	int32					select;
	bigtime_t				fLastSelectionChange;
	enum					{ I_V_PLUGIN };
	NavigationVideoFilter	*immersiveVideoFilter;	
	rtm_pool				*pool;
	uint32					*lTable;
};

#endif

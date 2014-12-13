#include <posix/string.h>
#include <posix/stdio.h>

#include "PluginManager.h"
#include "ImmersiveVideoPlugin.h"


PluginManager::PluginManager(NavigationVideoFilter *iVFilter):BLooper()
{
	INFO("PluginManager::PluginManager()\n");
	immersiveVideoFilter=iVFilter;
	Run();
	Init();
}
PluginManager::~PluginManager()
{
/*	delete iDim;
	delete oDim;*/
}
void PluginManager::Init()
{
	INFO("PluginManager::Init()\n");
	iDim=new BRect(0,0,800,600);
	oDim=new BRect(0,0,800,600);
	web=NULL;
	plugins=new BList();
	LoadPlugins();
}
void PluginManager::LoadPlugins()
{	
	INFO("PluginManager::LoadPlugins()\n");

	//notwendig um die standartisierte grafische Schnittstelle ein zu binden
	//**vielleicht vorher web löschen ?? wenn dynamisches Nachladen geplant
	if (web) delete web;
	web = new BParameterWeb();
	BParameterGroup *main = web->MakeGroup(Name());
	BDiscreteParameter *state = main->MakeDiscreteParameter(I_V_PLUGIN, B_MEDIA_RAW_VIDEO, "Plugin", "Plugin");

	//einen Systemweit eindeutige ID für Plugins
	image_id		addonId;
	//die Fehlervariable
   	status_t 		err = B_NO_ERROR; 
	BFile		*file;
	BEntry		*entry=new BEntry();
	ImmersiveVideoPlugin* aktPlugin=NULL;
	ImmersiveVideoPlugin* (*NewImmersiveVideoPlugin)(image_id);
	BPath *path=new BPath();
	BPath *pluginPath=new BPath();
	BPath *configPath=new BPath();
	find_directory(B_COMMON_SETTINGS_DIRECTORY,pluginPath,false,NULL);
	find_directory(B_COMMON_SETTINGS_DIRECTORY,configPath,false,NULL);
	pluginPath->Append("Immersive Video/plugins");
	configPath->Append("Immersive Video/config");
	BDirectory pluginDir(pluginPath->Path());
	BDirectory configDir(configPath->Path());
	DEBUG("Laden der Plugins..\n");
	INFO("PLuginspfad: %s\n",pluginPath->Path());
	//** toDo jeden einzelnen Eintrag freigeben.. 
	plugins->MakeEmpty();

	while( err == B_NO_ERROR )
	{
		// nächsten Verweis auf eine Datei auslesen
		err = pluginDir.GetNextEntry(entry, TRUE );			
		/*1. testen ob es ein gültiger Verweis ist
		2. einen Pfad(+Dateiname) von dem aktuellen Verweis erstellen lassen ->Path*/
		if ((entry->InitCheck()==B_NO_ERROR)&&(entry->GetPath(path)==B_NO_ERROR)&&(err==B_NO_ERROR))
		{
			addonId = load_add_on( path->Path() );
			if( addonId < 0 )
			{
				//Wenns schief ging, kein Poblem, dann wars irgendeine andere Datei
				//printf( "load_add_on( %s ) failed\n", path.Path() );
				
			}
			else
			{
				//wenns geklappt hatt dann kleine Naricht auf stdout
				INFO( "\t%s\t\tloaded\n", path->Leaf());
				/*überprüfen ob es ein Addon war, welches unsere Schnittstelle entspricht
				Schnittstelle heist hier NewImmersiveVideoPlugin und es muss mit dem Protypen
				(void **)NewImmersiveVideoPlugin gekennzeichnet*/
				if( get_image_symbol( addonId, 
									"NewImmersiveVideoPlugin", 
									B_SYMBOL_TYPE_TEXT, 
									(void **)&NewImmersiveVideoPlugin) )
				{
					// wenn wir ein ungülitges Plugin geladen hatten, wieder wegwerfen ;-)
					//printf( "get_image_symbol( NewImmersiveVideoPlugin ) failed\n" );
					unload_add_on(addonId );
				}
				else
				{
					//ansonsten von dem geleadenen Plugin mit der addonID ein gültiges Objekt erzeugen
					aktPlugin = (*NewImmersiveVideoPlugin)( addonId );
					if( !aktPlugin )
					{
						ERROR( "failed to create a new plugin\n" );
					}
					else
					{
						//wenn alles geklappt hat dann wir das Plugin initialisiert
						file=new BFile(&configDir,aktPlugin->GetName(),B_READ_ONLY);
						if (file->InitCheck()==B_OK)
						{
							//BMessage laden welches alle Einstellungen enhält
							BMessage *archive=new BMessage();
							archive->Unflatten(file);
							//daraus das Plugin alles bauen lassen, was es benötigt
							aktPlugin->Init(archive);
							/*delete archive;
							archive=NULL;*/
						}
						else
						{
							aktPlugin->Init();
						}
						//aktuelles Plugin der Liste hinzufügen
						plugins->AddItem(aktPlugin);
						//Plugin in das ParameterWeb eintragen
						Lock();
						AddHandler(aktPlugin);
						Unlock();
						state->AddItem(plugins->CountItems(), aktPlugin->GetName());
						delete file;
						file=NULL;
					}
				}
			}
		}
	}
	select=1;
	fLastSelectionChange = system_time();
	RunCalc();

}

status_t 
PluginManager::GetParameterValue(
	int32 id, bigtime_t *last_change, void *value, size_t *size)
{
	INFO("PluginManager::GetParameterValue()\n");

	if (id != I_V_PLUGIN)
		return B_BAD_VALUE;

	*last_change = fLastSelectionChange;
	*size = sizeof(int32);
	*((uint32 *)value) = select;

	return B_OK;
}
void
PluginManager::SetParameterValue(
	int32 id, bigtime_t when, const void *value, size_t size)
{
	INFO("PluginManager::SetParameterValue()\n");
	if ((id != I_V_PLUGIN) || !value || (size != sizeof(int32)))
		return;

	if (*(int32 *)value == select)
		return;

	select = *(uint32 *)value;
	fLastSelectionChange = when;

	//** Auswahl hat sich geändert!
//	ChangeSelect();
	//hier findet der Aufruf zur Neuberechnung statt
	RunCalc();
	//BroadcastNewParameterValue(fLastSelectionChange, I_V_PLUGIN, &select, sizeof(select));
}
BPoint*	PluginManager::GetLookUpTable(void)
{
	INFO("PluginManager::GetLookUpTable()\n");
	ImmersiveVideoPlugin *aktPlugin=(ImmersiveVideoPlugin *)plugins->ItemAt(select-1);
	return aktPlugin->GetLookUpTable();
}
uint32	*PluginManager::GetLookUpTableInt(void)
{
	INFO("PluginManager::GetLookUpTableInt()\n");
	uint32 tmp;
	Lock();
	ImmersiveVideoPlugin *aktPlugin=(ImmersiveVideoPlugin *)plugins->ItemAt(select-1);
	BPoint *tmpLookUp=aktPlugin->GetLookUpTable();
	if (!lTable) rtm_free(lTable);
	if (!pool)rtm_delete_pool(pool);
	uint32 lTableSize=(oDim->Width()+1)*(oDim->Height()+1);
	uint32 size=lTableSize*sizeof(tmp)*2;
	rtm_create_pool(&pool,size,"lookUpTable");
	lTable=(uint32 *)rtm_alloc(pool,size);
	//falls es Probleme beim Erstellen des Echtzeitspeichers gab
	if (lTable==NULL) lTable=new uint32[lTableSize];
	for (int i=0;i<lTableSize;i++)
	 	lTable[i]=(uint32)(tmpLookUp[i].x)+((uint32)tmpLookUp[i].y)*(iDim->Width()+1);
	Unlock();
	return lTable;	
}


void PluginManager::MessageReceived(BMessage *message) 
{ 
	INFO("PluginManager::MessageReceived()\n\t");
	switch ( message->what )
	{ 
		case LOOK_UP_TABLE_CALCULATED:
			ImmersiveVideoPlugin *pli=(ImmersiveVideoPlugin *)plugins->ItemAt(select-1);
			oDim=pli->oDim;
			oDim->PrintToStream();	
			status_t err = immersiveVideoFilter->HandleMessage(SET_LOOKUPTABLE, NULL, 0);
			ERROR("Errorcode: %s\n",strerror(err));
			//immersiveVideoFilter->LoadConfig();
	} 
} 

/*void PluginManager::Run()
{
//	ChangeSelect();
	RunCalc();
}*/

void PluginManager::ChangeSelect()
{
	INFO("PluginManager::ChangeSelect()\n");
	ImmersiveVideoPlugin *pli=(ImmersiveVideoPlugin *)plugins->ItemAt(select-1);
	//**!!!Speicherleck Loockuptable vorher löschen_lassen!!!
	if (pli!=NULL)
	{
	}
}


void PluginManager::RunCalc()
{
	INFO("PluginManager::RunCalc()\n\t");
	ImmersiveVideoPlugin *pli=(ImmersiveVideoPlugin *)plugins->ItemAt(select-1);
	if (pli!=NULL)
	{
		//**nötigen Werte an das Plugin übergeben
		iDim->PrintToStream();
		pli->SetInputDimension(iDim);
		pli->SetOutputDimension(iDim);
		/*Für die eigentliche Berechnung in einen eigenen Thread anlegen damit das Fenster
		und die Anwendung nicht blockiert werden*/
		thread_id pluginSam = spawn_thread((thread_func)(pli->RunCalculation), "PluginSam", B_NORMAL_PRIORITY, pli);
		if (pluginSam >= 0)
		{
			//thread starten, wenn Anlegen erfolgreiche
			INFO("Berechnung beginnt\n");
			resume_thread(pluginSam);
		}
		else 
		{
			//ansonsten halt ohne extra thread einfach so starten
			pli->Run();
		}
	}
}

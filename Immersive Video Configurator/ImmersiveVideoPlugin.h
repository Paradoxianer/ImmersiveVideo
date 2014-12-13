//
// ImmersiveVideo Plugin
//
//
#ifndef __IPLUGIN_H__
#define __IPLUGIN_H__
#include <app/Handler.h>
#include <kernel/image.h>
#include <interface/Bitmap.h>
#include <interface/Point.h>
#include <interface/Rect.h>
#include <interface/View.h>
#include <media/RealtimeAlloc.h>
#include <support/Locker.h>


class ImmersiveVideoPlugin : public BHandler
{ 
	public:
	ImmersiveVideoPlugin(image_id id);
	~ImmersiveVideoPlugin();
	/*saves all necesary Options in the BMessage Objekt into!!
	deep deep means if it shoud go rekursiv into the Options strukture*/
	virtual	status_t		Archive(BMessage *into, bool deep = true) const; 
	//returns the name This name occures in the List and in the Windowtitle
	virtual	const char*		GetName(void);
	/*returns a Icon wich shoud symbolize the kind of the plugin
	You dont need to overite this Mehtod because the standart Implementation
	returns the icon of the file as the Icon.
	If you dont want to have an Icon just overite it and return NULL	
	*/
				BBitmap*	GetIcon(void);
	/*This shoud return a correkt path to the helpfile from your Plugin.
	The Path shoud be relative to the Immersive Video Configurator or a 
	absolute Path
	*/
	virtual		const char*	GetHelpFile(void);
	/*this return the Variable view this ist the view wich shows up in 
	the Main programm wenn this plugin is choosen*/	
				BView*		GetView(void);
	/*this return the Variable view this ist the view wich shows up in 
	the Main programm wenn this plugin is choosen*/
				BView*		GetAboutView(void);
	/* You dont need to overite this!!
	This funktion simple returns a Pointer to the LookUpTable*/
				BPoint*		GetLookUpTable();
	/*deletes the lookuptable to prefent Memorylacks*/
				status_t	DeleteLookUpTable();
	/*Returns a String wich discribe the Version of the Plugin like
	  1.0.5a 
	*/
	virtual		const char*	GetVersion(void);
	/*
		is called after the Plugin was created, because a Plugin isn´t create (default) by the Konstucktor
		initialize all variables
	*/
	virtual		void		Init(void);
	virtual		void		Init(BMessage *message);
	virtual		void		SetInputDimension(BRect* inDim);
	virtual		void		SetOutputDimension(BRect* outDim);
	virtual		void		Run(void);
	static		int32		RunCalculation(void *castToThis);
	virtual		void 		Undo();
    virtual		void		Redo();
	virtual		bool		UnAndRedoPossible(void);
	virtual		bool		ToolsPossible(void);
	virtual		void		DoCopy();
    virtual		void		DoPaste();
	virtual		void		DoCut();
	virtual		void		MessageReceived(BMessage *message);


		BView		*view;
		BView		*aboutView;
		BPoint		*lTable;
		uint64		lTableSize;
		uint64		status;
		BRect		*inDim,*outDim;
		BMessenger	*messenger;


	protected:
		void		LoadIcon();
		image_id	mId;
		thread_id	pluginSam;
		BBitmap *	bmp;
		rtm_pool	*pool;
		BLocker		*locker;
};
#endif

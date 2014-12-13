#ifndef MEDIA_NODE_MANAGER_H
#define MEDIA_NODE_MANAGER_H

#include <app/Looper.h>
#include <media/MediaRoster.h>
#include <storage/FilePanel.h>
#include <support/List.h>

#include "PluginManager.h"
#include "NavigationVideoFilter.h"
#include "ImmersiveVideoFilter.h"
#include "VideoDrawConsumer.h"
#include "MediaSplitter.h"

#include "MediaNodeWrapper.h"


const int32		NUMBER_OF_INPUT_OUTPUT=15;

class MediaNodeManager:public BLooper
{

public:	
								MediaNodeManager();

	//these Methods are responsible to detect alle in the System available Nodes and return a list of them
	virtual	BList*				GetInputList(void);
	virtual	BList*				GetOutputList(void);



	//these Methods connect, set Timer and start the Metanodes wich are aktually set
	//if there are some "old" Nodes are running the are stopped and disconnected at first
	virtual	status_t			Prepare(void);
	virtual	status_t			Start(void);

//	virtual	status_t			Start(void);
	//these Methodes stop and disconnet the specified MethaNode
	virtual	status_t			Stop(void);


	//returns the actual set of MethaNodes
	virtual	MediaNodeWrapper*	GetInput(void){return (MediaNodeWrapper*) inputList->ItemAt(selectedInput);};
//	virtual	MetaNode*			GetFilter(void){return filter;};
	virtual	MediaNodeWrapper*	GetOutput(void){return (MediaNodeWrapper*) outputList->ItemAt(selectedOutput);};

	/*here the user/Programm can set the aktual MethaNodes.. if you use the BMessage
	you can set your own MethaNodes*/
	virtual	status_t				SetInput(MediaNodeWrapper* input);
//	virtual	status_t				SetFilter(MediaNodeWrapper* filter);
	virtual	status_t				SetOutput(MediaNodeWrapper* output);

	//here the user/Programm can set the aktual MethaNodes in reference to the returned List in the GetMethods
	virtual	status_t				SetInput(int32 index);
//	virtual	status_t				SetFilter(int32 index);
	virtual	status_t				SetOutput(int32 index);

	virtual BView*					GetInputConfigView(void);
	virtual BView*					GetOutputConfigView(void);

	//+++++++++++Looper related Stuff
	virtual bool					QuitRequested(void);
	virtual	void					MessageReceived(BMessage *message);
		
		
	
			PluginManager			*GetPluginManager(void){return pluginManager;};
//			NavigationsVideoFilter	*GetNaviFilter(void){return navFilter;};
			VideoDrawConsumer		*GetVideoDrawer(void){return videoDrawer;};
protected:
			void					Init(void);
			void					BuildOwnNodes(void);
			BView*					BuildConfigView(MediaNodeWrapper* mNWrapper);
			
			BMediaRoster			*mediaRoster;

			BFilePanel*				openPanel;
			BFilePanel*				savePanel;

			MediaNodeWrapper		*input;
			MediaNodeWrapper		*decoder;
			MediaNodeWrapper		*entzerrer;
			MediaNodeWrapper		*splitter;
			MediaNodeWrapper		*highlighter;
			MediaNodeWrapper		*navigator;
			MediaNodeWrapper		*darsteller;
			MediaNodeWrapper		*encoder;
			MediaNodeWrapper		*output;

			
			MediaNodeWrapper		*timeSource;
			
			BList					*inputList;
			BList					*outputList;
			
			bool					highlighted;
			
			int32					selectedInput;
			int32					selectedOutput;

			PluginManager			*pluginManager;
			NavigationVideoFilter	*navFilter;
			ImmersiveVideoFilter	*immersiveVideoFilter;
			VideoDrawConsumer		*videoDrawer;
			MediaSplitter			*theSplitter;


private:
};

#endif


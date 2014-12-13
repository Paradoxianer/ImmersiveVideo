/* PluginWin.h */
#include <Window.h>
#include <View.h>
#include <Box.h>
#include <Message.h>
#include <Locker.h>
#include <Messenger.h>
#include "ImmersiveVideoPlugin.h"

class PluginWin : public BWindow
{
 public:
 	PluginWin(BHandler* hand, ImmersiveVideoPlugin* pli);
 	void SetPlugin(ImmersiveVideoPlugin* pli);
 	void MessageReceived(BMessage* msg);
	void Quit();
	 
 private:
 	ImmersiveVideoPlugin*	plugin;
 	BView*			pluginView;
 	BBox*			bb;
 	BMessenger		*messenger;
};


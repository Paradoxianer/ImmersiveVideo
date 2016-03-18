#include <ListItem.h> 
#include <Point.h>
#include <View.h>
#include <Window.h>

#include "ImmersiveVideoPlugin.h"

const rgb_color kHighlight = {155,155,255,255};
const rgb_color kBlackColor = {0,0,0,255};
const rgb_color kMedGray = {100,100,100,255};
class PluginItem : public BListItem 
{
	public:
		PluginItem(ImmersiveVideoPlugin* plugin);
		virtual void DrawItem(BView *owner, BRect frame,bool complete = false);
		ImmersiveVideoPlugin* GetPlugin(void);
		virtual void Update(BView *owner, const BFont *font);

	private:
		BPoint					bitmapPoint;
		ImmersiveVideoPlugin*	plugin;
};

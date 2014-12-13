#include <interface/Point.h>
#include "ConfigTab.h"

ConfigTab::ConfigTab(RVCWindow *window)
{
	INFO("ConfigTab","ConfigTab()");
	rvcWindow=window;
}

void ConfigTab::SetLabel(const char* newLabel)
{
	INFO("ConfigTab","SetLabel(%s)",newLabel);
	label=(char *)newLabel;
}

void ConfigTab::Select(BView *owner)
{
	INFO("ConfigTab","Select()");
	BTab::Select(owner);
	//**connecting Input to Output ;-)
	/*configView=rvcWindow->pluginManager->GetVideConfigView();
	if (configView != NULL)
	{
		rvcWindow->darsteller->SetVideoView(configView);
	}
	else
	{
		rvcWindow->darsteller->SetVideoView(rvcWindow->videoView);
	}*/
}

void ConfigTab::Deselect(void)
{
	INFO("ConfigTab","Deselect()");
	BTab::Deselect();
}

void ConfigTab::DrawLabel(BView* owner, BRect tabFrame)
{
	INFO("ConfigTab","DrawLabel()");

	float restBreite=tabFrame.Width()-owner->StringWidth(label);
	font_height fheight; 
	owner->GetFontHeight(&fheight);
	float resthoehe=tabFrame.Height()-fheight.ascent;
	BPoint stringstart(tabFrame.left+restBreite/2,tabFrame.top+(fheight.ascent+resthoehe/2));	
	owner->DrawString(label,stringstart);
}

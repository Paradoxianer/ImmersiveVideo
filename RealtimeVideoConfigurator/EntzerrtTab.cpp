#include "EntzerrtTab.h"
EntzerrtTab::EntzerrtTab(RVCWindow *window)
{
	INFO("EntzerrtTab","EntzerrtTab()");
	rvcWindow=window;
}

void EntzerrtTab::SetLabel(const char* newLabel)
{
	INFO("EntzerrtTab","SetLabel(%s)",newLabel);
	label=(char *)newLabel;
}

void EntzerrtTab::Select(BView *owner)
{
	INFO("EntzerrtTab","Select()");
	BTab::Select(owner);
	//**connecting Input to Output ;-)
}

void EntzerrtTab::Deselect(void)
{
	INFO("EntzerrtTab","Deselect()");
	BTab::Deselect();
	//**disconnecting Input from Output 
}

void EntzerrtTab::DrawLabel(BView* owner, BRect tabFrame)
{
	INFO("EntzerrtTab","DrawLabel()");
	float restBreite=tabFrame.Width()-owner->StringWidth(label);
	font_height fheight; 
	owner->GetFontHeight(&fheight);
	float resthoehe=tabFrame.Height()-fheight.ascent;
	BPoint stringstart(tabFrame.left+restBreite/2,tabFrame.top+(fheight.ascent+resthoehe/2));	
	owner->DrawString(label,stringstart);
}

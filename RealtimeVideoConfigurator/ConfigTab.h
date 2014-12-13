#include <be/interface/TabView.h>
#include "RVCWindow.h"


class ConfigTab : public BTab 
{
public:
						ConfigTab(RVCWindow *);
		const char* 	Label() const{return label;};
virtual	void			SetLabel(const char* newLabel);


virtual	void			Select(BView* owner);
virtual	void			Deselect();

virtual void 			DrawLabel(BView* owner, BRect tabFrame);
protected:

		char* 			label;	
		RVCWindow*		rvcWindow;
};

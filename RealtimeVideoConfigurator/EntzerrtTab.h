#include <be/interface/TabView.h>
#include "RVCWindow.h"



class EntzerrtTab : public BTab 
{
public:
						EntzerrtTab(RVCWindow*);
		const char* 	Label() const{return label;};
virtual	void			SetLabel(const char* newLabel);


virtual	void			Select(BView* owner);
virtual	void			Deselect();

virtual void 			DrawLabel(BView* owner, BRect tabFrame);


protected:

		char* 			label;
		RVCWindow*		rvcWindow;
};

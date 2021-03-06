#ifndef FILTER_ADDON_H
#define FILTER_ADDON_H
#include "RVCDefs.h"
#include <media/MediaAddOn.h>

#define TOUCH(x) ((void)(x))

extern "C" _EXPORT BMediaAddOn *make_media_addon(image_id you);

const Debugger *myDebugger= new Debugger("VideoWriter");

class MediaAddOn : public BMediaAddOn
{
public:
						MediaAddOn(image_id imid);
	virtual 			~MediaAddOn();
	
	virtual	status_t	InitCheck(const char **out_failure_text);

	virtual	int32		CountFlavors();
	virtual	status_t	GetFlavorAt(int32 n, const flavor_info ** out_info);
	virtual	BMediaNode	*InstantiateNodeFor(
							const flavor_info * info,
							BMessage * config,
							status_t * out_error);

	virtual	status_t	GetConfigurationFor(BMediaNode *node, BMessage *message)
								{ TOUCH(node); TOUCH(message); return B_OK; }
	virtual	status_t	SaveConfigInfo(BMediaNode *node, BMessage *message)
								{ TOUCH(node); TOUCH(message); return B_OK; }

	virtual	bool		WantsAutoStart() { return false; }
	virtual	status_t	AutoStart(int in_count, BMediaNode **out_node,
								int32 *out_internal_id, bool *out_has_more)
								{	TOUCH(in_count); TOUCH(out_node);
									TOUCH(out_internal_id); TOUCH(out_has_more);
									return B_ERROR; }

private:
	status_t			fInitStatus;
	flavor_info			fFlavorInfo;
	media_format		fMediaInFormat;
	media_format		fMediaOutFormat;
};

#endif

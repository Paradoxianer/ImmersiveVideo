#ifndef MEDIA_NODE_WRAPPER_H
#define MEDIA_NODE_WRAPPER_H

#include <be/support/Archivable.h>

#include <be/media/MediaDefs.h>
#include <be/media/MediaAddOn.h>
#include <be/media/MediaRoster.h>


#include "RVCDefs.h"

class MediaNodeWrapper: BArchivable
{

public:
								MediaNodeWrapper(BMessage *archive);
								MediaNodeWrapper(dormant_node_info *mediaNode);
								MediaNodeWrapper(media_node *media_Node);
								MediaNodeWrapper(media_node *media_Node,char *name);

								~MediaNodeWrapper(void);
								
	static	BArchivable			*Instantiate(BMessage *archive);
	virtual	status_t			Archive(BMessage *archive, bool deep = true) const;
	
	virtual	media_node*			GetNode(void);
	virtual	dormant_node_info*	GetDormantNode(void){return dormantMediaNode;};
								
	virtual status_t			ReleaseNode(void);
	
	virtual	bool				IsAlive(void){return alive;};
	
	virtual	char*				GetName(void);				
	
	virtual uint32				GetID(void){return (uint32)this;};

protected:
	virtual	void				Init(void);
			dormant_node_info	*dormantMediaNode;
			media_node 			mediaNode;	
			media_input			*freeInput;
			media_output		*freeOutput;
			bool				alive;
			bool				wasAlive;
			BMediaRoster		*mediaRoster;
			char				*theName;
private:
};
#endif

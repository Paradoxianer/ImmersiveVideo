#ifndef META_NODE_H
#define META_NODE_H

#include <be/app/Message.h>
#include <be/support/Archivable.h>
#include <be/support/List.h>
#include <be/media/MediaRoster.h>

#include "MediaNodeConnection.h"
#include "MediaNodeWrapper.h"


class MetaNode : public  BArchivable
{

public:
								MetaNode();
								MetaNode(char *name);

								MetaNode(BMessage *archive);
								~MetaNode();

	virtual	status_t			Archive(BMessage *archive, bool deep = true) const;
	static	BArchivable			*Instantiate(BMessage *from);
	
	virtual void				AddNode(MediaNodeWrapper *node);
	virtual void				AddConnection(MediaNodeConnection *connection);

	virtual void				RemoveNode(MediaNodeWrapper *node);
	virtual void				RemoveConnection(MediaNodeConnection *connection);

	virtual void				SetName(char *metaNodeName){name=metaNodeName;};
	virtual	char*				GetName(void){return name;};	

	virtual BView*				GetConfigView(void);			

	virtual void				Setescription(char *metaNodeDescription){description=metaNodeDescription;};
	virtual char*				GetDescription(void){return description;};

	virtual MediaNodeWrapper	*FirstNode(void);
	virtual MediaNodeWrapper	*LastNode(void);

	virtual bool				IsConnected(void){return connected;};
	virtual bool				IsRunning(void){return started;};

	virtual void				SetRef(entry_ref *ref);
	
	virtual void				Connect(void);
	virtual void				Start(void);
	virtual void				Stop(void);
	virtual void				Disconnect(void);
	

protected:
			status_t			Init(void);

private:
			bool				connected;
			bool				started;
			char 				*name;
			char				*description;
			BList				*connections;
			BList				*nodes;
			BMediaRoster		*mediaRoster;	
};
#endif

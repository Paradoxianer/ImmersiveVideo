#ifndef MESSAGE_NAVIGATOR_H
#define MESSAGE_NAVIGATOR_H

#include <app/Message.h>
#include <app/MessageQueue.h>
#include <media/MediaDefs.h>
#include <media/MediaNode.h>

const uint32	MEDIA_NODE			='MEDN';
const uint32	METHA_NODE			='MTHN';

class MessageNavigator
{

public:
							MessageNavigator(BMessage *treeMessage);
	virtual	media_node*		GetCurrentOutputNode(void);
	virtual	media_node*		GetNextInput(void);
	virtual void			Reset(void){queue = saveQueue;};

protected:
	virtual	void			Init(void);
	virtual	status_t		AddBranch(BMessage *branch);
			media_node		*currentOutput;
			media_node		*currentInput;
			BMessageQueue	*queue;
			BMessageQueue	*saveQueue;

			BMessage		*nodeTree;
private:
};
#endif

#ifndef NODE_MANGAER_H
#define NODE_MANGAER_H

#include <app/Message.h>
#include <media/MediaDefs.h>
#include <media/MediaRoster.h>

#include "MessageNavigator.h"

const uint	max_in_out	=	5;

class NodeManager
{

public:
			NodeManager();
			~NodeManager();

		//**later such things like ... instantiate...


		void 		Connect(void);
		media_node*	ConnectInput(BMessage *inputMetaNode);
		void		ConnectFilter(media_node *inputNode,media_node *outputNode,BMessage* filterMetaNode);
		media_node*	ConnectOutput(BMessage *outputMetaNode);

		void		Start(void);
		status_t	StartInput(BMessage *inputMetaNode);
		status_t	StartFilter(BMessage* filterMetaNode);
		status_t	StartOutput(BMessage *outputMetaNode);
		
		void		Stop(void);
		status_t	StopInput(BMessage *inputMetaNode);
		status_t	StopFilter(BMessage* filterMetaNode);
		status_t	StopOutput(BMessage *outputMetaNode);

		void		Disconnect(void);
		media_node*	DisonnectInput(BMessage *inputMetaNode);
		status_t	DisconnectFilter(media_node *inputNode,media_node *outputNode,BMessage* filterMetaNode);
		media_node*	DisconnectOutput(BMessage *outputMetaNode);
		
		void		SetNodeConfiguration(BMessage *nodeConfig);
		BMessage*	GetNodeConfiguration(void){return configMessage;};

protected:
		status_t	Init(void);


private:
		BMessage			*configMessage;
		BMediaRoster		*mediaRoster;
		media_node			timeSourceNode;

};
#endif

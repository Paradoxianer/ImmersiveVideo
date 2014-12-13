#ifndef MEDIA_NODE_CONNECTION_H
#define MEDIA_NODE_CONNECTION_H

#include <app/Message.h>
#include <media/MediaDefs.h>
#include <media/MediaRoster.h>
#include <media/MediaNode.h>

#include <support/Archivable.h>

#include "RVCDefs.h"
#include "MediaNodeWrapper.h"

const	int32	max_count_inout		=	15;

class MediaNodeConnection : BArchivable
{

public:
								MediaNodeConnection(MediaNodeWrapper *beginning,MediaNodeWrapper *ending);
								MediaNodeConnection(BMessage *archive);
								MediaNodeConnection(void);
								~MediaNodeConnection(void);

	static	BArchivable*		Instantiate(BMessage *archive);
	virtual	status_t			Archive(BMessage *archive, bool deep=true) const;
									
	virtual	status_t			Connect(void);
	virtual	status_t			Disconnect(void);
	virtual	bool				IsConnected(void){return connected;};
	virtual	status_t			SetConnectionFormat(void);
	virtual	media_format*		GetConnectionFormat(void);

	virtual	status_t			SetEnding(MediaNodeWrapper *media_node);
	virtual	MediaNodeWrapper*	GetEnding(void){return ending;};
	virtual	status_t			SetBeginning(MediaNodeWrapper *media_node);
	virtual	MediaNodeWrapper*	GetBeginning(void){return beginning;};;

protected:
			void				Init(void);
			MediaNodeWrapper	*beginning,*ending;
			bool				connected;
			BMediaRoster		*mediaRoster;
			media_format 		*connectionFormat;
			media_output		*mediaOutput;
			media_input			*mediaInput;
			media_node			timeSourceNode;

private:
};
#endif

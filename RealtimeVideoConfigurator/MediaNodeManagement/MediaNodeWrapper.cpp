#include "MediaNodeWrapper.h"

MediaNodeWrapper::MediaNodeWrapper(BMessage *archive)
{
	INFO("MediaNodeWrapper","MediaNodeWrapper()");
	Init();
	ssize_t size;
	//**What about errors??
	archive->FindBool("MediaNodeWrapper::alive",&alive);
	archive->FindData("MediaNodeWrapper::mediaNode",B_RAW_TYPE,(const void **)&mediaNode,&size);
	archive->FindData("MediaNodeWrapper::dormantMediaNode",B_RAW_TYPE,(const void **)&dormantMediaNode,&size);
}
MediaNodeWrapper::MediaNodeWrapper(dormant_node_info *mediaNode)
{
	INFO("MediaNodeWrapper","MediaNodeWrapper(%ld)",mediaNode->addon);
	Init();
	dormantMediaNode = mediaNode;
}
MediaNodeWrapper::MediaNodeWrapper(media_node *media_Node)
{
	INFO("MediaNodeWrapper","MediaNodeWrapper(%ld)",media_Node->node);
	Init();
	mediaNode	= *media_Node;
	//**need check if it´s really alive
	if (!mediaRoster->GetDormantNodeFor(mediaNode,dormantMediaNode))
		dormantMediaNode=NULL;
	alive		= true;
	wasAlive	= true;
}
MediaNodeWrapper::MediaNodeWrapper(media_node *media_Node,char *name)
{
	INFO("MediaNodeWrapper","MediaNodeWrapper(%ld)",media_Node->node);
	Init();
	mediaNode	= *media_Node;
	//**need check if it´s really alive
	if (!mediaRoster->GetDormantNodeFor(mediaNode,dormantMediaNode))
		dormantMediaNode=NULL;
	theName		= name;
	alive		= true;
	wasAlive	= true;
}

MediaNodeWrapper::~MediaNodeWrapper(void)
{
	ReleaseNode();
}

void MediaNodeWrapper::Init(void)
{
	INFO("MediaNodeWrapper","Init()");
	status_t err		= B_OK;
	mediaRoster			= BMediaRoster::CurrentRoster();
	if (!mediaRoster)
	{
		ERROR("MediaNodeWrapper","MediaNodeWrapper::Init() - %s","Can´t Get Current Roster");
	}
	mediaNode			= media_node::null;
	dormantMediaNode	= NULL;
	alive				= false;
	wasAlive			= false;
	theName				= NULL;
}

status_t MediaNodeWrapper::Archive(BMessage *archive, bool deep = true) const
{
	INFO("MediaNodeWrapper","Archive()");
	archive->AddBool("MediaNodeWrapper::alive",alive);
	archive->AddData("MediaNodeWrapper::mediaNode",B_RAW_TYPE,&mediaNode,sizeof(mediaNode),true);
	archive->AddData("MediaNodeWrapper::dormantMediaNode",B_RAW_TYPE,dormantMediaNode,sizeof(*dormantMediaNode),true);
	archive->AddString("class", "MediaNodeWrapper");
}

BArchivable	*MediaNodeWrapper::Instantiate(BMessage *archive)
{
	INFO("MediaNodeWrapper","Instantiate()");
	if ( validate_instantiation(archive, "MediaNodeWrapper"))
		return new MediaNodeWrapper(archive);
	return NULL; 
}

media_node* MediaNodeWrapper::GetNode(void)
{
	INFO("MediaNodeWrapper","GetNode()");
	status_t err	= B_OK;
	if (!alive)
	{
		if ((mediaRoster) && (dormantMediaNode))
		{
			err = mediaRoster->InstantiateDormantNode(*dormantMediaNode, &mediaNode); 
			if (err==B_OK)
				alive=true;
			else
				ERROR("MediaNodeWrapper","Instantiate Error - %s",strerror(err));
		}
	}
	return &mediaNode;
}
status_t MediaNodeWrapper::ReleaseNode(void)
{
	INFO("MediaNodeWrapper","ReleaseNode()");
	status_t err	= B_OK;
	if (alive)
	{
		if ((mediaRoster)&&(mediaNode!=media_node::null))
		{
			err = mediaRoster->StopNode(mediaNode, 0, true); 
			if (wasAlive)
			{
				err = mediaRoster->ReleaseNode(mediaNode); 
				mediaNode=media_node::null;
			}
		}
	}
	return err;
}

char* MediaNodeWrapper::GetName(void)
{
	INFO("MediaNodeWrapper","GetName()");
	if (dormantMediaNode)
		return dormantMediaNode->name;
	else
	{
		return theName;
	}
}

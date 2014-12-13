#include <interface/Alert.h>
#include <interface/Box.h>
#include <interface/Button.h>
#include <media/MediaDefs.h>
#include <media/MediaNode.h>
#include <media/TimeSource.h>
#include <media/MediaTheme.h>


#if B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 


#include "MetaNode.h"


MetaNode::MetaNode()
{
	INFO("MetaNode","MetaNode()");
	Init();
}

MetaNode::MetaNode(char *nodeName)
{
	INFO("MetaNode","MetaNode(%s)",nodeName);
	Init();
	name=nodeName;
}

MetaNode::MetaNode(BMessage *archive):BArchivable(archive)
{
	INFO("MetaNode","MetaNode(BMessage *archive)");

	Init();
	int32 i=0,j=0;
	BMessage *tmpMessage=new BMessage();
	connections	= new BList();
	nodes		= new BList();
	while (archive->FindMessage("node",i,tmpMessage)==B_OK)
	{
		i++;
		MediaNodeWrapper *tmpNode=new MediaNodeWrapper(tmpMessage);
		nodes->AddItem(tmpNode);
		//**extract the id
	}
	while (archive->FindMessage("connection",i,tmpMessage)==B_OK)
	{
		i++;
		MediaNodeConnection *tmpConnection=new MediaNodeConnection(tmpMessage);
		tmpMessage->FindInt32("Beginning",&j);
		tmpConnection->SetBeginning((MediaNodeWrapper *)nodes->ItemAt(j));
		tmpMessage->FindInt32("Ending",&j);
		tmpConnection->SetEnding((MediaNodeWrapper *)nodes->ItemAt(j));
		connections->AddItem(tmpConnection);
	}
}


MetaNode::~MetaNode()
{
	INFO("MetaNode","~MetaNode()");

}
status_t MetaNode::Archive(BMessage *archive, bool deep = true) const
{
	INFO("MetaNode","Archive()");
	int32 				i;
	status_t 			err	=B_OK;
	MediaNodeWrapper 	*tmpNode;
	MediaNodeConnection *tmpConnection;
	//** need to store class
	BMessage			*tmpMessage;
	for (i=0;i<nodes->CountItems();i++)
	{
		tmpNode = (MediaNodeWrapper *)nodes->ItemAt(i);
		tmpNode->Archive(tmpMessage,true);
		tmpMessage->AddInt32("_ID",i);
		archive->AddMessage("node",tmpMessage);
	}
	for (i=0;i<connections->CountItems();i++)
	{
		tmpConnection = (MediaNodeConnection *)connections->ItemAt(i);
		tmpConnection->Archive(tmpMessage,true);
		tmpMessage->AddInt32("Beginning",nodes->IndexOf(tmpConnection->GetBeginning()));
		tmpMessage->AddInt32("Ending",nodes->IndexOf(tmpConnection->GetEnding()));
		archive->AddMessage("connection",tmpMessage);
	}
	archive->AddString("class", "MetaNode");
	return err;
}

BArchivable	*MetaNode::Instantiate(BMessage *archive)
{
	INFO("MetaNode","Instantiate()");

	if ( validate_instantiation(archive, "MetaNode"))
		return new MetaNode(archive);
	return NULL; 
}
status_t MetaNode::Init()
{
	INFO("MetaNode","Init()");

	status_t 	err	= B_OK;
	connections		= new BList();
	nodes			= new BList();
	//** need Memory error check???
	name			= NULL;
	description		= NULL;
	mediaRoster		= BMediaRoster::CurrentRoster();
	connected		=false;
	started			=false;
	return err;
}

void MetaNode::AddNode(MediaNodeWrapper *node)
{
	INFO("MetaNode","AddNode()");
	nodes->AddItem(node);
}
void MetaNode::AddConnection(MediaNodeConnection *connection)
{
	INFO("MetaNode","AddConnection()");
	connections->AddItem(connection);
}

void MetaNode::RemoveNode(MediaNodeWrapper *node)
{
	INFO("MetaNode","RemoveNode()");
	nodes->RemoveItem(node);
}
void MetaNode::RemoveConnection(MediaNodeConnection *connection)
{
	INFO("MetaNode","RemoveConnection()");
	connections->RemoveItem(connection);
}
BView* MetaNode::GetConfigView()
{
	INFO("MetaNode","GetConfigView()");
	BParameterWeb		*configWeb;
	dormant_flavor_info flavorInfo;
	status_t err=B_OK;
	MediaNodeWrapper *tmpNode=(MediaNodeWrapper* )nodes->ItemAt(0);
	dormant_node_info *dnInfo=tmpNode->GetDormantNode();
	if (dnInfo)
		mediaRoster->GetDormantFlavorInfoFor(*dnInfo,&flavorInfo);
	err = mediaRoster->GetParameterWebFor(*tmpNode->GetNode(),&configWeb); 
	if (err == B_OK)
	{
		if ((flavorInfo.kinds&B_FILE_INTERFACE) != 0)
		{
			BMessage *sender  = new BMessage(OPEN_FILE);
			sender->AddPointer("source",this);
			BView *container=new BView(BRect(0,0,200,40),"MetaNodeContainerView",B_FOLLOW_TOP|B_FOLLOW_LEFT,0);
			container->SetViewColor(B_TRANSPARENT_COLOR);
			container->AddChild(new BButton(BRect(0,0,80,20),"File",_T("File"),sender));
			if (configWeb)
			{
				BView *mediaView=BMediaTheme::ViewFor(configWeb,NULL,NULL);
				if (mediaView)
				{
					mediaView->MoveTo(85,0);
					container->AddChild(mediaView);
				}
			}
			return container;
		}
		else
			return BMediaTheme::ViewFor(configWeb,NULL,NULL);
	}
	else
	{
		if ((flavorInfo.kinds&B_FILE_INTERFACE) != 0)
		{
			BMessage *sender  = new BMessage(OPEN_FILE);
			sender->AddPointer("source",this);
			BBox *container=new BBox(BRect(0,0,200,40),"MetaNodeContainerView",B_FOLLOW_TOP|B_FOLLOW_LEFT,0);
			container->AddChild(new BButton(BRect(0,0,80,20),"File",_T("File"),sender));
			if (configWeb)
			{
				BView *mediaView=BMediaTheme::ViewFor(configWeb,NULL,NULL);
				if (mediaView)
				{
					mediaView->MoveTo(85,0);
					container->AddChild(mediaView);
				}
			}
			return container;
		}
		else
			return NULL;
	}
}


MediaNodeWrapper* MetaNode::FirstNode(void)
{
	if (nodes->CountItems()>0)
		return (MediaNodeWrapper*)nodes->FirstItem();
	else
		return NULL;
}

MediaNodeWrapper* MetaNode::LastNode(void)
{
	if (nodes->CountItems()>0)
		return (MediaNodeWrapper*)nodes->LastItem();
	else
		return NULL;
}

void MetaNode::SetRef(entry_ref *ref)
{
	INFO("MetaNode","SetRef()");
	status_t err;
	MediaNodeWrapper *tmpNode=(MediaNodeWrapper* )nodes->ItemAt(0);

	if (tmpNode)
	{
		bigtime_t duration;
		media_node *file_node= tmpNode->GetNode();
		err = mediaRoster->SetRefFor(*file_node,*ref,false,&duration);
		if (err)
		{
			ERROR("MetaNode", "SetRefFor %s", strerror(err));
		}
	}
	Connect();
	Start();
}

void MetaNode::Connect()
{
	INFO("MetaNode","Connect()");
	MediaNodeConnection *tmpConnection;
	int32 		i	= 0;
	DE_BUG("MetaNode","%s CountItems= %ld",name,connections->CountItems());
	for (i=0;i<connections->CountItems();i++)
	{
		tmpConnection = (MediaNodeConnection *)connections->ItemAt(i);
		if (tmpConnection)tmpConnection->Connect();
	}
	connected=true;
}
void MetaNode::Start()
{
	INFO("MetaNode","Start()");
	DE_BUG("MetaNode","%s CountItems= %ld",name,nodes->CountItems());
	MediaNodeWrapper *tmpNode;
	int32 i;
	status_t err=B_OK;
	for (i=nodes->CountItems();i>0;i--)
	{
		tmpNode=(MediaNodeWrapper *)nodes->ItemAt(i-1);
		DE_BUG("MetaNode","StartNode %ld",tmpNode->GetNode()->node);
		err = mediaRoster->StartNode(*tmpNode->GetNode(),0);
		if (err != B_OK)
			ERROR("MetaNode","- StartNode %s",strerror(err));
	}
	started=true;
}

void MetaNode::Stop()
{
	INFO("MetaNode","Stop()");
	MediaNodeWrapper *tmpNode;
	status_t 	err	= B_OK;
	int32 		i	= 0;
	DE_BUG("MetaNode","%s CountItems= %ld",name,nodes->CountItems());
	for (i = 0;i<nodes->CountItems();i++)
	{
			tmpNode=(MediaNodeWrapper *)nodes->ItemAt(i);
			err = mediaRoster->StopNode(*tmpNode->GetNode(),0,true);
			if (err != B_OK)
					ERROR("MetaNode","- StopNode = %s",strerror(err));
			//err = tmpNode->ReleaseNode();
			/*if (err != B_OK)
					ERROR("ERROR -- ReleaseNode = %s",strerror(err));*/
	}
	started=false;
}
void MetaNode::Disconnect()
{
	INFO("MetaNode","Disconnect()");
	status_t err=B_OK;
	MediaNodeConnection *tmpConnection;
	int32 		i	= 0;
	for (i = 0;i<connections->CountItems();i++)
	{
		tmpConnection = (MediaNodeConnection *)connections->ItemAt(i);
		err = tmpConnection->Disconnect();
		if (err != B_OK)
			ERROR("MetaNode","- Disconnect %s",strerror(err));
	}
	connected=false;
}

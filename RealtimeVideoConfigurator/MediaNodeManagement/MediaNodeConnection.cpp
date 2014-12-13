#include "MediaNodeConnection.h"

MediaNodeConnection::MediaNodeConnection(void)
{
	INFO("MediaNodeConnection","MediaNodeConnection()");

}
MediaNodeConnection::MediaNodeConnection(BMessage *archive):BArchivable(archive)
{
	INFO("MediaNodeConnection","MediaNodeConnection()");

	ssize_t size;
	Init();
	archive->FindData("MediaNodeConnection::timeSourceNode",B_RAW_TYPE,(const void **)&timeSourceNode,&size);
	archive->FindBool("MediaNodeConnection::connected",&connected);
	//** if connected =ture connect all Stuff.. ;-)
}
MediaNodeConnection::MediaNodeConnection(MediaNodeWrapper *begin,MediaNodeWrapper *end)
{
	INFO("MediaNodeConnection","MediaNodeConnection(%s,%s)",begin->GetName(),end->GetName());

	Init();
	beginning	= begin;
	ending		= end;
}


MediaNodeConnection::~MediaNodeConnection(void)
{
	if (beginning)
		beginning->ReleaseNode();
	if (ending)
		beginning->ReleaseNode();
}

void MediaNodeConnection::Init(void)
{
	INFO("MediaNodeConnection","Init()");
	status_t err		= B_OK;
	mediaRoster			= BMediaRoster::CurrentRoster();
	beginning		 	= NULL;
	ending				= NULL;
	connected			= false;
	connectionFormat	= new media_format();
	mediaOutput			= new media_output[max_count_inout];
	mediaInput			= new media_input[max_count_inout];
}

BArchivable* MediaNodeConnection::Instantiate(BMessage *archive)
{
	INFO("MediaNodeConnection","Instantiate()");
	if ( validate_instantiation(archive, "MediaNodeConnection"))
		return new MediaNodeConnection(archive);
	return NULL; 
}
status_t MediaNodeConnection::Archive(BMessage *archive, bool deep=true) const
{
	INFO("MediaNodeConnection","Archive()");
	status_t err = B_OK;
	err = archive->AddString("class", "MediaNodeConnection");
	err = err | archive->AddData("MediaNodeConnection::timeSourceNode",B_RAW_TYPE,&timeSourceNode,sizeof(timeSourceNode),true);
	err = err | archive->AddBool("MediaNodeConnection::connected",connected);
	return err;
}

status_t MediaNodeConnection::SetConnectionFormat(void)
{
	//**Do we need to implement this?
	return B_ERROR;
}
media_format* MediaNodeConnection::GetConnectionFormat(void)
{
	//**Do we need to implement this?
	return NULL;
}
status_t MediaNodeConnection::Connect(void)
{
	INFO("MediaNodeConnection","Connect()");
	status_t err	= B_OK;
	if ((beginning) && (ending))
	{
		if (!connected)
		{
			//**Do this in a Loop check every outputformat and every inputformat
			int32 outputCount=0;
			err =  mediaRoster->GetFreeOutputsFor(*beginning->GetNode(),mediaOutput,max_count_inout, &outputCount);
			DE_BUG("MediaNodeConnection","GetFreeOutputsFor(%s,%ld,%ld)",beginning->GetName(),max_count_inout,outputCount);
			if (err==B_OK)
			{
				int32 inputCount=0;
				err = 	mediaRoster->GetFreeInputsFor(*ending->GetNode(),mediaInput,max_count_inout, &inputCount);
					DE_BUG("MediaNodeConnection","GetFreeInputsFor(%s,%ld,%ld)",ending->GetName(),max_count_inout,inputCount);
				if (err==B_OK)
				{
					/*connectionFormat->type = B_MEDIA_RAW_VIDEO;	
					connectionFormat->u.raw_video = media_raw_video_format::wildcard;*/
					*connectionFormat=mediaOutput->format;
					err = mediaRoster->Connect(mediaOutput->source, mediaInput->destination, connectionFormat, mediaOutput, mediaInput);
					if (err == B_OK)
					{
						err = mediaRoster->GetSystemTimeSource(&timeSourceNode);
						if (err!=B_OK)
							ERROR("MediaNodeConnection"," - GetSystemTimeSource %s",strerror(err));

						err = mediaRoster->SetTimeSourceFor((beginning->GetNode())->node,timeSourceNode.node);
						if (err!=B_OK)
							ERROR("MediaNodeConnection"," SetTimeSource %s",strerror(err));

						err = mediaRoster->SetTimeSourceFor((ending->GetNode())->node,timeSourceNode.node);
						//** if error dann versuch nen normale TimeSource zu bekommen.
						if (err!=B_OK)
							ERROR("MediaNodeConnection","- SetTimeSource %s",strerror(err));
						err=B_OK;
						connected=true;
					}
					else
						ERROR("MediaNodeConnection"," Connect from %s to %s : %s",beginning->GetName(),ending->GetName(),strerror(err));		
				}
				else
				{
					ERROR("MediaNodeConnection"," GetFreeInputs for %s: %s",ending->GetName(),strerror(err));
				}
			}
			else
			{
				ERROR("MediaNodeConnection","GetFreeOutputs for %s: %s",beginning->GetName(),strerror(err));
			}
		}
		else
		{
			err=B_MEDIA_ALREADY_CONNECTED;
		}
	}
	else
	{
		err=B_NO_INIT;
	}
	return err;
}
status_t MediaNodeConnection::Disconnect(void)
{
	INFO("MediaNodeConnection","Disconnect(void)");
	status_t err	= B_OK; 
	if (connected)
	{
		//stop all Nodes
		err = mediaRoster->StopNode(*beginning->GetNode(), 0, true);
		err = err | mediaRoster->StopNode(*ending->GetNode(), 0, true);
		if (err != B_OK)
		{
			ERROR("MediaNodeConnection","- cant stop Nodes beginning=%s ending=%s ",beginning->GetName(),ending->GetName());
			//ignore this fact...
			err=B_OK;
		}
		err = mediaRoster->Disconnect(beginning->GetNode()->node,mediaOutput->source,ending->GetNode()->node,mediaInput->destination);
		beginning->ReleaseNode();
		ending->ReleaseNode();
		if (err != B_OK)
			ERROR("ERROR - cant dsiconnect %s from %s",beginning->GetName(),ending->GetName());
	}
	else
	{
		err=B_MEDIA_NOT_CONNECTED;
	}
	return err;	
}

status_t MediaNodeConnection::SetEnding(MediaNodeWrapper *media_node)
{
	INFO("MediaNodeConnection","SetEnding()");
	bool needRestart	= true;
	if (connected)
	{
		Disconnect();
		needRestart		= false;
	}
	delete ending;
	ending = media_node;
	if (needRestart)
	{
		Connect();	
	}
}

status_t MediaNodeConnection::SetBeginning(MediaNodeWrapper *media_node)
{
	INFO("MediaNodeConnection","SetBeginning()");
	status_t err=B_OK;
	bool needRestart	= true;
	if (connected)
	{
		Disconnect();
		needRestart		= false;
	}
	delete beginning;
	beginning = media_node;
	if (needRestart)
	{
		Connect();	
	}
	return err;
}


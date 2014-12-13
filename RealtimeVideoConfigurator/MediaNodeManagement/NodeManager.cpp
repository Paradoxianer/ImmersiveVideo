#include <interface/Alert.h>
#include <media/MediaDefs.h>
#include <media/MediaNode.h>
#include <media/TimeSource.h>
#if B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 


#include "NodeManager.h"

NodeManager::NodeManager()
{
	
}


NodeManager::~NodeManager()
{
}
status_t NodeManager::Init()
{
	status_t err			= B_OK;
	mediaRoster				= BMediaRoster::Roster(&err);
	if (err != B_OK) 
	{
		(new BAlert("", _T("Can't find the media roster"), "Quit"))->Go();
		return err;
	}	
	err = mediaRoster->GetTimeSource(&timeSourceNode);
	if (err != B_OK) {
		(new BAlert("", _T("Can't get a time source"), "Quit"))->Go();
		return err;
	}
	return err;
	
}
void NodeManager::Connect()
{
	BMessage *inputMethaNode=new BMessage();
	BMessage *filterMethaNode=new BMessage();
	BMessage *outuptMethaNode=new BMessage();
	if (configMessage->FindMessage("FilterNode",filterMethaNode) == B_OK) 
		ConnectFilter(ConnectInput(inputMethaNode),ConnectOutput(outuptMethaNode),filterMethaNode);
}

media_node* NodeManager::ConnectInput(BMessage *inputMetaNode)
{
	MessageNavigator	*connectNavigator	= new MessageNavigator(inputMetaNode);
	media_node			*outputNode			= NULL;
	media_node			*inputNode			= NULL;
	media_output		*videoOutput		= new media_output[max_in_out];
	media_input			*videoInput			= new media_input[max_in_out];
	int32 				count				= 1;
	status_t			err					= B_OK;
	media_format 		format;
	bool				done				=false;
	while (!done)
	{
		if ((outputNode=connectNavigator->GetCurrentOutputNode()) != NULL)
		{
			inputNode = connectNavigator->GetNextInput();
			if (inputNode != NULL)
			{
				err =  mediaRoster->GetFreeOutputsFor(*outputNode,videoOutput,max_in_out, &count);
		//		DEBUG("GetFreeOutputs: %s\n",strerror(err));
				//**testen ob alles ok..
				err = 	mediaRoster->GetFreeInputsFor(*inputNode,videoInput,max_in_out, &count);
		//		DEBUG("GetFreeInputs: %s\n",strerror(err));
				//**testen ob alles ok..
				format.type				= videoOutput->format.type;	
				format.u.raw_video		= media_raw_video_format::wildcard;
				format.u.encoded_video	= media_encoded_video_format::wildcard;
				err = mediaRoster->Connect(videoOutput->source, videoInput->destination, &format, videoOutput, videoInput);
			}
			else
			{
				done = true;
			}
		}
		else
		{
			inputNode=connectNavigator->GetNextInput();
		}
	}
	//we return the input node, because connectNavigarot returns the last correct media_node in there(if GetNextInput==NULL)
	return inputNode;
}

void NodeManager::ConnectFilter(media_node *filterInputNode,media_node *filterOutputNode,BMessage* filterMetaNode)
{
	MessageNavigator	*connectNavigator	= new MessageNavigator(filterMetaNode);
	media_node			*outputNode			= NULL;
	media_node			*inputNode			= NULL;
	media_output		*videoOutput		= new media_output[max_in_out];
	media_input			*videoInput			= new media_input[max_in_out];
	int32 				count				= 1;
	status_t			err					= B_OK;
	media_format 		format;
	bool				done				=false;
	while (!done)
	{
		if ((outputNode=connectNavigator->GetCurrentOutputNode()) != NULL)
			outputNode	= filterInputNode;
		inputNode = connectNavigator->GetNextInput();
		if (inputNode == NULL)
		{
			//if we connected the last output with the last input then we are done
			inputNode	= filterOutputNode;
			done		= true;
		}
		err =  mediaRoster->GetFreeOutputsFor(*outputNode,videoOutput,max_in_out, &count);
//		DEBUG("GetFreeOutputs: %s\n",strerror(err));
		//**testen ob alles ok..
		err = 	mediaRoster->GetFreeInputsFor(*inputNode,videoInput,max_in_out, &count);
//		DEBUG("GetFreeInputs: %s\n",strerror(err));
		//**testen ob alles ok..
		format.type				= videoOutput->format.type;	
		format.u.raw_video		= media_raw_video_format::wildcard;
		format.u.encoded_video	= media_encoded_video_format::wildcard;
		err = mediaRoster->Connect(videoOutput->source, videoInput->destination, &format, videoOutput, videoInput);
	}
}

media_node* NodeManager::ConnectOutput(BMessage *outputMetaNode)
{
	MessageNavigator	*connectNavigator	= new MessageNavigator(outputMetaNode);
	media_node			*outputNode			= NULL;
	media_node			*inputNode			= NULL;
	media_node			*firstNode			= NULL;
	media_output		*videoOutput		= new media_output[max_in_out];
	media_input			*videoInput			= new media_input[max_in_out];
	int32 				count				= 1;
	status_t			err					= B_OK;
	media_format 		format;
	bool				done				=false;
	while (!done)
	{
		if ((outputNode=connectNavigator->GetCurrentOutputNode()) != NULL)
		{
			inputNode = connectNavigator->GetNextInput();
			if (inputNode != NULL)
			{
				err =  mediaRoster->GetFreeOutputsFor(*outputNode,videoOutput,max_in_out, &count);
		//		DEBUG("GetFreeOutputs: %s\n",strerror(err));
				//**testen ob alles ok..
				err = 	mediaRoster->GetFreeInputsFor(*inputNode,videoInput,max_in_out, &count);
		//		DEBUG("GetFreeInputs: %s\n",strerror(err));
				//**testen ob alles ok..
				format.type				= videoOutput->format.type;	
				format.u.raw_video		= media_raw_video_format::wildcard;
				format.u.encoded_video	= media_encoded_video_format::wildcard;
				err = mediaRoster->Connect(videoOutput->source, videoInput->destination, &format, videoOutput, videoInput);
			}
			else
			{
				done 	= true;
			}
		}
		else
		{
			inputNode	= connectNavigator->GetNextInput();
			firstNode	= inputNode;
		}
	}
	//we return the first Node
	return firstNode;
}

status_t NodeManager::StartFilter(BMessage* filterMetaNode)
{
	MessageNavigator	*connectNavigator	= new MessageNavigator(filterMetaNode);
	media_node			*node				= NULL;
	status_t			err					= B_OK;
	media_format 		format;
	//solange wir noch Knoten finden sezte die Timesource und starte sie wenn ein Fehler auftritt abbrechen
	while (((node = connectNavigator->GetNextInput()) != NULL) && (err== B_OK))
	{
		err = mediaRoster->SetTimeSourceFor(node->node, timeSourceNode.node);
		if (err != B_OK) 
		{
			(new BAlert("", _T("Can't set the timesource for a node"), "Quit"))->Go();
		}
		else
		{
			err = mediaRoster->StartNode(*node,BTimeSource::RealTime());
			if (err != B_OK)
			{
				(new BAlert("", _T("Can't start node"), "Quit"))->Go();
			}
		}
	}
	return err;
}

status_t NodeManager::StartInput(BMessage *inputMetaNode)
{
	MessageNavigator	*connectNavigator	= new MessageNavigator(inputMetaNode);
	media_node			*node				= NULL;
	status_t			err					= B_OK;
	media_format 		format;
	//solange wir noch Knoten finden sezte die Timesource und starte sie wenn ein Fehler auftritt abbrechen
	while (((node = connectNavigator->GetNextInput()) != NULL) && (err== B_OK))
	{
		err = mediaRoster->SetTimeSourceFor(node->node, timeSourceNode.node);
		if (err != B_OK) 
		{
			(new BAlert("", _T("Can't set the timesource for a node"), "Quit"))->Go();
		}
		else
		{
			err = mediaRoster->StartNode(*node,BTimeSource::RealTime());
			if (err != B_OK)
			{
				(new BAlert("", _T("Can't start node"), "Quit"))->Go();
			}
		}
	}
	return err;
}



status_t NodeManager::StartOutput(BMessage *outputMetaNode)
{
	MessageNavigator	*connectNavigator	= new MessageNavigator(outputMetaNode);
	media_node			*node				= NULL;
	status_t			err					= B_OK;
	media_format 		format;
	//solange wir noch Knoten finden sezte die Timesource und starte sie wenn ein Fehler auftritt abbrechen
	while (((node = connectNavigator->GetNextInput()) != NULL) && (err== B_OK))
	{
		err = mediaRoster->SetTimeSourceFor(node->node, timeSourceNode.node);
		if (err != B_OK) 
		{
			(new BAlert("", _T("Can't set the timesource for a node"), "Quit"))->Go();
		}
		else
		{
			err = mediaRoster->StartNode(*node,BTimeSource::RealTime());
			if (err != B_OK)
			{
				(new BAlert("", _T("Can't start node"), "Quit"))->Go();
			}
		}
	}
	return err;
}


status_t NodeManager::StopInput(BMessage *inputMetaNode)
{
	MessageNavigator	*connectNavigator	= new MessageNavigator(inputMetaNode);
	media_node			*node				= NULL;
	status_t			err					= B_OK;
	media_format 		format;
	//solange wir noch Knoten finden sezte die Timesource und starte sie wenn ein Fehler auftritt abbrechen
	while (((node = connectNavigator->GetNextInput()) != NULL) && (err== B_OK))
	{
		err = mediaRoster->StopNode(*node,0, true);
		if (err != B_OK) 
		{
			(new BAlert("", _T("Can't stop node"), "Quit"))->Go();
		}
	}
	return err;
}

status_t NodeManager::StopFilter(BMessage* filterMetaNode)
{
	MessageNavigator	*connectNavigator	= new MessageNavigator(filterMetaNode);
	media_node			*node				= NULL;
	status_t			err					= B_OK;
	media_format 		format;
	//solange wir noch Knoten finden sezte die Timesource und starte sie wenn ein Fehler auftritt abbrechen
	while (((node = connectNavigator->GetNextInput()) != NULL) && (err== B_OK))
	{
		err = mediaRoster->StopNode(*node,0, true);
		if (err != B_OK) 
		{
			(new BAlert("", _T("Can't stop node"), "Quit"))->Go();
		}
	}
	return err;
}

status_t NodeManager::StopOutput(BMessage *outputMetaNode)
{
	MessageNavigator	*connectNavigator	= new MessageNavigator(outputMetaNode);
	media_node			*node				= NULL;
	status_t			err					= B_OK;
	media_format 		format;
	//solange wir noch Knoten finden sezte die Timesource und starte sie wenn ein Fehler auftritt abbrechen
	while (((node = connectNavigator->GetNextInput()) != NULL) && (err== B_OK))
	{
		err = mediaRoster->StopNode(*node,0, true);
		if (err != B_OK) 
		{
			(new BAlert("", _T("Can't stop node"), "Quit"))->Go();
		}
	}
	return err;
}

media_node* NodeManager::DisonnectInput(BMessage *inputMetaNode)
{
	MessageNavigator	*connectNavigator	= new MessageNavigator(inputMetaNode);
	media_node			*outputNode			= NULL;
	media_node			*inputNode			= NULL;
	media_output		*videoOutput		= new media_output[max_in_out];
	media_input			*videoInput			= new media_input[max_in_out];
	int32 				count				= 1;
	status_t			err					= B_OK;
	media_format 		format;
	bool				done				=false;
	while (!done)
	{
		if ((outputNode=connectNavigator->GetCurrentOutputNode()) != NULL)
		{
			inputNode = connectNavigator->GetNextInput();
			if (inputNode != NULL)
			{
				err =  mediaRoster->GetConnectedOutputsFor(*outputNode,videoOutput,max_in_out, &count);
		//		DEBUG("GetFreeOutputs: %s\n",strerror(err));
				//**testen ob alles ok..
				err = 	mediaRoster->GetConnectedInputsFor(*inputNode,videoInput,max_in_out, &count);
		//		DEBUG("GetFreeInputs: %s\n",strerror(err));
				//**testen ob alles ok..
				format.type				= videoOutput->format.type;	
				format.u.raw_video		= media_raw_video_format::wildcard;
				format.u.encoded_video	= media_encoded_video_format::wildcard;
				
		//		err = mediaRoster->Disconnect(0,videoOutput->node,videoOutput->source,videoInput->node, videoInput->destination);
			}
			else
			{
				done = true;
			}
		}
		else
		{
			inputNode=connectNavigator->GetNextInput();
		}
	}
	//we return the input node, because connectNavigarot returns the last correct media_node in there(if GetNextInput==NULL)
	return inputNode;
}

status_t NodeManager::DisconnectFilter(media_node *filterInputNode,media_node *filterOutputNode,BMessage* filterMetaNode)
{
	MessageNavigator	*connectNavigator	= new MessageNavigator(filterMetaNode);
	media_node			*outputNode			= NULL;
	media_node			*inputNode			= NULL;
	media_output		*videoOutput		= new media_output[max_in_out];
	media_input			*videoInput			= new media_input[max_in_out];
	int32 				count				= 1;
	status_t			err					= B_OK;
	media_format 		format;
	bool				done				=false;
	while (!done)
	{
		if ((outputNode=connectNavigator->GetCurrentOutputNode()) != NULL)
		{
			outputNode	= filterInputNode;
			inputNode = connectNavigator->GetNextInput();
			if (inputNode != NULL)
			{
				inputNode	= filterOutputNode;
				done		= true;
			}
		
			if (inputNode != NULL)
			{
				err =  mediaRoster->GetConnectedOutputsFor(*outputNode,videoOutput,max_in_out, &count);
		//		DEBUG("GetFreeOutputs: %s\n",strerror(err));
				//**testen ob alles ok..
				err = 	mediaRoster->GetConnectedInputsFor(*inputNode,videoInput,max_in_out, &count);
		//		DEBUG("GetFreeInputs: %s\n",strerror(err));
				//**testen ob alles ok..
				format.type				= videoOutput->format.type;	
				format.u.raw_video		= media_raw_video_format::wildcard;
				format.u.encoded_video	= media_encoded_video_format::wildcard;
				
		//		err = mediaRoster->Disconnect(0,videoOutput->node,videoOutput->source,videoInput->node, videoInput->destination);
			}
			else
			{
				done = true;
			}
		}
		else
		{
			inputNode=connectNavigator->GetNextInput();
		}
	}

	return B_OK;
}


media_node* NodeManager::DisconnectOutput(BMessage *outputMetaNode)
{
	MessageNavigator	*connectNavigator	= new MessageNavigator(outputMetaNode);
	media_node			*outputNode			= NULL;
	media_node			*inputNode			= NULL;
	media_node			*firstNode			= NULL;
	media_output		*videoOutput		= new media_output[max_in_out];
	media_input			*videoInput			= new media_input[max_in_out];
	int32 				count				= 1;
	status_t			err					= B_OK;
	media_format 		format;
	bool				done				=false;
	while (!done)
	{
		if ((outputNode=connectNavigator->GetCurrentOutputNode()) != NULL)
		{
			inputNode = connectNavigator->GetNextInput();
			if (inputNode != NULL)
			{
				err =  mediaRoster->GetConnectedOutputsFor(*outputNode,videoOutput,max_in_out, &count);
		//		DEBUG("GetFreeOutputs: %s\n",strerror(err));
				//**testen ob alles ok..
				err = 	mediaRoster->GetConnectedInputsFor(*inputNode,videoInput,max_in_out, &count);
		//		DEBUG("GetFreeInputs: %s\n",strerror(err));
				//**testen ob alles ok..
				format.type				= videoOutput->format.type;	
				format.u.raw_video		= media_raw_video_format::wildcard;
				format.u.encoded_video	= media_encoded_video_format::wildcard;
				
		//		err = mediaRoster->Disconnect(0,videoOutput->node,videoOutput->source,videoInput->node, videoInput->destination);
			}
			else
			{
				done = true;
			}
		}
		else
		{
			inputNode=connectNavigator->GetNextInput();
			firstNode=inputNode;
		}
	}
	//we return the input node, because connectNavigarot returns the last correct media_node in there(if GetNextInput==NULL)
	return firstNode;
}


void NodeManager::SetNodeConfiguration(BMessage *nodeConfig)
{
	if (nodeConfig != NULL)
		configMessage=nodeConfig;
}


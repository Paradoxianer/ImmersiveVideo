#include <interface/Button.h>
#include <interface/Box.h>
#include <interface/View.h>

#include <media/MediaAddOn.h>
#include <media/MediaDefs.h>
#include <media/MediaNode.h>
#include <media/MediaTheme.h>
#include <media/TimeSource.h>

#include "MediaNodeManager.h"

#if B_ZETA_VERSION_BETA
	#include <locale/Locale.h>
#else
	#define _T(a) a
#endif 


MediaNodeManager::MediaNodeManager():BLooper("MediaNodeManager")
{
	INFO("MediaNodeManager","MediaNodeManager()");
	Init();
	Run();
	BuildOwnNodes();
}
void MediaNodeManager::Init()
{
	INFO("MediaNodeManager","Init()");
	status_t err		= B_OK;
	media_node			timeSourceNode;

	mediaRoster			= BMediaRoster::Roster(&err);
	if (!mediaRoster || (err != B_OK))
	{
		err = B_OK;
		err = shutdown_media_server();
		if (err)
		{
			err = launch_media_server();
			if (err!=B_OK)
			{
				//wenn dass selbst nicht geht.. allerten..
			}
			else
			{
				mediaRoster			= BMediaRoster::Roster(&err);
			}
		}
	}
	//all "nodes" are without data at the beginning
	openPanel			= NULL;
	savePanel			= NULL;


	decoder				= NULL;
	entzerrer			= NULL;
	darsteller			= NULL;
	splitter			= NULL;
	highlighter			= NULL;
	timeSource			= NULL;
	inputList			= new BList();
	outputList			= new BList();
	highlighted			= false;
	err					= mediaRoster->GetSystemTimeSource(&timeSourceNode);
	timeSource			= new MediaNodeWrapper(&timeSourceNode);
//	err					= mediaRoster->GetTimeSource(&timeSourceNode);
	if (err)
		ERROR("MediaNodeManager","GetTimeSource: err=%s\n",strerror(err));		


	if (err)
	{
		//**later try to creat a other one???
		timeSource		= new MediaNodeWrapper(&timeSourceNode);
	}
	else
		ERROR("MediaNodeConnection"," - GetSystemTimeSource %s",strerror(err));

}
/*BList* MediaNodeManager::GetFilterList(void)
{
	INFO("MediaNodeManager","GetFilterList()");

	//** Check for known filters
}*/
BList* MediaNodeManager::GetInputList(void)
{
	INFO("MediaNodeManager","GetInputList()");
	if (Lock())
	{
		status_t			err					= B_OK;
		int32 				input_count			= NUMBER_OF_INPUT_OUTPUT;
		int32 				i					= 0;
		dormant_node_info	*input_dormant_info	= new dormant_node_info[input_count];
		//** all other 
		if (inputList)	
		{
			for(i=0;i<inputList->CountItems();i++)
			{
				//do we need to cast??
				delete inputList->ItemAt(i);
			}
			inputList->MakeEmpty();
		}	
		media_format outFormat;	

		outFormat.type = B_MEDIA_RAW_VIDEO;
		outFormat.u.raw_video = media_raw_video_format::wildcard;
		outFormat.u.raw_video.display.format = B_NO_COLOR_SPACE;
		mediaRoster->GetDormantNodes(input_dormant_info,&input_count,NULL,&outFormat,NULL,0,B_BUFFER_CONSUMER|B_FILE_INTERFACE);
//		mediaRoster->GetDormantNodes(input_dormant_info,&input_count,NULL,&outFormat,NULL,0,B_BUFFER_CONSUMER);
		for (i=0;i<input_count;i++)
		{
			inputList->AddItem(new MediaNodeWrapper(&input_dormant_info[i]));
		}	


	//** maby this could be a problem if we delete the pointer?? 
		//but it shoud not be a problem because we do not delete the struct...
		input_dormant_info	= new dormant_node_info[NUMBER_OF_INPUT_OUTPUT];
		input_count			= NUMBER_OF_INPUT_OUTPUT;
		//** workaround that filereader wrong detected

		mediaRoster->GetDormantNodes(input_dormant_info,&input_count,NULL,&outFormat,"Be media reader*",0,B_BUFFER_CONSUMER);


		media_format decoutFormat;
		decoutFormat.type = B_MEDIA_RAW_VIDEO;
		decoutFormat.u.raw_video = media_raw_video_format::wildcard;
		decoutFormat.u.raw_video.display.format = B_NO_COLOR_SPACE;
	
		outFormat.type = B_MEDIA_ENCODED_VIDEO;
		outFormat.u.raw_video = media_raw_video_format::wildcard;
		outFormat.u.raw_video.display.format = B_NO_COLOR_SPACE;
		int32 tmpCount=NUMBER_OF_INPUT_OUTPUT-input_count;
		mediaRoster->GetDormantNodes(&input_dormant_info[input_count],&tmpCount,NULL,&outFormat,NULL,0,B_BUFFER_CONSUMER);
		input_count+=tmpCount;
		//we need the Decodernode 
		int32 nodeCount = 1;
		dormant_node_info	*decoder_dormant_info=new dormant_node_info;
		err = mediaRoster->GetDormantNodes(decoder_dormant_info, &nodeCount,&outFormat,&decoutFormat);
		MediaNodeWrapper *decoder=new MediaNodeWrapper(decoder_dormant_info);
		if (nodeCount)
		{
			for (i=0;i<input_count;i++)
			{
				inputList->AddItem(new MediaNodeWrapper(&input_dormant_info[i]));
			}
		}
		return inputList;
		Unlock();
	}
	else
		return NULL;
}
BList* MediaNodeManager::GetOutputList(void)
{
	INFO("MediaNodeManager","GetOutputList()");	
	if (Lock())
	{
		status_t			err						= B_OK;
		int32 				input_count 			= 15;
		int32 				i						= 0;
		dormant_node_info	*input_dormant_info		= new dormant_node_info[input_count];
		media_format inFormat;
		inFormat.type = B_MEDIA_RAW_VIDEO;
		inFormat.u.raw_video = media_raw_video_format::wildcard;
		inFormat.u.raw_video.display.format = B_NO_COLOR_SPACE;
		mediaRoster->GetDormantNodes(input_dormant_info,&input_count,&inFormat,NULL,NULL,0,B_BUFFER_PRODUCER);
		for (int i=0;i<input_count;i++)
		{
			outputList->AddItem(new MediaNodeWrapper(&input_dormant_info[i]));
		}
		//** maby this could be a problem if we delete the pointer?? 
		//but it shoud not be a problem because we do not delete the struct...
		input_dormant_info = new dormant_node_info[input_count];
		inFormat.type = B_MEDIA_ENCODED_VIDEO;
		inFormat.u.raw_video = media_raw_video_format::wildcard;
		inFormat.u.raw_video.display.format = B_NO_COLOR_SPACE;
		//** we need the Enecodernode  but how do we get the Decoder node??
		int32 nodeCount = 1;
		dormant_node_info	*encoder_dormant_info=new dormant_node_info;
		err = mediaRoster->GetDormantNodes(encoder_dormant_info, &nodeCount,&inFormat);
		encoder=new MediaNodeWrapper(encoder_dormant_info);
		if (!nodeCount)
		{
			for (int i=0;i<input_count;i++)
			{
				outputList->AddItem(new MediaNodeWrapper(&input_dormant_info[i]));
			}	
		}
		return outputList;
	}
	else
		return NULL;
}
status_t MediaNodeManager::SetInput(MediaNodeWrapper *inputNode)
{
	INFO("MediaNodeManager","SetInput()");
	int32  newIndex=inputList->IndexOf(inputNode);
	if (newIndex)
	{
		SetInput(newIndex);
	}
	else
	{
		inputList->AddItem(inputNode);
		SetInput(inputList->CountItems()-1);
	}	
/*	bool wasStarted		=false;
	bool wasConnected	=false;
	if (input)
	{
		wasStarted = input->IsRunning();
		wasConnected = input->IsConnected();
		if (wasStarted)
			input->Stop();
		if (wasConnected)
			input->Disconnect();
	}
	if ((filter)&&(inputNode))
	{
		if (input2filter != NULL)
		{
			input2filter->Disconnect();
			delete input2filter;
	//		input->RemoveConnection(input2filter);
			input2filter=NULL;
		}
		input=inputNode;
		if (input!=NULL)
		{
			MediaNodeWrapper *in=input->LastNode();
			MediaNodeWrapper *out=filter->FirstNode();
			input2filter=new MediaNodeConnection(in,out);
			//input->AddConnection(input2filter);
		}
	}
	if (inputNode != input)
	{
		if (inputNode) input=inputNode;	
	}
	if (input)
	{
		if (wasConnected)
		{
			input->Connect();
			if (input2filter) input2filter->Connect();
		}
		if (wasStarted)
			input->Start();
	}*/
}

/*status_t MediaNodeManager::SetFilter(MediaNodeWrapper *filterNode)
{
	INFO("MediaNodeManager","SetFilter()");

	//** aus filterMethaNode input und output die filterconnection herausholen
	//** neuen filter übernehemen
	//und zu input und output connection herstellen
	bool wasStarted		= false;
	bool wasConnected	= false;

	if (filter)
	{
		wasStarted = filter->IsRunning();
		wasConnected = filter->IsConnected();
		if (wasStarted)
			filter->Stop();
		if (wasConnected)
			filter->Disconnect();
	}
		DE_BUG("MediaNodeManager","wasStarted =%d",wasStarted);
		DE_BUG("MediaNodeManager","wasConnected =%d",wasConnected);
	if ((filterNode)&&(input))
	{

		if (input2filter != NULL)
		{
			input2filter->Disconnect();
			delete input2filter;
			//input->RemoveConnection(input2filter);
			input2filter=NULL;
		}
		filter=filterNode;
		if (filter!=NULL)
		{
			input2filter=new MediaNodeConnection(input->LastNode(),filter->FirstNode());
			//input->AddConnection(input2filter);
		}
	}
	if ((filterNode)&&(output))
	{
		if (filter2output != NULL)
		{
			filter2output->Disconnect();
			delete filter2output;
			//output->RemoveConnection(filter2output);
			filter2output=NULL;
		}
		if (filter!=NULL)
		{
			filter2output=new MediaNodeConnection(filter->LastNode(),output->FirstNode());
			//output->AddConnection(filter2output);
		}
	}
	if (filterNode != filter)
	{
		if (filterNode) filter=filterNode;	
	}
	if (filter)
	{
		if (wasConnected)
		{
			if (input2filter) input2filter->Connect();
			filter->Connect();
			if (filter2output) filter2output->Connect();
		}
		if (wasStarted)
			filter->Start();
	}
}*/

status_t MediaNodeManager::SetOutput(MediaNodeWrapper  *outputNode)
{
	INFO("MediaNodeManager","SetOutput(MetaNode)");
	int32  newIndex=outputList->IndexOf(outputNode);
	if (newIndex)
	{
		SetOutput(newIndex);
	}
	else
	{
		inputList->AddItem(outputNode);
		SetOutput(outputList->CountItems()-1);
	}	
/*	bool wasStarted		= false;
	bool wasConnected	= false;
	if (output)
	{
		wasStarted = output->IsRunning();
		wasConnected = output->IsConnected();
		if (wasStarted)
			output->Stop();
		if (wasConnected)
			output->Disconnect();
	}

	if ((filter) && (outputNode))
	{
		if (filter2output != NULL)
		{
			filter2output->Disconnect();
			delete filter2output;
			filter2output = NULL;
		}
			//output->RemoveConnection(filter2output);
		filter2output=NULL;
		output=outputNode;
		if (output!=NULL)
		{
			filter2output=new MediaNodeConnection(filter->LastNode(),output->FirstNode());
			output->AddConnection(filter2output);
		}
		//** aus filterMethaNode input die filterconnection herausholen
		//** neuen input übernehemen
		//und zum filter connection herstellen
	}
	if (outputNode != output)
	{
		if (outputNode) output=outputNode;	
	}
	if (output)
	{
		if (wasConnected)
		{
			if (filter2output) filter2output->Connect();
			output->Connect();
		}
		if (wasStarted)
			output->Start();
	}*/
}

status_t MediaNodeManager::SetInput(int32 index)
{
	INFO("MediaNodeManager","SetOutput(%ld)",index);
	if (index<inputList->CountItems())
	{
		selectedInput = index;
	}
	else
		selectedInput =	 inputList->CountItems();
	input=(MediaNodeWrapper *)inputList->ItemAt(selectedInput);
	Start();
	//** need to check if we need to disconnect the old one :-) an "reconnect
}
/*status_t MediaNodeManager::SetFilter(int32 index)
{
	INFO("MediaNodeManager","SetFilter(%ld)",index);
	SetFilter((MetaNode* )filterList->ItemAt(index));
}*/
status_t MediaNodeManager::SetOutput(int32 index)
{
	INFO("MediaNodeManager","SetOutput(%ld)",index);
	if (index<outputList->CountItems())
	{
		selectedInput = index;
	}
	else
		selectedInput =	 outputList->CountItems();
	Prepare();
	
	//** need to check if we need to disconnect the old one :-) an "reconnect
}

status_t MediaNodeManager::Prepare(void)
{
	INFO("MediaNodeManager","Connect()");
//release all old Nodes

//	mediaRoster->ReleaseNode(input_nodes[0]);
	
/*	if (input)	input->Connect();
	if (input2filter) input2filter->Connect();
	if (filter)	filter->Connect();
	if (filter2output) filter2output->Connect();
	if (output)	output->Connect();*/
}
status_t MediaNodeManager::Stop(void)
{
	INFO("MediaNodeManager","Stop()");

/*	if (input)	input->Stop();
	if (filter)	filter->Stop();
	if (output)	output->Stop();*/
}
status_t MediaNodeManager::Start(void)
{
	INFO("MediaNodeManager","Start()");
	status_t 	err;
	
	media_node	input_node;
	media_node	decoder_node;
	media_node	entzerr_node;
	media_node	splitter_node;
	media_node	darsteller_node;
	media_node	highlight_node;
	media_node	encoder_node;
	media_node	output_node;

	media_node	timeSource_node;

	media_output	inputOutput;

	media_input		decoderInput;
	media_output	decoderOutput;

	media_input		entzerrerInput;
	media_output	entzerrerOutput;

	media_input		splitterInput;
	media_output	splitterOutput;

	media_input		highlightInput;
	media_output	highlightOutput;


	media_input		encoderInput;
	media_output	encoderOutput;

	media_input		darstellerInput;
	media_input		outputInput;
	
	int32 			count;
	
	BTimeSource		*theTimesource;
	bigtime_t		startTime;
	
	if (input)
		input_node		= *input->GetNode();
	if (decoder)		
		decoder_node	= *decoder->GetNode();
	if (entzerrer)
		entzerr_node	= *entzerrer->GetNode();
	if (splitter)
		splitter_node	= *splitter->GetNode();
	if ( (highlighted) && (highlighter) )
		highlight_node	= *highlighter->GetNode();
	if (darsteller)
		darsteller_node	= *darsteller->GetNode();
	if (encoder)		
		encoder_node	= *encoder->GetNode();
	if (output)
		output_node		= *output->GetNode();
/*	if (timeSource)
		timeSource_node	= *timeSource->GetNode();*/
	mediaRoster->GetSystemTimeSource(&timeSource_node);
	

	err	= mediaRoster->SetTimeSourceFor(input_node.node,timeSource_node.node);
	err = mediaRoster->GetFreeOutputsFor(input_node,&inputOutput,1, &count);
	if (err)
		ERROR("MediaNodeManager","GetFreeOutputsFor: %s err=%s\n",input->GetName(),strerror(err));
//	if (input_node.kind & B_FILE_INTERFACE)
	if (inputOutput.format.type == B_MEDIA_ENCODED_VIDEO)
	{
		err	=	mediaRoster->SetTimeSourceFor(decoder_node.node,timeSource_node.node);
		if (err)
			ERROR("MediaNodeManager","SetTimeSourceFor: %s err=%s\n",decoder->GetName(),strerror(err));
		err = 	mediaRoster->GetFreeInputsFor(decoder_node,&decoderInput,1, &count);
		if (err)
			ERROR("GetFreeInputsFor: %s err=%s\n",decoder->GetName(),strerror(err));
		err =	mediaRoster->Connect(inputOutput.source, decoderInput.destination,&inputOutput.format,&inputOutput,&decoderInput);
		if (err)
			ERROR("MediaNodeManager","Connect: %s err=%s\n",decoder->GetName(),strerror(err));
		err = 	mediaRoster->GetFreeOutputsFor(decoder_node,&decoderOutput,1, &count);
		if (err)
			ERROR("MediaNodeManager","GetFreeOutputsFor: %s err=%s\n",decoder->GetName(),strerror(err));
		err	=	mediaRoster->SetTimeSourceFor(entzerr_node.node,timeSource_node.node);
		if (err)
			ERROR("MediaNodeManager","SetTimeSourceFor: %s err=%s\n",entzerrer->GetName(),strerror(err));
		err =	mediaRoster->GetFreeInputsFor(entzerr_node,&entzerrerInput,1, &count);
		if (err)
			ERROR("MediaNodeManager","GetFreeInputsFor: %s err=%s\n",entzerrer->GetName(),strerror(err));		
		err =	mediaRoster->Connect(decoderOutput.source, entzerrerInput.destination,&decoderOutput.format,&decoderOutput,&entzerrerInput);
		if (err)
			ERROR("MediaNodeManager","Connect: %s err=%s\n",entzerrer->GetName(),strerror(err));		
		
	}
	else
	{
		err	=	mediaRoster->SetTimeSourceFor(entzerr_node.node,timeSource_node.node);
		err =	mediaRoster->GetFreeInputsFor(entzerr_node,&entzerrerInput,1, &count);
		err =	mediaRoster->Connect(inputOutput.source, entzerrerInput.destination,&inputOutput.format,&inputOutput,&entzerrerInput);
	}
	err = 	mediaRoster->GetFreeOutputsFor(entzerr_node,&entzerrerOutput,1, &count);
	//** we coud use the Programm without a splitte Node so maby we shoud implement a workaround???
//	err	=	mediaRoster->SetTimeSourceFor(splitter_node.node,timeSource_node.node);
//	err =	mediaRoster->GetFreeInputsFor(splitter_node,&splitterInput,1, &count);
//	err =	mediaRoster->Connect(entzerrerOutput.source, splitterInput.destination,&entzerrerOutput.format,&entzerrerOutput,&splitterInput);
	err = 	mediaRoster->GetFreeOutputsFor(splitter_node,&splitterOutput,1, &count);
	if (highlighted)	
	{
		err	=	mediaRoster->SetTimeSourceFor(highlight_node.node,timeSource_node.node);
		err =	mediaRoster->GetFreeInputsFor(highlight_node,&highlightInput,1, &count);
		err =	mediaRoster->Connect(splitterOutput.source, highlightInput.destination,&splitterOutput.format,&splitterOutput,&highlightInput);
		err = 	mediaRoster->GetFreeOutputsFor(highlight_node,&highlightOutput,1, &count);	
		err =	mediaRoster->SetTimeSourceFor(darsteller_node.node,timeSource_node.node);
		err =	mediaRoster->GetFreeInputsFor(darsteller_node,&darstellerInput,1, &count);
		err =	mediaRoster->Connect(highlightOutput.source, darstellerInput.destination,&highlightOutput.format,&highlightOutput,&darstellerInput);
	}
	else
	{
		err =	mediaRoster->SetTimeSourceFor(darsteller_node.node,timeSource_node.node);
		err =	mediaRoster->GetFreeInputsFor(darsteller_node,&darstellerInput,1, &count);
		err =	mediaRoster->Connect(entzerrerOutput.source, darstellerInput.destination,&entzerrerOutput.format,&entzerrerOutput,&darstellerInput);
//		err =	mediaRoster->Connect(splitterOutput.source, darstellerInput.destination,&splitterOutput.format,&splitterOutput,&darstellerInput);
	}
	if (output)
	{
		err = 	mediaRoster->GetFreeOutputsFor(splitter_node,&splitterOutput,1, &count);
		err =	mediaRoster->SetTimeSourceFor(output_node.node,timeSource_node.node);
		err =	mediaRoster->GetFreeInputsFor(output_node,&outputInput,1, &count);
		if (outputInput.format.type== B_MEDIA_ENCODED_VIDEO)
		{
			err =	mediaRoster->SetTimeSourceFor(encoder_node.node,timeSource_node.node);
			err =	mediaRoster->GetFreeInputsFor(encoder_node,&encoderInput,1, &count);	
			err =	mediaRoster->Connect(splitterOutput.source, encoderInput.destination,&splitterOutput.format,&splitterOutput,&encoderInput);
			err =	mediaRoster->GetFreeOutputsFor(encoder_node,&encoderOutput,1, &count);
			err =	mediaRoster->Connect(encoderOutput.source, outputInput.destination,&encoderOutput.format,&encoderOutput,&outputInput);
		}
		else
		{
			err =	mediaRoster->Connect(splitterOutput.source, outputInput.destination,&splitterOutput.format,&splitterOutput,&outputInput);
		}
	
	}
	//*** wenn wir bis hierher gekommen sind sind wir sehr gut :-) nun können wir das ganze starten


//	theTimesource	= mediaRoster->MakeTimeSourceFor(timeSource_node);
//	err				= mediaRoster->GetStartLatencyFor(timeSource_node, &startTime);
//	startTime 	   += theTimesource->PerformanceTimeFor(BTimeSource::RealTime() + 1000000 / 50); 
//	startTime		= BTimeSource::RealTime() + 100;
	startTime		= 0;
	err				= mediaRoster->StartNode(input_node,startTime);

	if (inputOutput.format.type == B_MEDIA_ENCODED_VIDEO)
		err			= mediaRoster->StartNode(decoder_node,startTime);		
	err				= mediaRoster->StartNode(entzerr_node,startTime);	
	err				= mediaRoster->StartNode(splitter_node,startTime);	
	err				= mediaRoster->StartNode(splitter_node,startTime);	
	if (highlighted)	
		err			= mediaRoster->StartNode(highlight_node,startTime);		
	err				= mediaRoster->StartNode(darsteller_node,startTime);	
	if (output)
	{
		if (outputInput.format.type== B_MEDIA_ENCODED_VIDEO)
		{
			err		= mediaRoster->StartNode(encoder_node,startTime);		
		}		
		err			= mediaRoster->StartNode(output_node,startTime);		

	}
//	err 			= mediaRoster->StartTimeSource(timeSource_node,0);
	
	return err;
/*	if (output)	output->Start();
	if (filter) filter->Start();
	if (input) input->Start();*/
}



BView* MediaNodeManager::GetInputConfigView(void)
{
	return BuildConfigView((MediaNodeWrapper*)inputList->ItemAt(selectedInput));
}

BView* MediaNodeManager::GetOutputConfigView(void)
{
	return BuildConfigView((MediaNodeWrapper*)outputList->ItemAt(selectedOutput));
}

void MediaNodeManager::BuildOwnNodes(void)
{
	immersiveVideoFilter	= new ImmersiveVideoFilter(NULL,"Entzerrer",0);
	pluginManager			= new PluginManager(immersiveVideoFilter,7);
	immersiveVideoFilter->SetPluginManager(pluginManager);
	mediaRoster->RegisterNode(immersiveVideoFilter);
	entzerrer				= new MediaNodeWrapper(&immersiveVideoFilter->Node());
	
	videoDrawer				= new VideoDrawConsumer(NULL,"Darsteller",NULL);
	mediaRoster->RegisterNode(videoDrawer);
	darsteller				=  new MediaNodeWrapper(&videoDrawer->Node());
	
	theSplitter				= new MediaSplitter(NULL,"Splitter",NULL);
	mediaRoster->RegisterNode(theSplitter);
	splitter				= new MediaNodeWrapper(&theSplitter->Node());
	
	
	navFilter				= new NavigationVideoFilter(NULL,"NavigationsFilter",3);
	mediaRoster->RegisterNode(navFilter);
	navigator				=  new MediaNodeWrapper(&navFilter->Node());
	
	
	
}

BView* MediaNodeManager::BuildConfigView(MediaNodeWrapper *forMediaNode)
{
	BParameterWeb		*configWeb;
	dormant_flavor_info flavorInfo;
	status_t 			err			= B_OK;
	if (forMediaNode!=NULL)
	{
		dormant_node_info *dnInfo=forMediaNode->GetDormantNode();
		if (dnInfo)
			mediaRoster->GetDormantFlavorInfoFor(*dnInfo,&flavorInfo);
		err = mediaRoster->GetParameterWebFor(*forMediaNode->GetNode(),&configWeb); 
		if (err == B_OK)
		{
			if ((flavorInfo.kinds&B_FILE_INTERFACE) != 0)
			{
				BMessage *sender  = new BMessage(OPEN_FILE);
				sender->AddPointer("source",forMediaNode);
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
	return NULL;
}

bool MediaNodeManager::QuitRequested(void)
{
	mediaRoster->UnregisterNode(immersiveVideoFilter);
	mediaRoster->UnregisterNode(videoDrawer);
	mediaRoster->UnregisterNode(navFilter);
	return true;
}

void MediaNodeManager::MessageReceived(BMessage *message)
{
	switch(message->what) 
	{
		case  B_REFS_RECEIVED:
		{
				entry_ref ref;
				bigtime_t duration;
				message->FindRef("refs",&ref);
				media_node input_node = *input->GetNode();
				if ( (input_node.kind & B_FILE_INTERFACE)!=NULL)
				{
					status_t err = mediaRoster->SetRefFor(input_node, ref, false, &duration); 
					DE_BUG ("SetRefFor = %s\n", strerror(err));
					if (err=B_OK)
						Start();		
				}
				else
				{
					//try to find a Node wich can handle this files :-)
					MediaNodeWrapper	*tmpNode;
					bool 				found		= false;
					int32				i			= 0;
					while ((!found) && (i<inputList->CountItems()))
					{
						tmpNode=(MediaNodeWrapper* )inputList->ItemAt(i);
						if ( (tmpNode->GetNode()->kind & B_FILE_INTERFACE) != NULL)
							found=true;
						else
							i++;
					}
					SetInput(i);
					status_t err = mediaRoster->SetRefFor(*input->GetNode(), ref, false, &duration); 
					DE_BUG ("SetRefFor = %s\n", strerror(err));
					if (err=B_OK)
						Start();		
				}
			break;
		}
		
		default:
			BLooper::MessageReceived(message);
			break;
	}

}
//===================================================================================//


#include <fcntl.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>

#include <media/Buffer.h>
#include <media/BufferGroup.h>
#include <media/ParameterWeb.h>
#include <media/TimeSource.h>
#include <storage/FindDirectory.h>
#include <storage/Path.h>
#include <storage/File.h>
#include <stdio.h>

#include "MediaSplitter.h"



int32 MediaSplitter::instances = 0;


MediaSplitter::MediaSplitter( BMediaAddOn *addon, const char *name, int32 internal_id):	BMediaNode(name),BBufferProducer(B_MEDIA_UNKNOWN_TYPE),BBufferConsumer(B_MEDIA_UNKNOWN_TYPE),BMediaEventLooper(),BControllable()
{
	initStatus = B_NO_INIT;
	//count the generated instances
	if (atomic_add(&instances, 1) >= MAX_ALLOWED_SPLITTER_INSTANCES)
		return;
	//save the id of this instance
	internalID				= internal_id;
	//save the addon from wich this instance created
	fAddOn 					= addon;
	//init BasicFormat for negotiation process
	outputFormatBase.type	= B_MEDIA_UNKNOWN_TYPE;

	inputFormatBase.type 						= B_MEDIA_RAW_VIDEO;
	inputFormatBase.u.raw_video 				= media_raw_video_format::wildcard;

	inputFormatBase.type 						= B_MEDIA_RAW_AUDIO;
	inputFormatBase.u.raw_audio					= media_raw_audio_format::wildcard;

	inputFormatBase.type 						= B_MEDIA_ENCODED_VIDEO;
	inputFormatBase.u.encoded_video 			= media_encoded_video_format::wildcard;

	inputFormatBase.type 						= B_MEDIA_ENCODED_AUDIO;
	inputFormatBase.u.encoded_audio 			= media_encoded_audio_format::wildcard;


	inputFormatBase.type	= B_MEDIA_UNKNOWN_TYPE;


	inputBufferGroup		= NULL;
	outputEnabled			= true;
	downstreamLatency		= 0;
	internalLatency			= 0;
	running					=false;
	initStatus				= B_OK;
}

MediaSplitter::~MediaSplitter()
{
	BMediaEventLooper::Quit();
	int32 changeTag;
	//set the OutputBuffer to NULL so that the MediaKit coud delet it.
	SetOutputBuffersFor(filterInput.source,filterInput.destination,NULL,NULL,&changeTag);
	if (initStatus == B_OK) {
		/*if (filterOutput->destination!=media_destination::null)
			Disconnect(filterOutput->source, filterOutput->destination);*/
	/*	if (filterInput.source!=media_source::null)
			Disconnect(filterInput.source, filterInput.destination);*/
	}
	//this instance ist going down...
	atomic_add(&instances, -1);
}

BMediaAddOn *
MediaSplitter::AddOn(int32 *internal_id) const
{
	INFO("MediaSplitter","AddOn(%ld)\n",*internal_id);
	if (internal_id)
		*internal_id = internalID;
	return fAddOn;
}


status_t 
MediaSplitter::HandleMessage(int32 message, const void *data, size_t size)
{

	if(	BBufferConsumer::HandleMessage(message, data, size) &&
		BBufferProducer::HandleMessage(message, data, size) &&
		BMediaNode::HandleMessage(message, data, size))
		BMediaNode::HandleBadMessage(message, data, size);
	return B_OK;
}

void 
MediaSplitter::Preroll()
{
	INFO("MediaSplitter","Preroll\n");
	/****************************************************************************
	 *	This hook may be called before the node is started to give the hardware *
	 *	a chance to start. 														*
	 ****************************************************************************/
}



//+++++++++++++++++++++++++++BBufferProducer+++++++++++++++++++++++//
status_t 
MediaSplitter::FormatSuggestionRequested(media_type type, int32 /*quality*/, media_format* format)
{
	/****************************************************************************************
	 * FormatSuggestionRequested() is not necessarily part of the format negotiation		*
	 * process; it's simply an interrogation -- the caller wants to see what the node's		*
	 * preferred data format is, given a suggestion by the caller.							*
 	 ****************************************************************************************/
	INFO("MediaSplitter","FormatSuggestionRequested\n");							
	if (!format)
	{
		ERROR("MediaSplitter","ERROR - NULL format pointer passed in!\n");
		return B_BAD_VALUE;
	}
	// this is the format we'll be returning (our base format)
	*format=outputFormatBase;
	return B_OK;
}

status_t 
MediaSplitter::FormatProposal(const media_source& output, media_format* format)
{
	/***************************************************************************************** 
	 *	FormatProposal() is the first stage in the BMediaRoster::Connect() process.  We hand *
	 *	out a suggested format, with wildcards for any variations we support.				 *
	 *****************************************************************************************/
	INFO("MediaSplitter","FormatProposal\n");

	status_t err= B_OK;

	if (!format)
		return B_BAD_VALUE;

	// is this a proposal for our one output?
	
	if (output != filterOutput->source)
	{
		ERROR("MediaSplitter","FormatProposal returning B_MEDIA_BAD_SOURCE\n");
		return B_MEDIA_BAD_SOURCE;
	}
	if ((filterOutput->format.type == B_MEDIA_UNKNOWN_TYPE) || (filterOutput->format.type == format->type))
	{
		filterOutput->format.type	= format->type;
		filterOutput->format			= *format;
	}
	else 
	{
		err = B_MEDIA_BAD_FORMAT;
	}
	//we just return our format ... so the consumer coud fill in his preffered values
	//*format = outputFormatBase;
	return err;
}

status_t 
MediaSplitter::PrepareToConnect(const media_source &source,
		const media_destination &destination, media_format *format,
		media_source *out_source, char *out_name)
{
	INFO("MediaSplitter","PrepareToConnect()\n");
	//some security checks ;-)
	if (source != filterOutput->source)
		return B_MEDIA_BAD_SOURCE;
	
	if (filterOutput->destination != media_destination::null)
		return B_MEDIA_ALREADY_CONNECTED;

	
	if (!format_is_compatible(filterOutput->format,*format )) 
	{
 		ERROR("MediaSplitter","PrepareToConnect - B_MEDIA_BAD_FORMAT\n");
 		/*filterOutput->format.type=format->type;
		filterOutput->format=*format;*/
		return B_MEDIA_BAD_FORMAT;
	}
	
	*out_source = filterOutput->source;
	strcpy(out_name, filterOutput->name);
	filterOutput->format=*format;
	return B_OK;
}



void 
MediaSplitter::Connect(status_t error, const media_source &source,
		const media_destination &destination, const media_format &format,
		char *io_name)
{
	INFO("MediaSplitter","Connect\n");
	bigtime_t start=0, produceLatency=0;
	media_node_id tsID = 0;
	if (filterOutput->destination!=media_destination::null) {
		ERROR("MediaSplitter","Connect: Already connected\n");
		return;
	}

	if (	(source != filterOutput->source) || (error < B_OK) ||
			!const_cast<media_format *>(&format)->Matches(&filterOutput->format)) {
		ERROR("MediaSplitter","Connect: Connect error\n");
		return;
	}
	
	filterOutput->destination = destination;
	//get the name of the conected Consumer input
	strcpy(io_name, filterOutput->name);
	//use this format
	filterOutput->format=format;
	if (filterInput.source==media_source::null)
		filterInput.format=filterOutput->format;
	//** We only can  Create the buffer group if we had recived the first Buffer ;-)
	if (AllocateOutputBufferGroup(filterOutputList->CountItems()-1)!=B_OK)
	{
		ERROR("MediaSplitter","AllocateOutputBufferGroup Error\n");
		return;
	}
	//get the Latency for all nodes that follow
	FindLatencyFor(filterOutput->destination, &downstreamLatency, &tsID);
	//prepare the calculation of the own latency
	start = ::system_time();
	//** find the right size....
	//**uint32 sz =(filterOutput->format.u.raw_video.display.line_width * filterOutput->format.u.raw_video.display.line_count);
	
	//BBuffer *outBuffer = outputBufferGroup->RequestBuffer(sz*4,30000);
//	outBuffer->Recycle();
	produceLatency = ::system_time();
	internalLatency = produceLatency - start;
	DE_BUG("MediaSplitter","internal Latency\t%ld\n",(int32)internalLatency);
//	SetEventLatency(downstreamLatency + internalLatency);
	SetEventLatency(internalLatency);
	//tell the Consumer if connected with us that his downstreamlatency hase changed
	SendLatencyChange(filterInput.source, filterInput.destination, EventLatency() + SchedulingLatency());
	PrepareNewOutput();
}

void 
MediaSplitter::Disconnect(const media_source& what, const media_destination& where)
{
	INFO("MediaSplitter","Disconnect()\n");

	// Make sure that our connection is the one being disconnected
	//** disconnect the right connections and delete it from the list
	media_output	*tmpOutput			= new media_output;
	outBufferStruct	*tmpBufferStruct	= new outBufferStruct;
	bool			found				= false;
	int32			i					= 0;
	for (i=0;(i<(filterOutputList->CountItems()-1) && (!found));i++)
	{
		tmpOutput=(media_output *)filterOutputList->ItemAt(i);
		tmpBufferStruct=(outBufferStruct*) outputBufferGroupList->ItemAt(i);
		if ((where == filterOutput->destination) && (what == filterOutput->source))
		{
			delete tmpOutput;
			delete tmpBufferStruct;
			if (filterInput.source == media_source::null)
				filterOutput->format = outputFormatBase;
			else
			{
				filterOutput->format = outputFormatBase;
				filterOutput->format = filterInput.format;
			}
			found=true;
			
		}
	}
	if (found == false)
	{
		ERROR("MediaSplitter","\tDisconnect() with wrong parameters...\n");
	}
}



status_t 
MediaSplitter::FormatChangeRequested(const media_source &source,
		const media_destination &destination, media_format *io_format,
		int32 *_deprecated_)
{
	//** später mal richtigen "Formatwechsel implementieren**//
	INFO("MediaSplitter","FormatChangeRequested\n");

	if (source != filterOutput->source)
		return B_MEDIA_BAD_SOURCE;
	return B_ERROR;	
}

status_t 
MediaSplitter::GetNextOutput(int32 *cookie, media_output *out_output)
{
	INFO("MediaSplitter","GetNextOutput(%ld, ...)\n",*cookie);
	DE_BUG("MediaSplitter","Connected Outputs=%ld\n",filterOutputList->CountItems());
	if (((*cookie) >= filterOutputList->CountItems()) || ((*cookie)<0))
		return B_BAD_INDEX;


	*out_output = *(media_output *)filterOutputList->ItemAt(*cookie);

	(*cookie)++;
	return B_OK;
}

status_t 
MediaSplitter::DisposeOutputCookie(int32 cookie)
{
	INFO("MediaSplitter","DisposeOutputCookie()\n");
	return B_OK;
}

status_t 
MediaSplitter::SetBufferGroup(const media_source &for_source,
		BBufferGroup *newGroup)
{
	INFO("MediaSplitter","SetBufferGroup\n");
	status_t 		err					= B_OK;
	media_output	*tmpOutput			= new media_output;
	outBufferStruct	*tmpBufferStruct	= new outBufferStruct;
	BBuffer			**newGroupBuffer	= (BBuffer**) malloc( sizeof(BBuffer*) * 10 );
//	BBuffer			**newGroupBuffer;
	int32			listcount			= 10;
	if (for_source.id<filterOutputList->CountItems())
	{
		tmpOutput=(media_output *)filterOutputList->ItemAt(for_source.id);
		//if the SetBufferGroup comes frome the first "Output" we pass it throut our source later we shoud take the first recived newGroup
		if (for_source.id == 0)
		{
		
			int32 changeTag;
			// if the size for the BBufferGroup-Creation wasnt set now we use the BufferGroup wich was given us to find the right size
			if (sz<0)
			{
				//check value
				
				if (newGroup != NULL)
				{
					err= newGroup->GetBufferList(listcount,newGroupBuffer);
					DE_BUG("MediaSplitter","listcount =%ld als error= %s\n",listcount,strerror(err));
					if (listcount>0)
						sz=newGroupBuffer[0]->SizeAvailable();
				}
				else
					err = B_BAD_VALUE;
			}
			//pass the Buffergroup throug if coudnt pass it throug we store it in the struct
			if (filterInput.source!=media_source::null)
			//	err=SetOutputBuffersFor(filterInput.source,tmpOutput->destination,newGroup,NULL,&changeTag);
				AllocateInputBufferGroup();
		//		err=SetOutputBuffersFor(filterInput.source,filterInput.destination,newGroup,NULL,&changeTag);
			DE_BUG("MediaSplitter","SetOutputBuffersFor= %s\n",strerror(err));
		}
		else
		{
			tmpBufferStruct=(outBufferStruct*)outputBufferGroupList->ItemAt(for_source.id);
			if (tmpBufferStruct->ownBufferGroup)
			{
				// waits for all buffers to recycle
				if (tmpBufferStruct->bufferGroup)
				{
					tmpBufferStruct->bufferGroup->ReclaimAllBuffers();
					delete tmpBufferStruct->bufferGroup;
					tmpBufferStruct->bufferGroup=NULL;
				}
			}
			if (newGroup != NULL)
			{
			// we were given a valid group; just use that one from now on
				tmpBufferStruct->ownBufferGroup = false;
				tmpBufferStruct->bufferGroup = newGroup;
				if (sz<0)
				{
					newGroup->GetBufferList(listcount,newGroupBuffer);
					if (listcount>0)
						sz=newGroupBuffer[0]->SizeAvailable();
				}
				else
					err = B_BAD_VALUE;
				DE_BUG("MediaSplitter","BufferGroup %ld  angenommen\n",for_source.id);
				return B_OK;
			}
			else
			{
				// we were passed a NULL group pointer; that means we construct
				// our own buffer group to use from now on
				tmpBufferStruct->ownBufferGroup = true;
				return AllocateOutputBufferGroup(for_source.id);
			}
		}
		
	}
	return err;
}

status_t 
MediaSplitter::GetLatency(bigtime_t* out_latency)
{
	INFO("MediaSplitter","GetLatency\n");

	// report our *total* latency:  internal plus downstream plus scheduling
	*out_latency = EventLatency() + SchedulingLatency();
	return B_OK;
}



status_t 
MediaSplitter::VideoClippingChanged(const media_source &for_source,
		int16 num_shorts, int16 *clip_data,
		const media_video_display_info &display, int32 *_deprecated_)
{
	INFO("MediaSplitter","VideoClippingChanged()\n");
	// we need to produce ... all not only cipped region....... because other Consumer maby need this stuff
	return B_ERROR;
}

void 
MediaSplitter::LateNoticeReceived(const media_source& what, bigtime_t how_much, bigtime_t performance_time)
{
	INFO("MediaSplitter","LateNoticeReceived\n");
	if (what != filterOutput->source)
	{
		ERROR("MediaSplitter","Bad source.\n");
		return;
	}

	if (filterInput.source == media_source::null)
	{
		ERROR("MediaSplitter","!!! No input to blame.\n");
		return;
	}
	// pass the buck, since this node doesn't schedule buffer production
	//** but maby we shoud fix our internal latency??
	NotifyLateProducer(filterInput.source, how_much,performance_time);
}


void MediaSplitter::EnableOutput(const media_source& what, bool enabled, int32* _deprecated_)
{
	INFO("MediaSplitter","EnableOutput\n");
	if (what == filterOutput->source)
	{
		outputEnabled = enabled;
	}
}



status_t 
MediaSplitter::SetPlayRate(int32 numer, int32 denom)
{
	INFO("MediaSplitter","SetPlayRate()\n");
	//***stent to the FiterInput Producer but how??
	return B_ERROR;
}



void 
MediaSplitter::AdditionalBufferRequested(const media_source &source,
		media_buffer_id prev_buffer, bigtime_t prev_time,
		const media_seek_tag *prev_tag)
{
	INFO("MediaSplitter","AdditionalBufferRequest()\n");
	RequestAdditionalBuffer(filterInput.source,TimeSource()->RealTime());
//	RequestAdditionalBuffer(filterInput.source, OfflineTime());
}

void 
MediaSplitter::LatencyChanged(const media_source &source,
		const media_destination &destination, bigtime_t new_latency,
		uint32 flags)
{
	INFO("MediaSplitter","LatencyChanged()\n");
	if (source != filterOutput->source)
	{
		ERROR("MediaSplitter","\tBad source.\n");
		return;
	}
	if (destination != filterOutput->destination)
	{
		ERROR("MediaSplitter","\tBad destination.\n");
		return;
	}
	
	downstreamLatency = new_latency;
	SetEventLatency(downstreamLatency + internalLatency);
	
	if (filterInput.source != media_source::null)
	{
		// pass new latency upstream
		status_t err = SendLatencyChange(filterInput.source, filterInput.destination, EventLatency() + SchedulingLatency());
		if(err < B_OK)
			DE_BUG("MediaSplitter","SendLatencyChange(): %s\n", strerror(err));
	}

}
//--------------------------BBufferProducer-----------------------//


status_t 
MediaSplitter::AcceptFormat(const media_destination& dest, media_format* format)
{
	INFO("MediaSplitter","AcceptFormat()\n");
	status_t err=B_OK;
	// return an error if this isn't really our one input's destination
	if (dest != filterInput.destination) 
		err=B_MEDIA_BAD_DESTINATION;
	DE_BUG("MediaSplitter","format: %ld\n",format->type);
	switch (format->type) 
	{
		case B_MEDIA_RAW_AUDIO:
			break;
		case B_MEDIA_RAW_VIDEO:
			{
				format->u.raw_video.display.bytes_per_row = format->u.raw_video.display.line_width*sizeof(format->u.raw_video.display.format);
			}
			break;
		case B_MEDIA_MULTISTREAM:
			break;
		case B_MEDIA_ENCODED_AUDIO:
			break;
		case B_MEDIA_ENCODED_VIDEO:
			break;
	}

	filterInput.format.type=format->type;
	filterInput.format = *format;
	format=&(filterInput.format);
	// the destination given really is our input, and we accept any kind of media data,
	// so now we just confirm that we can handle whatever the producer asked for.
	return err;
}

status_t 
MediaSplitter::Connected(
	const media_source& producer,
	const media_destination& where,
	const media_format& with_format,
	media_input* out_input)
{
	status_t err =B_OK;
	INFO("MediaSplitter","Connected()\n");
	if (where != filterInput.destination) return B_MEDIA_BAD_DESTINATION;
	// record useful information about the connection, and return success
	filterInput.source = producer;
	if (!format_is_compatible(with_format, filterInput.format)) 
	{
 		ERROR("MediaSplitter","AcceptFormat - B_MEDIA_BAD_FORMAT\n");
		return B_MEDIA_BAD_FORMAT;
	}
	filterInput.format =with_format;

/*	if (filterOutput->destination == media_destination::null)
	{*/
	filterOutput->format = filterInput.format;
	//}
	*out_input = filterInput;

	err=AllocateInputBufferGroup();
//	DE_BUG("\tAllocateInputBufferGroup: %s\n",strerror(err));
	int32 changeTag;
//	err=SetOutputBuffersFor(producer,where,inputBufferGroup,NULL,&changeTag);
//	outputFormatBase=with_format;

/*	if (filterOutput->destination!=media_destination::null)
	{
		ChangeFormat(filterOutput->source,filterOutput->destination,&outputFormatBase);
	}*/	
	DE_BUG("MediaSplitter","SetOutputBuffersFor: %s\n",strerror(err));
	return B_OK;
}

void 
MediaSplitter::Disconnected(
	const media_source& producer,
	const media_destination& where)
{
	INFO("MediaSplitter","Disconnected()\n");
	if (filterInput.source != producer)
	{
		ERROR("MediaSplitter","source mismatch: expected ID %ld, got %ld\n",	filterInput.source.id, producer.id);
		return;
	}
	if (where != filterInput.destination)
	{
		ERROR("MediaSplitter","destination mismatch: expected ID %ld, got %ld\n", filterInput.destination.id,where.id);
		return;
	}

	int32 changeTag;
	SetOutputBuffersFor(producer,where,NULL,NULL,&changeTag);
	// mark disconnected
	
	filterInput.source	= media_source::null;
	filterInput.format	= inputFormatBase;
	firstBufferRecived	= true;
	// no output? clear format:
	if (filterOutputList->CountItems() == 0)
	{
		filterOutput->format = outputFormatBase;
	}
}



status_t 
MediaSplitter::GetNextInput(int32* cookie, media_input* in_input)
{
	INFO("MediaSplitter","GetNextInput(%ld,...)\n",*cookie);
	if (!in_input)
		return B_BAD_VALUE;

	if ((*cookie) != 0)
		return B_BAD_INDEX;
	*in_input = filterInput;
	(*cookie)++;
	return B_OK;

}
	
void 
MediaSplitter::DisposeInputCookie(int32 /*cookie*/ cookie)
{
	INFO("MediaSplitter","DisposeInputCookie()\n");
	// we don't use any kind of state or extra storage for iterating over our
	// inputs, so we don't have to do any special disposal of input cookies.
	cookie--;
	
}

void 
MediaSplitter::BufferReceived(BBuffer* inBuffer)
{
	INFO("MediaSplitter","BufferReceived():\n");		
	if( RunMode() == B_OFFLINE ) {
//		int32 destinationID = inBuffer->Header()->destination;
		SetOfflineTime( inBuffer->Header()->start_time );
	}

	status_t err;
	media_timed_event event(inBuffer->Header()->start_time,
							BTimedEventQueue::B_HANDLE_BUFFER,
							inBuffer, BTimedEventQueue::B_RECYCLE_BUFFER );
	err = EventQueue()->AddEvent( event );
	
	if( err ) inBuffer->Recycle();
}




void MediaSplitter::ProducerDataStatus(const media_destination & for_whom, int32 status, bigtime_t at_performance_time)
{ 
	if (for_whom == filterInput.destination)
	{ 
		media_timed_event event(at_performance_time,
			BTimedEventQueue::B_DATA_STATUS, &filterInput,	
			BTimedEventQueue::B_NO_CLEANUP, status, 0, NULL);
		EventQueue()->AddEvent(event);
	}
}

status_t 
MediaSplitter::GetLatencyFor(const media_destination& for_whom, bigtime_t* out_latency, media_node_id* out_timesource)
{
	INFO("MediaSplitter","GetLatencyFor()\n");
	// make sure this is one of my valid inputs
	if (for_whom != filterInput.destination) return B_MEDIA_BAD_DESTINATION;

	// report internal latency + downstream latency here, NOT including scheduling latency.
	*out_latency = downstreamLatency+internalLatency;
	*out_timesource = TimeSource()->ID();
	return B_OK;
}


status_t 
MediaSplitter::FormatChanged(
	const media_source& producer,
	const media_destination& consumer,
	int32 change_tag,
	const media_format& format)
{
	INFO("MediaSplitter","FormatChanged()\n");
	status_t err=B_OK;
	err=AllocateInputBufferGroup();
	ERROR("MediaSplitter","AllocateInputBufferGroup: %s\n",strerror(err));
	int32 changeTag;
	err=SetOutputBuffersFor(producer,consumer,inputBufferGroup,NULL,&changeTag);
	ERROR("MediaSplitter","SetOutputBuffersFor: %s\n",strerror(err));
	return err;
}

status_t 
MediaSplitter::SeekTagRequested(
	const media_destination& destination,
	bigtime_t in_target_time,
	uint32 in_flags,
	media_seek_tag* out_seek_tag,
	bigtime_t* out_tagged_time,
	uint32* out_flags)
{
	INFO("MediaSplitter","SeekTagRequested()\n");
	//**pass it through
	return B_ERROR;
}


void 
MediaSplitter::NodeRegistered()
{
	if (initStatus != B_OK) 
	{
		ReportError(B_NODE_IN_DISTRESS);
		return;
	}
	SetParameterWeb(GetParameterWeb());
	Run();
	INFO("MediaSplitter","NodeRegistered()\n");
	filterOutputList				= new BList();
	outputBufferGroupList			= new BList();
	filterInput.destination.port	= ControlPort();
	filterInput.destination.id		= 0;
	filterInput.source				= media_source::null;
	filterInput.node				= Node();
	filterInput.format				= inputFormatBase;
	firstBufferRecived				= false;
	strcpy(filterInput.name, "MediaSplitter Input");
	sz								= -1;
	PrepareNewOutput();	
	
	
	
}

status_t MediaSplitter::GetParameterValue(int32 id, bigtime_t *last_change, void *value, size_t *size)
{
	return B_OK;
}

void MediaSplitter::SetParameterValue(int32 id, bigtime_t when, const void *value, size_t size)
{
}

status_t MediaSplitter::StartControlPanel(BMessenger *out_messenger)
{
	return BControllable::StartControlPanel(out_messenger);
}

void 
MediaSplitter::HandleEvent(const media_timed_event *event,
		bigtime_t lateness, bool realTimeEvent)
{
	INFO("MediaSplitter","HandleEvent()\n");
	switch( event->type ) 
	{
		case BTimedEventQueue::B_HANDLE_BUFFER:
			{
				BBuffer *buffer = const_cast<BBuffer*>((BBuffer*)event->pointer);
				if( buffer ) 
				{
					HandleBufferWrap( buffer, lateness );
				}
			}
			break;
			
		case BTimedEventQueue::B_START:
			running = true;
			break;
			
		case BTimedEventQueue::B_STOP:
			running = false;
			EventQueue()->FlushEvents( 0, BTimedEventQueue::B_ALWAYS, true, BTimedEventQueue::B_HANDLE_BUFFER );
			break;
			
		case BTimedEventQueue::B_DATA_STATUS:
			{
				media_destination *dest = (media_destination*)event->pointer;
				if( filterInput.destination== *dest) ;
					//**data available or not use ist later 
			}
			break;
			
		default:
			ERROR("MediaSplitter","Unhandled Event in FMediaNode (%i)\n", (int)event->type );
			break;
	}
}


status_t
MediaSplitter::AllocateInputBufferGroup()
{
	INFO("MediaSplitter","AllocateInputBufferGroup()\n");	
	if (inputBufferGroup)
	{
		inputBufferGroup->ReclaimAllBuffers();
		inputBufferGroup->WaitForBuffers();
		delete inputBufferGroup;
	}
	inputBufferGroup = new BBufferGroup(sz, 2);
	if (inputBufferGroup->InitCheck() < B_OK)
	{
		delete inputBufferGroup;
		inputBufferGroup = NULL;
		return B_NO_INIT;
	}
	int32 changeTag;
	SetOutputBuffersFor(filterInput.source,filterInput.destination,inputBufferGroup,NULL,&changeTag);
	return B_OK;
}


status_t
MediaSplitter::AllocateOutputBufferGroup(int32 index)
{
	status_t err =B_OK;
	INFO("MediaSplitter","AllocateOutputBufferGroup()\n");
	outBufferStruct	*tmpBufferStruct;
	if (index>0)	
	{
		tmpBufferStruct=(outBufferStruct *)outputBufferGroupList->ItemAt(index);
		if (tmpBufferStruct->ownBufferGroup)
		{
			DE_BUG("MediaSplitter","Alloziere Buffer = %ld mit Size= %ld\n",index,sz);
			tmpBufferStruct->bufferGroup = new BBufferGroup(sz, 2);
			if (tmpBufferStruct->bufferGroup->InitCheck() < B_OK)
			{
				delete tmpBufferStruct->bufferGroup;
				tmpBufferStruct->bufferGroup = NULL;
				err = B_NO_INIT;
			}
		}
	}
	else if (index == 0)
	{
		//** nix allozieren sondern der 1. wird weitergeschickt ;-)
		
	}
	else
	{
		int i;
		status_t err=B_OK;
		// verify that we didn't get bogus arguments before we proceed
		for (i=filterOutputList->CountItems()-2;((i>0) && (err == B_OK));i--)
		{
			tmpBufferStruct=(outBufferStruct*)outputBufferGroupList->ItemAt(i);
			if (tmpBufferStruct->ownBufferGroup)
			{
				DE_BUG("MediaSplitter","Alloziere Buffer = %ld mit Size= %ld\n",i,sz);
				if (tmpBufferStruct->bufferGroup != NULL)
					delete tmpBufferStruct->bufferGroup;
				tmpBufferStruct->bufferGroup = new BBufferGroup(sz, 2);
				if (tmpBufferStruct->bufferGroup->InitCheck() < B_OK)
				{
					delete tmpBufferStruct->bufferGroup;
					tmpBufferStruct->bufferGroup = NULL;
					ERROR("MediaSplitter","ERROR: BUFFERGROUP --=== NO_INIT ===--\n");
					err = B_NO_INIT;
				}
			}
		}
	}
	return err;
}
BParameterWeb* MediaSplitter::GetParameterWeb()
{
	INFO("MediaSplitter","GetParameterWeb()\n");
	return NULL;
}

void MediaSplitter::HandleBufferWrap( BBuffer *inBuffer, bigtime_t lateness )
{
	if ((!firstBufferRecived) && (sz<0))
	{
		firstBufferRecived=true;
		sz=inBuffer->SizeAvailable();
		DE_BUG("MediaSplitter","FirstInbuffer = %ld\n",sz);
		AllocateInputBufferGroup();
		AllocateOutputBufferGroup(-1);
	}
	
	int32 destinationID = inBuffer->Header()->destination;
	if( (destinationID==filterInput.destination.id) && running )
	{
		bigtime_t start = TimeSource()->RealTime();
/*		size_t sz=  filterOutput->format.u.raw_video.display.line_width *filterOutput->format.u.raw_video.display.line_count;
		BBuffer *outBuffer = outputBufferGroup->RequestBuffer(sz*4,400000);*/
		if (filterOutputList->CountItems()>0)
		{
			int32 i=1;
			status_t err=B_OK;
			media_output* 	tmpOutput			=new media_output;
			outBufferStruct* tmpBufferStruct	=new outBufferStruct;
			for (i=1;i<(filterOutputList->CountItems()-1);i++)
			{
				tmpOutput=(media_output *)filterOutputList->ItemAt(i);
				tmpBufferStruct=(outBufferStruct*) outputBufferGroupList->ItemAt(i);
				int32 counts;
				BBuffer *outBuffer=tmpBufferStruct->bufferGroup->RequestBuffer(inBuffer->SizeUsed());
				//BBuffer *outBuffer = NULL;
				//err=tmpBufferStruct->bufferGroup->RequestBuffer(outBuffer,40000);
				DE_BUG("MediaSplitter","RequestBuffer(%ld) Size=%ld Error=%s\n",i,inBuffer->SizeUsed(),strerror(tmpBufferStruct->bufferGroup->RequestError()));
				if ((outBuffer)&&(err == B_OK))
				{
					media_header *out_header = outBuffer->Header();
					area_id tmpid= out_header->owner;
					media_header *in_header = inBuffer->Header();
				
					memcpy(out_header,in_header,sizeof(media_header));
					out_header->owner =	tmpid;
					memcpy(outBuffer->Data(),inBuffer->Data(), inBuffer->SizeUsed());
					tmpOutput=(media_output*)filterOutputList->ItemAt(i);
					err = SendBuffer(outBuffer,tmpOutput->destination);
					if (err!=B_OK)			
					{
						outBuffer->Recycle();
						ERROR("MediaSplitter","ERROR: sending BBuffer = %s\n", strerror(err));
					}
				}
				else
				{
					ERROR("MediaSplitter","ERROR:requesting BBuffer %s\n ",strerror(errno));
				}

			}
			//** send the first Buffer without any change
			tmpOutput=(media_output*)filterOutputList->ItemAt(0);
			//tmpBufferStruct= new outBufferStruct;
			err=SendBuffer(inBuffer,tmpOutput->destination);	
			if (err!=B_OK)			
			{
				inBuffer->Recycle();
				ERROR("MediaSplitter","ERROR: sending inBuffer = %s\n", strerror(err));
			}			
			/*if (outBuffer)
			{
			
				//**copy memory here
			}*/

/*			media_header *out_header = outBuffer->Header();
			media_header *in_header = inBuffer->Header();
			out_header->destination=filterOutput->destination.id;
			out_header->type = B_MEDIA_RAW_VIDEO;
			//the size of the input header isnt equal to the size of the outputheader..
			out_header->size_used = sz*4;
			out_header->start_time = in_header->start_time; // A changer !! IMPORTANT
			out_header->file_pos = in_header->file_pos;
			inBuffer->Recycle();
			status_t err=SendBuffer(outBuffer,filterOutput->destination);
			if (err!=B_OK)			
				outBuffer->Recycle();*/
		}
		bigtime_t end = TimeSource()->RealTime();
		bigtime_t measuredLatency = (end-start)+500;
		if (measuredLatency > internalLatency +slowerLatencyTolerance|| measuredLatency < internalLatency - fasterLatencyTolerance )
		{
			internalLatency=measuredLatency;
			SetEventLatency(internalLatency);
			//tell the Consumer if connected with us that his downstreamlatency hase changed
			SendLatencyChange(filterInput.source, filterInput.destination, EventLatency() + SchedulingLatency());
		}
	} 
	else 
	{
		ERROR("MediaSplitter","HandleBufferWrap: Unhandled Destination (%i) or nor running. Recycling.\n", destinationID );
		inBuffer->Recycle();
	}
}

void MediaSplitter::PrepareNewOutput()
{
	INFO("MediaSplitter","PrepareNewOutput()\n");
	filterOutput				= new media_output();
	char *connectionName		= new char[B_MEDIA_NAME_LENGTH];
	filterOutput->node 			= Node();
	filterOutput->source.port	= ControlPort();
	filterOutput->source.id		= filterOutputList->CountItems();
	filterOutput->destination	= media_destination::null;

	sprintf(connectionName,"MediaSplitter Output %ld",filterOutputList->CountItems());
	strcpy(filterOutput->name, connectionName);
	
	if (filterInput.source == media_source::null)
	{
		filterOutput->format			= outputFormatBase;
	}
	else
	{
		filterOutput->format.type	= filterInput.format.type;
		filterOutput->format			= filterInput.format;
	}
	DE_BUG("MediaSplitter","Connected Outputs=%ld\n",filterOutputList->CountItems());
	filterOutputList->AddItem((void *)filterOutput);
	outBufferStruct		*tmpBufferStruct = new outBufferStruct;
	tmpBufferStruct->bufferGroup	= NULL;
	tmpBufferStruct->ownBufferGroup	= true;;
	outputBufferGroupList->AddItem((void *)tmpBufferStruct);
}

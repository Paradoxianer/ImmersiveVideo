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


#include "ImmersiveVideoFilter.h"
#include "PluginManager.h"


int32 ImmersiveVideoFilter::instances = 0;


ImmersiveVideoFilter::ImmersiveVideoFilter( BMediaAddOn *addon, const char *name, int32 internal_id):	BMediaNode(name),BBufferProducer(B_MEDIA_RAW_VIDEO),BBufferConsumer(B_MEDIA_RAW_VIDEO),BMediaEventLooper(),BControllable()
{
	initStatus = B_NO_INIT;
	//count the generated instances
	if (atomic_add(&instances, 1) >= MAX_ALLOWED_INSTANCES)
		return;
	//save the id of this instance
	internalID=internal_id;
	//save the addon from wich this instance created
	fAddOn = addon;
	//init BasicFormat for negotiation process
	outputFormatBase.type = B_MEDIA_RAW_VIDEO;
	outputFormatBase.u.raw_video.field_rate=25;
	outputFormatBase.u.raw_video = media_raw_video_format::wildcard;
	outputFormatBase.u.raw_video.display.format = B_NO_COLOR_SPACE;
	inputFormatBase.type = B_MEDIA_RAW_VIDEO;
	inputFormatBase.u.raw_video.field_rate = 25;
	inputFormatBase.u.raw_video = media_raw_video_format::wildcard;
	inputFormatBase.u.raw_video.display.format = B_NO_COLOR_SPACE;
	outputEnabled = true;
	downstreamLatency =0;
	internalLatency	=0;
	running=false;
	outDim=NULL;
	inDim=NULL;
	lTable=NULL;
	initStatus = B_OK;
}

ImmersiveVideoFilter::~ImmersiveVideoFilter()
{
	BMediaEventLooper::Quit();
	int32 changeTag;
	//set the OutputBuffer to NULL so that the MediaKit could delet it.
	SetOutputBuffersFor(filterInput.source,filterInput.destination,NULL,NULL,&changeTag);
	if (initStatus == B_OK) 
	{
		if (filterOutput.destination!=media_destination::null)
			Disconnect(filterOutput.source, filterOutput.destination);
		if (filterInput.source!=media_source::null)
			Disconnect(filterInput.source, filterInput.destination);
	}
	//this instance ist going down...
	atomic_add(&instances, -1);
}

BMediaAddOn *
ImmersiveVideoFilter::AddOn(int32 *internal_id) const
{
	INFO("ImmersiveVideoFilter","AddOn(%ld)\n",*internal_id);
	if (internal_id)
		*internal_id = internalID;
	return fAddOn;
}


status_t 
ImmersiveVideoFilter::HandleMessage(int32 message, const void *data, size_t size)
{
	INFO("ImmersiveVideoFilter","HandleMessage()");
	if (message==SET_LOOKUPTABLE)
	{
			media_timed_event event(TimeSource()->Now(), SET_LOOKUPTABLE, NULL, BTimedEventQueue::B_NO_CLEANUP);
			EventQueue()->AddEvent(event);
	}
	else if(	BBufferConsumer::HandleMessage(message, data, size) &&
		BBufferProducer::HandleMessage(message, data, size) &&
		BControllable::HandleMessage(message, data, size) &&
		BMediaEventLooper::HandleMessage(message, data, size) &&
		BMediaNode::HandleMessage(message, data, size))
		BMediaNode::HandleBadMessage(message, data, size);
	return B_OK;
}

void 
ImmersiveVideoFilter::Preroll()
{
	INFO("ImmersiveVideoFilter","Preroll");
	/****************************************************************************
	 *	This hook may be called before the node is started to give the hardware *
	 *	a chance to start. 														*
	 ****************************************************************************/
}



//+++++++++++++++++++++++++++BBufferProducer+++++++++++++++++++++++//
status_t 
ImmersiveVideoFilter::FormatSuggestionRequested(media_type type, int32 /*quality*/, media_format* format)
{
	/****************************************************************************************
	 * FormatSuggestionRequested() is not necessarily part of the format negotiation		*
	 * process; it's simply an interrogation -- the caller wants to see what the node's		*
	 * preferred data format is, given a suggestion by the caller.							*
 	 ****************************************************************************************/
	INFO("ImmersiveVideoFilter","FormatSuggestionRequested");							
	if (!format)
	{
		ERROR("ImmersiveVideoFilter"," NULL format pointer passed in!");
		return B_BAD_VALUE;
	}

	// this is the format we'll be returning (our base format)
	*format=outputFormatBase;

	if (type == B_MEDIA_UNKNOWN_TYPE) type = B_MEDIA_RAW_VIDEO;
	if (type != B_MEDIA_RAW_VIDEO) 
		return B_MEDIA_BAD_FORMAT;
	else return B_OK;
}

status_t 
ImmersiveVideoFilter::FormatProposal(const media_source& output, media_format* format)
{
	/***************************************************************************************** 
	 *	FormatProposal() is the first stage in the BMediaRoster::Connect() process.  We hand *
	 *	out a suggested format, with wildcards for any variations we support.				 *
	 *****************************************************************************************/
	INFO("ImmersiveVideoFilter","FormatProposal");

	status_t err;

	if (!format)
		return B_BAD_VALUE;

	// is this a proposal for our one output?
	if (output != filterOutput.source)
	{
		ERROR("ImmersiveVideoFilter","FormatProposal returning B_MEDIA_BAD_SOURCE");
		return B_MEDIA_BAD_SOURCE;
	}
	err = format_is_compatible(*format, filterOutput.format) ?
			B_OK : B_MEDIA_BAD_FORMAT;

	//we just return our format ... so the consumer coud fill in his preffered values
	*format = filterOutput.format;
	return err;
}

status_t 
ImmersiveVideoFilter::PrepareToConnect(const media_source &source,
		const media_destination &destination, media_format *format,
		media_source *out_source, char *out_name)
{
	INFO("ImmersiveVideoFilter","PrepareToConnect()");
	//some security checks ;-)
	if (source != filterOutput.source)
		return B_MEDIA_BAD_SOURCE;
	
	if (filterOutput.destination != media_destination::null)
		return B_MEDIA_ALREADY_CONNECTED;

	
	if (!format_is_compatible(*format, outputFormatBase)) 
	{
 		ERROR("ImmersiveVideoFilter","PrepareToConnect - B_MEDIA_BAD_FORMAT");
		 *format=outputFormatBase;
		return B_MEDIA_BAD_FORMAT;
	}
	//nobody would use a Black and white Movie.... but this has to be here because of completenes... and maby sometime somebody ... use it ->DAU ;-)
	if (format->u.raw_video.display.format==B_GRAY1){
 		ERROR("ImmersiveVideoFilter","PrepareToConnect - B_MEDIA_BAD_FORMAT");
		format->u.raw_video.display.format=B_GRAY8;
		return B_MEDIA_BAD_FORMAT;
	}
	format->u.raw_video.display.line_width=filterOutput.format.u.raw_video.display.line_width;
	format->u.raw_video.display.line_count=filterOutput.format.u.raw_video.display.line_count;
	
	*out_source = filterOutput.source;
	strcpy(out_name, filterOutput.name);
	filterOutput.format=*format;
	return B_OK;
}



void 
ImmersiveVideoFilter::Connect(status_t error, const media_source &source,
		const media_destination &destination, const media_format &format,
		char *io_name)
{
	INFO("ImmersiveVideoFilter","Connect");
	bigtime_t start=0, produceLatency=0;
	media_node_id tsID = 0;
	if (filterOutput.destination!=media_destination::null) {
		ERROR("ImmersiveVideoFilter","Connect: Already connected");
		return;
	}

	if (	(source != filterOutput.source) || (error < B_OK) ||
			!const_cast<media_format *>(&format)->Matches(&filterOutput.format)) {
		ERROR("ImmersiveVideoFilter","Connect: Connect error");
		return;
	}
	
	filterOutput.destination = destination;
	//get the name of the conected Consumer input
	strcpy(io_name, filterOutput.name);
	//use this format
	filterOutput.format=format;
	
	// Create the buffer group
	if (AllocateOutputBufferGroup()!=B_OK)
	{
		ERROR("ImmersiveVideoFilter","AllocateOutputBufferGroup Error");
		return;
	}
	//get the Latency for all nodes that follow
	FindLatencyFor(filterOutput.destination, &downstreamLatency, &tsID);
	//prepare the calculation of the own latency
	start = ::system_time();
	uint32 sz =(filterOutput.format.u.raw_video.display.line_width * filterOutput.format.u.raw_video.display.line_count);
	//we always use B__RGB32 ... until now this is wy we multiply sz with 4
	BBuffer *outBuffer = outputBufferGroup->RequestBuffer(sz*4);
	if (outBuffer);
	//	CalcInt32((int32 *)outBuffer->Data(),(int32 *)outBuffer->Data(),sz);
	outBuffer->Recycle();
	produceLatency = ::system_time();
	internalLatency = produceLatency - start;
	internalLatency+=10000;
	DE_BUG("ImmersiveVideoFilter","internal Latency\t%ld\n",(int32)internalLatency);
	SetEventLatency(downstreamLatency + internalLatency);
//	SetEventLatency(internalLatency);
	//tell the Consumer if connected with us that his downstreamlatency hase changed
	if (filterInput.source == media_source::null) 
	{
		filterInput.format=filterOutput.format;
	//	filterInput.format.u.raw_video.display.bytes_per_row=0;
/*		filterInput.format.u.raw_video.display.line_width=0;
		filterInput.format.u.raw_video.display.line_count=0;*/
	}

	SendLatencyChange(filterInput.source, filterInput.destination, EventLatency() + SchedulingLatency());
}

void 
ImmersiveVideoFilter::Disconnect(const media_source& what, const media_destination& where)
{
	INFO("ImmersiveVideoFilter","Disconnect()");

	// Make sure that our connection is the one being disconnected
	if ((where == filterOutput.destination) && (what == filterOutput.source))
	{
		BMediaEventLooper::Stop(TimeSource()->Now(),true);
		filterOutput.destination = media_destination::null;
		filterOutput.format = outputFormatBase;
		delete outputBufferGroup;
		outputBufferGroup = NULL;
	}
	else
	{
		ERROR("ImmersiveVideoFilter","Disconnect() with wrong parameters...");
	}
}



status_t 
ImmersiveVideoFilter::FormatChangeRequested(const media_source &source,
		const media_destination &destination, media_format *io_format,
		int32 *_deprecated_)
{
	//** später mal richtigen "Formatwechsel implementieren**//
	INFO("ImmersiveVideoFilter","FormatChangeRequested");

	if (source != filterOutput.source)
		return B_MEDIA_BAD_SOURCE;
	return B_ERROR;	
}

status_t 
ImmersiveVideoFilter::GetNextOutput(int32 *cookie, media_output *out_output)
{
	INFO("ImmersiveVideoFilter","GetNextOutput(%ld, ...)\n",*cookie);

	if (!out_output)
		return B_BAD_VALUE;

	if ((*cookie) != 0)
		return B_BAD_INDEX;
	*out_output = filterOutput;
	(*cookie)++;
	return B_OK;
}

status_t 
ImmersiveVideoFilter::DisposeOutputCookie(int32 cookie)
{
	INFO("ImmersiveVideoFilter","DisposeOutputCookie()");
	return B_OK;
}

status_t 
ImmersiveVideoFilter::SetBufferGroup(const media_source &for_source,
		BBufferGroup *newGroup)
{
	INFO("ImmersiveVideoFilter","SetBufferGroup");
	// verify that we didn't get bogus arguments before we proceed
	if (for_source != filterOutput.source) return B_MEDIA_BAD_SOURCE;

	// Are we being passed the buffer group we're already using?
	if (newGroup == outputBufferGroup) return B_OK;

	// Ahh, someone wants us to use a different buffer group.  At this point we delete
	// the one we are using and use the specified one instead.  If the specified group is
	// NULL, we need to recreate one ourselves, and use *that*.  Note that if we're
	// caching a BBuffer that we requested earlier, we have to Recycle() that buffer
	// *before* deleting the buffer group, otherwise we'll deadlock waiting for that
	// buffer to be recycled!
	if (ownBufferGroup)
	{
		// waits for all buffers to recycle
		//outputBufferGroup->ReclaimAllBuffers();
		delete outputBufferGroup;		
		outputBufferGroup=NULL;
	}
	if (newGroup != NULL)
	{
		// we were given a valid group; just use that one from now on
		ownBufferGroup=false;
		outputBufferGroup = newGroup;
	}
	else
	{
		// we were passed a NULL group pointer; that means we construct
		// our own buffer group to use from now on
		return AllocateOutputBufferGroup();
	}

	return B_OK;
}

status_t 
ImmersiveVideoFilter::GetLatency(bigtime_t* out_latency)
{
	INFO("ImmersiveVideoFilter","GetLatency");

	// report our *total* latency:  internal plus downstream plus scheduling
	*out_latency = EventLatency() + SchedulingLatency();
	return B_OK;
}



status_t 
ImmersiveVideoFilter::VideoClippingChanged(const media_source &for_source,
		int16 num_shorts, int16 *clip_data,
		const media_video_display_info &display, int32 *_deprecated_)
{
	INFO("ImmersiveVideoFilter","VideoClippingChanged()");
	// we need to produce ... all not only cipped region....... because other Consumer maby need this stuff
	return B_ERROR;
}

void 
ImmersiveVideoFilter::LateNoticeReceived(const media_source& what, bigtime_t how_much, bigtime_t performance_time)
{
	INFO("ImmersiveVideoFilter","LateNoticeReceived");
	if (what != filterOutput.source)
	{
		ERROR("ImmersiveVideoFilter","Bad source.");
		return;
	}

	if (filterInput.source == media_source::null)
	{
		ERROR("ImmersiveVideoFilter","!!! No input to blame.");
		return;
	}
	// pass the buck, since this node doesn't schedule buffer production
	//** but maby we shoud fix our internal latency??
	NotifyLateProducer(filterInput.source, how_much,performance_time);
}


void ImmersiveVideoFilter::EnableOutput(const media_source& what, bool enabled, int32* _deprecated_)
{
	INFO("ImmersiveVideoFilter","EnableOutput");
	if (what == filterOutput.source)
	{
		outputEnabled = enabled;
	}
}



status_t 
ImmersiveVideoFilter::SetPlayRate(int32 numer, int32 denom)
{
	INFO("ImmersiveVideoFilter","SetPlayRate()");
	//***stent to the FiterInput Producer but how??
	return B_ERROR;
}



void 
ImmersiveVideoFilter::AdditionalBufferRequested(const media_source &source,
		media_buffer_id prev_buffer, bigtime_t prev_time,
		const media_seek_tag *prev_tag)
{
	INFO("ImmersiveVideoFilter","AdditionalBufferRequest()");
	RequestAdditionalBuffer(filterInput.source, OfflineTime());
}

void 
ImmersiveVideoFilter::LatencyChanged(const media_source &source,
		const media_destination &destination, bigtime_t new_latency,
		uint32 flags)
{
	INFO("ImmersiveVideoFilter","LatencyChanged()");
	if (source != filterOutput.source)
	{
		ERROR("ImmersiveVideoFilter","\tBad source.");
		return;
	}
	if (destination != filterOutput.destination)
	{
		ERROR("ImmersiveVideoFilter","\tBad destination.");
		return;
	}
	
	downstreamLatency = new_latency;
	SetEventLatency(downstreamLatency + internalLatency);
	
	if (filterInput.source != media_source::null)
	{
		// pass new latency upstream
		status_t err = SendLatencyChange(filterInput.source, filterInput.destination, EventLatency() + SchedulingLatency());
		if(err < B_OK)
			DE_BUG("ImmersiveVideoFilter","SendLatencyChange(): %s\n", strerror(err));
	}

}
//--------------------------BBufferProducer-----------------------//


status_t 
ImmersiveVideoFilter::AcceptFormat(const media_destination& dest, media_format* format)
{
	INFO("ImmersiveVideoFilter","AcceptFormat()");
	status_t err=B_OK;
	// return an error if this isn't really our one input's destination
	if (dest != filterInput.destination) 
		err=B_MEDIA_BAD_DESTINATION;

	if (format->type == B_MEDIA_UNKNOWN_TYPE) 
		format->type = B_MEDIA_RAW_VIDEO;

	if (format->type != B_MEDIA_RAW_VIDEO) 
		err=B_MEDIA_BAD_FORMAT;
	
	if ((err==B_OK) &&
		(format->u.raw_video.display.format==B_GRAY1))
	{
		format->u.raw_video.display.format=B_GRAY8;
		err=B_MEDIA_BAD_FORMAT;
	}
	format->u.raw_video.pixel_width_aspect=1;
	format->u.raw_video.pixel_height_aspect=1;
	format->u.raw_video.display.bytes_per_row=format->u.raw_video.display.line_width*4;
	// the destination given really is our input, and we accept any kind of media data,
	// so now we just confirm that we can handle whatever the producer asked for.
	return err;
}

status_t 
ImmersiveVideoFilter::Connected(
	const media_source& producer,
	const media_destination& where,
	const media_format& with_format,
	media_input* out_input)
{
	status_t err;
	char *formatString=new char[512];
	INFO("ImmersiveVideoFilter","Connected()");
	DE_BUG("ImmersiveVideoFilter","LineWidth=%ld, LineCount=%ld\n",with_format.u.raw_video.display.line_width,with_format.u.raw_video.display.line_count);

	if (where != filterInput.destination) return B_MEDIA_BAD_DESTINATION;
	// record useful information about the connection, and return success
	filterInput.source = producer;
	filterInput.format =with_format;
	*out_input = filterInput;
//	filterOutputFormat.u.raw_video.field_rate=with_format.u.raw_video.field_rate;
	err=AllocateInputBufferGroup();
	DE_BUG("ImmersiveVideoFilter","AllocateInputBufferGroup: %s\n",strerror(err));
	if (filterOutput.destination==media_destination::null)
	{
		filterOutput.format=media_format();
		filterOutput.format.type = B_MEDIA_RAW_VIDEO;
//		filterOutput.format=filterInput.format;
		filterOutput.format.u.raw_video.field_rate=filterInput.format.u.raw_video.field_rate;
		filterOutput.format.u.raw_video.pixel_width_aspect=filterInput.format.u.raw_video.pixel_width_aspect;
		filterOutput.format.u.raw_video.pixel_height_aspect=filterInput.format.u.raw_video.pixel_height_aspect;
		filterOutput.format.u.raw_video.display.format=filterInput.format.u.raw_video.display.format;
		filterOutput.format.u.raw_video.display.bytes_per_row=0;
/*		filterOutput.format.u.raw_video.display.line_width=0;
		filterOutput.format.u.raw_video.display.line_count=0;*/
	}
	BRect *rect=new BRect(0,0,filterInput.format.u.raw_video.display.line_width-1,filterInput.format.u.raw_video.display.line_count-1);
	DE_BUG("ImmersiveVideoFilter","LineWidth=%ld LineCount=%ld\n",with_format.u.raw_video.display.line_width,with_format.u.raw_video.display.line_count);
	rect->PrintToStream();
	pluginManager->SetInputRect(rect);
	pluginManager->SetOutputRect(rect);
	pluginManager->RunCalc();
	
	/*if (filterOutput.destination!=media_destination::null)
	{
		ChangeFormat(filterOutput.source,filterOutput.destination,&format);
	}*/

	return B_OK;
}

void 
ImmersiveVideoFilter::Disconnected(
	const media_source& producer,
	const media_destination& where)
{
	INFO("ImmersiveVideoFilter","Disconnected()");
	if (filterInput.source != producer)
	{
		ERROR("ImmersiveVideoFilter","source mismatch: expected ID %ld, got %ld\n",	filterInput.source.id, producer.id);
		return;
	}
	if (where != filterInput.destination)
	{
		ERROR("ImmersiveVideoFilter","destination mismatch: expected ID %ld, got %ld\n", filterInput.destination.id,where.id);
		return;
	}
	
	BMediaEventLooper::Stop(TimeSource()->Now(),true);
	int32 changeTag;
	SetOutputBuffersFor(producer,where,NULL,NULL,&changeTag);
	// mark disconnected
	
	filterInput.source = media_source::null;
	filterInput.format=inputFormatBase;
	
	// no output? clear format:
	if (filterOutput.destination == media_destination::null)
	{
		filterOutput.format = outputFormatBase;
	}
}



status_t 
ImmersiveVideoFilter::GetNextInput(int32* cookie, media_input* in_input)
{
	INFO("ImmersiveVideoFilter","GetNextInput(%ld,...)\n",*cookie);
	if (!in_input)
		return B_BAD_VALUE;

	if ((*cookie) != 0)
		return B_BAD_INDEX;
	*in_input = filterInput;
	(*cookie)++;
	return B_OK;

}

void 
ImmersiveVideoFilter::DisposeInputCookie(int32 /*cookie*/ cookie)
{
	INFO("ImmersiveVideoFilter","DisposeInputCookie()");
	// we don't use any kind of state or extra storage for iterating over our
	// inputs, so we don't have to do any special disposal of input cookies.
	cookie--;
	
}

void 
ImmersiveVideoFilter::BufferReceived(BBuffer* inBuffer)
{
	INFO("ImmersiveVideoFilter","BufferReceived():");		
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




void ImmersiveVideoFilter::ProducerDataStatus(const media_destination & for_whom, int32 status, bigtime_t at_performance_time)
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
ImmersiveVideoFilter::GetLatencyFor(const media_destination& for_whom, bigtime_t* out_latency, media_node_id* out_timesource)
{
	INFO("ImmersiveVideoFilter","GetLatencyFor()");
	// make sure this is one of my valid inputs
	if (for_whom != filterInput.destination) return B_MEDIA_BAD_DESTINATION;

	// report internal latency + downstream latency here, NOT including scheduling latency.
	*out_latency = downstreamLatency+internalLatency;
	*out_timesource = TimeSource()->ID();
	return B_OK;
}


status_t 
ImmersiveVideoFilter::FormatChanged(
	const media_source& producer,
	const media_destination& consumer,
	int32 change_tag,
	const media_format& format)
{
	INFO("ImmersiveVideoFilter","FormatChanged()");
	status_t err=B_OK;
	BRect *rect=new BRect(0,0,filterInput.format.u.raw_video.display.line_width-1,filterInput.format.u.raw_video.display.line_count-1);
	rect->PrintToStream();
	pluginManager->SetInputRect(rect);
	pluginManager->SetOutputRect(rect);
	pluginManager->RunCalc();
	//err=AllocateInputBufferGroup();
//	ERROR("\tAllocateInputBufferGroup: %s\n",strerror(err));
	//int32 changeTag;
	//err=SetOutputBuffersFor(producer,consumer,inputBufferGroup,NULL,&changeTag);
//	ERROR("\tSetOutputBuffersFor: %s\n",strerror(err));
	return err;
}

status_t 
ImmersiveVideoFilter::SeekTagRequested(
	const media_destination& destination,
	bigtime_t in_target_time,
	uint32 in_flags,
	media_seek_tag* out_seek_tag,
	bigtime_t* out_tagged_time,
	uint32* out_flags)
{
	INFO("ImmersiveVideoFilter","SeekTagRequested()");
	//**pass it through
	return B_ERROR;
}


void 
ImmersiveVideoFilter::NodeRegistered()
{
	if (initStatus != B_OK) 
	{
		ReportError(B_NODE_IN_DISTRESS);
		return;
	}

	INFO("ImmersiveVideoFilter","NodeRegistered()");
	filterOutput.node = Node();
	filterOutput.source.port = ControlPort();
	filterOutput.source.id = 0;
	filterOutput.destination = media_destination::null;
	strcpy(filterOutput.name, "ImmersiveVideoFilter Output");	

	filterOutput.format=outputFormatBase;

	filterInput.destination.port = ControlPort();
	filterInput.destination.id = 0;
	filterInput.source = media_source::null;
	filterInput.node = Node();

	filterInput.format=inputFormatBase;


	strcpy(filterInput.name, "ImmersiveVideoFilter Input");
	pluginManager = new PluginManager(this,0);
	pluginManager->SetInputRect(new BRect(0,0,720,576));
	pluginManager->SetOutputRect(new BRect(0,0,720,576));
	SetParameterWeb(pluginManager->GetParameterWeb());
	Run();
}

status_t ImmersiveVideoFilter::GetParameterValue(int32 id, bigtime_t *last_change, void *value, size_t *size)
{
	return pluginManager->GetParameterValue(id,last_change,value,size);
}

void ImmersiveVideoFilter::SetParameterValue(int32 id, bigtime_t when, const void *value, size_t size)
{
	pluginManager->SetParameterValue(id,when,value,size);
}

status_t ImmersiveVideoFilter::StartControlPanel(BMessenger *out_messenger)
{
	return BControllable::StartControlPanel(out_messenger);
}

void 
ImmersiveVideoFilter::HandleEvent(const media_timed_event *event,
		bigtime_t lateness, bool realTimeEvent)
{
//	INFO("ImmersiveVideoFilter::HandleEvent()");
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
		case SET_LOOKUPTABLE:
			{
				LoadConfig();
				break;
			}
		default:
			ERROR("ImmersiveVideoFilter","Unhandled Event in FMediaNode (%i)\n", (int)event->type );
			break;
	}
}



void ImmersiveVideoFilter::CalcInt32(int32 *inbuffer, int32* outbuffer, uint64 size)
{
//	INFO("ImmersiveVideoFilter::CalcInt32()");
	uint64 i=0;

	for (i=0; i<size ;i++)
	{
		outbuffer[i]=inbuffer[lTable[i]];
	}
}

void ImmersiveVideoFilter::CalcInt16(int16 *inbuffer, int16* outbuffer, uint64 size)
{
//	INFO("ImmersiveVideoFilter::CalcInt16()");
	uint64 i;
	for (i=size; i>0;i--)
	{
		outbuffer[i]=inbuffer[i];
	}
}

void ImmersiveVideoFilter::CalcInt8(int8 *inbuffer, int8* outbuffer, uint64 size)
{
//	INFO("ImmersiveVideoFilter::CalcInt8()");
	uint64 i=0;
	for (i=0;i<size;i++)
	{
		outbuffer[i]=inbuffer[i];
	}
}

void ImmersiveVideoFilter::NoCalc(uint32 *inbuffer, uint32* outbuffer, uint64 size)
{
//	INFO("ImmersiveVideoFilter::NoCalc()");
	uint32 i=0;
	for (i=0; i<size;i++)
	{
		outbuffer[i]=inbuffer[i];
	}
}


status_t
ImmersiveVideoFilter::AllocateInputBufferGroup()
{
	INFO("ImmersiveVideoFilter","AllocateInputBufferGroup()");	
	int32 changeTag;
	if (inputBufferGroup)
	{
		SetOutputBuffersFor(filterInput.source,filterInput.destination,NULL,NULL,&changeTag);
	}
	uint32 sz=4 * filterInput.format.u.raw_video.display.line_width *filterInput.format.u.raw_video.display.line_count;
	
	inputBufferGroup = new BBufferGroup(sz, 3);
	if (inputBufferGroup->InitCheck() < B_OK)
	{
		delete inputBufferGroup;
		inputBufferGroup = NULL;
		return B_NO_INIT;
	}
	else
		SetOutputBuffersFor(filterInput.source,filterInput.destination,inputBufferGroup,NULL,&changeTag);
	return B_OK;
}


status_t
ImmersiveVideoFilter::AllocateOutputBufferGroup()
{
	INFO("ImmersiveVideoFilter","AllocateOutputBufferGroup()");
	ownBufferGroup=true;

	uint32 sz=4 * filterOutput.format.u.raw_video.display.line_width *filterOutput.format.u.raw_video.display.line_count;
	outputBufferGroup = new BBufferGroup(sz, 1);
	if (outputBufferGroup->InitCheck() < B_OK)
	{
		delete outputBufferGroup;
		outputBufferGroup = NULL;
		return B_NO_INIT;
	}
	return B_OK;
}


void ImmersiveVideoFilter::HandleBufferWrap( BBuffer *inBuffer, bigtime_t lateness )
{
	int32 destinationID = inBuffer->Header()->destination;
	if( (destinationID==filterInput.destination.id) && running )
	{
		bigtime_t start = TimeSource()->RealTime();
			uint64 sz=  filterOutput.format.u.raw_video.display.line_width *filterOutput.format.u.raw_video.display.line_count;
//			BBuffer *outBuffer = outputBufferGroup->RequestBuffer(sz*4,400000);
			BBuffer *outBuffer = outputBufferGroup->RequestBuffer(sz*4);
			if ((outBuffer)&&(inBuffer)&&(lTable))
			{
				CalcInt32((int32 *)inBuffer->Data(), (int32 *)outBuffer->Data(),sz);
			}
			else
				NoCalc((uint32 *)inBuffer->Data(), (uint32 *)outBuffer->Data(),sz);
			media_header *out_header = outBuffer->Header();
			media_header *in_header = inBuffer->Header();

			out_header->destination=filterOutput.destination.id;
			out_header->type = B_MEDIA_RAW_VIDEO;
			//the size of the input header isnt equal to the size of the outputheader..
			out_header->size_used = sz*4;
			out_header->start_time = in_header->start_time; // A changer !! IMPORTANT
			out_header->file_pos = in_header->file_pos;
			inBuffer->Recycle();
			status_t err=SendBuffer(outBuffer,filterOutput.destination);
			if (err!=B_OK)			
				outBuffer->Recycle();
			
		bigtime_t end = TimeSource()->RealTime();
		bigtime_t measuredLatency = (end-start);
		DE_BUG("ImmersiveVideoFilter","Latency\t%d\n",measuredLatency);
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
		ERROR("ImmersiveVideoFilter","HandleBufferWrap: Unhandled Destination (%i) or nor running. Recycling.\n", destinationID );
		inBuffer->Recycle();
	}
}
void ImmersiveVideoFilter::LoadConfig()
{
	INFO("ImmersiveVideoFilter","LoadConfig()");
	lTable=pluginManager->GetLookUpTableInt();
	outDim=pluginManager->GetOutputRect();
	//+1 muss sein da hier die null mit eingerechnet wird
	filterOutput.format.u.raw_video.display.line_width		= (outDim->IntegerWidth()+1);
	filterOutput.format.u.raw_video.display.line_count		= (outDim->IntegerHeight()+1);
	filterOutput.format.u.raw_video.display.bytes_per_row	= filterOutput.format.u.raw_video.display.line_width*4;
	//eventuell neue Buffergröße...
	DE_BUG("ImmersiveVideoFilter","LoadCOnfig::Widht: = %ld\n",filterOutput.format.u.raw_video.display.line_width );
	DE_BUG("ImmersiveVideoFilter","LoadCOnfig::Height: = %ld\n",filterOutput.format.u.raw_video.display.line_count );
	//**alte Dim deleten??
	outDim=pluginManager->GetOutputRect();
	DE_BUG("ImmersiveVideoFilter","outDim =%r ",outDim);
	inDim=pluginManager->GetInputRect();
	DE_BUG("ImmersiveVideoFilter","inDim = %r",inDim);
		
	AllocateOutputBufferGroup();
	if (filterOutput.destination!=media_destination::null)
		ChangeFormat(filterOutput.source,filterOutput.destination,&filterOutput.format);
	//nächsten Consumer benachrichtigen, dass die Daten sich geändert haben.
	//How handle the "CHANGE_IN_PROGRESS EXCEPTION
	
}

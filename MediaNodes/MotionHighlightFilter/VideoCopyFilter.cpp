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


#include "VideoCopyFilter.h"



int32 VideoCopyFilter::instances = 0;


VideoCopyFilter::VideoCopyFilter( BMediaAddOn *addon, const char *name, int32 internal_id):	BMediaNode(name),BBufferProducer(B_MEDIA_RAW_VIDEO),BBufferConsumer(B_MEDIA_RAW_VIDEO),BMediaEventLooper(),BControllable()
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
	outputFormatBase.u.raw_video = media_raw_video_format::wildcard;
	outputFormatBase.u.raw_video.display.format = B_NO_COLOR_SPACE;
	inputFormatBase.type = B_MEDIA_RAW_VIDEO;
	inputFormatBase.u.raw_video = media_raw_video_format::wildcard;
	inputFormatBase.u.raw_video.display.format = B_NO_COLOR_SPACE;
	outputEnabled = true;
	downstreamLatency =0;
	internalLatency	=0;
	running=false;
	initStatus = B_OK;
}

VideoCopyFilter::~VideoCopyFilter()
{
	BMediaEventLooper::Quit();
	int32 changeTag;
	//set the OutputBuffer to NULL so that the MediaKit coud delet it.
	SetOutputBuffersFor(filterInput.source,filterInput.destination,NULL,NULL,&changeTag);
	if (initStatus == B_OK) {
		if (filterOutput.destination!=media_destination::null)
			Disconnect(filterOutput.source, filterOutput.destination);
		if (filterInput.source!=media_source::null)
			Disconnect(filterInput.source, filterInput.destination);
	}
	//this instance ist going down...
	atomic_add(&instances, -1);
}


BMediaAddOn *
VideoCopyFilter::AddOn(int32 *internal_id) const
{
	INFO("VideoCopyFilter::AddOn(%ld)\n",*internal_id);
	if (internal_id)
		*internal_id = internalID;
	return fAddOn;
}


status_t 
VideoCopyFilter::HandleMessage(int32 message, const void *data, size_t size)
{

	if(	BBufferConsumer::HandleMessage(message, data, size) &&
		BBufferProducer::HandleMessage(message, data, size) &&
		BMediaNode::HandleMessage(message, data, size))
		BMediaNode::HandleBadMessage(message, data, size);
	return B_OK;
}

void 
VideoCopyFilter::Preroll()
{
	INFO("VideoCopyFilter::Preroll\n");
	/****************************************************************************
	 *	This hook may be called before the node is started to give the hardware *
	 *	a chance to start. 														*
	 ****************************************************************************/
}



//+++++++++++++++++++++++++++BBufferProducer+++++++++++++++++++++++//
status_t 
VideoCopyFilter::FormatSuggestionRequested(media_type type, int32 /*quality*/, media_format* format)
{
	/****************************************************************************************
	 * FormatSuggestionRequested() is not necessarily part of the format negotiation		*
	 * process; it's simply an interrogation -- the caller wants to see what the node's		*
	 * preferred data format is, given a suggestion by the caller.							*
 	 ****************************************************************************************/
	INFO("VideoCopyFilter::FormatSuggestionRequested\n");							
	if (!format)
	{
		ERROR("\tERROR - NULL format pointer passed in!\n");
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
VideoCopyFilter::FormatProposal(const media_source& output, media_format* format)
{
	/***************************************************************************************** 
	 *	FormatProposal() is the first stage in the BMediaRoster::Connect() process.  We hand *
	 *	out a suggested format, with wildcards for any variations we support.				 *
	 *****************************************************************************************/
	INFO("VideoCopyFilter::FormatProposal\n");

	status_t err;

	if (!format)
		return B_BAD_VALUE;

	// is this a proposal for our one output?
	if (output != filterOutput.source)
	{
		ERROR("VideoCopyFilter::FormatProposal returning B_MEDIA_BAD_SOURCE\n");
		return B_MEDIA_BAD_SOURCE;
	}
	err = format_is_compatible(*format, outputFormatBase) ?
			B_OK : B_MEDIA_BAD_FORMAT;

	//we just return our format ... so the consumer coud fill in his preffered values
	*format = outputFormatBase;
	return err;
}

status_t 
VideoCopyFilter::PrepareToConnect(const media_source &source,
		const media_destination &destination, media_format *format,
		media_source *out_source, char *out_name)
{
	INFO("VideoCopyFilter::PrepareToConnect()\n");
	//some security checks ;-)
	if (source != filterOutput.source)
		return B_MEDIA_BAD_SOURCE;
	
	if (filterOutput.destination != media_destination::null)
		return B_MEDIA_ALREADY_CONNECTED;

	
	if (!format_is_compatible(*format, outputFormatBase)) 
	{
 		ERROR("VideoCopyFilter::PrepareToConnect - B_MEDIA_BAD_FORMAT\n");
		 *format=outputFormatBase;
		return B_MEDIA_BAD_FORMAT;
	}
	//nobody would use a Black and white Movie.... but this has to be here because of completenes... and maby sometime somebody ... use it ->DAU ;-)
	if (format->u.raw_video.display.format==B_GRAY1){
 		ERROR("VideoCopyFilter::PrepareToConnect - B_MEDIA_BAD_FORMAT\n");
		format->u.raw_video.display.format=B_GRAY8;
		return B_MEDIA_BAD_FORMAT;
	}
	*out_source = filterOutput.source;
	strcpy(out_name, filterOutput.name);
	filterOutput.format=*format;
	return B_OK;
}



void 
VideoCopyFilter::Connect(status_t error, const media_source &source,
		const media_destination &destination, const media_format &format,
		char *io_name)
{
	INFO("VideoCopyFilter::Connect\n");
	bigtime_t start=0, produceLatency=0;
	media_node_id tsID = 0;
	if (filterOutput.destination!=media_destination::null) {
		ERROR("Connect: Already connected\n");
		return;
	}

	if (	(source != filterOutput.source) || (error < B_OK) ||
			!const_cast<media_format *>(&format)->Matches(&filterOutput.format)) {
		ERROR("Connect: Connect error\n");
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
		ERROR("AllocateOutputBufferGroup Error\n");
		return;
	}
	//get the Latency for all nodes that follow
	FindLatencyFor(filterOutput.destination, &downstreamLatency, &tsID);
	//prepare the calculation of the own latency
	start = ::system_time();
	uint32 sz =(filterOutput.format.u.raw_video.display.line_width * filterOutput.format.u.raw_video.display.line_count);
	//we always use B__RGB32 ... until now this is wy we multiply sz with 4
	BBuffer *outBuffer = outputBufferGroup->RequestBuffer(sz*4,30000);
	CalcInt32((int32 *)outBuffer->Data(),(int32 *)outBuffer->Data(),sz);
	outBuffer->Recycle();
	produceLatency = ::system_time();
	internalLatency = produceLatency - start;
	INFO("internal Latency\t%ld\n",(int32)internalLatency);
//	SetEventLatency(downstreamLatency + internalLatency);
	SetEventLatency(internalLatency);
	//tell the Consumer if connected with us that his downstreamlatency hase changed
	SendLatencyChange(filterInput.source, filterInput.destination, EventLatency() + SchedulingLatency());
}

void 
VideoCopyFilter::Disconnect(const media_source& what, const media_destination& where)
{
	INFO("VideoCopyFilter::Disconnect()\n");

	// Make sure that our connection is the one being disconnected
	if ((where == filterOutput.destination) && (what == filterOutput.source))
	{
		filterOutput.destination = media_destination::null;
		filterOutput.format = outputFormatBase;
		delete outputBufferGroup;
		outputBufferGroup = NULL;
	}
	else
	{
		ERROR("\tDisconnect() with wrong parameters...\n");
	}
}



status_t 
VideoCopyFilter::FormatChangeRequested(const media_source &source,
		const media_destination &destination, media_format *io_format,
		int32 *_deprecated_)
{
	//** später mal richtigen "Formatwechsel implementieren**//
	INFO("VideoCopyFilter::FormatChangeRequested\n");

	if (source != filterOutput.source)
		return B_MEDIA_BAD_SOURCE;
	return B_ERROR;	
}

status_t 
VideoCopyFilter::GetNextOutput(int32 *cookie, media_output *out_output)
{
	INFO("VideoCopyFilter::GetNextOutput(%ld, ...)\n",*cookie);

	if (!out_output)
		return B_BAD_VALUE;

	if ((*cookie) != 0)
		return B_BAD_INDEX;
	*out_output = filterOutput;
	(*cookie)++;
	return B_OK;
}

status_t 
VideoCopyFilter::DisposeOutputCookie(int32 cookie)
{
	INFO("VideoCopyFilter::DisposeOutputCookie()\n");
	return B_OK;
}

status_t 
VideoCopyFilter::SetBufferGroup(const media_source &for_source,
		BBufferGroup *newGroup)
{
	INFO("VideoCopyFilter::SetBufferGroup\n");
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
		outputBufferGroup->ReclaimAllBuffers();
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
VideoCopyFilter::GetLatency(bigtime_t* out_latency)
{
	INFO("VideoCopyFilter::GetLatency\n");

	// report our *total* latency:  internal plus downstream plus scheduling
	*out_latency = EventLatency() + SchedulingLatency();
	return B_OK;
}



status_t 
VideoCopyFilter::VideoClippingChanged(const media_source &for_source,
		int16 num_shorts, int16 *clip_data,
		const media_video_display_info &display, int32 *_deprecated_)
{
	INFO("VideoCopyFilter::VideoClippingChanged()\n");
	// we need to produce ... all not only cipped region....... because other Consumer maby need this stuff
	return B_ERROR;
}

void 
VideoCopyFilter::LateNoticeReceived(const media_source& what, bigtime_t how_much, bigtime_t performance_time)
{
	INFO("VideoCopyFilter::LateNoticeReceived\n");
	if (what != filterOutput.source)
	{
		ERROR(("\tBad source.\n"));
		return;
	}

	if (filterInput.source == media_source::null)
	{
		ERROR(("\t!!! No input to blame.\n"));
		return;
	}
	// pass the buck, since this node doesn't schedule buffer production
	//** but maby we shoud fix our internal latency??
	NotifyLateProducer(filterInput.source, how_much,performance_time);
}


void VideoCopyFilter::EnableOutput(const media_source& what, bool enabled, int32* _deprecated_)
{
	INFO("VideoCopyFilter::EnableOutput\n");
	if (what == filterOutput.source)
	{
		outputEnabled = enabled;
	}
}



status_t 
VideoCopyFilter::SetPlayRate(int32 numer, int32 denom)
{
	INFO("VideoCopyFilter::SetPlayRate()\n");
	//***stent to the FiterInput Producer but how??
	return B_ERROR;
}



void 
VideoCopyFilter::AdditionalBufferRequested(const media_source &source,
		media_buffer_id prev_buffer, bigtime_t prev_time,
		const media_seek_tag *prev_tag)
{
	INFO("VideoCopyFilter::AdditionalBufferRequest()\n");
	RequestAdditionalBuffer(filterInput.source, OfflineTime());
}

void 
VideoCopyFilter::LatencyChanged(const media_source &source,
		const media_destination &destination, bigtime_t new_latency,
		uint32 flags)
{
	INFO("VideoCopyFilter::LatencyChanged()\n");
	if (source != filterOutput.source)
	{
		ERROR(("\tBad source.\n"));
		return;
	}
	if (destination != filterOutput.destination)
	{
		ERROR(("\tBad destination.\n"));
		return;
	}
	
	downstreamLatency = new_latency;
	SetEventLatency(downstreamLatency + internalLatency);
	
	if (filterInput.source != media_source::null)
	{
		// pass new latency upstream
		status_t err = SendLatencyChange(filterInput.source, filterInput.destination, EventLatency() + SchedulingLatency());
		if(err < B_OK)
			DEBUG("\t!!! SendLatencyChange(): %s\n", strerror(err));
	}

}
//--------------------------BBufferProducer-----------------------//


status_t 
VideoCopyFilter::AcceptFormat(const media_destination& dest, media_format* format)
{
	INFO("VideoCopyFilter::AcceptFormat()\n");
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
	// the destination given really is our input, and we accept any kind of media data,
	// so now we just confirm that we can handle whatever the producer asked for.
	return err;
}

status_t 
VideoCopyFilter::Connected(
	const media_source& producer,
	const media_destination& where,
	const media_format& with_format,
	media_input* out_input)
{
	status_t err;
	INFO("VideoCopyFilter::Connected()\n");
	if (where != filterInput.destination) return B_MEDIA_BAD_DESTINATION;
	// record useful information about the connection, and return success
	filterInput.source = producer;
	filterInput.format =with_format;
	*out_input = filterInput;
//	filterOutputFormat.u.raw_video.field_rate=with_format.u.raw_video.field_rate;
	err=AllocateInputBufferGroup();
	DEBUG("\twidth=%ld,height=%ld\n %s\n",filterInput.format.u.raw_video.display.line_width,filterInput.format.u.raw_video.display.line_count);
	DEBUG("\tAllocateInputBufferGroup: %s\n",strerror(err));

	int32 changeTag;
	err=SetOutputBuffersFor(producer,where,inputBufferGroup,NULL,&changeTag);
	outputFormatBase=with_format;

	if (filterOutput.destination!=media_destination::null)
	{
		ChangeFormat(filterOutput.source,filterOutput.destination,&outputFormatBase);
	}
	
	helpbuffer = new uint32[filterInput.format.u.raw_video.display.line_width *filterInput.format.u.raw_video.display.line_count];
	
	DEBUG("\tSetOutputBuffersFor: %s\n",strerror(err));
	return B_OK;
}

void 
VideoCopyFilter::Disconnected(
	const media_source& producer,
	const media_destination& where)
{
	INFO("VideoCopyFilter::Disconnected()\n");
	if (filterInput.source != producer)
	{
		ERROR("\tsource mismatch: expected ID %ld, got %ld\n",	filterInput.source.id, producer.id);
		return;
	}
	if (where != filterInput.destination)
	{
		ERROR("\tdestination mismatch: expected ID %ld, got %ld\n", filterInput.destination.id,where.id);
		return;
	}

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
VideoCopyFilter::GetNextInput(int32* cookie, media_input* in_input)
{
	INFO("VideoCopyFilter::GetNextInput(%ld,...)\n",*cookie);
	if (!in_input)
		return B_BAD_VALUE;

	if ((*cookie) != 0)
		return B_BAD_INDEX;
	*in_input = filterInput;
	(*cookie)++;
	return B_OK;

}

void 
VideoCopyFilter::DisposeInputCookie(int32 /*cookie*/ cookie)
{
	INFO("VideoCopyFilter::DisposeInputCookie()\n");
	// we don't use any kind of state or extra storage for iterating over our
	// inputs, so we don't have to do any special disposal of input cookies.
	cookie--;
	
}

void 
VideoCopyFilter::BufferReceived(BBuffer* inBuffer)
{
	INFO("VideoCopyFilter::BufferReceived():\n");		
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




void VideoCopyFilter::ProducerDataStatus(const media_destination & for_whom, int32 status, bigtime_t at_performance_time)
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
VideoCopyFilter::GetLatencyFor(const media_destination& for_whom, bigtime_t* out_latency, media_node_id* out_timesource)
{
	INFO("VideoCopyFilter::GetLatencyFor()\n");
	// make sure this is one of my valid inputs
	if (for_whom != filterInput.destination) return B_MEDIA_BAD_DESTINATION;

	// report internal latency + downstream latency here, NOT including scheduling latency.
	*out_latency = downstreamLatency+internalLatency;
	*out_timesource = TimeSource()->ID();
	return B_OK;
}


status_t 
VideoCopyFilter::FormatChanged(
	const media_source& producer,
	const media_destination& consumer,
	int32 change_tag,
	const media_format& format)
{
	INFO("VideoCopyFilter::FormatChanged()\n");
	status_t err=B_OK;
	err=AllocateInputBufferGroup();
	ERROR("\tAllocateInputBufferGroup: %s\n",strerror(err));
	int32 changeTag;
	err=SetOutputBuffersFor(producer,consumer,inputBufferGroup,NULL,&changeTag);
	ERROR("\tSetOutputBuffersFor: %s\n",strerror(err));
	return err;
}

status_t 
VideoCopyFilter::SeekTagRequested(
	const media_destination& destination,
	bigtime_t in_target_time,
	uint32 in_flags,
	media_seek_tag* out_seek_tag,
	bigtime_t* out_tagged_time,
	uint32* out_flags)
{
	INFO("VideoCopyFilter::SeekTagRequested()\n");
	//**pass it through
	return B_ERROR;
}


void 
VideoCopyFilter::NodeRegistered()
{
	if (initStatus != B_OK) 
	{
		ReportError(B_NODE_IN_DISTRESS);
		return;
	}

	INFO("VideoCopyFilter::NodeRegistered()\n");
	filterOutput.node = Node();
	filterOutput.source.port = ControlPort();
	filterOutput.source.id = 0;
	filterOutput.destination = media_destination::null;
	strcpy(filterOutput.name, "MotionHighlighter Output");	

	filterOutput.format.type = B_MEDIA_RAW_VIDEO;
	filterOutput.format.u.raw_video = media_raw_video_format::wildcard;

	filterInput.destination.port = ControlPort();
	filterInput.destination.id = 0;
	filterInput.source = media_source::null;
	filterInput.node = Node();

	filterInput.format.type = B_MEDIA_RAW_VIDEO;
	filterInput.format.u.raw_video = media_raw_video_format::wildcard;

	strcpy(filterInput.name, "VideoCopyFilter Input");
	SetParameterWeb(GetParameterWeb());
	Run();
}

status_t VideoCopyFilter::GetParameterValue(int32 id, bigtime_t *last_change, void *value, size_t *size)
{
	float tmp=(float)sensitivity;
	value=(void *)&tmp;
	*size=sizeof(tmp);
	return B_OK;
}

void VideoCopyFilter::SetParameterValue(int32 id, bigtime_t when, const void *value, size_t size)
{
	sensitivity=(int32)*((float *)value);
}

status_t VideoCopyFilter::StartControlPanel(BMessenger *out_messenger)
{
	return BControllable::StartControlPanel(out_messenger);
}

void 
VideoCopyFilter::HandleEvent(const media_timed_event *event,
		bigtime_t lateness, bool realTimeEvent)
{
	INFO("VideoCopyFilter::HandleEvent()\n");
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
			ERROR("Unhandled Event in FMediaNode (%i)\n", (int)event->type );
			break;
	}
}



void VideoCopyFilter::CalcInt32(int32 *inbuffer, int32* outbuffer, uint32 size)
{
	INFO("VideoCopyFilter::CalcInt32()\n");
	uint32 i=0;
	int r, g, b, rh, gh, bh;
	for(i=0; i<size; i++)
	{
		r = inbuffer[i] & 0x00ff0000;
		rh = helpbuffer[i] & 0x00ff0000;
		g = inbuffer[i] & 0x0000ff00;
		gh = helpbuffer[i] & 0x0000ff00;
		b = inbuffer[i] & 0x000000ff;
		bh = helpbuffer[i] & 0x000000ff;
//		if (((b-bh)>>16+(g-gh))+(r-rh)*(r-rh))>10)
		//if((abs((r-rh)>>16)+abs((g-gh)>>8)+abs(b-bh))>sensitivity)
		int rd=(r-rh)>>16;
		int gd=(g-gh)>>8;
		int bd=(b-bh);
		if((rd*rd+gd*gd+bd*bd)>sensitivity)
		{
			outbuffer[i] =0xffff0000;//  (inbuffer[i] + 3276800); //Rot-Wert (max.) plus 50
		}
		else outbuffer[i] = inbuffer[i];
			helpbuffer[i] = inbuffer[i];
	}

}

void VideoCopyFilter::CalcInt16(int16 *inbuffer, int16* outbuffer, uint32 size)
{
	INFO("VideoCopyFilter::CalcInt16()\n");
	uint32 i;
	for (i=size; i>0;i--)
	{
		outbuffer[i]=inbuffer[i];
	}
}

void VideoCopyFilter::CalcInt8(int8 *inbuffer, int8* outbuffer, uint32 size)
{
	INFO("VideoCopyFilter::CalcInt8()\n");
	uint32 i=0;
	for (i=0;i<size;i++)
	{
		outbuffer[i]=inbuffer[i];
	}
}

void VideoCopyFilter::NoCalc(uint32 *inbuffer, uint32* outbuffer, uint32 size)
{
	INFO("VideoCopyFilter::NoCalc()\n");
	uint32 i=0;
	for (i=0; i<size;i++)
	{
		outbuffer[i]=inbuffer[i];
	}
}


status_t
VideoCopyFilter::AllocateInputBufferGroup()
{
	INFO("VideoCopyFilter::AllocateInputBufferGroup()\n");	
	if (inputBufferGroup)
	{
		inputBufferGroup->ReclaimAllBuffers();
		inputBufferGroup->WaitForBuffers();
		delete inputBufferGroup;
	}
	uint32 sz=4 * filterInput.format.u.raw_video.display.line_width *filterInput.format.u.raw_video.display.line_count;
	
	inputBufferGroup = new BBufferGroup(sz, 3);
	if (inputBufferGroup->InitCheck() < B_OK)
	{
		delete inputBufferGroup;
		inputBufferGroup = NULL;
		return B_NO_INIT;
	}

	return B_OK;
}


status_t
VideoCopyFilter::AllocateOutputBufferGroup()
{
	INFO("VideoCopyFilter::AllocateOutputBufferGroup()\n");
	ownBufferGroup=true;

	uint32 sz=4 * filterOutput.format.u.raw_video.display.line_width *filterOutput.format.u.raw_video.display.line_count;
	outputBufferGroup = new BBufferGroup(sz, 2);
	if (outputBufferGroup->InitCheck() < B_OK)
	{
		delete outputBufferGroup;
		outputBufferGroup = NULL;
		return B_NO_INIT;
	}
	return B_OK;
}
BParameterWeb* VideoCopyFilter::GetParameterWeb()
{
	BParameterWeb *web = new BParameterWeb();
	BParameterGroup *moverGroup = web->MakeGroup("Sensitivity");
	BContinuousParameter *move = moverGroup->MakeContinuousParameter(0, B_MEDIA_RAW_VIDEO, "sensitivity", B_GAIN,"",1,720.0,1);
	return web;
}

void VideoCopyFilter::HandleBufferWrap( BBuffer *inBuffer, bigtime_t lateness )
{
	int32 destinationID = inBuffer->Header()->destination;
	if( (destinationID==filterInput.destination.id) && running )
	{
		bigtime_t start = TimeSource()->RealTime();
			size_t sz=  filterOutput.format.u.raw_video.display.line_width *filterOutput.format.u.raw_video.display.line_count;
			BBuffer *outBuffer = outputBufferGroup->RequestBuffer(sz*4,400000);
			if (outBuffer)
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
		bigtime_t measuredLatency = (end-start)+500;
		printf("Latency\t%ld\n",measuredLatency);
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
		ERROR("FMediaNode::HandleBufferWrap: Unhandled Destination (%i) or nor running. Recycling.\n", destinationID );
		inBuffer->Recycle();
	}
}

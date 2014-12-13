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


#include "Weichzeichner.h"
#include "PluginManager.h"

#define PRINTF printf




int32 ImmersiveVideoFilter::instances = 0;


ImmersiveVideoFilter::ImmersiveVideoFilter( BMediaAddOn *addon, const char *name, int32 internal_id):	BMediaNode(name),BBufferProducer(B_MEDIA_RAW_VIDEO),BBufferConsumer(B_MEDIA_RAW_VIDEO),BMediaEventLooper(),BControllable()
{
	initStatus = B_NO_INIT;
	//mitzählen wie viele Instanzen schon erzeugt wurden
	if (atomic_add(&instances, 1) >= MAX_ALLOWED_INSTANCES)
		return;
	//unsere ID merken
	internalID=internal_id;
	//Addon merken
	fAddOn = addon;
	//und unser Medienformat initalisieren
	filterOutputFormat.type = B_MEDIA_RAW_VIDEO;
	filterOutputFormat.u.raw_video = media_raw_video_format::wildcard;
	filterOutputFormat.u.raw_video.interlace = 1;
	filterOutputFormat.u.raw_video.display.format = B_NO_COLOR_SPACE;
	outputEnabled = true;
	initStatus = B_OK;
}

ImmersiveVideoFilter::~ImmersiveVideoFilter()
{
	BMediaEventLooper::Quit();
	if (initStatus == B_OK) {
		if (filterOutput.destination!=media_destination::null)
			Disconnect(filterOutput.source, filterOutput.destination);
		if (filterInput.source!=media_source::null)
			Disconnect(filterInput.source, filterInput.destination);
	}
	atomic_add(&instances, -1);
}

BMediaAddOn *
ImmersiveVideoFilter::AddOn(int32 *internal_id) const
{
	PRINTF("ImmersiveVideoFilter::AddOn(%ld)\n",*internal_id);
	if (internal_id)
		*internal_id = internalID;
	return fAddOn;
}


status_t 
ImmersiveVideoFilter::HandleMessage(int32 message, const void *data, size_t size)
{
	PRINTF("ImmersiveVideoFilter::HandleMessage(%ld,%ld,%ld)\n",message,(uint32)data,size);
	if(
		BBufferConsumer::HandleMessage(message, data, size) &&
		BBufferProducer::HandleMessage(message, data, size) &&
		BMediaNode::HandleMessage(message, data, size))
		BMediaNode::HandleBadMessage(message, data, size);
		return B_OK;
}

void 
ImmersiveVideoFilter::Preroll()
{
	PRINTF("ImmersiveVideoFilter::Preroll\n");
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
	PRINTF("ImmersiveVideoFilter::FormatSuggestionRequested\n");							
	if (!format)
	{
		PRINTF("\tERROR - NULL format pointer passed in!\n");
		return B_BAD_VALUE;
	}

	// this is the format we'll be returning (our preferred format)
	*format=filterOutputFormat;

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
	PRINTF("ImmersiveVideoFilter::FormatProposal\n");

	status_t err;

	if (!format)
		return B_BAD_VALUE;

	// is this a proposal for our one output?
	if (output != filterOutput.source)
	{
		PRINTF("ImmersiveVideoFilter::FormatProposal returning B_MEDIA_BAD_SOURCE\n");
		return B_MEDIA_BAD_SOURCE;
	}
	err = format_is_compatible(*format, filterOutputFormat) ?
			B_OK : B_MEDIA_BAD_FORMAT;

	*format = filterOutputFormat;
	return err;
}

status_t 
ImmersiveVideoFilter::PrepareToConnect(const media_source &source,
		const media_destination &destination, media_format *format,
		media_source *out_source, char *out_name)
{
	PRINTF("ImmersiveVideoFilter::PrepareToConnect()\n");
	if (source != filterOutput.source)
		return B_MEDIA_BAD_SOURCE;
	
	if (filterOutput.destination != media_destination::null)
		return B_MEDIA_ALREADY_CONNECTED;

	
	if (!format_is_compatible(*format, filterOutputFormat)) {
		 *format=filterOutputFormat;
		return B_MEDIA_BAD_FORMAT;
	}
	//nobody would use a Black and white Movie.... but this has to be here because of completenes... and maby sometime somebody ... use it ->DAU ;-)
	if (format->u.raw_video.display.format==B_GRAY1){
		format->u.raw_video.display.format=B_GRAY8;
		return B_MEDIA_BAD_FORMAT;
	}
	*out_source = filterOutput.source;
	strcpy(out_name, filterOutput.name);
	filterOutput.format=*format;
	//filterOutput.destination = destination;
	return B_OK;
}



void 
ImmersiveVideoFilter::Connect(status_t error, const media_source &source,
		const media_destination &destination, const media_format &format,
		char *io_name)
{
	bigtime_t start, produceLatency;

	media_node_id tsID = 0;
	
	PRINTF("ImmersiveVideoFilter::Connect\n");


	if (filterOutput.destination!=media_destination::null) {
		PRINTF("Connect: Already connected\n");
		return;
	}

	if (	(source != filterOutput.source) || (error < B_OK) ||
			!const_cast<media_format *>(&format)->Matches(&filterOutput.format)) {
		PRINTF("Connect: Connect error\n");
		return;
	}
	filterOutput.destination = destination;
	strcpy(io_name, filterOutput.name);
//	filterOutputFormat = format;
	filterOutput.format=format;
	
	/* Create the buffer group */
	if (!bufferGroup)
	{
		if (AllocateBufferGroup()!=B_OK)
		{
			return;
		}
	}
	FindLatencyFor(filterOutput.destination, &downstreamLatency, &tsID);
	start = ::system_time();
	uint32 sz =(filterOutput.format.u.raw_video.display.line_width * filterOutput.format.u.raw_video.display.line_count);
	BBuffer *outBuffer = bufferGroup->RequestBuffer(sz*4,50000);
	CalcInt32((int32 *)outBuffer->Data(),(int32 *)outBuffer->Data(),sz);
	outBuffer->Recycle();
	produceLatency = ::system_time();
	internalLatency = produceLatency - start;
	PRINTF("internal Latency\t%ld\n",internalLatency);
	// noch einen kleinen Nachsprung geben.. 
	internalLatency+=100;
	SetEventLatency(downstreamLatency + internalLatency);
	//** err behandlung	
	//status_t err = 
	SendLatencyChange(filterInput.source, filterInput.destination, EventLatency() + SchedulingLatency());
}

void 
ImmersiveVideoFilter::Disconnect(const media_source& what, const media_destination& where)
{
	PRINTF("ImmersiveVideoFilter::Disconnect()\n");

	// Make sure that our connection is the one being disconnected
	if ((where == filterOutput.destination) && (what == filterOutput.source))
	{
		filterOutput.destination = media_destination::null;
		filterOutput.format = filterOutputFormat;
		delete bufferGroup;
		bufferGroup = NULL;
	}
	else
	{
		PRINTF("\tDisconnect() with wrong parameters...\n");
	}
}



status_t 
ImmersiveVideoFilter::FormatChangeRequested(const media_source &source,
		const media_destination &destination, media_format *io_format,
		int32 *_deprecated_)
{
	//** später mal richtigen "Formatwechsel implementieren**//
	PRINTF("ImmersiveVideoFilter::FormatChangeRequested\n");

	if (source != filterOutput.source)
		return B_MEDIA_BAD_SOURCE;
		
	return B_ERROR;	
}

status_t 
ImmersiveVideoFilter::GetNextOutput(int32 *cookie, media_output *out_output)
{
	PRINTF("ImmersiveVideoFilter::GetNextOutput(%ld, ...)\n",*cookie);

	if (!out_output)
		return B_BAD_VALUE;

	if ((*cookie) != 0)
		return B_BAD_INDEX;
//	filterOutput.format=filterFormat;
//	*out_output = filterOutput;
	*out_output = filterOutput;
	(*cookie)++;
	return B_OK;
}

status_t 
ImmersiveVideoFilter::DisposeOutputCookie(int32 cookie)
{
	PRINTF("ImmersiveVideoFilter::DisposeOutputCookie()\n");
	return B_OK;
}

status_t 
ImmersiveVideoFilter::SetBufferGroup(const media_source &for_source,
		BBufferGroup *newGroup)
{
	PRINTF("ImmersiveVideoFilter::SetBufferGroup\n");

	// verify that we didn't get bogus arguments before we proceed
	if (for_source != filterOutput.source) return B_MEDIA_BAD_SOURCE;

	// Are we being passed the buffer group we're already using?
	if (newGroup == bufferGroup) return B_OK;

	// Ahh, someone wants us to use a different buffer group.  At this point we delete
	// the one we are using and use the specified one instead.  If the specified group is
	// NULL, we need to recreate one ourselves, and use *that*.  Note that if we're
	// caching a BBuffer that we requested earlier, we have to Recycle() that buffer
	// *before* deleting the buffer group, otherwise we'll deadlock waiting for that
	// buffer to be recycled!
	delete bufferGroup;		// waits for all buffers to recycle
	if (newGroup != NULL)
	{
		// we were given a valid group; just use that one from now on
		bufferGroup = newGroup;
	}
	else
	{
		// we were passed a NULL group pointer; that means we construct
		// our own buffer group to use from now on
		return AllocateBufferGroup();
	}

	return B_OK;
}

status_t 
ImmersiveVideoFilter::GetLatency(bigtime_t* out_latency)
{
	PRINTF("ImmersiveVideoFilter::GetLatency\n");

	// report our *total* latency:  internal plus downstream plus scheduling
	*out_latency = EventLatency() + SchedulingLatency();
	return B_OK;
}



status_t 
ImmersiveVideoFilter::VideoClippingChanged(const media_source &for_source,
		int16 num_shorts, int16 *clip_data,
		const media_video_display_info &display, int32 *_deprecated_)
{
	PRINTF("ImmersiveVideoFilter::VideoClippingChanged()\n");
	//** we need to produce ... all not only cipped region....... because other Consumer maby need this stuff
	return B_ERROR;
}

void 
ImmersiveVideoFilter::LateNoticeReceived(const media_source& what, bigtime_t how_much, bigtime_t performance_time)
{
	PRINTF("ImmersiveVideoFilter::LateNoticeReceived\n");
	if (what != filterOutput.source)
	{
		PRINTF(("\tBad source.\n"));
		return;
	}

	if (filterInput.source == media_source::null)
	{
		PRINTF(("\t!!! No input to blame.\n"));
		return;
	}
	// pass the buck, since this node doesn't schedule buffer
	// production
	NotifyLateProducer(filterInput.source, how_much,performance_time);
}


void ImmersiveVideoFilter::EnableOutput(const media_source& what, bool enabled, int32* _deprecated_)
{
	PRINTF("ImmersiveVideoFilter::EnableOutput\n");
	if (what == filterOutput.source)
	{
		outputEnabled = enabled;
	}
}



status_t 
ImmersiveVideoFilter::SetPlayRate(int32 numer, int32 denom)
{
	PRINTF("ImmersiveVideoFilter::SetPlayRate()\n");
	//***weiterleiten an FiterInput
	return B_ERROR;
}



void 
ImmersiveVideoFilter::AdditionalBufferRequested(const media_source &source,
		media_buffer_id prev_buffer, bigtime_t prev_time,
		const media_seek_tag *prev_tag)
{
	PRINTF("ImmersiveVideoFilter::AdditionalBufferRequest()\n");
	RequestAdditionalBuffer(filterInput.source, OfflineTime());
}

void 
ImmersiveVideoFilter::LatencyChanged(const media_source &source,
		const media_destination &destination, bigtime_t new_latency,
		uint32 flags)
{
	PRINTF("ImmersiveVideoFilter::LatencyChanged()\n");
	if (source != filterOutput.source)
	{
		PRINTF(("\tBad source.\n"));
		return;
	}
	if (destination != filterOutput.destination)
	{
		PRINTF(("\tBad destination.\n"));
		return;
	}
	
	downstreamLatency = new_latency;
	SetEventLatency(downstreamLatency + internalLatency);
	
	if (filterInput.source != media_source::null)
	{
		// pass new latency upstream
		status_t err = SendLatencyChange(filterInput.source, filterInput.destination, EventLatency() + SchedulingLatency());
		if(err < B_OK)
			PRINTF("\t!!! SendLatencyChange(): %s\n", strerror(err));
	}

}
//--------------------------BBufferProducer-----------------------//


status_t 
ImmersiveVideoFilter::AcceptFormat(const media_destination& dest, media_format* format)
{
	PRINTF("ImmersiveVideoFilter::AcceptFormat()\n");
	status_t err=B_OK;
	// return an error if this isn't really our one input's destination
	if (dest != filterInput.destination) 
		err=B_MEDIA_BAD_DESTINATION;

	if (format->type == B_MEDIA_UNKNOWN_TYPE) 
		format->type = B_MEDIA_RAW_VIDEO;

	if (format->type != B_MEDIA_RAW_VIDEO) 
		err=B_MEDIA_BAD_FORMAT;
	
/*	if ((err==B_OK) &&
		(!format->u.raw_video.display.line_width<=filterInputFormat.u.raw_video.display.line_width) && 
	    (format->u.raw_video.display.line_count<=filterInputFormat.u.raw_video.display.line_count))
		err=B_OK;
	else
		err=B_MEDIA_BAD_FORMAT;*/

	if ((err==B_OK) &&
		(format->u.raw_video.display.format==B_GRAY1))
	{
		format->u.raw_video.display.format=B_GRAY8;
		err=B_MEDIA_BAD_FORMAT;
	}

	
//	*format=filterInputFormat;
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
	PRINTF("ImmersiveVideoFilter::Connected()\n");
	if (where != filterInput.destination) return B_MEDIA_BAD_DESTINATION;
	// record useful information about the connection, and return success
	filterInput.source = producer;
//	filterFormat = with_format;
	filterInput.format =with_format;
	*out_input = filterInput;
	BRect *rect = new BRect(0,0,filterInput.format.u.raw_video.display.line_width,filterInput.format.u.raw_video.display.line_count);
	((PluginManager *)pluginManager)->SetInputRect(rect);
	((PluginManager *)pluginManager)->SetOutputRect(rect);
	((PluginManager *)pluginManager)->RunCalc();
	return B_OK;
}

void 
ImmersiveVideoFilter::Disconnected(
	const media_source& producer,
	const media_destination& where)
{
	PRINTF("ImmersiveVideoFilter::Disconnected()\n");
	// wipe out our input record
//	::memset(&filterInput, 0, sizeof(filterInput));
	if (filterInput.source != producer)
	{
		PRINTF("\tsource mismatch: expected ID %ld, got %ld\n",	filterInput.source.id, producer.id);
		return;
	}
	if (where != filterInput.destination)
	{
		PRINTF("\tdestination mismatch: expected ID %ld, got %ld\n", filterInput.destination.id,where.id);
		return;
	}

	// mark disconnected
	filterInput.source = media_source::null;
	filterInput.format=filterInputFormat;
	
	// no output? clear format:
	if (filterOutput.destination == media_destination::null)
	{
		filterOutput.format = filterOutputFormat;
	}
	
	//filterInput.format = filterFormat;


}



status_t 
ImmersiveVideoFilter::GetNextInput(int32* cookie, media_input* in_input)
{
	if (!in_input)
		return B_BAD_VALUE;

	if ((*cookie) != 0)
		return B_BAD_INDEX;
//	filterInput.format=filterFormat;
	*in_input = filterInput;
	(*cookie)++;
	return B_OK;
	PRINTF("ImmersiveVideoFilter::GetNextInput(%ld,...)\n",*cookie);
/*	// we have a single hardcoded input that can accept any kind of media data
	if (0 == *cookie)
	{
//		filterInput.format.type = B_MEDIA_RAW_VIDEO;		// accept any format
		filterInput.format=filterFormat;
		*out_input = filterInput;
		*cookie = 1;
		return B_OK;
	}
	else return B_BAD_INDEX;*/
}

void 
ImmersiveVideoFilter::DisposeInputCookie(int32 /*cookie*/ cookie)
{
	PRINTF("ImmersiveVideoFilter::DisposeInputCookie()\n");
	// we don't use any kind of state or extra storage for iterating over our
	// inputs, so we don't have to do any special disposal of input cookies.
	cookie--;
	
}

void 
ImmersiveVideoFilter::BufferReceived(BBuffer* inBuffer)
{
//	PRINTF("ImmersiveVideoFilter::BufferReceived():\n");
	// check buffer destination
	if(inBuffer->Header()->destination != filterInput.destination.id) 
	{
		PRINTF("ImmersiveVideoFilter::BufferReceived():\n\tBad destination.\n");
		inBuffer->Recycle();
		return;
	}
	

	// check output
	if(filterOutput.destination == media_destination::null || !outputEnabled || RunState() == B_STOPPED) 
	{
		inBuffer->Recycle();
		return;
	}
	if (RunMode() == B_OFFLINE)
	{
		if((RunMode() != B_OFFLINE) && (inBuffer->Header()->time_source != TimeSource()->ID())) 
		{
			inBuffer->Recycle();
			PRINTF("* timesource mismatch\n");
		}

		SetOfflineTime(inBuffer->Header()->start_time);
	}

	// process and retransmit buffer
	size_t sz=  filterOutput.format.u.raw_video.display.line_width *filterOutput.format.u.raw_video.display.line_count;
	//4 mit entsprechenden size_of Farbe.. ersetzen..
	BBuffer *outBuffer = bufferGroup->RequestBuffer(sz*4,50000);
	if (outBuffer)
	{
		CalcInt32((int32 *)inBuffer->Data(), (int32 *)outBuffer->Data(),sz);
		media_header *out_header = outBuffer->Header();
		media_header *in_header = inBuffer->Header();
//				out_header->type = B_MEDIA_RAW_VIDEO;
		out_header->size_used = in_header->size_used;
		out_header->start_time = in_header->start_time; // A changer !! IMPORTANT
		out_header->file_pos = in_header->file_pos;
//				memcpy(&h->u.raw_video, &m_format.u.raw_video, sizeof(media_video_header));
	//	SendBuffer(outBuffer,filterOutput.destination);
	}
	status_t err = SendBuffer(outBuffer, filterOutput.destination);
	inBuffer->Recycle();
	if (err < B_OK)
	{
		PRINTF("\tSendBuffer() failed: %s\n", strerror(err));
		outBuffer->Recycle();
	}
	// sent!
}


/*void 
ImmersiveVideoFilter::ProducerDataStatus(const media_destination& for_whom, int32 status, bigtime_t at_performance_time)
{
	PRINTF("ImmersiveVideoFilter::ProducerDataStatus(%ld,%lld)\n",status,at_performance_time);
	status_t err=B_OK;
	if (for_whom != filterInput.destination)
	{
		PRINTF("\tbad destination\n");
	}
	
	if (filterOutput.destination != media_destination::null)
	{
		// pass status downstream
		PRINTF("\tSendDataStatus()\n");
		err = SendDataStatus(status, filterOutput.destination, at_performance_time);
		if (err < B_OK)
		{
			PRINTF("\tSendDataStatus(): %s\n", strerror(err));
		}
	}
	/*if (status==B_DATA_AVAILABLE){
		err=RequestAdditionalBuffer(filterInput.source,(bigtime_t )0,NULL) ;
		if (err<B_OK)
		{
			PRINTF("\tSendDataStatus(): %s\n", strerror(err));
		}
	}*/
	
//}

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
	PRINTF("ImmersiveVideoFilter::GetLatencyFor()\n");
	// make sure this is one of my valid inputs
	if (for_whom != filterInput.destination) return B_MEDIA_BAD_DESTINATION;

	// report internal latency + downstream latency here, NOT including scheduling latency.
	// we're a final consumer (no outputs), so we have no downstream latency.
	//** Where do i get the downstream Latency
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
	PRINTF("ImmersiveVideoFilter::FormatChanged()\n");
	//**Pass it throug...
	return B_ERROR;
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
	PRINTF("ImmersiveVideoFilter::SeekTagRequested()\n");
	//pass it through
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

	PRINTF("ImmersiveVideoFilter::NodeRegistered()\n");
	filterOutput.node = Node();
	filterOutput.source.port = ControlPort();
	filterOutput.source.id = 0;
	filterOutput.destination = media_destination::null;
	strcpy(filterOutput.name, "ImmersiveVideoFilter Output");	

	filterOutput.format.type = B_MEDIA_RAW_VIDEO;
	filterOutput.format.u.raw_video = media_raw_video_format::wildcard;


	filterInput.destination.port = ControlPort();
	filterInput.destination.id = 0;
	filterInput.source = media_source::null;
	filterInput.node = Node();

	filterInput.format.type = B_MEDIA_RAW_VIDEO;
	filterInput.format.u.raw_video = media_raw_video_format::wildcard;

	strcpy(filterInput.name, "ImmersiveVideoFilter Input");
	PRINTF("create Pluginmanager\n");
	pluginManager =(void *)new PluginManager(this);
	SetParameterWeb(((PluginManager *)pluginManager)->GetParameterWeb());
	/*BPath *path=new BPath();
	find_directory(B_COMMON_SETTINGS_DIRECTORY,path,false,NULL);
	path->Append("Immersive Video/Config.conf");
	lLoader=new LookUpTableLoader(new BFile(path->Path(),B_READ_ONLY));
	lTable=lLoader->GetLookUpTable();
	filterInputFormat.u.raw_video.display.line_width=lLoader->GetInputRect()->IntegerWidth();
	filterInputFormat.u.raw_video.display.line_count=lLoader->GetInputRect()->IntegerHeight();
	filterOutputFormat.u.raw_video.display.line_width=lLoader->GetOutputRect()->IntegerWidth();
	filterOutputFormat.u.raw_video.display.line_count=lLoader->GetOutputRect()->IntegerHeight();*/

	
	/* Start the BMediaEventLooper control loop running */
	Run();
}

status_t ImmersiveVideoFilter::GetParameterValue(int32 id, bigtime_t *last_change, void *value, size_t *size)
{
	
	return ((PluginManager *)pluginManager)->GetParameterValue(id,last_change,value,size);
}

void ImmersiveVideoFilter::SetParameterValue(int32 id, bigtime_t when, const void *value, size_t size)
{
	/*//wenn eine Lookuptable fertig calculiert wurde, dann übernehmen wir sie
	if (id=LOOK_UP_TABLE_CALCULATED)
	{
			lTable=pluginManager->GetLookUpTable();
	}
	//ansonsten die Berechnung starten
	else
	{*/
		((PluginManager *)pluginManager)->SetParameterValue(id,when,value,size);

}

status_t ImmersiveVideoFilter::StartControlPanel(BMessenger *out_messenger)
{
	return BControllable::StartControlPanel(out_messenger);
}

void 
ImmersiveVideoFilter::HandleEvent(const media_timed_event *event,
		bigtime_t lateness, bool realTimeEvent)
{
	PRINTF("ImmersiveVideoFilter::HandleEvent()\n");
/*	switch(event->type)
	{
		case BTimedEventQueue::B_START:
				PRINTF("ImmersiveVideoFilter::HandleEvent\t-\tB_START\n");
				HandleStart(event->event_time);
			break;
		case BTimedEventQueue::B_STOP:
				PRINTF("ImmersiveVideoFilter::HandleEvent\t-\tB_STOP\n");
				HandleStop();
				EventQueue()->FlushEvents(event->event_time, BTimedEventQueue::B_ALWAYS, true, BTimedEventQueue::B_HANDLE_BUFFER);
			break;
		case BTimedEventQueue::B_WARP:
				PRINTF("ImmersiveVideoFilter::HandleEvent\t-\tB_WARP\n");
				HandleTimeWarp(event->bigdata);
			break;
		case BTimedEventQueue::B_SEEK:
				PRINTF("ImmersiveVideoFilter::HandleEvent\t-\tB_SEEK\n");		
				HandleSeek(event->bigdata);
			break;
		case BTimedEventQueue::B_HANDLE_BUFFER:
				PRINTF("ImmersiveVideoFilter::HandleEvent\t-\tB_HANDLE_BUFFER\n");
				HandleBuffer(event,lateness,realTimeEvent);
			break;
		case BTimedEventQueue::B_DATA_STATUS:
				PRINTF("ImmersiveVideoFilter::HandleEvent\t-\tB_DATA_STATUS\n");
			break;
		case BTimedEventQueue::B_PARAMETER:
		default:
			PRINTF("ImmersiveVideoFilter::HandleEvent\t-\tUnhandeld Event\n");
			break;
	}
	BBuffer *inBuffer = const_cast<BBuffer*>((BBuffer*)event->pointer);
	inBuffer->Recycle();*/

}

void ImmersiveVideoFilter::CalcInt32(int32 *inbuffer, int32* outbuffer, uint32 size)
{
//	PRINTF("ImmersiveVideoFilter::CalcInt32()\n");
	uint32 i=0, j=0;
	BRect *outDim=((PluginManager *)pluginManager)->GetOutputRect();
	int xmax = outDim->IntegerWidth();
	uint32 r[24];
	uint32 g[24];
	uint32 b[24];
	int pos[24];
	uint32 rges, gges, bges;
	pos[0] = -2*xmax-2;
	pos[1] = -2*xmax-1;
	pos[2] = -2*xmax;
	pos[3] = -2*xmax+1;
	pos[4] = -2*xmax+2;
	pos[5] = -xmax-2;
	pos[6] = -xmax-1;
	pos[7] = -xmax;
	pos[8] = -xmax+1;
	pos[9] = -xmax+2;
	pos[10] = -2;
	pos[11] = -1;
	pos[12] = 0;
	pos[13] = 1;
	pos[14] = 2;
	pos[15] = xmax-2;
	pos[16] = xmax-1;
	pos[17] = xmax;
	pos[18] = xmax+1;
	pos[19] = xmax+2;
	pos[20] = 2*xmax-2;
	pos[21] = 2*xmax-1;
	pos[22] = 2*xmax;
	pos[23] = 2*xmax+1;
	pos[24] = 2*xmax+2;
	for (i=0; i<size; i++)
	{
		for(j=0; j<=24; j++)
		{
			r[j] = (inbuffer[i+pos[j]] & 0x00ff0000);
			g[j] = (inbuffer[i+pos[j]] & 0x0000ff00);
			b[j] = (inbuffer[i+pos[j]] & 0x000000ff);
		}
		rges = r[0] + r[4] + r[20] + r[24] + ((r[1]+r[3]+r[5]+r[9]+r[15]+r[19]+r[21]+r[23])*2) + ((r[2]+r[6]+r[8]+r[10]+r[14]+r[16]+r[18]+r[22])*3) + ((r[7]+r[11]+r[12]+r[13]+r[17])*4);
		gges = g[0] + g[4] + g[20] + g[24] + ((g[1]+g[3]+g[5]+g[9]+g[15]+g[19]+g[21]+g[23])*2) + ((g[2]+g[6]+g[8]+g[10]+g[14]+g[16]+g[18]+g[22])*3) + ((g[7]+g[11]+g[12]+g[13]+g[17])*4);
		bges = b[0] + b[4] + b[20] + b[24] + ((b[1]+b[3]+b[5]+b[9]+b[15]+b[19]+b[21]+b[23])*2) + ((b[2]+b[6]+b[8]+b[10]+b[14]+b[16]+b[18]+b[22])*3) + ((b[7]+b[11]+b[12]+b[13]+b[17])*4);
		outbuffer[i] = ((rges>>6)&0x00ff0000) + ((gges>>6)&0x0000ff00) + ((bges>>6)&0x000000ff);
	}
}

void ImmersiveVideoFilter::CalcIn16(int16 *inbuffer, int16* outbuffer, uint32 size)
{
//	PRINTF("ImmersiveVideoFilter::CalcInt16()\n");
	uint32 i=0;
	for (i=0; i<size;i++)
	{
		outbuffer[i]=inbuffer[lTable[i]];
	}
}

void ImmersiveVideoFilter::CalcInt8(int8 *inbuffer, int8* outbuffer, uint32 size)
{
//	PRINTF("ImmersiveVideoFilter::CalcInt8()\n");
	uint32 i=0;
	for (i=0; i<size;i++)
	{
		outbuffer[i]=inbuffer[lTable[i]];
	}
}

void ImmersiveVideoFilter::NoCalc(uint *inbuffer, uint* outbuffer, uint32 size)
{
//	PRINTF("ImmersiveVideoFilter::NoCalc()\n");
	uint32 i=0;
	for (i=0; i<size;i++)
	{
		outbuffer[i]=inbuffer[i];
	}
}


status_t	ImmersiveVideoFilter::HandleBuffer(const media_timed_event *event,bigtime_t lateness, bool realTimeEvent)
{
	PRINTF("ImmersiveVideoFilter::HandleBuffer()\n");
	if (RunState() == BMediaEventLooper::B_STARTED)
	{
		BBuffer *inBuffer = const_cast<BBuffer*>((BBuffer*)event->pointer);
		if(inBuffer) 
		{
			size_t sz= 4 * filterOutput.format.u.raw_video.display.line_width *filterOutput.format.u.raw_video.display.line_count;
			BBuffer *outBuffer = bufferGroup->RequestBuffer(sz,50000);
			if (outBuffer)
			{
				CalcInt32((int32 *)inBuffer->Data(), (int32 *)outBuffer->Data(),sz/4);
				media_header *out_header = outBuffer->Header();
				media_header *in_header = inBuffer->Header();
//				out_header->type = B_MEDIA_RAW_VIDEO;
				out_header->size_used = in_header->size_used;
				out_header->start_time = in_header->start_time; // A changer !! IMPORTANT
				out_header->file_pos = in_header->file_pos;
//				memcpy(&h->u.raw_video, &m_format.u.raw_video, sizeof(media_video_header));
				SendBuffer(outBuffer,filterOutput.destination);
			}
			inBuffer->Recycle();
	
		}
				//**Handle OfflineMode and Recodmode...
	}
	return B_ERROR;

}

void ImmersiveVideoFilter::LoadConfig()
{
	lTable=((PluginManager *)pluginManager)->GetLookUpTableInt();
	BRect *outDim=((PluginManager *)pluginManager)->GetOutputRect();
	filterOutputFormat.u.raw_video.display.line_width=outDim->IntegerWidth();
	filterOutputFormat.u.raw_video.display.line_count=outDim->IntegerHeight();
	//**nächsten Consumer benachrichtigen, dass die Daten sich geändert haben.

}



status_t
ImmersiveVideoFilter::AllocateBufferGroup()
{
	bufferGroup = new BBufferGroup(4 * filterOutput.format.u.raw_video.display.line_width *filterOutput.format.u.raw_video.display.line_count, 2);
	if (bufferGroup->InitCheck() < B_OK)
	{
		delete bufferGroup;
		bufferGroup = NULL;
		return B_NO_INIT;
	}
	return B_OK;
}

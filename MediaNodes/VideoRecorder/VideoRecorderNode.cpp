#include "VideoRecorderNode.h"
#include <string.h>


int32 VideoRecorderNode::instances = 0;


VideoRecorderNode::VideoRecorderNode(BMediaAddOn *mediaAddon, const char *name, int32 internal_id)
											:BMediaNode(name),
											BBufferConsumer(B_MEDIA_RAW_VIDEO),
											BMediaEventLooper()
{
	initStatus = B_NO_INIT;
	//count Instances, how many VideoRecorderNodes are in use
	if (atomic_add(&instances, 1) >= MAX_ALLOWED_INSTANCES)
		return;
	//unsere ID merken
	internalID=internal_id;
	//Addon merken
	addon=mediaAddon;

	
	// at the beginning the Output is enabled
	outputEnabled	= true;
	// at the beginning this node dont have any latency
	internalLatency	= 0;
	// the codec List to find fast the codec....
	codecList=new BList();
	formatList=new BList();
	
	// we are done.. everything is OK...
	initStatus		= B_OK;
}

VideoRecorderNode::~VideoRecorderNode()
{
	BMediaEventLooper::Quit();
	//delete all entrys in the List
	if (initStatus == B_OK)
	{
		if (filterInput.destination!=media_destination::null)
			Disconnected(filterInput.source, filterInput.destination);
		if (bitmap)
			delete bitmap;
		if (movie)
			delete movie;

	}
	atomic_add(&instances, -1);
}

BMediaAddOn *
VideoRecorderNode::AddOn(int32 *internal_id) const
{
	INFO("VideoRecorderNode::AddOn(%ld)\n",*internal_id);
	if (internal_id)
		*internal_id = internalID;
	return addon;
}


status_t 
VideoRecorderNode::HandleMessage(int32 message, const void *data, size_t size)
{
	INFO("VideoRecorderNode::HandleMessage(%ld,%ld,%ld)\n",message,(uint32)data,size);
	if(
		BBufferConsumer::HandleMessage(message, data, size) &&
		BMediaNode::HandleMessage(message, data, size))
		BMediaNode::HandleBadMessage(message, data, size);
		return B_OK;
}

/*void 
VideoRecorderNode::Preroll()
{
	INFO("VideoRecorderNode::Preroll\n");
	/****************************************************************************
	 *	This hook may be called before the node is started to give the hardware *
	 *	a chance to start. 														*
	 ****************************************************************************/
//}

status_t 
VideoRecorderNode::AcceptFormat(const media_destination& dest, media_format* format)
{
	status_t err=B_OK;
	INFO("VideoRecorderNode::AcceptFormat()\n");
	if (dest != filterInput.destination) 
		err=B_MEDIA_BAD_DESTINATION;

	if (format->type == B_MEDIA_UNKNOWN_TYPE) 
		format->type = B_MEDIA_RAW_VIDEO;

	if (format->type != B_MEDIA_RAW_VIDEO) 
		err=B_MEDIA_BAD_FORMAT;
	return err;
}


status_t 
VideoRecorderNode::Connected(
	const media_source& producer,
	const media_destination& dest,
	const media_format& with_format,
	media_input* out_input)
{
	INFO("VideoRecorderNode::Connected()\n");
	if (dest != filterInput.destination) return B_MEDIA_BAD_DESTINATION;
	filterInput.source = producer;
	filterInput.format =with_format;
	media_video_display_info videoInfo=with_format.u.raw_video.display;
	movie	= new BitmapMovie(videoInfo.line_width-1, videoInfo.line_count-1, videoInfo.format);
//	bitmap	= new BBitmap(BRect(0, 0, videoInfo.line_width, videoInfo.line_count), B_BITMAP_ACCEPTS_VIEWS|B_BITMAP_IS_AREA|B_BITMAP_IS_CONTIGUOUS, movie->ColorSpace());
//	DEBUG("bitma Breite: %lf",bitmap->Bounds().Width());
	if (movie)
	//&& (bitmap))
	{
	//	int32 changeTag;
		//**check ifB_OK
//		BBufferConsumer::SetOutputBuffersFor(producer,filterInput.destination,GetBitmapBufferGroup(),NULL,&changeTag);
		return B_OK;
	}
	else
		return B_ERROR;
}


void 
VideoRecorderNode::Disconnected(
	const media_source& producer,
	const media_destination& dest)
{
	INFO("VideoRecorderNode::Disconnected()\n");
	if (filterInput.source != producer)
	{
		ERROR("\tsource mismatch: expected ID %ld, got %ld\n",	filterInput.source.id, producer.id);
		return;
	}
	if (dest != filterInput.destination)
	{
		ERROR("\tdestination mismatch: expected ID %ld, got %ld\n", filterInput.destination.id,dest.id);
		return;
	}

}



status_t 
VideoRecorderNode::GetNextInput(int32* cookie, media_input* in_input)
{
	INFO("VideoRecorderNode::GetNextInput(%ld,...)\n",*cookie);
	if (!in_input)
		return B_BAD_VALUE;

	if ((*cookie) != 0)
		return B_BAD_INDEX;
//	filterInput.format=filterFormat;
	*in_input = filterInput;
	(*cookie)++;
	return B_OK;
}

void 
VideoRecorderNode::DisposeInputCookie(int32 /*cookie*/ cookie)
{
	INFO("VideoRecorderNode::DisposeInputCookie(%ld)\n",cookie);
	//keine Ahnung was Dispose im cookie liefert der liefert... mit dem cookie... eventuell die gesamtZahl
	//oder index
//	cookie--;
}

void 
VideoRecorderNode::BufferReceived(BBuffer* inBuffer)
{
	INFO("VideoRecorderNode::BufferReceived():\n");		
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


void VideoRecorderNode::ProducerDataStatus(const media_destination & dest, int32 status, bigtime_t at_performance_time)
{ 
	INFO("VideoRecorderNode::ProducerDataStatus()\n");
}

status_t 
VideoRecorderNode::GetLatencyFor(const media_destination& dest, bigtime_t* out_latency, media_node_id* out_timesource)
{
	INFO("VideoRecorderNode::GetLatencyFor()\n");
	return internalLatency;
}


status_t 
VideoRecorderNode::FormatChanged(
	const media_source& producer,
	const media_destination& dest,
	int32 change_tag,
	const media_format& format)
{
	INFO("status_t::FormatChanged()\n");
	return B_ERROR;
}

status_t 
VideoRecorderNode::SeekTagRequested(
	const media_destination& destination,
	bigtime_t in_target_time,
	uint32 in_flags,
	media_seek_tag* out_seek_tag,
	bigtime_t* out_tagged_time,
	uint32* out_flags)
{
	INFO("VideoRecorderNode::SeekTagRequested()\n");
	//pass it through
	return B_ERROR;
}


void 
VideoRecorderNode::NodeRegistered()
{
	INFO("VideoRecorderNode::NodeRegistered()\n");
	if (initStatus != B_OK) 
	{
		ReportError(B_NODE_IN_DISTRESS);
		return;
	}
	filterInput.destination.port = ControlPort();
	filterInput.destination.id = 0;
	filterInput.source = media_source::null;
	filterInput.node = Node();
	strcpy(filterInput.name, "VideoRecorderNode Input");	
	filterInput.format.type = B_MEDIA_RAW_VIDEO;
	filterInput.format.u.raw_video = media_raw_video_format::wildcard;
	//**replace with wildcard
	filterInput.format.u.raw_video.display.format=B_RGB32;
	SetParameterWeb(GetParameterWeb());
	Run();
	INFO("ERROR CODES\n");
/*	INFO("\tB_BAD_VALUE=%ld\n",B_BAD_VALUE);
	INFO("\tB_NO_INIT=%ld\n",B_NO_INIT);
	INFO("\tB_MISMATCHED_VALUES=%ld\n",B_MISMATCHED_VALUES);*/
//	INFO("\tB_NO_INIT=%ld\n",B_NO_INIT);	

}

void 
VideoRecorderNode::Start(bigtime_t performance_time)
{
	status_t err;
	BEntry *tester=new BEntry();;
	BMediaEventLooper::Start(performance_time);
	entry_ref ref;
	//preparation to a Algorihtm wich dosent overite existingVideoFiles...
	uint32 i=0;
	char *path = new char[B_PATH_NAME_LENGTH];
	do
	{
		sprintf(path,"/tmp/test%ld",i);
		::get_ref_for_path(path, &ref);
		tester->SetTo(&ref,true);
		i++;
	}
	while (tester->Exists());
	media_codec_info current_codec= *((media_codec_info *)(codecList->ItemAt(videoCodec)));
	INFO("VideoFileFormat: %s\n",videoFileFormat.pretty_name);
	INFO("Codec: %s\n",current_codec.pretty_name);
	INFO("Quality: %lf\n",videoQuality);
	tmpMovie=new BFile(tester,B_CREATE_FILE|B_WRITE_ONLY);
	//err=movie->CreateFile(ref, videoFileFormat, filterInput.format, current_codec,videoQuality);
/*	track=movie->GetTrack();
	track->GetParameterWeb(&web);
	SetParameterWeb(web);*/
	if (err!=B_OK) INFO("Fehler beim erstelln des Files %s\n",strerror(err));
/*	BMediaTrack* t = movie->GetTrack();
	t->SetQuality(videoQuality);*/
}


void 
VideoRecorderNode::HandleEvent(const media_timed_event *event,
		bigtime_t lateness, bool realTimeEvent)
{
	INFO("MediaSplitter::HandleEvent()\n");
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

status_t 
VideoRecorderNode::GetParameterValue(int32 id, bigtime_t *last_change,
									void *value, size_t *size)
{
	status_t err=B_BAD_VALUE;
	if (id == VIDEO_FORMAT)
	{
		*size = sizeof(media_format_family);
		*((uint32 *)value) = videoFormat;
		err=B_OK;
	}
	else if (id == VIDEO_CODEC)
	{
		*size = sizeof(int32);
		*((uint32 *)value) = videoCodec;
		err=B_OK;
	}
	else if (id == VIDEO_QUALITY)
	{
		*size = sizeof(float);
		*((float *)value) = videoQuality;
		err=B_OK;
	}
	return err;

}
void 
VideoRecorderNode::SetParameterValue(int32 id, bigtime_t when,
							const void *value, size_t size)
{
//	*last_change = fLastSelectionChange;
	if (id == VIDEO_FORMAT)
	{
		if (size == sizeof(uint32))
		{
			
			videoFormat=*((uint32 *)value);
			media_format_family	mffFamily=(media_format_family)videoFormat;

			INFO("%ld selected\n",videoFormat);
		//	media_format_family	mffFamily=*((media_format_family *)formatList->ItemAt(videoFormat));
			//videoFileFormat=*((media_file_format *)formatList->ItemAt(videoFormat));
		//	INFO("build codec with %s\n",videoFileFormat.pretty_name);
//			BuildCodecMenu(videoFileFormat.family);		
			BuildCodecMenu(mffFamily);		
			SetParameterWeb(web);
//			StartControlPanel(NULL);
		}
		
	}
	else if (id == VIDEO_CODEC)
	{
		if (size == sizeof(int32))
			videoCodec= *((uint32 *)value);
	}
	else if (id = VIDEO_QUALITY)
	{
		if (size == sizeof(float))
		INFO("QUALITY=%lf\n",*((float *)value));
			videoQuality=*((float *)value);
	}
}

status_t 
VideoRecorderNode::StartControlPanel(BMessenger *out_messenger)
{
	return BControllable::StartControlPanel(out_messenger);
}


BBufferGroup *VideoRecorderNode::GetBitmapBufferGroup(void)
{
	BBufferGroup *my_group = new BBufferGroup; 
	area_info bm_info; 
	if (bitmap != NULL)
	{
		if (get_area_info(bitmap->Area(), &bm_info) != B_OK)
			return NULL;
		buffer_clone_info bc_info; 
		bc_info.area = bm_info.area; 
		bc_info.offset = ((char *) bitmap->Bits())-((char *) bm_info.address); 
		bc_info.size = bitmap->BitsLength(); 
		BBuffer *out_buffer = NULL; 
		if(my_group->AddBuffer(bc_info, &out_buffer) != B_OK)
			return NULL;
		else
		{
			out_buffer->Recycle();
			return my_group;
		}
	}
	else
		return NULL;
}

BParameterWeb	*VideoRecorderNode::GetParameterWeb(void)
{
	int32 cookie = 0;
	bool first=true;
	media_file_format *mff=new media_file_format;
//	if (web) delete web;
	web = new BParameterWeb();
	BParameterGroup *videoFormat=web->MakeGroup("VideoFormat");
	BParameterGroup *formatGroup = videoFormat->MakeGroup("Fileformat");
	format = formatGroup->MakeDiscreteParameter(VIDEO_FORMAT, B_MEDIA_RAW_VIDEO, "VideoFileFormat", "FileFormat");
	BParameterGroup *codecGroup = videoFormat->MakeGroup("Codec");
	codec = codecGroup->MakeDiscreteParameter(VIDEO_CODEC, B_MEDIA_RAW_VIDEO, "Codec", "Codec");
	BParameterGroup *qualityGroup = videoFormat->MakeGroup("Quality");
	quality = qualityGroup->MakeContinuousParameter(VIDEO_QUALITY, B_MEDIA_RAW_VIDEO, "Video Quality", B_QUALITY,"",0,1.0,0.005);
	while (get_next_file_format(&cookie, mff) == B_OK)
	{
		if (mff->capabilities & media_file_format::B_KNOWS_ENCODED_VIDEO)
		{
//			format->AddItem(formatList->CountItems(), mff->pretty_name);
			format->AddItem((uint32)mff->family, mff->pretty_name);

			formatList->AddItem(&(mff->family));
			if (first)
			{
				videoFileFormat=*mff;
				BuildCodecMenu(videoFileFormat.family);
				first=false;
			}
		}
	}
	videoQuality=1.0;
	return web;	
}

void 
VideoRecorderNode::BuildCodecMenu(media_format_family mf_family)
{
	LockParameterWeb();
	INFO("VideoRecorderNode::BuildCodecMenu()");
	// remember the currently-selected codec
/*	media_codec_info current_codec= *((media_codec_info *)(codecList->ItemAt(videoCodec)));
	int32 current = current_codec.id;	// remember the current codec
	current_codec.id = -2;						// magic invalid value*/

	// find the full media_file_format corresponding to the given format family (e.g. AVI)
	status_t err;
	media_file_format mff;
	int32 cookie = 0;
	while ((err = get_next_file_format(&cookie, &mff)) == B_OK)
	{
		if (mff.family == mf_family) break;
	}

	// hmm, something is desperately wrong -- we couldn't find the "current" format family
	if (err)
	{
		fprintf(stderr, "ERROR:  couldn't find current media format family\n");
		//*** we need some other error handling..
//		exit(1);
	}

	codec->MakeEmpty();
	void *item;
	while ((item = codecList->RemoveItem(int32(0))) != NULL)
	{
		delete item;
	}
	codecList->MakeEmpty();

	// fill in the list of available codecs for this media format and file family
	media_codec_info *codec_info=new media_codec_info;
//	media_codec_info firstCodec;
	media_format outFormat;
	cookie = 0;
	while (get_next_encoder(&cookie, &mff, &filterInput.format, &outFormat, codec_info) == B_OK)
	{
		codec->AddItem(codecList->CountItems(),codec_info->pretty_name);
		codecList->AddItem(codec_info);
		codec_info=new media_codec_info;
		videoCodec=1;
	}
	UnlockParameterWeb();
	//** Select one???
//	SetParameterWeb(web);
}
void 
VideoRecorderNode::HandleBufferWrap( BBuffer *inBuffer, bigtime_t lateness )
{
}
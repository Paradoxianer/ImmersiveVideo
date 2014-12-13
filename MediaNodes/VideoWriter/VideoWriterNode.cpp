#include "VideoWriterNode.h"
#include <string.h>


int32 VideoWriterNode::instances = 0;


VideoWriterNode::VideoWriterNode(BMediaAddOn *mediaAddon, const char *name, int32 internal_id)
											:BMediaNode(name),
											BBufferConsumer(B_MEDIA_RAW_VIDEO),
											BMediaEventLooper()
{
	initStatus = B_NO_INIT;
	//count Instances, how many VideoWriterNodes are in use
	if (atomic_add(&instances, 1) >= MAX_ALLOWED_INSTANCES_VWR)
		return;
	//unsere ID merken
	internalID=internal_id;
	//Addon merken
	addon=mediaAddon;

	
	// at the beginning the Output is enabled
	outputEnabled	= true;
	// at the beginning this node dont have any latency
	internalLatency	= 2000000000;
	// the codec List to find fast the codec....
	codecList=new BList();
	formatList=new BList();
	
	// we are done.. everything is OK...
	initStatus		= B_OK;
}

VideoWriterNode::~VideoWriterNode()
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
VideoWriterNode::AddOn(int32 *internal_id) const
{
	INFO("VideoWriterNode","AddOn(%ld)\n",*internal_id);
	if (internal_id)
		*internal_id = internalID;
	return addon;
}


status_t 
VideoWriterNode::HandleMessage(int32 message, const void *data, size_t size)
{
	INFO("VideoWriterNode","HandleMessage(%ld,%ld,%ld)\n",message,(uint32)data,size);
	if(
		BMediaEventLooper::HandleMessage(message, data, size) &&
		BBufferConsumer::HandleMessage(message, data, size) &&
		BMediaNode::HandleMessage(message, data, size))
		BMediaNode::HandleBadMessage(message, data, size);
		return B_OK;
}

/*void 
VideoWriterNode::Preroll()
{
	INFO("VideoWriterNode::Preroll\n");
	/****************************************************************************
	 *	This hook may be called before the node is started to give the hardware *
	 *	a chance to start. 														*
	 ****************************************************************************/
//}

status_t 
VideoWriterNode::AcceptFormat(const media_destination& dest, media_format* format)
{
	status_t err=B_OK;
	INFO("VideoWriterNode","AcceptFormat()\n");
	if (dest != filterInput.destination) 
		err=B_MEDIA_BAD_DESTINATION;

	if (format->type == B_MEDIA_UNKNOWN_TYPE) 
		format->type = B_MEDIA_RAW_VIDEO;

	if (format->type != B_MEDIA_RAW_VIDEO) 
		err=B_MEDIA_BAD_FORMAT;
	DE_BUG("VideoWriterNode","format: width = %ld, height=%ld\n",format->u.raw_video.display.line_width, format->u.raw_video.display.line_count);

	return err;
}


status_t 
VideoWriterNode::Connected(
	const media_source& producer,
	const media_destination& dest,
	const media_format& with_format,
	media_input* out_input)
{
	INFO("VideoWriterNode","Connected()\n");
	if (dest != filterInput.destination) return B_MEDIA_BAD_DESTINATION;
	filterInput.source = producer;
	filterInput.format =with_format;
	media_video_display_info videoInfo=with_format.u.raw_video.display;
	DE_BUG("VideoWriterNode","with_format: width = %ld, height=%ld\n",with_format.u.raw_video.display.line_width, with_format.u.raw_video.display.line_count);
	DE_BUG("VideoWriterNode","filterInput: width = %ld, height=%ld\n",filterInput.format.u.raw_video.display.line_width, filterInput.format.u.raw_video.display.line_count);
//	bitmap	= new BBitmap(BRect(0, 0, videoInfo.line_width, videoInfo.line_count), B_BITMAP_ACCEPTS_VIEWS|B_BITMAP_IS_AREA|B_BITMAP_IS_CONTIGUOUS, movie->ColorSpace());
//	DEBUG("bitma Breite: %lf",bitmap->Bounds().Width());
/*	if (movie)
	//&& (bitmap))
	{
	//	int32 changeTag;
		//**check ifB_OK
//		BBufferConsumer::SetOutputBuffersFor(producer,filterInput.destination,GetBitmapBufferGroup(),NULL,&changeTag);
		return B_OK;
	}
	else
		return B_ERROR;*/
	return B_OK;
}


void 
VideoWriterNode::Disconnected(
	const media_source& producer,
	const media_destination& dest)
{
	INFO("VideoWriterNode","Disconnected()\n");
	if (filterInput.source != producer)
	{
		ERROR("VideoWriterNode","\tsource mismatch: expected ID %ld, got %ld\n",	filterInput.source.id, producer.id);
		return;
	}
	if (dest != filterInput.destination)
	{
		ERROR("VideoWriterNode","\tdestination mismatch: expected ID %ld, got %ld\n", filterInput.destination.id,dest.id);
		return;
	}

}



status_t 
VideoWriterNode::GetNextInput(int32* cookie, media_input* in_input)
{
	INFO("VideoWriterNode","GetNextInput(%ld,...)\n",*cookie);
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
VideoWriterNode::DisposeInputCookie(int32 /*cookie*/ cookie)
{
	INFO("VideoWriterNode","DisposeInputCookie(%ld)\n",cookie);
	//keine Ahnung was Dispose im cookie liefert der liefert... mit dem cookie... eventuell die gesamtZahl
	//oder index
//	cookie--;
}

void 
VideoWriterNode::BufferReceived(BBuffer* inBuffer)
{
	INFO("VideoWriterNode","BufferReceived(%ld)\n",inBuffer);
	status_t err;
	countFrames++;
//	INFO("VideoWriterNode::BufferReceived():\n");
	/*bitmap->LockBits();
	//err = movie->WriteFrame(bitmap, false);
	
	DEBUG("err=%s\n",strerror(err));
	bitmap->UnlockBits();*/
/*	if (RunState()==B_STARTED)
	{*/
		err=movie->WriteFrame(inBuffer->Data(),false);
		ERROR("Fehler beim Schreiben des VideoFrames %s\n",strerror(err));
		inBuffer->Recycle();
	/*}
	else
	{
		inBuffer->Recycle();
	}*/
}


void VideoWriterNode::ProducerDataStatus(const media_destination & dest, int32 status, bigtime_t at_performance_time)
{ 
	INFO("VideoWriterNode","ProducerDataStatus()\n");
}

status_t 
VideoWriterNode::GetLatencyFor(const media_destination& dest, bigtime_t* out_latency, media_node_id* out_timesource)
{
	INFO("VideoWriterNode","GetLatencyFor()\n");
	return internalLatency;
}


status_t 
VideoWriterNode::FormatChanged(
	const media_source& producer,
	const media_destination& dest,
	int32 change_tag,
	const media_format& format)
{
	INFO("VideoWriterNode","FormatChanged()\n");
	filterInput.format=format;
	DE_BUG("VideoWriterNode","filterInput: width = %ld, height=%ld\n",filterInput.format.u.raw_video.display.line_width, filterInput.format.u.raw_video.display.line_count);
	return B_OK;
}

status_t 
VideoWriterNode::SeekTagRequested(
	const media_destination& destination,
	bigtime_t in_target_time,
	uint32 in_flags,
	media_seek_tag* out_seek_tag,
	bigtime_t* out_tagged_time,
	uint32* out_flags)
{
	INFO("VideoWriterNode","SeekTagRequested()\n");
	//pass it through
	return B_ERROR;
}


void 
VideoWriterNode::NodeRegistered()
{
	INFO("VideoWriterNode","NodeRegistered()\n");
	if (initStatus != B_OK) 
	{
		ReportError(B_NODE_IN_DISTRESS);
		return;
	}
	filterInput.destination.port = ControlPort();
	filterInput.destination.id = 0;
	filterInput.source = media_source::null;
	filterInput.node = Node();
	strcpy(filterInput.name, "VideoWriterNode Input");	
	filterInput.format.type = B_MEDIA_RAW_VIDEO;
	filterInput.format.u.raw_video = media_raw_video_format::wildcard;
	//**replace with wildcard
	filterInput.format.u.raw_video.display.format=B_RGB32;
	SetParameterWeb(GetParameterWeb());
	Run();
/*	INFO("\tB_BAD_VALUE=%ld\n",B_BAD_VALUE);
	INFO("\tB_NO_INIT=%ld\n",B_NO_INIT);
	INFO("\tB_MISMATCHED_VALUES=%ld\n",B_MISMATCHED_VALUES);*/
//	INFO("\tB_NO_INIT=%ld\n",B_NO_INIT);	

}

void 
VideoWriterNode::Start(bigtime_t performance_time)
{
	status_t err;
	BEntry *tester=new BEntry();;
	BMediaEventLooper::Start(performance_time);
	entry_ref ref;
	DE_BUG("VideoWriterNode","filterInput: width = %ld, height=%ld\n",filterInput.format.u.raw_video.display.line_width, filterInput.format.u.raw_video.display.line_count);
	movie	= new BitmapMovie(filterInput.format.u.raw_video.display.line_width-1, filterInput.format.u.raw_video.display.line_count-1, filterInput.format.u.raw_video.display.format);
	//preparation to a Algorihtm wich dosent overite existingVideoFiles...
	uint32 i=0;
	char *path = new char[B_PATH_NAME_LENGTH];
	do
	{
		sprintf(path,"/boot/home/test%ld",i);
		::get_ref_for_path(path, &ref);
		tester->SetTo(&ref,true);
		i++;
	}
	while (tester->Exists());
	media_codec_info current_codec= *((media_codec_info *)(codecList->ItemAt(videoCodec)));
	DE_BUG("VideoWriterNode","VideoFileFormat: %s\n",videoFileFormat.pretty_name);
	DE_BUG("VideoWriterNode","Codec: %s\n",current_codec.pretty_name);
	DE_BUG("VideoWriterNode","Quality: %lf\n",videoQuality);
	
	err=movie->CreateFile(ref, videoFileFormat, filterInput.format, current_codec,videoQuality);
/*	track=movie->GetTrack();
	track->GetParameterWeb(&web);
	SetParameterWeb(web);*/
	if (err!=B_OK) INFO("Fehler beim erstelln des Files %s\n",strerror(err));
/*	BMediaTrack* t = movie->GetTrack();
	t->SetQuality(videoQuality);*/
	countFrames=0;
}

void 
VideoWriterNode::HandleEvent(
					const media_timed_event* event,
					bigtime_t lateness,
					bool realTimeEvent = false)
{
}
status_t 
VideoWriterNode::GetParameterValue(int32 id, bigtime_t *last_change,
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
VideoWriterNode::SetParameterValue(int32 id, bigtime_t when,
							const void *value, size_t size)
{
//	*last_change = fLastSelectionChange;
	if (id == VIDEO_FORMAT)
	{
		if (size == sizeof(uint32))
		{
			
			videoFormat=*((uint32 *)value);
			media_format_family	mffFamily=(media_format_family)videoFormat;

			DE_BUG("VideoWriterNode","%ld selected\n",videoFormat);
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
		DE_BUG("VideoWriterNode","QUALITY=%lf\n",*((float *)value));
			videoQuality=*((float *)value);
	}
}

status_t 
VideoWriterNode::StartControlPanel(BMessenger *out_messenger)
{
	return BControllable::StartControlPanel(out_messenger);
}


BBufferGroup *VideoWriterNode::GetBitmapBufferGroup(void)
{
	INFO("VideoWriterNode","GetBitmapBufferGroup()\n");
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

BParameterWeb	*VideoWriterNode::GetParameterWeb(void)
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
VideoWriterNode::BuildCodecMenu(media_format_family mf_family)
{
	LockParameterWeb();
	INFO("VideoWriterNode","BuildCodecMenu()");
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

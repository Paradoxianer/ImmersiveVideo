#ifndef VIDEO_RECORDER_NODE_H
#define VIDEO_RECORDER_NODE_H

#include <interface/Bitmap.h>
#include <media/BufferConsumer.h>
#include <media/Buffer.h>
#include <media/BufferGroup.h>
#include <media/MediaDefs.h>
#include <media/MediaEventLooper.h>
#include <media/Controllable.h>
#include <media/ParameterWeb.h>
#include <media/MediaNode.h>
#include <media/MediaFormats.h>
#include <media/MediaTrack.h>

#include <support/List.h>
#include <storage/Entry.h>

#include <stdio.h>
#include "BitmapMovie.h"


const int32 MAX_ALLOWED_INSTANCES			= 5;

#define INFO printf
#define DEBUG printf
#define ERROR printf

class VideoRecorderNode :	public BBufferConsumer,
						public BMediaEventLooper,
						public BControllable
						
{

public:
						VideoRecorderNode(BMediaAddOn *mediaAddon, const char *name, int32 internal_id);
						~VideoRecorderNode();

//+++++++++++++++++++++++++++BMediaNode+++++++++++++++++++++++//

	BMediaAddOn*		AddOn(int32 *outInternalID) const;
	status_t			HandleMessage(	int32 message, const void *data,
										size_t size);

//+++++++++++++++++++++++++++BBufferConsumer+++++++++++++++++++++++//
	status_t			AcceptFormat(
						const media_destination& dest,
						media_format* format);

	status_t			GetNextInput(
						int32* cookie,
						media_input* out_input);

	void				DisposeInputCookie( int32 cookie );
	
	void				BufferReceived( BBuffer* buffer );

	void				ProducerDataStatus(
						const media_destination& for_whom,
						int32 status,
						bigtime_t at_performance_time);

	status_t			GetLatencyFor(
						const media_destination& for_whom,
						bigtime_t* out_latency,
						media_node_id* out_timesource);

	status_t			Connected(
						const media_source& producer,	/* here's a good place to request buffer group usage */
						const media_destination& where,
						const media_format& with_format,
						media_input* out_input);

	void				Disconnected(
						const media_source& producer,
						const media_destination& where);

	/* The notification comes from the upstream producer, so he's already cool with */
	/* the format; you should not ask him about it in here. */
	status_t			FormatChanged(
						const media_source& producer,
						const media_destination& consumer, 
						int32 change_tag,
						const media_format& format);

	/* Given a performance time of some previous buffer, retrieve the remembered tag */
	/* of the closest (previous or exact) performance time. Set *out_flags to 0; the */
	/* idea being that flags can be added later, and the understood flags returned in */
	/* *out_flags. */
	status_t			SeekTagRequested(
						const media_destination& destination,
						bigtime_t in_target_time,
						uint32 in_flags, 
						media_seek_tag* out_seek_tag,
						bigtime_t* out_tagged_time,
						uint32* out_flags);
/*	status_t	 		SetBuffersFor(		//	new in 4.1
						const media_source & source,
						const media_destination & destination,
						BBufferGroup * group,
						void * user_data,
						int32 * change_tag, 		//	passed to RequestCompleted()
						bool will_reclaim = false,
						void * _reserved_ = 0){SetOutputBuffersFor(source,destination,group,user_data,change_tag,will_reclaim,_reserved_);};*/
//---------------------------BBufferConsumer-----------------------//


//+++++++++++++++++++++++++++BMediaEventLooper+++++++++++++++++++++++//

	void				NodeRegistered();

	void				Start(bigtime_t performance_time);
	
	void				Stop(bigtime_t performance_time, bool immediate)
						{
							if (BMediaEventLooper::RunState()==B_STARTED)	movie->CloseFile();
						};
/*	void				Seek(bigtime_t media_time, bigtime_t performance_time);
	void				TimeWarp(bigtime_t at_real_time,
							bigtime_t to_performance_time);


	void				SetRunMode(run_mode mode);*/

	void				HandleEvent(
						const media_timed_event* event,
						bigtime_t lateness,
						bool realTimeEvent = false);

//---------------------------BMediaEventLooper-----------------------//

public:
//Workaround to get acces to seteventlatency from outside...
	void				SetInternalLatency(bigtime_t latency){SetEventLatency(latency);};

//+++++++++++++++++++++++++++BControllable+++++++++++++++++++++++//

protected:
virtual status_t		GetParameterValue(	int32 id, bigtime_t *last_change,
											void *value, size_t *size);
virtual void			SetParameterValue(int32 id, bigtime_t when,
											const void *value, size_t size);
virtual status_t		StartControlPanel(BMessenger *out_messenger);

//---------------------------BControllable-----------------------//

//+++++++++++++++++++++++++++Own Methods+++++++++++++++++++++++//
		void			HandleBufferWrap( BBuffer *inBuffer, bigtime_t lateness );

//---------------------------Own Methods-----------------------//
//** own Methods
public:
	status_t			InitCheck() const { return initStatus; }
	BBufferGroup		*GetBitmapBufferGroup(void);
	BParameterWeb		*GetParameterWeb(void);
	void				BuildCodecMenu(media_format_family mf_family);
	static	int32		instances;

protected:
	BParameterWeb		*web;
	BMediaAddOn			*addon;
	int32				internalID;
	status_t			initStatus;
	media_format		filterInputFormat;
	media_input			filterInput;

	bool				outputEnabled;
	bool				running;
	bigtime_t			internalLatency;
	BBitmap				*bitmap;
	BitmapMovie			*movie;
	BFile				*tmpMovie;
	BMediaTrack			*track;
	enum				{VIDEO_FORMAT,VIDEO_CODEC,VIDEO_QUALITY};
	BDiscreteParameter	*format;
	BDiscreteParameter	*codec;
	BContinuousParameter *quality;

	BList				*codecList;
	BList				*formatList;

	uint32				videoFormat;
	media_file_format	videoFileFormat;	// currently selected file format family, e.g. B_AVI_FORMAT_FAMILY
//	media_codec_info	videoCodec;
	uint32				videoCodec;
	float				videoQuality;  
};
#endif

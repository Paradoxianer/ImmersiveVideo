#ifndef IMMERSIVE_VIDEO_FILTER_H
#define IMMERSIVE_VIDEO_FILTER_H

#include <kernel/OS.h>
#include <interface/Rect.h>
#include <media/BufferProducer.h>
#include <media/BufferConsumer.h>
#include <media/Controllable.h>
#include <media/MediaDefs.h>
#include <media/MediaEventLooper.h>
#include <media/MediaNode.h>

#include "Constants.h"

const int32 MAX_ALLOWED_INSTANCES			= 50;

class ImmersiveVideoFilter :	public BBufferProducer,
								public BBufferConsumer,
								public BMediaEventLooper,
								public BControllable
{
public:
				ImmersiveVideoFilter(BMediaAddOn *addon,	const char *name, int32 internal_id);
				~ImmersiveVideoFilter();

// BMediaNode methods
public:
	BMediaAddOn	*AddOn(int32 * internal_id) const;
	status_t	HandleMessage(int32 message, const void *data,
							size_t size);
protected:	
	void 		Preroll();
//		void		SetTimeSource(BTimeSource * time_source);


public:
//+++++++++++++++++++++++++++BBufferProducer+++++++++++++++++++++++//

			//** dort sollten wir wenn ein Puffer fertig "bearbeited" wurde den weiterschicken und danach den aufruf weiter "durchleiten" BBufferConuser->RequestAdditionalBuffer()**//
/*virtual		status_t	ChangeFormat(
				const media_source &source,
				const media_destination &destination,
				media_format *newFormat);*/

	status_t	FormatSuggestionRequested(
				media_type type,
				int32 quality,
				media_format* format);

	status_t	FormatProposal(
				const media_source& output,
				media_format* format);

	status_t	FormatChangeRequested(
				const media_source& source,
				const media_destination& destination,
				media_format* io_format,
				int32* _deprecated_);

	status_t	GetNextOutput(
				int32* cookie,
				media_output* out_output);

	status_t	DisposeOutputCookie( int32 cookie);

	status_t	SetBufferGroup(
				const media_source& for_source,
				BBufferGroup* group);

	status_t	VideoClippingChanged(const media_source &for_source,
				int16 num_shorts, int16 *clip_data,
				const media_video_display_info &display,
				int32 * _deprecated_);

	status_t	InitCheck() const { return initStatus; }

	/* Iterates over all outputs and maxes the latency found */
	status_t	GetLatency( bigtime_t* out_latency);

	status_t	PrepareToConnect(
				const media_source& what,
				const media_destination& where,
				media_format* format,
				media_source* out_source,
				char* out_name);

	void		Connect(
				status_t error, 
				const media_source& source,
				const media_destination& destination,
				const media_format& format,
				char* io_name);

	void		Disconnect(
				const media_source& what,
				const media_destination& where);

	void		LateNoticeReceived(
				const media_source& what,
				bigtime_t how_much,
				bigtime_t performance_time);

	void		EnableOutput(
				const media_source & what,
				bool enabled,
				int32* _deprecated_);

	status_t	SetPlayRate(
				int32 numer,
				int32 denom);


	void		AdditionalBufferRequested(
				const media_source& source,
				media_buffer_id prev_buffer,
				bigtime_t prev_time,
				const media_seek_tag* prev_tag);	//	may be NULL

	void		LatencyChanged(
				const media_source& source,
				const media_destination& destination,
				bigtime_t new_latency,
				uint32 flags);

			
//---------------------------BBufferProducer-----------------------//



//+++++++++++++++++++++++++++BBufferConsumer+++++++++++++++++++++++//


	status_t	AcceptFormat(
				const media_destination& dest,
				media_format* format);

	status_t	GetNextInput(
				int32* cookie,
				media_input* out_input);

	void		DisposeInputCookie( int32 cookie );
	
	void		BufferReceived( BBuffer* buffer );

	void		ProducerDataStatus(
				const media_destination& for_whom,
				int32 status,
				bigtime_t at_performance_time);

	status_t	GetLatencyFor(
				const media_destination& for_whom,
				bigtime_t* out_latency,
				media_node_id* out_timesource);

	status_t	Connected(
				const media_source& producer,	/* here's a good place to request buffer group usage */
				const media_destination& where,
				const media_format& with_format,
				media_input* out_input);

	void		Disconnected(
				const media_source& producer,
				const media_destination& where);

	/* The notification comes from the upstream producer, so he's already cool with */
	/* the format; you should not ask him about it in here. */
	status_t	FormatChanged(
				const media_source& producer,
				const media_destination& consumer, 
				int32 change_tag,
				const media_format& format);

	/* Given a performance time of some previous buffer, retrieve the remembered tag */
	/* of the closest (previous or exact) performance time. Set *out_flags to 0; the */
	/* idea being that flags can be added later, and the understood flags returned in */
	/* *out_flags. */
	status_t	SeekTagRequested(
				const media_destination& destination,
				bigtime_t in_target_time,
				uint32 in_flags, 
				media_seek_tag* out_seek_tag,
				bigtime_t* out_tagged_time,
				uint32* out_flags);
//---------------------------BBufferConsumer-----------------------//


//+++++++++++++++++++++++++++BMediaEventLooper+++++++++++++++++++++++//

	void		NodeRegistered();

/*	void		Start(bigtime_t performance_time);
	
	void		Stop(bigtime_t performance_time, bool immediate);
	void		Seek(bigtime_t media_time, bigtime_t performance_time);
	void		TimeWarp(bigtime_t at_real_time,
							bigtime_t to_performance_time);


	void		SetRunMode(run_mode mode);*/

	void		HandleEvent(
				const media_timed_event* event,
				bigtime_t lateness,
				bool realTimeEvent = false);


//---------------------------BMediaEventLooper-----------------------//


//+++++++++++++++++++++++++++BMediaEventLooper+++++++++++++++++++++++//

protected:
virtual status_t	GetParameterValue(int32 id, bigtime_t *last_change,
							void *value, size_t *size);
virtual void		SetParameterValue(int32 id, bigtime_t when,
							const void *value, size_t size);
virtual status_t	StartControlPanel(BMessenger *out_messenger);

//---------------------------BMediaEventLooper-----------------------//


	inline	void		CalcInt32(int32 *inputData,int32* outputData, uint32 size);
	inline	void		CalcIn16(int16 *inbuffer, int16* outbuffer, uint32 size);
	inline	void		CalcInt8(int8 *inbuffer, int8* outbuffer, uint32 size);
	inline	void		NoCalc(uint *inputData,uint* outputData,uint32 size);
			
			status_t	HandleBuffer(const media_timed_event *event,
		bigtime_t lateness, bool realTimeEvent);
public:
			void		LoadConfig();
	

private:
//+++++++++++++++++++++++++++ToolFunktios+++++++++++++++++++++++//
			status_t	AllocateBufferGroup();
//---------------------------ToolFunktios-----------------------//


//+++++++++++++++++++++++++++++allgemein+++++++++++++++++++++++++++//
			BMediaAddOn				*fAddOn;
	static	int32					instances;
			status_t				initStatus;
			int32					internalID;
			bigtime_t				internalLatency;
			bigtime_t				downstreamLatency;
			void					*pluginManager;
			uint32					*lTable;


//-----------------------------allgemein---------------------------//


//+++++++++++++++++++++++++++BBufferProducer+++++++++++++++++++++++//
			BBufferGroup*			bufferGroup;
			media_output			filterOutput;
			bigtime_t				filterLatency;
			bool 					outputEnabled;
			media_format			filterInputFormat;

			//**brauche wir das wirklich??
			bigtime_t				fPerformanceTimeBase;
//--------------------------BBufferProducer-----------------------//

//+++++++++++++++++++++++++++BBufferConsumer+++++++++++++++++++++++//

			media_input				filterInput;
			media_format			filterOutputFormat;

//--------------------------BBufferConsumer-----------------------//


};

#endif

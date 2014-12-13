#ifndef VIDEO_COPY_FILTER_H
#define VIDEO_COPY_FILTER_H

#include <kernel/OS.h>
#include <interface/Rect.h>
#include <media/BufferProducer.h>
#include <media/BufferConsumer.h>
#include <media/Controllable.h>
#include <media/MediaDefs.h>
#include <media/MediaEventLooper.h>
#include <media/MediaNode.h>
#include <media/MediaDefs15.h>
#include <media/MediaEffect.h>
//#include "MediaDefs15.h"
//#include "MediaEffect.h"


const int32 MAX_ALLOWED_INSTANCES			= 50;

const bigtime_t slowerLatencyTolerance		= 100;
const bigtime_t fasterLatencyTolerance		= 100;

#define INFO //printf
#define DEBUG printf
#define ERROR printf

class MotionHighlightEffekt :public BMediaEffect
{
public:
					MotionHighlightEffekt();

// BMediaNode methods


virtual status_t	GetParameterValue(int32 id, bigtime_t *last_change,
							void *value, size_t *size);
virtual void		SetParameterValue(int32 id, bigtime_t when,
							const void *value, size_t size);

virtual status_t		GetParameterWeb( BParameterWeb** );

virtual status_t		PrepareToRun( void );
virtual status_t		HandleBuffer( BBuffer* _inBuffer, BBuffer* _outBuffer );			

protected:

			uint32					*helpbuffer;
			media_format			*mediaFormat;
			int32					sensitivity;
			uint64					size;

};

#endif

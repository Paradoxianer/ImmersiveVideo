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
#include "MediaDefs15.h"
#include "MotionHighlightEffekt.h"



MotionHighlightEffekt::MotionHighlightEffekt():	BMediaEffect("Video: MotionHighlightEffekt","highlights areas were changes are detected","Copyright 2005 by rundumvideo",MEDIA_KIT_15::B_BUFFER_VIDEO_EFFECT)
{
	mediaFormat=NULL;
	sensitivity=20;
}

status_t	MotionHighlightEffekt::PrepareToRun( void )
{
	INFO("MotionHighlightEffekt::PrepareToRun()\n");
	mediaFormat=&BMediaEffect::GetFormat();
		DEBUG("widht=%l height=%l\n",mediaFormat->u.raw_video.display.line_width,mediaFormat->u.raw_video.display.line_count);
		helpbuffer = new uint32[mediaFormat->u.raw_video.display.line_width *mediaFormat->u.raw_video.display.line_count];
		size=mediaFormat->u.raw_video.display.line_width *mediaFormat->u.raw_video.display.line_count;
		DEBUG("size=%l\n",size);

	return B_OK;
}



status_t MotionHighlightEffekt::HandleBuffer(BBuffer* _inBuffer, BBuffer* _outBuffer )
{
	INFO("MotionHighlightEffekt::HandleBuffer()\n");
	uint32 *inbuffer=(uint32 *)_inBuffer->Data();
	uint32 *outbuffer=(uint32 *)_outBuffer->Data();
	uint64 i=0;
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
	
	return B_OK;
}

status_t MotionHighlightEffekt::GetParameterValue(int32 id, bigtime_t *last_change, void *value, size_t *size)
{
	float tmp=(float)sensitivity;
	value=(void *)&tmp;
	*size=sizeof(tmp);
	return B_OK;
}

void MotionHighlightEffekt::SetParameterValue(int32 id, bigtime_t when, const void *value, size_t size)
{
	sensitivity=(int32)*((float *)value);
}
status_t MotionHighlightEffekt::GetParameterWeb( BParameterWeb** returnWeb)
{
	BParameterWeb *web = new BParameterWeb();
	BParameterGroup *moverGroup = web->MakeGroup("Sensitivity");
	BContinuousParameter *move = moverGroup->MakeContinuousParameter(0, B_MEDIA_RAW_VIDEO, "sensitivity", B_GAIN,"",1,720.0,1);
	returnWeb=&web;
	return B_OK;
}

BMediaEffect* make_media_effect()
{
	return new MotionHighlightEffekt();
}

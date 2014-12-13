/*
  -----------------------------------------------------------------------------

	File: MediaDefs15.h
	
	Date: Thursday November 03, 2005

	Description: node_kind addition for effects.
	

	Copyright 2005, yellowTAB GmbH, All rights reserved.
	

  -----------------------------------------------------------------------------
*/

#ifndef _MEDIA_DEFS15_H
#define _MEDIA_DEFS15_H

#include <media/MediaDefs.h>

namespace MEDIA_KIT_15
{

enum node_kind {
	B_BUFFER_PRODUCER = 0x1,
	B_BUFFER_CONSUMER = 0x2,
	B_BUFFER_AUDIO_EFFECT = 0x100 | B_BUFFER_PRODUCER | B_BUFFER_CONSUMER,
	B_BUFFER_VIDEO_EFFECT = 0x200 | B_BUFFER_PRODUCER | B_BUFFER_CONSUMER,
	B_TIME_SOURCE = 0x4,
	B_CONTROLLABLE = 0x8,
	B_FILE_INTERFACE = 0x10,
	B_ENTITY_INTERFACE = 0x20,
	/* Set these flags for nodes that are suitable as default entities */
	B_PHYSICAL_INPUT = 0x10000,
	B_PHYSICAL_OUTPUT = 0x20000,
	B_SYSTEM_MIXER = 0x40000
};

}
#endif
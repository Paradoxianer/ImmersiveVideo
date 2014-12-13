#include <support/Autolock.h>
#include <media/MediaFormats.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "VideoCopyFilterAddon.h"
#include "VideoCopyFilter.h"

MediaAddOn::MediaAddOn(image_id imid)
	: BMediaAddOn(imid)
{
	/* Customize these parameters to match those of your node */
	fFlavorInfo.name = "VideoCopyFilter";
	fFlavorInfo.info = "A Filter to copy and manipulate video data";
	fFlavorInfo.kinds = B_BUFFER_PRODUCER | B_BUFFER_CONSUMER | B_CONTROLLABLE;
	fFlavorInfo.flavor_flags = B_FLAVOR_IS_LOCAL;
	fFlavorInfo.internal_id = 220679;
	fFlavorInfo.possible_count = MAX_ALLOWED_INSTANCES;
	fFlavorInfo.in_format_count = 1;
	fFlavorInfo.in_format_flags = 0;

	fMediaInFormat.type = B_MEDIA_RAW_VIDEO;
	fMediaInFormat.u.raw_video = media_raw_video_format::wildcard;
	fMediaInFormat.u.raw_video.interlace = 1;
	fMediaInFormat.u.raw_video.display.format = B_RGB32;
	fFlavorInfo.in_formats = &fMediaInFormat;

	fFlavorInfo.out_format_count = 1;
	fFlavorInfo.out_format_flags = 0;

	fMediaOutFormat.type = B_MEDIA_RAW_VIDEO;
	fMediaOutFormat.u.raw_video = media_raw_video_format::wildcard;
	fMediaOutFormat.u.raw_video.interlace = 1;
	fMediaOutFormat.u.raw_video.display.format = B_RGB32;

	fFlavorInfo.out_formats = &fMediaOutFormat;

	fInitStatus = B_OK;
}

MediaAddOn::~MediaAddOn()
{
}


status_t 
MediaAddOn::InitCheck(const char **out_failure_text)
{
	if (fInitStatus < B_OK) {
		*out_failure_text = "Unknown error";
		return fInitStatus;
	}

	return B_OK;
}

int32 
MediaAddOn::CountFlavors()
{
	if (fInitStatus < B_OK)
		return fInitStatus;

	/* This addon only supports a single flavor, as defined in the
	 * constructor */
	return 1;
}

/*
 * The pointer to the flavor received only needs to be valid between 
 * successive calls to BMediaAddOn::GetFlavorAt().
 */
status_t 
MediaAddOn::GetFlavorAt(int32 n, const flavor_info **out_info)
{
	if (fInitStatus < B_OK)
		return fInitStatus;

	if (n != 0)
		return B_BAD_INDEX;

	/* Return the flavor defined in the constructor */
	*out_info = &fFlavorInfo;
	return B_OK;
}

BMediaNode *
MediaAddOn::InstantiateNodeFor(
		const flavor_info *info, BMessage *config, status_t *out_error)
{
	VideoCopyFilter *node;

	if (fInitStatus < B_OK)
		return NULL;

	if (info->internal_id != fFlavorInfo.internal_id)
		return NULL;

	node = new VideoCopyFilter(this, fFlavorInfo.name, fFlavorInfo.internal_id);
	if (node && (node->InitCheck() < B_OK)) {
		delete node;
		node = NULL;
	}

	return node;
}

BMediaAddOn *
make_media_addon(image_id imid)
{
	return new MediaAddOn(imid);
}

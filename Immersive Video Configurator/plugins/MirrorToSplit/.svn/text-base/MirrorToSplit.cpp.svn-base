//****************************************
// Mirror.cpp
//****************************************

#include "ImmersiveVideoPlugin.h"
#include "MirrorToSplitPlugin.h"

#include <image.h>

//****************************************

extern "C" _EXPORT ImmersiveVideoPlugin* NewImmersiveVideoPlugin(image_id);

//****************************************

ImmersiveVideoPlugin* NewImmersiveVideoPlugin( image_id id )
{
	MirrorToSplitPlugin *mirror=new MirrorToSplitPlugin( id );
  	return mirror;
}

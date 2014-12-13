//****************************************
// Mirror.cpp
//****************************************

#include "ImmersiveVideoPlugin.h"
#include "MirrorPlugin.h"

#include <image.h>

//****************************************

extern "C" _EXPORT ImmersiveVideoPlugin* NewImmersiveVideoPlugin(image_id);

//****************************************

ImmersiveVideoPlugin* NewImmersiveVideoPlugin( image_id id )
{
	MirrorPlugin *barrel=new MirrorPlugin( id );
  	return barrel;
}

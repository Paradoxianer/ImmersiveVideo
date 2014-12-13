//****************************************
// Fisheye.cpp
//****************************************

#include "ImmersiveVideoPlugin.h"
#include "FisheyePlugin.h"

#include <image.h>

//****************************************

extern "C" _EXPORT ImmersiveVideoPlugin* NewImmersiveVideoPlugin(image_id);

//****************************************

ImmersiveVideoPlugin* NewImmersiveVideoPlugin( image_id id )
{
	FisheyePlugin *barrel=new FisheyePlugin( id );
  	return barrel;
}

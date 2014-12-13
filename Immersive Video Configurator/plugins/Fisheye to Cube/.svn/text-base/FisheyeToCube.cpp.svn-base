//****************************************
// FisheyeToCube.cpp
//****************************************

#include "ImmersiveVideoPlugin.h"
#include "FisheyeToCubePlugin.h"

#include <image.h>

//****************************************

extern "C" _EXPORT ImmersiveVideoPlugin* NewImmersiveVideoPlugin(image_id);

//****************************************

ImmersiveVideoPlugin* NewImmersiveVideoPlugin( image_id id )
{
	FisheyeToCubePlugin *barrel=new FisheyeToCubePlugin( id );
  	return barrel;
}

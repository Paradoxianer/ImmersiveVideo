//****************************************
// Fisheye.cpp
//****************************************

#include "ImmersiveVideoPlugin.h"
#include "SpiegelPlugin.h"

#include <image.h>

//****************************************

extern "C" _EXPORT ImmersiveVideoPlugin* NewImmersiveVideoPlugin(image_id);

//****************************************

ImmersiveVideoPlugin* NewImmersiveVideoPlugin( image_id id )
{
	SpiegelPlugin *fish;
	fish=new SpiegelPlugin(id);
  	return fish;
}

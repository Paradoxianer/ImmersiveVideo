//****************************************
// Fisheye.cpp
//****************************************

#include "ImmersiveVideoPlugin.h"
#include "3DpurPlugin.h"

#include <image.h>

//****************************************

extern "C" _EXPORT ImmersiveVideoPlugin* NewImmersiveVideoPlugin(image_id);

//****************************************

ImmersiveVideoPlugin* NewImmersiveVideoPlugin( image_id id )
{
	DDDPlugin *fish;
	fish=new DDDPlugin(id);
  	return fish;
}

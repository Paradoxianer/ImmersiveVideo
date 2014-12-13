//****************************************
// BarrelCorrection.cpp
//****************************************

#include "ImmersiveVideoPlugin.h"
#include "BarrelCorrectionPlugin.h"

#include <image.h>

//****************************************

extern "C" _EXPORT ImmersiveVideoPlugin* NewImmersiveVideoPlugin(image_id);

//****************************************

ImmersiveVideoPlugin* NewImmersiveVideoPlugin( image_id id )
{
	BarrelCorrectionPlugin *barrel=new BarrelCorrectionPlugin( id );
  	return barrel;
}

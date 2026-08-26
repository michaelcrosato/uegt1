#include "World/UEGT1RegionSettings.h"

const UUEGT1RegionSettings& UUEGT1RegionSettings::Get()
{
	return *GetDefault<UUEGT1RegionSettings>();
}

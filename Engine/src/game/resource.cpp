#include "game/resource.h"

Resource::Resource()
{
	key_.clear();
}

Resource::~Resource()
{
}

bool Resource::Release()
{
	return false;
}

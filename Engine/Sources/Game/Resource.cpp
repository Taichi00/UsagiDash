#include "Resource.h"

Resource::Resource()
{
	m_key.clear();
}

Resource::~Resource()
{
}

bool Resource::Release()
{
	return false;
}

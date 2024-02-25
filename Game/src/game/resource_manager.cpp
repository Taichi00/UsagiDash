#include "game/resource_manager.h"
#include "game/resource/resource.h"
#include "game/resource/model.h"
#include "game/resource/texture2d.h"

ResourceManager::ResourceManager()
{
	resource_map_.clear();
	resource_name_map_.clear();
}

ResourceManager::~ResourceManager()
{
}

const Resource* ResourceManager::GetResourceFromName(const std::wstring& name)
{
	if (!resource_name_map_.contains(name))
	{
		return nullptr;
	}

	return resource_name_map_[name];
}

void ResourceManager::RegisterResourceName(const Resource* resource, const std::wstring& name)
{
	if (!resource)
	{
		return;
	}

	if (!resource->Name().empty())
	{
		resource_name_map_.erase(resource->Name());
	}

	resource_name_map_[name] = resource;
}

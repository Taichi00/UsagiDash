#include "game/resource_manager.h"
#include "game/resource/resource.h"
#include "game/resource/model.h"
#include "game/resource/texture2d.h"

ResourceManager::ResourceManager()
{
	resource_map_.clear();
}

ResourceManager::~ResourceManager()
{
}

bool ResourceManager::Release(const std::string& key)
{
	if (resource_map_.find(key) == resource_map_.end())	// Œ©‚Â‚©‚ç‚È‚©‚Á‚½ê‡
	{
		return false;
	}

	resource_map_[key]->Release();
	resource_map_.erase(key);

	return true;
}

void ResourceManager::AddResource(std::shared_ptr<Resource> resource)
{

}

std::string ResourceManager::GetExtension(const std::string& path)
{
	int idx = path.find_last_of(".");
	return path.substr(idx, path.size() - idx);
}



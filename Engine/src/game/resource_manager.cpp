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

void ResourceManager::AddResource(std::shared_ptr<Resource> resource)
{

}

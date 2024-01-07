#include "ResourceManager.h"
#include "Resource.h"
#include "Model.h"
#include "Texture2D.h"

ResourceManager::ResourceManager()
{
	m_resourceMap.clear();
}

ResourceManager::~ResourceManager()
{
}

bool ResourceManager::Release(const std::string& key)
{
	if (m_resourceMap.find(key) == m_resourceMap.end())	// Œ©‚Â‚©‚ç‚È‚©‚Á‚½ê‡
	{
		return false;
	}

	m_resourceMap[key]->Release();
	m_resourceMap.erase(key);

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



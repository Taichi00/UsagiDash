#include "gbuffer_manager.h"
#include "engine/gbuffer.h"

GBufferManager::GBufferManager()
{
	gbuffer_map_.clear();
}

GBufferManager::~GBufferManager()
{
}

void GBufferManager::CreateGBuffer(std::string name, GBufferProperty prop)
{
	gbuffer_map_[name] = std::make_shared<GBuffer>(name, prop);
}

std::shared_ptr<GBuffer> GBufferManager::Get(std::string name)
{
	return gbuffer_map_[name];
}

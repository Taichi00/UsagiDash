#include "GBufferManager.h"
#include "GBuffer.h"

GBufferManager::GBufferManager()
{
	m_pGBuffers.clear();
	m_pGBufferMap.clear();
}

GBufferManager::~GBufferManager()
{
	for (auto gbuffer : m_pGBuffers)
	{
		delete gbuffer;
	}
}

void GBufferManager::CreateGBuffer(std::string name, GBufferProperty prop)
{
	auto gbuffer = new GBuffer(name, prop);

	m_pGBuffers.push_back(gbuffer);
	m_pGBufferMap[gbuffer->GetName()] = gbuffer;
}

GBuffer* GBufferManager::Get(std::string name)
{
	return m_pGBufferMap[name];
}

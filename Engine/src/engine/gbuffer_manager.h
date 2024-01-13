#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>

class GBuffer;
struct GBufferProperty;

class GBufferManager
{
public:
	GBufferManager();
	~GBufferManager();

	//GBuffer* operator[](std::string name) { return m_pGBufferMap[name]; };

	void CreateGBuffer(std::string name, GBufferProperty prop);
	std::shared_ptr<GBuffer> Get(std::string name);

private:
	std::map<std::string, std::shared_ptr<GBuffer>> gbuffer_map_;
};
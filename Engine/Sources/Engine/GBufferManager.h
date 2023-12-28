#pragma once

#include <vector>
#include <map>
#include <string>

class GBuffer;
struct GBufferProperty;

class GBufferManager
{
public:
	GBufferManager();
	~GBufferManager();

	//GBuffer* operator[](std::string name) { return m_pGBufferMap[name]; };

	void CreateGBuffer(std::string name, GBufferProperty prop);
	GBuffer* Get(std::string name);

private:
	std::vector<GBuffer*> m_pGBuffers;
	std::map<std::string, GBuffer*> m_pGBufferMap;
};
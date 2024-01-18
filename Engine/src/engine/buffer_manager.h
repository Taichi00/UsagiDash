#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include "engine/buffer.h"
#include "math/color.h"

class DescriptorHeap;

class BufferManager
{
public:
	BufferManager();
	~BufferManager();

	//GBuffer* operator[](std::string name) { return m_pGBufferMap[name]; };

	void CreateGBuffer(
		const std::string& name, 
		const DXGI_FORMAT& format, const Color& clear_color);

	void CreateGBuffer(
		const std::string& name,
		const DXGI_FORMAT& format, const Color& clear_color,
		const unsigned int width, const unsigned int height);

	void CreateDepthStencilBuffer(const std::string& name,
		const unsigned int width, const unsigned int height);

	std::shared_ptr<Buffer> Get(const std::string& name);

	void CreateAll();
	void ResetAll();

private:
	std::map<std::string, std::shared_ptr<Buffer>> gbuffer_map_;
};
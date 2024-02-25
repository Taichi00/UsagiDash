#include "engine/buffer_manager.h"
#include "engine/buffer.h"
#include "engine/engine.h"
#include "game/game.h"

BufferManager::BufferManager()
{
	buffer_map_.clear();
}

BufferManager::~BufferManager()
{
}

void BufferManager::CreateGBuffer(
	const std::string& name,
	const DXGI_FORMAT& format, const Color& clear_color)
{
	auto engine = Game::Get()->GetEngine();

	auto width = engine->FrameBufferWidth();
	auto height = engine->FrameBufferHeight();

	CreateGBuffer(name, format, clear_color, width, height);
}

void BufferManager::CreateGBuffer(
	const std::string& name, 
	const DXGI_FORMAT& format, const Color& clear_color, 
	const unsigned int width, const unsigned int height)
{
	auto engine = Game::Get()->GetEngine();

	D3D12_CLEAR_VALUE clear_value{};
	clear_value.Format = format;
	clear_value.Color[0] = clear_color.r;
	clear_value.Color[1] = clear_color.g;
	clear_value.Color[2] = clear_color.b;
	clear_value.Color[3] = clear_color.a;

	Buffer::BufferProperty prop{};
	prop.format = format;
	prop.width = width;
	prop.height = height;
	prop.flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	prop.states = D3D12_RESOURCE_STATE_RENDER_TARGET;
	prop.clear_value = clear_value;
	prop.rtv_heap = engine->RtvHeap().get();
	prop.srv_heap = engine->SrvHeap().get();

	buffer_map_[name] = std::make_shared<Buffer>(name, prop);
}

void BufferManager::CreateDepthStencilBuffer(
	const std::string& name,
	const unsigned int width, const unsigned int height)
{
	auto engine = Game::Get()->GetEngine();

	D3D12_CLEAR_VALUE clear_value{};
	clear_value.Format = DXGI_FORMAT_D32_FLOAT;
	clear_value.DepthStencil.Depth = 1.0;

	Buffer::BufferProperty prop{};
	prop.format = DXGI_FORMAT_D32_FLOAT;
	prop.width = width;
	prop.height = height;
	prop.flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	prop.states = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	prop.clear_value = clear_value;
	prop.dsv_heap = engine->DsvHeap().get();

	buffer_map_[name] = std::make_shared<Buffer>(name, prop);
}

std::shared_ptr<Buffer> BufferManager::Get(const std::string& name)
{
	return buffer_map_[name];
}

void BufferManager::CreateAll()
{
	for (auto iter = buffer_map_.begin(); iter != buffer_map_.end(); ++iter)
	{
		iter->second->Create();
	}
}

void BufferManager::ResetAll()
{
	for (auto iter = buffer_map_.begin(); iter != buffer_map_.end(); ++iter)
	{
		iter->second->Reset();
	}
}

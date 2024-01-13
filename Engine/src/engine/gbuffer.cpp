#include "gbuffer.h"
#include "d3dx12.h"
#include "engine/descriptor_heap.h"
#include "engine/engine.h"
#include "game/game.h"

GBuffer::GBuffer(std::string name, GBufferProperty prop)
{
	name_ = name;

	if (!CreateResource(prop))
	{
		is_valid_ = false;
		return;
	}

	if (!CreateSrv(prop))
	{
		is_valid_ = false;
		return;
	}

	if (!CreateRtv(prop))
	{
		is_valid_ = false;
		return;
	}

	is_valid_ = true;
}

GBuffer::~GBuffer()
{
	printf("Delete GBuffer[%s]\n", name_.c_str());
}

ID3D12Resource* GBuffer::Resource()
{
	return resource_.Get();
}

const DescriptorHandle& GBuffer::SrvHandle() const
{
	return srv_handle_;
}

const DescriptorHandle& GBuffer::RtvHandle() const
{
	return rtv_handle_;
}

bool GBuffer::IsValid()
{
	return is_valid_;
}

std::string GBuffer::GetName()
{
	return name_;
}

bool GBuffer::CreateResource(GBufferProperty prop)
{
	auto desc = CD3DX12_RESOURCE_DESC::Tex2D(
		prop.format,
		prop.width, prop.height,
		1, 1,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);

	// clear value
	D3D12_CLEAR_VALUE clear{};
	clear.Format = prop.format;

	auto device = Game::Get()->GetEngine()->Device();

	// リソースの生成
	const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clear,
		IID_PPV_ARGS(resource_.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
		return false;

	// 名前を設定
	resource_->SetName(std::wstring(name_.begin(), name_.end()).c_str());

	return true;
}

bool GBuffer::CreateSrv(GBufferProperty prop)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.Format = prop.format;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Texture2D.MipLevels = 1;
	desc.Texture2D.MostDetailedMip = 0;

	// handleの取得
	srv_handle_ = prop.srvHeap->Alloc();

	// SRVの生成
	Game::Get()->GetEngine()->Device()->CreateShaderResourceView(resource_.Get(), &desc, srv_handle_.HandleCPU());

	return true;
}

bool GBuffer::CreateRtv(GBufferProperty prop)
{
	D3D12_RENDER_TARGET_VIEW_DESC desc{};
	desc.Format = prop.format;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// handleの取得
	rtv_handle_ = prop.rtvHeap->Alloc();

	// RTVの生成
	Game::Get()->GetEngine()->Device()->CreateRenderTargetView(resource_.Get(), &desc, rtv_handle_.HandleCPU());

	return true;
}

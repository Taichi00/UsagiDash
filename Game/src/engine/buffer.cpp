#include "engine/buffer.h"
#include "engine/d3dx12.h"
#include "engine/descriptor_heap.h"
#include "engine/engine.h"
#include "game/game.h"

Buffer::Buffer(const std::string& name, const BufferProperty& prop)
{
	name_ = name;
	prop_ = prop;

	is_valid_ = Create();
}

Buffer::~Buffer()
{
	printf("Delete GBuffer[%s]\n", name_.c_str());
}

void Buffer::Clear()
{
	auto command_list = Game::Get()->GetEngine()->CommandList();
	command_list->ClearRenderTargetView(rtv_handle_.HandleCPU(), prop_.clear_value.Color, 0, nullptr);
}

ID3D12Resource* Buffer::Resource()
{
	return resource_.Get();
}

const DescriptorHandle& Buffer::SrvHandle() const
{
	return srv_handle_;
}

const DescriptorHandle& Buffer::RtvHandle() const
{
	return rtv_handle_;
}

const DescriptorHandle& Buffer::DsvHandle() const
{
	return dsv_handle_;
}

bool Buffer::IsValid()
{
	return is_valid_;
}

std::string Buffer::GetName()
{
	return name_;
}

bool Buffer::Create()
{
	if (!CreateResource())
	{
		return false;
	}

	if (!CreateSrv())
	{
		return false;
	}

	if (!CreateRtv())
	{
		return false;
	}

	if (!CreateDsv())
	{
		return false;
	}

	return true;
}

void Buffer::Reset()
{
	resource_.Reset();

	if (prop_.rtv_heap) prop_.rtv_heap->Free(rtv_handle_);
	if (prop_.srv_heap) prop_.srv_heap->Free(srv_handle_);
	if (prop_.dsv_heap) prop_.dsv_heap->Free(dsv_handle_);
}

bool Buffer::CreateResource()
{
	auto desc = CD3DX12_RESOURCE_DESC::Tex2D(
		prop_.format,
		prop_.width, prop_.height,
		1, 1,
		1, 0,
		prop_.flags
	);

	// clear value
	/*D3D12_CLEAR_VALUE clear{};
	clear.Format = prop_.format;*/

	auto device = Game::Get()->GetEngine()->Device();

	// リソースの生成
	const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		prop_.states,
		&prop_.clear_value,
		IID_PPV_ARGS(resource_.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
		return false;

	// 名前を設定
	resource_->SetName(std::wstring(name_.begin(), name_.end()).c_str());

	return true;
}

bool Buffer::CreateSrv()
{
	if (prop_.srv_heap == nullptr)
	{
		return true;
	}

	D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.Format = prop_.format;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Texture2D.MipLevels = 1;
	desc.Texture2D.MostDetailedMip = 0;

	// handleの取得
	srv_handle_ = prop_.srv_heap->Alloc();

	// SRVの生成
	Game::Get()->GetEngine()->Device()->CreateShaderResourceView(resource_.Get(), &desc, srv_handle_.HandleCPU());

	return true;
}

bool Buffer::CreateRtv()
{
	if (prop_.rtv_heap == nullptr)
	{
		return true;
	}

	D3D12_RENDER_TARGET_VIEW_DESC desc{};
	desc.Format = prop_.format;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// handleの取得
	rtv_handle_ = prop_.rtv_heap->Alloc();

	// RTVの生成
	Game::Get()->GetEngine()->Device()->CreateRenderTargetView(resource_.Get(), &desc, rtv_handle_.HandleCPU());

	return true;
}

bool Buffer::CreateDsv()
{
	if (prop_.dsv_heap == nullptr)
	{
		return true;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC desc{};
	desc.Format = prop_.format;
	desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	// handleの取得
	dsv_handle_ = prop_.dsv_heap->Alloc();

	// DSVの生成
	Game::Get()->GetEngine()->Device()->CreateDepthStencilView(resource_.Get(), &desc, dsv_handle_.HandleCPU());

	return true;
}

#include "GBuffer.h"
#include "d3dx12.h"
#include "DescriptorHeap.h"

GBuffer::GBuffer(std::string name, GBufferProperty prop)
{
	m_name = name;

	if (!CreateResource(prop))
	{
		m_isValid = false;
		return;
	}

	if (!CreateSrv(prop))
	{
		m_isValid = false;
		return;
	}

	if (!CreateRtv(prop))
	{
		m_isValid = false;
		return;
	}

	m_isValid = true;
}

GBuffer::~GBuffer()
{
}

ID3D12Resource* GBuffer::Resource()
{
	return m_pResource.Get();
}

DescriptorHandle* GBuffer::SrvHandle()
{
	return m_pSrvHandle.get();
}

DescriptorHandle* GBuffer::RtvHandle()
{
	return m_pRtvHandle.get();
}

bool GBuffer::IsValid()
{
	return m_isValid;
}

std::string GBuffer::GetName()
{
	return m_name;
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

	auto device = g_Engine->Device();

	// リソースの生成
	const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clear,
		IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
		return false;

	// 名前を設定
	m_pResource->SetName(std::wstring(m_name.begin(), m_name.end()).c_str());

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
	m_pSrvHandle = std::make_shared<DescriptorHandle>(prop.srvHeap->Alloc());

	// SRVの生成
	g_Engine->Device()->CreateShaderResourceView(m_pResource.Get(), &desc, m_pSrvHandle->HandleCPU());

	return true;
}

bool GBuffer::CreateRtv(GBufferProperty prop)
{
	D3D12_RENDER_TARGET_VIEW_DESC desc{};
	desc.Format = prop.format;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	// handleの取得
	m_pRtvHandle = std::make_shared<DescriptorHandle>(prop.rtvHeap->Alloc());

	// RTVの生成
	g_Engine->Device()->CreateRenderTargetView(m_pResource.Get(), &desc, m_pRtvHandle->HandleCPU());

	return true;
}

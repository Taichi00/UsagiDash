#include "DescriptorHeap.h"
#include "Texture2D.h"
#include "d3dx12.h"
#include "Engine.h"

DescriptorHeap::DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
	auto device = Engine::Get()->Device();

	// ディスクリプタヒープを生成
	auto hr = device->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(m_pHeap.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		m_IsValid = false;
		return;
	}

	m_handleCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart();

	// シェーダを利用する場合のみhandleGPUを設定する
	if (desc.Flags && D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
	{
		m_handleGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart();
	}

	m_IncrementSize = device->GetDescriptorHandleIncrementSize(desc.Type);
	m_index = 0;

	m_IsValid = true;
}

DescriptorHeap::~DescriptorHeap()
{
}

ID3D12DescriptorHeap* DescriptorHeap::GetHeap()
{
	return m_pHeap.Get();
}

DescriptorHandle DescriptorHeap::Alloc()
{
	if (!m_freeList.empty())	// 解放済みのディスクリプタがあればそれを使う
	{
		auto ret = m_freeList.front();
		m_freeList.pop_front();
		return ret;
	}

	auto hCPU = m_handleCPU;
	hCPU.ptr += m_IncrementSize * m_index;

	auto hGPU = m_handleGPU;
	hGPU.ptr += m_IncrementSize * m_index;

	m_index++;

	return DescriptorHandle(hCPU, hGPU);
}

void DescriptorHeap::Free(DescriptorHandle& handle)
{
	m_freeList.push_back(handle);
}

bool DescriptorHeap::IsValid()
{
	return m_IsValid;
}


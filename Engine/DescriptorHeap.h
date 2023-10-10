#pragma once
#include "ComPtr.h"
#include "d3dx12.h"
#include <vector>
#include <list>

class DescriptorHandle
{
public:
	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE hCPU, D3D12_GPU_DESCRIPTOR_HANDLE hGPU) :
		m_handleCPU(hCPU), m_handleGPU(hGPU) {}

	DescriptorHandle(DescriptorHandle* handle) :
		m_handleCPU(handle->HandleCPU()), m_handleGPU(handle->HandleGPU()) {}

	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU() const { return m_handleCPU; }
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU() const { return m_handleGPU; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_handleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE m_handleGPU;
};

class DescriptorHeap
{
public:
	DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc);	// コンストラクタで生成する
	ID3D12DescriptorHeap* GetHeap();				// ディスクリプタヒープを返す
	DescriptorHandle* Alloc();						// ディスクリプタを割り当てる
	void Free(DescriptorHandle* handle);			// ディスクリプタを開放する
	bool IsValid();

private:
	bool m_IsValid = false;	// 生成に成功したかどうか
	UINT m_IncrementSize = 0;
	ComPtr<ID3D12DescriptorHeap> m_pHeap = nullptr;	// ディスクリプタヒープ本体
	D3D12_CPU_DESCRIPTOR_HANDLE m_handleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE m_handleGPU;
	
	UINT m_index;
	std::list<DescriptorHandle*> m_freeList;
};
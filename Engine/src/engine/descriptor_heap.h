#pragma once
#include "engine/comptr.h"
#include "d3dx12.h"
#include <vector>
#include <list>

class DescriptorHandle
{
public:
	DescriptorHandle() : handle_cpu_(), handle_gpu_() {}
	~DescriptorHandle() {};

	DescriptorHandle(D3D12_CPU_DESCRIPTOR_HANDLE hCPU, D3D12_GPU_DESCRIPTOR_HANDLE hGPU) :
		handle_cpu_(hCPU), handle_gpu_(hGPU) {}

	/*DescriptorHandle(DescriptorHandle* handle) :
		m_handleCPU(handle->HandleCPU()), m_handleGPU(handle->HandleGPU()) {}*/

	D3D12_CPU_DESCRIPTOR_HANDLE HandleCPU() const { return handle_cpu_; }
	D3D12_GPU_DESCRIPTOR_HANDLE HandleGPU() const { return handle_gpu_; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE handle_cpu_;
	D3D12_GPU_DESCRIPTOR_HANDLE handle_gpu_;
};

class DescriptorHeap
{
public:
	DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc);	// コンストラクタで生成する
	~DescriptorHeap();

	ID3D12DescriptorHeap* GetHeap();				// ディスクリプタヒープを返す
	DescriptorHandle Alloc();						// ディスクリプタを割り当てる
	void Free(DescriptorHandle& handle);			// ディスクリプタを開放する
	bool IsValid();

private:
	bool is_valid_ = false;	// 生成に成功したかどうか
	UINT increment_size_ = 0;
	ComPtr<ID3D12DescriptorHeap> heap_ = nullptr;	// ディスクリプタヒープ本体
	D3D12_CPU_DESCRIPTOR_HANDLE handle_cpu_;
	D3D12_GPU_DESCRIPTOR_HANDLE handle_gpu_;
	
	UINT current_index_;
	std::list<DescriptorHandle> free_list_;
};
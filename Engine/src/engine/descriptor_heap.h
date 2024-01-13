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
	DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc);	// �R���X�g���N�^�Ő�������
	~DescriptorHeap();

	ID3D12DescriptorHeap* GetHeap();				// �f�B�X�N���v�^�q�[�v��Ԃ�
	DescriptorHandle Alloc();						// �f�B�X�N���v�^�����蓖�Ă�
	void Free(DescriptorHandle& handle);			// �f�B�X�N���v�^���J������
	bool IsValid();

private:
	bool is_valid_ = false;	// �����ɐ����������ǂ���
	UINT increment_size_ = 0;
	ComPtr<ID3D12DescriptorHeap> heap_ = nullptr;	// �f�B�X�N���v�^�q�[�v�{��
	D3D12_CPU_DESCRIPTOR_HANDLE handle_cpu_;
	D3D12_GPU_DESCRIPTOR_HANDLE handle_gpu_;
	
	UINT current_index_;
	std::list<DescriptorHandle> free_list_;
};
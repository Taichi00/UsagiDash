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
	DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc);	// �R���X�g���N�^�Ő�������
	ID3D12DescriptorHeap* GetHeap();				// �f�B�X�N���v�^�q�[�v��Ԃ�
	DescriptorHandle* Alloc();						// �f�B�X�N���v�^�����蓖�Ă�
	void Free(DescriptorHandle* handle);			// �f�B�X�N���v�^���J������
	bool IsValid();

private:
	bool m_IsValid = false;	// �����ɐ����������ǂ���
	UINT m_IncrementSize = 0;
	ComPtr<ID3D12DescriptorHeap> m_pHeap = nullptr;	// �f�B�X�N���v�^�q�[�v�{��
	D3D12_CPU_DESCRIPTOR_HANDLE m_handleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE m_handleGPU;
	
	UINT m_index;
	std::list<DescriptorHandle*> m_freeList;
};
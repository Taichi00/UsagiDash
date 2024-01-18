#pragma once
#include "engine/d3dx12.h"
#include "engine/comptr.h"

class ConstantBuffer
{
public:
	ConstantBuffer(size_t size);	// �R���X�g���N�^�Œ萔�o�b�t�@�𐶐�
	bool IsValid();	// �o�b�t�@�����ɐ�����������Ԃ�
	D3D12_GPU_VIRTUAL_ADDRESS GetAddress() const;	// �o�b�t�@��GPU��̃A�h���X��Ԃ�
	D3D12_CONSTANT_BUFFER_VIEW_DESC ViewDesc();		// �萔�o�b�t�@�r���[��Ԃ�

	void* GetPtr() const;	// �萔�o�b�t�@�Ƀ}�b�s���O���ꂽ�|�C���^��Ԃ�

	template<typename T>
	T* GetPtr()
	{
		return reinterpret_cast<T*>(GetPtr());
	}

private:
	bool is_valid_ = false;	// �萔�o�b�t�@�����ɐ���������
	ComPtr<ID3D12Resource> buffer_;		// �萔�o�b�t�@
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc_;	// �萔�o�b�t�@�r���[�̐ݒ�
	void* mapped_ptr_ = nullptr;

	ConstantBuffer(const ConstantBuffer&) = delete;
	void operator = (const ConstantBuffer&) = delete;
};
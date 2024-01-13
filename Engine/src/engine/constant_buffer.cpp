#include "constant_buffer.h"
#include "engine/engine.h"
#include "game/game.h"

ConstantBuffer::ConstantBuffer(size_t size)
{
	size_t align = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
	UINT64 sizeAligned = (size + (align - 1)) & ~(align - 1);	// align�ɐ؂�グ��
	
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);	// �q�[�v�v���p�e�B
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(sizeAligned);			// ���\�[�X�̐ݒ�

	// ���\�[�X�𐶐�
	auto hr = Game::Get()->GetEngine()->Device()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(buffer_.GetAddressOf())
	);
	if (FAILED(hr))
	{
		printf("�萔�o�b�t�@���\�[�X�̐����Ɏ��s\n");
		return;
	}

	hr = buffer_->Map(0, nullptr, &mapped_ptr_);
	if (FAILED(hr))
	{
		printf("�萔�o�b�t�@�̃}�b�s���O�Ɏ��s\n");
		return;
	}

	desc_ = {};
	desc_.BufferLocation = buffer_->GetGPUVirtualAddress();
	desc_.SizeInBytes = UINT(sizeAligned);

	is_valid_ = true;
}

bool ConstantBuffer::IsValid()
{
	return is_valid_;
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetAddress() const
{
	return desc_.BufferLocation;
}

D3D12_CONSTANT_BUFFER_VIEW_DESC ConstantBuffer::ViewDesc()
{
	return desc_;
}

void* ConstantBuffer::GetPtr() const
{
	return mapped_ptr_;
}

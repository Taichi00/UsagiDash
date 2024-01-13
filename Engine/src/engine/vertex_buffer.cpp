#include "vertex_buffer.h"
#include "engine/engine.h"
#include "d3dx12.h"
#include "game/game.h"

VertexBuffer::VertexBuffer(size_t size, size_t stride, const void* pInitData)
{
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);	// �q�[�v�v���p�e�B
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);	// ���\�[�X�̐ݒ�

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
		printf("���_�o�b�t�@���\�[�X�̐����Ɏ��s\n");
		return;
	}

	// ���_�o�b�t�@�r���[�̐ݒ�
	view_.BufferLocation	= buffer_->GetGPUVirtualAddress();
	view_.SizeInBytes		= static_cast<UINT>(size);
	view_.StrideInBytes	= static_cast<UINT>(stride);

	// �}�b�s���O����
	if (pInitData != nullptr)
	{
		void* ptr = nullptr;
		hr = buffer_->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			printf("���_�o�b�t�@�}�b�s���O�Ɏ��s\n");
			return;
		}

		// ���_�f�[�^���}�b�s���O��ɐݒ�
		memcpy(ptr, pInitData, size);

		// �}�b�s���O����
		buffer_->Unmap(0, nullptr);
	}

	is_valid_ = true;
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::View() const
{
	return view_;
}

bool VertexBuffer::IsValid()
{
	return is_valid_;
}



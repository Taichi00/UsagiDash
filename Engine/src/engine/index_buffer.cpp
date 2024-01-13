#include "index_buffer.h"
#include "d3dx12.h"
#include "engine/engine.h"
#include "game/game.h"

IndexBuffer::IndexBuffer(size_t size, const uint32_t* pInitData)
{
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); // �q�[�v�v���p�e�B
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);	// ���\�[�X�̐ݒ�

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
		printf("[OnInit] �C���f�b�N�X�o�b�t�@���\�[�X�̐����Ɏ��s");
		return;
	}

	// �C���f�b�N�X�o�b�t�@�r���[�̐ݒ�
	view_ = {};
	view_.BufferLocation	= buffer_->GetGPUVirtualAddress();
	view_.Format			= DXGI_FORMAT_R32_UINT;
	view_.SizeInBytes		= static_cast<UINT>(size);

	// �}�b�s���O����
	if (pInitData != nullptr)
	{
		void* ptr = nullptr;
		hr = buffer_->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			printf("[OnInit] �C���f�b�N�X�o�b�t�@�}�b�s���O�Ɏ��s");
			return;
		}

		// �C���f�b�N�X�f�[�^���}�b�s���O��ɐݒ�
		memcpy(ptr, pInitData, size);

		// �}�b�s���O����
		buffer_->Unmap(0, nullptr);
	}

	is_valid_ = true;
}

bool IndexBuffer::IsValid()
{
	return is_valid_;
}

D3D12_INDEX_BUFFER_VIEW IndexBuffer::View() const
{
	return view_;
}

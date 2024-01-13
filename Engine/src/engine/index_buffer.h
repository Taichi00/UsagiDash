#pragma once
#include <cstdint>
#include <d3d12.h>
#include "engine/comptr.h"

class IndexBuffer
{
public:
	IndexBuffer(size_t size, const uint32_t* pInitData = nullptr);
	bool IsValid();
	D3D12_INDEX_BUFFER_VIEW View() const;

private:
	bool is_valid_ = false;
	ComPtr<ID3D12Resource> buffer_;	// �C���f�b�N�X�o�b�t�@
	D3D12_INDEX_BUFFER_VIEW view_;		// �C���f�b�N�X�o�b�t�@�r���[

	IndexBuffer(const IndexBuffer&) = delete;
	void operator = (const IndexBuffer&) = delete;
};
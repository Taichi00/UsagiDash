#pragma once
#include <d3d12.h>
#include "engine/comptr.h"

class VertexBuffer
{
public:
	VertexBuffer(size_t size, size_t stride, const void* pInitData);	// �R���X�g���N�^�Ńo�b�t�@�𐶐�
	D3D12_VERTEX_BUFFER_VIEW View() const;	// ���_�o�b�t�@�r���[���擾
	bool IsValid();	// �o�b�t�@�̐����ɐ������������擾

private:
	bool is_valid_ = false;	// �o�b�t�@�̐����ɐ������������擾
	ComPtr<ID3D12Resource> buffer_ = nullptr;	// �o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW view_ = {};		// ���_�o�b�t�@�r���[

	VertexBuffer(const VertexBuffer&) = delete;
	void operator = (const VertexBuffer&) = delete;
};
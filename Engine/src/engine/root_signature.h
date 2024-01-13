#pragma once
#include "engine/comptr.h"
#include <vector>

struct ID3D12RootSignature;

enum RootSignatureParameter
{
	RSConstantBuffer = 0,
	RSTexture,
};

class RootSignature
{
public:
	RootSignature(const int numParams, RootSignatureParameter params[]);	// �R���X�g���N�^�Ń��[�g�V�O�l�`���𐶐�
	bool IsValid();		// ���[�g�V�O�l�`���̐����ɐ����������ǂ�����Ԃ�
	ID3D12RootSignature* Get();	// ���[�g�V�O�l�`����Ԃ�

private:
	bool is_valid_ = false;	// ���[�g�V�O�l�`���̐����ɐ����������ǂ���
	ComPtr<ID3D12RootSignature> root_signature_ = nullptr;	// ���[�g�V�O�l�`��
};
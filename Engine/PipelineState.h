#pragma once
#include "ComPtr.h"
#include "d3dx12.h"
#include <string>

class PipelineState
{
public:
	PipelineState();	// �R���X�g���N�^�ł�����x�̐ݒ������
	bool IsValid();		// �����ɐ����������ǂ�����Ԃ�

	void SetInputLayout(D3D12_INPUT_LAYOUT_DESC layout);		// ���̓��C�A�E�g��ݒ�
	void SetRootSignature(ID3D12RootSignature* rootSignature);	// ���[�g�V�O�l�`����ݒ�
	void SetVS(std::wstring filePath);			// ���_�V�F�[�_�[��ݒ�
	void SetPS(std::wstring filePath);			// �s�N�Z���V�F�[�_�[��ݒ�
	void SetCullMode(D3D12_CULL_MODE cullMode);	// �J�����O���[�h��ݒ�
	void SetAlpha();							// ���߃e�N�X�`���p�ɐݒ�
	void Create();	// �p�C�v���C���X�e�[�g�𐶐�

	ID3D12PipelineState* Get();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC* GetDesc();

private:
	bool m_IsValid = false;		// �����ɐ����������ǂ���
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};			// �p�C�v���C���X�e�[�g�̐ݒ�
	ComPtr<ID3D12PipelineState> m_pPipelineState = nullptr;	// �p�C�v���C���X�e�[�g
	ComPtr<ID3DBlob> m_pVSBlob;	// ���_�V�F�[�_�[
	ComPtr<ID3DBlob> m_pPSBlob;	// �s�N�Z���V�F�[�_�[
};
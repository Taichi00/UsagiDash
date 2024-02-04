#include "pipeline_state.h"
#include "engine/engine.h"
#include "game/game.h"
#include "engine/d3dx12.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

PipelineState::PipelineState()
{
	// �p�C�v���C���X�e�[�g�̐ݒ�
	desc_.RasterizerState			= CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);		// ���X�^���C�U�[�̓f�t�H���g
	desc_.RasterizerState.CullMode	= D3D12_CULL_MODE_NONE;							// �J�����O
	desc_.BlendState					= CD3DX12_BLEND_DESC(D3D12_DEFAULT);			// �u�����h�X�e�[�g���f�t�H���g
	desc_.DepthStencilState			= CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);	// �[�x�X�e���V���̓f�t�H���g���g��
	desc_.SampleMask					= UINT_MAX;
	desc_.PrimitiveTopologyType		= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;		// �O�p�`��`��
	desc_.NumRenderTargets			= 1;											// �`��Ώۂ�1
	desc_.RTVFormats[0]				= DXGI_FORMAT_R8G8B8A8_UNORM;
	desc_.DSVFormat					= DXGI_FORMAT_D32_FLOAT;
	desc_.SampleDesc.Count			= 1;											// �T���v���[��1
	desc_.SampleDesc.Quality			= 0;
}

bool PipelineState::IsValid()
{
	return is_valid_;
}

void PipelineState::SetInputLayout(D3D12_INPUT_LAYOUT_DESC layout)
{
	desc_.InputLayout = layout;
}

void PipelineState::SetRootSignature(ID3D12RootSignature* rootSignature)
{
	desc_.pRootSignature = rootSignature;
}

void PipelineState::SetVS(std::wstring filePath)
{
	// ���_�V�F�[�_�[�ǂݍ���
	auto hr = D3DReadFileToBlob((L"Assets/Shaders/" + filePath).c_str(), vs_blob_.GetAddressOf());
	if (FAILED(hr))
	{
		printf("���_�V�F�[�_�[�̓ǂݍ��݂Ɏ��s\n");
		return;
	}

	desc_.VS = CD3DX12_SHADER_BYTECODE(vs_blob_.Get());
}

void PipelineState::SetPS(std::wstring filePath)
{
	// �s�N�Z���V�F�[�_�[�ǂݍ���
	auto hr = D3DReadFileToBlob((L"Assets/Shaders/" + filePath).c_str(), ps_blob_.GetAddressOf());
	if (FAILED(hr))
	{
		printf("�s�N�Z���V�F�[�_�[�̓ǂݍ��݂Ɏ��s\n");
		return;
	}

	desc_.PS = CD3DX12_SHADER_BYTECODE(ps_blob_.Get());
}

void PipelineState::SetCullMode(D3D12_CULL_MODE cullMode)
{
	desc_.RasterizerState.CullMode = cullMode;
}

void PipelineState::SetRTVFormat(DXGI_FORMAT format)
{
	desc_.RTVFormats[0] = format;
}

void PipelineState::SetAlpha()
{
	// �u�����h�X�e�[�g�ݒ�
	desc_.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	auto& target = desc_.BlendState.RenderTarget[0];
	target.BlendEnable = TRUE;
	target.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	target.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	target.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	target.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;

	// �f�v�X�e�X�g�݂̂�L����
	desc_.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
}

void PipelineState::Create()
{
	// �p�C�v���C���X�e�[�g�𐶐�
	auto hr = Game::Get()->GetEngine()->Device()->CreateGraphicsPipelineState(&desc_, IID_PPV_ARGS(pipeline_state_.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		printf("�p�C�v���C���X�e�[�g�̐����Ɏ��s\n");
		return;
	}

	is_valid_ = true;
}

ID3D12PipelineState* PipelineState::Get()
{
	return pipeline_state_.Get();
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC* PipelineState::GetDesc()
{
	return &desc_;
}

ID3D12RootSignature* PipelineState::RootSignature()
{
	return desc_.pRootSignature;
}





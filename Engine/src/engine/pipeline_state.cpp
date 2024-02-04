#include "pipeline_state.h"
#include "engine/engine.h"
#include "game/game.h"
#include "engine/d3dx12.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

PipelineState::PipelineState()
{
	// パイプラインステートの設定
	desc_.RasterizerState			= CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);		// ラスタライザーはデフォルト
	desc_.RasterizerState.CullMode	= D3D12_CULL_MODE_NONE;							// カリング
	desc_.BlendState					= CD3DX12_BLEND_DESC(D3D12_DEFAULT);			// ブレンドステートもデフォルト
	desc_.DepthStencilState			= CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);	// 深度ステンシルはデフォルトを使う
	desc_.SampleMask					= UINT_MAX;
	desc_.PrimitiveTopologyType		= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;		// 三角形を描画
	desc_.NumRenderTargets			= 1;											// 描画対象は1
	desc_.RTVFormats[0]				= DXGI_FORMAT_R8G8B8A8_UNORM;
	desc_.DSVFormat					= DXGI_FORMAT_D32_FLOAT;
	desc_.SampleDesc.Count			= 1;											// サンプラーは1
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
	// 頂点シェーダー読み込み
	auto hr = D3DReadFileToBlob((L"Assets/Shaders/" + filePath).c_str(), vs_blob_.GetAddressOf());
	if (FAILED(hr))
	{
		printf("頂点シェーダーの読み込みに失敗\n");
		return;
	}

	desc_.VS = CD3DX12_SHADER_BYTECODE(vs_blob_.Get());
}

void PipelineState::SetPS(std::wstring filePath)
{
	// ピクセルシェーダー読み込み
	auto hr = D3DReadFileToBlob((L"Assets/Shaders/" + filePath).c_str(), ps_blob_.GetAddressOf());
	if (FAILED(hr))
	{
		printf("ピクセルシェーダーの読み込みに失敗\n");
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
	// ブレンドステート設定
	desc_.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	auto& target = desc_.BlendState.RenderTarget[0];
	target.BlendEnable = TRUE;
	target.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	target.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	target.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	target.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;

	// デプステストのみを有効化
	desc_.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
}

void PipelineState::Create()
{
	// パイプラインステートを生成
	auto hr = Game::Get()->GetEngine()->Device()->CreateGraphicsPipelineState(&desc_, IID_PPV_ARGS(pipeline_state_.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		printf("パイプラインステートの生成に失敗\n");
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





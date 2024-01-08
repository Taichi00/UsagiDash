#include "PipelineState.h"
#include "Engine.h"
#include "d3dx12.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

PipelineState::PipelineState()
{
	// パイプラインステートの設定
	desc.RasterizerState			= CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);		// ラスタライザーはデフォルト
	desc.RasterizerState.CullMode	= D3D12_CULL_MODE_NONE;							// カリング
	desc.BlendState					= CD3DX12_BLEND_DESC(D3D12_DEFAULT);			// ブレンドステートもデフォルト
	desc.DepthStencilState			= CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);	// 深度ステンシルはデフォルトを使う
	desc.SampleMask					= UINT_MAX;
	desc.PrimitiveTopologyType		= D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;		// 三角形を描画
	desc.NumRenderTargets			= 1;											// 描画対象は1
	desc.RTVFormats[0]				= DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.DSVFormat					= DXGI_FORMAT_D32_FLOAT;
	desc.SampleDesc.Count			= 1;											// サンプラーは1
	desc.SampleDesc.Quality			= 0;
}

bool PipelineState::IsValid()
{
	return m_IsValid;
}

void PipelineState::SetInputLayout(D3D12_INPUT_LAYOUT_DESC layout)
{
	desc.InputLayout = layout;
}

void PipelineState::SetRootSignature(ID3D12RootSignature* rootSignature)
{
	desc.pRootSignature = rootSignature;
}

void PipelineState::SetVS(std::wstring filePath)
{
	// 頂点シェーダー読み込み
	auto hr = D3DReadFileToBlob((L"Assets/Shaders/" + filePath).c_str(), m_pVSBlob.GetAddressOf());
	if (FAILED(hr))
	{
		printf("頂点シェーダーの読み込みに失敗\n");
		return;
	}

	desc.VS = CD3DX12_SHADER_BYTECODE(m_pVSBlob.Get());
}

void PipelineState::SetPS(std::wstring filePath)
{
	// ピクセルシェーダー読み込み
	auto hr = D3DReadFileToBlob((L"Assets/Shaders/" + filePath).c_str(), m_pPSBlob.GetAddressOf());
	if (FAILED(hr))
	{
		printf("ピクセルシェーダーの読み込みに失敗\n");
		return;
	}

	desc.PS = CD3DX12_SHADER_BYTECODE(m_pPSBlob.Get());
}

void PipelineState::SetCullMode(D3D12_CULL_MODE cullMode)
{
	desc.RasterizerState.CullMode = cullMode;
}

void PipelineState::SetRTVFormat(DXGI_FORMAT format)
{
	desc.RTVFormats[0] = format;
}

void PipelineState::SetAlpha()
{
	// ブレンドステート設定
	desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	auto& target = desc.BlendState.RenderTarget[0];
	target.BlendEnable = TRUE;
	target.SrcBlend = D3D12_BLEND_SRC_ALPHA;
	target.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	target.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
	target.DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;

	// デプステストのみを有効化
	desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
}

void PipelineState::Create()
{
	// パイプラインステートを生成
	auto hr = Engine::Get()->Device()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(m_pPipelineState.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		printf("パイプラインステートの生成に失敗\n");
		return;
	}

	m_IsValid = true;
}

ID3D12PipelineState* PipelineState::Get()
{
	return m_pPipelineState.Get();
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC* PipelineState::GetDesc()
{
	return &desc;
}





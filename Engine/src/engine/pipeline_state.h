#pragma once
#include "engine/comptr.h"
#include "d3dx12.h"
#include <string>
#include <dxgiformat.h>

class PipelineState
{
public:
	PipelineState();	// コンストラクタである程度の設定をする
	bool IsValid();		// 生成に成功したかどうかを返す

	void SetInputLayout(D3D12_INPUT_LAYOUT_DESC layout);		// 入力レイアウトを設定
	void SetRootSignature(ID3D12RootSignature* rootSignature);	// ルートシグネチャを設定
	void SetVS(std::wstring filePath);			// 頂点シェーダーを設定
	void SetPS(std::wstring filePath);			// ピクセルシェーダーを設定
	void SetCullMode(D3D12_CULL_MODE cullMode);	// カリングモードを設定
	void SetRTVFormat(DXGI_FORMAT format);		// RTVのフォーマットを設定
	void SetAlpha();							// 透過テクスチャ用に設
	void Create();	// パイプラインステートを生成

	ID3D12PipelineState* Get();
	D3D12_GRAPHICS_PIPELINE_STATE_DESC* GetDesc();

private:
	bool is_valid_ = false;		// 生成に成功したかどうか
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_ = {};			// パイプラインステートの設定
	ComPtr<ID3D12PipelineState> pipeline_state_ = nullptr;	// パイプラインステート
	ComPtr<ID3DBlob> vs_blob_;	// 頂点シェーダー
	ComPtr<ID3DBlob> ps_blob_;	// ピクセルシェーダー
};
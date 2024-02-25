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
	RootSignature(const int numParams, RootSignatureParameter params[]);	// コンストラクタでルートシグネチャを生成
	bool IsValid();		// ルートシグネチャの生成に成功したかどうかを返す
	ID3D12RootSignature* Get();	// ルートシグネチャを返す

private:
	bool is_valid_ = false;	// ルートシグネチャの生成に成功したかどうか
	ComPtr<ID3D12RootSignature> root_signature_ = nullptr;	// ルートシグネチャ
};
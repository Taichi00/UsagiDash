#pragma once
#include "ComPtr.h"
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
	bool m_IsValid = false;	// ルートシグネチャの生成に成功したかどうか
	ComPtr<ID3D12RootSignature> m_pRootSignature = nullptr;	// ルートシグネチャ
};
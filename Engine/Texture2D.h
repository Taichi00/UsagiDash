#pragma once
#include "ComPtr.h"
#include <string>
#include "d3dx12.h"

namespace DirectX
{
	struct ScratchImage;
	struct TexMetadata;
}

class DescriptorHeap;
class DescriptorHandle;

class Texture2D
{
public:
	static Texture2D* Get(std::string path);	// stringで受け取ったパスからテクスチャを読み込む
	static Texture2D* Get(std::wstring path);	// wstringで受け取ったパスからテクスチャを読み込む
	static Texture2D* Get(const void* pSource, size_t size);	// バイナリデータから読み込む
	static Texture2D* GetWhite();				// 白の単色テクスチャを生成する
	bool IsValid();	// 正常に読み込まれているかどうかを返す

	ID3D12Resource* Resource();	// リソースを返す
	D3D12_SHADER_RESOURCE_VIEW_DESC ViewDesc();	// シェーダーリソースビューの設定を返す
	DXGI_FORMAT Format();

private:
	bool m_IsValid;	// 正常に読み込まれているか

	Texture2D(std::wstring path);
	Texture2D(const void* pSource, size_t size);
	Texture2D(ID3D12Resource* buffer);

	ComPtr<ID3D12Resource> m_pResource;	// リソース
	DXGI_FORMAT m_pFormat;

	bool Load(std::wstring& path);
	bool Load(const void* pSource, size_t size);

	bool CreateResource(const CD3DX12_RESOURCE_DESC& desc, const void* src, UINT rowPitch, UINT slicePitch);
	bool CreateResource(const DirectX::ScratchImage* image, const DirectX::TexMetadata& metadata);

	static ID3D12Resource* GetDefaultResource(size_t width, size_t height);

	Texture2D(const Texture2D&) = delete;
	void operator = (const Texture2D&) = delete;
};
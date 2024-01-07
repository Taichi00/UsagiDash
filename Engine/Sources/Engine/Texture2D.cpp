#include "Texture2D.h"
#include "Engine.h"
#include "stb_image.h"

#pragma comment(lib, "DirectXTex.lib")

using namespace DirectX;


float Texture2D::defaultColor[4] = { 1, 1, 1, 1 };

// std::string(マルチバイト文字列)からstd::wstring(ワイド文字列)を得る
std::wstring GetWideString(const std::string& str)
{
	/*auto num1 = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, nullptr, 0);

	std::wstring wstr;
	wstr.resize(num1);

	auto num2 = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, &wstr[0], num1);

	assert(num1 == num2);
	return wstr;*/

	// SJIS → wstring
	int iBufferSize = MultiByteToWideChar(CP_ACP, 0, str.c_str()
		, -1, (wchar_t*)NULL, 0);

	// バッファの取得
	wchar_t* cpUCS2 = new wchar_t[iBufferSize];

	// SJIS → wstring
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, cpUCS2
		, iBufferSize);

	// stringの生成
	std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

	// バッファの破棄
	delete[] cpUCS2;

	// 変換結果を返す
	return oRet;
}

// 拡張子を返す
std::wstring FileExtension(const std::wstring& path)
{
	auto idx = path.rfind(L'.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

std::unique_ptr<Texture2D> Texture2D::Load(std::string path)
{
	auto wpath = GetWideString(path);
    return Load(wpath);
}

std::unique_ptr<Texture2D> Texture2D::Load(std::wstring path)
{
	auto tex = std::make_unique<Texture2D>(path);
	if (!tex->IsValid())
	{
		return GetMono(defaultColor);	// 読み込みに失敗したときは単色テクスチャを返す
	}
	return tex;
}

std::unique_ptr<Texture2D> Texture2D::Load(const void* pSource, size_t size)
{
	auto tex = std::make_unique<Texture2D>(pSource, size);
	if (!tex->IsValid())
	{
		return GetMono(defaultColor);	// 読み込みに失敗したときは単色テクスチャを返す
	}
	return tex;
}

std::unique_ptr<Texture2D> Texture2D::GetMono(const float color[4])
{
	auto buff = GetDefaultResource(4, 4);

	std::vector<unsigned char> data(4 * 4 * 4);

	for (size_t i = 0; i < 4 * 4 * 4; i += 4)
	{
		data[i + 0] = color[0] * 255;
		data[i + 1] = color[1] * 255;
		data[i + 2] = color[2] * 255;
		data[i + 3] = color[3] * 255;
	}
	//std::fill(data.begin(), data.end(), 0xff);

	auto hr = buff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, data.size());
	if (FAILED(hr))
	{
		return nullptr;
	}

	return std::make_unique<Texture2D>(buff.Get());

	/*std::vector<unsigned char> data(4 * 4 * 4);

	for (size_t i = 0; i < 4 * 4 * 4; i += 4)
	{
		data[i + 0] = color[0] * 255;
		data[i + 1] = color[1] * 255;
		data[i + 2] = color[2] * 255;
		data[i + 3] = color[3] * 255;
	}

	return new Texture2D(data.data(), 16);*/
}

std::unique_ptr<Texture2D> Texture2D::GetWhite()
{
	float white[4] = {1, 1, 1, 1};
	return GetMono(white);
}

void Texture2D::SetDefaultColor(const float color[4])
{
	defaultColor[0] = color[0];
	defaultColor[1] = color[1];
	defaultColor[2] = color[2];
	defaultColor[3] = color[3];
}

bool Texture2D::IsValid()
{
	return m_IsValid;
}

ID3D12Resource* Texture2D::Resource()
{
	return m_pResource.Get();
}

D3D12_SHADER_RESOURCE_VIEW_DESC Texture2D::ViewDesc()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = m_pFormat;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; //2Dテクスチャ
	desc.Texture2D.MipLevels = 1; //ミップマップは使用しないので1
	return desc;
}

DXGI_FORMAT Texture2D::Format()
{
	return m_pFormat;
}

DirectX::TexMetadata Texture2D::Metadata()
{
	return m_metadata;
}

Texture2D::Texture2D()
{
}

Texture2D::Texture2D(std::wstring path)
{
	m_IsValid = LoadTexture(path);
}

Texture2D::Texture2D(const void* pSource, size_t size)
{
	m_IsValid = LoadTexture(pSource, size);
}

Texture2D::Texture2D(ID3D12Resource* buffer)
{
	m_pResource = buffer;
	m_IsValid = m_pResource != nullptr;
}

Texture2D::~Texture2D()
{
}

bool Texture2D::LoadTexture(std::wstring& path)
{
	// WICテクスチャのロード
	TexMetadata meta = {};
	ScratchImage image = {};
	auto ext = FileExtension(path);

	HRESULT hr = E_FAIL;
	if (ext == L"png")		// pngの時はWICFileを使う
	{
		hr = LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, &meta, image);
	}
	else if (ext == L"tga")	// tgaの時はTGAFileを使う
	{
		hr = LoadFromTGAFile(path.c_str(), &meta, image);
	}
	else if (ext == L"dds")
	{
		hr = LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, &meta, image);
	}
	if (FAILED(hr))
	{
		return false;
	}

	m_metadata = meta;

	auto img = image.GetImage(0, 0, 0);
	auto desc = CD3DX12_RESOURCE_DESC::Tex2D(
		meta.format,
		meta.width,
		meta.height,
		static_cast<UINT16>(meta.arraySize),
		static_cast<UINT16>(meta.mipLevels)
	);

	//return CreateResource(desc, img->pixels, static_cast<UINT>(img->rowPitch), static_cast<UINT>(img->slicePitch));
	return CreateResource(&image, meta);
}

bool Texture2D::LoadTexture(const void* pSource, size_t size)
{
	// WICテクスチャのロード
	DirectX::ScratchImage image = {};

	auto hr = DirectX::LoadFromWICMemory(pSource, size, DirectX::WIC_FLAGS_NONE, nullptr, image);
	if (FAILED(hr))
	{
		return false;
	}

	auto img = image.GetImage(0, 0, 0);
	auto meta = image.GetMetadata();
	auto desc = CD3DX12_RESOURCE_DESC::Tex2D(
		meta.format,
		meta.width,
		meta.height,
		static_cast<UINT16>(meta.arraySize),
		static_cast<UINT16>(meta.mipLevels)
	);
	
	return CreateResource(desc, img->pixels, static_cast<UINT>(img->rowPitch), static_cast<UINT>(img->slicePitch));
	//return CreateResource(&image, meta);
}

bool Texture2D::CreateResource(
	const CD3DX12_RESOURCE_DESC& desc, const void* src, UINT rowPitch, UINT slicePitch)
{
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);

	// リソースを生成
	auto hr = g_Engine->Device()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(m_pResource.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}

	hr = m_pResource->WriteToSubresource(
		0,
		nullptr,	// 全領域へコピー
		src,		// 元データアドレス
		rowPitch,	// 1ラインサイズ
		slicePitch	// 全サイズ
	);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}


bool Texture2D::CreateResource(
	const DirectX::ScratchImage* image, const DirectX::TexMetadata& metadata)
{
	// テクスチャ生成
	ComPtr<ID3D12Resource> texture;
	CreateTexture(g_Engine->Device(), metadata, &texture);

	// アップロードヒープ用準備
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	PrepareUpload(
		g_Engine->Device(), image->GetImages(), image->GetImageCount(),
		metadata, subresources
	);

	// アップロード
	if (!g_Engine->UploadTexture(texture.Get(), subresources))
	{
		printf("テクスチャのアップロードに失敗\n");
		return false;
	}

	m_pResource = texture;
	m_pFormat = metadata.format;
	
	return true;
}

ComPtr<ID3D12Resource> Texture2D::GetDefaultResource(size_t width, size_t height)
{
	auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
	auto texHeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_CPU_PAGE_PROPERTY_WRITE_BACK, D3D12_MEMORY_POOL_L0);
	ComPtr<ID3D12Resource> buff = nullptr;
	
	auto result = g_Engine->Device()->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(buff.ReleaseAndGetAddressOf())
	);
	if (FAILED(result))
	{
		auto hr = g_Engine->Device()->GetDeviceRemovedReason();
		assert(SUCCEEDED(result));
		return nullptr;
	}

	return std::move(buff);
}

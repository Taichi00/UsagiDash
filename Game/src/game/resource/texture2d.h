#pragma once
#include "engine/d3dx12.h"
#include "engine/comptr.h"
#include "game/resource/resource.h"
#include <DirectXTex.h>
#include <memory>
#include <string>

class DescriptorHeap;
class DescriptorHandle;

class Texture2D : public Resource
{
public:
	Texture2D();
	~Texture2D();

	Texture2D(std::wstring path);
	Texture2D(const void* pSource, size_t size);
	Texture2D(ID3D12Resource* buffer);

	static std::unique_ptr<Texture2D> Load(const std::string& path);	// string�Ŏ󂯎�����p�X����e�N�X�`����ǂݍ���
	static std::unique_ptr<Texture2D> Load(const std::wstring& path);	// wstring�Ŏ󂯎�����p�X����e�N�X�`����ǂݍ���
	static std::unique_ptr<Texture2D> Load(const void* pSource, size_t size);	// �o�C�i���f�[�^����ǂݍ���

	static std::unique_ptr<Texture2D> GetMono(const float color[4]);	// �P�F�e�N�X�`���𐶐�����
	static std::unique_ptr<Texture2D> GetWhite();

	static void SetDefaultColor(const float color[4]); // �ǂݍ��ݎ��s���̃e�N�X�`���F��ݒ肷��

	bool IsValid();	// ����ɓǂݍ��܂�Ă��邩�ǂ�����Ԃ�

	ID3D12Resource* Resource() const;	// ���\�[�X��Ԃ�
	D3D12_SHADER_RESOURCE_VIEW_DESC ViewDesc() const;	// �V�F�[�_�[���\�[�X�r���[�̐ݒ��Ԃ�
	DXGI_FORMAT Format() const;
	DirectX::TexMetadata Metadata() const;

private:
	bool LoadTexture(std::wstring& path);
	bool LoadTexture(const void* pSource, size_t size);

	bool CreateResource(const CD3DX12_RESOURCE_DESC& desc, const void* src, UINT rowPitch, UINT slicePitch);
	bool CreateResource(const DirectX::ScratchImage* image, const DirectX::TexMetadata& metadata);

	static ComPtr<ID3D12Resource> GetDefaultResource(size_t width, size_t height);

private:
	bool is_valid_ = false;	// ����ɓǂݍ��܂�Ă��邩

	ComPtr<ID3D12Resource> resource_;	// ���\�[�X
	DXGI_FORMAT format_ = {};

	static float default_color_[];

	DirectX::TexMetadata metadata_ = {};

private:
	Texture2D(const Texture2D&) = delete;
	void operator = (const Texture2D&) = delete;
	
};
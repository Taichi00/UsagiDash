#pragma once
#include "ComPtr.h"
#include <string>
#include "d3dx12.h"
#include <DirectXTex.h>
#include <memory>
#include "Resource.h"

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

	static std::unique_ptr<Texture2D> Load(std::string path);	// string�Ŏ󂯎�����p�X����e�N�X�`����ǂݍ���
	static std::unique_ptr<Texture2D> Load(std::wstring path);	// wstring�Ŏ󂯎�����p�X����e�N�X�`����ǂݍ���
	static std::unique_ptr<Texture2D> Load(const void* pSource, size_t size);	// �o�C�i���f�[�^����ǂݍ���

	static std::unique_ptr<Texture2D> GetMono(const float color[4]);	// �P�F�e�N�X�`���𐶐�����
	static std::unique_ptr<Texture2D> GetWhite();

	static void SetDefaultColor(const float color[4]); // �ǂݍ��ݎ��s���̃e�N�X�`���F��ݒ肷��

	bool IsValid();	// ����ɓǂݍ��܂�Ă��邩�ǂ�����Ԃ�

	ID3D12Resource* Resource();	// ���\�[�X��Ԃ�
	D3D12_SHADER_RESOURCE_VIEW_DESC ViewDesc();	// �V�F�[�_�[���\�[�X�r���[�̐ݒ��Ԃ�
	DXGI_FORMAT Format();
	DirectX::TexMetadata Metadata();

private:
	bool m_IsValid;	// ����ɓǂݍ��܂�Ă��邩

	ComPtr<ID3D12Resource> m_pResource;	// ���\�[�X
	DXGI_FORMAT m_pFormat;

	bool LoadTexture(std::wstring& path);
	bool LoadTexture(const void* pSource, size_t size);

	bool CreateResource(const CD3DX12_RESOURCE_DESC& desc, const void* src, UINT rowPitch, UINT slicePitch);
	bool CreateResource(const DirectX::ScratchImage* image, const DirectX::TexMetadata& metadata);

	static ComPtr<ID3D12Resource> GetDefaultResource(size_t width, size_t height);

	Texture2D(const Texture2D&) = delete;
	void operator = (const Texture2D&) = delete;

public:
	static float defaultColor[];

private:
	DirectX::TexMetadata m_metadata;
};
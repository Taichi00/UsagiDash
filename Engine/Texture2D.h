#pragma once
#include "ComPtr.h"
#include <string>
#include "d3dx12.h"
#include <DirectXTex.h>

class DescriptorHeap;
class DescriptorHandle;

class Texture2D
{
public:
	static Texture2D* Get(std::string path);	// string�Ŏ󂯎�����p�X����e�N�X�`����ǂݍ���
	static Texture2D* Get(std::wstring path);	// wstring�Ŏ󂯎�����p�X����e�N�X�`����ǂݍ���
	static Texture2D* Get(const void* pSource, size_t size);	// �o�C�i���f�[�^����ǂݍ���
	static Texture2D* GetMono(const float color[4]);	// �P�F�e�N�X�`���𐶐�����
	static Texture2D* GetWhite();

	static void SetDefaultColor(const float color[4]); // �ǂݍ��ݎ��s���̃e�N�X�`���F��ݒ肷��

	bool IsValid();	// ����ɓǂݍ��܂�Ă��邩�ǂ�����Ԃ�

	ID3D12Resource* Resource();	// ���\�[�X��Ԃ�
	D3D12_SHADER_RESOURCE_VIEW_DESC ViewDesc();	// �V�F�[�_�[���\�[�X�r���[�̐ݒ��Ԃ�
	DXGI_FORMAT Format();
	DirectX::TexMetadata Metadata();

private:
	bool m_IsValid;	// ����ɓǂݍ��܂�Ă��邩

	Texture2D(std::wstring path);
	Texture2D(const void* pSource, size_t size);
	Texture2D(ID3D12Resource* buffer);

	ComPtr<ID3D12Resource> m_pResource;	// ���\�[�X
	DXGI_FORMAT m_pFormat;

	bool Load(std::wstring& path);
	bool Load(const void* pSource, size_t size);

	bool CreateResource(const CD3DX12_RESOURCE_DESC& desc, const void* src, UINT rowPitch, UINT slicePitch);
	bool CreateResource(const DirectX::ScratchImage* image, const DirectX::TexMetadata& metadata);

	static ID3D12Resource* GetDefaultResource(size_t width, size_t height);

	Texture2D(const Texture2D&) = delete;
	void operator = (const Texture2D&) = delete;

public:
	static float defaultColor[];

private:
	DirectX::TexMetadata m_metadata;
};
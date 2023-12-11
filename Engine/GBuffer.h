#pragma once

#include <string>
#include "Engine.h"
#include <string>

class DescriptorHandle;
class DescriptorHeap;

struct GBufferProperty
{
	DXGI_FORMAT format;
	UINT width;
	UINT height;
	DescriptorHeap* srvHeap;
	DescriptorHeap* rtvHeap;
};

class GBuffer
{
public:
	GBuffer(std::string name, GBufferProperty prop);
	~GBuffer();

	ID3D12Resource* Resource();
	DescriptorHandle* SrvHandle();
	DescriptorHandle* RtvHandle();

	bool IsValid();	// 成功したかどうか
	std::string GetName();

private:
	bool CreateResource(GBufferProperty prop);
	bool CreateSrv(GBufferProperty prop);
	bool CreateRtv(GBufferProperty prop);

private:
	std::string m_name;	// 名前
	bool m_isValid = false;

	ComPtr<ID3D12Resource> m_pResource;				// リソース
	std::shared_ptr<DescriptorHandle> m_pSrvHandle;	// SRVハンドル
	std::shared_ptr<DescriptorHandle> m_pRtvHandle;	// RTVハンドル
};
#pragma once

#include <string>
#include <dxgiformat.h>
#include <d3d12.h>
#include "comptr.h"
#include "descriptor_heap.h"

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
	const DescriptorHandle& SrvHandle() const;
	const DescriptorHandle& RtvHandle() const;

	bool IsValid();	// 成功したかどうか
	std::string GetName();

private:
	bool CreateResource(GBufferProperty prop);
	bool CreateSrv(GBufferProperty prop);
	bool CreateRtv(GBufferProperty prop);

private:
	std::string name_;	// 名前
	bool is_valid_ = false;

	ComPtr<ID3D12Resource> resource_;				// リソース
	DescriptorHandle srv_handle_;	// SRVハンドル
	DescriptorHandle rtv_handle_;	// RTVハンドル
};
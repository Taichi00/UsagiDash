#pragma once

#include <string>
#include <dxgiformat.h>
#include <d3d12.h>
#include "comptr.h"
#include "descriptor_heap.h"

class Buffer
{
public:
	struct BufferProperty
	{
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		UINT width = 0;
		UINT height = 0;
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		D3D12_RESOURCE_STATES states = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		D3D12_CLEAR_VALUE clear_value = D3D12_CLEAR_VALUE{};
		DescriptorHeap* srv_heap = nullptr;
		DescriptorHeap* rtv_heap = nullptr;
		DescriptorHeap* dsv_heap = nullptr;
	};

	Buffer(const std::string& name, const BufferProperty& prop);
	~Buffer();

	void Clear();

	ID3D12Resource* Resource();
	const DescriptorHandle& SrvHandle() const;
	const DescriptorHandle& RtvHandle() const;
	const DescriptorHandle& DsvHandle() const;

	bool IsValid();	// 成功したかどうか
	std::string GetName();

	bool Create();
	void Reset();

private:
	bool CreateResource();
	bool CreateSrv();
	bool CreateRtv();
	bool CreateDsv();

private:
	std::string name_;	// 名前
	BufferProperty prop_;
	bool is_valid_ = false;

	ComPtr<ID3D12Resource> resource_;				// リソース
	DescriptorHandle srv_handle_;	// SRVハンドル
	DescriptorHandle rtv_handle_;	// RTVハンドル
	DescriptorHandle dsv_handle_;	// DSVハンドル
};
#pragma once
#include "engine/d3dx12.h"
#include "engine/comptr.h"

class ConstantBuffer
{
public:
	ConstantBuffer(size_t size);	// コンストラクタで定数バッファを生成
	bool IsValid();	// バッファ生成に成功したかを返す
	D3D12_GPU_VIRTUAL_ADDRESS GetAddress() const;	// バッファのGPU上のアドレスを返す
	D3D12_CONSTANT_BUFFER_VIEW_DESC ViewDesc();		// 定数バッファビューを返す

	void* GetPtr() const;	// 定数バッファにマッピングされたポインタを返す

	template<typename T>
	T* GetPtr()
	{
		return reinterpret_cast<T*>(GetPtr());
	}

private:
	bool is_valid_ = false;	// 定数バッファ生成に成功したか
	ComPtr<ID3D12Resource> buffer_;		// 定数バッファ
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc_;	// 定数バッファビューの設定
	void* mapped_ptr_ = nullptr;

	ConstantBuffer(const ConstantBuffer&) = delete;
	void operator = (const ConstantBuffer&) = delete;
};
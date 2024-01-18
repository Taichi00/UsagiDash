#include "engine/descriptor_heap.h"
#include "engine/texture2d.h"
#include "engine/d3dx12.h"
#include "engine/engine.h"
#include "game/game.h"

DescriptorHeap::DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc)
{
	auto device = Game::Get()->GetEngine()->Device();

	// ディスクリプタヒープを生成
	auto hr = device->CreateDescriptorHeap(
		&desc,
		IID_PPV_ARGS(heap_.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		is_valid_ = false;
		return;
	}

	handle_cpu_ = heap_->GetCPUDescriptorHandleForHeapStart();

	// シェーダを利用する場合のみhandleGPUを設定する
	if (desc.Flags && D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
	{
		handle_gpu_ = heap_->GetGPUDescriptorHandleForHeapStart();
	}

	increment_size_ = device->GetDescriptorHandleIncrementSize(desc.Type);
	current_index_ = 0;

	is_valid_ = true;
}

DescriptorHeap::~DescriptorHeap()
{
}

ID3D12DescriptorHeap* DescriptorHeap::GetHeap()
{
	return heap_.Get();
}

DescriptorHandle DescriptorHeap::Alloc()
{
	if (!free_list_.empty())	// 解放済みのディスクリプタがあればそれを使う
	{
		auto ret = free_list_.front();
		free_list_.pop_front();
		return ret;
	}

	auto hCPU = handle_cpu_;
	hCPU.ptr += increment_size_ * current_index_;

	auto hGPU = handle_gpu_;
	hGPU.ptr += increment_size_ * current_index_;

	current_index_++;

	return DescriptorHandle(hCPU, hGPU);
}

void DescriptorHeap::Free(DescriptorHandle& handle)
{
	free_list_.push_back(handle);
}

bool DescriptorHeap::IsValid()
{
	return is_valid_;
}


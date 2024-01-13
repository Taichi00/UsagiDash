#include "vertex_buffer.h"
#include "engine/engine.h"
#include "d3dx12.h"
#include "game/game.h"

VertexBuffer::VertexBuffer(size_t size, size_t stride, const void* pInitData)
{
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);	// ヒーププロパティ
	auto desc = CD3DX12_RESOURCE_DESC::Buffer(size);	// リソースの設定

	// リソースを生成
	auto hr = Game::Get()->GetEngine()->Device()->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(buffer_.GetAddressOf())
	);

	if (FAILED(hr))
	{
		printf("頂点バッファリソースの生成に失敗\n");
		return;
	}

	// 頂点バッファビューの設定
	view_.BufferLocation	= buffer_->GetGPUVirtualAddress();
	view_.SizeInBytes		= static_cast<UINT>(size);
	view_.StrideInBytes	= static_cast<UINT>(stride);

	// マッピングする
	if (pInitData != nullptr)
	{
		void* ptr = nullptr;
		hr = buffer_->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			printf("頂点バッファマッピングに失敗\n");
			return;
		}

		// 頂点データをマッピング先に設定
		memcpy(ptr, pInitData, size);

		// マッピング解除
		buffer_->Unmap(0, nullptr);
	}

	is_valid_ = true;
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::View() const
{
	return view_;
}

bool VertexBuffer::IsValid()
{
	return is_valid_;
}



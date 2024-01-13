#include "index_buffer.h"
#include "d3dx12.h"
#include "engine/engine.h"
#include "game/game.h"

IndexBuffer::IndexBuffer(size_t size, const uint32_t* pInitData)
{
	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); // ヒーププロパティ
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);	// リソースの設定

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
		printf("[OnInit] インデックスバッファリソースの生成に失敗");
		return;
	}

	// インデックスバッファビューの設定
	view_ = {};
	view_.BufferLocation	= buffer_->GetGPUVirtualAddress();
	view_.Format			= DXGI_FORMAT_R32_UINT;
	view_.SizeInBytes		= static_cast<UINT>(size);

	// マッピングする
	if (pInitData != nullptr)
	{
		void* ptr = nullptr;
		hr = buffer_->Map(0, nullptr, &ptr);
		if (FAILED(hr))
		{
			printf("[OnInit] インデックスバッファマッピングに失敗");
			return;
		}

		// インデックスデータをマッピング先に設定
		memcpy(ptr, pInitData, size);

		// マッピング解除
		buffer_->Unmap(0, nullptr);
	}

	is_valid_ = true;
}

bool IndexBuffer::IsValid()
{
	return is_valid_;
}

D3D12_INDEX_BUFFER_VIEW IndexBuffer::View() const
{
	return view_;
}

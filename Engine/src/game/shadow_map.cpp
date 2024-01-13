#include "game/shadow_map.h"
#include "engine/engine.h"
#include "engine/descriptor_heap.h"
#include "engine/root_signature.h"
#include "engine/pipeline_state.h"
#include "game/game.h"

ShadowMap::ShadowMap()
{
	if (!CreateShadowBuffer())
	{
		printf("ShadowMap: Bufferの生成に失敗\n");
		return;
	}

	if (!PrepareRootSignature())
	{
		printf("ShadowMap: RootSignatureの生成に失敗\n");
		return;
	}

	if (!PreparePSO())
	{
		printf("ShadowMap: PSOの生成に失敗\n");
		return;
	}
}

ShadowMap::~ShadowMap()
{
	Game::Get()->GetEngine()->RtvHeap()->Free(rtv_handle_[0]);
	Game::Get()->GetEngine()->RtvHeap()->Free(rtv_handle_[1]);
	Game::Get()->GetEngine()->DsvHeap()->Free(dsv_handle_);
	Game::Get()->GetEngine()->GBufferHeap()->Free(srv_handle_[0]);
	Game::Get()->GetEngine()->GBufferHeap()->Free(srv_handle_[1]);
}

void ShadowMap::BeginRender()
{
	auto commandList = Game::Get()->GetEngine()->CommandList();

	// レンダーターゲットが使用可能になるまで待つ
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			color_[0].Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			color_[1].Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		),
	};
	commandList->ResourceBarrier(_countof(barriers), barriers);

	// レンダーターゲットを設定
	auto rtvHandle = rtv_handle_[0].HandleCPU();
	auto dsvHandle = dsv_handle_.HandleCPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// レンダーターゲットをクリア
	const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	commandList->ClearRenderTargetView(rtv_handle_[0].HandleCPU(), clearColor, 0, nullptr);
	commandList->ClearRenderTargetView(rtv_handle_[1].HandleCPU(), clearColor, 0, nullptr);

	// 深度ステンシルビューをクリア
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// ビューポートとシザー矩形を設定
	auto width = width_;
	auto height = height_;
	auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, float(width), float(height));
	auto scissor = CD3DX12_RECT(0, 0, LONG(width), LONG(height));
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);

}

void ShadowMap::EndRender()
{	
	auto commandList = Game::Get()->GetEngine()->CommandList();
	
	// バリア設定
	auto barrierToSR = CD3DX12_RESOURCE_BARRIER::Transition(
		color_[0].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	commandList->ResourceBarrier(1, &barrierToSR);

	// ブラー
	BlurHorizontal();
	BlurVertical();

	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			color_[0].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		),
	};
	commandList->ResourceBarrier(_countof(barriers), barriers);
}

ID3D12Resource* ShadowMap::Resource()
{
	return color_[0].Get();
}

DescriptorHandle* ShadowMap::SrvHandle()
{
	return &srv_handle_[0];
}

bool ShadowMap::CreateShadowBuffer()
{
	// リソースの生成
	auto colorDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		width_, height_,
		1, 1,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);

	auto depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		width_, height_,
		1, 1,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	D3D12_CLEAR_VALUE clearColor{}, clearDepth{};

	clearColor.Format = colorDesc.Format;
	clearColor.Color[0] = 1.0f;	clearColor.Color[1] = 1.0f;
	clearColor.Color[2] = 1.0f;	clearColor.Color[3] = 1.0f;

	clearDepth.Format = depthDesc.Format;
	clearDepth.DepthStencil.Depth = 1.0f;

	auto device = Game::Get()->GetEngine()->Device();

	// Render Target の生成
	const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&colorDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearColor,
		IID_PPV_ARGS(color_[0].ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}

	hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&colorDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearColor,
		IID_PPV_ARGS(color_[1].ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}

	// Depth Buffer の生成
	hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearDepth,
		IID_PPV_ARGS(depth_.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}

	color_[0]->SetName(L"ShadowMap[0]");
	color_[1]->SetName(L"ShadowMap[1]");

	// ディスクリプタの登録
	rtv_handle_[0] = Game::Get()->GetEngine()->RtvHeap()->Alloc();
	rtv_handle_[1] = Game::Get()->GetEngine()->RtvHeap()->Alloc();
	dsv_handle_ = Game::Get()->GetEngine()->DsvHeap()->Alloc();
	srv_handle_[0] = Game::Get()->GetEngine()->GBufferHeap()->Alloc();
	srv_handle_[1] = Game::Get()->GetEngine()->GBufferHeap()->Alloc();

	// ビューの生成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = colorDesc.Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(color_[0].Get(), &rtvDesc, rtv_handle_[0].HandleCPU());
	device->CreateRenderTargetView(color_[1].Get(), &rtvDesc, rtv_handle_[1].HandleCPU());

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = depthDesc.Format;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(depth_.Get(), &dsvDesc, dsv_handle_.HandleCPU());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = colorDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	device->CreateShaderResourceView(color_[0].Get(), &srvDesc, srv_handle_[0].HandleCPU());
	device->CreateShaderResourceView(color_[1].Get(), &srvDesc, srv_handle_[1].HandleCPU());

	return true;
}

bool ShadowMap::PreparePSO()
{
	// BlurHorizontal用
	blur_horizontal_pso_ = new PipelineState();
	blur_horizontal_pso_->SetInputLayout({ nullptr, 0 });
	blur_horizontal_pso_->SetRootSignature(root_signature_->Get());
	blur_horizontal_pso_->SetVS(L"ScreenVS.cso");
	blur_horizontal_pso_->SetPS(L"GaussianBlurHorizontal.cso");
	blur_horizontal_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	blur_horizontal_pso_->SetRTVFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);

	auto desc = blur_horizontal_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	blur_horizontal_pso_->Create();
	if (!blur_horizontal_pso_->IsValid())
	{
		return false;
	}

	// BlurVertical用
	blur_vertical_pso_ = new PipelineState();
	blur_vertical_pso_->SetInputLayout({ nullptr, 0 });
	blur_vertical_pso_->SetRootSignature(root_signature_->Get());
	blur_vertical_pso_->SetVS(L"ScreenVS.cso");
	blur_vertical_pso_->SetPS(L"GaussianBlurVertical.cso");
	blur_vertical_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	blur_vertical_pso_->SetRTVFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);

	desc = blur_vertical_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	blur_vertical_pso_->Create();
	if (!blur_vertical_pso_->IsValid())
	{
		return false;
	}

	return true;
}

bool ShadowMap::PrepareRootSignature()
{
	RootSignatureParameter params[] = {
		RSTexture,
	};

	root_signature_ = new RootSignature(_countof(params), params);
	if (!root_signature_->IsValid())
	{
		return false;
	}

	return true;
}

void ShadowMap::BlurHorizontal()
{
	auto commandList = Game::Get()->GetEngine()->CommandList();

	// レンダーターゲットを設定
	auto rtvHandle = rtv_handle_[1].HandleCPU();
	auto dsvHandle = dsv_handle_.HandleCPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// ルートシグネチャをセット
	commandList->SetGraphicsRootSignature(root_signature_->Get());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		Game::Get()->GetEngine()->GBufferHeap()->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetPipelineState(blur_horizontal_pso_->Get());		// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(0, srv_handle_[0].HandleGPU()); // ディスクリプタテーブルをセット

	commandList->DrawInstanced(4, 1, 0, 0);
}

void ShadowMap::BlurVertical()
{
	auto commandList = Game::Get()->GetEngine()->CommandList();

	// レンダーターゲットが使用可能になるまで待つ
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			color_[1].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			color_[0].Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		),
	};
	commandList->ResourceBarrier(_countof(barriers), barriers);

	// レンダーターゲットを設定
	auto rtvHandle = rtv_handle_[0].HandleCPU();
	auto dsvHandle = dsv_handle_.HandleCPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	commandList->SetPipelineState(blur_vertical_pso_->Get());		// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(0, srv_handle_[1].HandleGPU()); // ディスクリプタテーブルをセット

	commandList->DrawInstanced(4, 1, 0, 0);
}

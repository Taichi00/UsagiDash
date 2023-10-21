#include "ShadowMap.h"
#include "Engine.h"
#include "DescriptorHeap.h"

ShadowMap::ShadowMap()
{
	if (!CreateShadowBuffer())
	{
		printf("シャドウマップの生成に失敗\n");
		return;
	}
}

void ShadowMap::BeginRender()
{
	auto commandList = g_Engine->CommandList();

	// レンダーターゲットが使用可能になるまで待つ
	auto barrierToRT = CD3DX12_RESOURCE_BARRIER::Transition(
		m_pColor.Get(),
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	commandList->ResourceBarrier(1, &barrierToRT);

	// レンダーターゲットを設定
	auto rtvHandle = m_pRtvHandle->HandleCPU();
	auto dsvHandle = m_pDsvHandle->HandleCPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// レンダーターゲットをクリア
	const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// 深度ステンシルビューをクリア
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// ビューポートとシザー矩形を設定
	auto width = m_width;
	auto height = m_height;
	auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, float(width), float(height));
	auto scissor = CD3DX12_RECT(0, 0, LONG(width), LONG(height));
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);

}

void ShadowMap::EndRender()
{	
	auto commandList = g_Engine->CommandList();
	
	// バリア設定
	auto barrierToSR = CD3DX12_RESOURCE_BARRIER::Transition(
		m_pColor.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	commandList->ResourceBarrier(1, &barrierToSR);
}

ID3D12Resource* ShadowMap::Resource()
{
	return m_pColor.Get();
}

bool ShadowMap::CreateShadowBuffer()
{
	// リソースの生成
	auto colorDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		m_width, m_height,
		1, 1,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);

	auto depthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		m_width, m_height,
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

	auto device = g_Engine->Device();

	const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&colorDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearColor,
		IID_PPV_ARGS(m_pColor.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}

	hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearDepth,
		IID_PPV_ARGS(m_pDepth.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}

	// ディスクリプタの登録
	m_pRtvHandle = g_Engine->RtvHeap()->Alloc();
	m_pDsvHandle = g_Engine->DsvHeap()->Alloc();

	// ビューの生成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = colorDesc.Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(m_pColor.Get(), &rtvDesc, m_pRtvHandle->HandleCPU());

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = depthDesc.Format;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(m_pDepth.Get(), &dsvDesc, m_pDsvHandle->HandleCPU());

	return true;
}
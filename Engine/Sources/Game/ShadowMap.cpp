#include "ShadowMap.h"
#include "Engine.h"
#include "DescriptorHeap.h"
#include "RootSignature.h"
#include "PipelineState.h"

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

void ShadowMap::BeginRender()
{
	auto commandList = g_Engine->CommandList();

	// レンダーターゲットが使用可能になるまで待つ
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pColor[0].Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pColor[1].Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		),
	};
	commandList->ResourceBarrier(_countof(barriers), barriers);

	// レンダーターゲットを設定
	auto rtvHandle = m_pRtvHandle[0].HandleCPU();
	auto dsvHandle = m_pDsvHandle.HandleCPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// レンダーターゲットをクリア
	const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	commandList->ClearRenderTargetView(m_pRtvHandle[0].HandleCPU(), clearColor, 0, nullptr);
	commandList->ClearRenderTargetView(m_pRtvHandle[1].HandleCPU(), clearColor, 0, nullptr);

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
		m_pColor[0].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	commandList->ResourceBarrier(1, &barrierToSR);

	// ブラー
	BlurHorizontal();
	BlurVertical();

	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pColor[0].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		),
	};
	commandList->ResourceBarrier(_countof(barriers), barriers);
}

ID3D12Resource* ShadowMap::Resource()
{
	return m_pColor[0].Get();
}

DescriptorHandle* ShadowMap::SrvHandle()
{
	return &m_pSrvHandle[0];
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

	// Render Target の生成
	const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&colorDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearColor,
		IID_PPV_ARGS(m_pColor[0].ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}

	hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&colorDesc,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		&clearColor,
		IID_PPV_ARGS(m_pColor[1].ReleaseAndGetAddressOf())
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
		IID_PPV_ARGS(m_pDepth.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}

	// ディスクリプタの登録
	m_pRtvHandle[0] = g_Engine->RtvHeap()->Alloc();
	m_pRtvHandle[1] = g_Engine->RtvHeap()->Alloc();
	m_pDsvHandle = g_Engine->DsvHeap()->Alloc();
	m_pSrvHandle[0] = g_Engine->GBufferHeap()->Alloc();
	m_pSrvHandle[1] = g_Engine->GBufferHeap()->Alloc();

	// ビューの生成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = colorDesc.Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	device->CreateRenderTargetView(m_pColor[0].Get(), &rtvDesc, m_pRtvHandle[0].HandleCPU());
	device->CreateRenderTargetView(m_pColor[1].Get(), &rtvDesc, m_pRtvHandle[1].HandleCPU());

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = depthDesc.Format;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(m_pDepth.Get(), &dsvDesc, m_pDsvHandle.HandleCPU());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = colorDesc.Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	device->CreateShaderResourceView(m_pColor[0].Get(), &srvDesc, m_pSrvHandle[0].HandleCPU());
	device->CreateShaderResourceView(m_pColor[1].Get(), &srvDesc, m_pSrvHandle[1].HandleCPU());

	return true;
}

bool ShadowMap::PreparePSO()
{
	// BlurHorizontal用
	m_pBlurHorizontalPSO = new PipelineState();
	m_pBlurHorizontalPSO->SetInputLayout({ nullptr, 0 });
	m_pBlurHorizontalPSO->SetRootSignature(m_pRootSignature->Get());
	m_pBlurHorizontalPSO->SetVS(L"ScreenVS.cso");
	m_pBlurHorizontalPSO->SetPS(L"GaussianBlurHorizontal.cso");
	m_pBlurHorizontalPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	auto desc = m_pBlurHorizontalPSO->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	m_pBlurHorizontalPSO->Create();
	if (!m_pBlurHorizontalPSO->IsValid())
	{
		return false;
	}

	// BlurVertical用
	m_pBlurVerticalPSO = new PipelineState();
	m_pBlurVerticalPSO->SetInputLayout({ nullptr, 0 });
	m_pBlurVerticalPSO->SetRootSignature(m_pRootSignature->Get());
	m_pBlurVerticalPSO->SetVS(L"ScreenVS.cso");
	m_pBlurVerticalPSO->SetPS(L"GaussianBlurVertical.cso");
	m_pBlurVerticalPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = m_pBlurVerticalPSO->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	m_pBlurVerticalPSO->Create();
	if (!m_pBlurVerticalPSO->IsValid())
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

	m_pRootSignature = new RootSignature(_countof(params), params);
	if (!m_pRootSignature->IsValid())
	{
		return false;
	}

	return true;
}

void ShadowMap::BlurHorizontal()
{
	auto commandList = g_Engine->CommandList();

	// レンダーターゲットが使用可能になるまで待つ
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pColor[0].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		),
	};
	commandList->ResourceBarrier(_countof(barriers), barriers);

	// レンダーターゲットを設定
	auto rtvHandle = m_pRtvHandle[1].HandleCPU();
	auto dsvHandle = m_pDsvHandle.HandleCPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// ルートシグネチャをセット
	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		g_Engine->GBufferHeap()->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetPipelineState(m_pBlurHorizontalPSO->Get());		// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(0, m_pSrvHandle[0].HandleGPU()); // ディスクリプタテーブルをセット

	commandList->DrawInstanced(4, 1, 0, 0);
}

void ShadowMap::BlurVertical()
{
	auto commandList = g_Engine->CommandList();

	// レンダーターゲットが使用可能になるまで待つ
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pColor[1].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pColor[0].Get(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		),
	};
	commandList->ResourceBarrier(_countof(barriers), barriers);

	// レンダーターゲットを設定
	auto rtvHandle = m_pRtvHandle[0].HandleCPU();
	auto dsvHandle = m_pDsvHandle.HandleCPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	commandList->SetPipelineState(m_pBlurVerticalPSO->Get());		// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(0, m_pSrvHandle[1].HandleGPU()); // ディスクリプタテーブルをセット

	commandList->DrawInstanced(4, 1, 0, 0);
}

#include "Engine.h"
#include <d3d12.h>
#include <stdio.h>
#include "Window.h"
#include "PipelineState.h"
#include "DescriptorHeap.h"
#include "d3dx12.h"

Engine* g_Engine;

Engine::~Engine()
{

}

bool Engine::Init(Window* window)
{
	m_pWindow = window;
	m_FrameBufferWidth = window->Width();
	m_FrameBufferHeight = window->Height();
	m_hWnd = window->HWnd();

	if (!CreateDevice())
	{
		printf("デバイスの生成に失敗\n");
		return false;
	}

	if (!CreateCommandQueue())
	{
		printf("コマンドキューの生成に失敗\n");
		return false;
	}

	if (!CreateSwapChain())
	{
		printf("スワップチェインの生成に失敗\n");
		return false;
	}

	if (!CreateCommandList())
	{
		printf("コマンドリストの生成に失敗\n");
		return false;
	}

	if (!CreateFence())
	{
		printf("フェンスの生成に失敗\n");
		return false;
	}

	// ビューポートとシザー矩形を生成
	CreateViewPort();
	CreateScissorRect();

	if (!CreateRenderTarget())
	{
		printf("レンダーターゲットの生成に失敗\n");
		return false;
	}

	if (!CreateDepthStencil())
	{
		printf("デプスステンシルバッファの生成に失敗\n");
	}

	if (!CreateMSAA())
	{
		printf("MSAAリソースの生成に失敗\n");
	}

	printf("描画エンジンの初期化に成功\n");
	return true;
}

void Engine::BeginRender()
{
	// ビューポートとシザー矩形を設定
	m_pCommandList->RSSetViewports(1, &m_Viewport);
	m_pCommandList->RSSetScissorRects(1, &m_Scissor);

	// 現在のフレームのレンダーターゲットビューのディスクリプタヒープの開始アドレスを取得
	auto currentRtvHandle = m_pRtvHandles[m_CurrentBackBufferIndex]->HandleCPU();

	// 深度ステンシルのディスクリプタヒープの開始アドレス取得
	auto currentDsvHandle = m_pDsvHandle->HandleCPU();

	// レンダーターゲットが使用可能になるまで待つ
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_currentRenderTarget,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_pCommandList->ResourceBarrier(1, &barrier);

	// レンダーターゲットを設定
	m_pCommandList->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);

	// レンダーターゲットをクリア
	const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_pCommandList->ClearRenderTargetView(currentRtvHandle, clearColor, 0, nullptr);

	// 深度ステンシルビューをクリア
	m_pCommandList->ClearDepthStencilView(currentDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Engine::InitRender()
{
	// 現在のレンダーターゲットを更新
	m_currentRenderTarget = m_pRenderTargets[m_CurrentBackBufferIndex].Get();

	// コマンドを初期化してためる準備をする
	m_pAllocator[m_CurrentBackBufferIndex]->Reset();
	m_pCommandList->Reset(m_pAllocator[m_CurrentBackBufferIndex].Get(), nullptr);
}

void Engine::BeginRenderMSAA()
{
	// ビューポートとシザー矩形を設定
	m_pCommandList->RSSetViewports(1, &m_Viewport);
	m_pCommandList->RSSetScissorRects(1, &m_Scissor);

	// レンダーターゲットが使用可能になるまで待つ
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_pMSAAColorTarget.Get(),
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_pCommandList->ResourceBarrier(1, &barrier);

	// レンダーターゲットを設定
	auto rtvHandle = m_pMSAARtvHandle->HandleCPU();
	auto dsvHandle = m_pMSAADsvHandle->HandleCPU();
	m_pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// レンダーターゲットをクリア
	const float clearColor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	m_pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// 深度ステンシルビューをクリア
	m_pCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Engine::EndRender()
{
	// レンダーターゲットに書き込み終わるまで待つ
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_currentRenderTarget,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	m_pCommandList->ResourceBarrier(1, &barrier);

	// コマンドの記録を終了
	m_pCommandList->Close();

	// コマンドを実行
	ID3D12CommandList* ppCmdLists[] = { m_pCommandList.Get() };
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	// スワップチェーンを切り替え
	m_pSwapChain->Present(1, 0);

	// 描画完了を待つ
	WaitRender();

	// バックバッファ番号更新
	m_CurrentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

void Engine::EndRenderMSAA()
{
	// レンダーターゲットに書き込み終わるまで待つ
	auto barrierRenderToResolve = CD3DX12_RESOURCE_BARRIER::Transition(
		m_pMSAAColorTarget.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	m_pCommandList->ResourceBarrier(1, &barrierRenderToResolve);

	auto barrierPresentToResolve = CD3DX12_RESOURCE_BARRIER::Transition(
		m_currentRenderTarget,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RESOLVE_DEST
	);
	m_pCommandList->ResourceBarrier(1, &barrierPresentToResolve);

	// Resolve
	m_pCommandList->ResolveSubresource(
		m_currentRenderTarget, 0, m_pMSAAColorTarget.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM
	);

	// レンダーターゲットに書き込み終わるまで待つ
	auto barrierResolveToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		m_currentRenderTarget,
		D3D12_RESOURCE_STATE_RESOLVE_DEST,
		D3D12_RESOURCE_STATE_PRESENT
	);
	m_pCommandList->ResourceBarrier(1, &barrierResolveToPresent);

	// コマンドの記録を終了
	m_pCommandList->Close();

	// コマンドを実行
	ID3D12CommandList* ppCmdLists[] = { m_pCommandList.Get() };
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	// スワップチェーンを切り替え
	m_pSwapChain->Present(1, 0);

	// 描画完了を待つ
	WaitRender();

	// バックバッファ番号更新
	m_CurrentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

ID3D12Device6* Engine::Device()
{
	return m_pDevice.Get();
}

ID3D12GraphicsCommandList* Engine::CommandList()
{
	return m_pCommandList.Get();
}

ID3D12CommandAllocator* Engine::Allocator()
{
	return m_pAllocator[m_CurrentBackBufferIndex].Get();
}

ID3D12CommandQueue* Engine::Queue()
{
	return m_pQueue.Get();
}

DescriptorHeap* Engine::RtvHeap()
{
	return m_pRtvHeap.get();
}

DescriptorHeap* Engine::DsvHeap()
{
	return m_pDsvHeap.get();
}

UINT Engine::CurrentBackBufferIndex()
{
	return m_CurrentBackBufferIndex;
}

float Engine::AspectRate()
{
	return m_pWindow->AspectRate();
}

ID3D12Resource* Engine::UploadTexture(
	ID3D12Resource* textureData, std::vector<D3D12_SUBRESOURCE_DATA> subresources)
{
	ComPtr<ID3D12Resource> staging;

	const auto totalBytes = GetRequiredIntermediateSize(textureData, 0, UINT(subresources.size()));
	const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);
	
	auto hr = m_pDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&staging)
	);
	if (FAILED(hr))
	{
		return nullptr;
	}

	// 転送処理
	ComPtr<ID3D12GraphicsCommandList> command;
	hr = m_pDevice->CreateCommandList(
		0, 
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pAllocator[m_CurrentBackBufferIndex].Get(),
		nullptr, 
		IID_PPV_ARGS(&command)
	);
	if (FAILED(hr))
	{
		return nullptr;
	}
	
	UpdateSubresources(
		command.Get(),
		textureData, 
		staging.Get(),
		0, 
		0, 
		uint32_t(subresources.size()), subresources.data()
	);

	// リソースバリアのセット
	auto barrierTex = CD3DX12_RESOURCE_BARRIER::Transition(
		textureData,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	command->ResourceBarrier(1, &barrierTex);

	// コマンドの実行
	command->Close();
	ID3D12CommandList* cmds[] = { command.Get() };
	m_pQueue->ExecuteCommandLists(1, cmds);

	WaitRender();

	return staging.Get();
}

bool Engine::CreateDevice()
{
	auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_pDevice.ReleaseAndGetAddressOf()));
	return SUCCEEDED(hr);
}

bool Engine::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type		= D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask	= 0;

	auto hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pQueue.ReleaseAndGetAddressOf()));

	return SUCCEEDED(hr);
}

bool Engine::CreateSwapChain()
{
	// DXGIファクトリーの生成
	IDXGIFactory4* pFactory = nullptr;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	if (FAILED(hr))
	{
		return false;
	}

	// スワップチェインの生成
	DXGI_SWAP_CHAIN_DESC desc = {};
	desc.BufferDesc.Width					= m_FrameBufferWidth;
	desc.BufferDesc.Width					= m_FrameBufferWidth;
	desc.BufferDesc.Height					= m_FrameBufferHeight;
	desc.BufferDesc.RefreshRate.Numerator	= 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.ScanlineOrdering		= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;
	desc.BufferDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count					= 1;
	desc.SampleDesc.Quality					= 0;
	desc.BufferUsage						= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount						= FRAME_BUFFER_COUNT;
	desc.OutputWindow						= m_hWnd;
	desc.Windowed							= TRUE;
	desc.SwapEffect							= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.Flags								= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	IDXGISwapChain* pSwapChain = nullptr;
	hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, &pSwapChain);
	if (FAILED(hr))
	{
		pFactory->Release();
		return false;
	}

	// IDXGISwapChain3を取得
	hr = pSwapChain->QueryInterface(IID_PPV_ARGS(m_pSwapChain.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		pFactory->Release();
		pSwapChain->Release();
		return false;
	}

	// バックバッファ番号を取得
	m_CurrentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	pFactory->Release();
	pSwapChain->Release();
	return true;
}

bool Engine::CreateCommandList()
{
	// コマンドアロケーターの作成
	HRESULT hr;
	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		hr = m_pDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(m_pAllocator[i].ReleaseAndGetAddressOf())
		);
	}

	if (FAILED(hr))
	{
		return false;
	}

	// コマンドリストの生成
	hr = m_pDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pAllocator[m_CurrentBackBufferIndex].Get(),
		nullptr,
		IID_PPV_ARGS(&m_pCommandList)
	);

	if (FAILED(hr))
	{
		return false;
	}

	// コマンドリストは開かれている状態で作成されるので、いったん閉じる
	m_pCommandList->Close();

	return true;
}

bool Engine::CreateFence()
{
	for (auto i = 0u; i < FRAME_BUFFER_COUNT; i++)
	{
		m_fenceValue[i] = 0;
	}

	auto hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pFence.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}

	m_fenceValue[m_CurrentBackBufferIndex]++;

	// 同期を行うときのイベントハンドラを作成する
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	return m_fenceEvent != nullptr;
}

void Engine::CreateViewPort()
{
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width	= static_cast<float>(m_FrameBufferWidth);
	m_Viewport.Height	= static_cast<float>(m_FrameBufferHeight);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;
}

void Engine::CreateScissorRect()
{
	m_Scissor.left = 0;
	m_Scissor.right = m_FrameBufferWidth;
	m_Scissor.top = 0;
	m_Scissor.bottom = m_FrameBufferHeight;
}

bool Engine::CreateRenderTarget()
{
	// RTV用のディスクリプタヒープを作成する
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors	= FRAME_BUFFER_COUNT + 2;
	desc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	m_pRtvHeap = std::make_shared<DescriptorHeap>(desc);
	if (!m_pRtvHeap->IsValid())
	{
		return false;
	}

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		m_pRtvHandles[i] = std::make_shared<DescriptorHandle>(m_pRtvHeap->Alloc());
		m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pRenderTargets[i].ReleaseAndGetAddressOf()));
		m_pDevice->CreateRenderTargetView(m_pRenderTargets[i].Get(), &rtvDesc, m_pRtvHandles[i]->HandleCPU());
	}

	return true;
}

bool Engine::CreateDepthStencil()
{
	// DSV用のディスクリプタヒープを作成する
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors	= 3;
	desc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	
	m_pDsvHeap = std::make_shared<DescriptorHeap>(desc);
	if (!m_pRtvHeap->IsValid())
	{
		return false;
	}

	D3D12_CLEAR_VALUE dsvClearValue;
	dsvClearValue.Format				= DXGI_FORMAT_D32_FLOAT;
	dsvClearValue.DepthStencil.Depth	= 1.0f;
	dsvClearValue.DepthStencil.Stencil	= 0;

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		m_FrameBufferWidth,
		m_FrameBufferHeight,
		1,
		1,
		DXGI_FORMAT_D32_FLOAT,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
	);

	auto hr = m_pDevice->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&dsvClearValue,
		IID_PPV_ARGS(m_pDepthStencilBuffer.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}

	// ディスクリプタを作成
	m_pDsvHandle = std::make_shared<DescriptorHandle>(m_pDsvHeap->Alloc());
	m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, m_pDsvHandle->HandleCPU());

	return true;
}

bool Engine::CreateMSAA()
{
	auto format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	auto width = m_FrameBufferWidth;
	auto height = m_FrameBufferHeight;
	m_sampleCount = 4;

	// MSAA 描画先バッファ（カラー）の準備
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels{};
	levels.Format = format;
	levels.SampleCount = m_sampleCount;

	D3D12_CLEAR_VALUE clearColor{};
	clearColor.Format = format;

	auto msaaColorDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		format,
		width,
		height,
		1,
		1,
		levels.SampleCount
	);
	msaaColorDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto hr = m_pDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&msaaColorDesc,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		&clearColor,
		IID_PPV_ARGS(m_pMSAAColorTarget.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}

	// MSAA 描画先バッファ（デプス）の準備
	auto msaaDepthDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		DXGI_FORMAT_D32_FLOAT,
		width,
		height,
		1,
		1,
		m_sampleCount
	);
	msaaDepthDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearDepth{};
	clearDepth.Format = msaaDepthDesc.Format;
	clearDepth.DepthStencil.Depth = 1.0f;

	hr = m_pDevice->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&msaaDepthDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearDepth,
		IID_PPV_ARGS(m_pMSAADepthTarget.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}

	// MSAA に出力するためのビューを準備する
	D3D12_RENDER_TARGET_VIEW_DESC msaaRtvDesc{};
	msaaRtvDesc.Format = format;
	msaaRtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

	D3D12_DEPTH_STENCIL_VIEW_DESC msaaDsvDesc{};
	msaaDsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	msaaDsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;

	m_pMSAARtvHandle = std::make_shared<DescriptorHandle>(m_pRtvHeap->Alloc());
	m_pMSAADsvHandle = std::make_shared<DescriptorHandle>(m_pDsvHeap->Alloc());

	m_pDevice->CreateRenderTargetView(m_pMSAAColorTarget.Get(), &msaaRtvDesc, m_pMSAARtvHandle->HandleCPU());
	m_pDevice->CreateDepthStencilView(m_pMSAADepthTarget.Get(), &msaaDsvDesc, m_pMSAADsvHandle->HandleCPU());

	return true;
}

void Engine::WaitRender()
{
	// 描画終了待ち
	const UINT64 fenceValue = m_fenceValue[m_CurrentBackBufferIndex];
	m_pQueue->Signal(m_pFence.Get(), fenceValue);
	m_fenceValue[m_CurrentBackBufferIndex]++;

	// 次のフレームの描画準備がまだであれば待機する
	if (m_pFence->GetCompletedValue() < fenceValue)
	{
		// 完了時にイベントを設定
		auto hr = m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		if (FAILED(hr))
		{
			return;
		}

		// 待機処理
		if (WAIT_OBJECT_0 != WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE))
		{
			return;
		}
	}
}



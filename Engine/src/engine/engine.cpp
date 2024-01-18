#include "engine/d3dx12.h"
#include "engine.h"
#include "engine/descriptor_heap.h"
#include "engine/engine2d.h"
#include "engine/buffer.h"
#include "engine/buffer_manager.h"
#include "engine/pipeline_state.h"
#include "engine/window.h"
#include "engine/vertex_buffer.h"
#include "engine/index_buffer.h"
#include "engine/constant_buffer.h"
#include "game/shadow_map.h"
#include <d3d12.h>
#include <stdexcept>
#include <stdio.h>

//std::unique_ptr<Engine> g_Engine;

Engine::Engine()
{
}

Engine::~Engine()
{
	printf("Delete Engine\n");
	shadowmap_.release();
	debug_device_->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
}

bool Engine::Init(std::shared_ptr<Window> window)
{
	window_ = window;
	frame_buffer_width_ = window->Width();
	frame_buffer_height_ = window->Height();
	hwnd_ = window->HWnd();

#if _DEBUG
	EnableDebugLayer();
#endif

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

	if (!CreateDescriptorHeap())
	{
		printf("ディスクリプタヒープの生成に失敗\n");
	}

	if (!CreateRenderTarget())
	{
		printf("レンダーターゲットの生成に失敗\n");
		return false;
	}

	if (!CreateDepthStencil())
	{
		printf("デプスステンシルバッファの生成に失敗\n");
	}

	//if (!CreateMSAA())
	//{
	//	printf("MSAAリソースの生成に失敗\n");
	//}

	// GBufferManagerの生成
	buffer_manager_ = std::make_unique<BufferManager>();

	if (!CreateGBuffer())
	{
		printf("G-Bufferの生成に失敗\n");
	}

	// 2D描画エンジンの初期化
	engine2d_ = std::make_unique<Engine2D>();
	if (!engine2d_->Init())
	{
		return false;
	}

	// シャドウマップの生成
	shadowmap_ = std::make_unique<ShadowMap>();

	printf("描画エンジンの初期化に成功\n");
	return true;
}

void Engine::BeginRender()
{
	// ビューポートとシザー矩形を設定
	command_list_->RSSetViewports(1, &viewport_);
	command_list_->RSSetScissorRects(1, &scissor_);

	// 現在のフレームのレンダーターゲットビューのディスクリプタヒープの開始アドレスを取得
	auto currentRtvHandle = rtv_handles_[current_back_buffer_index_].HandleCPU();

	// 深度ステンシルのディスクリプタヒープの開始アドレス取得
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// レンダーターゲットが使用可能になるまで待つ
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		current_render_target_,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	command_list_->ResourceBarrier(1, &barrier);

	// レンダーターゲットを設定
	command_list_->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);

	// レンダーターゲットをクリア
	const float clearColor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	command_list_->ClearRenderTargetView(currentRtvHandle, clearColor, 0, nullptr);

	// 深度ステンシルビューをクリア
	command_list_->ClearDepthStencilView(currentDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Engine::InitRender()
{
	// 現在のレンダーターゲットを更新
	current_render_target_ = render_targets_[current_back_buffer_index_].Get();

	// コマンドを初期化してためる準備をする
	allocator_[current_back_buffer_index_]->Reset();
	command_list_->Reset(allocator_[current_back_buffer_index_].Get(), nullptr);
}

void Engine::BeginRenderMSAA()
{
	// ビューポートとシザー矩形を設定
	command_list_->RSSetViewports(1, &viewport_);
	command_list_->RSSetScissorRects(1, &scissor_);

	// レンダーターゲットが使用可能になるまで待つ
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		msaa_color_target_.Get(),
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	command_list_->ResourceBarrier(1, &barrier);

	// レンダーターゲットを設定
	auto rtvHandle = msaa_rtv_handle_.HandleCPU();
	auto dsvHandle = msaa_dsv_handle_.HandleCPU();
	command_list_->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// レンダーターゲットをクリア
	const float clearColor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	command_list_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// 深度ステンシルビューをクリア
	command_list_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Engine::EndRender()
{
	// レンダーターゲットに書き込み終わるまで待つ
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		current_render_target_,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	command_list_->ResourceBarrier(1, &barrier);

	// コマンドの記録を終了
	command_list_->Close();

	// コマンドを実行
	ID3D12CommandList* ppCmdLists[] = { command_list_.Get() };
	queue_->ExecuteCommandLists(1, ppCmdLists);

	// スワップチェーンを切り替え
	swapchain_->Present(1, 0);

	// 描画完了を待つ
	WaitGPU();

	// バックバッファ番号更新
	current_back_buffer_index_ = swapchain_->GetCurrentBackBufferIndex();
}

void Engine::EndRenderMSAA()
{
	// レンダーターゲットに書き込み終わるまで待つ
	auto barrierRenderToResolve = CD3DX12_RESOURCE_BARRIER::Transition(
		msaa_color_target_.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
	command_list_->ResourceBarrier(1, &barrierRenderToResolve);

	auto barrierPresentToResolve = CD3DX12_RESOURCE_BARRIER::Transition(
		current_render_target_,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RESOLVE_DEST
	);
	command_list_->ResourceBarrier(1, &barrierPresentToResolve);

	// Resolve
	command_list_->ResolveSubresource(
		current_render_target_, 0, msaa_color_target_.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM
	);

	// レンダーターゲットに書き込み終わるまで待つ
	auto barrierResolveToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		current_render_target_,
		D3D12_RESOURCE_STATE_RESOLVE_DEST,
		D3D12_RESOURCE_STATE_PRESENT
	);
	command_list_->ResourceBarrier(1, &barrierResolveToPresent);

	// コマンドの記録を終了
	command_list_->Close();

	// コマンドを実行
	ID3D12CommandList* ppCmdLists[] = { command_list_.Get() };
	queue_->ExecuteCommandLists(1, ppCmdLists);

	// スワップチェーンを切り替え
	swapchain_->Present(1, 0);

	// 描画完了を待つ
	WaitGPU();

	// バックバッファ番号更新
	current_back_buffer_index_ = swapchain_->GetCurrentBackBufferIndex();
}

void Engine::EndRenderD3D()
{
	D3D12_RESOURCE_BARRIER barriers[] = {
		/*CD3DX12_RESOURCE_BARRIER::Transition(
			current_render_target_,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT),*/
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("Position")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("Normal")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("Albedo")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("MetallicRoughness")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("Depth")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("Lighting")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		/*CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("SSAO")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("BlurredSSAO1")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("BlurredSSAO2")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),*/
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("PostProcess")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("ShadowMapColor_0")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("ShadowMapColor_1")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
	};
	command_list_->ResourceBarrier(_countof(barriers), barriers);

	// コマンドの記録を終了
	command_list_->Close();

	// コマンドを実行
	ID3D12CommandList* ppCmdLists[] = { command_list_.Get() };
	queue_->ExecuteCommandLists(1, ppCmdLists);

	// 描画完了を待つ
	WaitGPU();
}

void Engine::BeginDeferredRender()
{
	// ビューポートとシザー矩形を設定
	command_list_->RSSetViewports(1, &viewport_);
	command_list_->RSSetScissorRects(1, &scissor_);

	// 現在のフレームのレンダーターゲットビューのディスクリプタヒープの開始アドレスを取得
	auto currentRtvHandle = rtv_handles_[current_back_buffer_index_].HandleCPU();

	// 深度ステンシルのディスクリプタヒープの開始アドレス取得
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// レンダーターゲットが使用可能になるまで待つ
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			current_render_target_,
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
	};
	command_list_->ResourceBarrier(_countof(barriers), barriers);

	// レンダーターゲットをクリア
	const float clearColor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	const float zeroFloat[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	const float oneFloat[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const float zeroAlbedo[] = { 0, 0, 0, 1 };	// Aはアウトラインマスクなので１で初期化
	command_list_->ClearRenderTargetView(currentRtvHandle, clearColor, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("Position")->RtvHandle().HandleCPU(), zeroFloat, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("Normal")->RtvHandle().HandleCPU(), zeroFloat, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("Albedo")->RtvHandle().HandleCPU(), zeroAlbedo, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("MetallicRoughness")->RtvHandle().HandleCPU(), zeroFloat, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("Depth")->RtvHandle().HandleCPU(), oneFloat, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("Lighting")->RtvHandle().HandleCPU(), zeroFloat, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("PostProcess")->RtvHandle().HandleCPU(), zeroFloat, 0, nullptr);

	// 深度ステンシルビューをクリア
	command_list_->ClearDepthStencilView(currentDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Engine::DepthPrePath()
{
	// 現在のフレームのレンダーターゲットビューのディスクリプタヒープの開始アドレスを取得
	auto currentRtvHandle = rtv_handles_[current_back_buffer_index_].HandleCPU();

	// 深度ステンシルのディスクリプタヒープの開始アドレス取得
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// レンダーターゲットを設定
	command_list_->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::GBufferPath()
{
	// レンダーターゲットを設定
	D3D12_CPU_DESCRIPTOR_HANDLE handleRtvs[] = {
		buffer_manager_->Get("Position")->RtvHandle().HandleCPU(),
		buffer_manager_->Get("Normal")->RtvHandle().HandleCPU(),
		buffer_manager_->Get("Albedo")->RtvHandle().HandleCPU(),
		buffer_manager_->Get("MetallicRoughness")->RtvHandle().HandleCPU(),
		buffer_manager_->Get("Depth")->RtvHandle().HandleCPU()
	};
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	command_list_->OMSetRenderTargets(_countof(handleRtvs), handleRtvs, FALSE, &currentDsvHandle);
}

void Engine::LightingPath()
{
	// レンダーターゲットに書き込み終わるまで待つ
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("Position")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("Normal")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("Albedo")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("MetallicRoughness")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("Depth")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
	};
	command_list_->ResourceBarrier(_countof(barriers), barriers);

	// 現在のフレームのレンダーターゲットビューのディスクリプタヒープの開始アドレスを取得
	auto currentRtvHandle = buffer_manager_->Get("Lighting")->RtvHandle().HandleCPU();

	// 深度ステンシルのディスクリプタヒープの開始アドレス取得
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// レンダーターゲットを設定
	command_list_->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::SSAOPath()
{
	// 現在のフレームのレンダーターゲットビューのディスクリプタヒープの開始アドレスを取得
	auto currentRtvHandle = buffer_manager_->Get("SSAO")->RtvHandle().HandleCPU();

	// 深度ステンシルのディスクリプタヒープの開始アドレス取得
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// レンダーターゲットを設定
	command_list_->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::BlurHorizontalPath()
{
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("SSAO")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
	};
	command_list_->ResourceBarrier(_countof(barriers), barriers);

	// 現在のフレームのレンダーターゲットビューのディスクリプタヒープの開始アドレスを取得
	auto currentRtvHandle = buffer_manager_->Get("BlurredSSAO1")->RtvHandle().HandleCPU();

	// 深度ステンシルのディスクリプタヒープの開始アドレス取得
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// レンダーターゲットを設定
	command_list_->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::BlurVerticalPath()
{
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("BlurredSSAO1")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
	};
	command_list_->ResourceBarrier(_countof(barriers), barriers);

	// 現在のフレームのレンダーターゲットビューのディスクリプタヒープの開始アドレスを取得
	auto currentRtvHandle = buffer_manager_->Get("BlurredSSAO2")->RtvHandle().HandleCPU();

	// 深度ステンシルのディスクリプタヒープの開始アドレス取得
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// レンダーターゲットを設定
	command_list_->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::PostProcessPath()
{
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("Lighting")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		/*CD3DX12_RESOURCE_BARRIER::Transition(
			buffer_manager_->Get("BlurredSSAO2")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),*/
	};
	command_list_->ResourceBarrier(_countof(barriers), barriers);
	
	// 現在のフレームのレンダーターゲットビューのディスクリプタヒープの開始アドレスを取得
	auto currentRtvHandle = buffer_manager_->Get("PostProcess")->RtvHandle().HandleCPU();

	// 深度ステンシルのディスクリプタヒープの開始アドレス取得
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// レンダーターゲットを設定
	command_list_->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::FXAAPath()
{
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			 buffer_manager_->Get("PostProcess")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
	};
	command_list_->ResourceBarrier(_countof(barriers), barriers);

	// 現在のフレームのレンダーターゲットビューのディスクリプタヒープの開始アドレスを取得
	auto currentRtvHandle = rtv_handles_[current_back_buffer_index_].HandleCPU();

	// 深度ステンシルのディスクリプタヒープの開始アドレス取得
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// レンダーターゲットを設定
	command_list_->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::EndDeferredRender()
{
	//// コマンドの記録を終了
	//command_list_->Close();

	//// コマンドを実行
	//ID3D12CommandList* ppCmdLists[] = { command_list_.Get() };
	//queue_->ExecuteCommandLists(1, ppCmdLists);

	// スワップチェーンを切り替え
	swapchain_->Present(1, 0);

	// 描画完了を待つ
	WaitGPU();

	// バックバッファ番号更新
	current_back_buffer_index_ = swapchain_->GetCurrentBackBufferIndex();
}

ID3D12Device6* Engine::Device()
{
	return device_.Get();
}

ID3D12GraphicsCommandList* Engine::CommandList()
{
	return command_list_.Get();
}

ID3D12CommandAllocator* Engine::Allocator()
{
	return allocator_[current_back_buffer_index_].Get();
}

ID3D12CommandQueue* Engine::Queue()
{
	return queue_.Get();
}

ID3D12CommandQueue** Engine::QueueAddress()
{
	return queue_.GetAddressOf();
}

ID3D12Resource* Engine::RenderTarget(int index)
{
	return render_targets_[index].Get();
}

std::shared_ptr<DescriptorHeap> Engine::RtvHeap()
{
	return rtv_heap_;
}

std::shared_ptr<DescriptorHeap> Engine::DsvHeap()
{
	return dsv_heap_;
}
std::shared_ptr<DescriptorHeap> Engine::SrvHeap()
{
	return srv_heap_;
}

UINT Engine::CurrentBackBufferIndex()
{
	return current_back_buffer_index_;
}

std::shared_ptr<Window> Engine::GetWindow()
{
	return window_;
}

UINT Engine::FrameBufferWidth()
{
	return frame_buffer_width_;
}

UINT Engine::FrameBufferHeight()
{
	return frame_buffer_height_;
}

Engine2D* Engine::GetEngine2D()
{
	return engine2d_.get();
}

ID3D12DebugDevice* Engine::DebugDevice()
{
	return debug_device_.Get();
}

float Engine::AspectRate()
{
	return window_->AspectRate();
}

bool Engine::UploadTexture(
	ID3D12Resource* textureData, std::vector<D3D12_SUBRESOURCE_DATA> subresources)
{
	ComPtr<ID3D12Resource> staging;

	const auto totalBytes = GetRequiredIntermediateSize(textureData, 0, UINT(subresources.size()));
	const auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	const auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(totalBytes);
	
	auto hr = device_->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&staging)
	);
	if (FAILED(hr))
	{
		return false;
	}

	// 転送処理
	ComPtr<ID3D12GraphicsCommandList> command;
	hr = device_->CreateCommandList(
		0, 
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		oneshot_allocator_.Get(),
		nullptr, 
		IID_PPV_ARGS(&command)
	);
	if (FAILED(hr))
	{
		return false;
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
	queue_->ExecuteCommandLists(1, cmds);

	WaitGPU();

	oneshot_allocator_->Reset();

	return true;
}

std::unique_ptr<VertexBuffer> Engine::CreateVertexBuffer(const std::vector<Vertex>& vertices)
{
	auto size = sizeof(Vertex) * vertices.size();
	auto stride = sizeof(Vertex);
	auto data = vertices.data();

	auto vb = std::make_unique<VertexBuffer>(size, stride, data);
	if (!vb->IsValid())
	{
		printf("頂点バッファの生成に失敗\n");
		return nullptr;
	}

	return std::move(vb);
}

std::unique_ptr<IndexBuffer> Engine::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
	auto size = sizeof(uint32_t) * indices.size();
	auto data = indices.data();

	auto ib = std::make_unique<IndexBuffer>(size, data);
	if (!ib->IsValid())
	{
		printf("インデックスバッファの生成に失敗\n");
		return nullptr;
	}

	return std::move(ib);
}

std::unique_ptr<ConstantBuffer> Engine::CreateConstantBuffer(const unsigned long long size)
{
	auto cb = std::make_unique<ConstantBuffer>(size);
	if (!cb->IsValid())
	{
		printf("定数バッファの生成に失敗\n");
		return nullptr;
	}

	return std::move(cb);
}

void Engine::CreateShaderResourceView(const Texture2D& texture, const DescriptorHandle& handle)
{
	auto resource = texture.Resource();
	auto desc = texture.ViewDesc();

	device_->CreateShaderResourceView(resource, &desc, handle.HandleCPU());
}

std::unique_ptr<DescriptorHeap> Engine::CreateDescriptorHeap(const unsigned int num)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NodeMask = 0; // どのGPU向けのディスクリプタヒープかを指定（GPU１つの場合は０）
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = num;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	auto dh = std::make_unique<DescriptorHeap>(desc);
	if (!dh->IsValid())
	{
		printf("ディスクリプタヒープの生成に失敗\n");
		return nullptr;
	}

	return std::move(dh);
}

void Engine::ResizeWindow(const unsigned int width, const unsigned int height)
{
	FlushGPU();

	for (auto i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		render_targets_[i].Reset();
		rtv_heap_->Free(rtv_handles_[i]);
	}

	depth_stencil_buffer_.Reset();
	dsv_heap_->Free(dsv_handle_);

	auto bm = buffer_manager_.get();
	bm->Get("Position"         )->Reset();
	bm->Get("Normal"           )->Reset();
	bm->Get("Albedo"           )->Reset();
	bm->Get("MetallicRoughness")->Reset();
	bm->Get("Depth"            )->Reset();
	bm->Get("Lighting"         )->Reset();
	bm->Get("SSAO"             )->Reset();
	bm->Get("BlurredSSAO1"     )->Reset();
	bm->Get("BlurredSSAO2"     )->Reset();
	bm->Get("PostProcess"      )->Reset();

	engine2d_->ResetRenderTargets();
	
	DXGI_SWAP_CHAIN_DESC desc = {};
	swapchain_->GetDesc(&desc);

	auto hr = swapchain_->ResizeBuffers(desc.BufferCount, width, height, desc.BufferDesc.Format, desc.Flags);
	if (FAILED(hr))
	{
		throw std::runtime_error("バックバッファのサイズ変更に失敗");
	}
	
	frame_buffer_width_ = width;
	frame_buffer_height_ = height;

	CreateViewPort();
	CreateScissorRect();

	CreateRenderTarget();
	CreateDepthStencil();
	CreateGBuffer();

	engine2d_->CreateD2DRenderTarget();

	current_back_buffer_index_ = swapchain_->GetCurrentBackBufferIndex();
}

bool Engine::EnableDebugLayer()
{
	// デバッグレイヤーを有効化
	ComPtr<ID3D12Debug> pDebug;
	auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug));
	if (FAILED(hr))
	{
		return false;
	}

	pDebug->EnableDebugLayer();

	return true;
}

bool Engine::CreateDevice()
{
	auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device_.ReleaseAndGetAddressOf()));

	device_.As(&debug_device_);

	ComPtr<ID3D12InfoQueue> pInfoQueue;
	device_.As(&pInfoQueue);
	pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);

	return SUCCEEDED(hr);
}

bool Engine::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type		= D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority	= D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags		= D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask	= 0;

	auto hr = device_->CreateCommandQueue(&desc, IID_PPV_ARGS(queue_.ReleaseAndGetAddressOf()));

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
	desc.BufferDesc.Width					= frame_buffer_width_;
	desc.BufferDesc.Height					= frame_buffer_height_;
	desc.BufferDesc.RefreshRate.Numerator	= 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.ScanlineOrdering		= DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling					= DXGI_MODE_SCALING_UNSPECIFIED;
	desc.BufferDesc.Format					= DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count					= 1;
	desc.SampleDesc.Quality					= 0;
	desc.BufferUsage						= DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount						= FRAME_BUFFER_COUNT;
	desc.OutputWindow						= hwnd_;
	desc.Windowed							= TRUE;
	desc.SwapEffect							= DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.Flags								= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	IDXGISwapChain* pSwapChain = nullptr;
	hr = pFactory->CreateSwapChain(queue_.Get(), &desc, &pSwapChain);
	if (FAILED(hr))
	{
		pFactory->Release();
		return false;
	}

	// IDXGISwapChain3を取得
	hr = pSwapChain->QueryInterface(IID_PPV_ARGS(swapchain_.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		pFactory->Release();
		pSwapChain->Release();
		return false;
	}


	// Alt-Enterでフルスクリーンを防止
	pFactory->MakeWindowAssociation(hwnd_, DXGI_MWA_NO_ALT_ENTER);

	// バックバッファ番号を取得
	current_back_buffer_index_ = swapchain_->GetCurrentBackBufferIndex();

	//swapchain_->SetFullscreenState(TRUE, NULL);
	
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
		hr = device_->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(allocator_[i].ReleaseAndGetAddressOf())
		);
	}

	if (FAILED(hr))
	{
		return false;
	}

	hr = device_->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&oneshot_allocator_)
	);

	if (FAILED(hr))
	{
		return false;
	}

	// コマンドリストの生成
	hr = device_->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		allocator_[current_back_buffer_index_].Get(),
		nullptr,
		IID_PPV_ARGS(&command_list_)
	);

	if (FAILED(hr))
	{
		return false;
	}

	// コマンドリストは開かれている状態で作成されるので、いったん閉じる
	command_list_->Close();

	return true;
}

bool Engine::CreateFence()
{
	for (auto i = 0u; i < FRAME_BUFFER_COUNT; i++)
	{
		fence_value_[i] = 0;
	}

	auto hr = device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence_.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}

	fence_value_[current_back_buffer_index_]++;

	// 同期を行うときのイベントハンドラを作成する
	fence_event_ = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	return fence_event_ != nullptr;
}

void Engine::CreateViewPort()
{
	viewport_.TopLeftX = 0;
	viewport_.TopLeftY = 0;
	viewport_.Width	= static_cast<float>(frame_buffer_width_);
	viewport_.Height	= static_cast<float>(frame_buffer_height_);
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;
}

void Engine::CreateScissorRect()
{
	scissor_.left = 0;
	scissor_.right = frame_buffer_width_;
	scissor_.top = 0;
	scissor_.bottom = frame_buffer_height_;
}

bool Engine::CreateRenderTarget()
{
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		rtv_handles_[i] = rtv_heap_->Alloc();
		swapchain_->GetBuffer(i, IID_PPV_ARGS(render_targets_[i].ReleaseAndGetAddressOf()));
		device_->CreateRenderTargetView(render_targets_[i].Get(), &rtvDesc, rtv_handles_[i].HandleCPU());
	}

	return true;
}

bool Engine::CreateDepthStencil()
{
	D3D12_CLEAR_VALUE dsvClearValue;
	dsvClearValue.Format				= DXGI_FORMAT_D32_FLOAT;
	dsvClearValue.DepthStencil.Depth	= 1.0f;
	dsvClearValue.DepthStencil.Stencil	= 0;

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		frame_buffer_width_,
		frame_buffer_height_,
		1,
		1,
		DXGI_FORMAT_D32_FLOAT,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE
	);

	auto hr = device_->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&dsvClearValue,
		IID_PPV_ARGS(depth_stencil_buffer_.ReleaseAndGetAddressOf())
	);
	if (FAILED(hr))
	{
		return false;
	}

	// ディスクリプタを作成
	dsv_handle_ = dsv_heap_->Alloc();
	device_->CreateDepthStencilView(depth_stencil_buffer_.Get(), nullptr, dsv_handle_.HandleCPU());

	return true;
}

bool Engine::CreateMSAA()
{
	auto format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	auto width = frame_buffer_width_;
	auto height = frame_buffer_height_;
	sample_count_ = 4;

	// MSAA 描画先バッファ（カラー）の準備
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS levels{};
	levels.Format = format;
	levels.SampleCount = sample_count_;

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
	auto hr = device_->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&msaaColorDesc,
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		&clearColor,
		IID_PPV_ARGS(msaa_color_target_.ReleaseAndGetAddressOf())
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
		sample_count_
	);
	msaaDepthDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearDepth{};
	clearDepth.Format = msaaDepthDesc.Format;
	clearDepth.DepthStencil.Depth = 1.0f;

	hr = device_->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&msaaDepthDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearDepth,
		IID_PPV_ARGS(msaa_depth_target_.ReleaseAndGetAddressOf())
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

	msaa_rtv_handle_ = rtv_heap_->Alloc();
	msaa_dsv_handle_ = dsv_heap_->Alloc();

	device_->CreateRenderTargetView(msaa_color_target_.Get(), &msaaRtvDesc, msaa_rtv_handle_.HandleCPU());
	device_->CreateDepthStencilView(msaa_depth_target_.Get(), &msaaDsvDesc, msaa_dsv_handle_.HandleCPU());

	return true;
}

bool Engine::CreateGBuffer()
{
	// GBufferの生成
	auto bm = buffer_manager_.get();

	auto color_format = DXGI_FORMAT_R8G8B8A8_UNORM;
	auto float_format = DXGI_FORMAT_R32G32B32A32_FLOAT;

	bm->CreateGBuffer("Position"         , float_format, { 0, 0, 0, 0 });
	bm->CreateGBuffer("Normal"           , float_format, { 0, 0, 0, 0 });
	bm->CreateGBuffer("Albedo"           , color_format, { 0, 0, 0, 1 });
	bm->CreateGBuffer("MetallicRoughness", color_format, { 0, 0, 0, 0 });
	bm->CreateGBuffer("Depth"            , float_format, { 1, 1, 1, 1 });
	bm->CreateGBuffer("Lighting"         , color_format, { 0, 0, 0, 0 });
	bm->CreateGBuffer("SSAO"             , color_format, { 0, 0, 0, 0 });
	bm->CreateGBuffer("BlurredSSAO1"     , color_format, { 0, 0, 0, 0 });
	bm->CreateGBuffer("BlurredSSAO2"     , color_format, { 0, 0, 0, 0 });
	bm->CreateGBuffer("PostProcess"      , color_format, { 0, 0, 0, 0 });

	//Buffer::BufferProperty propRGBA{
	//	DXGI_FORMAT_R8G8B8A8_UNORM,
	//	frame_buffer_width_, frame_buffer_height_,
	//	srv_heap_.get(), rtv_heap_.get()
	//};

	//Buffer::BufferProperty propFloat{
	//	DXGI_FORMAT_R32G32B32A32_FLOAT,
	//	frame_buffer_width_, frame_buffer_height_,
	//	srv_heap_.get(), rtv_heap_.get()
	//};

	//// GBufferの作成
	//buffer_manager_->CreateGBuffer("Position", propFloat);
	//buffer_manager_->CreateGBuffer("Normal", propFloat);
	//buffer_manager_->CreateGBuffer("Albedo", propRGBA);
	//buffer_manager_->CreateGBuffer("MetallicRoughness", propRGBA);
	//buffer_manager_->CreateGBuffer("Depth", propFloat);
	//buffer_manager_->CreateGBuffer("Lighting", propRGBA);
	//buffer_manager_->CreateGBuffer("SSAO", propFloat);
	//buffer_manager_->CreateGBuffer("BlurredSSAO1", propFloat);
	//buffer_manager_->CreateGBuffer("BlurredSSAO2", propFloat);
	//buffer_manager_->CreateGBuffer("PostProcess", propRGBA);

	return true;
}

bool Engine::CreateDescriptorHeap()
{
	// RTV用のディスクリプタヒープを作成する
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 32; //FRAME_BUFFER_COUNT + 2;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		rtv_heap_ = std::make_unique<DescriptorHeap>(desc);
		if (!rtv_heap_->IsValid())
		{
			return false;
		}
	}

	// DSV用のディスクリプタヒープを作成する
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = 32;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		dsv_heap_ = std::make_unique<DescriptorHeap>(desc);
		if (!dsv_heap_->IsValid())
		{
			return false;
		}
	}

	// SRV用のディスクリプタヒープを作成
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
		srvHeapDesc.NodeMask = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.NumDescriptors = 32; //6 + 1 + 1;	// GBuffer * 6 + ShadowMap + Skybox
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		srv_heap_ = std::make_unique<DescriptorHeap>(srvHeapDesc);
		if (!srv_heap_->IsValid())
			return false;
	}

	return true;
}

void Engine::WaitGPU()
{
	// 描画終了待ち
	const UINT64 fenceValue = fence_value_[current_back_buffer_index_];
	queue_->Signal(fence_.Get(), fenceValue);
	fence_value_[current_back_buffer_index_]++;

	// 次のフレームの描画準備がまだであれば待機する
	if (fence_->GetCompletedValue() < fenceValue)
	{
		// 完了時にイベントを設定
		auto hr = fence_->SetEventOnCompletion(fenceValue, fence_event_);
		if (FAILED(hr))
		{
			return;
		}

		// 待機処理
		if (WAIT_OBJECT_0 != WaitForSingleObjectEx(fence_event_, INFINITE, FALSE))
		{
			return;
		}
	}
}

void Engine::FlushGPU()
{
	//for (auto i = 0; i < FRAME_BUFFER_COUNT; i++)
	//{
	//	const UINT64 fenceValue = fence_value_[i];
	//	queue_->Signal(fence_.Get(), fenceValue);
	//	fence_value_[i]++;

	//	// 次のフレームの描画準備がまだであれば待機する
	//	if (fence_->GetCompletedValue() < fenceValue)
	//	{
	//		// 完了時にイベントを設定
	//		auto hr = fence_->SetEventOnCompletion(fenceValue, fence_event_);
	//		if (FAILED(hr))
	//		{
	//			return;
	//		}

	//		// 待機処理
	//		if (WAIT_OBJECT_0 != WaitForSingleObjectEx(fence_event_, INFINITE, FALSE))
	//		{
	//			return;
	//		}
	//	}
	//}
	ComPtr<ID3D12Fence1> fence;
	const UINT64 expect_value = 1;
	device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	queue_->Signal(fence.Get(), expect_value);
	if (fence->GetCompletedValue() != expect_value)
	{
		fence->SetEventOnCompletion(expect_value, fence_event_);
		WaitForSingleObject(fence_event_, INFINITE);
	}
}

void Engine::ToggleFullscreen()
{
	BOOL is_fullscreen;
	swapchain_->GetFullscreenState(&is_fullscreen, nullptr);

	if (is_fullscreen)
	{
		swapchain_->SetFullscreenState(FALSE, nullptr);
		SetWindowLong(hwnd_, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		ShowWindow(hwnd_, SW_NORMAL);

		ResizeWindow(1280, 720);
	}
	else
	{
		swapchain_->SetFullscreenState(TRUE, nullptr);

		DXGI_SWAP_CHAIN_DESC desc = {};
		swapchain_->GetDesc(&desc);

		DXGI_MODE_DESC mode_desc = desc.BufferDesc;
		mode_desc.Width = 1920;
		mode_desc.Height = 1080;
		swapchain_->ResizeTarget(&mode_desc);

		ResizeWindow(1920, 1080);
	}
	
}

BufferManager* Engine::GetGBufferManager()
{
	return buffer_manager_.get();
}

ShadowMap* Engine::GetShadowMap()
{
	return shadowmap_.get();
}



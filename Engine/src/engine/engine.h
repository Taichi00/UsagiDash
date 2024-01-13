#pragma once
#define NOMINMAX
#include "engine/comptr.h"
#include "engine/descriptor_heap.h"
#include "engine/shared_struct.h"
#include "engine/texture2d.h"
#include <d3d12.h>
#include <d3d12sdklayers.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#pragma comment(lib, "d3d12.lib")	// d3d12ライブラリをリンクする
#pragma comment(lib, "dxgi.lib")	// dxgiライブラリをリンクする

class Window;
class GBufferManager;
class ShadowMap;
class Engine2D;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;

class Engine
{
public:
	enum { FRAME_BUFFER_COUNT = 2 }; // ダブルバッファリング

public:
	Engine();
	~Engine();

public:
	/*static Engine* Get() {
		static Engine instance;
		return &instance;
	}*/

	bool Init(Window* window); // エンジン初期化

	void InitRender();
	void BeginRender(); // 描画の開始処理
	void EndRender(); // 描画の終了処理
	void BeginRenderMSAA();
	void EndRenderMSAA();
	void EndRenderD3D();

	void BeginDeferredRender();
	void DepthPrePath();
	void GBufferPath();
	void LightingPath();
	void SSAOPath();
	void BlurHorizontalPath();
	void BlurVerticalPath();
	void PostProcessPath();
	void FXAAPath();
	void EndDeferredRender();

	void WaitRender(); // 描画完了を待つ処理

public:
	ID3D12Device6* Device();
	ID3D12GraphicsCommandList* CommandList();
	ID3D12CommandAllocator* Allocator();
	ID3D12CommandQueue* Queue();
	ID3D12CommandQueue** QueueAddress();
	ID3D12Resource* RenderTarget(int index);
	std::shared_ptr<DescriptorHeap> RtvHeap();
	std::shared_ptr<DescriptorHeap> DsvHeap();
	std::shared_ptr<DescriptorHeap> GBufferHeap();
	GBufferManager* GetGBufferManager();
	ShadowMap* GetShadowMap();
	UINT CurrentBackBufferIndex();
	Window* GetWindow();
	UINT FrameBufferWidth();
	UINT FrameBufferHeight();
	Engine2D* GetEngine2D();
	ID3D12DebugDevice* DebugDevice();

	float AspectRate();

public:
	bool UploadTexture(
		ID3D12Resource* textureData, std::vector<D3D12_SUBRESOURCE_DATA> subresources);

	std::unique_ptr<VertexBuffer> CreateVertexBuffer(const std::vector<Vertex>& vertices); // 頂点バッファを生成
	std::unique_ptr<IndexBuffer> CreateIndexBuffer(const std::vector<uint32_t>& indices); // インデックスバッファを生成
	std::unique_ptr<ConstantBuffer> CreateConstantBuffer(const unsigned long long size); // 定数バッファを生成
	void CreateShaderResourceView(const Texture2D& texture, const DescriptorHandle& handle); // SRVを生成
	std::unique_ptr<DescriptorHeap> CreateDescriptorHeap(const unsigned int num); // ディスクリプタヒープを生成

private: // DirectX12初期化に使う関数
	bool EnableDebugLayer();	// デバッグデバイスを生成
	bool CreateDevice();		// デバイスを生成
	bool CreateCommandQueue();	// コマンドキューを生成
	bool CreateSwapChain();		// スワップチェインを生成
	bool CreateCommandList();	// コマンドリストとコマンドアロケーターを生成
	bool CreateFence();			// フェンスを生成
	void CreateViewPort();		// ビューポートを生成
	void CreateScissorRect();	// シザー矩形を生成
	
private:
	Window* window_ = nullptr;
	HWND hwnd_;
	UINT frame_buffer_width_ = 0;
	UINT frame_buffer_height_ = 0;
	UINT current_back_buffer_index_ = 0;

	ComPtr<ID3D12Device6> device_; // デバイス
	ComPtr<ID3D12CommandQueue> queue_; // コマンドキュー
	ComPtr<IDXGISwapChain3> swapchain_; // スワップチェイン
	ComPtr<ID3D12CommandAllocator> allocator_[FRAME_BUFFER_COUNT]; // コマンドアロケーター
	ComPtr<ID3D12CommandAllocator> oneshot_allocator_;
	ComPtr<ID3D12GraphicsCommandList> command_list_; // コマンドリスト
	HANDLE fence_event_; // フェンスで使うイベント
	ComPtr<ID3D12Fence> fence_; // フェンス
	UINT64 fence_value_[FRAME_BUFFER_COUNT]; // フェンスの値
	D3D12_VIEWPORT viewport_; // ビューポート
	D3D12_RECT scissor_; // シザー矩形
	ComPtr<ID3D12DebugDevice> debug_device_;

private:
	bool CreateRenderTarget();	// レンダーターゲットを生成
	bool CreateDepthStencil();	// 深度ステンシルバッファを生成
	bool CreateMSAA();
	bool CreateGBuffer();		// ディファードレンダリング用のG-Bufferを生成

	UINT rtv_descriptor_size_ = 0; // レンダーターゲットビューのディスクリプタサイズ
	std::shared_ptr<DescriptorHeap> rtv_heap_; // レンダーターゲットのディスクリプタヒープ
	DescriptorHandle rtv_handles_[FRAME_BUFFER_COUNT];
	ComPtr<ID3D12Resource> render_targets_[FRAME_BUFFER_COUNT];	// レンダーターゲット

	UINT dsv_descriptor_size_ = 0; // 深度ステンシルのディスクリプタサイズ
	std::shared_ptr<DescriptorHeap> dsv_heap_; // 深度ステンシルのディスクリプタヒープ
	DescriptorHandle dsv_handle_;
	ComPtr<ID3D12Resource> depth_stencil_buffer_; // 深度ステンシルバッファ

	UINT sample_count_;
	ComPtr<ID3D12Resource> msaa_color_target_;
	ComPtr<ID3D12Resource> msaa_depth_target_;
	DescriptorHandle msaa_rtv_handle_;
	DescriptorHandle msaa_dsv_handle_;

	std::unique_ptr<ShadowMap> shadowmap_;

	std::unique_ptr<GBufferManager> gbuffer_manager_;
	std::shared_ptr<DescriptorHeap> gbuffer_heap_;

	std::unique_ptr<Engine2D> engine2d_; // 2D描画エンジンへのポインタ

private:
	ID3D12Resource* current_render_target_ = nullptr; // 現在フレームのレンダーターゲットを一時的に保存しておく変数
	

};

//extern std::unique_ptr<Engine> g_Engine; // どこからでも参照したいのでグローバルにする
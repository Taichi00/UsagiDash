#pragma once
#define NOMINMAX
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include "ComPtr.h"
#include <vector>
#include <memory>

#pragma comment(lib, "d3d12.lib")	// d3d12ライブラリをリンクする
#pragma comment(lib, "dxgi.lib")	// dxgiライブラリをリンクする

class Window;
class DescriptorHeap;
class DescriptorHandle;

struct GBuffer
{
	ComPtr<ID3D12Resource> pPositionTex = nullptr;
	ComPtr<ID3D12Resource> pNormalTex = nullptr;
	ComPtr<ID3D12Resource> pAlbedoTex = nullptr;
	ComPtr<ID3D12Resource> pDepthTex = nullptr;
	ComPtr<ID3D12Resource> pLightingTex = nullptr;
	ComPtr<ID3D12Resource> pPostProcessTex = nullptr;

	std::shared_ptr<DescriptorHandle> pPositionSrvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> pNormalSrvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> pAlbedoSrvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> pDepthSrvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> pLightingSrvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> pPostProcessSrvHandle = nullptr;

	std::shared_ptr<DescriptorHandle> pPositionRtvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> pNormalRtvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> pAlbedoRtvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> pDepthRtvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> pLightingRtvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> pPostProcessRtvHandle = nullptr;
};

class Engine
{
public:
	enum { FRAME_BUFFER_COUNT = 2 };	// ダブルバッファリング

public:
	~Engine();

	bool Init(Window* window);	// エンジン初期化

	void InitRender();
	void BeginRender();	// 描画の開始処理
	void EndRender();	// 描画の終了処理
	void BeginRenderMSAA();
	void EndRenderMSAA();

	void BeginDeferredRender();
	void DepthPrePath();
	void GBufferPath();
	void LightingPath();
	void PostProcessPath();
	void FXAAPath();
	void EndDeferredRender();

public: // 外からアクセスしたいのでGetterとして公開するもの
	ID3D12Device6* Device();
	ID3D12GraphicsCommandList* CommandList();
	ID3D12CommandAllocator* Allocator();
	ID3D12CommandQueue* Queue();
	DescriptorHeap* RtvHeap();
	DescriptorHeap* DsvHeap();
	DescriptorHeap* GBufferHeap();
	GBuffer* GetGBuffer();
	UINT CurrentBackBufferIndex();

	float AspectRate();

public:
	bool UploadTexture(
		ID3D12Resource* textureData, std::vector<D3D12_SUBRESOURCE_DATA> subresources);

private: // DirectX12初期化に使う関数たち
	bool CreateDevice();		// デバイスを生成
	bool CreateCommandQueue();	// コマンドキューを生成
	bool CreateSwapChain();		// スワップチェインを生成
	bool CreateCommandList();	// コマンドリストとコマンドアロケーターを生成
	bool CreateFence();			// フェンスを生成
	void CreateViewPort();		// ビューポートを生成
	void CreateScissorRect();	// シザー矩形を生成

private: // 描画に使うDirectX12のオブジェクトたち
	Window* m_pWindow = nullptr;
	HWND m_hWnd;
	UINT m_FrameBufferWidth = 0;
	UINT m_FrameBufferHeight = 0;
	UINT m_CurrentBackBufferIndex = 0;

	ComPtr<ID3D12Device6> m_pDevice = nullptr;		// デバイス
	ComPtr<ID3D12CommandQueue> m_pQueue = nullptr;	// コマンドキュー
	ComPtr<IDXGISwapChain3> m_pSwapChain = nullptr;	// スワップチェイン
	ComPtr<ID3D12CommandAllocator> m_pAllocator[FRAME_BUFFER_COUNT] = { nullptr };	// コマンドアロケーター
	ComPtr<ID3D12CommandAllocator> m_pOneshotAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList = nullptr;	// コマンドリスト
	HANDLE m_fenceEvent = nullptr;					// フェンスで使うイベント
	ComPtr<ID3D12Fence> m_pFence = nullptr;			// フェンス
	UINT64 m_fenceValue[FRAME_BUFFER_COUNT];		// フェンスの値
	D3D12_VIEWPORT m_Viewport;						// ビューポート
	D3D12_RECT m_Scissor;							// シザー矩形

private: // 描画に使うオブジェクトとその生成関数たち
	bool CreateRenderTarget();	// レンダーターゲットを生成
	bool CreateDepthStencil();	// 深度ステンシルバッファを生成
	bool CreateMSAA();
	bool CreateGBuffer();		// ディファードレンダリング用のG-Bufferを生成

	UINT m_RtvDescriptorSize = 0;												// レンダーターゲットビューのディスクリプタサイズ
	std::shared_ptr<DescriptorHeap> m_pRtvHeap = nullptr;						// レンダーターゲットのディスクリプタヒープ
	std::shared_ptr<DescriptorHandle> m_pRtvHandles[FRAME_BUFFER_COUNT] = { nullptr };
	ComPtr<ID3D12Resource> m_pRenderTargets[FRAME_BUFFER_COUNT] = { nullptr };	// レンダーターゲット

	UINT m_DsvDescriptorSize = 0;							// 深度ステンシルのディスクリプタサイズ
	std::shared_ptr<DescriptorHeap> m_pDsvHeap = nullptr;	// 深度ステンシルのディスクリプタヒープ
	std::shared_ptr<DescriptorHandle> m_pDsvHandle = nullptr;
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer = nullptr;	// 深度ステンシルバッファ

	UINT m_sampleCount;
	ComPtr<ID3D12Resource> m_pMSAAColorTarget = nullptr;
	ComPtr<ID3D12Resource> m_pMSAADepthTarget = nullptr;
	std::shared_ptr<DescriptorHandle> m_pMSAARtvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> m_pMSAADsvHandle = nullptr;

	std::shared_ptr<DescriptorHeap> m_pShadowTexHeap = nullptr;
	std::shared_ptr<DescriptorHandle> m_pShadowDsvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> m_pShadowRtvHandle = nullptr;
	std::shared_ptr<DescriptorHandle> m_pShadowTexHandle = nullptr;
	ComPtr<ID3D12Resource> m_pShadowTex = nullptr;
	ComPtr<ID3D12Resource> m_pShadowDepth = nullptr;

	GBuffer m_gbuffer;
	std::shared_ptr<DescriptorHeap> m_pGBufferHeap = nullptr;

private: // 描画ループで使用するもの
	ID3D12Resource* m_currentRenderTarget = nullptr;	// 現在フレームのレンダーターゲットを一時的に保存しておく変数
	void WaitRender();	// 描画完了を待つ処理

};

extern Engine* g_Engine;	// どこからでも参照したいのでグローバルにする
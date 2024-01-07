#pragma once
#define NOMINMAX
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <d3d11on12.h>
#include <d2d1_3.h>
#include <dwrite_3.h>
#include "ComPtr.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <string>
#include "DescriptorHeap.h"

#pragma comment(lib, "d3d12.lib")	// d3d12ライブラリをリンクする
#pragma comment(lib, "dxgi.lib")	// dxgiライブラリをリンクする
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

class Window;
class GBufferManager;


class Engine
{
public:
	enum { FRAME_BUFFER_COUNT = 2 };	// ダブルバッファリング

public:
	Engine();
	~Engine();

	bool Init(Window* window);	// エンジン初期化

	void InitRender();
	void BeginRender();	// 描画の開始処理
	void EndRender();	// 描画の終了処理
	void BeginRenderMSAA();
	void EndRenderMSAA();
	void EndRenderD3D();
	void BeginRenderD2D();
	void EndRenderD2D();

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

	void drawText(const std::string& textFormatKey, const std::string& solidColorBrushKey,
		const std::wstring& text, const D2D1_RECT_F& rect) const noexcept;

	void WaitRender();	// 描画完了を待つ処理

public: // 外からアクセスしたいのでGetterとして公開するもの
	ID3D12Device6* Device();
	ID3D12GraphicsCommandList* CommandList();
	ID3D12CommandAllocator* Allocator();
	ID3D12CommandQueue* Queue();
	DescriptorHeap* RtvHeap();
	DescriptorHeap* DsvHeap();
	DescriptorHeap* GBufferHeap();
	GBufferManager* GetGBufferManager();
	UINT CurrentBackBufferIndex();

	float AspectRate();

public:
	bool UploadTexture(
		ID3D12Resource* textureData, std::vector<D3D12_SUBRESOURCE_DATA> subresources);

private: // DirectX12初期化に使う関数たち
	bool CreateDevice();		// デバイスを生成
	bool CreateCommandQueue();	// コマンドキューを生成
	bool CreateD3D11Device();	// D3D11Deviceを生成
	bool CreateSwapChain();		// スワップチェインを生成
	bool CreateCommandList();	// コマンドリストとコマンドアロケーターを生成
	bool CreateFence();			// フェンスを生成
	void CreateViewPort();		// ビューポートを生成
	void CreateScissorRect();	// シザー矩形を生成
	bool CreateD2DDeviceContext();
	bool CreateDWriteFactory();

private: // 描画に使うDirectX12のオブジェクト
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
	ComPtr<ID3D11DeviceContext> m_pD3d11DeviceContext = nullptr; //D3D11のデバイスコンテキスト
	ComPtr<ID3D11On12Device> m_pD3d11On12Device = nullptr; // D3D11On12のデバイス（Direct2Dを使用するために必要）
	ComPtr<ID3D11Device> m_pD3d11Device = nullptr;	// D3D11のデバイス
	ComPtr<IDWriteFactory> m_pDirectWriteFactory = nullptr;
	ComPtr<ID2D1DeviceContext> m_pD2dDeviceContext = nullptr;

private: // 描画に使うオブジェクトとその生成関数たち
	bool CreateRenderTarget();	// レンダーターゲットを生成
	bool CreateDepthStencil();	// 深度ステンシルバッファを生成
	bool CreateMSAA();
	bool CreateGBuffer();		// ディファードレンダリング用のG-Bufferを生成
	bool CreateD2DRenderTarget(); // Direct2D用のレンダーターゲットを生成
	void RegisterSolidColorBrush(const std::string& key, const D2D1::ColorF color) noexcept;
	void RegisterTextFormat(const std::string& key, const std::wstring& fontName, const FLOAT fontSize) noexcept;

	UINT m_RtvDescriptorSize = 0;									// レンダーターゲットビューのディスクリプタサイズ
	std::unique_ptr<DescriptorHeap> m_pRtvHeap;						// レンダーターゲットのディスクリプタヒープ
	DescriptorHandle m_pRtvHandles[FRAME_BUFFER_COUNT];
	ComPtr<ID3D12Resource> m_pRenderTargets[FRAME_BUFFER_COUNT];	// レンダーターゲット

	UINT m_DsvDescriptorSize = 0;							// 深度ステンシルのディスクリプタサイズ
	std::unique_ptr<DescriptorHeap> m_pDsvHeap;				// 深度ステンシルのディスクリプタヒープ
	DescriptorHandle m_pDsvHandle;
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer;			// 深度ステンシルバッファ

	UINT m_sampleCount;
	ComPtr<ID3D12Resource> m_pMSAAColorTarget;
	ComPtr<ID3D12Resource> m_pMSAADepthTarget;
	DescriptorHandle m_pMSAARtvHandle;
	DescriptorHandle m_pMSAADsvHandle;

	std::unique_ptr<DescriptorHeap> m_pShadowTexHeap;
	DescriptorHandle m_pShadowDsvHandle;
	DescriptorHandle m_pShadowRtvHandle;
	DescriptorHandle m_pShadowTexHandle;
	ComPtr<ID3D12Resource> m_pShadowTex;
	ComPtr<ID3D12Resource> m_pShadowDepth;

	std::unique_ptr<GBufferManager> m_pGBufferManager;
	std::unique_ptr<DescriptorHeap> m_pGBufferHeap;

	ComPtr<ID3D11Resource> m_pWrappedBackBuffers[FRAME_BUFFER_COUNT];
	ComPtr<ID2D1Bitmap1> m_pD2dRenderTargets[FRAME_BUFFER_COUNT];

	std::unordered_map<std::string, ComPtr<ID2D1SolidColorBrush>> m_solidColorBrushMap;
	std::unordered_map<std::string, ComPtr<IDWriteTextFormat>> m_textFormatMap;

private: // 描画ループで使用するもの
	ID3D12Resource* m_currentRenderTarget = nullptr;	// 現在フレームのレンダーターゲットを一時的に保存しておく変数
	

};

extern std::unique_ptr<Engine> g_Engine;	// どこからでも参照したいのでグローバルにする
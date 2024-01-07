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

#pragma comment(lib, "d3d12.lib")	// d3d12���C�u�����������N����
#pragma comment(lib, "dxgi.lib")	// dxgi���C�u�����������N����
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

class Window;
class GBufferManager;


class Engine
{
public:
	enum { FRAME_BUFFER_COUNT = 2 };	// �_�u���o�b�t�@�����O

public:
	Engine();
	~Engine();

	bool Init(Window* window);	// �G���W��������

	void InitRender();
	void BeginRender();	// �`��̊J�n����
	void EndRender();	// �`��̏I������
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

	void WaitRender();	// �`�抮����҂���

public: // �O����A�N�Z�X�������̂�Getter�Ƃ��Č��J�������
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

private: // DirectX12�������Ɏg���֐�����
	bool CreateDevice();		// �f�o�C�X�𐶐�
	bool CreateCommandQueue();	// �R�}���h�L���[�𐶐�
	bool CreateD3D11Device();	// D3D11Device�𐶐�
	bool CreateSwapChain();		// �X���b�v�`�F�C���𐶐�
	bool CreateCommandList();	// �R�}���h���X�g�ƃR�}���h�A���P�[�^�[�𐶐�
	bool CreateFence();			// �t�F���X�𐶐�
	void CreateViewPort();		// �r���[�|�[�g�𐶐�
	void CreateScissorRect();	// �V�U�[��`�𐶐�
	bool CreateD2DDeviceContext();
	bool CreateDWriteFactory();

private: // �`��Ɏg��DirectX12�̃I�u�W�F�N�g
	Window* m_pWindow = nullptr;
	HWND m_hWnd;
	UINT m_FrameBufferWidth = 0;
	UINT m_FrameBufferHeight = 0;
	UINT m_CurrentBackBufferIndex = 0;

	ComPtr<ID3D12Device6> m_pDevice = nullptr;		// �f�o�C�X
	ComPtr<ID3D12CommandQueue> m_pQueue = nullptr;	// �R�}���h�L���[
	ComPtr<IDXGISwapChain3> m_pSwapChain = nullptr;	// �X���b�v�`�F�C��
	ComPtr<ID3D12CommandAllocator> m_pAllocator[FRAME_BUFFER_COUNT] = { nullptr };	// �R�}���h�A���P�[�^�[
	ComPtr<ID3D12CommandAllocator> m_pOneshotAllocator = nullptr;
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList = nullptr;	// �R�}���h���X�g
	HANDLE m_fenceEvent = nullptr;					// �t�F���X�Ŏg���C�x���g
	ComPtr<ID3D12Fence> m_pFence = nullptr;			// �t�F���X
	UINT64 m_fenceValue[FRAME_BUFFER_COUNT];		// �t�F���X�̒l
	D3D12_VIEWPORT m_Viewport;						// �r���[�|�[�g
	D3D12_RECT m_Scissor;							// �V�U�[��`
	ComPtr<ID3D11DeviceContext> m_pD3d11DeviceContext = nullptr; //D3D11�̃f�o�C�X�R���e�L�X�g
	ComPtr<ID3D11On12Device> m_pD3d11On12Device = nullptr; // D3D11On12�̃f�o�C�X�iDirect2D���g�p���邽�߂ɕK�v�j
	ComPtr<ID3D11Device> m_pD3d11Device = nullptr;	// D3D11�̃f�o�C�X
	ComPtr<IDWriteFactory> m_pDirectWriteFactory = nullptr;
	ComPtr<ID2D1DeviceContext> m_pD2dDeviceContext = nullptr;

private: // �`��Ɏg���I�u�W�F�N�g�Ƃ��̐����֐�����
	bool CreateRenderTarget();	// �����_�[�^�[�Q�b�g�𐶐�
	bool CreateDepthStencil();	// �[�x�X�e���V���o�b�t�@�𐶐�
	bool CreateMSAA();
	bool CreateGBuffer();		// �f�B�t�@�[�h�����_�����O�p��G-Buffer�𐶐�
	bool CreateD2DRenderTarget(); // Direct2D�p�̃����_�[�^�[�Q�b�g�𐶐�
	void RegisterSolidColorBrush(const std::string& key, const D2D1::ColorF color) noexcept;
	void RegisterTextFormat(const std::string& key, const std::wstring& fontName, const FLOAT fontSize) noexcept;

	UINT m_RtvDescriptorSize = 0;									// �����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�T�C�Y
	std::unique_ptr<DescriptorHeap> m_pRtvHeap;						// �����_�[�^�[�Q�b�g�̃f�B�X�N���v�^�q�[�v
	DescriptorHandle m_pRtvHandles[FRAME_BUFFER_COUNT];
	ComPtr<ID3D12Resource> m_pRenderTargets[FRAME_BUFFER_COUNT];	// �����_�[�^�[�Q�b�g

	UINT m_DsvDescriptorSize = 0;							// �[�x�X�e���V���̃f�B�X�N���v�^�T�C�Y
	std::unique_ptr<DescriptorHeap> m_pDsvHeap;				// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v
	DescriptorHandle m_pDsvHandle;
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer;			// �[�x�X�e���V���o�b�t�@

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

private: // �`�惋�[�v�Ŏg�p�������
	ID3D12Resource* m_currentRenderTarget = nullptr;	// ���݃t���[���̃����_�[�^�[�Q�b�g���ꎞ�I�ɕۑ����Ă����ϐ�
	

};

extern std::unique_ptr<Engine> g_Engine;	// �ǂ�����ł��Q�Ƃ������̂ŃO���[�o���ɂ���
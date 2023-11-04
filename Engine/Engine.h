#pragma once
#define NOMINMAX
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include "ComPtr.h"
#include <vector>
#include <memory>

#pragma comment(lib, "d3d12.lib")	// d3d12���C�u�����������N����
#pragma comment(lib, "dxgi.lib")	// dxgi���C�u�����������N����

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
	enum { FRAME_BUFFER_COUNT = 2 };	// �_�u���o�b�t�@�����O

public:
	~Engine();

	bool Init(Window* window);	// �G���W��������

	void InitRender();
	void BeginRender();	// �`��̊J�n����
	void EndRender();	// �`��̏I������
	void BeginRenderMSAA();
	void EndRenderMSAA();

	void BeginDeferredRender();
	void DepthPrePath();
	void GBufferPath();
	void LightingPath();
	void PostProcessPath();
	void FXAAPath();
	void EndDeferredRender();

public: // �O����A�N�Z�X�������̂�Getter�Ƃ��Č��J�������
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

private: // DirectX12�������Ɏg���֐�����
	bool CreateDevice();		// �f�o�C�X�𐶐�
	bool CreateCommandQueue();	// �R�}���h�L���[�𐶐�
	bool CreateSwapChain();		// �X���b�v�`�F�C���𐶐�
	bool CreateCommandList();	// �R�}���h���X�g�ƃR�}���h�A���P�[�^�[�𐶐�
	bool CreateFence();			// �t�F���X�𐶐�
	void CreateViewPort();		// �r���[�|�[�g�𐶐�
	void CreateScissorRect();	// �V�U�[��`�𐶐�

private: // �`��Ɏg��DirectX12�̃I�u�W�F�N�g����
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

private: // �`��Ɏg���I�u�W�F�N�g�Ƃ��̐����֐�����
	bool CreateRenderTarget();	// �����_�[�^�[�Q�b�g�𐶐�
	bool CreateDepthStencil();	// �[�x�X�e���V���o�b�t�@�𐶐�
	bool CreateMSAA();
	bool CreateGBuffer();		// �f�B�t�@�[�h�����_�����O�p��G-Buffer�𐶐�

	UINT m_RtvDescriptorSize = 0;												// �����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�T�C�Y
	std::shared_ptr<DescriptorHeap> m_pRtvHeap = nullptr;						// �����_�[�^�[�Q�b�g�̃f�B�X�N���v�^�q�[�v
	std::shared_ptr<DescriptorHandle> m_pRtvHandles[FRAME_BUFFER_COUNT] = { nullptr };
	ComPtr<ID3D12Resource> m_pRenderTargets[FRAME_BUFFER_COUNT] = { nullptr };	// �����_�[�^�[�Q�b�g

	UINT m_DsvDescriptorSize = 0;							// �[�x�X�e���V���̃f�B�X�N���v�^�T�C�Y
	std::shared_ptr<DescriptorHeap> m_pDsvHeap = nullptr;	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v
	std::shared_ptr<DescriptorHandle> m_pDsvHandle = nullptr;
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer = nullptr;	// �[�x�X�e���V���o�b�t�@

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

private: // �`�惋�[�v�Ŏg�p�������
	ID3D12Resource* m_currentRenderTarget = nullptr;	// ���݃t���[���̃����_�[�^�[�Q�b�g���ꎞ�I�ɕۑ����Ă����ϐ�
	void WaitRender();	// �`�抮����҂���

};

extern Engine* g_Engine;	// �ǂ�����ł��Q�Ƃ������̂ŃO���[�o���ɂ���
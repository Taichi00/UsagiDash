#include "Engine.h"
#include <d3d12.h>
#include <stdio.h>
#include "Window.h"
#include "PipelineState.h"
#include "DescriptorHeap.h"
#include "d3dx12.h"
#include "GBufferManager.h"
#include "GBuffer.h"
#include "ShadowMap.h"

//std::unique_ptr<Engine> g_Engine;

Engine::Engine()
{
}

Engine::~Engine()
{
	printf("Delete Engine\n");
	m_pShadowMap.release();
	m_pDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
}

bool Engine::Init(Window* window)
{
	m_pWindow = window;
	m_FrameBufferWidth = window->Width();
	m_FrameBufferHeight = window->Height();
	m_hWnd = window->HWnd();

#if _DEBUG
	EnableDebugLayer();
#endif

	if (!CreateDevice())
	{
		printf("�f�o�C�X�̐����Ɏ��s\n");
		return false;
	}

	if (!CreateCommandQueue())
	{
		printf("�R�}���h�L���[�̐����Ɏ��s\n");
		return false;
	}

	if (!CreateD3D11Device())
	{
		printf("D3D11�̃f�o�C�X�����Ɏ��s\n");
	}

	if (!CreateSwapChain())
	{
		printf("�X���b�v�`�F�C���̐����Ɏ��s\n");
		return false;
	}

	if (!CreateCommandList())
	{
		printf("�R�}���h���X�g�̐����Ɏ��s\n");
		return false;
	}

	if (!CreateFence())
	{
		printf("�t�F���X�̐����Ɏ��s\n");
		return false;
	}

	if (!CreateD2DDeviceContext())
	{
		printf("D2DDeviceContext�̐����Ɏ��s\n");
	}

	if (!CreateDWriteFactory())
	{
		printf("DWriteFactory�̐����Ɏ��s\n");
	}

	// �r���[�|�[�g�ƃV�U�[��`�𐶐�
	CreateViewPort();
	CreateScissorRect();

	if (!CreateRenderTarget())
	{
		printf("�����_�[�^�[�Q�b�g�̐����Ɏ��s\n");
		return false;
	}

	if (!CreateDepthStencil())
	{
		printf("�f�v�X�X�e���V���o�b�t�@�̐����Ɏ��s\n");
	}

	if (!CreateMSAA())
	{
		printf("MSAA���\�[�X�̐����Ɏ��s\n");
	}

	if (!CreateGBuffer())
	{
		printf("G-Buffer�̐����Ɏ��s\n");
	}

	if (!CreateD2DRenderTarget())
	{
		printf("D2dRenderTarget�̐����Ɏ��s\n");
	}

	RegisterTextFormat("normal", L"Source Han Sans VF", 24);
	RegisterSolidColorBrush("white", D2D1::ColorF::White);

	// �V���h�E�}�b�v�̐���
	m_pShadowMap = std::make_unique<ShadowMap>();

	printf("�`��G���W���̏������ɐ���\n");
	return true;
}

void Engine::BeginRender()
{
	// �r���[�|�[�g�ƃV�U�[��`��ݒ�
	m_pCommandList->RSSetViewports(1, &m_Viewport);
	m_pCommandList->RSSetScissorRects(1, &m_Scissor);

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = m_pRtvHandles[m_CurrentBackBufferIndex].HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = m_pDsvHandle.HandleCPU();

	// �����_�[�^�[�Q�b�g���g�p�\�ɂȂ�܂ő҂�
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_currentRenderTarget,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_pCommandList->ResourceBarrier(1, &barrier);

	// �����_�[�^�[�Q�b�g��ݒ�
	m_pCommandList->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);

	// �����_�[�^�[�Q�b�g���N���A
	const float clearColor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	m_pCommandList->ClearRenderTargetView(currentRtvHandle, clearColor, 0, nullptr);

	// �[�x�X�e���V���r���[���N���A
	m_pCommandList->ClearDepthStencilView(currentDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Engine::InitRender()
{
	// ���݂̃����_�[�^�[�Q�b�g���X�V
	m_currentRenderTarget = m_pRenderTargets[m_CurrentBackBufferIndex].Get();

	// �R�}���h�����������Ă��߂鏀��������
	m_pAllocator[m_CurrentBackBufferIndex]->Reset();
	m_pCommandList->Reset(m_pAllocator[m_CurrentBackBufferIndex].Get(), nullptr);
}

void Engine::BeginRenderMSAA()
{
	// �r���[�|�[�g�ƃV�U�[��`��ݒ�
	m_pCommandList->RSSetViewports(1, &m_Viewport);
	m_pCommandList->RSSetScissorRects(1, &m_Scissor);

	// �����_�[�^�[�Q�b�g���g�p�\�ɂȂ�܂ő҂�
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_pMSAAColorTarget.Get(),
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	m_pCommandList->ResourceBarrier(1, &barrier);

	// �����_�[�^�[�Q�b�g��ݒ�
	auto rtvHandle = m_pMSAARtvHandle.HandleCPU();
	auto dsvHandle = m_pMSAADsvHandle.HandleCPU();
	m_pCommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// �����_�[�^�[�Q�b�g���N���A
	const float clearColor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	m_pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// �[�x�X�e���V���r���[���N���A
	m_pCommandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Engine::EndRender()
{
	// �����_�[�^�[�Q�b�g�ɏ������ݏI���܂ő҂�
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_currentRenderTarget,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	m_pCommandList->ResourceBarrier(1, &barrier);

	// �R�}���h�̋L�^���I��
	m_pCommandList->Close();

	// �R�}���h�����s
	ID3D12CommandList* ppCmdLists[] = { m_pCommandList.Get() };
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	// �X���b�v�`�F�[����؂�ւ�
	m_pSwapChain->Present(1, 0);

	// �`�抮����҂�
	WaitRender();

	// �o�b�N�o�b�t�@�ԍ��X�V
	m_CurrentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

void Engine::EndRenderMSAA()
{
	// �����_�[�^�[�Q�b�g�ɏ������ݏI���܂ő҂�
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

	// �����_�[�^�[�Q�b�g�ɏ������ݏI���܂ő҂�
	auto barrierResolveToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		m_currentRenderTarget,
		D3D12_RESOURCE_STATE_RESOLVE_DEST,
		D3D12_RESOURCE_STATE_PRESENT
	);
	m_pCommandList->ResourceBarrier(1, &barrierResolveToPresent);

	// �R�}���h�̋L�^���I��
	m_pCommandList->Close();

	// �R�}���h�����s
	ID3D12CommandList* ppCmdLists[] = { m_pCommandList.Get() };
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	// �X���b�v�`�F�[����؂�ւ�
	m_pSwapChain->Present(1, 0);

	// �`�抮����҂�
	WaitRender();

	// �o�b�N�o�b�t�@�ԍ��X�V
	m_CurrentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

void Engine::EndRenderD3D()
{
	// �R�}���h�̋L�^���I��
	m_pCommandList->Close();

	// �R�}���h�����s
	ID3D12CommandList* ppCmdLists[] = { m_pCommandList.Get() };
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	// �`�抮����҂�
	WaitRender();
}

void Engine::BeginRenderD2D()
{
	const auto& wrappedBackBuffer = m_pWrappedBackBuffers[m_CurrentBackBufferIndex];
	const auto& backBufferForD2D = m_pD2dRenderTargets[m_CurrentBackBufferIndex];

	m_pD3d11On12Device->AcquireWrappedResources(wrappedBackBuffer.GetAddressOf(), 1);
	m_pD2dDeviceContext->SetTarget(backBufferForD2D.Get());
	m_pD2dDeviceContext->BeginDraw();
	m_pD2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
}

void Engine::EndRenderD2D()
{
	const auto& wrappedBackBuffer = m_pWrappedBackBuffers[m_CurrentBackBufferIndex];
	
	m_pD2dDeviceContext->EndDraw();
	m_pD3d11On12Device->ReleaseWrappedResources(wrappedBackBuffer.GetAddressOf(), 1);
	m_pD3d11DeviceContext->Flush();
}

void Engine::BeginDeferredRender()
{
	// �r���[�|�[�g�ƃV�U�[��`��ݒ�
	m_pCommandList->RSSetViewports(1, &m_Viewport);
	m_pCommandList->RSSetScissorRects(1, &m_Scissor);

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = m_pRtvHandles[m_CurrentBackBufferIndex].HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = m_pDsvHandle.HandleCPU();

	// �����_�[�^�[�Q�b�g���g�p�\�ɂȂ�܂ő҂�
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_currentRenderTarget,
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("Position")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("Normal")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("Albedo")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("MetallicRoughness")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("Depth")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("Lighting")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		/*CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("SSAO")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("BlurredSSAO1")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("BlurredSSAO2")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),*/
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("PostProcess")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
	};
	m_pCommandList->ResourceBarrier(_countof(barriers), barriers);

	// �����_�[�^�[�Q�b�g���N���A
	const float clearColor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	const float zeroFloat[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	const float oneFloat[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const float zeroAlbedo[] = { 0, 0, 0, 1 };	// A�̓A�E�g���C���}�X�N�Ȃ̂łP�ŏ�����
	m_pCommandList->ClearRenderTargetView(currentRtvHandle, clearColor, 0, nullptr);
	m_pCommandList->ClearRenderTargetView(m_pGBufferManager->Get("Position")->RtvHandle()->HandleCPU(), zeroFloat, 0, nullptr);
	m_pCommandList->ClearRenderTargetView(m_pGBufferManager->Get("Normal")->RtvHandle()->HandleCPU(), zeroFloat, 0, nullptr);
	m_pCommandList->ClearRenderTargetView(m_pGBufferManager->Get("Albedo")->RtvHandle()->HandleCPU(), zeroAlbedo, 0, nullptr);
	m_pCommandList->ClearRenderTargetView(m_pGBufferManager->Get("MetallicRoughness")->RtvHandle()->HandleCPU(), zeroFloat, 0, nullptr);
	m_pCommandList->ClearRenderTargetView(m_pGBufferManager->Get("Depth")->RtvHandle()->HandleCPU(), oneFloat, 0, nullptr);
	m_pCommandList->ClearRenderTargetView(m_pGBufferManager->Get("Lighting")->RtvHandle()->HandleCPU(), zeroFloat, 0, nullptr);
	m_pCommandList->ClearRenderTargetView(m_pGBufferManager->Get("PostProcess")->RtvHandle()->HandleCPU(), zeroFloat, 0, nullptr);

	// �[�x�X�e���V���r���[���N���A
	m_pCommandList->ClearDepthStencilView(currentDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Engine::DepthPrePath()
{
	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = m_pRtvHandles[m_CurrentBackBufferIndex].HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = m_pDsvHandle.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
	m_pCommandList->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::GBufferPath()
{
	// �����_�[�^�[�Q�b�g��ݒ�
	D3D12_CPU_DESCRIPTOR_HANDLE handleRtvs[] = {
		m_pGBufferManager->Get("Position")->RtvHandle()->HandleCPU(),
		m_pGBufferManager->Get("Normal")->RtvHandle()->HandleCPU(),
		m_pGBufferManager->Get("Albedo")->RtvHandle()->HandleCPU(),
		m_pGBufferManager->Get("MetallicRoughness")->RtvHandle()->HandleCPU(),
		m_pGBufferManager->Get("Depth")->RtvHandle()->HandleCPU()
	};
	auto currentDsvHandle = m_pDsvHandle.HandleCPU();

	m_pCommandList->OMSetRenderTargets(_countof(handleRtvs), handleRtvs, FALSE, &currentDsvHandle);
}

void Engine::LightingPath()
{
	// �����_�[�^�[�Q�b�g�ɏ������ݏI���܂ő҂�
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("Position")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("Normal")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("Albedo")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("MetallicRoughness")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("Depth")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
	};
	m_pCommandList->ResourceBarrier(_countof(barriers), barriers);

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = m_pGBufferManager->Get("Lighting")->RtvHandle()->HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = m_pDsvHandle.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
	m_pCommandList->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::SSAOPath()
{
	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = m_pGBufferManager->Get("SSAO")->RtvHandle()->HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = m_pDsvHandle.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
	m_pCommandList->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::BlurHorizontalPath()
{
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("SSAO")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
	};

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = m_pGBufferManager->Get("BlurredSSAO1")->RtvHandle()->HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = m_pDsvHandle.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
	m_pCommandList->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::BlurVerticalPath()
{
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("BlurredSSAO1")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
	};

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = m_pGBufferManager->Get("BlurredSSAO2")->RtvHandle()->HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = m_pDsvHandle.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
	m_pCommandList->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::PostProcessPath()
{
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("Lighting")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
		/*CD3DX12_RESOURCE_BARRIER::Transition(
			m_pGBufferManager->Get("BlurredSSAO2")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),*/
	};
	m_pCommandList->ResourceBarrier(_countof(barriers), barriers);
	
	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = m_pGBufferManager->Get("PostProcess")->RtvHandle()->HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = m_pDsvHandle.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
	m_pCommandList->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::FXAAPath()
{
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			 m_pGBufferManager->Get("PostProcess")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE),
	};
	m_pCommandList->ResourceBarrier(_countof(barriers), barriers);

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = m_pRtvHandles[m_CurrentBackBufferIndex].HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = m_pDsvHandle.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
	m_pCommandList->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::EndDeferredRender()
{
	// �����_�[�^�[�Q�b�g�ɏ������ݏI���܂ő҂�
	//auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
	//	m_currentRenderTarget,
	//	D3D12_RESOURCE_STATE_RENDER_TARGET,
	//	D3D12_RESOURCE_STATE_PRESENT);
	//m_pCommandList->ResourceBarrier(1, &barrier);

	//// �R�}���h�̋L�^���I��
	//m_pCommandList->Close();

	//// �R�}���h�����s
	//ID3D12CommandList* ppCmdLists[] = { m_pCommandList.Get() };
	//m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	// �X���b�v�`�F�[����؂�ւ�
	m_pSwapChain->Present(1, 0);

	// �`�抮����҂�
	WaitRender();

	// �o�b�N�o�b�t�@�ԍ��X�V
	m_CurrentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

void Engine::drawText(const std::string& textFormatKey, const std::string& solidColorBrushKey, const std::wstring& text, const D2D1_RECT_F& rect) const noexcept
{
	const auto& textFormat = m_textFormatMap.at(textFormatKey);
	const auto& solidColorBrush = m_solidColorBrushMap.at(solidColorBrushKey);

	m_pD2dDeviceContext->DrawTextW(
		text.c_str(),
		static_cast<UINT32>(text.length()),
		textFormat.Get(),
		&rect,
		solidColorBrush.Get()
	);
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

std::shared_ptr<DescriptorHeap> Engine::RtvHeap()
{
	return m_pRtvHeap;
}

std::shared_ptr<DescriptorHeap> Engine::DsvHeap()
{
	return m_pDsvHeap;
}
std::shared_ptr<DescriptorHeap> Engine::GBufferHeap()
{
	return m_pGBufferHeap;
}

UINT Engine::CurrentBackBufferIndex()
{
	return m_CurrentBackBufferIndex;
}

float Engine::AspectRate()
{
	return m_pWindow->AspectRate();
}

bool Engine::UploadTexture(
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
		return false;
	}

	// �]������
	ComPtr<ID3D12GraphicsCommandList> command;
	hr = m_pDevice->CreateCommandList(
		0, 
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pOneshotAllocator.Get(),
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

	// ���\�[�X�o���A�̃Z�b�g
	auto barrierTex = CD3DX12_RESOURCE_BARRIER::Transition(
		textureData,
		D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	command->ResourceBarrier(1, &barrierTex);

	// �R�}���h�̎��s
	command->Close();
	ID3D12CommandList* cmds[] = { command.Get() };
	m_pQueue->ExecuteCommandLists(1, cmds);

	WaitRender();

	m_pOneshotAllocator->Reset();

	return true;
}

bool Engine::EnableDebugLayer()
{
	// �f�o�b�O���C���[��L����
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
	auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_pDevice.ReleaseAndGetAddressOf()));

	m_pDevice.As(&m_pDebugDevice);

	ComPtr<ID3D12InfoQueue> pInfoQueue;
	m_pDevice.As(&pInfoQueue);
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

	auto hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pQueue.ReleaseAndGetAddressOf()));

	return SUCCEEDED(hr);
}

bool Engine::CreateSwapChain()
{
	// DXGI�t�@�N�g���[�̐���
	IDXGIFactory4* pFactory = nullptr;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	if (FAILED(hr))
	{
		return false;
	}

	// �X���b�v�`�F�C���̐���
	DXGI_SWAP_CHAIN_DESC desc = {};
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

	// IDXGISwapChain3���擾
	hr = pSwapChain->QueryInterface(IID_PPV_ARGS(m_pSwapChain.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		pFactory->Release();
		pSwapChain->Release();
		return false;
	}

	// �o�b�N�o�b�t�@�ԍ����擾
	m_CurrentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	pFactory->Release();
	pSwapChain->Release();
	return true;
}

bool Engine::CreateCommandList()
{
	// �R�}���h�A���P�[�^�[�̍쐬
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

	hr = m_pDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&m_pOneshotAllocator)
	);

	if (FAILED(hr))
	{
		return false;
	}

	// �R�}���h���X�g�̐���
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

	// �R�}���h���X�g�͊J����Ă����Ԃō쐬�����̂ŁA�����������
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

	// �������s���Ƃ��̃C�x���g�n���h�����쐬����
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

bool Engine::CreateD2DDeviceContext()
{
	ComPtr<ID2D1Factory3> d2dfactory = nullptr;
	constexpr D2D1_FACTORY_OPTIONS factoryOptions{};

	auto hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory3),
		&factoryOptions,
		&d2dfactory
	);
	if (FAILED(hr))
	{
		return false;
	}

	ComPtr<IDXGIDevice> dxgiDevice = nullptr;
	hr = m_pD3d11On12Device.As(&dxgiDevice);
	if (FAILED(hr))
	{
		return false;
	}

	ComPtr<ID2D1Device> d2dDevice = nullptr;
	hr = d2dfactory->CreateDevice(
		dxgiDevice.Get(),
		d2dDevice.ReleaseAndGetAddressOf()
	);
	if (FAILED(hr))
	{
		return false;
	}

	hr = d2dDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		m_pD2dDeviceContext.ReleaseAndGetAddressOf()
	);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool Engine::CreateDWriteFactory()
{
	// �e�L�X�g�t�H�[�}�b�g�𐶐����邽�߂̃t�@�N�g���ł���IDWriteFactory�𐶐�
	auto hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory), &m_pDirectWriteFactory
	);

	return SUCCEEDED(hr);
}

bool Engine::CreateD3D11Device()
{
	ComPtr<ID3D11Device> d3d11Device;
	UINT d3d11DeviceFlags = 0U;

#ifdef _DEBUG
	d3d11DeviceFlags = D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#else
	d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#endif

	auto hr = D3D11On12CreateDevice(
		m_pDevice.Get(),
		d3d11DeviceFlags,
		nullptr,
		0U,
		reinterpret_cast<IUnknown**>(m_pQueue.GetAddressOf()),
		1U,
		0U,
		&d3d11Device,
		&m_pD3d11DeviceContext,
		nullptr
	);
	if (FAILED(hr))
	{
		return false;
	}

	d3d11Device.As(&m_pD3d11On12Device);
	m_pD3d11Device = d3d11Device;
	
	return true;
}

bool Engine::CreateRenderTarget()
{
	// RTV�p�̃f�B�X�N���v�^�q�[�v���쐬����
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = 100; //FRAME_BUFFER_COUNT + 2;
	desc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// SRGB��Ԃɂ���
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	m_pRtvHeap = std::make_unique<DescriptorHeap>(desc);
	if (!m_pRtvHeap->IsValid())
	{
		return false;
	}

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		m_pRtvHandles[i] = m_pRtvHeap->Alloc();
		m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pRenderTargets[i].ReleaseAndGetAddressOf()));
		m_pDevice->CreateRenderTargetView(m_pRenderTargets[i].Get(), &rtvDesc, m_pRtvHandles[i].HandleCPU());
	}

	return true;
}

bool Engine::CreateDepthStencil()
{
	// DSV�p�̃f�B�X�N���v�^�q�[�v���쐬����
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors	= 3;
	desc.Type			= D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	desc.Flags			= D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	
	m_pDsvHeap = std::make_unique<DescriptorHeap>(desc);
	if (!m_pDsvHeap->IsValid())
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

	// �f�B�X�N���v�^���쐬
	m_pDsvHandle = m_pDsvHeap->Alloc();
	m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, m_pDsvHandle.HandleCPU());

	return true;
}

bool Engine::CreateMSAA()
{
	auto format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	auto width = m_FrameBufferWidth;
	auto height = m_FrameBufferHeight;
	m_sampleCount = 4;

	// MSAA �`���o�b�t�@�i�J���[�j�̏���
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

	// MSAA �`���o�b�t�@�i�f�v�X�j�̏���
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

	// MSAA �ɏo�͂��邽�߂̃r���[����������
	D3D12_RENDER_TARGET_VIEW_DESC msaaRtvDesc{};
	msaaRtvDesc.Format = format;
	msaaRtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

	D3D12_DEPTH_STENCIL_VIEW_DESC msaaDsvDesc{};
	msaaDsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	msaaDsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;

	m_pMSAARtvHandle = m_pRtvHeap->Alloc();
	m_pMSAADsvHandle = m_pDsvHeap->Alloc();

	m_pDevice->CreateRenderTargetView(m_pMSAAColorTarget.Get(), &msaaRtvDesc, m_pMSAARtvHandle.HandleCPU());
	m_pDevice->CreateDepthStencilView(m_pMSAADepthTarget.Get(), &msaaDsvDesc, m_pMSAADsvHandle.HandleCPU());

	return true;
}

bool Engine::CreateGBuffer()
{
	// SRV�p�̃f�B�X�N���v�^�q�[�v���쐬
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{};
	srvHeapDesc.NodeMask = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.NumDescriptors = 32; //6 + 1 + 1;	// GBuffer * 6 + ShadowMap + Skybox
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	m_pGBufferHeap = std::make_unique<DescriptorHeap>(srvHeapDesc);
	if (!m_pGBufferHeap->IsValid())
		return false;
 
	// GBufferManager�̐���
	m_pGBufferManager = std::make_unique<GBufferManager>();

	GBufferProperty propRGBA{
		DXGI_FORMAT_R8G8B8A8_UNORM,
		m_FrameBufferWidth, m_FrameBufferHeight,
		m_pGBufferHeap.get(), m_pRtvHeap.get()
	};

	GBufferProperty propFloat{
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		m_FrameBufferWidth, m_FrameBufferHeight,
		m_pGBufferHeap.get(), m_pRtvHeap.get()
	};

	// GBuffer�̍쐬
	m_pGBufferManager->CreateGBuffer("Position", propFloat);
	m_pGBufferManager->CreateGBuffer("Normal", propFloat);
	m_pGBufferManager->CreateGBuffer("Albedo", propRGBA);
	m_pGBufferManager->CreateGBuffer("MetallicRoughness", propRGBA);
	m_pGBufferManager->CreateGBuffer("Depth", propFloat);
	m_pGBufferManager->CreateGBuffer("Lighting", propRGBA);
	m_pGBufferManager->CreateGBuffer("SSAO", propFloat);
	m_pGBufferManager->CreateGBuffer("BlurredSSAO1", propFloat);
	m_pGBufferManager->CreateGBuffer("BlurredSSAO2", propFloat);
	m_pGBufferManager->CreateGBuffer("PostProcess", propRGBA);
}

bool Engine::CreateD2DRenderTarget()
{
	D3D11_RESOURCE_FLAGS flags = { D3D11_BIND_RENDER_TARGET };

	float dpi = GetDpiForWindow(m_hWnd);
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		static_cast<float>(dpi),
		static_cast<float>(dpi)
	);

	for (UINT i = 0U; i < FRAME_BUFFER_COUNT; ++i)
	{
		ComPtr<ID3D11Resource> wrappedBackBuffer = nullptr;
		
		auto hr = m_pD3d11On12Device->CreateWrappedResource(
			m_pRenderTargets[i].Get(),
			&flags,
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT,
			IID_PPV_ARGS(wrappedBackBuffer.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr))
		{
			return false;
		}

		ComPtr<IDXGISurface> dxgiSurface = nullptr;
		hr = wrappedBackBuffer.As(&dxgiSurface);
		if (FAILED(hr))
		{
			return false;
		}

		ComPtr<ID2D1Bitmap1> d2dRenderTarget = nullptr;
		hr = m_pD2dDeviceContext->CreateBitmapFromDxgiSurface(
			dxgiSurface.Get(),
			&bitmapProperties,
			&d2dRenderTarget
		);
		if (FAILED(hr))
		{
			return false;
		}

		m_pWrappedBackBuffers[i] = wrappedBackBuffer;
		m_pD2dRenderTargets[i] = d2dRenderTarget;
	}

	return true;
}

void Engine::RegisterSolidColorBrush(const std::string& key, const D2D1::ColorF color) noexcept
{
	if (m_solidColorBrushMap.contains(key))
	{
		return;
	}

	ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	m_pD2dDeviceContext->CreateSolidColorBrush(
		D2D1::ColorF(color),
		brush.GetAddressOf()
	);

	m_solidColorBrushMap[key] = brush;
}

void Engine::RegisterTextFormat(const std::string& key, const std::wstring& fontName, const FLOAT fontSize) noexcept
{
	if (m_textFormatMap.contains(key))
	{
		return;
	}

	ComPtr<IDWriteTextFormat> textFormat = nullptr;
	m_pDirectWriteFactory->CreateTextFormat(
		fontName.c_str(),
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fontSize,
		L"ja-jp",
		textFormat.GetAddressOf()
	);

	m_textFormatMap[key] = textFormat;
}

void Engine::WaitRender()
{
	// �`��I���҂�
	const UINT64 fenceValue = m_fenceValue[m_CurrentBackBufferIndex];
	m_pQueue->Signal(m_pFence.Get(), fenceValue);
	m_fenceValue[m_CurrentBackBufferIndex]++;

	// ���̃t���[���̕`�揀�����܂��ł���Αҋ@����
	if (m_pFence->GetCompletedValue() < fenceValue)
	{
		// �������ɃC�x���g��ݒ�
		auto hr = m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		if (FAILED(hr))
		{
			return;
		}

		// �ҋ@����
		if (WAIT_OBJECT_0 != WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE))
		{
			return;
		}
	}
}

GBufferManager* Engine::GetGBufferManager()
{
	return m_pGBufferManager.get();
}

ShadowMap* Engine::GetShadowMap()
{
	return m_pShadowMap.get();
}



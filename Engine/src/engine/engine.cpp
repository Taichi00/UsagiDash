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
		printf("�f�o�C�X�̐����Ɏ��s\n");
		return false;
	}

	if (!CreateCommandQueue())
	{
		printf("�R�}���h�L���[�̐����Ɏ��s\n");
		return false;
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

	// �r���[�|�[�g�ƃV�U�[��`�𐶐�
	CreateViewPort();
	CreateScissorRect();

	if (!CreateDescriptorHeap())
	{
		printf("�f�B�X�N���v�^�q�[�v�̐����Ɏ��s\n");
	}

	if (!CreateRenderTarget())
	{
		printf("�����_�[�^�[�Q�b�g�̐����Ɏ��s\n");
		return false;
	}

	if (!CreateDepthStencil())
	{
		printf("�f�v�X�X�e���V���o�b�t�@�̐����Ɏ��s\n");
	}

	//if (!CreateMSAA())
	//{
	//	printf("MSAA���\�[�X�̐����Ɏ��s\n");
	//}

	// GBufferManager�̐���
	buffer_manager_ = std::make_unique<BufferManager>();

	if (!CreateGBuffer())
	{
		printf("G-Buffer�̐����Ɏ��s\n");
	}

	// 2D�`��G���W���̏�����
	engine2d_ = std::make_unique<Engine2D>();
	if (!engine2d_->Init())
	{
		return false;
	}

	// �V���h�E�}�b�v�̐���
	shadowmap_ = std::make_unique<ShadowMap>();

	printf("�`��G���W���̏������ɐ���\n");
	return true;
}

void Engine::BeginRender()
{
	// �r���[�|�[�g�ƃV�U�[��`��ݒ�
	command_list_->RSSetViewports(1, &viewport_);
	command_list_->RSSetScissorRects(1, &scissor_);

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = rtv_handles_[current_back_buffer_index_].HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// �����_�[�^�[�Q�b�g���g�p�\�ɂȂ�܂ő҂�
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		current_render_target_,
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	command_list_->ResourceBarrier(1, &barrier);

	// �����_�[�^�[�Q�b�g��ݒ�
	command_list_->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);

	// �����_�[�^�[�Q�b�g���N���A
	const float clearColor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	command_list_->ClearRenderTargetView(currentRtvHandle, clearColor, 0, nullptr);

	// �[�x�X�e���V���r���[���N���A
	command_list_->ClearDepthStencilView(currentDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Engine::InitRender()
{
	// ���݂̃����_�[�^�[�Q�b�g���X�V
	current_render_target_ = render_targets_[current_back_buffer_index_].Get();

	// �R�}���h�����������Ă��߂鏀��������
	allocator_[current_back_buffer_index_]->Reset();
	command_list_->Reset(allocator_[current_back_buffer_index_].Get(), nullptr);
}

void Engine::BeginRenderMSAA()
{
	// �r���[�|�[�g�ƃV�U�[��`��ݒ�
	command_list_->RSSetViewports(1, &viewport_);
	command_list_->RSSetScissorRects(1, &scissor_);

	// �����_�[�^�[�Q�b�g���g�p�\�ɂȂ�܂ő҂�
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		msaa_color_target_.Get(),
		D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
		D3D12_RESOURCE_STATE_RENDER_TARGET
	);
	command_list_->ResourceBarrier(1, &barrier);

	// �����_�[�^�[�Q�b�g��ݒ�
	auto rtvHandle = msaa_rtv_handle_.HandleCPU();
	auto dsvHandle = msaa_dsv_handle_.HandleCPU();
	command_list_->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// �����_�[�^�[�Q�b�g���N���A
	const float clearColor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	command_list_->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// �[�x�X�e���V���r���[���N���A
	command_list_->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Engine::EndRender()
{
	// �����_�[�^�[�Q�b�g�ɏ������ݏI���܂ő҂�
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
		current_render_target_,
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	command_list_->ResourceBarrier(1, &barrier);

	// �R�}���h�̋L�^���I��
	command_list_->Close();

	// �R�}���h�����s
	ID3D12CommandList* ppCmdLists[] = { command_list_.Get() };
	queue_->ExecuteCommandLists(1, ppCmdLists);

	// �X���b�v�`�F�[����؂�ւ�
	swapchain_->Present(1, 0);

	// �`�抮����҂�
	WaitGPU();

	// �o�b�N�o�b�t�@�ԍ��X�V
	current_back_buffer_index_ = swapchain_->GetCurrentBackBufferIndex();
}

void Engine::EndRenderMSAA()
{
	// �����_�[�^�[�Q�b�g�ɏ������ݏI���܂ő҂�
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

	// �����_�[�^�[�Q�b�g�ɏ������ݏI���܂ő҂�
	auto barrierResolveToPresent = CD3DX12_RESOURCE_BARRIER::Transition(
		current_render_target_,
		D3D12_RESOURCE_STATE_RESOLVE_DEST,
		D3D12_RESOURCE_STATE_PRESENT
	);
	command_list_->ResourceBarrier(1, &barrierResolveToPresent);

	// �R�}���h�̋L�^���I��
	command_list_->Close();

	// �R�}���h�����s
	ID3D12CommandList* ppCmdLists[] = { command_list_.Get() };
	queue_->ExecuteCommandLists(1, ppCmdLists);

	// �X���b�v�`�F�[����؂�ւ�
	swapchain_->Present(1, 0);

	// �`�抮����҂�
	WaitGPU();

	// �o�b�N�o�b�t�@�ԍ��X�V
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

	// �R�}���h�̋L�^���I��
	command_list_->Close();

	// �R�}���h�����s
	ID3D12CommandList* ppCmdLists[] = { command_list_.Get() };
	queue_->ExecuteCommandLists(1, ppCmdLists);

	// �`�抮����҂�
	WaitGPU();
}

void Engine::BeginDeferredRender()
{
	// �r���[�|�[�g�ƃV�U�[��`��ݒ�
	command_list_->RSSetViewports(1, &viewport_);
	command_list_->RSSetScissorRects(1, &scissor_);

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = rtv_handles_[current_back_buffer_index_].HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// �����_�[�^�[�Q�b�g���g�p�\�ɂȂ�܂ő҂�
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			current_render_target_,
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET),
	};
	command_list_->ResourceBarrier(_countof(barriers), barriers);

	// �����_�[�^�[�Q�b�g���N���A
	const float clearColor[] = { 0.9f, 0.9f, 0.9f, 1.0f };
	const float zeroFloat[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	const float oneFloat[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const float zeroAlbedo[] = { 0, 0, 0, 1 };	// A�̓A�E�g���C���}�X�N�Ȃ̂łP�ŏ�����
	command_list_->ClearRenderTargetView(currentRtvHandle, clearColor, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("Position")->RtvHandle().HandleCPU(), zeroFloat, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("Normal")->RtvHandle().HandleCPU(), zeroFloat, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("Albedo")->RtvHandle().HandleCPU(), zeroAlbedo, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("MetallicRoughness")->RtvHandle().HandleCPU(), zeroFloat, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("Depth")->RtvHandle().HandleCPU(), oneFloat, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("Lighting")->RtvHandle().HandleCPU(), zeroFloat, 0, nullptr);
	command_list_->ClearRenderTargetView(buffer_manager_->Get("PostProcess")->RtvHandle().HandleCPU(), zeroFloat, 0, nullptr);

	// �[�x�X�e���V���r���[���N���A
	command_list_->ClearDepthStencilView(currentDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void Engine::DepthPrePath()
{
	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = rtv_handles_[current_back_buffer_index_].HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
	command_list_->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::GBufferPath()
{
	// �����_�[�^�[�Q�b�g��ݒ�
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
	// �����_�[�^�[�Q�b�g�ɏ������ݏI���܂ő҂�
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

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = buffer_manager_->Get("Lighting")->RtvHandle().HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
	command_list_->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::SSAOPath()
{
	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = buffer_manager_->Get("SSAO")->RtvHandle().HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
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

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = buffer_manager_->Get("BlurredSSAO1")->RtvHandle().HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
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

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = buffer_manager_->Get("BlurredSSAO2")->RtvHandle().HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
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
	
	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = buffer_manager_->Get("PostProcess")->RtvHandle().HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
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

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = rtv_handles_[current_back_buffer_index_].HandleCPU();

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = dsv_handle_.HandleCPU();

	// �����_�[�^�[�Q�b�g��ݒ�
	command_list_->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);
}

void Engine::EndDeferredRender()
{
	//// �R�}���h�̋L�^���I��
	//command_list_->Close();

	//// �R�}���h�����s
	//ID3D12CommandList* ppCmdLists[] = { command_list_.Get() };
	//queue_->ExecuteCommandLists(1, ppCmdLists);

	// �X���b�v�`�F�[����؂�ւ�
	swapchain_->Present(1, 0);

	// �`�抮����҂�
	WaitGPU();

	// �o�b�N�o�b�t�@�ԍ��X�V
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

	// �]������
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
		printf("���_�o�b�t�@�̐����Ɏ��s\n");
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
		printf("�C���f�b�N�X�o�b�t�@�̐����Ɏ��s\n");
		return nullptr;
	}

	return std::move(ib);
}

std::unique_ptr<ConstantBuffer> Engine::CreateConstantBuffer(const unsigned long long size)
{
	auto cb = std::make_unique<ConstantBuffer>(size);
	if (!cb->IsValid())
	{
		printf("�萔�o�b�t�@�̐����Ɏ��s\n");
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
	desc.NodeMask = 0; // �ǂ�GPU�����̃f�B�X�N���v�^�q�[�v�����w��iGPU�P�̏ꍇ�͂O�j
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = num;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	auto dh = std::make_unique<DescriptorHeap>(desc);
	if (!dh->IsValid())
	{
		printf("�f�B�X�N���v�^�q�[�v�̐����Ɏ��s\n");
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
		throw std::runtime_error("�o�b�N�o�b�t�@�̃T�C�Y�ύX�Ɏ��s");
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
	// DXGI�t�@�N�g���[�̐���
	IDXGIFactory4* pFactory = nullptr;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	if (FAILED(hr))
	{
		return false;
	}

	// �X���b�v�`�F�C���̐���
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

	// IDXGISwapChain3���擾
	hr = pSwapChain->QueryInterface(IID_PPV_ARGS(swapchain_.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		pFactory->Release();
		pSwapChain->Release();
		return false;
	}


	// Alt-Enter�Ńt���X�N���[����h�~
	pFactory->MakeWindowAssociation(hwnd_, DXGI_MWA_NO_ALT_ENTER);

	// �o�b�N�o�b�t�@�ԍ����擾
	current_back_buffer_index_ = swapchain_->GetCurrentBackBufferIndex();

	//swapchain_->SetFullscreenState(TRUE, NULL);
	
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

	// �R�}���h���X�g�̐���
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

	// �R�}���h���X�g�͊J����Ă����Ԃō쐬�����̂ŁA�����������
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

	// �������s���Ƃ��̃C�x���g�n���h�����쐬����
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

	// �f�B�X�N���v�^���쐬
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

	// MSAA �`���o�b�t�@�i�J���[�j�̏���
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

	// MSAA �`���o�b�t�@�i�f�v�X�j�̏���
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

	// MSAA �ɏo�͂��邽�߂̃r���[����������
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
	// GBuffer�̐���
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

	//// GBuffer�̍쐬
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
	// RTV�p�̃f�B�X�N���v�^�q�[�v���쐬����
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

	// DSV�p�̃f�B�X�N���v�^�q�[�v���쐬����
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

	// SRV�p�̃f�B�X�N���v�^�q�[�v���쐬
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
	// �`��I���҂�
	const UINT64 fenceValue = fence_value_[current_back_buffer_index_];
	queue_->Signal(fence_.Get(), fenceValue);
	fence_value_[current_back_buffer_index_]++;

	// ���̃t���[���̕`�揀�����܂��ł���Αҋ@����
	if (fence_->GetCompletedValue() < fenceValue)
	{
		// �������ɃC�x���g��ݒ�
		auto hr = fence_->SetEventOnCompletion(fenceValue, fence_event_);
		if (FAILED(hr))
		{
			return;
		}

		// �ҋ@����
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

	//	// ���̃t���[���̕`�揀�����܂��ł���Αҋ@����
	//	if (fence_->GetCompletedValue() < fenceValue)
	//	{
	//		// �������ɃC�x���g��ݒ�
	//		auto hr = fence_->SetEventOnCompletion(fenceValue, fence_event_);
	//		if (FAILED(hr))
	//		{
	//			return;
	//		}

	//		// �ҋ@����
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



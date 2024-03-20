#include "game/shadow_map.h"
#include "engine/engine.h"
#include "engine/descriptor_heap.h"
#include "engine/root_signature.h"
#include "engine/pipeline_state.h"
#include "engine/buffer_manager.h"
#include "engine/pipeline_state_manager.h"
#include "game/game.h"

ShadowMap::ShadowMap()
{
	if (!CreateShadowBuffer())
	{
		printf("ShadowMap: Buffer�̐����Ɏ��s\n");
		return;
	}

	if (!PrepareRootSignature())
	{
		printf("ShadowMap: RootSignature�̐����Ɏ��s\n");
		return;
	}

	if (!PreparePSO())
	{
		printf("ShadowMap: PSO�̐����Ɏ��s\n");
		return;
	}
}

ShadowMap::~ShadowMap()
{
}

void ShadowMap::BeginRender()
{
	auto engine = Game::Get()->GetEngine();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto bm = Game::Get()->GetEngine()->GetBufferManager();

	// �����_�[�^�[�Q�b�g��ݒ�
	auto rtvHandle = bm->Get("ShadowMapColor_0")->RtvHandle().HandleCPU();
	auto dsvHandle = bm->Get("ShadowMapDepth")->DsvHandle().HandleCPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// �����_�[�^�[�Q�b�g���N���A
	const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	bm->Get("ShadowMapColor_0")->Clear();
	bm->Get("ShadowMapColor_1")->Clear();
	
	// �[�x�X�e���V���r���[���N���A
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// �r���[�|�[�g�ƃV�U�[��`��ݒ�
	auto viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, float(width_), float(height_));
	auto scissor = CD3DX12_RECT(0, 0, LONG(width_), LONG(height_));
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);

}

void ShadowMap::EndRender()
{	
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto bm = Game::Get()->GetEngine()->GetBufferManager();

	// �o���A�ݒ�
	auto barrierToSR = CD3DX12_RESOURCE_BARRIER::Transition(
		bm->Get("ShadowMapColor_0")->Resource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);
	commandList->ResourceBarrier(1, &barrierToSR);

	// �u���[
	BlurHorizontal();
	BlurVertical();

	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			bm->Get("ShadowMapColor_0")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		),
	};
	commandList->ResourceBarrier(_countof(barriers), barriers);
}

ID3D12Resource* ShadowMap::Resource()
{
	auto bm = Game::Get()->GetEngine()->GetBufferManager();
	return bm->Get("ShadowMapColor_0")->Resource();
}

const DescriptorHandle& ShadowMap::SrvHandle()
{
	auto bm = Game::Get()->GetEngine()->GetBufferManager();
	return bm->Get("ShadowMapColor_0")->SrvHandle();
}

bool ShadowMap::CreateShadowBuffer()
{
	auto bm = Game::Get()->GetEngine()->GetBufferManager();

	bm->CreateGBuffer("ShadowMapColor_0", DXGI_FORMAT_R32G32B32A32_FLOAT, { 1, 1, 1, 1 }, width_, height_);
	bm->CreateGBuffer("ShadowMapColor_1", DXGI_FORMAT_R32G32B32A32_FLOAT, { 1, 1, 1, 1 }, width_, height_);
	bm->CreateDepthStencilBuffer("ShadowMapDepth", width_, height_);

	return true;
}

bool ShadowMap::PreparePSO()
{
	auto pm = Game::Get()->GetEngine()->GetPipelineStateManager();

	// BlurHorizontal�p
	blur_horizontal_pso_ = pm->Create("GaussianBlurHorizontal");
	blur_horizontal_pso_->SetInputLayout({ nullptr, 0 });
	blur_horizontal_pso_->SetRootSignature(root_signature_->Get());
	blur_horizontal_pso_->SetVS(L"screen_vs.cso");
	blur_horizontal_pso_->SetPS(L"gaussian_blur_horizontal.cso");
	blur_horizontal_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	blur_horizontal_pso_->SetRTVFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);

	auto desc = blur_horizontal_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	blur_horizontal_pso_->Create();
	if (!blur_horizontal_pso_->IsValid())
	{
		return false;
	}

	// BlurVertical�p
	blur_vertical_pso_ = pm->Create("GaussianBlurVertical");
	blur_vertical_pso_->SetInputLayout({ nullptr, 0 });
	blur_vertical_pso_->SetRootSignature(root_signature_->Get());
	blur_vertical_pso_->SetVS(L"screen_vs.cso");
	blur_vertical_pso_->SetPS(L"gaussian_blur_vertical.cso");
	blur_vertical_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	blur_vertical_pso_->SetRTVFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);

	desc = blur_vertical_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	blur_vertical_pso_->Create();
	if (!blur_vertical_pso_->IsValid())
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

	root_signature_ = new RootSignature(_countof(params), params);
	if (!root_signature_->IsValid())
	{
		return false;
	}

	return true;
}

void ShadowMap::BlurHorizontal()
{
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto bm = Game::Get()->GetEngine()->GetBufferManager();

	// �����_�[�^�[�Q�b�g��ݒ�
	auto rtvHandle = bm->Get("ShadowMapColor_1")->RtvHandle().HandleCPU();
	auto dsvHandle = bm->Get("ShadowMapDepth")->DsvHandle().HandleCPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootSignature(root_signature_->Get());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		Game::Get()->GetEngine()->SrvHeap()->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(blur_horizontal_pso_->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(0, bm->Get("ShadowMapColor_0")->SrvHandle().HandleGPU()); // �f�B�X�N���v�^�e�[�u�����Z�b�g

	commandList->DrawInstanced(4, 1, 0, 0);
}

void ShadowMap::BlurVertical()
{
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto bm = Game::Get()->GetEngine()->GetBufferManager();

	// �����_�[�^�[�Q�b�g���g�p�\�ɂȂ�܂ő҂�
	D3D12_RESOURCE_BARRIER barriers[] = {
		CD3DX12_RESOURCE_BARRIER::Transition(
			bm->Get("ShadowMapColor_1")->Resource(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
		),
		CD3DX12_RESOURCE_BARRIER::Transition(
			bm->Get("ShadowMapColor_0")->Resource(),
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		),
	};
	commandList->ResourceBarrier(_countof(barriers), barriers);

	// �����_�[�^�[�Q�b�g��ݒ�
	auto rtvHandle = bm->Get("ShadowMapColor_0")->RtvHandle().HandleCPU();
	auto dsvHandle = bm->Get("ShadowMapDepth")->DsvHandle().HandleCPU();
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	commandList->SetPipelineState(blur_vertical_pso_->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(0, bm->Get("ShadowMapColor_1")->SrvHandle().HandleGPU()); // �f�B�X�N���v�^�e�[�u�����Z�b�g

	commandList->DrawInstanced(4, 1, 0, 0);
}

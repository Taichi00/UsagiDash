#include "particle_renderer_opaque.h"
#include "game/game.h"
#include "engine/engine.h"
#include "game/component/particle_emitter.h"
#include "engine/constant_buffer.h"
#include "engine/root_signature.h"
#include "engine/pipeline_state.h"
#include "engine/vertex_buffer.h"
#include "engine/index_buffer.h"
#include "game/component/camera.h"
#include "math/vec.h"
#include "math/quaternion.h"

void ParticleRendererOpaque::DrawShadow()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();

	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootSignature(root_signature_->Get());

	// �萔�o�b�t�@���Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, particle_cb_[currentIndex]->GetAddress());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetPipelineState(shadow_pso_->Get());

	// �f�B�X�N���v�^�q�[�v���Z�b�g
	ID3D12DescriptorHeap* heaps[] = {
		particle_model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);

	// ���b�V���̕`��
	for (auto& mesh : particle_model_->meshes)
	{
		auto vbView = mesh.vertex_buffer->View();
		auto ibView = mesh.index_buffer->View();
		auto mat = &particle_model_->materials[mesh.material_index];

		commandList->IASetVertexBuffers(0, 1, &vbView);
		commandList->IASetIndexBuffer(&ibView);
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());

		commandList->DrawIndexedInstanced((UINT)mesh.indices.size(), MAX_PARTICLE_COUNT, 0, 0, 0);
	}
}

void ParticleRendererOpaque::DrawDepth()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();

	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootSignature(root_signature_->Get());

	// �萔�o�b�t�@���Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, particle_cb_[currentIndex]->GetAddress());

	// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetPipelineState(depth_pso_->Get());

	// �f�B�X�N���v�^�q�[�v���Z�b�g
	ID3D12DescriptorHeap* heaps[] = {
		particle_model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);

	// ���b�V���̕`��
	for (auto& mesh : particle_model_->meshes)
	{
		auto vbView = mesh.vertex_buffer->View();
		auto ibView = mesh.index_buffer->View();
		auto mat = &particle_model_->materials[mesh.material_index];

		commandList->IASetVertexBuffers(0, 1, &vbView);
		commandList->IASetIndexBuffer(&ibView);
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());

		commandList->DrawIndexedInstanced((UINT)mesh.indices.size(), MAX_PARTICLE_COUNT, 0, 0, 0);
	}
}

void ParticleRendererOpaque::DrawGBuffer()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();

	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootSignature(root_signature_->Get());

	// �萔�o�b�t�@���Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, particle_cb_[currentIndex]->GetAddress());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetPipelineState(gbuffer_pso_->Get());

	// �f�B�X�N���v�^�q�[�v���Z�b�g
	ID3D12DescriptorHeap* heaps[] = {
		particle_model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);

	// ���b�V���̕`��
	for (auto& mesh : particle_model_->meshes)
	{
		auto vbView = mesh.vertex_buffer->View();
		auto ibView = mesh.index_buffer->View();
		auto mat = &particle_model_->materials[mesh.material_index];

		commandList->IASetVertexBuffers(0, 1, &vbView);
		commandList->IASetIndexBuffer(&ibView);
		commandList->SetGraphicsRootConstantBufferView(3, materials_cb_[mesh.material_index]->GetAddress());
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());
		commandList->SetGraphicsRootDescriptorTable(5, mat->pbr_handle.HandleGPU());
		commandList->SetGraphicsRootDescriptorTable(6, mat->normal_handle.HandleGPU());

		commandList->DrawIndexedInstanced((UINT)mesh.indices.size(), MAX_PARTICLE_COUNT, 0, 0, 0);
	}
}

bool ParticleRendererOpaque::PrepareRootSignature()
{
	RootSignatureParameter params[] = {
		RSConstantBuffer,
		RSConstantBuffer,
		RSConstantBuffer,
		RSConstantBuffer,
		RSTexture,
		RSTexture,
		RSTexture,
	};

	root_signature_ = std::make_unique<RootSignature>((int)_countof(params), params);
	if (!root_signature_->IsValid())
	{
		printf("���[�g�V�O�l�`���̐����Ɏ��s\n");
		return false;
	}

	return true;
}

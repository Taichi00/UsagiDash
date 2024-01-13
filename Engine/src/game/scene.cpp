#include "game/scene.h"
#include "engine/engine.h"
#include "game/shadow_map.h"
#include "game/entity.h"
#include "game/component/camera.h"
#include "game/collision_manager.h"
#include "game/component/collider/collider.h"
#include "engine/pipeline_state.h"
#include "engine/shared_struct.h"
#include "engine/root_signature.h"
#include "engine/descriptor_heap.h"
#include "engine/constant_buffer.h"
#include "engine/texture2d.h"
#include "engine/vertex_buffer.h"
#include "engine/index_buffer.h"
#include "engine/gbuffer.h"
#include "engine/gbuffer_manager.h"
#include "game/model.h"
#include "game/game.h"
#include "game/resource_manager.h"
#include "engine/engine2d.h"

Scene::Scene()
{
}

Scene::~Scene()
{
	printf("Delete Scene\n");

	gbuffer_heap_->Free(skybox_handle_);
	gbuffer_heap_->Free(diffusemap_handle_);
	gbuffer_heap_->Free(specularmap_handle_);
	gbuffer_heap_->Free(brdf_handle_);
}

bool Scene::Init()
{
	// �G���e�B�e�B�̃N���A
	entities_.clear();

	// �V���h�E�}�b�v�̐���
	//m_pShadowMap = std::make_unique<ShadowMap>();

	// CollisionManager�̐���
	collision_manager_ = std::make_unique<CollisionManager>();

	rtv_heap_ = Game::Get()->GetEngine()->RtvHeap();
	dsv_heap_ = Game::Get()->GetEngine()->DsvHeap();
	gbuffer_heap_ = Game::Get()->GetEngine()->GBufferHeap();

	// ���[�g�V�O�l�`���̐���
	RootSignatureParameter params[] = {
		RSConstantBuffer,	// TransformParameter
		RSConstantBuffer,	// SceneParameter
		RSTexture,			// Position
		RSTexture,			// Normal
		RSTexture,			// Albedo
		RSTexture,			// MetallicRoughness
		RSTexture,			// Depth
		RSTexture,			// ShadowMap
		RSTexture,			// Skybox
		RSTexture,			
		RSTexture,			
	};
	root_signature_ = std::make_unique<RootSignature>(_countof(params), params);
	if (!root_signature_->IsValid())
	{
		printf("���[�g�V�O�l�`���̐����Ɏ��s\n");
		return false;
	}

	RootSignatureParameter skyboxParams[] = {
		RSConstantBuffer,	// TransformParameter
		RSConstantBuffer,	// SceneParameter
		RSTexture,			// Skybox
	};
	skybox_root_signature_ = std::make_unique<RootSignature>(_countof(skyboxParams), skyboxParams);
	if (!skybox_root_signature_->IsValid())
	{
		printf("���[�g�V�O�l�`���̐����Ɏ��s\n");
		return false;
	}

	// PipelineStateObject�̐���
	if (!PreparePSO())
	{
		printf("Scene�pPSO�̐����Ɏ��s���܂���\n");
	}

	// �萔�o�b�t�@�̐���
	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		transform_cb_[i] = std::make_unique<ConstantBuffer>(sizeof(TransformParameter));
		if (!transform_cb_[i]->IsValid())
		{
			printf("�ϊ��s��p�萔�o�b�t�@�̐����Ɏ��s\n");
			return false;
		}
	}

	const auto eye = XMVectorSet(0.5f, 7.5f, 2.5f, 0.0f);
	const auto target = XMVectorSet(0.0f, 4.0f, 0.0f, 0.0f);
	const auto up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	auto dir = Vec3(0.0f - 0.5f, 4.0f - 7.5f, 0.0f - 2.5f).Normalized();

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		scene_cb_[i] = std::make_unique<ConstantBuffer>(sizeof(SceneParameter));
		if (!scene_cb_[i]->IsValid())
		{
			printf("���C�g�p�萔�o�b�t�@�̐����Ɏ��s\n");
			return false;
		}

		auto ptr = scene_cb_[i]->GetPtr<SceneParameter>();
		ptr->light_direction = { dir.x, dir.y, dir.z };
		ptr->light_view = XMMatrixLookAtRH(eye, target, up);
		ptr->light_proj = XMMatrixOrthographicRH(45, 45, 0.1f, 100.0f);
		ptr->camera_position = { 0, 0, 1 };
	}

    return true;
}

void Scene::Update()
{
	// Camera�̍X�V
	for (auto& entity : entities_) entity->BeforeCameraUpdate();
	for (auto& entity : entities_) entity->CameraUpdate();

	// �G���e�B�e�B�̍X�V
	for (auto& entity : entities_) entity->Update();

	// �����̍X�V
	for (auto& entity : entities_) entity->PhysicsUpdate();

	// �Փ˔���
	collision_manager_->Update();

	// �萔�o�b�t�@�̍X�V
	UpdateCB();
}

void Scene::Draw()
{
	// �����_�����O�̏���
	Game::Get()->GetEngine()->InitRender();

	// �V���h�E�}�b�v�̕`��
	Game::Get()->GetEngine()->GetShadowMap()->BeginRender();
	for (auto& entity : entities_) entity->DrawShadow();
	Game::Get()->GetEngine()->GetShadowMap()->EndRender();

	/*Game::Get()->GetEngine()->BeginRenderMSAA();

	for (auto entity : m_pEntities) entity->Draw();
	for (auto entity : m_pEntities) entity->DrawAlpha();

	Game::Get()->GetEngine()->EndRenderMSAA();*/

	// �����_�����O�̊J�n
	Game::Get()->GetEngine()->BeginDeferredRender();

	// �f�v�X�v���p�X
	Game::Get()->GetEngine()->DepthPrePath();
	for (auto& entity : entities_) entity->DrawDepth();

	// G-Buffer�ւ̏�������
	Game::Get()->GetEngine()->GBufferPath();
	for (auto& entity : entities_) entity->DrawGBuffer();

	// ���C�e�B���O�p�X
	Game::Get()->GetEngine()->LightingPath();
	DrawLighting();

	// Skybox�`��i�t�H���[�h�����_�����O�j
	DrawSkybox();

	// SSAO
	//Game::Get()->GetEngine()->SSAOPath();
	//DrawSSAO();

	//// �ڂ���
	//Game::Get()->GetEngine()->BlurHorizontalPath();
	//DrawBlurHorizontal();
	//Game::Get()->GetEngine()->BlurVerticalPath();
	//DrawBlurVertical();

	// �|�X�g�v���Z�X
	Game::Get()->GetEngine()->PostProcessPath();
	DrawPostProcess();

	// �A�E�g���C���`��i�t�H���[�h�����_�����O�j
	//for (auto entity : m_pEntities) entity->DrawOutline();

	// FXAA
	Game::Get()->GetEngine()->FXAAPath();
	DrawFXAA();

	Game::Get()->GetEngine()->EndRenderD3D();

	// 2D�`��
	Game::Get()->GetEngine()->GetEngine2D()->BeginRenderD2D();
	for (auto& entity : entities_) entity->Draw2D();
	Game::Get()->GetEngine()->GetEngine2D()->EndRenderD2D();

	// �����_�����O�̏I��
	Game::Get()->GetEngine()->EndDeferredRender();
}

void Scene::DrawLighting()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->GBufferHeap()->GetHeap();
	auto gbufferManager = Game::Get()->GetEngine()->GetGBufferManager();
	auto shadowMap = Game::Get()->GetEngine()->GetShadowMap();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, scene_cb_[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(lighting_pso_->Get());	// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("Position")->SrvHandle().HandleGPU());	// �f�B�X�N���v�^�e�[�u�����Z�b�g
	commandList->SetGraphicsRootDescriptorTable(3, gbufferManager->Get("Normal")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(4, gbufferManager->Get("Albedo")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(5, gbufferManager->Get("MetallicRoughness")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(6, gbufferManager->Get("Depth")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(7, shadowMap->SrvHandle()->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(8, diffusemap_handle_.HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(9, specularmap_handle_.HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(10, brdf_handle_.HandleGPU());

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawSkybox()
{
	if (skybox_tex_ == nullptr)
		return;

	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->GBufferHeap()->GetHeap();

	commandList->SetGraphicsRootSignature(skybox_root_signature_->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g
	//commandList->SetGraphicsRootConstantBufferView(1, m_pSceneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, skybox_handle_.HandleGPU());	// �f�B�X�N���v�^�e�[�u�����Z�b�g

	// ���b�V���̕`��
	auto vbView = skybox_mesh_.vertex_buffer->View();
	auto ibView = skybox_mesh_.index_buffer->View();

	commandList->SetPipelineState(skybox_pso_->Get());				// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->IASetVertexBuffers(0, 1, &vbView);					// ���_�o�b�t�@���X���b�g0�Ԃ��g����1�����ݒ肷��
	commandList->IASetIndexBuffer(&ibView);							// �C���f�b�N�X�o�b�t�@���Z�b�g

	commandList->DrawIndexedInstanced(skybox_mesh_.indices.size(), 1, 0, 0, 0);
}

void Scene::DrawSSAO()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->GBufferHeap()->GetHeap();
	auto gbufferManager = Game::Get()->GetEngine()->GetGBufferManager();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(ssao_pso_->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("Position")->SrvHandle().HandleGPU()); // �f�B�X�N���v�^�e�[�u�����Z�b�g
	commandList->SetGraphicsRootDescriptorTable(3, gbufferManager->Get("Normal")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(4, gbufferManager->Get("Albedo")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(5, gbufferManager->Get("Depth")->SrvHandle().HandleGPU());

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawBlurHorizontal()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->GBufferHeap()->GetHeap();
	auto gbufferManager = Game::Get()->GetEngine()->GetGBufferManager();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(blur_horizontal_pso_->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("SSAO")->SrvHandle().HandleGPU()); // �f�B�X�N���v�^�e�[�u�����Z�b�g

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawBlurVertical()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->GBufferHeap()->GetHeap();
	auto gbufferManager = Game::Get()->GetEngine()->GetGBufferManager();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(blur_vertical_pso_->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("BlurredSSAO1")->SrvHandle().HandleGPU()); // �f�B�X�N���v�^�e�[�u�����Z�b�g

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawPostProcess()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->GBufferHeap()->GetHeap();
	auto gbufferManager = Game::Get()->GetEngine()->GetGBufferManager();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(postprocess_pso_->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("Lighting")->SrvHandle().HandleGPU());	// �f�B�X�N���v�^�e�[�u�����Z�b�g
	commandList->SetGraphicsRootDescriptorTable(3, gbufferManager->Get("Normal")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(4, gbufferManager->Get("Albedo")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(5, gbufferManager->Get("Depth")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(6, gbufferManager->Get("Position")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(7, gbufferManager->Get("BlurredSSAO2")->SrvHandle().HandleGPU());

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawFXAA()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->GBufferHeap()->GetHeap();
	auto gbufferManager = Game::Get()->GetEngine()->GetGBufferManager();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(fxaa_pso_->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("PostProcess")->SrvHandle().HandleGPU());	// �f�B�X�N���v�^�e�[�u�����Z�b�g

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::CreateEntity(Entity* entity)
{
	std::unique_ptr<Entity> uentity;
	uentity.reset(entity);
	entities_.push_back(std::move(uentity));

	entity->RegisterScene(this);
	entity->Init();
}

void Scene::SetMainCamera(Entity* camera)
{
	main_camera_ = camera->GetComponent<Camera>();
}

Camera* Scene::GetMainCamera()
{
	return main_camera_;
}

//ShadowMap* Scene::GetShadowMap()
//{
//	return m_pShadowMap.get();
//}

CollisionManager* Scene::GetCollisionManager()
{
	return collision_manager_.get();
}

void Scene::SetSkybox(const std::string path)
{
	auto device = Game::Get()->GetEngine()->Device();

	skybox_tex_ = Texture2D::Load(path + "EnvHDR.dds");

	// SRV���쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC skyboxDesc{};
	skyboxDesc.Format = skybox_tex_->Format();
	skyboxDesc.TextureCube.MipLevels = skybox_tex_->Metadata().mipLevels;
	skyboxDesc.TextureCube.MostDetailedMip = 0;
	skyboxDesc.TextureCube.ResourceMinLODClamp = 0;
	skyboxDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	skyboxDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

	skybox_handle_ = Game::Get()->GetEngine()->GBufferHeap()->Alloc();	// GBufferHeap�ɒǉ�
	device->CreateShaderResourceView(skybox_tex_->Resource(), &skyboxDesc, skybox_handle_.HandleCPU());	// �V�F�[�_�[���\�[�X�r���[�쐬

	// DiffuseMap
	diffusemap_tex_ = Texture2D::Load(path + "DiffuseHDR.dds");

	D3D12_SHADER_RESOURCE_VIEW_DESC diffuseDesc{};
	diffuseDesc.Format = diffusemap_tex_->Format();
	diffuseDesc.TextureCube.MipLevels = diffusemap_tex_->Metadata().mipLevels;
	diffuseDesc.TextureCube.MostDetailedMip = 0;
	diffuseDesc.TextureCube.ResourceMinLODClamp = 0;
	diffuseDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	diffuseDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

	diffusemap_handle_ = Game::Get()->GetEngine()->GBufferHeap()->Alloc();
	device->CreateShaderResourceView(diffusemap_tex_->Resource(), &diffuseDesc, diffusemap_handle_.HandleCPU());

	// SpecularMap
	specularmap_tex_ = Texture2D::Load(path + "SpecularHDR.dds");

	D3D12_SHADER_RESOURCE_VIEW_DESC specularDesc{};
	specularDesc.Format = specularmap_tex_->Format();
	specularDesc.TextureCube.MipLevels = specularmap_tex_->Metadata().mipLevels;
	specularDesc.TextureCube.MostDetailedMip = 0;
	specularDesc.TextureCube.ResourceMinLODClamp = 0;
	specularDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	specularDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

	specularmap_handle_ = Game::Get()->GetEngine()->GBufferHeap()->Alloc();
	device->CreateShaderResourceView(specularmap_tex_->Resource(), &specularDesc, specularmap_handle_.HandleCPU());

	// brdfLUT
	brdf_tex_ = Texture2D::Load(path + "Brdf.dds");

	D3D12_SHADER_RESOURCE_VIEW_DESC brdfDesc{};
	brdfDesc.Format = brdf_tex_->Format();
	brdfDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	brdfDesc.Texture2D.MipLevels = brdf_tex_->Metadata().mipLevels;
	brdfDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	brdf_handle_ = Game::Get()->GetEngine()->GBufferHeap()->Alloc();
	device->CreateShaderResourceView(brdf_tex_->Resource(), &brdfDesc, brdf_handle_.HandleCPU());


	// Mesh���쐬
	float size = 500;
	std::vector<Vertex> vertices;
	vertices.resize(8);
	vertices[0].position = { -1 * size, -1 * size, -1 * size };
	vertices[1].position = {  1 * size, -1 * size, -1 * size };
	vertices[2].position = { -1 * size,  1 * size, -1 * size };
	vertices[3].position = {  1 * size,  1 * size, -1 * size };
	vertices[4].position = { -1 * size, -1 * size,  1 * size };
	vertices[5].position = {  1 * size, -1 * size,  1 * size };
	vertices[6].position = { -1 * size,  1 * size,  1 * size };
	vertices[7].position = {  1 * size,  1 * size,  1 * size };

	std::vector<uint32_t> indices = {
		3, 2, 0, 1, 3, 0,
		2, 6, 4, 0, 2, 4,
		5, 4, 6, 7, 5, 6,
		1, 5, 7, 3, 1, 7,
		3, 7, 2, 7, 6, 2,
		5, 1, 0, 4, 5, 0
	};

	skybox_mesh_ = Mesh{};
	skybox_mesh_.vertices = vertices;
	skybox_mesh_.indices = indices;

	// ���_�o�b�t�@�̐���
	auto vSize = sizeof(Vertex) * skybox_mesh_.vertices.size();
	auto stride = sizeof(Vertex);
	auto pVB = std::make_unique<VertexBuffer>(vSize, stride, vertices.data());
	if (!pVB->IsValid())
	{
		printf("���_�o�b�t�@�̐����Ɏ��s\n");
		return;
	}
	skybox_mesh_.vertex_buffer = std::move(pVB);

	// �C���f�b�N�X�o�b�t�@�̐���
	auto iSize = sizeof(uint32_t) * skybox_mesh_.indices.size();
	auto pIB = std::make_unique<IndexBuffer>(iSize, indices.data());
	if (!pIB->IsValid())
	{
		printf("�C���f�b�N�X�o�b�t�@�̐����Ɏ��s\n");
		return;
	}
	skybox_mesh_.index_buffer = std::move(pIB);
}

bool Scene::PreparePSO()
{
	// ���C�e�B���O�p
	lighting_pso_ = std::make_unique<PipelineState>();
	lighting_pso_->SetInputLayout({nullptr, 0});
	lighting_pso_->SetRootSignature(root_signature_->Get());
	lighting_pso_->SetVS(L"ScreenVS.cso");
	lighting_pso_->SetPS(L"PBR2.cso");
	lighting_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	auto desc = lighting_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;
	
	lighting_pso_->Create();
	if (!lighting_pso_->IsValid())
	{
		return false;
	}

	// SSAO�p
	ssao_pso_ = std::make_unique<PipelineState>();
	ssao_pso_->SetInputLayout({ nullptr, 0 });
	ssao_pso_->SetRootSignature(root_signature_->Get());
	ssao_pso_->SetVS(L"ScreenVS.cso");
	ssao_pso_->SetPS(L"SSAO.cso");
	ssao_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = ssao_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	ssao_pso_->Create();
	if (!ssao_pso_->IsValid())
	{
		return false;
	}

	// BlurHorizontal�p
	blur_horizontal_pso_ = std::make_unique<PipelineState>();
	blur_horizontal_pso_->SetInputLayout({ nullptr, 0 });
	blur_horizontal_pso_->SetRootSignature(root_signature_->Get());
	blur_horizontal_pso_->SetVS(L"ScreenVS.cso");
	blur_horizontal_pso_->SetPS(L"GaussianBlurHorizontal.cso");
	blur_horizontal_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = blur_horizontal_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	blur_horizontal_pso_->Create();
	if (!blur_horizontal_pso_->IsValid())
	{
		return false;
	}

	// BlurVertical�p
	blur_vertical_pso_ = std::make_unique<PipelineState>();
	blur_vertical_pso_->SetInputLayout({ nullptr, 0 });
	blur_vertical_pso_->SetRootSignature(root_signature_->Get());
	blur_vertical_pso_->SetVS(L"ScreenVS.cso");
	blur_vertical_pso_->SetPS(L"GaussianBlurVertical.cso");
	blur_vertical_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = blur_vertical_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	blur_vertical_pso_->Create();
	if (!blur_vertical_pso_->IsValid())
	{
		return false;
	}

	// PostProcess�p
	postprocess_pso_ = std::make_unique<PipelineState>();
	postprocess_pso_->SetInputLayout({ nullptr, 0 });
	postprocess_pso_->SetRootSignature(root_signature_->Get());
	postprocess_pso_->SetVS(L"ScreenVS.cso");
	postprocess_pso_->SetPS(L"PostProcessPS.cso");
	postprocess_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = postprocess_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	postprocess_pso_->Create();
	if (!postprocess_pso_->IsValid())
	{
		return false;
	}

	// FXAA�p
	fxaa_pso_ = std::make_unique<PipelineState>();
	fxaa_pso_->SetInputLayout({ nullptr, 0 });
	fxaa_pso_->SetRootSignature(root_signature_->Get());
	fxaa_pso_->SetVS(L"ScreenVS.cso");
	fxaa_pso_->SetPS(L"FXAA.cso");
	fxaa_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = fxaa_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	fxaa_pso_->Create();
	if (!fxaa_pso_->IsValid())
	{
		return false;
	}

	// Skybox�p
	skybox_pso_ = std::make_unique<PipelineState>();
	skybox_pso_->SetInputLayout(Vertex::InputLayout);
	skybox_pso_->SetRootSignature(skybox_root_signature_->Get());
	skybox_pso_->SetVS(L"SkyboxVS.cso");
	skybox_pso_->SetPS(L"SkyboxPS.cso");
	skybox_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = skybox_pso_->GetDesc();
	desc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	// �f�v�X�o�b�t�@�ɂ͏������܂Ȃ�
	desc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	
	skybox_pso_->Create();
	if (!skybox_pso_->IsValid())
	{
		return false;
	}

	return true;
}

void Scene::UpdateCB()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto camera = GetMainCamera();

	// Transform
	auto currentTransform = transform_cb_[currentIndex]->GetPtr<TransformParameter>();

	// world�s��
	auto world = XMMatrixIdentity();

	// view�s��
	//auto view = XMMatrixRotationQuaternion(camera->transform->rotation);
	auto view = camera->GetViewMatrix();

	// proj�s��
	auto proj = camera->GetProjMatrix();

	currentTransform->world = world;
	currentTransform->view = view;
	currentTransform->proj = proj;

	// SceneParameter
	auto currentScene = scene_cb_[currentIndex]->GetPtr<SceneParameter>();

	auto cameraPos = camera->transform->position;
	currentScene->camera_position = cameraPos;

	auto targetPos = camera->GetFocusPosition();
	auto lightPos = targetPos + Vec3(0.5, 3.5, 2.5).Normalized() * 500;
	currentScene->light_view = XMMatrixLookAtRH(lightPos, targetPos, { 0, 1, 0 });
	currentScene->light_proj = XMMatrixOrthographicRH(100, 100, 0.1f, 1000.0f);
	currentScene->light_color = { 20, 20, 20 };
}


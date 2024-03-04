#include "particle_renderer_mesh.h"
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

void ParticleRendererMesh::PrepareModel()
{
	// テクスチャ情報の取得
	auto albedoTexture = prop_.mesh.albedo_texture;
	auto normalTexture = prop_.mesh.normal_texture;
	auto pbrTexture = prop_.mesh.pbr_texture;

	float width = (float)albedoTexture->Metadata().width;
	float height = (float)albedoTexture->Metadata().height;
	float width2 = width * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5f;
	float height2 = height * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5f;

	// モデルの作成
	auto model = std::make_shared<Model>();

	std::vector<Vertex> vertices(4);
	std::vector<uint32_t> indices(6);
	auto& meshes = model->meshes;
	auto& materials = model->materials;

	vertices[0].position = Vec3(-1 * width2, 1 * height2, 0);
	vertices[1].position = Vec3(1 * width2, 1 * height2, 0);
	vertices[2].position = Vec3(1 * width2, -1 * height2, 0);
	vertices[3].position = Vec3(-1 * width2, -1 * height2, 0);

	vertices[0].normal = Vec3(0, 0, 1);
	vertices[1].normal = Vec3(0, 0, 1);
	vertices[2].normal = Vec3(0, 0, 1);
	vertices[3].normal = Vec3(0, 0, 1);

	vertices[0].tangent = Vec3(1, 0, 0);
	vertices[1].tangent = Vec3(1, 0, 0);
	vertices[2].tangent = Vec3(1, 0, 0);
	vertices[3].tangent = Vec3(1, 0, 0);

	vertices[0].uv = Vec2(0, 0);
	vertices[1].uv = Vec2(1, 0);
	vertices[2].uv = Vec2(1, 1);
	vertices[3].uv = Vec2(0, 1);

	indices = {
		0, 2, 1,
		0, 3, 2
	};

	Mesh mesh{};
	mesh.vertices = vertices;
	mesh.indices = indices;
	mesh.material_index = 0;
	meshes.push_back(mesh);

	Material material{};
	material.albedo_texture = albedoTexture;
	material.pbr_texture = pbrTexture;
	material.normal_texture = normalTexture;
	material.base_color = { 0.72f, 0.72f, 0.72f, 1 };
	materials.push_back(material);

	// 頂点バッファ・インデックスバッファの生成
	for (size_t i = 0; i < meshes.size(); i++)
	{
		meshes[i].vertex_buffer = Game::Get()->GetEngine()->CreateVertexBuffer(meshes[i].vertices);
		meshes[i].index_buffer = Game::Get()->GetEngine()->CreateIndexBuffer(meshes[i].indices);
	}

	// ディスクリプタヒープの生成
	model->descriptor_heap = Game::Get()->GetEngine()->CreateDescriptorHeap((unsigned int)materials.size() * 3);

	// ディスクリプタハンドルの取得、SRVの生成
	for (auto& mat : materials)
	{
		mat.albedo_handle = model->descriptor_heap->Alloc();
		mat.pbr_handle = model->descriptor_heap->Alloc();
		mat.normal_handle = model->descriptor_heap->Alloc();

		Game::Get()->GetEngine()->CreateShaderResourceView(*mat.albedo_texture, mat.albedo_handle);
		Game::Get()->GetEngine()->CreateShaderResourceView(*mat.pbr_texture, mat.pbr_handle);
		Game::Get()->GetEngine()->CreateShaderResourceView(*mat.normal_texture, mat.normal_handle);
	}

	particle_model_ = std::move(model);
}

bool ParticleRendererMesh::PreparePSO()
{
	// ShadowMap用
	shadow_pso_ = std::make_unique<PipelineState>();
	shadow_pso_->SetInputLayout(Vertex::InputLayout);
	shadow_pso_->SetRootSignature(root_signature_->Get());
	shadow_pso_->SetVS(L"ParticleShadowVS.cso");
	shadow_pso_->SetPS(L"ShadowPS.cso");
	shadow_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	shadow_pso_->SetRTVFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
	shadow_pso_->Create();
	if (!shadow_pso_->IsValid())
	{
		printf("ShadowPSOの生成に失敗\n");
		return false;
	}

	// Depthプリパス用
	depth_pso_ = std::make_unique<PipelineState>();
	depth_pso_->SetInputLayout(Vertex::InputLayout);
	depth_pso_->SetRootSignature(root_signature_->Get());
	depth_pso_->SetVS(L"ParticleVS.cso");
	depth_pso_->SetPS(L"DepthPS.cso");
	depth_pso_->SetCullMode(prop_.mesh.is_two_side ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_FRONT);
	depth_pso_->SetRTVFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

	depth_pso_->Create();
	if (!depth_pso_->IsValid())
	{
		printf("DepthPSOの生成に失敗\n");
		return false;
	}

	// G-Buffer出力用
	gbuffer_pso_ = std::make_unique<PipelineState>();
	gbuffer_pso_->SetInputLayout(Vertex::InputLayout);
	gbuffer_pso_->SetRootSignature(root_signature_->Get());
	gbuffer_pso_->SetVS(L"ParticleVS.cso");
	gbuffer_pso_->SetPS(L"GBufferPS.cso");
	gbuffer_pso_->SetCullMode(prop_.mesh.is_two_side ? D3D12_CULL_MODE_NONE : D3D12_CULL_MODE_FRONT);

	auto desc = gbuffer_pso_->GetDesc();
	desc->NumRenderTargets = 5;
	desc->RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// Position
	desc->RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// Normal
	desc->RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;		// Albedo
	desc->RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;		// MetallicRoughness
	desc->RTVFormats[4] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// Depth
	desc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	// デプスバッファには書き込まない
	desc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;

	gbuffer_pso_->Create();
	if (!gbuffer_pso_->IsValid())
	{
		printf("GBufferPSOの生成に失敗\n");
		return false;
	}

	return true;
}

void ParticleRendererMesh::UpdateCB()
{
	auto current_index = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto camera = Game::Get()->GetCurrentScene()->GetMainCamera();

	// Transform CB
	auto current_transform = transform_cb_[current_index]->GetPtr<TransformParameter>();
	auto view = camera->GetViewMatrix();
	auto proj = camera->GetProjMatrix();
	auto viewRot = XMQuaternionRotationMatrix(view);

	current_transform->view = view;
	current_transform->proj = proj;

	// Scene CB
	auto current_scene = scene_cb_[current_index]->GetPtr<SceneParameter>();
	auto target_pos = camera->GetFocusPosition();
	auto light_pos = target_pos + Vec3(0.5, 3.5, 2.5).Normalized() * 500;
	auto light_view = XMMatrixLookAtRH(light_pos, target_pos, { 0, 1, 0 });
	auto light_view_rot = XMQuaternionRotationMatrix(light_view);

	current_scene->camera_position = camera->transform->position;
	current_scene->light_view = light_view;
	current_scene->light_proj = XMMatrixOrthographicRH(100, 100, 0.1f, 1000.0f);

	// Particle CB
	auto current_particle = particle_cb_[current_index]->GetPtr<ParticleParameter>();

	// すべてのデータを初期化
	std::fill(current_particle->data, current_particle->data + MAX_PARTICLE_COUNT, ParticleData{});

	auto& particles = particle_emitter_->Particles();
	for (int i = 0; i < particles.size(); i++)
	{
		auto position = particles[i].position;
		auto rotation = Quaternion::FromEuler(particles[i].rotation);
		auto scale = particles[i].scale;
		auto height = particles[i].height;
		auto color = particles[i].final_color;

		auto world = XMMatrixIdentity();
		world *= XMMatrixScalingFromVector(scale);
		world *= XMMatrixRotationQuaternion(rotation);
		world *= XMMatrixTranslationFromVector(position);
		
		current_particle->data[i].world = world;
		current_particle->data[i].light_world = world;
		current_particle->data[i].color = { color.r, color.g, color.b, color.a };
	}
}

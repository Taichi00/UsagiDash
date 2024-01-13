#include "game/component/particle_emitter.h"
#include "game/particle.h"
#include "engine/texture2d.h"
#include "engine/vertex_buffer.h"
#include "engine/index_buffer.h"
#include "engine/constant_buffer.h"
#include "engine/root_signature.h"
#include "engine/pipeline_state.h"
#include "game/entity.h"
#include "game/scene.h"
#include "game/component/camera.h"
#include "engine/descriptor_heap.h"
#include "math/quaternion.h"
#include "math/easing.h"
#include "game/model.h"
#include <memory>

ParticleEmitter::ParticleEmitter(ParticleEmitterProperty prop)
{
	prop_ = prop;

	particles_.clear();
	spawn_timer_ = 0;
	spawn_counter_ = 0;

	// 乱数生成
	std::random_device rd;
	rand_ = std::mt19937(rd());
}

ParticleEmitter::~ParticleEmitter()
{
}

bool ParticleEmitter::Init()
{
	PrepareModel();
	if (!PrepareCB()) return false;
	if (!PrepareRootSignature()) return false;
	if (!PreparePSO()) return false;

	return true;
}

void ParticleEmitter::Update()
{
	SpawnParticles();
	UpdateTimer();
	UpdatePosition(prop_);
	UpdateRotation(prop_);
	UpdateScale(prop_);
	UpdateCB();
}

void ParticleEmitter::DrawShadow()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();

	// ルートシグネチャをセット
	commandList->SetGraphicsRootSignature(root_signature_->Get());

	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, particle_cb_[currentIndex]->GetAddress());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// パイプラインステートをセット
	commandList->SetPipelineState(shadow_pso_->Get());

	// ディスクリプタヒープをセット
	ID3D12DescriptorHeap* heaps[] = {
		particle_model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);

	// メッシュの描画
	for (auto& mesh : particle_model_->meshes)
	{
		auto vbView = mesh.vertex_buffer->View();
		auto ibView = mesh.index_buffer->View();
		auto mat = &particle_model_->materials[mesh.material_index];

		commandList->IASetVertexBuffers(0, 1, &vbView);
		commandList->IASetIndexBuffer(&ibView);
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());

		commandList->DrawIndexedInstanced(mesh.indices.size(), MAX_PARTICLE_COUNT, 0, 0, 0);
	}
}

void ParticleEmitter::DrawDepth()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();

	// ルートシグネチャをセット
	commandList->SetGraphicsRootSignature(root_signature_->Get());

	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, particle_cb_[currentIndex]->GetAddress());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// パイプラインステートをセット
	commandList->SetPipelineState(depth_pso_->Get());

	// ディスクリプタヒープをセット
	ID3D12DescriptorHeap* heaps[] = {
		particle_model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);

	// メッシュの描画
	for (auto& mesh : particle_model_->meshes)
	{
		auto vbView = mesh.vertex_buffer->View();
		auto ibView = mesh.index_buffer->View();
		auto mat = &particle_model_->materials[mesh.material_index];
		
		commandList->IASetVertexBuffers(0, 1, &vbView);
		commandList->IASetIndexBuffer(&ibView);
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());

		commandList->DrawIndexedInstanced(mesh.indices.size(), MAX_PARTICLE_COUNT, 0, 0, 0);
	}
}

void ParticleEmitter::DrawGBuffer()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();

	// ルートシグネチャをセット
	commandList->SetGraphicsRootSignature(root_signature_->Get());

	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, particle_cb_[currentIndex]->GetAddress());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// パイプラインステートをセット
	commandList->SetPipelineState(gbuffer_pso_->Get());

	// ディスクリプタヒープをセット
	ID3D12DescriptorHeap* heaps[] = {
		particle_model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);

	// メッシュの描画
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

		commandList->DrawIndexedInstanced(mesh.indices.size(), MAX_PARTICLE_COUNT, 0, 0, 0);
	}
}

void ParticleEmitter::SetSpawnRate(float rate)
{
	prop_.spawn_rate = rate;
}

void ParticleEmitter::Emit()
{
	spawn_counter_ = 0;
	is_active_ = true;
}

void ParticleEmitter::PrepareModel()
{
	// テクスチャ情報の取得
	auto albedoTexture = prop_.sprite.albedo_texture;
	auto normalTexture = prop_.sprite.normal_texture;
	auto pbrTexture = prop_.sprite.pbr_texture;

	float width = albedoTexture->Metadata().width;
	float height = albedoTexture->Metadata().height;
	float width2 = width * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5;
	float height2 = height * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5;

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
	material.base_color = { 0.72, 0.72, 0.72, 1 };
	materials.push_back(material);

	// 頂点バッファ・インデックスバッファの生成
	for (size_t i = 0; i < meshes.size(); i++)
	{
		meshes[i].vertex_buffer = Game::Get()->GetEngine()->CreateVertexBuffer(meshes[i].vertices);
		meshes[i].index_buffer = Game::Get()->GetEngine()->CreateIndexBuffer(meshes[i].indices);
	}

	// ディスクリプタヒープの生成
	model->descriptor_heap = Game::Get()->GetEngine()->CreateDescriptorHeap(materials.size() * 3);

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

bool ParticleEmitter::PrepareCB()
{
	auto engine = Game::Get()->GetEngine();

	// Transform CB
	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		transform_cb_[i] = engine->CreateConstantBuffer(sizeof(TransformParameter));
		scene_cb_[i] = engine->CreateConstantBuffer(sizeof(SceneParameter));
		particle_cb_[i] = engine->CreateConstantBuffer(sizeof(ParticleParameter));
	}

	// Material CBs
	for (size_t i = 0; i < particle_model_->materials.size(); i++)
	{
		auto cb = engine->CreateConstantBuffer(sizeof(MaterialParameter));
		
		auto mat = &particle_model_->materials[i];
		auto ptr = cb->GetPtr<MaterialParameter>();

		ptr->base_color = mat->base_color;

		materials_cb_.push_back(std::move(cb));
	}

	return true;
}

bool ParticleEmitter::PrepareRootSignature()
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

	root_signature_ = std::make_unique<RootSignature>(_countof(params), params);
	if (!root_signature_->IsValid())
	{
		printf("ルートシグネチャの生成に失敗\n");
		return false;
	}

	return true;
}

bool ParticleEmitter::PreparePSO()
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
	depth_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	depth_pso_->SetRTVFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

	/*auto desc = m_pDepthPSO->GetDesc();
	desc->NumRenderTargets = 0;
	desc->RTVFormats[0] = DXGI_FORMAT_UNKNOWN;*/

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
	gbuffer_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

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

void ParticleEmitter::SpawnParticles()
{
	if (!is_active_)
		return;

	// spawnCountを超えていたらやめる
	if (prop_.spawn_count > 0 && spawn_counter_ > prop_.spawn_count)
		return;

	if (particles_.size() > MAX_PARTICLE_COUNT)
		return;

	if (prop_.spawn_rate == 0)
		return;

	std::uniform_real_distribution<float> frand(-1, 1);

	// タイマーの更新
	spawn_timer_ += 1;
	
	// 1個発生させるのにかかる時間（frame / 個)
	auto spawnInterval = 1.0 / (prop_.spawn_rate + frand(rand_) * prop_.spawn_rate_range);

	while (spawn_timer_ >= spawnInterval)
	{
		if (particles_.size() > MAX_PARTICLE_COUNT)
			break;

		spawn_timer_ -= spawnInterval;

		auto timeToLive = prop_.time_to_live;

		// パーティクルの生成
		Particle particle;
		particle.timeToLive = timeToLive + frand(rand_) * prop_.time_to_live_range;
		particle.time = 0;
		InitSpawningMethod(particle, prop_);
		InitScale(particle, prop_);
		InitRotation(particle, prop_);
		InitPosition(particle, prop_);
		InitSprite(particle, prop_.sprite);

		particles_.push_back(particle);

		spawn_counter_ += 1;

		// spawnCountを超えていたらやめる
		if (prop_.spawn_count > 0 && spawn_counter_ > prop_.spawn_count)
		{
			is_active_ = false;
			break;
		}
	}
}

void ParticleEmitter::InitPosition(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.position_type)
	{
	case PARTICAL_PROP_TYPE_PVA:
		InitPosition(particle, prop.position_pva);
		break;
	case PARTICAL_PROP_TYPE_EASING:
		InitPosition(particle, prop.position_easing);
		break;
	}
}

void ParticleEmitter::InitPosition(Particle& particle, const ParticleEmitterPositionPropertyPVA& prop)
{
	auto position = CalcStartVector(prop.position, prop.position_range);
	auto velocity = CalcStartVector(prop.velocity, prop.velocity_range);
	auto acceleration = CalcStartVector(prop.acceleration, prop.acceleration_range);
	auto pos = particle.position;

	auto world = transform->GetWorldMatrix();
	auto rot = transform->GetWorldRotation().Conjugate();
	auto p = (XMMatrixTranslationFromVector(position + pos) * world).r[3].m128_f32;

	auto q = particle.local_direction;

	particle.position = Vec3(p[0], p[1], p[2]);
	particle.position_velocity = rot * q * velocity;
	particle.position_acceleration = rot * q * acceleration;
}

void ParticleEmitter::InitPosition(Particle& particle, const ParticleEmitterPositionPropertyEasing& prop)
{
	auto start = CalcStartVector(prop.start, prop.start_range);
	auto end = CalcStartVector(prop.end, prop.end_range);
	auto middle = CalcStartVector(prop.middle, prop.middle_range);
	auto pos = particle.position;

	auto world = transform->GetWorldMatrix();
	auto q = particle.local_direction;

	auto s = (XMMatrixTranslationFromVector(q * start + pos) * world).r[3].m128_f32;
	auto e = (XMMatrixTranslationFromVector(q * end + pos) * world).r[3].m128_f32;
	auto m = (XMMatrixTranslationFromVector(q * middle + pos) * world).r[3].m128_f32;
	
	particle.position_start = Vec3(s[0], s[1], s[2]);
	particle.position_end = Vec3(e[0], e[1], e[2]);
	particle.position_middle = Vec3(m[0], m[1], m[2]);
}

void ParticleEmitter::InitRotation(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.rotationType)
	{
	case PARTICAL_PROP_TYPE_PVA:
		InitRotation(particle, prop.rotation_pva);
		break;
	}
}

void ParticleEmitter::InitRotation(Particle& particle, const ParticleEmitterRotationPropertyPVA& prop)
{
	particle.rotation = CalcStartVector(prop.rotation, prop.rotation_range);
	particle.rotation_velocity = CalcStartVector(prop.velocity, prop.velocity_range);
	particle.rotation_acceleration = CalcStartVector(prop.acceleration, prop.acceleration_range);
}

void ParticleEmitter::InitScale(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.scaleType)
	{
	case PARTICAL_PROP_TYPE_PVA:
		InitScale(particle, prop.scale_pva);
		break;
	case PARTICAL_PROP_TYPE_EASING:
		InitScale(particle, prop.scale_easing);
		break;
	}
}

void ParticleEmitter::InitScale(Particle& particle, const ParticleEmitterScalePropertyPVA& prop)
{
	particle.scale = CalcStartVector(prop.scale, prop.scale_range);
	particle.scale_velocity = CalcStartVector(prop.velocity, prop.velocity_range);
	particle.scale_acceleration = CalcStartVector(prop.acceleration, prop.acceleration_range);
}

void ParticleEmitter::InitScale(Particle& particle, const ParticleEmitterScalePropertyEasing& prop)
{
	auto scaleStart = CalcStartVector(prop.start, prop.start_range);
	auto scaleEnd = CalcStartVector(prop.end, prop.end_range);
	auto scaleMiddle = CalcStartVector(prop.middle, prop.middle_range);

	if (prop.keep_aspect)
	{
		scaleStart = Vec3(1, 1, 1) * scaleStart.x;
		scaleEnd = Vec3(1, 1, 1) * scaleEnd.x;
		scaleMiddle = Vec3(1, 1, 1) * scaleMiddle.x;
	}

	particle.scale_start = scaleStart;
	particle.scale_end = scaleEnd;
	particle.scale_middle = scaleMiddle;
}

Vec3 ParticleEmitter::CalcStartVector(const Vec3& v, const Vec3& range)
{
	std::uniform_real_distribution<float> frand(-1, 1);

	auto res = Vec3();
	res.x = v.x + range.x * frand(rand_);
	res.y = v.y + range.y * frand(rand_);
	res.z = v.z + range.z * frand(rand_);

	return res;
}

float ParticleEmitter::CalcStartFloat(const float& f, const float& range)
{
	std::uniform_real_distribution<float> frand(-1, 1);

	return f + range * frand(rand_);
}

void ParticleEmitter::InitSpawningMethod(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.spawning_method)
	{
	case PARTICAL_SPAWN_METHOD_POINT:
		InitSpawningMethod(particle, prop.spawning_point);
		break;
	case PARTICAL_SPAWN_METHOD_CIRCLE:
		InitSpawningMethod(particle, prop.spawning_circle);
		break;
	}
}

void ParticleEmitter::InitSpawningMethod(Particle& particle, const ParticleEmitterSpawningPoint& prop)
{
	auto pos = CalcStartVector(prop.location, prop.location_range);

	particle.position = pos;
	particle.position_middle = pos;
}

void ParticleEmitter::InitSpawningMethod(Particle& particle, const ParticleEmitterSpawningCircle& prop)
{
	float n = spawn_counter_ % prop.vertices;
	float r = CalcStartFloat(prop.radius, prop.radius_range);
	float rad = 2 * PI / prop.vertices * n;

	Vec3 pos = Vec3::Zero();
	pos.x = cos(rad) * r;
	pos.y = 0;
	pos.z = sin(rad) * r;

	auto q = Quaternion::FromToRotation(Vec3(0, 0, 1), pos.Normalized());

	particle.position = pos;
	particle.local_direction = q;
}

void ParticleEmitter::InitSprite(Particle& particle, const ParticleEmitterSpriteProperty& prop)
{
	auto meta = prop.albedo_texture->Metadata();

	particle.width = meta.width * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5;
	particle.height = meta.height * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5;
}

ParticleEmitterProperty& ParticleEmitter::GetProperety()
{
	return prop_;
}

void ParticleEmitter::UpdateTimer()
{
	for (auto& particle : particles_)
	{
		particle.time += 1;
	}

	// timeToLiveを過ぎたら消去
	particles_.erase(
		std::remove_if(particles_.begin(), particles_.end(),[](Particle const& p) {
			return p.time > p.timeToLive;
		}), particles_.end());
}

void ParticleEmitter::UpdatePosition(const ParticleEmitterProperty& prop)
{
	switch (prop.position_type)
	{
	case PARTICAL_PROP_TYPE_PVA:
		UpdatePosition(prop.position_pva);
		break;
	case PARTICAL_PROP_TYPE_EASING:
		UpdatePosition(prop.position_easing);
		break;
	}
}

void ParticleEmitter::UpdatePosition(const ParticleEmitterPositionPropertyPVA& prop)
{
	for (auto& particle : particles_)
	{
		particle.position_velocity += particle.position_acceleration;
		particle.position += particle.position_velocity;
	}
}

void ParticleEmitter::UpdatePosition(const ParticleEmitterPositionPropertyEasing& prop)
{
	float (*easing)(const float&) = GetEasingFunc(prop.type);

	if (prop.middle_enabled)
	{
		for (auto& particle : particles_)
		{
			auto t = particle.time / particle.timeToLive;
			auto t1 = std::clamp(t * 2, 0.0f, 1.0f);
			auto t2 = std::clamp(t * 2 - 1.0f, 0.0f, 1.0f);
			auto start = particle.position_start;
			auto middle = particle.position_middle;
			auto end = particle.position_end;

			particle.position = Vec3::Lerp(Vec3::Lerp(start, middle, easing(t1)), end, easing(t2));
		}
	}
	else
	{
		for (auto& particle : particles_)
		{
			auto t = particle.time / particle.timeToLive;
			auto start = particle.position_start;
			auto end = particle.position_end;

			particle.position = Vec3::Lerp(start, end, easing(t));
		}
	}
}

void ParticleEmitter::UpdateRotation(const ParticleEmitterProperty& prop)
{
	switch (prop.rotationType)
	{
	case PARTICAL_PROP_TYPE_PVA:
		UpdateRotation(prop.rotation_pva);
		break;
	}
}

void ParticleEmitter::UpdateRotation(const ParticleEmitterRotationPropertyPVA& prop)
{
	for (auto& particle : particles_)
	{
		particle.rotation_velocity += particle.rotation_acceleration;
		particle.rotation += particle.rotation_velocity;
	}
}

void ParticleEmitter::UpdateScale(const ParticleEmitterProperty& prop)
{
	switch (prop.scaleType)
	{
	case PARTICAL_PROP_TYPE_PVA:
		UpdateScale(prop.scale_pva);
		break;
	case PARTICAL_PROP_TYPE_EASING:
		UpdateScale(prop.scale_easing);
		break;
	}
}

void ParticleEmitter::UpdateScale(const ParticleEmitterScalePropertyPVA& prop)
{
	for (auto& particle : particles_)
	{
		particle.scale_velocity += particle.scale_acceleration;
		particle.scale += particle.scale_velocity;
	}
}

void ParticleEmitter::UpdateScale(const ParticleEmitterScalePropertyEasing& prop)
{
	float (*easing)(const float&) = GetEasingFunc(prop.type);
	
	if (prop.middle_enabled)
	{
		for (auto& particle : particles_)
		{
			auto t = particle.time / particle.timeToLive;
			auto t1 = std::clamp(t * 2, 0.0f, 1.0f);
			auto t2 = std::clamp(t * 2 - 1.0f, 0.0f, 1.0f);
			auto start = particle.scale_start;
			auto middle = particle.scale_middle;
			auto end = particle.scale_end;

			particle.scale = Vec3::Lerp(Vec3::Lerp(start, middle, easing(t1)), end, easing(t2));
		}
	}
	else
	{
		for (auto& particle : particles_)
		{
			auto t = particle.time / particle.timeToLive;
			auto start = particle.scale_start;
			auto end = particle.scale_end;

			particle.scale = Vec3::Lerp(start, end, easing(t));
		}
	}
	
}

float (*ParticleEmitter::GetEasingFunc(const ParticleEmitterEasingType& type))(const float&)
{
	switch (type)
	{
	case PARTICLE_EASE_LINEAR:
		return &Easing::Linear;
	case PARTICLE_EASE_OUT_CUBIC:
		return &Easing::OutCubic;
	case PARTICLE_EASE_INOUT_CUBIC:
		return &Easing::InOutCubic;
	}
}

void ParticleEmitter::UpdateCB()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto camera = GetEntity()->GetScene()->GetMainCamera();

	// Transform CB
	auto currentTransform = transform_cb_[currentIndex]->GetPtr<TransformParameter>();
	auto view = camera->GetViewMatrix();
	auto proj = camera->GetProjMatrix();
	auto viewRot = XMQuaternionRotationMatrix(view);

	currentTransform->view = view;
	currentTransform->proj = proj;

	// Scene CB
	auto currentScene = scene_cb_[currentIndex]->GetPtr<SceneParameter>();
	auto targetPos = camera->GetFocusPosition();
	auto lightPos = targetPos + Vec3(0.5, 3.5, 2.5).Normalized() * 500;
	auto lightView = XMMatrixLookAtRH(lightPos, targetPos, { 0, 1, 0 });
	auto lightViewRot = XMQuaternionRotationMatrix(lightView);

	currentScene->camera_position = camera->transform->position;
	currentScene->light_view = lightView;
	currentScene->light_proj = XMMatrixOrthographicRH(100, 100, 0.1f, 1000.0f);

	// Particle CB
	auto currentParticle = particle_cb_[currentIndex]->GetPtr<ParticleParameter>();

	// すべてのデータを初期化
	std::fill(currentParticle->data, currentParticle->data + MAX_PARTICLE_COUNT, ParticleData{});

	for (int i = 0; i < particles_.size(); i++)
	{
		auto position = particles_[i].position;
		auto rotation = Quaternion::FromEuler(particles_[i].rotation);
		auto scale = particles_[i].scale;
		auto height = particles_[i].height;

		auto world = XMMatrixIdentity();
		world *= XMMatrixScalingFromVector(scale);
		world *= XMMatrixRotationQuaternion(rotation);

		auto lightWorld = world;
		lightWorld *= XMMatrixTranslation(0, height, 0); // 下端基準で回転
		lightWorld *= XMMatrixRotationQuaternion(XMQuaternionInverse(lightViewRot));
		lightWorld *= XMMatrixTranslation(position.x, position.y - height, position.z);

		world *= XMMatrixRotationQuaternion(XMQuaternionInverse(viewRot));
		world *= XMMatrixTranslationFromVector(position);

		currentParticle->data[i].world = world;
		currentParticle->data[i].lightWorld = lightWorld;
	}
}

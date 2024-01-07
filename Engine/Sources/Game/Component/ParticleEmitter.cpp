#include "ParticleEmitter.h"
#include "Particle.h"
#include "Texture2D.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "Entity.h"
#include "Scene.h"
#include "Camera.h"
#include "DescriptorHeap.h"
#include "Quaternion.h"
#include "Easing.h"
#include "Model.h"
#include <memory>

ParticleEmitter::ParticleEmitter(ParticleEmitterProperty prop)
{
	m_prop = prop;

	m_particles.clear();
	m_spawnTimer = 0;
	m_spawnCounter = 0;

	// 乱数生成
	std::random_device rd;
	m_rand = std::mt19937(rd());
}

ParticleEmitter::~ParticleEmitter()
{
	/*delete[] m_pTransformCB;
	delete[] m_pSceneCB;
	delete[] m_pParticleCB;*/
	delete m_pRootSignature;
	delete m_pShadowPSO;
	delete m_pGBufferPSO;
	delete m_pDepthPSO;
	delete m_pDescriptorHeap;
}

bool ParticleEmitter::Init()
{
	PrepareModel();
	if (!PrepareCB()) return false;
	if (!PrepareSRV()) return false;
	if (!PrepareRootSignature()) return false;
	if (!PreparePSO()) return false;

	return true;
}

void ParticleEmitter::Update()
{
	SpawnParticles();
	UpdateTimer();
	UpdatePosition(m_prop);
	UpdateRotation(m_prop);
	UpdateScale(m_prop);
	UpdateCB();
}

void ParticleEmitter::DrawShadow()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();

	// ルートシグネチャをセット
	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());

	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(1, m_pSceneCB[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, m_pParticleCB[currentIndex]->GetAddress());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// パイプラインステートをセット
	commandList->SetPipelineState(m_pShadowPSO->Get());

	// ディスクリプタヒープをセット
	ID3D12DescriptorHeap* heaps[] = {
		m_pDescriptorHeap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);

	// メッシュの描画
	for (auto& mesh : m_particleModel->meshes)
	{
		auto vbView = mesh.vertexBuffer->View();
		auto ibView = mesh.indexBuffer->View();
		auto mat = &m_particleModel->materials[mesh.materialIndex];

		commandList->IASetVertexBuffers(0, 1, &vbView);
		commandList->IASetIndexBuffer(&ibView);
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedoHandle.HandleGPU());

		commandList->DrawIndexedInstanced(mesh.indices.size(), MAX_PARTICLE_COUNT, 0, 0, 0);
	}
}

void ParticleEmitter::DrawDepth()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();

	// ルートシグネチャをセット
	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());

	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, m_pParticleCB[currentIndex]->GetAddress());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// パイプラインステートをセット
	commandList->SetPipelineState(m_pDepthPSO->Get());

	// ディスクリプタヒープをセット
	ID3D12DescriptorHeap* heaps[] = {
		m_pDescriptorHeap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);

	// メッシュの描画
	for (auto& mesh : m_particleModel->meshes)
	{
		auto vbView = mesh.vertexBuffer->View();
		auto ibView = mesh.indexBuffer->View();
		auto mat = &m_particleModel->materials[mesh.materialIndex];
		
		commandList->IASetVertexBuffers(0, 1, &vbView);
		commandList->IASetIndexBuffer(&ibView);
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedoHandle.HandleGPU());

		commandList->DrawIndexedInstanced(mesh.indices.size(), MAX_PARTICLE_COUNT, 0, 0, 0);
	}
}

void ParticleEmitter::DrawGBuffer()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();

	// ルートシグネチャをセット
	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());

	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(1, m_pSceneCB[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, m_pParticleCB[currentIndex]->GetAddress());

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// パイプラインステートをセット
	commandList->SetPipelineState(m_pGBufferPSO->Get());

	// ディスクリプタヒープをセット
	ID3D12DescriptorHeap* heaps[] = {
		m_pDescriptorHeap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);

	// メッシュの描画
	for (auto& mesh : m_particleModel->meshes)
	{
		auto vbView = mesh.vertexBuffer->View();
		auto ibView = mesh.indexBuffer->View();
		auto mat = &m_particleModel->materials[mesh.materialIndex];

		commandList->IASetVertexBuffers(0, 1, &vbView);
		commandList->IASetIndexBuffer(&ibView);
		commandList->SetGraphicsRootConstantBufferView(3, m_pMaterialCBs[mesh.materialIndex]->GetAddress());
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedoHandle.HandleGPU());
		commandList->SetGraphicsRootDescriptorTable(5, mat->pbrHandle.HandleGPU());
		commandList->SetGraphicsRootDescriptorTable(6, mat->normalHandle.HandleGPU());

		commandList->DrawIndexedInstanced(mesh.indices.size(), MAX_PARTICLE_COUNT, 0, 0, 0);
	}
}

void ParticleEmitter::SetSpawnRate(float rate)
{
	m_prop.spawnRate = rate;
}

void ParticleEmitter::Emit()
{
	m_spawnCounter = 0;
	m_isActive = true;
}

void ParticleEmitter::PrepareModel()
{
	// テクスチャ情報の取得
	auto albedoTexture = m_prop.sprite.albedoTexture;
	auto normalTexture = m_prop.sprite.normalTexture;
	auto pbrTexture = m_prop.sprite.pbrTexture;

	float width = albedoTexture->Metadata().width;
	float height = albedoTexture->Metadata().height;
	float width2 = width * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5;
	float height2 = height * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5;

	// モデルの作成
	std::vector<Vertex> vertices(4);
	std::vector<uint32_t> indices(6);
	std::vector<Mesh> meshes;
	std::vector<Material> materials;

	vertices[0].Position = Vec3(-1 * width2, 1 * height2, 0);
	vertices[1].Position = Vec3(1 * width2, 1 * height2, 0);
	vertices[2].Position = Vec3(1 * width2, -1 * height2, 0);
	vertices[3].Position = Vec3(-1 * width2, -1 * height2, 0);

	vertices[0].Normal = Vec3(0, 0, 1);
	vertices[1].Normal = Vec3(0, 0, 1);
	vertices[2].Normal = Vec3(0, 0, 1);
	vertices[3].Normal = Vec3(0, 0, 1);

	vertices[0].Tangent = Vec3(1, 0, 0);
	vertices[1].Tangent = Vec3(1, 0, 0);
	vertices[2].Tangent = Vec3(1, 0, 0);
	vertices[3].Tangent = Vec3(1, 0, 0);

	vertices[0].UV = Vec2(0, 0);
	vertices[1].UV = Vec2(1, 0);
	vertices[2].UV = Vec2(1, 1);
	vertices[3].UV = Vec2(0, 1);

	indices = {
		0, 2, 1,
		0, 3, 2
	};

	Mesh mesh{};
	mesh.vertices = vertices;
	mesh.indices = indices;
	mesh.materialIndex = 0;
	meshes.push_back(mesh);

	Material material{};
	material.albedoTexture = albedoTexture;
	material.pbrTexture = pbrTexture;
	material.normalTexture = normalTexture;
	material.baseColor = { 0.72, 0.72, 0.72, 1 };
	materials.push_back(material);

	for (int i = 0; i < meshes.size(); i++)
	{
		// 頂点バッファの生成
		auto vSize = sizeof(Vertex) * meshes[i].vertices.size();
		auto stride = sizeof(Vertex);
		auto vertices = meshes[i].vertices.data();
		auto pVB = std::make_unique<VertexBuffer>(vSize, stride, vertices);
		if (!pVB->IsValid())
		{
			printf("頂点バッファの生成に失敗\n");
			break;
		}
		meshes[i].vertexBuffer = std::move(pVB);

		// インデックスバッファの生成
		auto iSize = sizeof(uint32_t) * meshes[i].indices.size();
		auto indices = meshes[i].indices.data();
		auto pIB = std::make_unique<IndexBuffer>(iSize, indices);
		if (!pIB->IsValid())
		{
			printf("インデックスバッファの生成に失敗\n");
			break;
		}
		meshes[i].indexBuffer = std::move(pIB);
	}

	auto model = std::make_shared<Model>();
	model->meshes = meshes;
	model->materials = materials;

	m_particleModel = std::move(model);
}

bool ParticleEmitter::PrepareCB()
{
	// Transform CB
	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		m_pTransformCB[i] = new ConstantBuffer(sizeof(TransformParameter));
		if (!m_pTransformCB[i]->IsValid())
		{
			printf("TransformCBの生成に失敗\n");
			return false;
		}
	}

	// Scene CB
	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		m_pSceneCB[i] = new ConstantBuffer(sizeof(SceneParameter));
		if (!m_pSceneCB[i]->IsValid())
		{
			printf("SceneCBの生成に失敗\n");
			return false;
		}
	}

	// Material CBs
	for (size_t i = 0; i < m_particleModel->materials.size(); i++)
	{
		ConstantBuffer* cb = new ConstantBuffer(sizeof(MaterialParameter));
		if (!cb->IsValid())
		{
			printf("MaterialCBの生成に失敗\n");
			return false;
		}

		auto mat = &m_particleModel->materials[i];
		auto ptr = cb->GetPtr<MaterialParameter>();

		ptr->BaseColor = mat->baseColor;

		m_pMaterialCBs.push_back(cb);
	}

	// Particle CB
	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		m_pParticleCB[i] = new ConstantBuffer(sizeof(ParticleParameter));
		if (!m_pParticleCB[i]->IsValid())
		{
			printf("ParticleCBの生成に失敗\n");
			return false;
		}
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

	m_pRootSignature = new RootSignature(_countof(params), params);
	if (!m_pRootSignature->IsValid())
	{
		printf("ルートシグネチャの生成に失敗\n");
		return false;
	}

	return true;
}

bool ParticleEmitter::PreparePSO()
{
	// ShadowMap用
	m_pShadowPSO = new PipelineState();
	m_pShadowPSO->SetInputLayout(Vertex::InputLayout);
	m_pShadowPSO->SetRootSignature(m_pRootSignature->Get());
	m_pShadowPSO->SetVS(L"ParticleShadowVS.cso");
	m_pShadowPSO->SetPS(L"ShadowPS.cso");
	m_pShadowPSO->SetCullMode(D3D12_CULL_MODE_FRONT);
	m_pShadowPSO->Create();
	if (!m_pShadowPSO->IsValid())
	{
		printf("ShadowPSOの生成に失敗\n");
		return false;
	}

	// Depthプリパス用
	m_pDepthPSO = new PipelineState();
	m_pDepthPSO->SetInputLayout(Vertex::InputLayout);
	m_pDepthPSO->SetRootSignature(m_pRootSignature->Get());
	m_pDepthPSO->SetVS(L"ParticleVS.cso");
	m_pDepthPSO->SetPS(L"DepthPS.cso");
	m_pDepthPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	auto desc = m_pDepthPSO->GetDesc();
	desc->NumRenderTargets = 0;
	desc->RTVFormats[0] = DXGI_FORMAT_UNKNOWN;

	m_pDepthPSO->Create();
	if (!m_pDepthPSO->IsValid())
	{
		printf("DepthPSOの生成に失敗\n");
		return false;
	}

	// G-Buffer出力用
	m_pGBufferPSO = new PipelineState();
	m_pGBufferPSO->SetInputLayout(Vertex::InputLayout);
	m_pGBufferPSO->SetRootSignature(m_pRootSignature->Get());
	m_pGBufferPSO->SetVS(L"ParticleVS.cso");
	m_pGBufferPSO->SetPS(L"GBufferPS.cso");
	m_pGBufferPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = m_pGBufferPSO->GetDesc();
	desc->NumRenderTargets = 5;
	desc->RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// Position
	desc->RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// Normal
	desc->RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;		// Albedo
	desc->RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;		// MetallicRoughness
	desc->RTVFormats[4] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// Depth
	desc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	// デプスバッファには書き込まない
	desc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;

	m_pGBufferPSO->Create();
	if (!m_pGBufferPSO->IsValid())
	{
		printf("GBufferPSOの生成に失敗\n");
		return false;
	}

	return true;
}

void ParticleEmitter::SpawnParticles()
{
	if (!m_isActive)
		return;

	// spawnCountを超えていたらやめる
	if (m_prop.spawnCount > 0 && m_spawnCounter > m_prop.spawnCount)
		return;

	if (m_particles.size() > MAX_PARTICLE_COUNT)
		return;

	if (m_prop.spawnRate == 0)
		return;

	std::uniform_real_distribution<float> frand(-1, 1);

	// タイマーの更新
	m_spawnTimer += 1;
	
	// 1個発生させるのにかかる時間（frame / 個)
	auto spawnInterval = 1.0 / (m_prop.spawnRate + frand(m_rand) * m_prop.spawnRateRange);

	while (m_spawnTimer >= spawnInterval)
	{
		if (m_particles.size() > MAX_PARTICLE_COUNT)
			break;

		m_spawnTimer -= spawnInterval;

		auto timeToLive = m_prop.timeToLive;

		// パーティクルの生成
		Particle particle;
		particle.timeToLive = timeToLive + frand(m_rand) * m_prop.timeToLiveRange;
		particle.time = 0;
		InitSpawningMethod(particle, m_prop);
		InitScale(particle, m_prop);
		InitRotation(particle, m_prop);
		InitPosition(particle, m_prop);
		InitSprite(particle, m_prop.sprite);

		m_particles.push_back(particle);

		m_spawnCounter += 1;

		// spawnCountを超えていたらやめる
		if (m_prop.spawnCount > 0 && m_spawnCounter > m_prop.spawnCount)
		{
			m_isActive = false;
			break;
		}
	}
}

void ParticleEmitter::InitPosition(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.positionType)
	{
	case PARTICAL_PROP_TYPE_PVA:
		InitPosition(particle, prop.positionPVA);
		break;
	case PARTICAL_PROP_TYPE_EASING:
		InitPosition(particle, prop.positionEasing);
		break;
	}
}

void ParticleEmitter::InitPosition(Particle& particle, const ParticleEmitterPositionPropertyPVA& prop)
{
	auto position = CalcStartVector(prop.position, prop.positionRange);
	auto velocity = CalcStartVector(prop.velocity, prop.velocityRange);
	auto acceleration = CalcStartVector(prop.acceleration, prop.accelerationRange);
	auto pos = particle.position;

	auto world = transform->GetWorldMatrix();
	auto rot = transform->GetWorldRotation().conjugate();
	auto p = (XMMatrixTranslationFromVector(position + pos) * world).r[3].m128_f32;

	auto q = particle.localDirection;

	particle.position = Vec3(p[0], p[1], p[2]);
	particle.positionVelocity = rot * q * velocity;
	particle.positionAcceleration = rot * q * acceleration;
}

void ParticleEmitter::InitPosition(Particle& particle, const ParticleEmitterPositionPropertyEasing& prop)
{
	auto start = CalcStartVector(prop.start, prop.startRange);
	auto end = CalcStartVector(prop.end, prop.endRange);
	auto middle = CalcStartVector(prop.middle, prop.middleRange);
	auto pos = particle.position;

	auto world = transform->GetWorldMatrix();
	auto q = particle.localDirection;

	auto s = (XMMatrixTranslationFromVector(q * start + pos) * world).r[3].m128_f32;
	auto e = (XMMatrixTranslationFromVector(q * end + pos) * world).r[3].m128_f32;
	auto m = (XMMatrixTranslationFromVector(q * middle + pos) * world).r[3].m128_f32;
	
	particle.positionStart = Vec3(s[0], s[1], s[2]);
	particle.positionEnd = Vec3(e[0], e[1], e[2]);
	particle.positionMiddle = Vec3(m[0], m[1], m[2]);
}

void ParticleEmitter::InitRotation(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.rotationType)
	{
	case PARTICAL_PROP_TYPE_PVA:
		InitRotation(particle, prop.rotationPVA);
		break;
	}
}

void ParticleEmitter::InitRotation(Particle& particle, const ParticleEmitterRotationPropertyPVA& prop)
{
	particle.rotation = CalcStartVector(prop.rotation, prop.rotationRange);
	particle.rotationVelocity = CalcStartVector(prop.velocity, prop.velocityRange);
	particle.rotationAcceleration = CalcStartVector(prop.acceleration, prop.accelerationRange);
}

void ParticleEmitter::InitScale(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.scaleType)
	{
	case PARTICAL_PROP_TYPE_PVA:
		InitScale(particle, prop.scalePVA);
		break;
	case PARTICAL_PROP_TYPE_EASING:
		InitScale(particle, prop.scaleEasing);
		break;
	}
}

void ParticleEmitter::InitScale(Particle& particle, const ParticleEmitterScalePropertyPVA& prop)
{
	particle.scale = CalcStartVector(prop.scale, prop.scaleRange);
	particle.scaleVelocity = CalcStartVector(prop.velocity, prop.velocityRange);
	particle.scaleAcceleration = CalcStartVector(prop.acceleration, prop.accelerationRange);
}

void ParticleEmitter::InitScale(Particle& particle, const ParticleEmitterScalePropertyEasing& prop)
{
	auto scaleStart = CalcStartVector(prop.start, prop.startRange);
	auto scaleEnd = CalcStartVector(prop.end, prop.endRange);
	auto scaleMiddle = CalcStartVector(prop.middle, prop.middleRange);

	if (prop.keepAspect)
	{
		scaleStart = Vec3(1, 1, 1) * scaleStart.x;
		scaleEnd = Vec3(1, 1, 1) * scaleEnd.x;
		scaleMiddle = Vec3(1, 1, 1) * scaleMiddle.x;
	}

	particle.scaleStart = scaleStart;
	particle.scaleEnd = scaleEnd;
	particle.scaleMiddle = scaleMiddle;
}

Vec3 ParticleEmitter::CalcStartVector(const Vec3& v, const Vec3& range)
{
	std::uniform_real_distribution<float> frand(-1, 1);

	auto res = Vec3();
	res.x = v.x + range.x * frand(m_rand);
	res.y = v.y + range.y * frand(m_rand);
	res.z = v.z + range.z * frand(m_rand);

	return res;
}

float ParticleEmitter::CalcStartFloat(const float& f, const float& range)
{
	std::uniform_real_distribution<float> frand(-1, 1);

	return f + range * frand(m_rand);
}

void ParticleEmitter::InitSpawningMethod(Particle& particle, const ParticleEmitterProperty& prop)
{
	switch (prop.spawningMethod)
	{
	case PARTICAL_SPAWN_METHOD_POINT:
		InitSpawningMethod(particle, prop.spawningPoint);
		break;
	case PARTICAL_SPAWN_METHOD_CIRCLE:
		InitSpawningMethod(particle, prop.spawningCircle);
		break;
	}
}

void ParticleEmitter::InitSpawningMethod(Particle& particle, const ParticleEmitterSpawningPoint& prop)
{
	auto pos = CalcStartVector(prop.location, prop.locationRange);

	particle.position = pos;
	particle.positionMiddle = pos;
}

void ParticleEmitter::InitSpawningMethod(Particle& particle, const ParticleEmitterSpawningCircle& prop)
{
	float n = m_spawnCounter % prop.vertices;
	float r = CalcStartFloat(prop.radius, prop.radiusRange);
	float rad = 2 * PI / prop.vertices * n;

	Vec3 pos = Vec3::Zero();
	pos.x = cos(rad) * r;
	pos.y = 0;
	pos.z = sin(rad) * r;

	auto q = Quaternion::FromToRotation(Vec3(0, 0, 1), pos.normalized());

	particle.position = pos;
	particle.localDirection = q;
}

void ParticleEmitter::InitSprite(Particle& particle, const ParticleEmitterSpriteProperty& prop)
{
	auto meta = prop.albedoTexture->Metadata();

	particle.width = meta.width * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5;
	particle.height = meta.height * PARTICLE_SPRITE_SIZE_PER_PIXEL * 0.5;
}

ParticleEmitterProperty& ParticleEmitter::GetProperety()
{
	return m_prop;
}

void ParticleEmitter::UpdateTimer()
{
	for (auto& particle : m_particles)
	{
		particle.time += 1;
	}

	// timeToLiveを過ぎたら消去
	m_particles.erase(
		std::remove_if(m_particles.begin(), m_particles.end(),[](Particle const& p) {
			return p.time > p.timeToLive;
		}), m_particles.end());
}

void ParticleEmitter::UpdatePosition(const ParticleEmitterProperty& prop)
{
	switch (prop.positionType)
	{
	case PARTICAL_PROP_TYPE_PVA:
		UpdatePosition(prop.positionPVA);
		break;
	case PARTICAL_PROP_TYPE_EASING:
		UpdatePosition(prop.positionEasing);
		break;
	}
}

void ParticleEmitter::UpdatePosition(const ParticleEmitterPositionPropertyPVA& prop)
{
	for (auto& particle : m_particles)
	{
		particle.positionVelocity += particle.positionAcceleration;
		particle.position += particle.positionVelocity;
	}
}

void ParticleEmitter::UpdatePosition(const ParticleEmitterPositionPropertyEasing& prop)
{
	float (*easing)(const float&) = GetEasingFunc(prop.type);

	if (prop.middleEnabled)
	{
		for (auto& particle : m_particles)
		{
			auto t = particle.time / particle.timeToLive;
			auto t1 = std::clamp(t * 2, 0.0f, 1.0f);
			auto t2 = std::clamp(t * 2 - 1.0f, 0.0f, 1.0f);
			auto start = particle.positionStart;
			auto middle = particle.positionMiddle;
			auto end = particle.positionEnd;

			particle.position = Vec3::lerp(Vec3::lerp(start, middle, easing(t1)), end, easing(t2));
		}
	}
	else
	{
		for (auto& particle : m_particles)
		{
			auto t = particle.time / particle.timeToLive;
			auto start = particle.positionStart;
			auto end = particle.positionEnd;

			particle.position = Vec3::lerp(start, end, easing(t));
		}
	}
}

void ParticleEmitter::UpdateRotation(const ParticleEmitterProperty& prop)
{
	switch (prop.rotationType)
	{
	case PARTICAL_PROP_TYPE_PVA:
		UpdateRotation(prop.rotationPVA);
		break;
	}
}

void ParticleEmitter::UpdateRotation(const ParticleEmitterRotationPropertyPVA& prop)
{
	for (auto& particle : m_particles)
	{
		particle.rotationVelocity += particle.rotationAcceleration;
		particle.rotation += particle.rotationVelocity;
	}
}

void ParticleEmitter::UpdateScale(const ParticleEmitterProperty& prop)
{
	switch (prop.scaleType)
	{
	case PARTICAL_PROP_TYPE_PVA:
		UpdateScale(prop.scalePVA);
		break;
	case PARTICAL_PROP_TYPE_EASING:
		UpdateScale(prop.scaleEasing);
		break;
	}
}

void ParticleEmitter::UpdateScale(const ParticleEmitterScalePropertyPVA& prop)
{
	for (auto& particle : m_particles)
	{
		particle.scaleVelocity += particle.scaleAcceleration;
		particle.scale += particle.scaleVelocity;
	}
}

void ParticleEmitter::UpdateScale(const ParticleEmitterScalePropertyEasing& prop)
{
	float (*easing)(const float&) = GetEasingFunc(prop.type);
	
	if (prop.middleEnabled)
	{
		for (auto& particle : m_particles)
		{
			auto t = particle.time / particle.timeToLive;
			auto t1 = std::clamp(t * 2, 0.0f, 1.0f);
			auto t2 = std::clamp(t * 2 - 1.0f, 0.0f, 1.0f);
			auto start = particle.scaleStart;
			auto middle = particle.scaleMiddle;
			auto end = particle.scaleEnd;

			particle.scale = Vec3::lerp(Vec3::lerp(start, middle, easing(t1)), end, easing(t2));
		}
	}
	else
	{
		for (auto& particle : m_particles)
		{
			auto t = particle.time / particle.timeToLive;
			auto start = particle.scaleStart;
			auto end = particle.scaleEnd;

			particle.scale = Vec3::lerp(start, end, easing(t));
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

bool ParticleEmitter::PrepareSRV()
{
	// DescriptorHeapの生成
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NodeMask = 0;	// どのGPU向けのディスクリプタヒープかを指定（GPU１つの場合は０）
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = m_particleModel->materials.size();
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	m_pDescriptorHeap = new DescriptorHeap(desc);

	// SRVの生成
	for (size_t i = 0; i < m_particleModel->materials.size(); i++)
	{
		// Albedoテクスチャ
		auto& texture = m_particleModel->materials[i].albedoTexture;
		auto pHandle = m_pDescriptorHeap->Alloc();
		auto resource = texture->Resource();
		auto desc = texture->ViewDesc();
		g_Engine->Device()->CreateShaderResourceView(resource, &desc, pHandle.HandleCPU());
		m_particleModel->materials[i].albedoHandle = pHandle;

		// PBRテクスチャ
		texture = m_particleModel->materials[i].pbrTexture;
		pHandle = m_pDescriptorHeap->Alloc();
		resource = texture->Resource();
		desc = texture->ViewDesc();
		g_Engine->Device()->CreateShaderResourceView(resource, &desc, pHandle.HandleCPU());
		m_particleModel->materials[i].pbrHandle = pHandle;

		// Normalテクスチャ
		texture = m_particleModel->materials[i].normalTexture;
		pHandle = m_pDescriptorHeap->Alloc();
		resource = texture->Resource();
		desc = texture->ViewDesc();
		g_Engine->Device()->CreateShaderResourceView(resource, &desc, pHandle.HandleCPU());
		m_particleModel->materials[i].normalHandle = pHandle;
	}

	return true;
}

void ParticleEmitter::UpdateCB()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto camera = GetEntity()->GetScene()->GetMainCamera();

	// Transform CB
	auto currentTransform = m_pTransformCB[currentIndex]->GetPtr<TransformParameter>();
	auto view = camera->GetViewMatrix();
	auto proj = camera->GetProjMatrix();
	auto viewRot = XMQuaternionRotationMatrix(view);

	currentTransform->View = view;
	currentTransform->Proj = proj;

	// Scene CB
	auto currentScene = m_pSceneCB[currentIndex]->GetPtr<SceneParameter>();
	auto targetPos = camera->GetFocusPosition();
	auto lightPos = targetPos + Vec3(0.5, 3.5, 2.5).normalized() * 500;
	auto lightView = XMMatrixLookAtRH(lightPos, targetPos, { 0, 1, 0 });
	auto lightViewRot = XMQuaternionRotationMatrix(lightView);

	currentScene->CameraPosition = camera->transform->position;
	currentScene->LightView = lightView;
	currentScene->LightProj = XMMatrixOrthographicRH(100, 100, 0.1f, 1000.0f);

	// Particle CB
	auto currentParticle = m_pParticleCB[currentIndex]->GetPtr<ParticleParameter>();

	// すべてのデータを初期化
	std::fill(currentParticle->data, currentParticle->data + MAX_PARTICLE_COUNT, ParticleData{});

	for (int i = 0; i < m_particles.size(); i++)
	{
		auto position = m_particles[i].position;
		auto rotation = Quaternion::FromEuler(m_particles[i].rotation);
		auto scale = m_particles[i].scale;
		auto height = m_particles[i].height;

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

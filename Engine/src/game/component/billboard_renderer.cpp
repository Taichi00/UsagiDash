#include "game/component/billboard_renderer.h"
#include "game/resource/texture2d.h"
#include "engine/shared_struct.h"
#include "math/vec.h"
#include "engine/vertex_buffer.h"
#include "engine/index_buffer.h"
#include "game/component/transform.h"
#include "game/entity.h"
#include "game/scene.h"
#include "game/component/camera.h"
#include "engine/constant_buffer.h"
#include <vector>

BillboardRenderer::BillboardRenderer(std::shared_ptr<Texture2D> albedoTexture, std::shared_ptr<Texture2D> normalTexture)
{
	albedo_texture_ = albedoTexture;
	normal_texture_ = normalTexture;

	float width = albedoTexture->Metadata().width;
	float height = albedoTexture->Metadata().height;
	float width2 = width * 0.005;
	float height2 = height * 0.005;
	width_ = width2;
	height_ = height2;

	float pbrColor[] = { 0, 1, 0, 1 };
	float normalColor[] = { 0.5, 0.5, 1, 1 };
	std::shared_ptr pbrTexture = Texture2D::GetMono(pbrColor);
	//auto normalTexture = Texture2D::GetMono(normalColor);

	// モデルの作成
	auto model = std::make_shared<Model>();
	std::vector<Vertex> vertices(4);
	std::vector<uint32_t> indices(6);
	auto& meshes = model->meshes;
	auto& materials = model->materials;

	vertices[0].position = Vec3(-1 * width2,  1 * height2, 0);
	vertices[1].position = Vec3( 1 * width2,  1 * height2, 0);
	vertices[2].position = Vec3( 1 * width2, -1 * height2, 0);
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
		meshes[i].vertex_buffer = std::move(pVB);

		// インデックスバッファの生成
		auto iSize = sizeof(uint32_t) * meshes[i].indices.size();
		auto indices = meshes[i].indices.data();
		auto pIB = std::make_unique<IndexBuffer>(iSize, indices);
		if (!pIB->IsValid())
		{
			printf("インデックスバッファの生成に失敗\n");
			break;
		}
		meshes[i].index_buffer = std::move(pIB);
	}

	model_ = model;
}

BillboardRenderer::~BillboardRenderer()
{
}

void BillboardRenderer::Update()
{
	UpdateCB();
}

void BillboardRenderer::UpdateCB()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();

	auto camera = GetEntity()->GetScene()->GetMainCamera();

	// Transform
	auto currentTransform = transform_cb_[currentIndex]->GetPtr<TransformParameter>();
	auto world = transform->GetBillboardWorldMatrix();
	auto view = camera->GetViewMatrix();
	auto viewRot = XMQuaternionRotationMatrix(view);
	auto proj = camera->GetProjMatrix();
	auto ditherLevel = (1.0 - ((transform->position - camera->transform->position).Length() - 3) / 3.0) * 16;

	currentTransform->world = world;
	currentTransform->view = view;
	currentTransform->proj = proj;
	currentTransform->dither_level = ditherLevel;

	// SceneParameter
	auto currentScene = scene_cb_[currentIndex]->GetPtr<SceneParameter>();
	auto cameraPos = camera->transform->position;
	auto targetPos = camera->GetFocusPosition();
	auto lightPos = targetPos + Vec3(0.5, 3.5, 2.5).Normalized() * 500;
	auto lightView = XMMatrixLookAtRH(lightPos, targetPos, { 0, 1, 0 });
	auto lightViewRot = XMQuaternionRotationMatrix(lightView);

	currentScene->camera_position = cameraPos;
	currentScene->light_view = lightView;
	currentScene->light_proj = XMMatrixOrthographicRH(100, 100, 0.1f, 1000.0f);

	auto lightWorld = XMMatrixIdentity();
	lightWorld *= XMMatrixScalingFromVector(transform->scale);
	lightWorld *= XMMatrixRotationQuaternion(transform->rotation);
	lightWorld *= XMMatrixTranslation(0, height_, 0); // 下端基準で回転
	lightWorld *= XMMatrixRotationQuaternion(XMQuaternionInverse(lightViewRot));
	lightWorld *= XMMatrixTranslationFromVector(transform->position - Vec3(0, height_, 0));
	currentScene->light_world = lightWorld;
}

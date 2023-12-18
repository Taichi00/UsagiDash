#include "BillboardRenderer.h"
#include "Texture2D.h"
#include "SharedStruct.h"
#include "Vec.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Transform.h"
#include "Entity.h"
#include "Scene.h"
#include "Camera.h"
#include "ConstantBuffer.h"
#include <vector>

BillboardRenderer::BillboardRenderer(Texture2D* albedoTexture, Texture2D* normalTexture)
{
	m_pTexture = albedoTexture;
	m_pNormalTexture = normalTexture;

	float width = albedoTexture->Metadata().width;
	float height = albedoTexture->Metadata().height;
	float width2 = width * 0.005;
	float height2 = height * 0.005;
	m_width = width2;
	m_height = height2;

	float pbrColor[] = { 1, 1, 0, 1 };
	float normalColor[] = { 0.5, 0.5, 1, 1 };
	auto pbrTexture = Texture2D::GetMono(pbrColor);
	//auto normalTexture = Texture2D::GetMono(normalColor);

	// モデルの作成
	std::vector<Vertex> vertices(4);
	std::vector<uint32_t> indices(6);
	std::vector<Mesh> meshes;
	std::vector<Material> materials;

	vertices[0].Position = Vec3(-1 * width2,  1 * height2, 0);
	vertices[1].Position = Vec3( 1 * width2,  1 * height2, 0);
	vertices[2].Position = Vec3( 1 * width2, -1 * height2, 0);
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
	mesh.Vertices = vertices;
	mesh.Indices = indices;
	mesh.MaterialIndex = 0;
	meshes.push_back(mesh);

	Material material{};
	material.Texture = albedoTexture;
	material.PbrTexture = pbrTexture;
	material.NormalTexture = normalTexture;
	material.BaseColor = { 0.72, 0.72, 0.72, 1 };
	materials.push_back(material);

	for (int i = 0; i < meshes.size(); i++)
	{
		// 頂点バッファの生成
		auto vSize = sizeof(Vertex) * meshes[i].Vertices.size();
		auto stride = sizeof(Vertex);
		auto vertices = meshes[i].Vertices.data();
		auto pVB = new VertexBuffer(vSize, stride, vertices);
		if (!pVB->IsValid())
		{
			printf("頂点バッファの生成に失敗\n");
			break;
		}
		meshes[i].pVertexBuffer = pVB;

		// インデックスバッファの生成
		auto iSize = sizeof(uint32_t) * meshes[i].Indices.size();
		auto indices = meshes[i].Indices.data();
		auto pIB = new IndexBuffer(iSize, indices);
		if (!pIB->IsValid())
		{
			printf("インデックスバッファの生成に失敗\n");
			break;
		}
		meshes[i].pIndexBuffer = pIB;
	}

	Model model{};
	model.Meshes = meshes;
	model.Materials = materials;

	MeshRenderer::SetProperties({ model });
}

BillboardRenderer::~BillboardRenderer()
{
}

void BillboardRenderer::Update()
{
	UpdateCB();
}

void BillboardRenderer::DrawShadow()
{
	/*auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto currentTransform = m_pTransformCB[currentIndex]->GetPtr<TransformParameter>();

	auto camera = m_pEntity->GetScene()->GetMainCamera();

	auto targetPos = camera->GetFocusPosition();
	auto lightPos = targetPos + Vec3(0.5, 3.5, 2.5).normalized() * 500;
	auto lightView = XMMatrixLookAtRH(lightPos, targetPos, { 0, 1, 0 });
	auto lightViewRot = XMQuaternionRotationMatrix(lightView);

	auto scale = transform->scale;
	auto rotation = transform->rotation;
	auto position = transform->position;

	auto world = XMMatrixIdentity();
	world *= XMMatrixScaling(scale.x, scale.y, scale.z);
	world *= XMMatrixRotationQuaternion(rotation);
	world *= XMMatrixRotationQuaternion(XMQuaternionInverse(lightViewRot));
	world *= XMMatrixTranslation(position.x, position.y, position.z);

	currentTransform->World = world;*/

	MeshRenderer::DrawShadow();
}

void BillboardRenderer::DrawDepth()
{
	/*auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto currentTransform = m_pTransformCB[currentIndex]->GetPtr<TransformParameter>();

	auto camera = m_pEntity->GetScene()->GetMainCamera();

	auto view = camera->GetViewMatrix();
	auto viewRot = XMQuaternionRotationMatrix(view);

	auto scale = transform->scale;
	auto rotation = transform->rotation;
	auto position = transform->position;

	auto world = XMMatrixIdentity();
	world *= XMMatrixScaling(scale.x, scale.y, scale.z);
	world *= XMMatrixRotationQuaternion(rotation);
	world *= XMMatrixRotationQuaternion(XMQuaternionInverse(viewRot));
	world *= XMMatrixTranslation(position.x, position.y, position.z);

	currentTransform->World = world;*/

	MeshRenderer::DrawDepth();
}

void BillboardRenderer::UpdateCB()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();

	auto camera = m_pEntity->GetScene()->GetMainCamera();

	// Transform
	auto currentTransform = m_pTransformCB[currentIndex]->GetPtr<TransformParameter>();

	// world行列
	auto scale = transform->scale;
	auto rotation = transform->rotation;
	auto position = transform->position;

	auto view = camera->GetViewMatrix();
	auto viewRot = XMQuaternionRotationMatrix(view);

	auto world = XMMatrixIdentity();
	world *= XMMatrixScaling(scale.x, scale.y, scale.z);
	world *= XMMatrixRotationQuaternion(rotation);
	world *= XMMatrixRotationQuaternion(XMQuaternionInverse(viewRot));
	world *= XMMatrixTranslation(position.x, position.y, position.z);
	
	auto proj = camera->GetProjMatrix();

	// DitherLevel
	auto ditherLevel = (1.0 - ((transform->position - camera->transform->position).length() - 3) / 3.0) * 16;

	currentTransform->World = world;
	currentTransform->View = view;
	currentTransform->Proj = proj;
	currentTransform->DitherLevel = ditherLevel;

	// SceneParameter
	auto currentScene = m_pSceneCB[currentIndex]->GetPtr<SceneParameter>();

	auto cameraPos = camera->transform->position;
	currentScene->CameraPosition = cameraPos;

	auto targetPos = camera->GetFocusPosition();
	auto lightPos = targetPos + Vec3(0.5, 3.5, 2.5).normalized() * 500;
	auto lightView = XMMatrixLookAtRH(lightPos, targetPos, { 0, 1, 0 });
	auto lightViewRot = XMQuaternionRotationMatrix(lightView);

	currentScene->LightView = lightView;
	currentScene->LightProj = XMMatrixOrthographicRH(100, 100, 0.1f, 1000.0f);

	auto lightWorld = XMMatrixIdentity();
	lightWorld *= XMMatrixScaling(scale.x, scale.y, scale.z);
	lightWorld *= XMMatrixRotationQuaternion(rotation);
	lightWorld *= XMMatrixTranslation(0, m_height, 0); // 下端基準で回転
	lightWorld *= XMMatrixRotationQuaternion(XMQuaternionInverse(lightViewRot));
	lightWorld *= XMMatrixTranslation(position.x, position.y - m_height, position.z);
	currentScene->LightWorld = lightWorld;
}

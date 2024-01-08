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

BillboardRenderer::BillboardRenderer(std::shared_ptr<Texture2D> albedoTexture, std::shared_ptr<Texture2D> normalTexture)
{
	m_pTexture = albedoTexture;
	m_pNormalTexture = normalTexture;

	float width = albedoTexture->Metadata().width;
	float height = albedoTexture->Metadata().height;
	float width2 = width * 0.005;
	float height2 = height * 0.005;
	m_width = width2;
	m_height = height2;

	float pbrColor[] = { 0, 1, 0, 1 };
	float normalColor[] = { 0.5, 0.5, 1, 1 };
	std::shared_ptr pbrTexture = Texture2D::GetMono(pbrColor);
	//auto normalTexture = Texture2D::GetMono(normalColor);

	// ���f���̍쐬
	auto model = std::make_shared<Model>();
	std::vector<Vertex> vertices(4);
	std::vector<uint32_t> indices(6);
	auto& meshes = model->meshes;
	auto& materials = model->materials;

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
		// ���_�o�b�t�@�̐���
		auto vSize = sizeof(Vertex) * meshes[i].vertices.size();
		auto stride = sizeof(Vertex);
		auto vertices = meshes[i].vertices.data();
		auto pVB = std::make_unique<VertexBuffer>(vSize, stride, vertices);
		if (!pVB->IsValid())
		{
			printf("���_�o�b�t�@�̐����Ɏ��s\n");
			break;
		}
		meshes[i].vertexBuffer = std::move(pVB);

		// �C���f�b�N�X�o�b�t�@�̐���
		auto iSize = sizeof(uint32_t) * meshes[i].indices.size();
		auto indices = meshes[i].indices.data();
		auto pIB = std::make_unique<IndexBuffer>(iSize, indices);
		if (!pIB->IsValid())
		{
			printf("�C���f�b�N�X�o�b�t�@�̐����Ɏ��s\n");
			break;
		}
		meshes[i].indexBuffer = std::move(pIB);
	}

	m_pModel = model;
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
	/*auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
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
	/*auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
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
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();

	auto camera = GetEntity()->GetScene()->GetMainCamera();

	// Transform
	auto currentTransform = m_pTransformCB[currentIndex]->GetPtr<TransformParameter>();
	auto world = transform->GetBillboardWorldMatrix();
	auto view = camera->GetViewMatrix();
	auto viewRot = XMQuaternionRotationMatrix(view);
	auto proj = camera->GetProjMatrix();
	auto ditherLevel = (1.0 - ((transform->position - camera->transform->position).length() - 3) / 3.0) * 16;

	currentTransform->World = world;
	currentTransform->View = view;
	currentTransform->Proj = proj;
	currentTransform->DitherLevel = ditherLevel;

	// SceneParameter
	auto currentScene = m_pSceneCB[currentIndex]->GetPtr<SceneParameter>();
	auto cameraPos = camera->transform->position;
	auto targetPos = camera->GetFocusPosition();
	auto lightPos = targetPos + Vec3(0.5, 3.5, 2.5).normalized() * 500;
	auto lightView = XMMatrixLookAtRH(lightPos, targetPos, { 0, 1, 0 });
	auto lightViewRot = XMQuaternionRotationMatrix(lightView);

	currentScene->CameraPosition = cameraPos;
	currentScene->LightView = lightView;
	currentScene->LightProj = XMMatrixOrthographicRH(100, 100, 0.1f, 1000.0f);

	auto lightWorld = XMMatrixIdentity();
	lightWorld *= XMMatrixScalingFromVector(transform->scale);
	lightWorld *= XMMatrixRotationQuaternion(transform->rotation);
	lightWorld *= XMMatrixTranslation(0, m_height, 0); // ���[��ŉ�]
	lightWorld *= XMMatrixRotationQuaternion(XMQuaternionInverse(lightViewRot));
	lightWorld *= XMMatrixTranslationFromVector(transform->position - Vec3(0, m_height, 0));
	currentScene->LightWorld = lightWorld;
}

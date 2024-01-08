#include "Scene.h"
#include "Engine.h"
#include "ShadowMap.h"
#include "Entity.h"
#include "Camera.h"
#include "CollisionManager.h"
#include "Collider.h"
#include "PipelineState.h"
#include "SharedStruct.h"
#include "RootSignature.h"
#include "DescriptorHeap.h"
#include "ConstantBuffer.h"
#include "Texture2D.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "GBuffer.h"
#include "GBufferManager.h"
#include "Model.h"
#include "Game.h"
#include "ResourceManager.h"

Scene::Scene()
{
}

Scene::~Scene()
{
	printf("Delete Scene\n");

	m_pGBufferHeap->Free(m_pSkyboxHandle);
	m_pGBufferHeap->Free(m_pDiffuseMapHandle);
	m_pGBufferHeap->Free(m_pSpecularMapHandle);
	m_pGBufferHeap->Free(m_pBrdfHandle);
}

bool Scene::Init()
{
	// �G���e�B�e�B�̃N���A
	m_pEntities.clear();

	// �V���h�E�}�b�v�̐���
	//m_pShadowMap = std::make_unique<ShadowMap>();

	// CollisionManager�̐���
	m_pCollisionManager = std::make_unique<CollisionManager>();

	m_pRtvHeap = Engine::Get()->RtvHeap();
	m_pDsvHeap = Engine::Get()->DsvHeap();
	m_pGBufferHeap = Engine::Get()->GBufferHeap();

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
	m_pRootSignature = std::make_unique<RootSignature>(_countof(params), params);
	if (!m_pRootSignature->IsValid())
	{
		printf("���[�g�V�O�l�`���̐����Ɏ��s\n");
		return false;
	}

	RootSignatureParameter skyboxParams[] = {
		RSConstantBuffer,	// TransformParameter
		RSConstantBuffer,	// SceneParameter
		RSTexture,			// Skybox
	};
	m_pSkyboxRootSignature = std::make_unique<RootSignature>(_countof(skyboxParams), skyboxParams);
	if (!m_pSkyboxRootSignature->IsValid())
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
		m_pTransformCB[i] = std::make_unique<ConstantBuffer>(sizeof(TransformParameter));
		if (!m_pTransformCB[i]->IsValid())
		{
			printf("�ϊ��s��p�萔�o�b�t�@�̐����Ɏ��s\n");
			return false;
		}
	}

	const auto eye = XMVectorSet(0.5f, 7.5f, 2.5f, 0.0f);
	const auto target = XMVectorSet(0.0f, 4.0f, 0.0f, 0.0f);
	const auto up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	auto dir = Vec3(0.0f - 0.5f, 4.0f - 7.5f, 0.0f - 2.5f).normalized();

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		m_pSceneCB[i] = std::make_unique<ConstantBuffer>(sizeof(SceneParameter));
		if (!m_pSceneCB[i]->IsValid())
		{
			printf("���C�g�p�萔�o�b�t�@�̐����Ɏ��s\n");
			return false;
		}

		auto ptr = m_pSceneCB[i]->GetPtr<SceneParameter>();
		ptr->LightDirection = { dir.x, dir.y, dir.z };
		ptr->LightView = XMMatrixLookAtRH(eye, target, up);
		ptr->LightProj = XMMatrixOrthographicRH(45, 45, 0.1f, 100.0f);
		ptr->CameraPosition = { 0, 0, 1 };
	}

    return true;
}

void Scene::Update()
{
	// Camera�̍X�V
	for (auto& entity : m_pEntities) entity->BeforeCameraUpdate();
	for (auto& entity : m_pEntities) entity->CameraUpdate();

	// �G���e�B�e�B�̍X�V
	for (auto& entity : m_pEntities) entity->Update();

	// �����̍X�V
	for (auto& entity : m_pEntities) entity->PhysicsUpdate();

	// �Փ˔���
	m_pCollisionManager->Update();

	// �萔�o�b�t�@�̍X�V
	UpdateCB();
}
float ang = 0;
void Scene::Draw()
{
	// �����_�����O�̏���
	Engine::Get()->InitRender();

	// �V���h�E�}�b�v�̕`��
	Engine::Get()->GetShadowMap()->BeginRender();
	for (auto& entity : m_pEntities) entity->DrawShadow();
	Engine::Get()->GetShadowMap()->EndRender();

	/*Engine::Get()->BeginRenderMSAA();

	for (auto entity : m_pEntities) entity->Draw();
	for (auto entity : m_pEntities) entity->DrawAlpha();

	Engine::Get()->EndRenderMSAA();*/

	// �����_�����O�̊J�n
	Engine::Get()->BeginDeferredRender();

	// �f�v�X�v���p�X
	Engine::Get()->DepthPrePath();
	for (auto& entity : m_pEntities) entity->DrawDepth();

	// G-Buffer�ւ̏�������
	Engine::Get()->GBufferPath();
	for (auto& entity : m_pEntities)	entity->DrawGBuffer();

	// ���C�e�B���O�p�X
	Engine::Get()->LightingPath();
	DrawLighting();

	// Skybox�`��i�t�H���[�h�����_�����O�j
	DrawSkybox();

	// SSAO
	//Engine::Get()->SSAOPath();
	//DrawSSAO();

	//// �ڂ���
	//Engine::Get()->BlurHorizontalPath();
	//DrawBlurHorizontal();
	//Engine::Get()->BlurVerticalPath();
	//DrawBlurVertical();

	// �|�X�g�v���Z�X
	Engine::Get()->PostProcessPath();
	DrawPostProcess();

	// �A�E�g���C���`��i�t�H���[�h�����_�����O�j
	//for (auto entity : m_pEntities) entity->DrawOutline();

	// FXAA
	Engine::Get()->FXAAPath();
	DrawFXAA();

	Engine::Get()->EndRenderD3D();

	Engine::Get()->BeginRenderD2D();

	float x = cos(ang) * 100 + 1280 / 2.0;
	float y = sin(ang) * 100 + 720 / 2.0;
	D2D1_RECT_F rect = {
		x - 400, y - 100, x + 400, y + 100
	};
	ang += 0.04;
	Engine::Get()->drawText("normal", "white", L"DirectWrite�ŕ����\��\nHello, Direct2D & DirectWrite", rect);

	Engine::Get()->EndRenderD2D();

	// �����_�����O�̏I��
	Engine::Get()->EndDeferredRender();
}

void Scene::DrawLighting()
{
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();
	auto gbufferHeap = Engine::Get()->GBufferHeap()->GetHeap();
	auto gbufferManager = Engine::Get()->GetGBufferManager();
	auto shadowMap = Engine::Get()->GetShadowMap();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, m_pSceneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(m_pLightingPSO->Get());	// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("Position")->SrvHandle()->HandleGPU());	// �f�B�X�N���v�^�e�[�u�����Z�b�g
	commandList->SetGraphicsRootDescriptorTable(3, gbufferManager->Get("Normal")->SrvHandle()->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(4, gbufferManager->Get("Albedo")->SrvHandle()->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(5, gbufferManager->Get("MetallicRoughness")->SrvHandle()->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(6, gbufferManager->Get("Depth")->SrvHandle()->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(7, shadowMap->SrvHandle()->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(8, m_pDiffuseMapHandle.HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(9, m_pSpecularMapHandle.HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(10, m_pBrdfHandle.HandleGPU());

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawSkybox()
{
	if (m_pSkyboxTex == nullptr)
		return;

	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();
	auto gbufferHeap = Engine::Get()->GBufferHeap()->GetHeap();

	commandList->SetGraphicsRootSignature(m_pSkyboxRootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g
	//commandList->SetGraphicsRootConstantBufferView(1, m_pSceneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, m_pSkyboxHandle.HandleGPU());	// �f�B�X�N���v�^�e�[�u�����Z�b�g

	// ���b�V���̕`��
	auto vbView = m_skyboxMesh.vertexBuffer->View();
	auto ibView = m_skyboxMesh.indexBuffer->View();

	commandList->SetPipelineState(m_pSkyboxPSO->Get());				// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->IASetVertexBuffers(0, 1, &vbView);					// ���_�o�b�t�@���X���b�g0�Ԃ��g����1�����ݒ肷��
	commandList->IASetIndexBuffer(&ibView);							// �C���f�b�N�X�o�b�t�@���Z�b�g

	commandList->DrawIndexedInstanced(m_skyboxMesh.indices.size(), 1, 0, 0, 0);
}

void Scene::DrawSSAO()
{
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();
	auto gbufferHeap = Engine::Get()->GBufferHeap()->GetHeap();
	auto gbufferManager = Engine::Get()->GetGBufferManager();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g
	commandList->SetGraphicsRootConstantBufferView(1, m_pSceneCB[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(m_pSSAOPSO->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("Position")->SrvHandle()->HandleGPU()); // �f�B�X�N���v�^�e�[�u�����Z�b�g
	commandList->SetGraphicsRootDescriptorTable(3, gbufferManager->Get("Normal")->SrvHandle()->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(4, gbufferManager->Get("Albedo")->SrvHandle()->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(5, gbufferManager->Get("Depth")->SrvHandle()->HandleGPU());

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawBlurHorizontal()
{
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();
	auto gbufferHeap = Engine::Get()->GBufferHeap()->GetHeap();
	auto gbufferManager = Engine::Get()->GetGBufferManager();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(m_pBlurHorizontalPSO->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("SSAO")->SrvHandle()->HandleGPU()); // �f�B�X�N���v�^�e�[�u�����Z�b�g

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawBlurVertical()
{
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();
	auto gbufferHeap = Engine::Get()->GBufferHeap()->GetHeap();
	auto gbufferManager = Engine::Get()->GetGBufferManager();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(m_pBlurVerticalPSO->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("BlurredSSAO1")->SrvHandle()->HandleGPU()); // �f�B�X�N���v�^�e�[�u�����Z�b�g

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawPostProcess()
{
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();
	auto gbufferHeap = Engine::Get()->GBufferHeap()->GetHeap();
	auto gbufferManager = Engine::Get()->GetGBufferManager();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g
	commandList->SetGraphicsRootConstantBufferView(1, m_pSceneCB[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(m_pPostProcessPSO->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("Lighting")->SrvHandle()->HandleGPU());	// �f�B�X�N���v�^�e�[�u�����Z�b�g
	commandList->SetGraphicsRootDescriptorTable(3, gbufferManager->Get("Normal")->SrvHandle()->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(4, gbufferManager->Get("Albedo")->SrvHandle()->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(5, gbufferManager->Get("Depth")->SrvHandle()->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(6, gbufferManager->Get("Position")->SrvHandle()->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(7, gbufferManager->Get("BlurredSSAO2")->SrvHandle()->HandleGPU());

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawFXAA()
{
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();
	auto gbufferHeap = Engine::Get()->GBufferHeap()->GetHeap();
	auto gbufferManager = Engine::Get()->GetGBufferManager();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(m_pFXAAPSO->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("PostProcess")->SrvHandle()->HandleGPU());	// �f�B�X�N���v�^�e�[�u�����Z�b�g

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::CreateEntity(Entity* entity)
{
	std::unique_ptr<Entity> uentity;
	uentity.reset(entity);
	m_pEntities.push_back(std::move(uentity));

	entity->RegisterScene(this);
	entity->Init();
}

void Scene::SetMainCamera(Entity* camera)
{
	m_pMainCamera = camera->GetComponent<Camera>();
}

Camera* Scene::GetMainCamera()
{
	return m_pMainCamera;
}

//ShadowMap* Scene::GetShadowMap()
//{
//	return m_pShadowMap.get();
//}

CollisionManager* Scene::GetCollisionManager()
{
	return m_pCollisionManager.get();
}

void Scene::SetSkybox(const std::string path)
{
	auto device = Engine::Get()->Device();

	m_pSkyboxTex = Texture2D::Load(path + "EnvHDR.dds");

	// SRV���쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC skyboxDesc{};
	skyboxDesc.Format = m_pSkyboxTex->Format();
	skyboxDesc.TextureCube.MipLevels = m_pSkyboxTex->Metadata().mipLevels;
	skyboxDesc.TextureCube.MostDetailedMip = 0;
	skyboxDesc.TextureCube.ResourceMinLODClamp = 0;
	skyboxDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	skyboxDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

	m_pSkyboxHandle = Engine::Get()->GBufferHeap()->Alloc();	// GBufferHeap�ɒǉ�
	device->CreateShaderResourceView(m_pSkyboxTex->Resource(), &skyboxDesc, m_pSkyboxHandle.HandleCPU());	// �V�F�[�_�[���\�[�X�r���[�쐬

	// DiffuseMap
	m_pDiffuseMapTex = Texture2D::Load(path + "DiffuseHDR.dds");

	D3D12_SHADER_RESOURCE_VIEW_DESC diffuseDesc{};
	diffuseDesc.Format = m_pDiffuseMapTex->Format();
	diffuseDesc.TextureCube.MipLevels = m_pDiffuseMapTex->Metadata().mipLevels;
	diffuseDesc.TextureCube.MostDetailedMip = 0;
	diffuseDesc.TextureCube.ResourceMinLODClamp = 0;
	diffuseDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	diffuseDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

	m_pDiffuseMapHandle = Engine::Get()->GBufferHeap()->Alloc();
	device->CreateShaderResourceView(m_pDiffuseMapTex->Resource(), &diffuseDesc, m_pDiffuseMapHandle.HandleCPU());

	// SpecularMap
	m_pSpecularMapTex = Texture2D::Load(path + "SpecularHDR.dds");

	D3D12_SHADER_RESOURCE_VIEW_DESC specularDesc{};
	specularDesc.Format = m_pSpecularMapTex->Format();
	specularDesc.TextureCube.MipLevels = m_pSpecularMapTex->Metadata().mipLevels;
	specularDesc.TextureCube.MostDetailedMip = 0;
	specularDesc.TextureCube.ResourceMinLODClamp = 0;
	specularDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	specularDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

	m_pSpecularMapHandle = Engine::Get()->GBufferHeap()->Alloc();
	device->CreateShaderResourceView(m_pSpecularMapTex->Resource(), &specularDesc, m_pSpecularMapHandle.HandleCPU());

	// brdfLUT
	m_pBrdfTex = Texture2D::Load(path + "Brdf.dds");

	D3D12_SHADER_RESOURCE_VIEW_DESC brdfDesc{};
	brdfDesc.Format = m_pBrdfTex->Format();
	brdfDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	brdfDesc.Texture2D.MipLevels = m_pBrdfTex->Metadata().mipLevels;
	brdfDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	m_pBrdfHandle = Engine::Get()->GBufferHeap()->Alloc();
	device->CreateShaderResourceView(m_pBrdfTex->Resource(), &brdfDesc, m_pBrdfHandle.HandleCPU());


	// Mesh���쐬
	float size = 500;
	std::vector<Vertex> vertices;
	vertices.resize(8);
	vertices[0].Position = { -1 * size, -1 * size, -1 * size };
	vertices[1].Position = {  1 * size, -1 * size, -1 * size };
	vertices[2].Position = { -1 * size,  1 * size, -1 * size };
	vertices[3].Position = {  1 * size,  1 * size, -1 * size };
	vertices[4].Position = { -1 * size, -1 * size,  1 * size };
	vertices[5].Position = {  1 * size, -1 * size,  1 * size };
	vertices[6].Position = { -1 * size,  1 * size,  1 * size };
	vertices[7].Position = {  1 * size,  1 * size,  1 * size };

	std::vector<uint32_t> indices = {
		3, 2, 0, 1, 3, 0,
		2, 6, 4, 0, 2, 4,
		5, 4, 6, 7, 5, 6,
		1, 5, 7, 3, 1, 7,
		3, 7, 2, 7, 6, 2,
		5, 1, 0, 4, 5, 0
	};

	m_skyboxMesh = Mesh{};
	m_skyboxMesh.vertices = vertices;
	m_skyboxMesh.indices = indices;

	// ���_�o�b�t�@�̐���
	auto vSize = sizeof(Vertex) * m_skyboxMesh.vertices.size();
	auto stride = sizeof(Vertex);
	auto pVB = std::make_unique<VertexBuffer>(vSize, stride, vertices.data());
	if (!pVB->IsValid())
	{
		printf("���_�o�b�t�@�̐����Ɏ��s\n");
		return;
	}
	m_skyboxMesh.vertexBuffer = std::move(pVB);

	// �C���f�b�N�X�o�b�t�@�̐���
	auto iSize = sizeof(uint32_t) * m_skyboxMesh.indices.size();
	auto pIB = std::make_unique<IndexBuffer>(iSize, indices.data());
	if (!pIB->IsValid())
	{
		printf("�C���f�b�N�X�o�b�t�@�̐����Ɏ��s\n");
		return;
	}
	m_skyboxMesh.indexBuffer = std::move(pIB);
}

bool Scene::PreparePSO()
{
	// ���C�e�B���O�p
	m_pLightingPSO = std::make_unique<PipelineState>();
	m_pLightingPSO->SetInputLayout({nullptr, 0});
	m_pLightingPSO->SetRootSignature(m_pRootSignature->Get());
	m_pLightingPSO->SetVS(L"ScreenVS.cso");
	m_pLightingPSO->SetPS(L"PBR2.cso");
	m_pLightingPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	auto desc = m_pLightingPSO->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;
	
	m_pLightingPSO->Create();
	if (!m_pLightingPSO->IsValid())
	{
		return false;
	}

	// SSAO�p
	m_pSSAOPSO = std::make_unique<PipelineState>();
	m_pSSAOPSO->SetInputLayout({ nullptr, 0 });
	m_pSSAOPSO->SetRootSignature(m_pRootSignature->Get());
	m_pSSAOPSO->SetVS(L"ScreenVS.cso");
	m_pSSAOPSO->SetPS(L"SSAO.cso");
	m_pSSAOPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = m_pSSAOPSO->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	m_pSSAOPSO->Create();
	if (!m_pSSAOPSO->IsValid())
	{
		return false;
	}

	// BlurHorizontal�p
	m_pBlurHorizontalPSO = std::make_unique<PipelineState>();
	m_pBlurHorizontalPSO->SetInputLayout({ nullptr, 0 });
	m_pBlurHorizontalPSO->SetRootSignature(m_pRootSignature->Get());
	m_pBlurHorizontalPSO->SetVS(L"ScreenVS.cso");
	m_pBlurHorizontalPSO->SetPS(L"GaussianBlurHorizontal.cso");
	m_pBlurHorizontalPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = m_pBlurHorizontalPSO->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	m_pBlurHorizontalPSO->Create();
	if (!m_pBlurHorizontalPSO->IsValid())
	{
		return false;
	}

	// BlurVertical�p
	m_pBlurVerticalPSO = std::make_unique<PipelineState>();
	m_pBlurVerticalPSO->SetInputLayout({ nullptr, 0 });
	m_pBlurVerticalPSO->SetRootSignature(m_pRootSignature->Get());
	m_pBlurVerticalPSO->SetVS(L"ScreenVS.cso");
	m_pBlurVerticalPSO->SetPS(L"GaussianBlurVertical.cso");
	m_pBlurVerticalPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = m_pBlurVerticalPSO->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	m_pBlurVerticalPSO->Create();
	if (!m_pBlurVerticalPSO->IsValid())
	{
		return false;
	}

	// PostProcess�p
	m_pPostProcessPSO = std::make_unique<PipelineState>();
	m_pPostProcessPSO->SetInputLayout({ nullptr, 0 });
	m_pPostProcessPSO->SetRootSignature(m_pRootSignature->Get());
	m_pPostProcessPSO->SetVS(L"ScreenVS.cso");
	m_pPostProcessPSO->SetPS(L"PostProcessPS.cso");
	m_pPostProcessPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = m_pPostProcessPSO->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	m_pPostProcessPSO->Create();
	if (!m_pPostProcessPSO->IsValid())
	{
		return false;
	}

	// FXAA�p
	m_pFXAAPSO = std::make_unique<PipelineState>();
	m_pFXAAPSO->SetInputLayout({ nullptr, 0 });
	m_pFXAAPSO->SetRootSignature(m_pRootSignature->Get());
	m_pFXAAPSO->SetVS(L"ScreenVS.cso");
	m_pFXAAPSO->SetPS(L"FXAA.cso");
	m_pFXAAPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = m_pFXAAPSO->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	m_pFXAAPSO->Create();
	if (!m_pFXAAPSO->IsValid())
	{
		return false;
	}

	// Skybox�p
	m_pSkyboxPSO = std::make_unique<PipelineState>();
	m_pSkyboxPSO->SetInputLayout(Vertex::InputLayout);
	m_pSkyboxPSO->SetRootSignature(m_pSkyboxRootSignature->Get());
	m_pSkyboxPSO->SetVS(L"SkyboxVS.cso");
	m_pSkyboxPSO->SetPS(L"SkyboxPS.cso");
	m_pSkyboxPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = m_pSkyboxPSO->GetDesc();
	desc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	// �f�v�X�o�b�t�@�ɂ͏������܂Ȃ�
	desc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	
	m_pSkyboxPSO->Create();
	if (!m_pSkyboxPSO->IsValid())
	{
		return false;
	}

	return true;
}

void Scene::UpdateCB()
{
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto camera = GetMainCamera();

	// Transform
	auto currentTransform = m_pTransformCB[currentIndex]->GetPtr<TransformParameter>();

	// world�s��
	auto world = XMMatrixIdentity();

	// view�s��
	//auto view = XMMatrixRotationQuaternion(camera->transform->rotation);
	auto view = camera->GetViewMatrix();

	// proj�s��
	auto proj = camera->GetProjMatrix();

	currentTransform->World = world;
	currentTransform->View = view;
	currentTransform->Proj = proj;

	// SceneParameter
	auto currentScene = m_pSceneCB[currentIndex]->GetPtr<SceneParameter>();

	auto cameraPos = camera->transform->position;
	currentScene->CameraPosition = cameraPos;

	auto targetPos = camera->GetFocusPosition();
	auto lightPos = targetPos + Vec3(0.5, 3.5, 2.5).normalized() * 500;
	currentScene->LightView = XMMatrixLookAtRH(lightPos, targetPos, { 0, 1, 0 });
	currentScene->LightProj = XMMatrixOrthographicRH(100, 100, 0.1f, 1000.0f);
	currentScene->LightColor = { 20, 20, 20 };
}


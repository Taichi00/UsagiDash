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

Scene::Scene()
{
}

Scene::~Scene()
{
	delete m_pLightingPSO;
	delete m_pCollisionManager;
	delete m_pShadowMap;

	for (auto entity : m_pEntities)
	{
		delete entity;
	}
}

bool Scene::Init()
{
	// �G���e�B�e�B�̃N���A
	m_pEntities.clear();

	// �V���h�E�}�b�v�̐���
	m_pShadowMap = new ShadowMap();

	// �V���h�E�}�b�v�̓o�^
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	m_pShadowHandle = g_Engine->GBufferHeap()->Alloc();
	g_Engine->Device()->CreateShaderResourceView(m_pShadowMap->Resource(), &srvDesc, m_pShadowHandle->HandleCPU());

	// CollisionManager�̐���
	m_pCollisionManager = new CollisionManager();

	// ���[�g�V�O�l�`���̐���
	RootSignatureParameter params[] = {
		RSConstantBuffer,	// SceneParameter
		RSTexture,			// Position
		RSTexture,			// Normal
		RSTexture,			// Albedo
		RSTexture,			// Depth
		RSTexture,			// ShadowMap
		RSTexture,			// Skybox
	};
	m_pRootSignature = new RootSignature(_countof(params), params);
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
	m_pSkyboxRootSignature = new RootSignature(_countof(skyboxParams), skyboxParams);
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
		m_pTransformCB[i] = new ConstantBuffer(sizeof(TransformParameter));
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
		m_pSceneCB[i] = new ConstantBuffer(sizeof(SceneParameter));
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
	for (auto entity : m_pEntities) entity->BeforeCameraUpdate();
	for (auto entity : m_pEntities) entity->CameraUpdate();

	// �G���e�B�e�B�̍X�V
	for (auto entity : m_pEntities) entity->Update();

	// �����̍X�V
	for (auto entity : m_pEntities) entity->PhysicsUpdate();

	// �Փ˔���
	m_pCollisionManager->Update();

	// �萔�o�b�t�@�̍X�V
	UpdateCB();
}

void Scene::Draw()
{
	// �����_�����O�̏���
	g_Engine->InitRender();

	// �V���h�E�}�b�v�̕`��
	m_pShadowMap->BeginRender();
	for (auto entity : m_pEntities) entity->DrawShadow();
	m_pShadowMap->EndRender();

	/*g_Engine->BeginRenderMSAA();

	for (auto entity : m_pEntities) entity->Draw();
	for (auto entity : m_pEntities) entity->DrawAlpha();

	g_Engine->EndRenderMSAA();*/

	// �����_�����O�̊J�n
	g_Engine->BeginDeferredRender();

	// �f�v�X�v���p�X
	g_Engine->DepthPrePath();
	for (auto entity : m_pEntities) entity->DrawDepth();

	// G-Buffer�ւ̏�������
	g_Engine->GBufferPath();
	for (auto entity : m_pEntities)	entity->DrawGBuffer();

	// ���C�e�B���O�p�X
	g_Engine->LightingPath();
	DrawLighting();

	// Skybox�`��i�t�H���[�h�����_�����O�j
	DrawSkybox();

	// �|�X�g�v���Z�X
	g_Engine->PostProcessPath();
	DrawPostProcess();

	// �A�E�g���C���`��i�t�H���[�h�����_�����O�j
	for (auto entity : m_pEntities) entity->DrawOutline();

	// FXAA
	g_Engine->FXAAPath();
	DrawFXAA();

	// �����_�����O�̏I��
	g_Engine->EndDeferredRender();
}

void Scene::DrawLighting()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto gbufferHeap = g_Engine->GBufferHeap()->GetHeap();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, m_pSceneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(m_pLightingPSO->Get());	// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(1, g_Engine->GetGBuffer()->pPositionSrvHandle->HandleGPU());	// �f�B�X�N���v�^�e�[�u�����Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, g_Engine->GetGBuffer()->pNormalSrvHandle->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(3, g_Engine->GetGBuffer()->pAlbedoSrvHandle->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(4, g_Engine->GetGBuffer()->pDepthSrvHandle->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(5, m_pShadowHandle->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(6, m_pSkyboxHandle->HandleGPU());

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawSkybox()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto gbufferHeap = g_Engine->GBufferHeap()->GetHeap();

	commandList->SetGraphicsRootSignature(m_pSkyboxRootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g
	//commandList->SetGraphicsRootConstantBufferView(1, m_pSceneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, m_pSkyboxHandle->HandleGPU());	// �f�B�X�N���v�^�e�[�u�����Z�b�g

	// ���b�V���̕`��
	auto vbView = m_skyboxMesh.pVertexBuffer->View();
	auto ibView = m_skyboxMesh.pIndexBuffer->View();

	commandList->SetPipelineState(m_pSkyboxPSO->Get());				// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->IASetVertexBuffers(0, 1, &vbView);					// ���_�o�b�t�@���X���b�g0�Ԃ��g����1�����ݒ肷��
	commandList->IASetIndexBuffer(&ibView);							// �C���f�b�N�X�o�b�t�@���Z�b�g

	commandList->DrawIndexedInstanced(m_skyboxMesh.Indices.size(), 1, 0, 0, 0);
}

void Scene::DrawPostProcess()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto gbufferHeap = g_Engine->GBufferHeap()->GetHeap();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(m_pPostProcessPSO->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(1, g_Engine->GetGBuffer()->pLightingSrvHandle->HandleGPU());	// �f�B�X�N���v�^�e�[�u�����Z�b�g
	commandList->SetGraphicsRootDescriptorTable(2, g_Engine->GetGBuffer()->pNormalSrvHandle->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(3, g_Engine->GetGBuffer()->pAlbedoSrvHandle->HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(4, g_Engine->GetGBuffer()->pDepthSrvHandle->HandleGPU());

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawFXAA()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto gbufferHeap = g_Engine->GBufferHeap()->GetHeap();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// �f�B�X�N���v�^�q�[�v���Z�b�g
	commandList->SetPipelineState(m_pFXAAPSO->Get());		// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetGraphicsRootDescriptorTable(1, g_Engine->GetGBuffer()->pPostProcessSrvHandle->HandleGPU());	// �f�B�X�N���v�^�e�[�u�����Z�b�g

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::CreateEntity(Entity* entity)
{
	m_pEntities.push_back(entity);
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

ShadowMap* Scene::GetShadowMap()
{
	return m_pShadowMap;
}

CollisionManager* Scene::GetCollisionManager()
{
	return m_pCollisionManager;
}

void Scene::SetSkybox(const wchar_t* path)
{
	m_pSkyboxTex = Texture2D::Get(path);

	// SRV���쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.Format = m_pSkyboxTex->Format();
	desc.TextureCube.MipLevels = 1;
	desc.TextureCube.MostDetailedMip = 0;
	desc.TextureCube.ResourceMinLODClamp = 0;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

	m_pSkyboxHandle = g_Engine->GBufferHeap()->Alloc();	// GBufferHeap�ɒǉ�
	auto resource = m_pSkyboxTex->Resource();

	g_Engine->Device()->CreateShaderResourceView(resource, &desc, m_pSkyboxHandle->HandleCPU());	// �V�F�[�_�[���\�[�X�r���[�쐬

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
	m_skyboxMesh.Vertices = vertices;
	m_skyboxMesh.Indices = indices;

	// ���_�o�b�t�@�̐���
	auto vSize = sizeof(Vertex) * m_skyboxMesh.Vertices.size();
	auto stride = sizeof(Vertex);
	auto pVB = new VertexBuffer(vSize, stride, vertices.data());
	if (!pVB->IsValid())
	{
		printf("���_�o�b�t�@�̐����Ɏ��s\n");
		return;
	}
	m_skyboxMesh.pVertexBuffer = pVB;

	// �C���f�b�N�X�o�b�t�@�̐���
	auto iSize = sizeof(uint32_t) * m_skyboxMesh.Indices.size();
	auto pIB = new IndexBuffer(iSize, indices.data());
	if (!pIB->IsValid())
	{
		printf("�C���f�b�N�X�o�b�t�@�̐����Ɏ��s\n");
		return;
	}
	m_skyboxMesh.pIndexBuffer = pIB;
}

bool Scene::PreparePSO()
{
	// ���C�e�B���O�p
	m_pLightingPSO = new PipelineState();
	m_pLightingPSO->SetInputLayout({nullptr, 0});
	m_pLightingPSO->SetRootSignature(m_pRootSignature->Get());
	m_pLightingPSO->SetVS(L"../x64/Debug/ScreenVS.cso");
	m_pLightingPSO->SetPS(L"../x64/Debug/DeferredPS.cso");
	m_pLightingPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	auto desc = m_pLightingPSO->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	m_pLightingPSO->Create();
	if (!m_pLightingPSO->IsValid())
	{
		return false;
	}

	// PostProcess�p
	m_pPostProcessPSO = new PipelineState();
	m_pPostProcessPSO->SetInputLayout({ nullptr, 0 });
	m_pPostProcessPSO->SetRootSignature(m_pRootSignature->Get());
	m_pPostProcessPSO->SetVS(L"../x64/Debug/ScreenVS.cso");
	m_pPostProcessPSO->SetPS(L"../x64/Debug/PostProcessPS.cso");
	m_pPostProcessPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = m_pPostProcessPSO->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	m_pPostProcessPSO->Create();
	if (!m_pPostProcessPSO->IsValid())
	{
		return false;
	}

	// FXAA�p
	m_pFXAAPSO = new PipelineState();
	m_pFXAAPSO->SetInputLayout({ nullptr, 0 });
	m_pFXAAPSO->SetRootSignature(m_pRootSignature->Get());
	m_pFXAAPSO->SetVS(L"../x64/Debug/ScreenVS.cso");
	m_pFXAAPSO->SetPS(L"../x64/Debug/FXAA.cso");
	m_pFXAAPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = m_pFXAAPSO->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	m_pFXAAPSO->Create();
	if (!m_pFXAAPSO->IsValid())
	{
		return false;
	}

	// Skybox�p
	m_pSkyboxPSO = new PipelineState();
	m_pSkyboxPSO->SetInputLayout(Vertex::InputLayout);
	m_pSkyboxPSO->SetRootSignature(m_pSkyboxRootSignature->Get());
	m_pSkyboxPSO->SetVS(L"../x64/Debug/SkyboxVS.cso");
	m_pSkyboxPSO->SetPS(L"../x64/Debug/SkyboxPS.cso");
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
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto camera = GetMainCamera();

	// Transform
	auto currentTransform = m_pTransformCB[currentIndex]->GetPtr<TransformParameter>();

	// world�s��
	auto world = XMMatrixIdentity();

	// view�s��
	auto view = XMMatrixRotationQuaternion(camera->transform->rotation);

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
}


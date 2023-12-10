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
	// エンティティのクリア
	m_pEntities.clear();

	// シャドウマップの生成
	m_pShadowMap = new ShadowMap();

	// シャドウマップの登録
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	m_pShadowHandle = g_Engine->GBufferHeap()->Alloc();
	g_Engine->Device()->CreateShaderResourceView(m_pShadowMap->Resource(), &srvDesc, m_pShadowHandle->HandleCPU());

	// CollisionManagerの生成
	m_pCollisionManager = new CollisionManager();

	// ルートシグネチャの生成
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
		printf("ルートシグネチャの生成に失敗\n");
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
		printf("ルートシグネチャの生成に失敗\n");
		return false;
	}

	// PipelineStateObjectの生成
	if (!PreparePSO())
	{
		printf("Scene用PSOの生成に失敗しました\n");
	}

	// 定数バッファの生成
	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		m_pTransformCB[i] = new ConstantBuffer(sizeof(TransformParameter));
		if (!m_pTransformCB[i]->IsValid())
		{
			printf("変換行列用定数バッファの生成に失敗\n");
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
			printf("ライト用定数バッファの生成に失敗\n");
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
	// Cameraの更新
	for (auto entity : m_pEntities) entity->BeforeCameraUpdate();
	for (auto entity : m_pEntities) entity->CameraUpdate();

	// エンティティの更新
	for (auto entity : m_pEntities) entity->Update();

	// 物理の更新
	for (auto entity : m_pEntities) entity->PhysicsUpdate();

	// 衝突判定
	m_pCollisionManager->Update();

	// 定数バッファの更新
	UpdateCB();
}

void Scene::Draw()
{
	// レンダリングの準備
	g_Engine->InitRender();

	// シャドウマップの描画
	m_pShadowMap->BeginRender();
	for (auto entity : m_pEntities) entity->DrawShadow();
	m_pShadowMap->EndRender();

	/*g_Engine->BeginRenderMSAA();

	for (auto entity : m_pEntities) entity->Draw();
	for (auto entity : m_pEntities) entity->DrawAlpha();

	g_Engine->EndRenderMSAA();*/

	// レンダリングの開始
	g_Engine->BeginDeferredRender();

	// デプスプリパス
	g_Engine->DepthPrePath();
	for (auto entity : m_pEntities) entity->DrawDepth();

	// G-Bufferへの書き込み
	g_Engine->GBufferPath();
	for (auto entity : m_pEntities)	entity->DrawGBuffer();

	// ライティングパス
	g_Engine->LightingPath();
	DrawLighting();

	// Skybox描画（フォワードレンダリング）
	DrawSkybox();

	// ポストプロセス
	g_Engine->PostProcessPath();
	DrawPostProcess();

	// アウトライン描画（フォワードレンダリング）
	for (auto entity : m_pEntities) entity->DrawOutline();

	// FXAA
	g_Engine->FXAAPath();
	DrawFXAA();

	// レンダリングの終了
	g_Engine->EndDeferredRender();
}

void Scene::DrawLighting()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto gbufferHeap = g_Engine->GBufferHeap()->GetHeap();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, m_pSceneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetPipelineState(m_pLightingPSO->Get());	// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(1, g_Engine->GetGBuffer()->pPositionSrvHandle->HandleGPU());	// ディスクリプタテーブルをセット
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

	commandList->SetGraphicsRootSignature(m_pSkyboxRootSignature->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());	// 定数バッファをセット
	//commandList->SetGraphicsRootConstantBufferView(1, m_pSceneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetGraphicsRootDescriptorTable(2, m_pSkyboxHandle->HandleGPU());	// ディスクリプタテーブルをセット

	// メッシュの描画
	auto vbView = m_skyboxMesh.pVertexBuffer->View();
	auto ibView = m_skyboxMesh.pIndexBuffer->View();

	commandList->SetPipelineState(m_pSkyboxPSO->Get());				// パイプラインステートをセット
	commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
	commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット

	commandList->DrawIndexedInstanced(m_skyboxMesh.Indices.size(), 1, 0, 0, 0);
}

void Scene::DrawPostProcess()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto gbufferHeap = g_Engine->GBufferHeap()->GetHeap();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ルートシグネチャをセット
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetPipelineState(m_pPostProcessPSO->Get());		// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(1, g_Engine->GetGBuffer()->pLightingSrvHandle->HandleGPU());	// ディスクリプタテーブルをセット
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

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ルートシグネチャをセット
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetPipelineState(m_pFXAAPSO->Get());		// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(1, g_Engine->GetGBuffer()->pPostProcessSrvHandle->HandleGPU());	// ディスクリプタテーブルをセット

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

	// SRVを作成
	D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.Format = m_pSkyboxTex->Format();
	desc.TextureCube.MipLevels = 1;
	desc.TextureCube.MostDetailedMip = 0;
	desc.TextureCube.ResourceMinLODClamp = 0;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

	m_pSkyboxHandle = g_Engine->GBufferHeap()->Alloc();	// GBufferHeapに追加
	auto resource = m_pSkyboxTex->Resource();

	g_Engine->Device()->CreateShaderResourceView(resource, &desc, m_pSkyboxHandle->HandleCPU());	// シェーダーリソースビュー作成

	// Meshを作成
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

	// 頂点バッファの生成
	auto vSize = sizeof(Vertex) * m_skyboxMesh.Vertices.size();
	auto stride = sizeof(Vertex);
	auto pVB = new VertexBuffer(vSize, stride, vertices.data());
	if (!pVB->IsValid())
	{
		printf("頂点バッファの生成に失敗\n");
		return;
	}
	m_skyboxMesh.pVertexBuffer = pVB;

	// インデックスバッファの生成
	auto iSize = sizeof(uint32_t) * m_skyboxMesh.Indices.size();
	auto pIB = new IndexBuffer(iSize, indices.data());
	if (!pIB->IsValid())
	{
		printf("インデックスバッファの生成に失敗\n");
		return;
	}
	m_skyboxMesh.pIndexBuffer = pIB;
}

bool Scene::PreparePSO()
{
	// ライティング用
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

	// PostProcess用
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

	// FXAA用
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

	// Skybox用
	m_pSkyboxPSO = new PipelineState();
	m_pSkyboxPSO->SetInputLayout(Vertex::InputLayout);
	m_pSkyboxPSO->SetRootSignature(m_pSkyboxRootSignature->Get());
	m_pSkyboxPSO->SetVS(L"../x64/Debug/SkyboxVS.cso");
	m_pSkyboxPSO->SetPS(L"../x64/Debug/SkyboxPS.cso");
	m_pSkyboxPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = m_pSkyboxPSO->GetDesc();
	desc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	// デプスバッファには書き込まない
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

	// world行列
	auto world = XMMatrixIdentity();

	// view行列
	auto view = XMMatrixRotationQuaternion(camera->transform->rotation);

	// proj行列
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


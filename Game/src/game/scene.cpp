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
#include "engine/pipeline_state_manager.h"
#include "game/resource/texture2d.h"
#include "engine/vertex_buffer.h"
#include "engine/index_buffer.h"
#include "engine/buffer.h"
#include "engine/buffer_manager.h"
#include "game/resource/model.h"
#include "game/game.h"
#include "game/resource_manager.h"
#include "engine/engine2d.h"
#include <chrono>

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

void Scene::Awake()
{
	// ルートエンティティを生成
	root_entity_ = std::make_unique<Entity>("Root");
}

bool Scene::Init()
{
	rtv_heap_ = Game::Get()->GetEngine()->RtvHeap();
	dsv_heap_ = Game::Get()->GetEngine()->DsvHeap();
	gbuffer_heap_ = Game::Get()->GetEngine()->SrvHeap();

	// ルートシグネチャの生成
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
		RSTexture,			
	};
	root_signature_ = std::make_unique<RootSignature>((int)_countof(params), params);
	if (!root_signature_->IsValid())
	{
		printf("ルートシグネチャの生成に失敗\n");
		return false;
	}

	RootSignatureParameter mesh_params[] = {
		RSConstantBuffer,
		RSConstantBuffer,
		RSConstantBuffer,
		RSConstantBuffer,
		RSTexture,
		RSTexture,
		RSTexture,
	};
	mesh_root_signature_ = std::make_unique<RootSignature>((int)_countof(mesh_params), mesh_params);
	if (!mesh_root_signature_->IsValid())
	{
		printf("ルートシグネチャの生成に失敗\n");
		return false;
	}

	RootSignatureParameter skybox_params[] = {
		RSConstantBuffer,	// TransformParameter
		RSConstantBuffer,	// SceneParameter
		RSTexture,			// Skybox
	};
	skybox_root_signature_ = std::make_unique<RootSignature>((int)_countof(skybox_params), skybox_params);
	if (!skybox_root_signature_->IsValid())
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
		transform_cb_[i] = std::make_unique<ConstantBuffer>(sizeof(TransformParameter));
		if (!transform_cb_[i]->IsValid())
		{
			printf("変換行列用定数バッファの生成に失敗\n");
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
			printf("ライト用定数バッファの生成に失敗\n");
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

void Scene::Update(const float delta_time)
{
	UpdateEntityList();

	// Cameraの更新
	for (auto& entity : entity_list_) entity->BeforeCameraUpdate(delta_time);
	for (auto& entity : entity_list_) entity->CameraUpdate(delta_time);

	// エンティティの更新
	for (auto& entity : entity_list_) entity->Update(delta_time);

	// 物理の更新
	for (auto& entity : entity_list_) entity->PhysicsUpdate(delta_time);

	// 衝突判定
	Game::Get()->GetCollisionManager()->Update(delta_time);

	// 変換行列の更新
	for (auto& entity : entity_list_) entity->TransformUpdate(delta_time);

	// 定数バッファの更新
	UpdateCB();
}

void Scene::AfterUpdate()
{
	// エンティティを削除
	DestroyEntities();
}

void Scene::Draw()
{
	auto engine = Game::Get()->GetEngine();

	for (auto& entity : entity_list_) entity->BeforeDraw();

	// レンダリングの準備
	engine->InitRender();

	// シャドウマップの描画
	engine->GetShadowMap()->BeginRender();
	for (auto& entity : entity_list_) entity->DrawShadow();
	engine->GetShadowMap()->EndRender();
	
	// レンダリングの開始
	engine->BeginDeferredRender();

	// デプスプリパス
	engine->DepthPrePath();
	for (auto& entity : entity_list_) entity->DrawDepth();

	// G-Bufferへの書き込み
	engine->GBufferPath();
	for (auto& entity : entity_list_) entity->DrawGBuffer();

	// ライティングパス
	engine->LightingPath();
	DrawLighting();

	// Skybox描画（フォワードレンダリング）
	DrawSkybox();

	//// SSAO
	//engine->SSAOPath();
	//DrawSSAO();

	//// ぼかし
	//engine->BlurHorizontalPath();
	//DrawBlurHorizontal();
	//engine->BlurVerticalPath();
	//DrawBlurVertical();
	
	// アウトライン描画（フォワードレンダリング）
	for (auto& entity : entity_list_) entity->DrawOutline();

	// ポストプロセス
	engine->PostProcessPath();
	DrawPostProcess();

	// FXAA
	engine->FXAAPath();
	DrawFXAA();
	
	engine->EndRenderD3D();
	
	// 2D描画
	engine->GetEngine2D()->BeginRenderD2D();
	for (auto& entity : entity_list_) entity->Draw2D();
	engine->GetEngine2D()->EndRenderD2D();

	// レンダリングの終了
	engine->EndDeferredRender();
}

void Scene::OnDestroy()
{
	auto& entities = dont_destroy_entities_;

	ExecuteOnAllEntities([&entities](Entity& entity) {
		auto it = std::find(entities.begin(), entities.end(), &entity);

		if (it == entities.end())
		{
			entity.OnDestroy();
		}
		});
}

void Scene::DrawLighting()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->SrvHeap()->GetHeap();
	auto gbufferManager = Game::Get()->GetEngine()->GetBufferManager();
	auto shadowMap = Game::Get()->GetEngine()->GetShadowMap();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, scene_cb_[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetPipelineState(lighting_pso_->Get());	// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("Position")->SrvHandle().HandleGPU());	// ディスクリプタテーブルをセット
	commandList->SetGraphicsRootDescriptorTable(3, gbufferManager->Get("Normal")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(4, gbufferManager->Get("Albedo")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(5, gbufferManager->Get("MetallicRoughness")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(6, gbufferManager->Get("Depth")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(7, shadowMap->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(8, diffusemap_handle_.HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(9, specularmap_handle_.HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(10, brdf_handle_.HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(11, skybox_handle_.HandleGPU());

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawSkybox()
{
	if (skybox_tex_ == nullptr)
		return;

	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->SrvHeap()->GetHeap();

	commandList->SetGraphicsRootSignature(skybox_root_signature_->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());	// 定数バッファをセット
	//commandList->SetGraphicsRootConstantBufferView(1, m_pSceneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetGraphicsRootDescriptorTable(2, skybox_handle_.HandleGPU());	// ディスクリプタテーブルをセット

	// メッシュの描画
	auto vbView = skybox_mesh_.vertex_buffer->View();
	auto ibView = skybox_mesh_.index_buffer->View();

	commandList->SetPipelineState(skybox_pso_->Get());				// パイプラインステートをセット
	commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
	commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット

	commandList->DrawIndexedInstanced((UINT)skybox_mesh_.indices.size(), 1, 0, 0, 0);
}

void Scene::DrawSSAO()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->SrvHeap()->GetHeap();
	auto gbufferManager = Game::Get()->GetEngine()->GetBufferManager();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetPipelineState(ssao_pso_->Get());		// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("Position")->SrvHandle().HandleGPU()); // ディスクリプタテーブルをセット
	commandList->SetGraphicsRootDescriptorTable(3, gbufferManager->Get("Normal")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(4, gbufferManager->Get("Albedo")->SrvHandle().HandleGPU());
	commandList->SetGraphicsRootDescriptorTable(5, gbufferManager->Get("Depth")->SrvHandle().HandleGPU());

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawBlurHorizontal()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->SrvHeap()->GetHeap();
	auto gbufferManager = Game::Get()->GetEngine()->GetBufferManager();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ルートシグネチャをセット
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetPipelineState(blur_horizontal_pso_->Get());		// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("SSAO")->SrvHandle().HandleGPU()); // ディスクリプタテーブルをセット

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawBlurVertical()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->SrvHeap()->GetHeap();
	auto gbufferManager = Game::Get()->GetEngine()->GetBufferManager();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ルートシグネチャをセット
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetPipelineState(blur_vertical_pso_->Get());		// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("BlurredSSAO1")->SrvHandle().HandleGPU()); // ディスクリプタテーブルをセット

	commandList->DrawInstanced(4, 1, 0, 0);
}

void Scene::DrawPostProcess()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto gbufferHeap = Game::Get()->GetEngine()->SrvHeap()->GetHeap();
	auto gbufferManager = Game::Get()->GetEngine()->GetBufferManager();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetPipelineState(postprocess_pso_->Get());		// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("Lighting")->SrvHandle().HandleGPU());	// ディスクリプタテーブルをセット
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
	auto gbufferHeap = Game::Get()->GetEngine()->SrvHeap()->GetHeap();
	auto gbufferManager = Game::Get()->GetEngine()->GetBufferManager();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ルートシグネチャをセット
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D12DescriptorHeap* heaps[] = {
		gbufferHeap,
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetPipelineState(fxaa_pso_->Get());		// パイプラインステートをセット
	commandList->SetGraphicsRootDescriptorTable(2, gbufferManager->Get("PostProcess")->SrvHandle().HandleGPU());	// ディスクリプタテーブルをセット
	
	commandList->DrawInstanced(4, 1, 0, 0);
}

Entity* Scene::CreateEntity(Entity* entity)
{
	root_entity_->AddChild(entity);

	entity->ExecuteOnAllChildrenBack([this](Entity& e) {
		e.RegisterScene(this);
		e.Init();
		});

	return entity;
}

void Scene::AddDestroyEntity(Entity* entity)
{
	destroy_entities_.push_back(entity);
}

bool Scene::DontDestroyOnLoad(Entity* entity)
{
	// すでに追加していたらやめる
	if (std::find(dont_destroy_entities_.begin(), dont_destroy_entities_.end(), entity) != dont_destroy_entities_.end())
		return false;

	dont_destroy_entities_.push_back(entity);
	return true;
}

std::vector<std::unique_ptr<Entity>> Scene::MoveDontDestroyEntities()
{
	std::vector<std::unique_ptr<Entity>> entities;
	
	for (auto dd_entity : dont_destroy_entities_)
	{
		auto e = root_entity_->FindEntityIf([&dd_entity](Entity& entity) {
			return &entity == dd_entity;
			});

		auto parent = e->Parent();

		if (!parent)
			continue;

		entities.push_back(parent->MoveChild(e));
	}

	return entities;
}

Entity* Scene::FindEntity(const std::string& name)
{
	return root_entity_->FindEntityIf([&name](Entity& entity) {
		return entity.name == name;
		});
}

Entity* Scene::FindEntityWithTag(const std::string& tag)
{
	return root_entity_->FindEntityIf([&tag](Entity& entity) {
		return entity.tag == tag;
		});
}

Entity* Scene::RootEntity()
{
	return root_entity_.get();
}

void Scene::SetMainCamera(Entity* camera)
{
	main_camera_ = camera->GetComponent<Camera>();
}

Camera* Scene::GetMainCamera()
{
	return main_camera_;
}

void Scene::SetSkybox(const std::wstring& path)
{
	auto engine = Game::Get()->GetEngine();
	auto heap = engine->SrvHeap();

	skybox_tex_ = LoadResource<Texture2D>(path + L"/EnvHDR.dds");
	skybox_handle_ = heap->Alloc();
	engine->CreateShaderResourceViewCube(*skybox_tex_, skybox_handle_);

	diffusemap_tex_ = LoadResource<Texture2D>(path + L"/DiffuseHDR.dds");
	diffusemap_handle_ = heap->Alloc();
	engine->CreateShaderResourceViewCube(*diffusemap_tex_, diffusemap_handle_);

	specularmap_tex_ = LoadResource<Texture2D>(path + L"/SpecularHDR.dds");
	specularmap_handle_ = heap->Alloc();
	engine->CreateShaderResourceViewCube(*specularmap_tex_, specularmap_handle_);

	brdf_tex_ = LoadResource<Texture2D>(path + L"/Brdf.dds");
	brdf_handle_ = heap->Alloc();
	engine->CreateShaderResourceView2D(*brdf_tex_, brdf_handle_);


	// Meshを作成
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

	// 頂点バッファ、インデックスバッファの生成
	skybox_mesh_.vertex_buffer = engine->CreateVertexBuffer(vertices);
	skybox_mesh_.index_buffer = engine->CreateIndexBuffer(indices);
}

bool Scene::PreparePSO()
{
	auto pm = Game::Get()->GetEngine()->GetPipelineStateManager();

	// ライティング用
	lighting_pso_ = pm->Create("Lighting");
	lighting_pso_->SetInputLayout({nullptr, 0});
	lighting_pso_->SetRootSignature(root_signature_->Get());
	lighting_pso_->SetVS(L"screen_vs.cso");
	lighting_pso_->SetPS(L"pbr_ps.cso");
	lighting_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	lighting_pso_->SetRTVFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);

	auto desc = lighting_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;
	
	lighting_pso_->Create();
	if (!lighting_pso_->IsValid())
	{
		return false;
	}

	// SSAO用
	ssao_pso_ = pm->Create("SSAO");
	ssao_pso_->SetInputLayout({ nullptr, 0 });
	ssao_pso_->SetRootSignature(root_signature_->Get());
	ssao_pso_->SetVS(L"screen_vs.cso");
	ssao_pso_->SetPS(L"ssao_ps.cso");
	ssao_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = ssao_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	ssao_pso_->Create();
	if (!ssao_pso_->IsValid())
	{
		return false;
	}

	// BlurHorizontal用
	blur_horizontal_pso_ = pm->Create("BlurHorizontal");
	blur_horizontal_pso_->SetInputLayout({ nullptr, 0 });
	blur_horizontal_pso_->SetRootSignature(root_signature_->Get());
	blur_horizontal_pso_->SetVS(L"screen_vs.cso");
	blur_horizontal_pso_->SetPS(L"gaussian_blur_horizontal.cso");
	blur_horizontal_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = blur_horizontal_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	blur_horizontal_pso_->Create();
	if (!blur_horizontal_pso_->IsValid())
	{
		return false;
	}

	// BlurVertical用
	blur_vertical_pso_ = pm->Create("BlurVertical");
	blur_vertical_pso_->SetInputLayout({ nullptr, 0 });
	blur_vertical_pso_->SetRootSignature(root_signature_->Get());
	blur_vertical_pso_->SetVS(L"screen_vs.cso");
	blur_vertical_pso_->SetPS(L"gaussian_blur_vertical.cso");
	blur_vertical_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = blur_vertical_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	blur_vertical_pso_->Create();
	if (!blur_vertical_pso_->IsValid())
	{
		return false;
	}

	// PostProcess用
	postprocess_pso_ = pm->Create("PostProcess");
	postprocess_pso_->SetInputLayout({ nullptr, 0 });
	postprocess_pso_->SetRootSignature(root_signature_->Get());
	postprocess_pso_->SetVS(L"screen_vs.cso");
	postprocess_pso_->SetPS(L"postprocess_ps.cso");
	postprocess_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = postprocess_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	postprocess_pso_->Create();
	if (!postprocess_pso_->IsValid())
	{
		return false;
	}

	// FXAA用
	fxaa_pso_ = pm->Create("FXAA");
	fxaa_pso_->SetInputLayout({ nullptr, 0 });
	fxaa_pso_->SetRootSignature(root_signature_->Get());
	fxaa_pso_->SetVS(L"screen_vs.cso");
	fxaa_pso_->SetPS(L"fxaa_ps.cso");
	fxaa_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = fxaa_pso_->GetDesc();
	desc->DepthStencilState.DepthEnable = FALSE;

	fxaa_pso_->Create();
	if (!fxaa_pso_->IsValid())
	{
		return false;
	}

	// Skybox用
	skybox_pso_ = pm->Create("Skybox");
	skybox_pso_->SetInputLayout(Vertex::InputLayout);
	skybox_pso_->SetRootSignature(skybox_root_signature_->Get());
	skybox_pso_->SetVS(L"skybox_vs.cso");
	skybox_pso_->SetPS(L"skybox_ps.cso");
	skybox_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	skybox_pso_->SetRTVFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);

	desc = skybox_pso_->GetDesc();
	desc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	// デプスバッファには書き込まない
	desc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	
	skybox_pso_->Create();
	if (!skybox_pso_->IsValid())
	{
		return false;
	}

	outline_pso_ = pm->Create("Outline");
	outline_pso_->SetInputLayout(Vertex::InputLayout);
	outline_pso_->SetRootSignature(mesh_root_signature_->Get());
	outline_pso_->SetVS(L"outline_vs.cso");
	outline_pso_->SetPS(L"outline_ps.cso");
	outline_pso_->SetCullMode(D3D12_CULL_MODE_BACK);
	outline_pso_->SetRTVFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
	outline_pso_->Create();
	if (!outline_pso_->IsValid())
	{
		return false;
	}

	shadow_pso_ = pm->Create("Shadow");
	shadow_pso_->SetInputLayout(Vertex::InputLayout);
	shadow_pso_->SetRootSignature(mesh_root_signature_->Get());
	shadow_pso_->SetVS(L"shadow_vs.cso");
	shadow_pso_->SetPS(L"shadow_ps.cso");
	shadow_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	shadow_pso_->SetRTVFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
	shadow_pso_->Create();
	if (!shadow_pso_->IsValid())
	{
		return false;
	}

	// Depthプリパス用
	depth_pso_ = pm->Create("Depth");
	depth_pso_->SetInputLayout(Vertex::InputLayout);
	depth_pso_->SetRootSignature(mesh_root_signature_->Get());
	depth_pso_->SetVS(L"simple_vs.cso");
	depth_pso_->SetPS(L"depth_ps.cso");
	depth_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	depth_pso_->SetRTVFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

	/*auto desc = m_pDepthPSO->GetDesc();
	desc->NumRenderTargets = 0;
	desc->RTVFormats[0] = DXGI_FORMAT_UNKNOWN;*/

	depth_pso_->Create();
	if (!depth_pso_->IsValid())
	{
		return false;
	}

	// G-Buffer出力用
	gbuffer_pso_ = pm->Create("GBuffer");
	gbuffer_pso_->SetInputLayout(Vertex::InputLayout);
	gbuffer_pso_->SetRootSignature(mesh_root_signature_->Get());
	gbuffer_pso_->SetVS(L"simple_vs.cso");
	gbuffer_pso_->SetPS(L"gbuffer_ps.cso");
	gbuffer_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	desc = gbuffer_pso_->GetDesc();
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

	// world行列
	auto world = XMMatrixIdentity();

	// view行列
	auto view = camera->GetViewMatrix();

	// proj行列
	auto proj = camera->GetProjMatrix();

	currentTransform->world = world;
	currentTransform->view = view;
	currentTransform->proj = proj;

	// SceneParameter
	auto currentScene = scene_cb_[currentIndex]->GetPtr<SceneParameter>();

	auto cameraPos = camera->transform->position;
	currentScene->camera_position = cameraPos;

	auto targetPos = camera->GetFocusPosition();
	auto lightPos = targetPos + Vec3(0.5f, 3.5f, 2.5f).Normalized() * 500;
	currentScene->light_view = XMMatrixLookAtRH(lightPos, targetPos, { 0, 1, 0 });
	currentScene->light_proj = XMMatrixOrthographicRH(100, 100, 0.1f, 1000.0f);
	currentScene->light_color = Vec3(1.f, 1.f, 1.f) * 10;
}

void Scene::UpdateEntityList()
{
	entity_list_ = root_entity_->AllChildren();
}

void Scene::DestroyEntities()
{
	if (destroy_entities_.empty())
	{
		return;
	}

	Game::Get()->GetEngine()->WaitGPU();

	for (auto& entity : destroy_entities_)
	{
		auto destroy_entity = root_entity_->FindEntityIf([&entity](Entity& e) {
			return &e == entity;
			});

		// イベント通知
		destroy_entity->OnDestroy();

		// 削除
		destroy_entity->Parent()->RemoveChild(destroy_entity);
	}

	destroy_entities_.clear();
}


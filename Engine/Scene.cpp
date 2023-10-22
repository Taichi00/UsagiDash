#include "Scene.h"
#include "Engine.h"
#include "ShadowMap.h"
#include "Entity.h"
#include "Camera.h"

Scene::Scene()
{
}

Scene::~Scene()
{
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

    return true;
}

void Scene::Update()
{
	// Cameraの更新
	for (auto entity : m_pEntities)
	{
		entity->CameraUpdate();
	}

	// エンティティの更新
	for (auto entity : m_pEntities)
	{
		entity->Update();
	}
}

void Scene::Draw()
{
	// レンダリングの準備
	g_Engine->InitRender();

	// シャドウマップの描画
	m_pShadowMap->BeginRender();
	for (auto entity : m_pEntities)
	{
		entity->DrawShadow();
	}
	m_pShadowMap->EndRender();

	// レンダリング
	g_Engine->BeginRenderMSAA();
	for (auto entity : m_pEntities)
	{
		entity->Draw();
	}
	for (auto entity : m_pEntities)
	{
		entity->DrawAlpha();
	}
	g_Engine->EndRenderMSAA();
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

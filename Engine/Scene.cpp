#include "Scene.h"
#include "Engine.h"
#include "ShadowMap.h"
#include "Entity.h"
#include "Camera.h"
#include "CollisionManager.h"
#include "Collider.h"

Scene::Scene()
{
}

Scene::~Scene()
{
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

	// CollisionManager�̐���
	m_pCollisionManager = new CollisionManager();

    return true;
}

void Scene::Update()
{
	// Camera�̍X�V
	for (auto entity : m_pEntities)
	{
		entity->CameraUpdate();
	}

	// �G���e�B�e�B�̍X�V
	for (auto entity : m_pEntities)
	{
		entity->Update();
	}

	// �����̍X�V
	for (auto entity : m_pEntities)
	{
		entity->PhysicsUpdate();
	}

	// �Փ˔���
	m_pCollisionManager->Update();
}

void Scene::Draw()
{
	// �����_�����O�̏���
	g_Engine->InitRender();

	// �V���h�E�}�b�v�̕`��
	m_pShadowMap->BeginRender();
	for (auto entity : m_pEntities)
	{
		entity->DrawShadow();
	}
	m_pShadowMap->EndRender();

	// �����_�����O
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

CollisionManager* Scene::GetCollisionManager()
{
	return m_pCollisionManager;
}


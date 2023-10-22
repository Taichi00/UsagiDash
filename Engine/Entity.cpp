#include "Entity.h"
#include <stdio.h>
#include <typeinfo>

Entity::Entity()
{
	transform = new Transform();
	AddComponent(transform);

	m_pScene = nullptr;
}

Entity::~Entity()
{
	
}

void Entity::AddComponent(Component* component)
{
	std::string key = typeid(*component).name();

	// ���łɓ����^�̃R���|�[�l���g�����݂���ꍇ�A�ǉ����Ȃ�
	if (m_componentMap.find(key) != m_componentMap.end())
		return;

	m_components.push_back(component);
	m_componentMap[typeid(*component).name()] = component;

	component->RegisterEntity(this);
}


void Entity::RegisterScene(Scene* scene)
{
	m_pScene = scene;
}

Scene* Entity::GetScene()
{
	return m_pScene;
}

bool Entity::Init()
{
	for (auto component : m_components)
	{
		component->Init();
	}
	return true;
}

void Entity::CameraUpdate()
{
	for (auto component : m_components)
	{
		component->CameraUpdate();
	}
}

void Entity::Update()
{
	for (auto component : m_components)
	{
		component->Update();
	}
}

void Entity::Draw()
{
	for (auto component : m_components)
	{
		component->Draw();
	}
}

void Entity::DrawAlpha()
{
	for (auto component : m_components)
	{
		component->DrawAlpha();
	}
}

void Entity::DrawShadow()
{
	for (auto component : m_components)
	{
		component->DrawShadow();
	}
}


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

Component* Entity::AddComponent(Component* component)
{
	std::string key = typeid(*component).name();

	// すでに同じ型のコンポーネントが存在する場合、追加しない
	if (m_componentMap.find(key) != m_componentMap.end())
		return nullptr;

	m_components.push_back(component);
	m_componentMap[typeid(*component).name()] = component;

	component->RegisterEntity(this);

	return component;
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

void Entity::BeforeCameraUpdate()
{
	for (auto component : m_components)
	{
		component->BeforeCameraUpdate();
	}
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

void Entity::PhysicsUpdate()
{
	for (auto component : m_components)
	{
		component->PhysicsUpdate();
	}
}

void Entity::DrawDepth()
{
	for (auto component : m_components)
	{
		component->DrawDepth();
	}
}

void Entity::DrawGBuffer()
{
	for (auto component : m_components)
	{
		component->DrawGBuffer();
	}
}

void Entity::DrawOutline()
{
	for (auto component : m_components)
	{
		component->DrawOutline();
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

#include "Entity.h"
#include "Animator.h"
#include <stdio.h>
#include <typeinfo>

Entity::Entity()
{
	m_position = Vec3(0.0f, 0.0f, 0.0f);
	m_rotation = Vec3(0.0f, 0.0f, 0.0f);
	m_scale = Vec3(1.0f, 1.0f, 1.0f);

	m_pGame = nullptr;
}

Entity::~Entity()
{
}

void Entity::AddComponent(Component* component)
{
	std::string key = typeid(*component).name();

	// すでに同じ型のコンポーネントが存在する場合、追加しない
	if (m_componentMap.find(key) != m_componentMap.end())
		return;

	m_components.push_back(component);
	m_componentMap[typeid(*component).name()] = component;

	component->RegisterEntity(this);
}


void Entity::RegisterGame(Game* game)
{
	m_pGame = game;
}

Game* Entity::GetGame()
{
	return m_pGame;
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

void Entity::SetPosition(Vec3 position)
{
	m_position = position;
}

void Entity::SetPosition(float x, float y, float z)
{
	m_position = Vec3(x, y, z);
}

void Entity::SetRotation(Vec3 rotation)
{
	m_rotation = rotation;
}

void Entity::SetRotation(float x, float y, float z)
{
	m_rotation = Vec3(x, y, z);
}

void Entity::SetScale(Vec3 scale)
{
	m_scale = scale;
}

void Entity::SetScale(float x, float y, float z)
{
	m_scale = Vec3(x, y, z);
}

Vec3 Entity::Position()
{
	return m_position;
}

Vec3 Entity::Rotation()
{
	return m_rotation;
}

Vec3 Entity::Scale()
{
	return m_scale;
}

#include "Component.h"
#include "Entity.h"

Component::Component()
{
	
}

Component::~Component()
{
}

bool Component::Init()
{
	return true;
}

void Component::CameraUpdate()
{
}

void Component::Update()
{
}

void Component::PhysicsUpdate()
{
}

void Component::Draw()
{
}

void Component::DrawAlpha()
{
}

void Component::DrawShadow()
{
}

void Component::RegisterEntity(Entity* entity)
{
	m_pEntity = entity;
	transform = m_pEntity->transform;
}

Entity* Component::GetEntity()
{
	return m_pEntity;
}

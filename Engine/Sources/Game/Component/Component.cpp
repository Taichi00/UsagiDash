#include "Component.h"
#include "Entity.h"

Component::Component()
{
	
}

Component::~Component()
{
	printf("Delete Component\n");
}

bool Component::Init()
{
	return true;
}

void Component::BeforeCameraUpdate()
{
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

void Component::DrawDepth()
{
}

void Component::DrawGBuffer()
{
}

void Component::DrawOutline()
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

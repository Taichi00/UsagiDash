#include "Component.h"

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
}

Entity* Component::GetEntity()
{
	return m_pEntity;
}
